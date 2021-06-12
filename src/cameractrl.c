#include "mathc/float.h"
#include "camera.h"
#include "cameractrl.h"


struct CameraControlGlobals_s cameractrl;

//
// private
//

static struct {
    vec2 pos;
} L;

static void check_limits() {

}

static void apply_pos(float dtime) {
    L.pos = vec2_add_vec(cameractrl.pos, camera_center_offset());
}


//
// public
//

void cameractrl_init() {
    cameractrl.max_diff = vec2_set(50);
    cameractrl.diff_offset = (vec2) {{0, 0}};
}

void cameractrl_kill() {

}

void cameractrl_update(float dtime) {
//	vec2_println(cameractrl.pos);
    vec2 delta = vec2_sub_vec(cameractrl.in.dst, cameractrl.pos);

    float max_right = cameractrl.diff_offset.x + cameractrl.max_diff.x;
    float max_left = cameractrl.diff_offset.x - cameractrl.max_diff.x;
    float max_top = cameractrl.diff_offset.x + cameractrl.max_diff.y;
    float max_bottom = cameractrl.diff_offset.x - cameractrl.max_diff.y;
    
    if (delta.x > max_right) {
        delta.x -= max_right;
        cameractrl.pos.x += delta.x;
    } else if(delta.x < max_left) {
        delta.x -= max_left;
        cameractrl.pos.x += delta.x;
    }
    
    if (delta.y > max_top) {
        delta.y -= max_top;
        cameractrl.pos.y += delta.y;
    } else if (delta.y < max_bottom) {
        delta.y -= max_bottom;
        cameractrl.pos.y += delta.y;
    }

    apply_pos(dtime);

    check_limits();
    camera_set_pos(L.pos.x, L.pos.y);
}

