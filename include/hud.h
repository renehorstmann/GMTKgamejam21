#ifndef GMTKJAM21_HUD_H
#define GMTKJAM21_HUD_H

#include "camera.h"
#include "fish.h"
#include "feed.h"

typedef struct {
    const Camera_s *cam_ref;
    const Fish *fish_ref;
    const Feed *feed_ref;
    
    struct {
        RoSingle fish_icon;
        RoText fish_cnt;
        RoText min_info;
        float fish_time;
        int fish_collected;

        RoSingle feed_icon;
        RoText feed_cnt;
        int feed_collected;

        int score;
        RoText score_cnt;
        vec3 score_hsv;
    } L;
} Hud;

Hud *hud_new(const Camera_s *cam, const Fish *fish, const Feed *feed);

void hud_kill(Hud **self_ptr);

void hud_update(Hud *self, float dtime);

void hud_render(const Hud *self, const mat4 *hud_cam_mat);

void hud_score(Hud *self);

#endif //GMTKJAM21_HUD_H
