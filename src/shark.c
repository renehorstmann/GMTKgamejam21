#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "rhc/alloc.h"

#include "shark.h"


#define FRAMES 4
#define FPS 6.0
#define MOUTH_TIME 0.25

#define NUM_PARTICLES 64
#define PARTICLE_SIZE 2.0
#define PARTICLE_SPEED 100.0
#define PARTICLE_TIME 1.0
#define PARTICLE_ALPHA 1.5


static void emit_particles(Shark *self, float x, float y, vec3 color) {
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
        rects[i].start_time = self->particles_ref->time;
    }
    pixelparticles_add(self->particles_ref, rects, NUM_PARTICLES);
}

static void update_shark(Shark *self, int idx, float dtime) {
    Shark_s *sh = &self->shark[idx];

    vec2 delta = vec2_scale(sh->speed, dtime);
    sh->pos = vec2_add_vec(sh->pos, delta);

    sh->L.animate_time = sca_mod(sh->L.animate_time + dtime, FRAMES / FPS);
    int frame = sh->L.animate_time * FPS;

    self->L.ro.rects[idx].pose = u_pose_new(sh->pos.x, sh->pos.y, 64, 32);
    self->L.ro.rects[idx].uv = u_pose_new(0, 0, sh->speed.x>0 ? 1 : -1, 1);
    self->L.ro.rects[idx].color = sh->color;

    float sprite_y = 0;
    if(sh->L.mouth_time>0) {
        sh->L.mouth_time -= dtime;
        sprite_y = 1;
    }

    self->L.ro.rects[idx].sprite.x = frame;
    self->L.ro.rects[idx].sprite.y = sprite_y;
}

static void check_eat_fish(Shark *self, Shark_s *sh) {
    vec2 mouth = sh->pos;
    mouth.x += sh->speed.x<0? -16 : 16;
    for(int i=0; i<self->fish_ref->swarmed_size; i++) {
        if(vec2_distance(mouth, self->fish_ref->swarmed[i].pos) < 16) {
            emit_particles(self, 
                    self->fish_ref->swarmed[i].pos.x, 
                    self->fish_ref->swarmed[i].pos.y, 
                    (vec3) {{0.8, 0.2, 0.2}});
            fish_eat(self->fish_ref, i, true);
            sound_play_shark(self->sound_ref);
            sh->L.mouth_time = MOUTH_TIME;
            return;
        }
    }
    for(int i=0; i<self->fish_ref->alone_size; i++) {
        if(vec2_distance(mouth, self->fish_ref->alone[i].pos) < 16) {
            emit_particles(self, 
                    self->fish_ref->alone[i].pos.x, 
                    self->fish_ref->alone[i].pos.y, 
                    (vec3) {{0.4, 0.2, 0.2}});
            fish_eat(self->fish_ref, i, false);
            sound_play_shark(self->sound_ref);
            sh->L.mouth_time = MOUTH_TIME;
            return;
        }
    }
}

Shark *shark_new(Sound *sound, PixelParticles *particles, Fish *fish) {
    Shark *self = rhc_calloc(sizeof *self);
    
    self->sound_ref = sound;
    self->particles_ref = particles;
    self->fish_ref = fish;
    
    self->L.ro = ro_batch_new(SHARK_MAX, r_texture_new_file(4, 2, "res/shark.png"));

    for(int i=0; i<SHARK_MAX; i++) {
        self->L.ro.rects[i].pose = u_pose_new_hidden();
    }

    ro_batch_update(&self->L.ro);
    
    return self;
}

void shark_kill(Shark **self_ptr) {
    Shark *self = *self_ptr;
    if(!self)
        return;
    ro_batch_kill(&self->L.ro);
    rhc_free(self);
    *self_ptr = NULL;
}

void shark_update(Shark *self, float dtime) {
    for(int i=0; i<self->shark_size; i++) {
        update_shark(self, i, dtime);
        check_eat_fish(self, &self->shark[i]);
    }
    for(int i=self->shark_size; i<SHARK_MAX; i++) {
        self->L.ro.rects[i].pose = u_pose_new_hidden();
    }
    ro_batch_update(&self->L.ro);
}

void shark_render(const Shark *self, const mat4 *cam_mat) {
    ro_batch_render(&self->L.ro, cam_mat);
}

