#ifndef GMTKJAM21_SPAWN_H
#define GMTKJAM21_SPAWN_H


#include "camera.h"
#include "cameractrl.h"
#include "fish.h"
#include "feed.h"
#include "shark.h"

typedef struct {
    const Camera_s *cam_ref;
    CameraCtrl_s *camctrl_ref;

    Fish *fish_ref;
    Feed *feed_ref;
    Shark *shark_ref;

    struct {
        float fish_time;
        float feed_time;
        float shark_time;
        float game_time;
    } L;
} Spawn;

Spawn *spawn_new(const Camera_s *cam, CameraCtrl_s *camctrl, Fish *fish, Feed *feed, Shark *shark);

void spawn_kill(Spawn **self_ptr);

void spawn_update(Spawn *self, float dtime);

#endif //GMTKJAM21_SPAWN_H
