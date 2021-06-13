#include "e/e.h"
#include "r/r.h"
#include "u/u.h"
#include "rhc/rhc.h"
#include "mathc/mathc.h"

#include "camera.h"
#include "hudcamera.h"
#include "game.h"


#define UPDATES_PER_SECOND 200

static void main_loop(float delta_time);


int main(int argc, char **argv) {
    log_info("gmtkjam21");

    // init e (environment)
    e_window_init("gmtkjam21");
    e_input_init();
    e_gui_init();

    // init r (render)
    r_render_init(e_window.window);

    // init systems
    camera_init();
    hudcamera_init();
    game_init();



    e_window_main_loop(main_loop);

    e_gui_kill();

    return 0;
}


static void main_loop(float delta_time) {
    static float u_time = 0;

    // e updates
    e_input_update();

    // simulate
    camera_update();
    hudcamera_update();

    // fixed update ps
    u_time += delta_time;
    while (u_time > 0) {
        const float fixed_time = 1.0 / UPDATES_PER_SECOND;
        u_time -= fixed_time;

        // simulate game
        game_update(fixed_time);
    }


    // render
    r_render_begin_frame(e_window.size.x, e_window.size.y);

    game_render();

    // uncomment to clone the current framebuffer into r_render.framebuffer_tex
    r_render_blit_framebuffer(e_window.size.x, e_window.size.y);

    e_gui_render();

    // swap buffers
    r_render_end_frame();
}


