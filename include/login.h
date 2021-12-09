#ifndef GMTKJAM21_LOGIN_H
#define GMTKJAM21_LOGIN_H

#include "e/input.h"
#include "r/ro_types.h"
#include "camera.h"
#include "textinput.h"

#define LOGIN_NAME_MIN_LENGTH 3
#define LOGIN_NAME_MAX_LENGTH 16

typedef struct {
    eInput *input_ref;
    const Camera_s *cam_ref;
    
    struct {
        char name[LOGIN_NAME_MAX_LENGTH + 1]; // + null terminator
        bool done;
    } out;
    struct {
        TextInput *textinput;

        RoText title;
        RoText yourname;
        RoText name;
        mat4 name_click_box;
        RoSingle play;
    } L;
} Login;

Login *login_new(eInput *input, const Camera_s *cam);

void login_kill(Login **self_ptr);

void login_update(Login *self, ivec2 window_size, float dtime);

void login_render(const Login *self, const mat4 *hud_cam_mat);

#endif //GMTKJAM21_LOGIN_H
