#ifndef GMTKJAM21_FEED_H
#define GMTKJAM21_FEED_H


#define FEED_MAX 16

typedef struct {
    vec2 pos;
    vec2 speed;
    vec4 color;
} Feed_s;

struct FeedGlobals_s {
    Feed_s feed[FEED_MAX];
    int feed_size;
};
extern struct FeedGlobals_s feed;

void feed_init();

void feed_update(float dtime);

void feed_render();

#endif //GMTKJAM21_FEED_H
