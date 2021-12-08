#include "r/ro_text.h"
#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "rhc/error.h"
#include "rhc/log.h"
#include "rhc/alloc.h"
#include "mathc/sca/int.h"
#include "button.h"
#include "highscore.h"

#include "showscore.h"

#define LINE_LEN 32  // 1000:NAME16 SCORE
#define ROWS 8

#define SCORE_URL "https://rohl.svenhuis.de/api/swarm"

#ifndef SCORE_URL
#define SCORE_URL "http://127.0.0.1:1000/api/swarm"
#endif


static void fetch_score(ShowScore *self) {
    if(self->L.fetch) {
        log_error("showscore fetch failed, already fetching...");
        return;
    }
    String entry = highscore_entry_to_string(
            highscore_entry_new(self->name_ref, self->user_score)
            );
    self->L.fetch = u_fetch_new_post(SCORE_URL, entry.str);
    string_kill(&entry);
}

static void set_score(ShowScore *self) {
    log_info("showscore set_score page %i", self->L.page);
    Highscore *h = self->L.highscore;
    if(!h)
        return;
    int pages = 1 + h->entries_size / ROWS;
    assume(self->L.page>=0 && self->L.page<pages, "wtf");
    
    char text[ROWS*(LINE_LEN+1)+1];
    size_t used = 0;
    for(int i=0; i<ROWS; i++) {
        int e = self->L.page * ROWS + i;
        if(e >= h->entries_size)
            break;
        char line[LINE_LEN+1]; // + null
        snprintf(line, sizeof line, "%4i:%.*s %i\n", 
                e+1, HIGHSCORE_NAME_MAX_LENGTH, 
                h->entries[e].name, 
                h->entries[e].score);
        used += snprintf(text + used, sizeof text - used, "%s\n", line);
    }
    
    ro_text_set_text(&self->L.score, text);
}

ShowScore *showscore_new(const char *name, int score) {
    ShowScore *self = rhc_calloc(sizeof *self);
    
    self->name_ref = name;
    self->user_score = score;
    
    fetch_score(self);
    
    self->L.title = ro_text_new_font55(10);
    ro_text_set_text(&self->L.title, "Highscore:");
    
    self->L.score = ro_text_new_font85(LINE_LEN*ROWS);
    ro_text_set_text(&self->L.score, "loading...");
    
    self->L.btns = ro_batch_new(3, r_texture_new_file(2, 3, "res/highscore_btns.png"));
    
    return self;
}

void showscore_kill(ShowScore **self_ptr) {
    ShowScore *self = *self_ptr;
    if(!self)
        return;
    u_fetch_kill(&self->L.fetch);
    if(self->L.highscore) {
        highscore_kill(self->L.highscore);
        rhc_free(self->L.highscore);
    }
    rhc_free(self);
    *self_ptr = NULL;
}

void showscore_update(ShowScore *self, float dtime) {
    int code = 0;
    String res = u_fetch_check_response(&self->L.fetch, &code);
    if(string_valid(res)) {
        if(self->L.highscore)
            highscore_kill(self->L.highscore);
        else
            self->L.highscore = rhc_malloc(sizeof *self->L.highscore);
        *self->L.highscore = highscore_new_msg(res.str);
        self->L.page = 0;
        set_score(self);
    } else if(code) {
        if(self->L.highscore) {
            highscore_kill(self->L.highscore);
            rhc_free(self->L.highscore);
            self->L.highscore = NULL;
        }
        
        char error_text[64];
        snprintf(error_text, 64, "connection error: %i", code);
        ro_text_set_text(&self->L.score, error_text);
    }
    string_kill(&res);
    
    
    // set poses
    self->L.title.pose = u_pose_new_aa(self->in.pos.x, self->in.pos.y, 2, 2);
    self->L.score.pose = u_pose_new_aa(self->in.pos.x, self->in.pos.y - 12, 1, 1);
    
    
    Highscore *h = self->L.highscore;
    int pages = !h? 0 : 1 + h->entries_size / ROWS;
    
    
    if(!h && !self->L.fetch)
        self->L.btns.rects[0].pose = u_pose_new_aa(self->in.pos.x + 130, self->in.pos.y, 16, 16);
    else 
        self->L.btns.rects[0].pose = u_pose_new_hidden();
        
        
    if(h && self->L.page>0) 
        self->L.btns.rects[1].pose = u_pose_new_aa(self->in.pos.x + 130, self->in.pos.y, 16, 16);
    else
        self->L.btns.rects[1].pose = u_pose_new_hidden();
    
    if(h && self->L.page<pages-1) 
        self->L.btns.rects[2].pose = u_pose_new_aa(self->in.pos.x + 150, self->in.pos.y, 16, 16);
    else
        self->L.btns.rects[2].pose = u_pose_new_hidden();
        
    
    
    ro_batch_update(&self->L.btns);
    
}


void showscore_render(const ShowScore *self, const mat4 *hudcam_mat) {
    ro_text_render(&self->L.title, hudcam_mat);
    ro_text_render(&self->L.score, hudcam_mat);
    ro_batch_render(&self->L.btns, hudcam_mat);
}

void showscore_pointer(ShowScore *self, ePointer_s pointer) {
    
    // retry
    if(button_clicked(&self->L.btns.rects[0], pointer)) {
        fetch_score(self);
    }
    
    
    Highscore *h = self->L.highscore;
    if(!h)
        return;
    int pages = 1 + h->entries_size / ROWS;
    
    
    // page up (to the top, so --)
    if(button_clicked(&self->L.btns.rects[1], pointer)) {
        self->L.page = isca_max(0, self->L.page-1);
        set_score(self);
    }
    
    // page up (to the top, so --)
    if(button_clicked(&self->L.btns.rects[1], pointer)) {
        self->L.page = isca_min(pages-1, self->L.page+1);
        set_score(self);
    }
}
