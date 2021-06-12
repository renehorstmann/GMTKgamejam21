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

static struct {
    RoBatch ro;
    bool swarmed[FISH_MAX];

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
    ro_batch_update(&L.ro);
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
            if(!L.swarmed[i])
                continue;
            if(vec2_distance(pointer.pos.xy, u_pose_get_xy(L.ro.rects[i].pose)) <= POINTER_DISTANCE) {
                L.move.active = i;
                log_info("fish move start with %i", i);
            }
        }
    }
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
        L.swarmed[i] = true;
        L.ro.rects[i].pose = u_pose_new(
                sca_random_noise(0, 90),
                sca_random_noise(0, 90),
                32, 32
                );
        vec3 hsv = {{
                            sca_random_range(0, 360),
                            0.3, 1
        }};
        L.ro.rects[i].color.rgb = vec3_hsv2rgb(hsv);
    }

    ro_batch_update(&L.ro);
}

void fish_update(float dtime) {
    animate(dtime);

    if(L.move.active < 0) {
        L.move.ring_ro.rect.pose = u_pose_new_hidden();
    } else {
        vec2 fish_pos = u_pose_get_xy(L.ro.rects[L.move.active].pose);
        L.move.ring_ro.rect.pose = u_pose_new(fish_pos.x, fish_pos.y, 64, 64);
        vec2 diff = vec2_sub_vec(L.move.dst, fish_pos);
        vec2 dir = vec2_normalize(diff);
        float distance = vec2_norm(diff);
        vec2 delta = vec2_scale(dir, distance * SPEED_FACTOR * dtime);
        fish_pos = vec2_add_vec(fish_pos, delta);
        u_pose_set_xy(&L.ro.rects[L.move.active].pose, fish_pos.x, fish_pos.y);
    }
}

void fish_render() {
    ro_batch_render(&L.ro);
    ro_single_render(&L.move.ring_ro);
}
