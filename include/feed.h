#ifndef GMTKJAM21_FEED_H
#define GMTKJAM21_FEED_H

#include "mathc/types/float.h"

#define FEED_MAX 16

typedef struct {
    vec2 pos;
    vec2 speed;
    vec4 color;
    float size;
} Feed_s;

struct FeedGlobals_s {
    Feed_s feed[FEED_MAX];
    int feed_size;
    int eaten;
};
extern struct FeedGlobals_s feed;

void feed_init();

void feed_update(float dtime);

void feed_render();

void feed_eat(Feed_s *self, float time);

#endif //GMTKJAM21_FEED_H
