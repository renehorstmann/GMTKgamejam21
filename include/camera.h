#ifndef GMTKJAM21_CAMERA_H
#define GMTKJAM21_CAMERA_H

/*
 * PixelPerfect canvas camera with view matrix.
 * So a unit has an integral number of pixels, until its <1 (units_per_pixels aka scale)
 * To control the camera position and size
 */

#include <stdbool.h>
#include "mathc/types/float.h"
#include "mathc/types/int.h"


#define CAMERA_SIZE 240 // *3=720; *4.5=1080; *6=1440
#define CAMERA_BACKGROUNDS 6

struct CameraMatrices_s {
    mat4 v;
    mat4 v_inv;
    mat4 vp;
    mat4 v_p_inv;   // v @ p_inv
};

typedef struct {
    mat4 matrices_p;
    mat4 matrices_p_inv;

    struct CameraMatrices_s matrices_background[CAMERA_BACKGROUNDS];
    struct CameraMatrices_s matrices_main;


    struct {
        float scale; // units per pixel
        float left, right, bottom, top;
    } RO; // read only

} Camera_s;


Camera_s camera_new();

void camera_update(Camera_s *self, ivec2 window_size);


static float camera_width(const Camera_s *self) {
    return -self->RO.left + self->RO.right;
}

static float camera_height(const Camera_s *self) {
    return -self->RO.bottom + self->RO.top;
}

void camera_set_pos(Camera_s *self, float x, float y);

void camera_set_size(Camera_s *self, float size);

void camera_set_angle(Camera_s *self, float alpha);

#endif //GMTKJAM21_CAMERA_H
