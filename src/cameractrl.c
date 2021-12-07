#include "mathc/float.h"
#include "cameractrl.h"


//
// public
//

CameraCtrl_s cameractrl_new() {
    return (CameraCtrl_s) {0};
}


void cameractrl_update(CameraCtrl_s *self, Camera_s *cam, float dtime) {

    self->pos = vec2_mix(self->pos, self->in.dst, 1.0 * dtime);

    camera_set_pos(cam, self->pos.x, self->pos.y);
}

