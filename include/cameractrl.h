#ifndef GMTKJAM21_CAMERACTRL_H
#define GMTKJAM21_CAMERACTRL_H

#include "mathc/types/float.h"

struct CameraControlGlobals_s {
    vec2 pos;

    struct {
        vec2 dst;
    } in;
};
extern struct CameraControlGlobals_s cameractrl;

void cameractrl_init();

void cameractrl_kill();

void cameractrl_update(float dtime);

#endif //GMTKJAM21_CAMERACTRL_H
