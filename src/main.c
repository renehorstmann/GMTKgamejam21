#include "e/simple.h"

#include "camera.h"
#include "sound.h"
#include "background.h"
#include "bubbles.h"
#include "pixelparticles.h"
#include "login.h"
#include "game.h"

#define UPDATES_PER_SECOND 200

static struct {
    Camera_s camera;
    TextInput *textinput;

    Sound *sound;

    Background *bg;
    Bubbles *bubbles;
    PixelParticles *particles;

    Login *login;
    char name[LOGIN_NAME_MAX_LENGTH+1]; // + null

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

    L.login = login_new(simple->input, &L.camera);
}


#include "u/fetch.h"
#include "firstname.h"
#include "highscore.h"
static void post_random() {
    static uFetch *fetch = NULL;
    if(!fetch) {
        char name[128];
        firstname_generate(name);
        int score = (rand()%100)*100;
        String entry = highscore_entry_to_string(
            highscore_entry_new(name, score)
        );
        fetch = u_fetch_new_post("https://rohl.svenhuis.de/api/swarm", entry.str);
        string_kill(&entry);
    }
    String res = u_fetch_check_response(&fetch, NULL);
    string_kill(&res);
}


// this functions is called either each frame or at a specific update/s time
static void update(eSimple *simple, ivec2 window_size, float dtime) {
    //post_random();
    
    camera_update(&L.camera, window_size);

    sound_update(L.sound, dtime);

    bubbles_update(L.bubbles, dtime);
    pixelparticles_update(L.particles, dtime);

    if(L.login) {
        login_update(L.login, dtime);
        if(L.login->out.done) {
            strcpy(L.name, L.login->out.name);
            login_kill(&L.login);
            log_info("login done, starting game as: <%s>", L.name);
            L.game = game_new(simple->input, &L.camera, L.sound, L.particles, L.name);
        }
    }

    if(L.game)
        game_update(L.game, dtime);
}

// this function is calles each frame to render stuff, dtime is the time between frames
static void render(eSimple *simple, ivec2 window_size, float dtime) {
    const mat4 *cam_main_mat = &L.camera.matrices_main.vp;
    const mat4 *cam_hud_mat = &L.camera.matrices_p;

    background_render(L.bg, &L.camera);
    pixelparticles_render(L.particles, cam_main_mat);

    if(L.game)
        game_render_main(L.game, cam_main_mat);

    bubbles_render(L.bubbles, cam_hud_mat, L.camera.RO.real_pixel_per_pixel);

    if(L.login)
        login_render(L.login, cam_hud_mat);

    // clone the current framebuffer into r_render_get_framebuffer_tex
    r_render_blit_framebuffer(simple->render, window_size);

    if(L.game)
        game_render_hud(L.game, cam_hud_mat);

}

int main(int argc, char **argv) {
    e_simple_start("Swarm GMTK21", "Horsimann",
                   1.0f,   // startup block time (the time in which "Horsimann" is displayed at startup)
                   UPDATES_PER_SECOND,
                   init, update, render);
    return 0;
}
