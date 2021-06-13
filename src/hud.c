#include <stdio.h>
#include "r/ro_single.h"
#include "r/ro_batch.h"
#include "r/ro_text.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/color.h"
#include "rhc/error.h"
#include "camera.h"
#include "hudcamera.h"
#include "fish.h"
#include "feed.h"
#include "hud.h"


#define FISH_TIME 0.2

//
// private
//

static struct {
    RoSingle fish_icon;
    RoText fish_cnt;
    RoText min_info;
    float fish_time;
    int fish_collected;
    
    RoSingle feed_icon;
    RoText feed_cnt;
    int feed_collected;
} L;


static void update_fish(float dtime) {
    int collected = fish.swarmed_size;
    vec3 color = vec3_hsv2rgb(fish.swarmed[fish.last_catched_idx].hsv);

    if (collected > L.fish_collected) {
        L.fish_time = FISH_TIME;
    }
    L.fish_collected = collected;

    L.fish_time -= dtime;
    L.fish_icon.rect.sprite.y = L.fish_time > 0 ? 1 : 0;
    L.fish_icon.rect.color.rgb = color;
    L.fish_icon.rect.pose = u_pose_new_aa(
            sca_floor(camera_right() - 32 - 4 * 12),
            sca_floor(camera_top()),
            32, 32);
    L.fish_cnt.pose = u_pose_new(
                  sca_floor(camera_right() - 4 * 12 - 2),
                  sca_floor(camera_top() - (32 - 12) / 2),
                  2, 2);
    L.min_info.pose = u_pose_new(
                  sca_floor(camera_right() - 4 * 12 - 2),
                  sca_floor(camera_top() - (32 + 12) / 2),
                  1, 1);

    if(L.fish_collected < 3) {
        ro_text_set_color(&L.min_info, (vec4) {{0.7, 0.1, 0.1, 1.0}});
    } else if(L.fish_collected == 3) {
        ro_text_set_color(&L.min_info, (vec4) {{0.7, 0.7, 0.1, 1.0}});
    } else {
        ro_text_set_color(&L.min_info, (vec4) {{0.1, 0.7, 0.1, 1.0}});
    }
    char buf[9];
    assume(collected >= 0 && collected < 1000, "?");
    sprintf(buf, "x%-2i", collected);
    ro_text_set_text(&L.fish_cnt, buf);
}


static void update_feed(float dtime) {
    int collected = feed.eaten;

    L.feed_collected = collected;

    L.feed_icon.rect.pose = u_pose_new_aa(
            sca_floor(camera_left() + 12),
            sca_floor(camera_top() - 6),
            16, 16);
    L.feed_cnt.pose = u_pose_new(
                  sca_floor(camera_left() + 32),
                  sca_floor(camera_top() - (32 - 12) / 2),
                  2, 2);
    char buf[9];
    assume(collected >= 0 && collected < 1000, "?");
    sprintf(buf, "x%-7i", collected);
    ro_text_set_text(&L.feed_cnt, buf);
}


//
// public
//

void hud_init() {
    L.fish_icon = ro_single_new(hudcamera.gl, r_texture_new_file(4, 2, "res/fish.png"));

    L.fish_cnt = ro_text_new_font55(8, hudcamera.gl);
    ro_text_set_color(&L.fish_cnt, R_COLOR_BLACK);

    L.min_info = ro_text_new_font55(8, hudcamera.gl);
    ro_text_set_text(&L.min_info, "min 3");
    ro_text_set_color(&L.min_info, R_COLOR_BLACK);
    
    L.feed_icon = ro_single_new(hudcamera.gl, r_texture_new_file(1, 4, "res/food.png"));

    L.feed_cnt = ro_text_new_font55(8, hudcamera.gl);
    ro_text_set_color(&L.feed_cnt, R_COLOR_BLACK);

}

void hud_kill() {
    ro_single_kill(&L.fish_icon);
    ro_text_kill(&L.fish_cnt);
    ro_single_kill(&L.feed_icon);
    ro_text_kill(&L.feed_cnt);
    memset(&L, 0, sizeof(L));
}

void hud_update(float dtime) {
    update_fish(dtime);
    update_feed(dtime);
}

void hud_render() {
    ro_single_render(&L.fish_icon);
    ro_text_render(&L.fish_cnt);
    ro_text_render(&L.min_info);

    ro_single_render(&L.feed_icon);
    ro_text_render(&L.feed_cnt);
}
