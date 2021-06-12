#include <time.h>   //srand(time(NULL))

#include "u/pose.h"
#include "rhc/log.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "mathc/utils/color.h"
#include "fish.h"
#include "feed.h"
#include "camera.h"
#include "cameractrl.h"
#include "spawn.h"

#define RESPAWN_FISH_TIME 5.0
#define RESPAWN_FISH_SPEED 20.0

static struct {
   float time;
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
                        sca_random_range(0, 360),
                        sca_random_range(0.75, 1.0),
                        sca_random_range(0.5, 1.0)
                }};
    vec4 color;
    color.rgb = vec3_hsv2rgb(hsv);
    color.a = 1.0;
    return color;
}

static vec2 random_euler_pos(float min, float max) {
    float angle = sca_random_range(0, 2 * M_PI);
    float distance = sca_random_range(min, max);
    return vec2_scale((vec2) {{sca_cos(angle), sca_sin(angle)}}, distance);
}

static vec2 random_offscreen_pos(float range) {
    float x = sca_random_range(camera_width()/2, camera_width()/2+range);
    float y = sca_random_range(camera_height()/2, camera_height()/2+range);
    if(rand()%2==0)
        x*=-1;
    if(rand()%2==0)
        y*=-1;
    return (vec2) {{x, y}};
}

static Fish_s new_fish(vec2 pos) {
    return (Fish_s) {
            .pos = pos,
            .hsv = hsv_fish(),
            .L.animate_time = sca_random(),
            .L.looking_left = rand() % 2 == 0
    };
}

static Feed_s new_feed(vec2 pos) {
    return (Feed_s) {
        .pos = pos,
        .color = color_feed()
    };
}

static void respawn_fish() {
    log_info("respawn_fish");
    mat4 border = u_pose_new(cameractrl.pos.x, cameractrl.pos.y, camera_width()+32, camera_height()+32);
    for(int i=0; i<fish.alone_size; i++) {
        if(!u_pose_aa_contains(border, fish.alone[i].pos)) {
            log_info("respawn_fish -> found");
            vec2 new_pos = vec2_add_vec(cameractrl.pos,  random_offscreen_pos(64));
            vec2 speed = vec2_sub_vec(cameractrl.pos, new_pos);
            speed = vec2_normalize(speed);
            speed = vec2_scale(speed, RESPAWN_FISH_SPEED);
            fish.alone[i].pos = new_pos;
            fish.alone[i].speed = speed;
            fish.alone[i].set_speed = speed;
            return;
        }
    }
}

void spawn_init() {

    srand(time(NULL));

    fish.swarmed[0] = new_fish(vec2_set(0));
    fish.swarmed_size = 1;

    for (int i = 0; i < 3; i++) {
        fish.alone[i] = new_fish(random_euler_pos(96, CAMERA_SIZE/2));
    }
    for (int i = 3; i < FISH_MAX-1; i++) {
        fish.alone[i] = new_fish(random_euler_pos(CAMERA_SIZE*5/6, CAMERA_SIZE*2));
    }
    fish.alone_size = FISH_MAX-1;


    feed.feed[0] = new_feed(random_euler_pos(96, CAMERA_SIZE/2));
    for (int i = 1; i < FEED_MAX; i++) {
        feed.feed[i] = new_feed(random_euler_pos(CAMERA_SIZE*5/6, CAMERA_SIZE*2));
    }
    feed.feed_size = FEED_MAX;
}


void spawn_update(float dtime) {
    L.time += dtime;
    if(L.time > RESPAWN_FISH_TIME) {
        L.time -= RESPAWN_FISH_TIME;
        respawn_fish();
    }
}