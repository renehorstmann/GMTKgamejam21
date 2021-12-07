#ifndef GMTKJAM21_LOGIN_H
#define GMTKJAM21_LOGIN_H

#include "e/input.h"
#include "r/ro_types.h"

typedef struct {
    
    struct {
        RoText title;
        RoText yourname;
        RoText name;
        RoSingle play;
    } L;
} Login;

Login *login_new(eInput *input);

void login_kill(Login **self_ptr);

void login_update(Login *self, float dtime);

void login_render(const Login *self, const mat4 *hud_cam_mat);

#endif //GMTKJAM21_LOGIN_H
