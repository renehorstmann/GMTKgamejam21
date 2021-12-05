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

// this function will be called at the start of the app
static void init(eSimple *simple, ivec2 window_size) {
    // init systems
    camera_init();
    hudcamera_init();
    camera_update(window_size);
    hudcamera_update(window_size);
    sound_init();
    game_init(simple->input, simple->render);
}


// this functions is called either each frame or at a specific update/s time
static void update(eSimple *simple, ivec2 window_size, float delta_time) {
    camera_update(window_size);
    hudcamera_update(window_size);
    game_update(delta_time);
}

// this function is calles each frame to render stuff
static void render(eSimple *simple, ivec2 window_size) {
    game_render();

    // clone the current framebuffer into r_render_get_framebuffer_tex
    r_render_blit_framebuffer(simple->render, window_size);
}

int main(int argc, char **argv) {
    e_simple_start("Swarm GMTK21", "Horsimann",
                   UPDATES_PER_SECOND, // updates/s, <=0 to turn off and use fps
                   init, update, render);
    return 0;
}
