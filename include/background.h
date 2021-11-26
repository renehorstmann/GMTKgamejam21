#ifndef GMTKJAM21_BACKGROUND_H
#define GMTKJAM21_BACKGROUND_H

#include "r/render.h"
#include <stdbool.h>

void background_init(rRender *render, float level_width, float level_height,
        bool repeat_h, bool repeat_v, 
        const char *file);

void background_kill();

void background_update(float dtime);

void background_render();

#endif //GMTKJAM21_BACKGROUND_H
