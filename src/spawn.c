#include <time.h>   //srand(fish_time(NULL))

#include "u/pose.h"
#include "rhc/log.h"
#include "rhc/alloc.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "mathc/utils/color.h"
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

static vec4 random_spawn_pos_dir(const Camera_s *cam, vec2 center, float offset) {
    vec2 pos;

    if(rand()%2==0) {
        // left or right
        pos.x = rand()%2==0? cam->RO.left-offset : cam->RO.right+offset;
        pos.y = sca_random_range(cam->RO.bottom, cam->RO.top);
    } else {
        // top or bottom
        pos.x = sca_random_range(cam->RO.left, cam->RO.right);
        pos.y = rand()%2==0? cam->RO.bottom-offset : cam->RO.top+offset;
    }

    vec2 dst = {{
                        sca_random_range(cam->RO.left+offset, cam->RO.right-offset),
                        sca_random_range(cam->RO.bottom+offset, cam->RO.top-offset),
    }};

    vec2 dir = vec2_normalize(vec2_sub_vec(dst, pos));

    pos = vec2_add_vec(pos, center);

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

static FeedItem_s new_feed(vec2 pos) {
    return (FeedItem_s) {
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

static void respawn_fish(Spawn *self) {
    log_info("respawn_fish");
    mat4 border = u_pose_new(
            self->camctrl_ref->pos.x, 
            self->camctrl_ref->pos.y,
             camera_width(self->cam_ref) + 32,
             camera_height(self->cam_ref) + 32);
    static int last = 0;
    for (int i = 0; i < self->fish_ref->alone_size; i++) {
        int idx = (i + last) % self->fish_ref->alone_size;
        if (!u_pose_aa_contains(border, self->fish_ref->alone[idx].pos)) {
            log_info("respawn_fish -> found");
            vec4 pos_dir = random_spawn_pos_dir(self->cam_ref, self->camctrl_ref->pos, 16);
            vec2 new_pos = pos_dir.xy;
            vec2 speed = pos_dir.zw;
            speed = vec2_scale(speed, RESPAWN_FISH_SPEED);
            self->fish_ref->alone[idx].pos = new_pos;
            self->fish_ref->alone[idx].speed = speed;
            self->fish_ref->alone[idx].set_speed = speed;
            last = idx;
            return;
        }
    }
}


static void renew_feed(Spawn *self, FeedItem_s *feed) {
    log_info("renew_feed");
    vec4 pos_dir = random_spawn_pos_dir(self->cam_ref, self->camctrl_ref->pos, 8);
    vec2 new_pos = pos_dir.xy;
    vec2 speed = pos_dir.zw;
    speed = vec2_scale(speed, RESPAWN_FEED_SPEED);
    feed->pos = new_pos;
    feed->speed = speed;
    feed->size = sca_random_range(FEED_SIZE_MIN, FEED_SIZE_MAX);
}

static void respawn_feed(Spawn *self) {
    log_info("respawn_feed");
    mat4 border = u_pose_new(
    self->camctrl_ref->pos.x, 
    self->camctrl_ref->pos.y, 
    camera_width(self->cam_ref) + 16, 
    camera_height(self->cam_ref) + 16);
    static int last;
    for (int i = 0; i < self->feed_ref->feed_size; i++) {
        int idx = (i + last) % self->feed_ref->feed_size;
        if (!u_pose_aa_contains(border, self->feed_ref->feed[idx].pos)) {
            renew_feed(self, &self->feed_ref->feed[idx]);
            last = idx;
            return;
        }
    }
}

static void renew_shark(Spawn *self, Shark_s *shark) {
    log_info("renew_shark");
    vec4 pos_dir = random_spawn_pos_dir(self->cam_ref, self->camctrl_ref->pos, 32);
    vec2 new_pos = pos_dir.xy;
    vec2 speed = pos_dir.zw;
    float speed_value = sca_mix(SHARK_START_SPEED_START, SHARK_START_SPEED_END, self->L.game_time / SHARK_DURATION);
    speed_value = sca_min(speed_value, SHARK_MAX_SPEED);
    speed = vec2_scale(speed, speed_value);
    shark->pos = new_pos;
    shark->speed = speed;
}

static void respawn_shark(Spawn *self) {
    log_info("respawn_shark");
    mat4 border = u_pose_new(
    self->camctrl_ref->pos.x, 
    self->camctrl_ref->pos.y, 
    camera_width(self->cam_ref) + 64, 
    camera_height(self->cam_ref) + 32);
    static int last;
    for (int i = 0; i < self->shark_ref->shark_size; i++) {
        int idx = (i + last) % self->shark_ref->shark_size;
        if (!u_pose_aa_contains(border, self->shark_ref->shark[idx].pos)) {
            renew_shark(self, &self->shark_ref->shark[idx]);
            last = idx;
            return;
        }
    }
}


//
// public
//

Spawn *spawn_new(const Camera_s *cam, CameraCtrl_s *camctrl, Fish *fish, Feed *feed, Shark *shark) {
    Spawn *self = rhc_calloc(sizeof *self);
    
    self->cam_ref = cam;
    self->camctrl_ref = camctrl;
    self->fish_ref = fish;
    self->feed_ref = feed;
    self->shark_ref = shark;
    
    srand(time(NULL));

    self->fish_ref->swarmed_size = START_FISHS;
    self->fish_ref->alone_size = FISH_MAX - START_FISHS;

    for(int i=0; i<START_FISHS; i++) {
        self->fish_ref->swarmed[i] = new_fish(random_euler_pos(16, 32));
    }
    for (int i = 0; i < START_NEAR_FISHS; i++) {
        self->fish_ref->alone[i] = new_fish(random_euler_pos(96, CAMERA_SIZE / 2));
    }
    for (int i = START_NEAR_FISHS; i < self->fish_ref->alone_size; i++) {
        self->fish_ref->alone[i] = new_fish(random_euler_pos(CAMERA_SIZE * 5 / 6, CAMERA_SIZE * 2));
    }


    self->feed_ref->feed[0] = new_feed(random_euler_pos(96, CAMERA_SIZE / 2));
    for (int i = 1; i < FEED_MAX; i++) {
        self->feed_ref->feed[i] = new_feed(vec2_set(FLT_MAX));
    }
    self->feed_ref->feed_size = FEED_MAX;

    for(int i=0; i<SHARK_MAX; i++) {
        self->shark_ref->shark[i] = new_shark(vec2_set(-FLT_MAX)); // to not eat fishis set are set to FLT_MAX
    }
    self->shark_ref->shark_size = SHARK_MAX;
    self->L.shark_time = RESPAWN_SHARK_TIME_START * 0.75;
    self->L.game_time = 0;
    
    return self;
}

void spawn_kill(Spawn **self_ptr) {
    rhc_free(*self_ptr);
    *self_ptr = NULL;
}


void spawn_update(Spawn *self, float dtime) {
    self->L.game_time += dtime;
    
    self->L.fish_time += dtime;
    if (self->L.fish_time > RESPAWN_FISH_TIME) {
        self->L.fish_time -= RESPAWN_FISH_TIME;
        respawn_fish(self);
    }

    self->L.feed_time += dtime;
    if (self->L.feed_time > RESPAWN_FEED_TIME) {
        self->L.feed_time -= RESPAWN_FEED_TIME;
        respawn_feed(self);
    }

    for (int i = 0; i < self->feed_ref->feed_size; i++) {
        if (self->feed_ref->feed[i].size <= 0) {
            renew_feed(self, &self->feed_ref->feed[i]);
        }
    }

    self->L.shark_time += dtime;
    float respawn_time = sca_mix(RESPAWN_SHARK_TIME_START, RESPAWN_SHARK_TIME_END,
                                 self->L.game_time / SHARK_DURATION);
    respawn_time = sca_max(respawn_time, RESPAWN_SHARK_TIME_MIN);
    if (self->L.shark_time > respawn_time) {
        self->L.shark_time -= respawn_time;
        respawn_shark(self);
    }
}
