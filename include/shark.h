#ifndef GMTKJAM21_SHARK_H
#define GMTKJAM21_SHARK_H

#include "mathc/types/float.h"

#define SHARK_MAX 8

typedef struct {
    vec2 pos;
    vec2 speed;
    vec4 color;
} Shark_s;

struct SharkGlobals_s {
    Shark_s shark[SHARK_MAX];
    int shark_size;
};
extern struct SharkGlobals_s shark;

void shark_init();

void shark_update(float dtime);

void shark_render();

#endif //GMTKJAM21_SHARK_H
