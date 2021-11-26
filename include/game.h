#ifndef GMTKJAM21_GAME_H
#define GMTKJAM21_GAME_H

#include "e/input.h"
#include "r/render.h"

void game_init(eInput *input, rRender *render);

void game_update(float dtime);

void game_render();

void game_reset();

#endif //GMTKJAM21_GAME_H
