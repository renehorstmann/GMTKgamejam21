#include "e/simple.h"

#include "camera.h"
#include "sound.h"
#include "background.h"
#include "bubbles.h"
#include "pixelparticles.h"
#include "game.h"

#include "textinput.h"


#define UPDATES_PER_SECOND 200

static struct {
   Camera_s camera;
   TextInput *textinput;
   
   Sound *sound;
   
   Background *bg;
   Bubbles *bubbles;
   PixelParticles *particles;
   Game *game;
} L;

// this function will be called at the start of the app
static void init(eSimple *simple, ivec2 window_size) {
    // init systems
    L.camera = camera_new();
    camera_update(&L.camera, window_size);
    
    L.sound = sound_new(simple->input);
    
    L.bg = background_new(simple->render, 1280, 1280, true, true, "res/background.png");
    L.bubbles = bubbles_new(&L.camera);
    L.particles = pixelparticles_new();
    
    L.game = game_new(simple->input, &L.camera, L.sound, L.particles);

    //L.textinput = textinput_new(simple->input, &L.camera, "Your name:", 0);
}


// this functions is called either each frame or at a specific update/s time
static void update(eSimple *simple, ivec2 window_size, float delta_time) {
    camera_update(&L.camera, window_size);
    
    sound_update(L.sound, delta_time);
    
    bubbles_update(L.bubbles, delta_time);
    pixelparticles_update(L.particles, delta_time);
    game_update(L.game, delta_time);

    //textinput_update(L.textinput, delta_time);
}

// this function is calles each frame to render stuff
static void render(eSimple *simple, ivec2 window_size) {
    const mat4 *cam_main_mat = &L.camera.matrices_main.vp;
    const mat4 *cam_hud_mat = &L.camera.matrices_p;
    
    background_render(L.bg, &L.camera);
    pixelparticles_render(L.particles, cam_main_mat);
    game_render(L.game, cam_main_mat, cam_hud_mat);
    bubbles_render(L.bubbles, cam_main_mat, L.camera.RO.real_pixel_per_pixel);

    //textinput_render(L.textinput, cam_hud_mat);

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
