#ifndef GMTKJAM21_FISH_H
#define GMTKJAM21_FISH_H

#include "e/input.h"
#include "camera.h"
#include "sound.h"
#include "pixelparticles.h"
#include "feed.h"

#define FISH_MAX 24

enum FishState {
    FISH_STATE_SWIM,
    FISH_STATE_EAT,
    FISH_NUM_STATES
};

// single fish
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

// module
typedef struct {
    eInput *input_ref;
    const Camera_s *camera_ref;
    Sound *sound_ref;
    PixelParticles *particles_ref;
    Feed *feed_ref;

    Fish_s swarmed[FISH_MAX];
    Fish_s alone[FISH_MAX];

    // (swarmed_size + alone_size) <= FISH_MAX
    size_t swarmed_size;
    size_t alone_size;

    int last_catched_idx;
    bool game_running;

    struct {
        RoBatch ro;

        vec2 swarm_center;

        struct {
            RoSingle ring_ro;
            int active;
            vec2 dst;
        } move;
    } L;
} Fish;

Fish *fish_new(eInput *input, const Camera_s *cam, Sound *sound, PixelParticles *particles, Feed *feed);

void fish_kill(Fish **self_ptr);

void fish_update(Fish *self, float dtime);

void fish_render(const Fish *self, const mat4 *cam_mat);

vec2 fish_swarm_center(const Fish *self);

void fish_catch_alone(Fish *self, int idx);

void fish_eat(Fish *self, int idx, bool swarmed);


#endif //GMTKJAM21_FISH_H
