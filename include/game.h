#ifndef GMTKJAM21_GAME_H
#define GMTKJAM21_GAME_H

#include "e/input.h"
#include "camera.h"

#include "cameractrl.h"
#include "fish.h"
#include "feed.h"
#include "shark.h"
#include "spawn.h"
#include "hud.h"
#include "dead.h"

typedef struct Game {
    eInput *input_ref;
    Camera_s *cam_ref;
    Sound *sound_ref;
    PixelParticles *particles_ref;
    
    CameraCtrl_s camctrl;
    
    Fish *fish;
    Feed *feed;
    Shark *shark;
    Spawn *spawn;
    Hud *hud;
    Dead *dead;
} Game;


Game *game_new(eInput *input, Camera_s *cam, Sound *sound, PixelParticles *particles);

void game_kill(Game **self_ptr);

void game_update(Game *self, float dtime);

void game_render_main(const Game *self, const mat4 *cam_mat);

void game_render_hud(const Game *self, const mat4 *hudcam_mat);

void game_reset(Game *self);

#endif //GMTKJAM21_GAME_H
