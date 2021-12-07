#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "mathc/utils/color.h"
#include "rhc/alloc.h"
#include "feed.h"

#define EAT_SIZE_TIME 1.0

#define NUM_PARTICLES 12
#define PARTICLE_SIZE 2.0
#define PARTICLE_SPEED 100.0
#define PARTICLE_TIME 1.0
#define PARTICLE_ALPHA 1.5



static void emit_particles(Feed *self, float x, float y, vec3 color) {
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


//
// public
//

Feed *feed_new(Sound *sound, PixelParticles *particles) {
    Feed *self = rhc_calloc(sizeof *self);
    
    self->sound_ref = sound;
    self->particles_ref = particles;
    
    self->L.ro = ro_batch_new(FEED_MAX, r_texture_new_file(1, 4, "res/food.png"));

    for (int i = 0; i < FEED_MAX; i++) {
        self->L.ro.rects[i].pose = u_pose_new_hidden();

        self->L.ro.rects[i].uv = u_pose_new(0, 0,
                                      rand() % 2 == 0 ? -1 : 1,
                                      rand() % 2 == 0 ? -1 : 1);
        self->L.ro.rects[i].sprite.y = rand() % 4;
    }

    ro_batch_update(&self->L.ro);
    
    return self;
}

void feed_kill(Feed **self_ptr) {
    Feed *self = *self_ptr;
    if(!self)
        return;
    ro_batch_kill(&self->L.ro);
    rhc_free(self);
    *self_ptr = NULL;
}

void feed_update(Feed *self, float dtime) {
    for (int i = 0; i < self->feed_size; i++) {
        vec2 delta = vec2_scale(self->feed[i].speed, dtime);
        self->feed[i].pos = vec2_add_vec(self->feed[i].pos, delta);
        self->L.ro.rects[i].pose = u_pose_new(self->feed[i].pos.x, self->feed[i].pos.y, 16, 16);
        self->L.ro.rects[i].color = self->feed[i].color;
    }
    for(int i=self->feed_size; i<FEED_MAX; i++) {
        self->L.ro.rects[i].pose = u_pose_new_hidden();
    }

    ro_batch_update(&self->L.ro);
}

void feed_render(const Feed *self, const mat4 *cam_mat) {
    ro_batch_render(&self->L.ro, cam_mat);
}

void feed_eat(Feed *self, FeedItem_s *item, float time) {
    if(item->size <= 0)
        return;

    float prev_size = item->size;
    item->size -= EAT_SIZE_TIME * time;
    if(item->size<=0) {
        self->eaten++;
        self->out.score++;
        for(int i=0; i<5; i++)
            emit_particles(self, item->pos.x, item->pos.y, item->color.rgb);
    }

    if(sca_mod(prev_size, 0.5) < sca_mod(item->size, 0.5)) {
        emit_particles(self, item->pos.x, item->pos.y, item->color.rgb);
        sound_play_feed(self->sound_ref);
    }
}
