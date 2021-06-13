#include "r/ro_particlerefract.h"
#include "u/pose.h"
#include "u/image.h"
#include "mathc/utils/random.h"
#include "camera.h"
#include "hudcamera.h"
#include "bubbles.h"

#define MAX_TIME 48.0

static struct {
   RoParticleRefract ro;
   float time;
} L;


void bubbles_init() {
    uImage img = u_image_new_file(2, "res/bubble.png");
    assume(u_image_valid(img), "bubble not found?");

    L.ro = ro_particlerefract_new(BUBBLES_SIZE, hudcamera.gl, camera.gl_scale,
                                  r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, 0)),
                                  r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, 1))
                                  );

    float radius = hudcamera_height() > hudcamera_width() ? hudcamera_height() : hudcamera_width();
    for(int i=0; i<L.ro.num; i++) {
        L.ro.rects[i].speed.y = 10;
        L.ro.rects[i].pose = u_pose_new(
                sca_random_noise(0, radius),
                -radius,
                img.cols, img.rows
                );
        L.ro.rects[i].start_time = sca_random_range(-MAX_TIME, 0);
    }
    ro_particlerefract_update(&L.ro);

    u_image_kill(&img);
}

void bubbles_update(float dtime) {
    L.time += dtime;
    for(int i=0; i<L.ro.num; i++) {
        if(L.ro.rects[i].start_time<L.time-MAX_TIME)
            L.ro.rects[i].start_time = L.time;
    }
    ro_particlerefract_update(&L.ro);
}

void bubbles_render() {
    ro_particlerefract_render(&L.ro, L.time);
}
