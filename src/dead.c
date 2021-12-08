#include "e/input.h"
#include "r/ro_single.h"
#include "r/ro_text.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "game.h"
#include "button.h"
#include "dead.h"


#define RESCUE_TIME 5.0
#define TEXT_SIZE 3.0


static void pointer_callback(ePointer_s pointer, void *user_data) {
    Dead *self = user_data;
    if (!self->L.show || self->L.time > 0)
        return;
    pointer.pos = mat4_mul_vec(self->cam_ref->matrices_p_inv, pointer.pos);

    if(self->showscore)
        showscore_pointer(self->showscore, pointer);

    if (button_clicked(&self->L.btn.rect, pointer)) {
        game_reset(self->game_ref);
    }
}

Dead *dead_new(eInput *input, const Camera_s *cam, Sound *sound, Fish *fish, Game *game) {
    Dead *self = rhc_calloc(sizeof *self);

    self->input_ref = input;
    self->cam_ref = cam;
    self->sound_ref = sound;
    self->fish_ref = fish;
    self->game_ref = game;

    e_input_register_pointer_event(input, pointer_callback, self);

    self->L.ro = ro_single_new(r_texture_new_white_pixel());
    self->L.ro.rect.pose = u_pose_new(0, 0, 2048, 2048);
    self->L.ro.rect.color = (vec4) {{0.5, 0.1, 0.1, 0.0}};

    self->L.btn = ro_single_new(r_texture_new_file(2, 1, "res/retry.png"));

    self->L.info = ro_text_new_font55(64);
    ro_text_set_color(&self->L.info, R_COLOR_BLACK);

    self->L.credits = ro_text_new_font55(32);
    ro_text_set_text(&self->L.credits, "\"swarm\" by horsimann");

    return self;
}

void dead_kill(Dead **self_ptr) {
    Dead *self = *self_ptr;
    if (!self)
        return;
    showscore_kill(&self->showscore);
    e_input_unregister_pointer_event(self->input_ref, pointer_callback);
    ro_single_kill(&self->L.ro);
    ro_text_kill(&self->L.info);
    ro_single_kill(&self->L.btn);
    ro_text_kill(&self->L.credits);
    rhc_free(self);
    *self_ptr = NULL;
}

void dead_update(Dead *self, float dtime) {
    
    
    if (self->fish_ref->game_running && self->fish_ref->swarmed_size >= 3) {
        self->L.show = false;
        return;
    }

    if (!self->L.show) {
        self->L.show = true;
        self->L.time = RESCUE_TIME;
        sound_play_gameover(self->sound_ref);
    }

    self->L.time -= dtime;

    char buf[65];
    if (self->L.time > 0) {
        sprintf(buf, "FISH ALERT!\n\n%7.2f", self->L.time);
    } else {
        self->L.time = 0;
        sprintf(buf, "GAME OVER");
        self->fish_ref->game_running = false;
        
        if(!self->showscore) {
            log_info("dead: starting showscore");
            self->showscore = showscore_new(self->game_ref->name_ref, self->game_ref->hud->RO.score);
        }
    }

    vec2 size = ro_text_set_text(&self->L.info, buf);
    self->L.info.pose = u_pose_new(sca_floor(-size.x / 2 * TEXT_SIZE),
                                   -40 + sca_floor(+size.y / 2 * TEXT_SIZE),
                                   TEXT_SIZE, TEXT_SIZE);

    self->L.ro.rect.color.a = sca_mix(0.75, 0.1, self->L.time / RESCUE_TIME);

    self->L.btn.rect.pose = u_pose_new(0, -40-64, 64, 64);

    self->L.credits.pose = u_pose_new(sca_floor(1 - CAMERA_SIZE / 2), sca_floor(self->cam_ref->RO.top - 1), 1, 1);
    
    if(self->showscore) {
        self->showscore->in.pos = (vec2) {{-90, 100}};
        showscore_update(self->showscore, dtime);
    }
}

void dead_render(const Dead *self, const mat4 *cam_mat) {
    
    if (!self->L.show)
        return;
    ro_single_render(&self->L.ro, cam_mat);
    ro_text_render(&self->L.info, cam_mat);
    if(self->showscore)
        showscore_render(self->showscore, cam_mat);
    if (self->L.time <= 0)
        ro_single_render(&self->L.btn, cam_mat);
    ro_text_render(&self->L.credits, cam_mat);
}
