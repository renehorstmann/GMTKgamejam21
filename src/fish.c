#include "r/ro_batch.h"
#include "u/pose.h"
#include "mathc/sca/float.h"
#include "mathc/utils/random.h"
#include "mathc/utils/color.h"
#include "camera.h"
#include "fish.h"

#define FRAMES 4
#define FPS 6.0

static struct {
    RoBatch ro;
} L;


static void animate(float dtime) {
    static float time = 0;
    time = sca_mod(time + dtime, FRAMES / FPS);
    int frame = time * FPS;
    for(int i=0; i<L.ro.num; i++) {
        L.ro.rects[i].sprite.x = frame;
    }
    ro_batch_update(&L.ro);
}

void fish_init() {
    L.ro = ro_batch_new(FISH_MAX, camera.gl_main,
                        r_texture_new_file(4, 2, "res/fish.png"));

    for(int i=0; i<L.ro.num; i++) {
        L.ro.rects[i].pose = u_pose_new(
                sca_random_noise(0, 256),
                sca_random_noise(0, 256),
                32, 32
                );
        vec3 hsv = {{
                            sca_random_range(0, 360),
                            0.3, 1
        }};
        L.ro.rects[i].color.rgb = vec3_hsv2rgb(hsv);
    }

    ro_batch_update(&L.ro);
}

void fish_update(float dtime) {
    animate(dtime);
}

void fish_render() {
    ro_batch_render(&L.ro);
}
