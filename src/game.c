#include "rhc/alloc.h"
#include "game.h"


static void game_start(Game *self) {
    self->camctrl = cameractrl_new();
    
    self->feed = feed_new(self->sound_ref, self->particles_ref);
    self->fish = fish_new(self->input_ref, self->cam_ref, self->sound_ref, self->particles_ref, self->feed);
    self->shark = shark_new(self->sound_ref, self->particles_ref, self->fish);
    
    self->spawn = spawn_new(self->cam_ref, &self->camctrl, self->fish, self->feed, self->shark);
    
    self->hud = hud_new(self->cam_ref, self->fish, self->feed);
    
    self->dead = dead_new(self->input_ref, self->cam_ref, self->sound_ref, self->fish, self);
}

static void game_end(Game *self) {
    fish_kill(&self->fish);
    feed_kill(&self->feed);
    shark_kill(&self->shark);
    spawn_kill(&self->spawn);
    hud_kill(&self->hud);
    dead_kill(&self->dead);
}

Game *game_new(eInput *input, Camera_s *cam, Sound *sound, PixelParticles *particles) {
    Game *self = rhc_calloc(sizeof *self);
    
    self->input_ref = input;
    self->cam_ref = cam;
    self->sound_ref = sound;
    self->particles_ref = particles;
    
    game_start(self);
    
    return self;
}

void game_update(Game *self, float dtime) {
    cameractrl_update(&self->camctrl, self->cam_ref, dtime);
    
    fish_update(self->fish, dtime);
    feed_update(self->feed, dtime);
    shark_update(self->shark, dtime);
    spawn_update(self->spawn, dtime);
    hud_update(self->hud, dtime);
    dead_update(self->dead, dtime);
    
    // scripts
    self->camctrl.in.dst = fish_swarm_center(self->fish);
    while(self->feed->out.score>0) {
        self->feed->out.score--;
        hud_score(self->hud);
    }
}

void game_render(Game *self, const mat4 *cam_mat, const mat4 *hudcam_mat) {
    
    feed_render(self->feed, cam_mat);
    fish_render(self->fish, cam_mat);
    shark_render(self->shark, cam_mat);
    
    dead_render(self->dead, hudcam_mat);
    hud_render(self->hud, hudcam_mat);
}

void game_reset(Game *self) {
    game_end(self);
    game_start(self);
}
