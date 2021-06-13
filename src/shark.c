#include "r/ro_batch.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "pixelparticles.h"
#include "fish.h"
#include "camera.h"
#include "shark.h"

struct SharkGlobals_s shark;

static struct {
    RoBatch ro;
} L;



void shark_init() {
    L.ro = ro_batch_new(SHARK_MAX, camera.gl_main,
                        r_texture_new_file(4, 2, "res/shark.png"));

    for(int i=0; i<SHARK_MAX; i++) {
        L.ro.rects[i].pose = u_pose_new_hidden();
    }

    ro_batch_update(&L.ro);
}

void shark_update(float dtime) {
    for(int i=0; i<shark.shark_size; i++) {
        vec2 delta = vec2_scale(shark.shark[i].speed, dtime);
        shark.shark[i].pos = vec2_add_vec(shark.shark[i].pos, delta);
        L.ro.rects[i].pose = u_pose_new(shark.shark[i].pos.x, shark.shark[i].pos.x, 64, 32);
        L.ro.rects[i].color = shark.shark[i].color;
    }
    for(int i=shark.shark_size; i<SHARK_MAX; i++) {
        L.ro.rects[i].pose = u_pose_new_hidden();
    }
}

void shark_render() {
    ro_batch_render(&L.ro);
}

