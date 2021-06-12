#include "mathc/float.h"
#include "camera.h"
#include "cameractrl.h"


struct CameraControlGlobals_s cameractrl;


//
// public
//

void cameractrl_init() {
    
}

void cameractrl_kill() {

}

void cameractrl_update(float dtime) {
    
    cameractrl.pos = vec2_mix(cameractrl.pos, cameractrl.in.dst, 1.0*dtime);

    camera_set_pos(cameractrl.pos.x, cameractrl.pos.y);
}

