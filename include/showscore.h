#ifndef GMTKJAM21_SHOWSCORE_H
#define GMTKJAM21_SHOWSCORE_H

#include "e/input.h"
#include "r/ro_types.h"
#include "u/fetch.h"
#include "highscore.h"

typedef struct {
    const char *name_ref;
    int user_score;
    
    struct {
        vec2 pos; // as left, top
    } in;
    
    struct {
        uFetch *fetch;
        Highscore *highscore;
        int page;
        
        RoText title;
        RoText score;
        RoBatch btns;
    } L;
} ShowScore;


ShowScore *showscore_new(const char *name, int score);

void showscore_kill(ShowScore **self_ptr);

void showscore_update(ShowScore *self, float dtime);

void showscore_render(const ShowScore *self, const mat4 *hudcam_mat);

// pointer should be tranformed with the cam v_p_inv matrix
void showscore_pointer(ShowScore *self, ePointer_s pointer);

#endif //GMTKJAM21_SHOWSCORE_H
