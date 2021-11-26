#include "e/input.h"
#include "r/ro_single.h"
#include "r/ro_text.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "hudcamera.h"
#include "fish.h"
#include "game.h"
#include "button.h"
#include "sound.h"
#include "dead.h"


#define RESCUE_TIME 5.0
#define TEXT_SIZE 3.0

static struct {
    eInput *input_ref;

    RoSingle ro;
    RoSingle btn;
    RoText info;
    RoText credits;
    bool show;
    float time;
} L;


static void pointer_callback(ePointer_s pointer, void *user_data) {
    if (!L.show || L.time > 0)
        return;
    pointer.pos = mat4_mul_vec(hudcamera.matrices.p_inv, pointer.pos);

    if (button_clicked(&L.btn.rect, pointer)) {
        game_reset();
    }
}

void dead_init(eInput *input) {
    L.input_ref = input;
    e_input_register_pointer_event(input, pointer_callback, NULL);

    L.ro = ro_single_new(r_texture_new_white_pixel());
    L.ro.rect.pose = u_pose_new(0, 0, 2048, 2048);
    L.ro.rect.color = (vec4) {{0.5, 0.1, 0.1, 0.0}};

    L.btn = ro_single_new(r_texture_new_file(2, 1, "res/retry.png"));

    L.info = ro_text_new_font55(64);
    ro_text_set_color(&L.info, R_COLOR_BLACK);

    L.credits = ro_text_new_font55(32);
    ro_text_set_text(&L.credits, "\"swarm\" by horsimann");
}

void dead_kill() {
    e_input_unregister_pointer_event(L.input_ref, pointer_callback);
    ro_single_kill(&L.ro);
    ro_text_kill(&L.info);
    ro_single_kill(&L.btn);
    ro_text_kill(&L.credits);
    memset(&L, 0, sizeof(L));
}

void dead_update(float dtime) {
    if (fish.swarmed_size >= 3) {
        L.show = false;
        return;
    }

    if (!L.show) {
        L.show = true;
        L.time = RESCUE_TIME;
        sound_play_gameover();
    }

    L.time -= dtime;

    char buf[65];
    if (L.time > 0) {
        sprintf(buf, "FISH ALERT!\n\n%7.2f", L.time);
    } else {
        L.time = 0;
        sprintf(buf, "GAME OVER");
        fish.game_running = false;
    }

    vec2 size = ro_text_set_text(&L.info, buf);
    L.info.pose = u_pose_new(sca_floor(-size.x / 2 * TEXT_SIZE),
                             sca_floor(+size.y / 2 * TEXT_SIZE),
                             TEXT_SIZE, TEXT_SIZE);

    L.ro.rect.color.a = sca_mix(0.75, 0.1, L.time / RESCUE_TIME);

    L.btn.rect.pose = u_pose_new(0, -64, 64, 64);

    L.credits.pose = u_pose_new(sca_floor(1-HUDCAMERA_SIZE/2), sca_floor(hudcamera_top()-1), 1, 1);
}

void dead_render() {
    if (!L.show)
        return;
    ro_single_render(&L.ro, (const mat4*) hudcamera.gl);
    ro_text_render(&L.info, (const mat4*) hudcamera.gl);
    if (L.time <= 0)
        ro_single_render(&L.btn, (const mat4*) hudcamera.gl);
    ro_text_render(&L.credits, (const mat4*) hudcamera.gl);
}
