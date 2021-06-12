#include "r/ro_batch.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "mathc/utils/color.h"
#include "camera.h"
#include "feed.h"


struct FeedGlobals_s feed;

static struct {
    RoBatch ro;

} L;

void feed_init() {
    L.ro = ro_batch_new(FEED_MAX, camera.gl_main, r_texture_new_file(1, 4, "res/food.png"));

    for (int i = 0; i < FEED_MAX; i++) {
        L.ro.rects[i].pose = u_pose_new_hidden();

        L.ro.rects[i].uv = u_pose_new(0, 0,
                                      rand() % 2 == 0 ? -1 : 1,
                                      rand() % 2 == 0 ? -1 : 1);
        L.ro.rects[i].sprite.y = rand() % 4;
    }

    ro_batch_update(&L.ro);
}

void feed_update(float dtime) {
    for (int i = 0; i < feed.feed_size; i++) {
        vec2 delta = vec2_scale(feed.feed[i].speed, dtime);
        feed.feed[i].pos = vec2_add_vec(feed.feed[i].pos, delta);
        L.ro.rects[i].pose = u_pose_new(feed.feed[i].pos.x, feed.feed[i].pos.y, 16, 16);
    }

    ro_batch_update(&L.ro);
}

void feed_render() {
    ro_batch_render(&L.ro);
}
