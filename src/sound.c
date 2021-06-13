#include <SDL_mixer.h>
#include "rhc/log.h"
#include "sound.h"

static struct {
    Mix_Chunk *feed;
} L;

void sound_init() {
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
        log_warn("sound not working");
        return;
    }

    L.feed = Mix_LoadWAV("res/sound_feed.wav");
    if (!L.feed) {
        log_warn("failed to load feed");
        return;
    }
}

void sound_play_feed() {
    if (Mix_PlayChannel(-1, L.feed, 0) == -1) {
        log_warn("failed to play");
        return;
    }
}
