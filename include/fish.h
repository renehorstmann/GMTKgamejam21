#ifndef GMTKJAM21_FISH_H
#define GMTKJAM21_FISH_H

#include "mathc/types/float.h"

#define FISH_MAX 24

void fish_init();

void fish_update(float dtime);

void fish_render();

int fish_swarm_size();

vec2 fish_swarm_center();

#endif //GMTKJAM21_FISH_H
