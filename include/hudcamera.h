#ifndef GMTKJAM21_HUDCAMERA_H
#define GMTKJAM21_HUDCAMERA_H

/*
 * PixelPerfect hudcamera for hud elements
 */

#include <stdbool.h>
#include "mathc/types/float.h"
#include "mathc/types/int.h"


#define HUDCAMERA_SIZE 240 // *4=720; *6=1080; *8=1440

struct HudCameraMatrices_s {
    mat4 p;
    mat4 p_inv;
};

struct HudCameraGlobals_s {
    struct HudCameraMatrices_s matrices;
    const float *gl;
};
extern struct HudCameraGlobals_s hudcamera;


void hudcamera_init();

void hudcamera_update(ivec2 window_size);

float hudcamera_real_pixel_per_pixel();

float hudcamera_left();

float hudcamera_right();

float hudcamera_bottom();

float hudcamera_top();

static float hudcamera_width() {
    return -hudcamera_left() + hudcamera_right();
}

static float hudcamera_height() {
    return -hudcamera_bottom() + hudcamera_top();
}

static bool hudcamera_is_portrait_mode() {
    return hudcamera_height() > hudcamera_width();
}


#endif //GMTKJAM21_HUDCAMERA_H
