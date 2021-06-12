#include "e/input.h"
#include "r/ro_single.h"
#include "r/ro_batch.h"
#include "u/pose.h"
#include "rhc/log.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "mathc/utils/color.h"
#include "camera.h"
#include "fish.h"

#define FRAMES 4
#define FPS 6.0
#define POINTER_DISTANCE 16.0
#define SPEED_FACTOR 1.5
#define MAX_SPEED 100.0

#define SWARM_RADIUS 48.0
#define SWARM_NEAR 36.0

#define SWIM_AWAY_SPEED 5.0
#define NEAR_SPEED_FACTOR 0.5
#define CENTER_SPEED_FACTOR 1.0

typedef struct {
    vec2 pos;
    vec2 speed;
    vec3 hsv;
    bool swarmed;
} Fish_s;

static struct {
    RoBatch ro;
    Fish_s fish[FISH_MAX];

    int swarm_size;
    vec2 swarm_center;

    RoSingle center_ro;

    struct {
        RoSingle ring_ro;
        int active;
        vec2 dst;
    } move;
} L;

static vec2 random_euler_pos(float min, float max) {
    float angle = sca_random_range(0, 2*M_PI);
    float distance = sca_random_range(min, max);
    return vec2_scale((vec2) {{sca_cos(angle), sca_sin(angle)}}, distance);
}

static void animate(float dtime) {
    static float time = 0;
    time = sca_mod(time + dtime, FRAMES / FPS);
    int frame = time * FPS;
    for (int i = 0; i < FISH_MAX; i++) {
//        if(L.fish[i].speed.x>10) {
//            L.ro.rects[i].uv = u_pose_new(0, 0, -1, 1);
//        } else if (L.fish[i].speed.x<-10){
//            L.ro.rects[i].uv = u_pose_new(0, 0, 1, 1);
//        }
        L.ro.rects[i].sprite.x = frame;
    }
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
        for (int i = 0; i < FISH_MAX; i++) {
            if (!L.fish[i].swarmed)
                continue;
            if (vec2_distance(pointer.pos.xy, L.fish[i].pos) <= POINTER_DISTANCE) {
                L.move.active = i;
                log_info("fish move start with %i", i);
            }
        }
    }
}

static void swarm_center_size() {
    vec2 center = {{0, 0}};
    int cnt = 0;
    for (int i = 0; i < FISH_MAX; i++) {
        if (L.fish[i].swarmed) {
            center = vec2_add_vec(center, L.fish[i].pos);
            cnt++;
        }
    }
    L.swarm_center = vec2_div(center, cnt);
    L.swarm_size = cnt;
}

static void active_code() {
    assert(L.move.active>=0);
    vec2 fish_pos = L.fish[L.move.active].pos;
    vec2 diff = vec2_sub_vec(L.move.dst, fish_pos);
    vec2 dir = vec2_normalize(diff);
    float distance = vec2_norm(diff);
    float speed = distance * SPEED_FACTOR;
    speed = sca_min(speed, MAX_SPEED);
    L.fish[L.move.active].speed = vec2_scale(dir, speed);

    for(int i=0; i<FISH_MAX; i++) {
        if(L.fish[i].swarmed || i==L.move.active)
            continue;
        vec2 delta = vec2_sub_vec(L.fish[i].pos, L.fish[L.move.active].pos);
        float fish_distance = vec2_norm(delta);
        if (fish_distance <= SWARM_RADIUS) {
            L.fish[i].swarmed = true;
        }
    }
}

static void swarm_code(int fish_idx) {
    int swarm_radius_cnt = 0;
    int near_cnt = 0;
    vec2 speed;

    vec2 center_dir = vec2_sub_vec(L.swarm_center, L.fish[fish_idx].pos);
    speed = vec2_scale(center_dir, CENTER_SPEED_FACTOR);

    for (int i = 0; i < FISH_MAX; i++) {
        vec2 delta = vec2_sub_vec(L.fish[i].pos, L.fish[fish_idx].pos);
        float fish_distance = vec2_norm(delta);
        if (fish_distance <= SWARM_RADIUS) {
            swarm_radius_cnt++;
            L.fish[i].swarmed = true;
        }
        if (fish_distance <= SWARM_NEAR) {
            near_cnt++;
            vec2 dir = vec2_normalize(delta);
            dir = vec2_scale(dir, (SWARM_NEAR - fish_distance) * NEAR_SPEED_FACTOR);
            speed = vec2_sub_vec(speed, dir);
        }
    }
    if (swarm_radius_cnt <= 1) {
        log_trace("fish unswarmed: %i", fish_idx);
        L.fish[fish_idx].swarmed = false;
        // move the lost fish slowly away from the swam center
        vec2 swarm_center = fish_swarm_center();
        vec2 dir = vec2_normalize(vec2_sub_vec(L.fish[fish_idx].pos, swarm_center));
        L.fish[fish_idx].speed = vec2_scale(dir, SWIM_AWAY_SPEED);
        return;
    }

    L.fish[fish_idx].speed = speed;
}

void fish_init() {
    L.move.active = -1;

    e_input_register_pointer_event(pointer_callback, NULL);


    L.move.ring_ro = ro_single_new(camera.gl_main,
                                   r_texture_new_file(1, 1, "res/ring.png"));

    L.move.ring_ro.rect.pose = u_pose_new_hidden();

    L.ro = ro_batch_new(FISH_MAX, camera.gl_main,
                        r_texture_new_file(4, 2, "res/fish.png"));

    for (int i = 0; i < L.ro.num; i++) {
        L.fish[i].hsv = (vec3) {{
                                        sca_random_range(0, 360),
                                        0.3,
                                        sca_random_range(0.5, 1.0)
                                }};
        L.fish[i].swarmed = false;
        L.ro.rects[i].pose = u_pose_new(0, 0, 32, 32);
    }

    assert(FISH_MAX>=3);
    for(int i=0; i<2; i++) {
        L.fish[i].swarmed = true;
        L.fish[i].pos = random_euler_pos(SWARM_NEAR/2, SWARM_RADIUS/2);
    }
    for(int i=2; i<FISH_MAX; i++) {
        L.fish[i].pos = random_euler_pos(96, 256);
    }

    ro_batch_update(&L.ro);

    // debug
    L.center_ro = ro_single_new(camera.gl_main, r_texture_new_white_pixel());
}

void fish_update(float dtime) {

    swarm_center_size();

    if (L.move.active < 0) {
        L.move.ring_ro.rect.pose = u_pose_new_hidden();
    } else {
        vec2 fish_pos = L.fish[L.move.active].pos;
        L.move.ring_ro.rect.pose = u_pose_new(fish_pos.x, fish_pos.y, 64, 64);
        active_code();
    }


    for (int i = 0; i < FISH_MAX; i++) {
        if (!L.fish[i].swarmed || i == L.move.active)
            continue;
        swarm_code(i);
    }

    animate(dtime);

    for (int i = 0; i < FISH_MAX; i++) {
        vec2 delta = vec2_scale(L.fish[i].speed, dtime);
        L.fish[i].pos = vec2_add_vec(L.fish[i].pos, delta);
        u_pose_set_xy(&L.ro.rects[i].pose, L.fish[i].pos.x, L.fish[i].pos.y);

        vec3 hsv = L.fish[i].hsv;
        if(!L.fish[i].swarmed) {
            hsv.v1 = 0;
        }
        L.ro.rects[i].color.rgb = vec3_hsv2rgb(hsv);
    }

    ro_batch_update(&L.ro);

    L.center_ro.rect.pose = u_pose_new(L.swarm_center.x, L.swarm_center.y, 3, 3);
}

void fish_render() {
    ro_batch_render(&L.ro);
    ro_single_render(&L.move.ring_ro);
    ro_single_render(&L.center_ro);
}

int fish_swarm_size() {
    return L.swarm_size;
}

vec2 fish_swarm_center() {
    return L.swarm_center;
}
