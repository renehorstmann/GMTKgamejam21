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

#define RESPAWN_FEED_TIME 5.0
#define RESPAWN_FEED_SPEED 10.0

#define START_FISHS 3
#define START_NEAR_FISHS 2
#define FEED_SIZE_MIN 5
#define FEED_SIZE_MAX 10

#define SHARK_DURATION 120.0
#define RESPAWN_SHARK_TIME_START 10.0
#define RESPAWN_SHARK_TIME_END 2.0
#define RESPAWN_SHARK_TIME_MIN 0.5
#define SHARK_START_SPEED_START 30.0
#define SHARK_START_SPEED_END 60.0
#define SHARK_MAX_SPEED 120.0

static struct {
    float fish_time;
    float feed_time;
    float shark_time;
    float game_time;
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

static vec4 random_spawn_pos_dir(float offset) {
    vec2 pos;

    if(rand()%2==0) {
        // left or right
        pos.x = rand()%2==0? camera_left()-offset : camera_right()+offset;
        pos.y = sca_random_range(camera_bottom(), camera_top());
    } else {
        // top or bottom
        pos.x = sca_random_range(camera_left(), camera_right());
        pos.y = rand()%2==0? camera_bottom()-offset : camera_top()+offset;
    }

    vec2 dst = {{
                        sca_random_range(camera_left()+offset, camera_right()-offset),
                        sca_random_range(camera_bottom()+offset, camera_top()-offset),
    }};

    vec2 dir = vec2_normalize(vec2_sub_vec(dst, pos));

    pos = vec2_add_vec(pos, cameractrl.pos);

    vec4 pos_dir;
    pos_dir.xy = pos;
    pos_dir.zw = dir;
    return pos_dir;
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
    static int last = 0;
    for (int i = 0; i < fish.alone_size; i++) {
        int idx = (i + last) % fish.alone_size;
        if (!u_pose_aa_contains(border, fish.alone[idx].pos)) {
            log_info("respawn_fish -> found");
            vec4 pos_dir = random_spawn_pos_dir(16);
            vec2 new_pos = pos_dir.xy;
            vec2 speed = pos_dir.zw;
            speed = vec2_scale(speed, RESPAWN_FISH_SPEED);
            fish.alone[idx].pos = new_pos;
            fish.alone[idx].speed = speed;
            fish.alone[idx].set_speed = speed;
            last = idx;
            return;
        }
    }
}


static void renew_feed(Feed_s *self) {
    log_info("renew_feed");
    vec4 pos_dir = random_spawn_pos_dir(8);
    vec2 new_pos = pos_dir.xy;
    vec2 speed = pos_dir.zw;
    speed = vec2_scale(speed, RESPAWN_FEED_SPEED);
    self->pos = new_pos;
    self->speed = speed;
    self->size = sca_random_range(FEED_SIZE_MIN, FEED_SIZE_MAX);
}

static void respawn_feed() {
    log_info("respawn_feed");
    mat4 border = u_pose_new(cameractrl.pos.x, cameractrl.pos.y, camera_width() + 16, camera_height() + 16);
    static int last;
    for (int i = 0; i < feed.feed_size; i++) {
        int idx = (i + last) % feed.feed_size;
        if (!u_pose_aa_contains(border, feed.feed[idx].pos)) {
            renew_feed(&feed.feed[idx]);
            last = idx;
            return;
        }
    }
}

static void renew_shark(Shark_s *self) {
    log_info("renew_shark");
    vec4 pos_dir = random_spawn_pos_dir(32);
    vec2 new_pos = pos_dir.xy;
    vec2 speed = pos_dir.zw;
    float speed_value = sca_mix(SHARK_START_SPEED_START, SHARK_START_SPEED_END, L.game_time / SHARK_DURATION);
    speed_value = sca_min(speed_value, SHARK_MAX_SPEED);
    speed = vec2_scale(speed, speed_value);
    self->pos = new_pos;
    self->speed = speed;
}

static void respawn_shark() {
    log_info("respawn_shark");
    mat4 border = u_pose_new(cameractrl.pos.x, cameractrl.pos.y, camera_width() + 64, camera_height() + 32);
    static int last;
    for (int i = 0; i < shark.shark_size; i++) {
        int idx = (i + last) % shark.shark_size;
        if (!u_pose_aa_contains(border, shark.shark[idx].pos)) {
            renew_shark(&shark.shark[idx]);
            last = idx;
            return;
        }
    }
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
        feed.feed[i] = new_feed(vec2_set(FLT_MAX));
    }
    feed.feed_size = FEED_MAX;

    for(int i=0; i<SHARK_MAX; i++) {
        shark.shark[i] = new_shark(vec2_set(-FLT_MAX)); // to not eat fishis set are set to FLT_MAX
    }
    shark.shark_size = SHARK_MAX;
    L.shark_time = RESPAWN_SHARK_TIME_START * 0.75;
    L.game_time = 0;
}


void spawn_update(float dtime) {
    L.game_time += dtime;
    
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

    L.shark_time += dtime;
    float respawn_time = sca_mix(RESPAWN_SHARK_TIME_START, RESPAWN_SHARK_TIME_END,
                                 L.game_time / SHARK_DURATION);
    respawn_time = sca_max(respawn_time, RESPAWN_SHARK_TIME_MIN);
    if (L.shark_time > respawn_time) {
        L.shark_time -= respawn_time;
        respawn_shark();
    }
}
