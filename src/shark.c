#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "pixelparticles.h"
#include "fish.h"
#include "camera.h"
#include "sound.h"
#include "shark.h"


#define FRAMES 4
#define FPS 6.0
#define MOUTH_TIME 0.25

#define NUM_PARTICLES 64
#define PARTICLE_SIZE 2.0
#define PARTICLE_SPEED 100.0
#define PARTICLE_TIME 1.0
#define PARTICLE_ALPHA 1.5

struct SharkGlobals_s shark;

static struct {
    RoBatch ro;
} L;

static void emit_particles(float x, float y, vec3 color) {
    rParticleRect_s rects[NUM_PARTICLES];
    for (int i = 0; i < NUM_PARTICLES; i++) {
        rects[i] = r_particlerect_new();
        rects[i].pose = u_pose_new(x, y, PARTICLE_SIZE, PARTICLE_SIZE);
        float angle = sca_random_range(0, 2 * M_PI);
        float speed = sca_random_range(0.1 * PARTICLE_SPEED, PARTICLE_SPEED);
        rects[i].speed.x = sca_cos(angle) * speed;
        rects[i].speed.y = sca_sin(angle) * speed;
        rects[i].acc.xy = vec2_scale(rects[i].speed.xy, -0.5 / PARTICLE_TIME);
        rects[i].color.rgb = vec3_random_noise_vec(
                color,
                (vec3) {{0.2, 0.2, 0.2}});
        rects[i].color.a = PARTICLE_ALPHA;
        rects[i].color_speed.a = (float) -PARTICLE_ALPHA / PARTICLE_TIME;
        rects[i].start_time = pixelparticles.time;
    }
    pixelparticles_add(rects, NUM_PARTICLES);
}

static void update_shark(int idx, float dtime) {
    Shark_s *self = &shark.shark[idx];

    vec2 delta = vec2_scale(self->speed, dtime);
    self->pos = vec2_add_vec(self->pos, delta);

    self->L.animate_time = sca_mod(self->L.animate_time + dtime, FRAMES / FPS);
    int frame = self->L.animate_time * FPS;

    L.ro.rects[idx].pose = u_pose_new(self->pos.x, self->pos.y, 64, 32);
    L.ro.rects[idx].uv = u_pose_new(0, 0, self->speed.x>0 ? 1 : -1, 1);
    L.ro.rects[idx].color = self->color;

    float sprite_y = 0;
    if(self->L.mouth_time>0) {
        self->L.mouth_time -= dtime;
        sprite_y = 1;
    }

    L.ro.rects[idx].sprite.x = frame;
    L.ro.rects[idx].sprite.y = sprite_y;
}

static void check_eat_fish(Shark_s *self) {
    vec2 mouth = self->pos;
    mouth.x += self->speed.x<0? -16 : 16;
    for(int i=0; i<fish.swarmed_size; i++) {
        if(vec2_distance(mouth, fish.swarmed[i].pos) < 16) {
            emit_particles(fish.swarmed[i].pos.x, fish.swarmed[i].pos.y, (vec3) {{0.8, 0.2, 0.2}});
            fish_eat(i, true);
            sound_play_shark();
            self->L.mouth_time = MOUTH_TIME;
            return;
        }
    }
    for(int i=0; i<fish.alone_size; i++) {
        if(vec2_distance(mouth, fish.alone[i].pos) < 16) {
            emit_particles(fish.alone[i].pos.x, fish.alone[i].pos.y, (vec3) {{0.4, 0.2, 0.2}});
            fish_eat(i, false);
            sound_play_shark();
            self->L.mouth_time = MOUTH_TIME;
            return;
        }
    }
}

void shark_init() {
    L.ro = ro_batch_new(SHARK_MAX, r_texture_new_file(4, 2, "res/shark.png"));

    for(int i=0; i<SHARK_MAX; i++) {
        L.ro.rects[i].pose = u_pose_new_hidden();
    }

    ro_batch_update(&L.ro);
}

void shark_kill() {
    ro_batch_kill(&L.ro);
    memset(&L, 0, sizeof(L));
    memset(&shark, 0, sizeof(shark));
}

void shark_update(float dtime) {
    for(int i=0; i<shark.shark_size; i++) {
        update_shark(i, dtime);
        check_eat_fish(&shark.shark[i]);
    }
    for(int i=shark.shark_size; i<SHARK_MAX; i++) {
        L.ro.rects[i].pose = u_pose_new_hidden();
    }
    ro_batch_update(&L.ro);
}

void shark_render() {
    ro_batch_render(&L.ro, (const mat4*) camera.gl_main);
}

