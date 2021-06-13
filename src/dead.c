#include "r/ro_single.h"
#include "r/ro_text.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "hudcamera.h"
#include "fish.h"
#include "dead.h"


#define RESCUE_TIME 5.0
#define TEXT_SIZE 3.0

static struct {
   RoSingle ro;
   RoText info;
   bool show;
   float time;
} L;

void dead_init() {
    L.ro = ro_single_new(hudcamera.gl, r_texture_new_white_pixel());
    L.ro.rect.pose = u_pose_new(0, 0, 2048, 2048);
    L.ro.rect.color = (vec4) {{0.5, 0.1, 0.1, 0.0}};

    L.info = ro_text_new_font55(64, hudcamera.gl);
    ro_text_set_color(&L.info, R_COLOR_BLACK);
}

void dead_kill() {
    ro_single_kill(&L.ro);
    ro_text_kill(&L.info);
    memset(&L, 0, sizeof(L));
}

void dead_update(float dtime) {
    if(fish.swarmed_size >= 3) {
        L.show = false;
        return;
    }

    if(!L.show) {
        L.show = true;
        L.time = RESCUE_TIME;
    }

    L.time -= dtime;

    char buf[65];
    if(L.time > 0) {
        sprintf(buf, "FISH ALERT!\n\n%7.2f", L.time);
    } else {
        L.time = 0;
        sprintf(buf, "GAME OVER");
    }

    vec2 size = ro_text_set_text(&L.info, buf);
    L.info.pose = u_pose_new(-size.x/2*TEXT_SIZE, +size.y/2*TEXT_SIZE, TEXT_SIZE, TEXT_SIZE);

    L.ro.rect.color.a = sca_mix(0.75, 0.1, L.time/RESCUE_TIME);
}

void dead_render() {
    if(!L.show)
        return;
    ro_single_render(&L.ro);
    ro_text_render(&L.info);
}
