#include "cameractrl.h"
#include "background.h"
#include "pixelparticles.h"
#include "bubbles.h"
#include "fish.h"
#include "feed.h"
#include "shark.h"
#include "spawn.h"
#include "hud.h"
#include "dead.h"
#include "game.h"

static struct {
    eInput *input_ref;
    rRender *render_ref;
} L;

static void init() {
    cameractrl_init();
    background_init(L.render_ref, 1280, 1280, true, true, "res/background.png");
    pixelparticles_init();
    bubbles_init();
    
    fish_init(L.input_ref);
    feed_init();
    shark_init();
    spawn_init();
    hud_init();
    dead_init(L.input_ref);
}

static void kill() {
    cameractrl_kill();
    background_kill();
    pixelparticles_kill();
    bubbles_kill();
    
    fish_kill();
    feed_kill();
    shark_kill();
    spawn_kill();
    hud_kill();
    dead_kill();
}

void game_init(eInput *input, rRender *render) {
    L.input_ref = input;
    L.render_ref = render;
    init();
}

void game_update(float dtime) {
    cameractrl_update(dtime);
    background_update(dtime);
    pixelparticles_update(dtime);
    bubbles_update(dtime);
    
    fish_update(dtime);
    feed_update(dtime);
    shark_update(dtime);
    spawn_update(dtime);
    hud_update(dtime);
    dead_update(dtime);
    
    // scripts
    cameractrl.in.dst = fish_swarm_center();
}

void game_render() {
    background_render();
    pixelparticles_render();
    
    feed_render();
    fish_render();
    shark_render();
    bubbles_render();

    dead_render();
    hud_render();
}

void game_reset(eInput *input, rRender *render) {
    kill();
    init();
}
