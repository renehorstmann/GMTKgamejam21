#include <SDL2/SDL_mixer.h>
#include "rhc/log.h"
#include "rhc/alloc.h"
#include "mathc/sca/float.h"
#include "sound.h"

#define FEED_PER_SECOND 1.5

struct Sound {
    eInput *input_ref;

    Mix_Chunk *activate;
    Mix_Chunk *feed;
    Mix_Chunk *shark;
    Mix_Chunk *gameover;

    float feed_per_second;

    bool active;
};


static void init(Sound *self) {
    
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
        log_warn("sound not working");
        return;;
    }

    self->activate = Mix_LoadWAV("res/sound_activate.wav");
    if (!self->activate) {
        log_warn("failed to load activate");
        return;
    }

    self->feed = Mix_LoadWAV("res/sound_feed.wav");
    if (!self->feed) {
        log_warn("failed to load feed");
        return;
    }

    self->shark = Mix_LoadWAV("res/sound_shark.wav");
    if (!self->shark) {
        log_warn("failed to load shark");
        return;
    }

    self->gameover = Mix_LoadWAV("res/sound_gameover.wav");
    if (!self->gameover) {
        log_warn("failed to load gameover");
        return;
    }

    Mix_Music *bubbles = Mix_LoadMUS("res/sound_bubbles.ogg");
    if (!bubbles) {
        log_warn("failed to load bubbles: %s", Mix_GetError());
        return;
    }

    if (Mix_PlayMusic(bubbles, -1) == -1) {
        log_warn("failed to play");
        return;
    }

    log_info("sound activated");
    self->active = true;
}

static void pointer_event(ePointer_s pointer, void *user_data) {
    Sound *self = user_data;

    // wait for first user pointer action

    // init SDL_Mixer
    // on web, sound will be muted, if created before an user action....
    init(self);
    
    e_input_unregister_pointer_event(self->input_ref, pointer_event);
}


Sound *sound_new(eInput *input) {
    Sound *self = rhc_calloc(sizeof *self);
    self->input_ref = input;

    e_input_register_pointer_event(input, pointer_event, self);

    return self;
}

void sound_update(Sound *self, float dtime) {
    if (!self->active)
        return;
    self->feed_per_second = sca_max(0, self->feed_per_second - dtime * FEED_PER_SECOND);
}

void sound_play_activate(Sound *self) {
    if (!self->active)
        return;
    if (Mix_PlayChannel(-1, self->activate, 0) == -1) {
        log_warn("failed to play");
        return;
    }
}

void sound_play_feed(Sound *self) {
    if (!self->active)
        return;
    if (self->feed_per_second > FEED_PER_SECOND)
        return;
    self->feed_per_second += 1;
    if (Mix_PlayChannel(-1, self->feed, 0) == -1) {
        log_warn("failed to play");
        return;
    }
}

void sound_play_shark(Sound *self) {
    if (!self->active)
        return;
    if (Mix_PlayChannel(-1, self->shark, 0) == -1) {
        log_warn("failed to play");
        return;
    }
}

void sound_play_gameover(Sound *self) {
    if (!self->active)
        return;
    if (Mix_PlayChannel(-1, self->gameover, 0) == -1) {
        log_warn("failed to play");
        return;
    }
}
