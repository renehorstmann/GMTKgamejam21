#include "e/simple.h"

#include "camera.h"
#include "hudcamera.h"
#include "sound.h"
#include "game.h"

#include "textinput.h"


#define UPDATES_PER_SECOND 200

static struct {
   TextInput *textinput;
} L;

// this function will be called at the start of the app
static void init(eSimple *simple, ivec2 window_size) {
    // init systems
    camera_init();
    hudcamera_init();
    camera_update(window_size);
    hudcamera_update(window_size);
    sound_init();
    game_init(simple->input, simple->render);

    L.textinput = textinput_new(simple->input, "Your name:", 0);
}


// this functions is called either each frame or at a specific update/s time
static void update(eSimple *simple, ivec2 window_size, float delta_time) {
    camera_update(window_size);
    hudcamera_update(window_size);
    game_update(delta_time);

    textinput_update(L.textinput, delta_time);
}

// this function is calles each frame to render stuff
static void render(eSimple *simple, ivec2 window_size) {
    game_render();

    textinput_render(L.textinput, (const mat4*) hudcamera.gl);

    // clone the current framebuffer into r_render_get_framebuffer_tex
    r_render_blit_framebuffer(simple->render, window_size);
}

int main(int argc, char **argv) {
    e_simple_start("Swarm GMTK21", "Horsimann",
                   1.0f,   // startup block time (the time in which "Horsimann" is displayed at startup)
                   UPDATES_PER_SECOND,
                   init, update, render);
    return 0;
}
