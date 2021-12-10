#include "r/ro_single.h"
#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "rhc/log.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "mathc/utils/color.h"
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

static void emit_particles(Fish *self, float x, float y, vec3 color) {
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
        rects[i].start_time = self->particles_ref->time;
    }
    pixelparticles_add(self->particles_ref, rects, NUM_PARTICLES);
}

static void update_fish(Fish *self, Fish_s *fish, bool swarmed, int idx, float dtime) {
    fish->speed = vec2_mix(fish->speed, fish->set_speed, 1.0 * dtime);
    vec2 delta = vec2_scale(fish->speed, dtime);
    fish->pos = vec2_add_vec(fish->pos, delta);

    vec3 hsv = fish->hsv;
    if (!swarmed) {
        hsv.v1 = 0;
        hsv.v2 -= 0.25;
    }

    fish->L.animate_time = sca_mod(fish->L.animate_time + dtime, FRAMES / FPS);
    int frame = fish->L.animate_time * FPS;
    int mouth_frame = fish->L.animate_time * MOUTH_FPS;
    if (fish->L.looking_left && fish->speed.x > 10)
        fish->L.looking_left = false;
    if (!fish->L.looking_left && fish->speed.x < -10)
        fish->L.looking_left = true;

    self->L.ro.rects[idx].pose = u_pose_new(fish->pos.x, fish->pos.y, 32, 32);
    self->L.ro.rects[idx].color.rgb = vec3_hsv2rgb(hsv);
    self->L.ro.rects[idx].uv = u_pose_new(0, 0, fish->L.looking_left ? 1 : -1, 1);
    self->L.ro.rects[idx].sprite.x = frame;
    self->L.ro.rects[idx].sprite.y = (fish->L.state == FISH_STATE_EAT && mouth_frame % 2 == 0) ? 1 : 0;
}

static void pointer_callback(ePointer_s pointer, void *user_data) {
    Fish *self = user_data;

    if (pointer.id != 0 || !self->game_running)
        return;

    if (pointer.action == E_POINTER_UP) {
        if (self->L.move.active >= 0) {
            log_info("fish move end");
        }
        self->L.move.active = -1;
        return;
    }

    pointer.pos = mat4_mul_vec(self->camera_ref->matrices_main.v_p_inv, pointer.pos);

    self->L.move.dst = pointer.pos.xy;

    if (pointer.action == E_POINTER_DOWN) {
        for (int i = 0; i < self->swarmed_size; i++) {
            if (vec2_distance(pointer.pos.xy, self->swarmed[i].pos) <= POINTER_DISTANCE) {
                self->L.move.active = i;
                log_info("fish move start with %i", i);
            }
        }
    }
}

static void swarm_center(Fish *self) {
    vec2 center = {{0, 0}};
    for (int i = 0; i < self->swarmed_size; i++) {
        center = vec2_add_vec(center, self->swarmed[i].pos);
    }
    self->L.swarm_center = vec2_div(center, self->swarmed_size);
}

static void active_code(Fish *self) {
    if (!(self->L.move.active >= 0 && self->L.move.active < self->swarmed_size))
        return;
    vec2 fish_pos = self->swarmed[self->L.move.active].pos;
    vec2 diff = vec2_sub_vec(self->L.move.dst, fish_pos);
    vec2 dir = vec2_normalize(diff);
    float distance = vec2_norm(diff);
    float speed = distance * ACTIVE_SPEED_FACTOR;
    speed = sca_min(speed, ACTIVE_SPEED_MAX);
    self->swarmed[self->L.move.active].speed = vec2_scale(dir, speed);

    for (int i = 0; i < self->alone_size; i++) {
        vec2 delta = vec2_sub_vec(self->alone[i].pos, self->swarmed[self->L.move.active].pos);
        float fish_distance = vec2_norm(delta);
        if (fish_distance <= SWARM_RADIUS) {
            fish_catch_alone(self, i);
            break;
        }
    }
}

static vec2 feed_position(const Fish *self, int feed_idx, bool looking_left) {
    vec2 feed_pos = self->feed_ref->feed[feed_idx].pos;
    if (looking_left)
        feed_pos.x += 8.0;
    else
        feed_pos.x -= 8.0;
    return feed_pos;
}

static bool check_feed(Fish *self, int fish_idx, float dtime) {
    self->swarmed[fish_idx].L.state = FISH_STATE_SWIM;
    vec2 fish_pos = self->swarmed[fish_idx].pos;

    int dst = -1;
    vec2 feed_pos;
    bool near = false;
    for (int i = 0; i < self->feed_ref->feed_size; i++) {
        vec2 pos = feed_position(self, i, self->swarmed[fish_idx].L.looking_left);
        float dist = vec2_distance(pos, fish_pos);
        if (dist <= SWARM_FEED_RADIUS_NEAR) {
            dst = i;
            feed_pos = pos;
            near = true;
            break;
        }
        if (dist <= SWARM_FEED_RADIUS_FAR) {
            dst = i;
            feed_pos = pos;
        }
    }
    if (dst < 0)
        return false;

    if (near) {
        self->swarmed[fish_idx].L.state = FISH_STATE_EAT;
        self->swarmed[fish_idx].set_speed = vec2_set(0);
        feed_eat(self->feed_ref, &self->feed_ref->feed[dst], dtime);
        return true;
    }

    vec2 dir = vec2_normalize(vec2_sub_vec(feed_pos, fish_pos));
    self->swarmed[fish_idx].set_speed = vec2_scale(dir, SPEED_FEED);
    return true;
}

static void swarm_code(Fish *self, int fish_idx, float dtime) {
    assert(fish_idx < self->swarmed_size);

    if (check_feed(self, fish_idx, dtime))
        return;

    vec2 swarm_center_dir = {0};
    vec2 local_center_dir = {0};
    vec2 keep_distance_dir = {0};
    vec2 active_dir = {0};

    vec2 fish_pos = self->swarmed[fish_idx].pos;

    int local_cnt = 0;
    int near_cnt = 0;

    swarm_center_dir = vec2_sub_vec(self->L.swarm_center, fish_pos);

    vec2 local_center = {0};
    for (int i = 0; i < self->swarmed_size; i++) {
        vec2 delta = vec2_sub_vec(self->swarmed[i].pos, fish_pos);
        float fish_distance = vec2_norm(delta);
        if (fish_distance <= SWARM_RADIUS) {
            local_cnt++;
            local_center = vec2_add_vec(local_center, self->swarmed[i].pos);
            if (i == self->L.move.active) {
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
    self->swarmed[fish_idx].set_speed = vec2_scale(vec2_normalize(speed), speed_value);
}

Fish *fish_new(eInput *input, const Camera_s *cam, Sound *sound, PixelParticles *particles, Feed *feed) {
    Fish *self = rhc_calloc(sizeof *self);

    self->input_ref = input;
    self->camera_ref = cam;
    self->sound_ref = sound;
    self->particles_ref = particles;
    self->feed_ref = feed;

    self->L.move.active = -1;

    e_input_register_pointer_event(input, pointer_callback, self);

    self->L.move.ring_ro = ro_single_new(r_texture_new_file(1, 1, "res/ring.png"));

    self->L.move.ring_ro.rect.pose = u_pose_new_hidden();

    self->L.ro = ro_batch_new(FISH_MAX, r_texture_new_file(4, 2, "res/fish.png"));

    for (int i = 0; i < FISH_MAX; i++) {
        self->L.ro.rects[i].pose = u_pose_new_hidden();
    }

    ro_batch_update(&self->L.ro);
    self->game_running = true;

    return self;
}

void fish_kill(Fish **self_ptr) {
    Fish *self = *self_ptr;
    if (!self)
        return;
    e_input_unregister_pointer_event(self->input_ref, pointer_callback);
    ro_single_kill(&self->L.move.ring_ro);
    ro_batch_kill(&self->L.ro);
    rhc_free(self);
    *self_ptr = NULL;
}

void fish_update(Fish *self, float dtime) {
    assert(self->swarmed_size + self->alone_size <= FISH_MAX);

    if (self->L.move.active < 0 || self->swarmed_size <= 0) {
        self->L.move.ring_ro.rect.pose = u_pose_new_hidden();
    }

    if (self->swarmed_size > 0) {
        swarm_center(self);

        if (self->L.move.active >= 0) {
            vec2 fish_pos = self->swarmed[self->L.move.active].pos;
            self->L.move.ring_ro.rect.pose = u_pose_new(fish_pos.x, fish_pos.y, 64, 64);
            active_code(self);
        }

        for (int i = 0; i < self->swarmed_size; i++) {
            if (i == self->L.move.active)
                continue;
            swarm_code(self, i, dtime);
        }
    }

    for (int i = 0; i < self->swarmed_size; i++) {
        update_fish(self, &self->swarmed[i], true, i, dtime);
    }
    for (int i = 0; i < self->alone_size; i++) {
        update_fish(self, &self->alone[i], false, self->swarmed_size + i, dtime);
    }
    for (int i = self->swarmed_size + self->alone_size; i < self->L.ro.num; i++) {
        self->L.ro.rects[i].pose = u_pose_new_hidden();
    }

}

void fish_render(const Fish *self, const mat4 *cam_mat) {
    ro_batch_render(&self->L.ro, cam_mat, true);
    ro_single_render(&self->L.move.ring_ro, cam_mat);
}

vec2 fish_swarm_center(const Fish *self) {
    return self->L.swarm_center;
}

void fish_catch_alone(Fish *self, int idx) {
    assert(self->swarmed_size < FISH_MAX);
    assert(self->alone_size > idx);

    emit_particles(self, self->alone[idx].pos.x, self->alone[idx].pos.y, vec3_hsv2rgb(self->alone[idx].hsv));

    self->swarmed[self->swarmed_size] = self->alone[idx];
    self->alone_size--;
    for (int i = idx; i < self->alone_size; i++) {
        self->alone[i] = self->alone[i + 1];
    }

    self->last_catched_idx = self->swarmed_size;
    self->swarmed_size++;

    sound_play_activate(self->sound_ref);
}

void fish_eat(Fish *self, int idx, bool swarmed) {
    assert(idx >= 0);
    Fish_s eaten;
    if (swarmed) {
        assert(idx < self->swarmed_size);
        if (idx == self->L.move.active)
            self->L.move.active = -1;

        eaten = self->swarmed[idx];
        self->swarmed_size--;
        for (int i = idx; i < self->swarmed_size; i++) {
            self->swarmed[i] = self->swarmed[i + 1];
        }
    } else {
        assert(idx < self->alone_size);
        eaten = self->alone[idx];
        self->alone_size--;
        for (int i = idx; i < self->alone_size; i++) {
            self->alone[i] = self->alone[i + 1];
        }
    }
    eaten.pos = vec2_set(FLT_MAX);
    self->alone[self->alone_size++] = eaten;
}
