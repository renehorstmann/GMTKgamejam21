#include "u/pose.h"
#include "e/window.h"
#include "rhc/alloc.h"
#include "mathc/float.h"
#include "mathc/sca/int.h"
#include "mathc/utils/camera.h"
#include "camera.h"

#define BACKGROUND_SPEED_FACTOR 0.2



//
// private
//

static void camera_matrices_init(struct CameraMatrices_s *self) {
    self->v = mat4_eye();
    self->v_inv = mat4_eye();
    self->vp = mat4_eye();
    self->v_p_inv = mat4_eye();
}

static void camera_matrices_update(struct CameraMatrices_s *self, const Camera_s *cam) {
    self->v_inv = mat4_inv(self->v);
    self->vp = mat4_mul_mat(cam->matrices_p, self->v_inv);
    self->v_p_inv = mat4_mul_mat(self->v, cam->matrices_p_inv);
}


//
// public
//

Camera_s camera_new() {
    assert(CAMERA_SIZE % 2 == 0 && "CAMERA_SIZE must be even");

    Camera_s self = {0};

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera_matrices_init(&self.matrices_background[i]);
    camera_matrices_init(&self.matrices_main);

    self.matrices_p = mat4_eye();
    self.matrices_p_inv = mat4_eye();

    return self;
}

void camera_update(Camera_s *self, ivec2 window_size) {
    int wnd_width = window_size.x;
    int wnd_height = window_size.y;

    float smaller_size = wnd_width < wnd_height ? wnd_width : wnd_height;

    self->RO.scale = smaller_size / CAMERA_SIZE;

    // pixel perfect:
    if(self->RO.scale > 1)
        self->RO.scale = sca_floor(self->RO.scale);


    // test
    //L.scale = 7.5;

    float cam_width = (float) wnd_width / self->RO.scale;
    float cam_height = (float) wnd_height / self->RO.scale;

    float cam_width_2 = cam_width / 2;
    float cam_height_2 = cam_height / 2;

    // begin: (top, left) with a full pixel
    // end: (bottom, right) with a maybe splitted pixel
    float cam_left = -floorf(cam_width_2);
    float cam_top = floorf(cam_height_2);
    float cam_right = cam_width_2 + (cam_width_2 - floorf(cam_width_2));
    float cam_bottom = -cam_height_2 - (cam_height_2 - floorf(cam_height_2));

    self->matrices_p = mat4_camera_ortho(
            cam_left, cam_right, cam_bottom, cam_top,
            -1, 1);
    self->matrices_p_inv = mat4_inv(self->matrices_p);

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera_matrices_update(&self->matrices_background[i], self);
    camera_matrices_update(&self->matrices_main, self);

    self->RO.left = cam_left;
    self->RO.right = cam_right;
    self->RO.top = cam_top;
    self->RO.bottom = cam_bottom;

}

void camera_set_pos(Camera_s *self, float x, float y) {
    // bottom left 'c'orner
    float cx = x + self->RO.left;
    float cy = y + self->RO.bottom;

    x = floorf(x * self->RO.scale) / self->RO.scale;
    y = floorf(y * self->RO.scale) / self->RO.scale;

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++) {
        float t = (float) i / CAMERA_BACKGROUNDS;
        float scale = sca_mix(BACKGROUND_SPEED_FACTOR, 1, t);
        float bg_x = scale * cx - self->RO.left;
        float bg_y = scale * cy - self->RO.bottom;
        u_pose_set_xy(&self->matrices_background[i].v, bg_x, bg_y);
    }

    u_pose_set_xy(&self->matrices_main.v, x, y);
}

void camera_set_size(Camera_s *self, float size) {
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        u_pose_set_size(&self->matrices_background[i].v, size, size);

    u_pose_set_size(&self->matrices_main.v, size, size);
}

void camera_set_angle(Camera_s *self, float alpha) {
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        u_pose_set_angle(&self->matrices_background[i].v, alpha);

    u_pose_set_angle(&self->matrices_main.v, alpha);
}
