#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/sca/int.h"
#include "mathc/utils/random.h"
#include "rhc/alloc.h"
#include "pixelparticles.h"


#define MAX_PARTCLES 512

#define DIRT_LIFETIME 1.0
#define DIRT_SIZE 2.0
#define DIRT_ALPHA 1.0


//
// public
//

PixelParticles *pixelparticles_new() {
    PixelParticles *self = rhc_calloc(sizeof *self);

    self->L.ro = ro_particle_new(MAX_PARTCLES, r_texture_new_white_pixel());

    for (int i = 0; i < self->L.ro.num; i++) {
        self->L.ro.rects[i].pose = u_pose_new_hidden();
        self->L.ro.rects[i].color = vec4_set(0);
    }
    ro_particle_update(&self->L.ro);

    return self;
}

void pixelparticles_kill(PixelParticles **self_ptr) {
    PixelParticles *self = *self_ptr;
    if (!self)
        return;
    ro_particle_kill(&self->L.ro);
    rhc_free(self);
    *self_ptr = NULL;
}

void pixelparticles_update(PixelParticles *self, float dtime) {
    self->time += dtime;
}

void pixelparticles_render(const PixelParticles *self, const mat4 *cam_mat) {
    ro_particle_render(&self->L.ro, self->time, cam_mat, false);
}

void pixelparticles_add(PixelParticles *self, const rParticleRect_s *particles, int n) {
    int start_idx = self->L.next;
    n = isca_min(n, self->L.ro.num);

    if (self->L.next + n <= self->L.ro.num) {
        memcpy(&self->L.ro.rects[self->L.next], particles, n * sizeof(rParticleRect_s));
        self->L.next += n;
    } else {
        int n_b = self->L.next + n - self->L.ro.num;
        int n_a = n - n_b;
        memcpy(&self->L.ro.rects[self->L.next], particles, n_a * sizeof(rParticleRect_s));
        memcpy(self->L.ro.rects, particles + n_a, n_b * sizeof(rParticleRect_s));
        self->L.next = n_b;
    }
    if (self->L.next >= self->L.ro.num)
        self->L.next = 0;

//    ro_particle_update_sub(&self->L.ro, start_idx, n);
    ro_particle_update(&self->L.ro);
}
