#ifndef GMTKJAM21_BUBBLES_H
#define GMTKJAM21_BUBBLES_H

#define BUBBLES_SIZE 128

#include "r/ro_particlerefract.h"
#include "camera.h"

typedef struct {
    struct {
        RoParticleRefract ro;
        float time;
    } L;
} Bubbles;

Bubbles *bubbles_new(const Camera_s *cam);

void bubbles_kill(Bubbles **self_ptr);

void bubbles_update(Bubbles *self, float dtime);

void bubbles_render(const Bubbles *self, const mat4 *cam_mat, float cam_scale);

#endif //GMTKJAM21_BUBBLES_H
