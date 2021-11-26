#include "e/e.h"
#include "r/r.h"
#include "u/u.h"
#include "rhc/rhc.h"
#include "mathc/mathc.h"

#include "camera.h"
#include "hudcamera.h"
#include "sound.h"
#include "game.h"


#define UPDATES_PER_SECOND 200

static struct {
    eWindow *window;
    eInput *input;
    eGui *gui;
    rRender *render;
} L;


static void main_loop(float delta_time);


int main(int argc, char **argv) {
    log_info("Swarm GMTK21");

    // init e (environment)
    L.window = e_window_new("Swarm GMTK21");
    L.input = e_input_new(L.window);
    L.gui = e_gui_new(L.window);

    ivec2 window_size = e_window_get_size(L.window);

    // init r (render)
    L.render = r_render_new(e_window_get_sdl_window(L.window));

    // the startup screen acts as loading screen and also checks for render errors
    r_render_show_startup(L.render,
                          window_size.x, window_size.y,
                          1.0, // block time
                          "Horsimann");

    // init systems
    camera_init();
    hudcamera_init();
    camera_update(window_size);
    hudcamera_update(window_size);
    sound_init();
    game_init(L.input, L.render);



    e_window_main_loop(L.window, main_loop);

    r_render_kill(&L.render);
    e_gui_kill(&L.gui);
    e_input_kill(&L.input);
    e_window_kill(&L.window);

    return 0;
}


static void main_loop(float delta_time) {
    static float u_time = 0;

    ivec2 window_size = e_window_get_size(L.window);

    // e updates
    e_input_update(L.input);

    // simulate
    camera_update(window_size);
    hudcamera_update(window_size);

    // fixed update ps
    u_time += delta_time;
    while (u_time > 0) {
        const float fixed_time = 1.0 / UPDATES_PER_SECOND;
        u_time -= fixed_time;

        // simulate game
        game_update(fixed_time);
    }


    // render
    r_render_begin_frame(L.render, window_size.x, window_size.y);

    game_render();

    // uncomment to clone the current framebuffer into r_render.framebuffer_tex
    r_render_blit_framebuffer(L.render, window_size.x, window_size.y);

    e_gui_render(L.gui);

    // swap buffers
    r_render_end_frame(L.render);
}


