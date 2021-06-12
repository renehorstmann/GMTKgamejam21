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

#define SWARM_RADIUS 36.0
#define SWARM_NEAR 24.0

typedef struct {
    vec2 pos;
    vec2 speed;
    bool swarmed;
} Fish_s;

static struct {
    RoBatch ro;
    Fish_s fish[FISH_MAX];

    struct {
        RoSingle ring_ro;
        int active;
        vec2 dst;
    } move;
} L;


static void animate(float dtime) {
    static float time = 0;
    time = sca_mod(time + dtime, FRAMES / FPS);
    int frame = time * FPS;
    for(int i=0; i<L.ro.num; i++) {
        L.ro.rects[i].sprite.x = frame;
    }
}

static void pointer_callback(ePointer_s pointer, void *user_data) {
    if(pointer.id != 0)
        return;

    if(pointer.action == E_POINTER_UP) {
        if(L.move.active>=0)
            log_info("fish move end");
        L.move.active = -1;
        return;
    }

    pointer.pos = mat4_mul_vec(camera.matrices_main.v_p_inv, pointer.pos);

    L.move.dst = pointer.pos.xy;

    if(pointer.action == E_POINTER_DOWN) {
        for(int i=0; i<FISH_MAX; i++) {
            if(!L.fish[i].swarmed)
                continue;
            if(vec2_distance(pointer.pos.xy, L.fish[i].pos) <= POINTER_DISTANCE) {
                L.move.active = i;
                log_info("fish move start with %i", i);
            }
        }
    }
}

static void swarm_code(int fish_idx) {
    vec2 center = {{0, 0}};
    vec2 move_dir = {{0, 0}};
    int center_cnt = 0;
    int near_cnt = 0;
    for(int i=0; i<FISH_MAX; i++) {
        if(i == fish_idx)
            continue;
        vec2 delta = vec2_sub_vec(L.fish[i].pos, L.fish[fish_idx].pos);
        float fish_distance = vec2_norm(delta);
        if(fish_distance <= SWARM_RADIUS) {
            center_cnt++;
            vec2_add_vec(center, L.fish[i].pos);
            L.fish[i].swarmed = true;
        }
        if(fish_distance <= SWARM_NEAR) {
            near_cnt++;
            vec2 dir = vec2_normalize(delta);
            dir = vec2_scale(dir, SWARM_NEAR - fish_distance);
            move_dir = vec2_sub_vec(move_dir, dir);
        }
    }
    if(center_cnt == 0) {
        L.fish[fish_idx].swarmed = false;
        return;
    }
    if(near_cnt == 0) {
        move_dir = vec2_sub_vec(center, L.fish[fish_idx].pos);
    }
    L.fish[fish_idx].speed = vec2_scale(vec2_normalize(move_dir), 15.0);
}

void fish_init() {
    L.move.active = -1;

    e_input_register_pointer_event(pointer_callback, NULL);


    L.move.ring_ro = ro_single_new(camera.gl_main,
                                   r_texture_new_file(1, 1, "res/ring.png"));

    L.move.ring_ro.rect.pose = u_pose_new_hidden();

    L.ro = ro_batch_new(FISH_MAX, camera.gl_main,
                        r_texture_new_file(4, 2, "res/fish.png"));

    for(int i=0; i<L.ro.num; i++) {
        L.fish[i].pos = vec2_random_noise(0, 90);
        L.fish[i].swarmed = true;
        L.ro.rects[i].pose = u_pose_new(L.fish[i].pos.x, L.fish[i].pos.y, 32, 32);
        vec3 hsv = {{
                            sca_random_range(0, 360),
                            0.3, 1
        }};
        L.ro.rects[i].color.rgb = vec3_hsv2rgb(hsv);
    }

    ro_batch_update(&L.ro);
}

void fish_update(float dtime) {

    if(L.move.active < 0) {
        L.move.ring_ro.rect.pose = u_pose_new_hidden();
    } else {
        vec2 fish_pos = L.fish[L.move.active].pos;
        L.move.ring_ro.rect.pose = u_pose_new(fish_pos.x, fish_pos.y, 64, 64);
        vec2 diff = vec2_sub_vec(L.move.dst, fish_pos);
        vec2 dir = vec2_normalize(diff);
        float distance = vec2_norm(diff);
        float speed = distance * SPEED_FACTOR;
        speed = sca_min(speed, MAX_SPEED);
        L.fish[L.move.active].speed = vec2_scale(dir, speed);
    }

    for(int i=0; i<FISH_MAX; i++) {
        if(!L.fish[i].swarmed || i==L.move.active)
            continue;
        swarm_code(i);
    }

    animate(dtime);

    for(int i=0; i<FISH_MAX; i++) {
        vec2 delta = vec2_scale(L.fish[i].speed, dtime);
        L.fish[i].pos = vec2_add_vec(L.fish[i].pos, delta);
        u_pose_set_xy(&L.ro.rects[i].pose, L.fish[i].pos.x, L.fish[i].pos.y);
    }

    ro_batch_update(&L.ro);
}

void fish_render() {
    ro_batch_render(&L.ro);
    ro_single_render(&L.move.ring_ro);
}

vec2 fish_swarm_center() {
    vec2 center = {{0, 0}};
    int cnt = 0;
    for(int i=0; i<FISH_MAX; i++) {
        if(L.fish[i].swarmed) {
            center = vec2_add_vec(center, L.fish[i].pos);
            cnt++;
        }
    }
    if(cnt == 0) {
        log_warn("fish_swarm_center failed, no fish in the swarm");
        return center;
    }
    return vec2_div(center, cnt);
}
