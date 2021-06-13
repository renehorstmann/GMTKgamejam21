#include "e/input.h"
#include "r/ro_single.h"
#include "r/ro_batch.h"
#include "u/pose.h"
#include "rhc/log.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "mathc/utils/color.h"
#include "camera.h"
#include "pixelparticles.h"
#include "feed.h"
#include "fish.h"

#define FRAMES 4
#define FPS 6.0
#define MOUTH_FPS 2.0
#define POINTER_DISTANCE 16.0
#define ACTIVE_SPEED_FACTOR 1.5
#define ACTIVE_SPEED_MAX 200.0

#define SWARM_RADIUS 56.0
#define SWARM_NEAR 28.0
#define SWARM_FEED_RADIUS_FAR 48.0
#define SWARM_FEED_RADIUS_NEAR 16.0

#define SPEED_FEED 20.0

#define SPEED_MAX 100.0
#define SPEED_FACTOR 10.0
#define SPEED_SWARM_CENTER_FACTOR 0.5
#define SPEED_LOCAL_CENTER_FACTOR 2.5
#define SPEED_KEEP_DISTANCE_FACTOR 6.0
#define SPEED_FOLLOW_ACTIVE_FACTOR 3.0

#define NUM_PARTICLES 64
#define PARTICLE_SIZE 2.0
#define PARTICLE_SPEED 100.0
#define PARTICLE_TIME 1.0
#define PARTICLE_ALPHA 1.5


struct FishGlobals_s fish;

static struct {
    RoBatch ro;

    vec2 swarm_center;

    struct {
        RoSingle ring_ro;
        int active;
        vec2 dst;
    } move;
} L;

static void emit_particles(float x, float y, vec3 color) {
    rParticleRect_s rects[NUM_PARTICLES];
    for (int i = 0; i < NUM_PARTICLES; i++) {
        rects[i] = r_particlerect_new();
        rects[i].pose = u_pose_new(x, y, PARTICLE_SIZE, PARTICLE_SIZE);
        float angle = sca_random_range(0, 2 * M_PI);
        float speed = sca_random_range(0.1 * PARTICLE_SPEED, PARTICLE_SPEED);
        rects[i].speed.x = sca_cos(angle) * speed;
        rects[i].speed.y = sca_sin(angle) * speed;
        rects[i].acc.xy = vec2_scale(rects[i].speed.xy, -0.5 / PARTICLE_TIME);
        rects[i].color.rgb = vec3_random_noise_vec(
                color,
                (vec3) {{0.2, 0.2, 0.2}});
        rects[i].color.a = PARTICLE_ALPHA;
        rects[i].color_speed.a = (float) -PARTICLE_ALPHA / PARTICLE_TIME;
        rects[i].start_time = pixelparticles.time;
    }
    pixelparticles_add(rects, NUM_PARTICLES);
}

static void update_fish(Fish_s *self, bool swarmed, int idx, float dtime) {
    self->speed = vec2_mix(self->speed, self->set_speed, 1.0 * dtime);
    vec2 delta = vec2_scale(self->speed, dtime);
    self->pos = vec2_add_vec(self->pos, delta);

    vec3 hsv = self->hsv;
    if (!swarmed) {
        hsv.v1 = 0;
        hsv.v2 -= 0.25;
    }

    self->L.animate_time = sca_mod(self->L.animate_time + dtime, FRAMES / FPS);
    int frame = self->L.animate_time * FPS;
    int mouth_frame = self->L.animate_time * MOUTH_FPS;
    if (self->L.looking_left && self->speed.x > 10)
        self->L.looking_left = false;
    if (!self->L.looking_left && self->speed.x < -10)
        self->L.looking_left = true;

    L.ro.rects[idx].pose = u_pose_new(self->pos.x, self->pos.y, 32, 32);
    L.ro.rects[idx].color.rgb = vec3_hsv2rgb(hsv);
    L.ro.rects[idx].uv = u_pose_new(0, 0, self->L.looking_left ? 1 : -1, 1);
    L.ro.rects[idx].sprite.x = frame;
    L.ro.rects[idx].sprite.y = (self->L.state == FISH_STATE_EAT && mouth_frame%2==0) ? 1 : 0;
}


static void pointer_callback(ePointer_s pointer, void *user_data) {
    if (pointer.id != 0)
        return;

    if (pointer.action == E_POINTER_UP) {
        if (L.move.active >= 0) {
            log_info("fish move end");
        }
        L.move.active = -1;
        return;
    }

    pointer.pos = mat4_mul_vec(camera.matrices_main.v_p_inv, pointer.pos);

    L.move.dst = pointer.pos.xy;

    if (pointer.action == E_POINTER_DOWN) {
        for (int i = 0; i < fish.swarmed_size; i++) {
            if (vec2_distance(pointer.pos.xy, fish.swarmed[i].pos) <= POINTER_DISTANCE) {
                L.move.active = i;
                log_info("fish move start with %i", i);
            }
        }
    }
}

static void swarm_center() {
    vec2 center = {{0, 0}};
    for (int i = 0; i < fish.swarmed_size; i++) {
        center = vec2_add_vec(center, fish.swarmed[i].pos);
    }
    L.swarm_center = vec2_div(center, fish.swarmed_size);
}

static void active_code() {
    assert(L.move.active >= 0 && L.move.active < fish.swarmed_size);
    vec2 fish_pos = fish.swarmed[L.move.active].pos;
    vec2 diff = vec2_sub_vec(L.move.dst, fish_pos);
    vec2 dir = vec2_normalize(diff);
    float distance = vec2_norm(diff);
    float speed = distance * ACTIVE_SPEED_FACTOR;
    speed = sca_min(speed, ACTIVE_SPEED_MAX);
    fish.swarmed[L.move.active].speed = vec2_scale(dir, speed);

    for (int i = 0; i < fish.alone_size; i++) {
        vec2 delta = vec2_sub_vec(fish.alone[i].pos, fish.swarmed[L.move.active].pos);
        float fish_distance = vec2_norm(delta);
        if (fish_distance <= SWARM_RADIUS) {
            fish_catch_alone(i);
            break;
        }
    }
}


static vec2 feed_position(int feed_idx, bool looking_left) {
    vec2 feed_pos = feed.feed[feed_idx].pos;
    if(looking_left)
        feed_pos.x += 8.0;
    else
        feed_pos.x -= 8.0;
    return feed_pos;
}

static bool check_feed(int fish_idx, float dtime) {
    fish.swarmed[fish_idx].L.state = FISH_STATE_SWIM;
    vec2 fish_pos = fish.swarmed[fish_idx].pos;

    int dst = -1;
    vec2 feed_pos;
    bool near = false;
    for(int i=0; i<feed.feed_size; i++) {
        vec2 pos = feed_position(i, fish.swarmed[fish_idx].L.looking_left);
        float dist = vec2_distance(pos, fish_pos);
        if(dist <= SWARM_FEED_RADIUS_NEAR) {
            dst = i;
            feed_pos = pos;
            near = true;
            break;
        }
        if(dist <= SWARM_FEED_RADIUS_FAR) {
            dst = i;
            feed_pos = pos;
        }
    }
    if(dst<0)
        return false;

    if(near) {
        fish.swarmed[fish_idx].L.state = FISH_STATE_EAT;
        fish.swarmed[fish_idx].set_speed = vec2_set(0);
        feed_eat(&feed.feed[dst], dtime);
        return true;
    }

    vec2 dir = vec2_normalize(vec2_sub_vec(feed_pos, fish_pos));
    fish.swarmed[fish_idx].set_speed = vec2_scale(dir, SPEED_FEED);
    return true;
}

static void swarm_code(int fish_idx, float dtime) {
    assert(fish_idx < fish.swarmed_size);

    if(check_feed(fish_idx, dtime))
        return;

    vec2 swarm_center_dir = {0};
    vec2 local_center_dir = {0};
    vec2 keep_distance_dir = {0};
    vec2 active_dir = {0};

    vec2 fish_pos = fish.swarmed[fish_idx].pos;

    int local_cnt = 0;
    int near_cnt = 0;

    swarm_center_dir = vec2_sub_vec(L.swarm_center, fish_pos);

    vec2 local_center = {0};
    for (int i = 0; i < fish.swarmed_size; i++) {
        vec2 delta = vec2_sub_vec(fish.swarmed[i].pos, fish_pos);
        float fish_distance = vec2_norm(delta);
        if (fish_distance <= SWARM_RADIUS) {
            local_cnt++;
            local_center = vec2_add_vec(local_center, fish.swarmed[i].pos);
            if (i == L.move.active) {
                active_dir = delta;
            }
        }
        if (fish_distance <= SWARM_NEAR) {
            near_cnt++;
            vec2 dir = vec2_normalize(delta);
            dir = vec2_scale(dir, SWARM_NEAR - fish_distance);
            keep_distance_dir = vec2_sub_vec(keep_distance_dir, dir);
        }
    }
    local_center = vec2_div(local_center, local_cnt);
    local_center_dir = vec2_sub_vec(local_center, fish_pos);

    swarm_center_dir = vec2_normalize(swarm_center_dir);
    local_center_dir = vec2_normalize(local_center_dir);
    keep_distance_dir = vec2_normalize(keep_distance_dir);
    active_dir = vec2_normalize(active_dir);

    vec2 speed = vec2_scale(swarm_center_dir, SPEED_SWARM_CENTER_FACTOR);
    speed = vec2_add_vec(speed, vec2_scale(local_center_dir, SPEED_LOCAL_CENTER_FACTOR));
    speed = vec2_add_vec(speed, vec2_scale(keep_distance_dir, SPEED_KEEP_DISTANCE_FACTOR));
    speed = vec2_add_vec(speed, vec2_scale(active_dir, SPEED_FOLLOW_ACTIVE_FACTOR));
    speed = vec2_scale(speed, SPEED_FACTOR);

    float speed_value = sca_min(vec2_norm(speed), SPEED_MAX);
    fish.swarmed[fish_idx].set_speed = vec2_scale(vec2_normalize(speed), speed_value);
}

void fish_init() {
    L.move.active = -1;

    e_input_register_pointer_event(pointer_callback, NULL);


    L.move.ring_ro = ro_single_new(camera.gl_main,
                                   r_texture_new_file(1, 1, "res/ring.png"));

    L.move.ring_ro.rect.pose = u_pose_new_hidden();

    L.ro = ro_batch_new(FISH_MAX, camera.gl_main,
                        r_texture_new_file(4, 2, "res/fish.png"));

    for (int i = 0; i < FISH_MAX; i++) {
        L.ro.rects[i].pose = u_pose_new_hidden();
    }


    ro_batch_update(&L.ro);
}

void fish_update(float dtime) {
    assert(fish.swarmed_size + fish.alone_size <= FISH_MAX);

    swarm_center();

    if (L.move.active < 0) {
        L.move.ring_ro.rect.pose = u_pose_new_hidden();
    } else {
        vec2 fish_pos = fish.swarmed[L.move.active].pos;
        L.move.ring_ro.rect.pose = u_pose_new(fish_pos.x, fish_pos.y, 64, 64);
        active_code();
    }


    for (int i = 0; i < fish.swarmed_size; i++) {
        if (i == L.move.active)
            continue;
        swarm_code(i, dtime);
    }

    for (int i = 0; i < fish.swarmed_size; i++) {
        update_fish(&fish.swarmed[i], true, i, dtime);
    }
    for (int i = 0; i < fish.alone_size; i++) {
        update_fish(&fish.alone[i], false, fish.swarmed_size + i, dtime);
    }
    for (int i = fish.swarmed_size + fish.alone_size; i < L.ro.num; i++) {
        L.ro.rects[i].pose = u_pose_new_hidden();
    }

    ro_batch_update(&L.ro);
}

void fish_render() {
    ro_batch_render(&L.ro);
    ro_single_render(&L.move.ring_ro);
}

vec2 fish_swarm_center() {
    return L.swarm_center;
}

void fish_catch_alone(int idx) {
    assert(fish.swarmed_size < FISH_MAX);
    assert(fish.alone_size > idx);


    emit_particles(fish.alone[idx].pos.x, fish.alone[idx].pos.y, vec3_hsv2rgb(fish.alone[idx].hsv));

    fish.swarmed[fish.swarmed_size] = fish.alone[idx];
    fish.alone_size--;
    for (int i = idx; i < fish.alone_size; i++) {
        fish.alone[i] = fish.alone[i + 1];
    }

    fish.last_catched_idx = fish.swarmed_size;
    fish.swarmed_size++;
}


void fish_eat(int idx, bool swarmed) {
    assert(idx>=0);
    Fish_s eaten;
    if(swarmed) {
        assert(idx < fish.swarmed_size);
        eaten = fish.swarmed[idx];
        fish.swarmed_size--;
        for(int i=idx; i<fish.swarmed_size; i++) {
            fish.swarmed[i] = fish.swarmed[i+1];
        }
    } else {
        assert(idx < fish.alone_size);
        eaten = fish.alone[idx];
        fish.alone_size--;
        for(int i=idx; i<fish.alone_size; i++) {
            fish.alone[i] = fish.alone[i+1];
        }
    }
    eaten.pos = vec2_set(FLT_MAX);
    fish.alone[fish.alone_size++] = eaten;
    printf("eaten %i %i\n", idx, swarmed);
}
