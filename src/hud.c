#include <stdio.h>
#include "r/ro_single.h"
#include "r/ro_batch.h"
#include "r/ro_text.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/color.h"
#include "rhc/error.h"

#include "hud.h"


#define FISH_TIME 0.2

//
// private
//


static void update_fish(Hud *self, float dtime) {
    int collected = self->fish_ref->swarmed_size;
    vec3 color = vec3_hsv2rgb(
            self->fish_ref->swarmed[self->fish_ref->last_catched_idx].hsv);

    if (collected > self->L.fish_collected) {
        self->L.fish_time = FISH_TIME;
    }
    self->L.fish_collected = collected;

    self->L.fish_time -= dtime;
    self->L.fish_icon.rect.sprite.y = self->L.fish_time > 0 ? 1 : 0;
    self->L.fish_icon.rect.color.rgb = color;
    self->L.fish_icon.rect.pose = u_pose_new_aa(
            sca_floor(self->cam_ref->RO.right - 32 - 4 * 12),
            sca_floor(self->cam_ref->RO.top),
            32, 32);
    self->L.fish_cnt.pose = u_pose_new(
                  sca_floor(self->cam_ref->RO.right - 4 * 12 - 2),
                  sca_floor(self->cam_ref->RO.top - (32 - 12) / 2),
                  2, 2);
    self->L.min_info.pose = u_pose_new(
                  sca_floor(self->cam_ref->RO.right - 4 * 12 - 2),
                  sca_floor(self->cam_ref->RO.top - (32 + 12) / 2),
                  1, 1);

    if(self->L.fish_collected < 3) {
        ro_text_set_color(&self->L.min_info, (vec4) {{0.7, 0.1, 0.1, 1.0}});
    } else if(self->L.fish_collected == 3) {
        ro_text_set_color(&self->L.min_info, (vec4) {{0.7, 0.7, 0.1, 1.0}});
    } else {
        ro_text_set_color(&self->L.min_info, (vec4) {{0.1, 0.7, 0.1, 1.0}});
    }
    char buf[9];
    assume(collected >= 0 && collected < 1000, "?");
    sprintf(buf, "x%-2i", collected);
    ro_text_set_text(&self->L.fish_cnt, buf);
}


static void update_feed(Hud *self, float dtime) {
    int collected = self->feed_ref->eaten;

    self->L.feed_collected = collected;

    self->L.feed_icon.rect.pose = u_pose_new_aa(
            sca_floor(self->cam_ref->RO.left + 12),
            sca_floor(self->cam_ref->RO.top - 6),
            16, 16);
    self->L.feed_cnt.pose = u_pose_new(
                  sca_floor(self->cam_ref->RO.left + 32),
                  sca_floor(self->cam_ref->RO.top - (32 - 12) / 2),
                  2, 2);
    char buf[9];
    assume(collected >= 0 && collected < 1000, "?");
    sprintf(buf, "x%-7i", collected);
    ro_text_set_text(&self->L.feed_cnt, buf);
}

static void update_score(Hud *self, float dtime) {
    char buf[33];
    sprintf(buf, "SCORE %-i", self->L.score);
    vec2 size = ro_text_set_text(&self->L.score_cnt, buf);
    self->L.score_cnt.pose = u_pose_new(
            sca_floor(-size.x/2),
            sca_floor(self->cam_ref->RO.top - (32 - 12) / 2),
            1, 2);

//    self->L.score_hsv.v0 = sca_mod(self->L.score_hsv.v0 + 60 * dtime, 180);
//    vec4 color;
//    color.rgb = vec3_hsv2rgb(self->L.score_hsv);
//    color.a = 1;
//    ro_text_set_color(&self->L.score_cnt, color);
}


//
// public
//

Hud *hud_new(const Camera_s *cam, const Fish *fish, const Feed *feed) {
    Hud *self = rhc_calloc(sizeof *self);
    
    self->cam_ref = cam;
    self->fish_ref = fish;
    self->feed_ref = feed;
    
    self->L.fish_icon = ro_single_new(r_texture_new_file(4, 2, "res/fish.png"));

    self->L.fish_cnt = ro_text_new_font55(8);
    ro_text_set_color(&self->L.fish_cnt, R_COLOR_BLACK);

    self->L.min_info = ro_text_new_font55(8);
    ro_text_set_text(&self->L.min_info, "min 3");
    ro_text_set_color(&self->L.min_info, R_COLOR_BLACK);
    
    self->L.feed_icon = ro_single_new(r_texture_new_file(1, 4, "res/food.png"));

    self->L.feed_cnt = ro_text_new_font55(8);
    ro_text_set_color(&self->L.feed_cnt, R_COLOR_BLACK);

    self->L.score_cnt = ro_text_new_font55(32);
    ro_text_set_color(&self->L.score_cnt, R_COLOR_BLACK);
    self->L.score_hsv.v1 = 0.3;
    self->L.score_hsv.v2 = 1.0;
    
    return self;
}

void hud_kill(Hud **self_ptr) {
    Hud *self = *self_ptr;
    if(!self)
        return;
    ro_single_kill(&self->L.fish_icon);
    ro_text_kill(&self->L.fish_cnt);
    ro_single_kill(&self->L.feed_icon);
    ro_text_kill(&self->L.feed_cnt);
    rhc_free(self);
    *self_ptr = NULL;
}

void hud_update(Hud *self, float dtime) {
    update_fish(self, dtime);
    update_feed(self, dtime);
    update_score(self, dtime);
}

void hud_render(const Hud *self, const mat4 *hud_cam_mat) {
    ro_single_render(&self->L.fish_icon, hud_cam_mat);
    ro_text_render(&self->L.fish_cnt, hud_cam_mat);
    ro_text_render(&self->L.min_info, hud_cam_mat);

    ro_single_render(&self->L.feed_icon, hud_cam_mat);
    ro_text_render(&self->L.feed_cnt, hud_cam_mat);

    ro_text_render(&self->L.score_cnt, hud_cam_mat);
}

void hud_score(Hud *self) {
    self->L.score += 100 * self->fish_ref->swarmed_size;
}
