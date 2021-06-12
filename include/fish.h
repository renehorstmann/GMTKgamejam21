#ifndef GMTKJAM21_FISH_H
#define GMTKJAM21_FISH_H

#include "mathc/types/float.h"

#define FISH_MAX 24

enum FishState {
    FISH_STATE_SWIM,
    FISH_STATE_EAT,
    FISH_NUM_STATES
};

typedef struct {
    vec2 pos;
    vec2 speed;
    vec2 set_speed;
    vec3 hsv;

    struct {
        enum FishState state;
        float animate_time;
        bool looking_left;
    } L;
} Fish_s;

struct FishGlobals_s {
    Fish_s swarmed[FISH_MAX];
    Fish_s alone[FISH_MAX];

    // (swarmed_size + alone_size) <= FISH_MAX
    size_t swarmed_size;
    size_t alone_size;

    int last_catched_idx;
} ;
extern struct FishGlobals_s fish;

void fish_init();

void fish_update(float dtime);

void fish_render();

vec2 fish_swarm_center();

void fish_catch_alone(int idx);


#endif //GMTKJAM21_FISH_H
