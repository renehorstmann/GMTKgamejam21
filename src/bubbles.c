#include "r/ro_particlerefract.h"
#include "r/texture.h"
#include "u/pose.h"
#include "u/image.h"
#include "mathc/utils/random.h"
#include "bubbles.h"

#define MAX_TIME 48.0


Bubbles *bubbles_new(const Camera_s *cam) {
    Bubbles *self = rhc_calloc(sizeof *self);
    
    uImage img = u_image_new_file(2, "res/bubble.png");
    assume(u_image_valid(img), "bubble not found?");

    self->L.ro = ro_particlerefract_new(BUBBLES_SIZE,
                                  r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, 0)),
                                  r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, 1))
                                  );

    float radius = camera_height(cam) > camera_width(cam) ? camera_height(cam) : camera_width(cam);
    for(int i=0; i<self->L.ro.num; i++) {
        self->L.ro.rects[i].speed.y = 10;
        self->L.ro.rects[i].pose = u_pose_new(
                sca_random_noise(0, radius),
                -radius,
                img.cols, img.rows
                );
        self->L.ro.rects[i].start_time = sca_random_range(-MAX_TIME, 0);
    }
    ro_particlerefract_update(&self->L.ro);

    u_image_kill(&img);
    
    return self;
}

void bubbles_kill(Bubbles **self_ptr) {
    Bubbles *self = *self_ptr;
    if(!self)
        return;
    ro_particlerefract_kill(&self->L.ro);
    rhc_free(self);
    *self_ptr = NULL;
}

void bubbles_update(Bubbles *self, float dtime) {
    self->L.time += dtime;
    for(int i=0; i<self->L.ro.num; i++) {
        if(self->L.ro.rects[i].start_time<self->L.time-MAX_TIME)
            self->L.ro.rects[i].start_time = self->L.time;
    }
    ro_particlerefract_update(&self->L.ro);
}

void bubbles_render(const Bubbles *self, const mat4 *cam_mat, float cam_scale) {
    ro_particlerefract_render(&self->L.ro, self->L.time, cam_mat, cam_scale, NULL, NULL);
}
