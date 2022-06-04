#ifndef GMTKJAM21_SHARK_H
#define GMTKJAM21_SHARK_H

#include "pixelparticles.h"
#include "fish.h"
#include "sound.h"

#define SHARK_MAX 32

// single shark
typedef struct {
    vec2 pos;
    vec2 speed;
    vec4 color;

    struct {
        float animate_time;
        float mouth_time;
    } L;
} Shark_s;

// module
typedef struct {
    Sound *sound_ref;
    PixelParticles *particles_ref;
    Fish *fish_ref;

    Shark_s shark[SHARK_MAX];
    int shark_size;

    struct {
        RoBatch ro;
    } L;
} Shark;


Shark *shark_new(Sound *sound, PixelParticles *particles, Fish *fish);

void shark_kill(Shark **self_ptr);

void shark_update(Shark *self, float dtime);

void shark_render(const Shark *self, const mat4 *cam_mat);

#endif //GMTKJAM21_SHARK_H
