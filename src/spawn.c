#include <time.h>   //srand(fish_time(NULL))

#include "u/pose.h"
#include "rhc/log.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "mathc/utils/color.h"
#include "fish.h"
#include "feed.h"
#include "shark.h"
#include "camera.h"
#include "cameractrl.h"
#include "spawn.h"

#define RESPAWN_FISH_TIME 10.0
#define RESPAWN_FISH_SPEED 20.0

#define RESPAWN_FEED_TIME 1.0
#define RESPAWN_FEED_SPEED 10.0

#define START_FISHS 3
#define START_NEAR_FISHS 2
#define FEED_SIZE_MIN 5
#define FEED_SIZE_MAX 10

static struct {
    float fish_time;
    float feed_time;
    float shark_time;
} L;

static vec3 hsv_fish() {
    return (vec3) {{
                           sca_random_range(0, 360),
                           0.5,
                           sca_random_range(0.75, 1.0)
                   }};
}

static vec4 color_feed() {
    vec3 hsv = {{
                        sca_random_noise(120, 20),
                        sca_random_range(0.75, 1.0),
                        sca_random_range(0.5, 1.0)
                }};
    vec4 color;
    color.rgb = vec3_hsv2rgb(hsv);
    color.a = 1.0;
    return color;
}

static vec4 color_shark() {
    vec4 color;
    color.rgb = vec3_random_range(0.7, 1.0);
    color.a = 1.0;
    return color;
}

static vec2 random_euler_pos(float min, float max) {
    float angle = sca_random_range(0, 2 * M_PI);
    float distance = sca_random_range(min, max);
    return vec2_scale((vec2) {{sca_cos(angle), sca_sin(angle)}}, distance);
}

static Fish_s new_fish(vec2 pos) {
    return (Fish_s) {
            .pos = pos,
            .hsv = hsv_fish(),
            .L.animate_time = sca_random_range(0, 4),
            .L.looking_left = rand() % 2 == 0
    };
}

static Feed_s new_feed(vec2 pos) {
    return (Feed_s) {
            .pos = pos,
            .color = color_feed(),
            .size = sca_random_range(FEED_SIZE_MIN, FEED_SIZE_MAX)
    };
}

static Shark_s new_shark(vec2 pos) {
    return (Shark_s) {
            .pos = pos,
            .color = color_shark(),
            .L.animate_time = sca_random_range(0, 4)
    };
}

static void respawn_fish() {
    log_info("respawn_fish");
    mat4 border = u_pose_new(cameractrl.pos.x, cameractrl.pos.y, camera_width() + 32, camera_height() + 32);
    float radius = camera_height()>camera_width()?camera_height() : camera_width();
    for (int i = 0; i < fish.alone_size; i++) {
        if (!u_pose_aa_contains(border, fish.alone[i].pos)) {
            log_info("respawn_fish -> found");
            vec2 new_pos = vec2_add_vec(cameractrl.pos, random_euler_pos(radius + 32, radius + 64));
            vec2 speed = vec2_sub_vec(cameractrl.pos, new_pos);
            speed = vec2_normalize(speed);
            speed = vec2_add_vec(speed, vec2_random_noise(0, 0.5));
            speed = vec2_normalize(speed);
            speed = vec2_scale(speed, RESPAWN_FISH_SPEED);
            fish.alone[i].pos = new_pos;
            fish.alone[i].speed = speed;
            fish.alone[i].set_speed = speed;
            return;
        }
    }
}


static void renew_feed(Feed_s *self) {
    log_info("renew_feed");
    float radius = camera_height()>camera_width()?camera_height() : camera_width();
    vec2 new_pos = vec2_add_vec(cameractrl.pos, random_euler_pos(radius + 32, radius + 64));
    vec2 speed = vec2_sub_vec(cameractrl.pos, new_pos);
    speed = vec2_normalize(speed);
    speed = vec2_add_vec(speed, vec2_random_noise(0, 0.5));
    speed = vec2_normalize(speed);
    speed = vec2_scale(speed, RESPAWN_FEED_SPEED);
    self->pos = new_pos;
    self->speed = speed;
    self->size = sca_random_range(FEED_SIZE_MIN, FEED_SIZE_MAX);
}

static void respawn_feed() {
    log_info("respawn_feed");
    mat4 border = u_pose_new(cameractrl.pos.x, cameractrl.pos.y, camera_width() + 16, camera_height() + 16);
    for (int i = 0; i < feed.feed_size; i++) {
        if (!u_pose_aa_contains(border, feed.feed[i].pos)) {
            renew_feed(&feed.feed[i]);
            return;
        }
    }
}

static void renew_shark(Shark_s *self) {
    log_info("renew_shark");
    float radius = camera_height()>camera_width()?camera_height() : camera_width();
    vec2 new_pos = vec2_add_vec(cameractrl.pos, random_euler_pos(radius + 64, radius + 128));
    vec2 speed = vec2_sub_vec(cameractrl.pos, new_pos);
    speed = vec2_normalize(speed);
//    speed = vec2_add_vec(speed, vec2_random_noise(0, 0.5));
    speed = vec2_normalize(speed);
    speed = vec2_scale(speed, 30.0);
    self->pos = new_pos;
    self->speed = speed;
}

void spawn_init() {

    srand(time(NULL));

    fish.swarmed_size = START_FISHS;
    fish.alone_size = FISH_MAX - START_FISHS;

    for(int i=0; i<START_FISHS; i++) {
        fish.swarmed[i] = new_fish(random_euler_pos(16, 32));
    }
    for (int i = 0; i < START_NEAR_FISHS; i++) {
        fish.alone[i] = new_fish(random_euler_pos(96, CAMERA_SIZE / 2));
    }
    for (int i = START_NEAR_FISHS; i < fish.alone_size; i++) {
        fish.alone[i] = new_fish(random_euler_pos(CAMERA_SIZE * 5 / 6, CAMERA_SIZE * 2));
    }


    feed.feed[0] = new_feed(random_euler_pos(96, CAMERA_SIZE / 2));
    for (int i = 1; i < FEED_MAX; i++) {
        feed.feed[i] = new_feed(random_euler_pos(CAMERA_SIZE * 5 / 6, CAMERA_SIZE * 2));
    }
    feed.feed_size = FEED_MAX;

    for(int i=0; i<SHARK_MAX; i++) {
        shark.shark[i] = new_shark(vec2_set(FLT_MAX));
    }
    shark.shark_size = SHARK_MAX;

}


void spawn_update(float dtime) {
    L.fish_time += dtime;
    if (L.fish_time > RESPAWN_FISH_TIME) {
        L.fish_time -= RESPAWN_FISH_TIME;
        respawn_fish();
    }

    L.feed_time += dtime;
    if (L.feed_time > RESPAWN_FEED_TIME) {
        L.feed_time -= RESPAWN_FEED_TIME;
        respawn_feed();
    }

    for (int i = 0; i < feed.feed_size; i++) {
        if (feed.feed[i].size <= 0) {
            renew_feed(&feed.feed[i]);
        }
    }

//    L.shark_time += dtime;
//    if (L.shark_time > 5.0) {
//        L.shark_time -= 5.0;
//        static int sh_idx = 0;
//        renew_shark(&shark.shark[sh_idx]);
//        sh_idx = (sh_idx+1) % SHARK_MAX;
//    }
}
