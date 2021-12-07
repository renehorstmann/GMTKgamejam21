#ifndef GMTKJAM21_DEAD_H
#define GMTKJAM21_DEAD_H

#include "e/input.h"
#include "r/ro_types.h"
#include "camera.h"
#include "sound.h"
#include "fish.h"

struct Game;

typedef struct {
    eInput *input_ref;
    const Camera_s *cam_ref;
    Sound *sound_ref;
    Fish *fish_ref;
    struct Game *game_ref;

    struct {
        RoSingle ro;
        RoSingle btn;
        RoText info;
        RoText credits;
        bool show;
        float time;
    } L;
} Dead;

Dead *dead_new(eInput *input, const Camera_s *cam, Sound *sound, Fish *fish, struct Game *game);

void dead_kill(Dead **self_ptr);

void dead_update(Dead *self, float dtime);

void dead_render(const Dead *self, const mat4 *cam_mat);

#endif //GMTKJAM21_DEAD_H
