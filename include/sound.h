#ifndef GMTKJAM21_SOUND_H
#define GMTKJAM21_SOUND_H

#include "e/input.h"

typedef struct Sound Sound;

Sound *sound_new(eInput *input);

void sound_update(Sound *self, float dtime);

void sound_play_activate(Sound *self);

void sound_play_feed(Sound *self);

void sound_play_shark(Sound *self);

void sound_play_gameover(Sound *self);

#endif //GMTKJAM21_SOUND_H
