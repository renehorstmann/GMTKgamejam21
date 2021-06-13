#include "r/ro_batch.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "pixelparticles.h"
#include "fish.h"
#include "camera.h"
#include "shark.h"


#define FRAMES 4
#define FPS 6.0
#define MOUTH_FPS 2.0

struct SharkGlobals_s shark;

static struct {
    RoBatch ro;
} L;


static void update_shark(int idx, float dtime) {
    Shark_s *self = &shark.shark[idx];

    vec2 delta = vec2_scale(self->speed, dtime);
    self->pos = vec2_add_vec(self->pos, delta);

    self->L.animate_time = sca_mod(self->L.animate_time + dtime, FRAMES / FPS);
    int frame = self->L.animate_time * FPS;
    int mouth_frame = self->L.animate_time * MOUTH_FPS;

    L.ro.rects[idx].pose = u_pose_new(self->pos.x, self->pos.x, 64, 32);
    L.ro.rects[idx].uv = u_pose_new(0, 0, self->speed.x>0 ? 1 : -1, 1);
    L.ro.rects[idx].color = self->color;

    L.ro.rects[idx].sprite.x = frame;
    L.ro.rects[idx].sprite.y = mouth_frame%2==0 ? 1 : 0;
}


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
        update_shark(i, dtime);
    }
    for(int i=shark.shark_size; i<SHARK_MAX; i++) {
        L.ro.rects[i].pose = u_pose_new_hidden();
    }
    ro_batch_update(&L.ro);
}

void shark_render() {
    ro_batch_render(&L.ro);
}

