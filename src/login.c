#include "e/io.h"
#include "r/ro_text.h"
#include "r/ro_single.h"
#include "r/texture.h"
#include "rhc/log.h"
#include "rhc/alloc.h"
#include "rhc/str.h"
#include "u/pose.h"

#include "button.h"
#include "firstname.h"
#include "login.h"

_Static_assert(FIRSTNAME_MAX_LENGTH <= LOGIN_NAME_MAX_LENGTH, "wtf");

static void start_textinput(Login *self) {
    self->L.textinput = textinput_new(self->input_ref, self->cam_ref, "Your name:", LOGIN_NAME_MAX_LENGTH);
    strcpy(self->L.textinput->out.text, self->out.name);
}

static void pointer_callback(ePointer_s pointer, void *user_data) {
    Login *self = user_data;

    pointer.pos = mat4_mul_vec(self->cam_ref->matrices_main.v_p_inv, pointer.pos);

    if (button_clicked(&self->L.play.rect, pointer)) {
        bool saved = e_io_savestate_write("name.txt", strc(self->out.name), true);
        log_info("login: play, (saved=%i): <%s>", saved, self->out.name);
        self->out.done = true;
    }

    if (pointer.action == E_POINTER_DOWN
        && u_pose_aa_contains(self->L.name_click_box, pointer.pos.xy)) {
        log_info("login: rename");
        start_textinput(self);
    }
}

Login *login_new(eInput *input, const Camera_s *cam) {
    Login *self = rhc_calloc(sizeof *self);

    self->input_ref = input;
    self->cam_ref = cam;

    e_input_register_pointer_event(input, pointer_callback, self);

    self->L.title = ro_text_new_font55(5);
    ro_text_set_text(&self->L.title, "SWARM");

    self->L.yourname = ro_text_new_font85(10);
    ro_text_set_text(&self->L.yourname, "Your name:");
    ro_text_set_color(&self->L.yourname, (vec4) {{0, 0, 0, 0.5}});

    self->L.name = ro_text_new_font85(LOGIN_NAME_MAX_LENGTH);
    ro_text_set_color(&self->L.name, R_COLOR_BLACK);

    self->L.play = ro_single_new(r_texture_new_file(2, 1, "res/play.png"));

    // init name
    String name = e_io_savestate_read("name.txt", true);
    if (string_valid(name)) {
        // strncpy does not set null terminator, but out.name is initialized with zeros and has LOGIN_NAME_MAX_LENGTH+1
        strncpy(self->out.name, name.data, name.size < LOGIN_NAME_MAX_LENGTH ? name.size : LOGIN_NAME_MAX_LENGTH);
    } else {
        firstname_generate(self->out.name);
        start_textinput(self);
    }
    string_kill(&name);


    // poses
    self->L.title.pose = u_pose_new(-80, 80, 4, 4);
    self->L.yourname.pose = u_pose_new(-60, 40, 2, 2);
    self->L.name.pose = u_pose_new(-60, 10, 1, 1);
    self->L.name_click_box = u_pose_new_aa(-60, 10,
                                           LOGIN_NAME_MAX_LENGTH * self->L.name.offset.x,
                                           self->L.name.offset.y);
    self->L.play.rect.pose = u_pose_new(0, -44, 64, 64);

    return self;
}

void login_kill(Login **self_ptr) {
    Login *self = *self_ptr;
    if (!self)
        return;
    e_input_unregister_pointer_event(self->input_ref, pointer_callback);
    ro_text_kill(&self->L.title);
    ro_text_kill(&self->L.yourname);
    ro_text_kill(&self->L.name);
    ro_single_kill(&self->L.play);
    rhc_free(self);
    *self_ptr = NULL;
}

void login_update(Login *self, float dtime) {
    if(self->L.textinput) {
        textinput_update(self->L.textinput, dtime);

        if(self->L.textinput->out.state == TEXTINPUT_CANCELED) {
            textinput_kill(&self->L.textinput);
            self->L.textinput = NULL;   // just to be safe here...
        } else if(self->L.textinput->out.state == TEXTINPUT_DONE) {
            strcpy(self->out.name, self->L.textinput->out.text);
            textinput_kill(&self->L.textinput);
            self->L.textinput = NULL;   // just to be safe here...
        }
    }

    ro_text_set_text(&self->L.name, self->out.name);
}

void login_render(const Login *self, const mat4 *hud_cam_mat) {
    ro_text_render(&self->L.title, hud_cam_mat);

    if(self->L.textinput) {
        textinput_render(self->L.textinput, hud_cam_mat);
    } else {
        ro_text_render(&self->L.yourname, hud_cam_mat);
        ro_text_render(&self->L.name, hud_cam_mat);
        ro_single_render(&self->L.play, hud_cam_mat);
    }
}
