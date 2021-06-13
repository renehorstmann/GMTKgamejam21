#include "r/ro_batch.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "mathc/utils/color.h"
#include "camera.h"
#include "pixelparticles.h"
#include "hud.h"
#include "feed.h"

#define EAT_SIZE_TIME 1.0

#define NUM_PARTICLES 12
#define PARTICLE_SIZE 2.0
#define PARTICLE_SPEED 100.0
#define PARTICLE_TIME 1.0
#define PARTICLE_ALPHA 1.5

struct FeedGlobals_s feed;

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

void feed_kill() {
    ro_batch_kill(&L.ro);
    memset(&L, 0, sizeof(L));
    memset(&feed, 0, sizeof(feed));
}

void feed_update(float dtime) {
    for (int i = 0; i < feed.feed_size; i++) {
        vec2 delta = vec2_scale(feed.feed[i].speed, dtime);
        feed.feed[i].pos = vec2_add_vec(feed.feed[i].pos, delta);
        L.ro.rects[i].pose = u_pose_new(feed.feed[i].pos.x, feed.feed[i].pos.y, 16, 16);
        L.ro.rects[i].color = feed.feed[i].color;
    }
    for(int i=feed.feed_size; i<FEED_MAX; i++) {
        L.ro.rects[i].pose = u_pose_new_hidden();
    }

    ro_batch_update(&L.ro);
}

void feed_render() {
    ro_batch_render(&L.ro);
}

void feed_eat(Feed_s *self, float time) {
    if(self->size <= 0)
        return;

    float prev_size = self->size;
    self->size -= EAT_SIZE_TIME * time;
    if(self->size<=0) {
        feed.eaten++;
        hud_score();
        for(int i=0; i<5; i++)
            emit_particles(self->pos.x, self->pos.y, self->color.rgb);
    }

    if(sca_mod(prev_size, 0.5) < sca_mod(self->size, 0.5)) {
        emit_particles(self->pos.x, self->pos.y, self->color.rgb);
    }
}
