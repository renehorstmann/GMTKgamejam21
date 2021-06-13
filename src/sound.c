#include <SDL_mixer.h>
#include "rhc/log.h"
#include "sound.h"

static struct {
    Mix_Chunk *activate;
    Mix_Chunk *feed;
    Mix_Chunk *shark;
    Mix_Chunk *gameover;
} L;

void sound_init() {
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
        log_warn("sound not working");
        return;
    }

    L.activate = Mix_LoadWAV("res/sound_activate.wav");
    if (!L.activate) {
        log_warn("failed to load activate");
        return;
    }

    L.feed = Mix_LoadWAV("res/sound_feed.wav");
    if (!L.feed) {
        log_warn("failed to load feed");
        return;
    }

    L.shark = Mix_LoadWAV("res/sound_shark.wav");
    if (!L.shark) {
        log_warn("failed to load shark");
        return;
    }

    L.gameover = Mix_LoadWAV("res/sound_gameover.wav");
    if (!L.gameover) {
        log_warn("failed to load gameover");
        return;
    }

    Mix_Chunk *bubbles = Mix_LoadWAV("res/sound_bubbles.wav");
    if (bubbles) {
        log_warn("failed to load bubbles: %s", Mix_GetError());
        return;
    }

    if (Mix_PlayChannel(-1, bubbles, -1) == -1) {
        log_warn("failed to play");
        return;
    }
}

void sound_play_activate() {
    if (Mix_PlayChannel(-1, L.activate, 0) == -1) {
        log_warn("failed to play");
        return;
    }
}

void sound_play_feed() {
    if (Mix_PlayChannel(-1, L.feed, 0) == -1) {
        log_warn("failed to play");
        return;
    }
}

void sound_play_shark() {
    if (Mix_PlayChannel(-1, L.shark, 0) == -1) {
        log_warn("failed to play");
        return;
    }
}

void sound_play_gameover() {
    if (Mix_PlayChannel(-1, L.gameover, 0) == -1) {
        log_warn("failed to play");
        return;
    }
}
