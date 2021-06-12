#include "e/e.h"
#include "r/r.h"
#include "u/u.h"
#include "rhc/rhc.h"
#include "mathc/mathc.h"

#include "camera.h"
#include "hudcamera.h"
#include "cameractrl.h"
#include "background.h"
#include "pixelparticles.h"
#include "bubbles.h"
#include "fish.h"

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
    cameractrl_init();
    background_init(1280, 1280, true, true, "res/background.png");
    pixelparticles_init();
    bubbles_init();
    fish_init();


    e_window_main_loop(main_loop);

    e_gui_kill();

    return 0;
}


static void main_loop(float delta_time) {
    // e updates
    e_input_update();

    // simulate
    camera_update();
    hudcamera_update();
    cameractrl_update(delta_time);
    background_update(delta_time);
    pixelparticles_update(delta_time);
    bubbles_update(delta_time);
    fish_update(delta_time);

    // scripts
//    cameractrl.in.dst = fish_swarm_center();
//    vec2_println(cameractrl.in.dst);



    // render
    r_render_begin_frame(e_window.size.x, e_window.size.y);

    background_render();
    pixelparticles_render();
    fish_render();
    bubbles_render();


    // uncomment to clone the current framebuffer into r_render.framebuffer_tex
    r_render_blit_framebuffer(e_window.size.x, e_window.size.y);

    e_gui_render();

    // swap buffers
    r_render_end_frame();
}


