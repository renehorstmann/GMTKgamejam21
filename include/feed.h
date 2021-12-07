#ifndef GMTKJAM21_FEED_H
#define GMTKJAM21_FEED_H

#include "sound.h"
#include "pixelparticles.h"

#define FEED_MAX 24

typedef struct {
    vec2 pos;
    vec2 speed;
    vec4 color;
    float size;
} FeedItem_s;

typedef struct {
    Sound *sound_ref;
    PixelParticles *particles_ref;
    
    FeedItem_s feed[FEED_MAX];
    int feed_size;
    int eaten;
    struct {
        int score;
    } out;
    struct {
        RoBatch ro;
    } L;
} Feed;

Feed *feed_new(Sound *sound, PixelParticles *particles);

void feed_kill(Feed **self_ptr);

void feed_update(Feed *self, float dtime);

void feed_render(const Feed *self, const mat4 *cam_mat);

void feed_eat(Feed *self, FeedItem_s *item, float time);

#endif //GMTKJAM21_FEED_H
