#include "r/ro_text.h"
#include "r/ro_single.h"
#include "r/texture.h"
#include "rhc/alloc.h"
#include "u/pose.h"

#include "highscore.h"
#include "login.h"


Login *login_new(eInput *input) {
    Login *self = rhc_calloc(sizeof *self);

    self->L.title = ro_text_new_font55(5);
    ro_text_set_text(&self->L.title, "SWARM");

    self->L.yourname = ro_text_new_font85(10);
    ro_text_set_text(&self->L.yourname, "Your name:");

    self->L.name = ro_text_new_font85(HIGHSCORE_NAME_MAX_LENGTH);

    self->L.play = ro_single_new(r_texture_new_file(2, 1, "res/play.png"));

    return self;
}

void login_kill(Login **self_ptr) {
    Login *self = *self_ptr;
    if(!self)
        return;
    ro_text_kill(&self->L.title);
    ro_text_kill(&self->L.yourname);
    ro_text_kill(&self->L.name);
    ro_single_kill(&self->L.play);
    rhc_free(self);
    *self_ptr = NULL;
}

void login_update(Login *self, float dtime) {
    self->L.title.pose = u_pose_new(-80, 80, 4, 4);
    self->L.yourname.pose = u_pose_new(-60, 20, 2, 2);
    self->L.name.pose = u_pose_new(-60, 0, 1, 1);
    self->L.play.rect.pose = u_pose_new(0, -32, 32, 32);
}

void login_render(const Login *self, const mat4 *hud_cam_mat) {
    ro_text_render(&self->L.title, hud_cam_mat);
    ro_text_render(&self->L.yourname, hud_cam_mat);
    ro_text_render(&self->L.name, hud_cam_mat);
    ro_single_render(&self->L.play, hud_cam_mat);
}
