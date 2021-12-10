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


#ifndef SHOWSCORE_URL
#define SHOWSCORE_URL "http://127.0.0.1:1000/api/swarm"
#endif


#define CAM_SIZE_FACTOR (240.0f/180.0f)


static const vec4 SCORE_COLOR = {{1, 1, 1, 0.8}};
static const vec4 SCORE_OWN_COLOR = {{0.0, 1.0, 0.5, 1.0}};
    

static void fetch_score(ShowScore *self) {
    if(self->L.fetch) {
        log_error("showscore fetch failed, already fetching...");
        return;
    }
    ro_text_set_text(&self->L.score, "loading...");
    
    String entry = highscore_entry_to_string(
            highscore_entry_new(self->name_ref, self->user_score)
            );
    self->L.fetch = u_fetch_new_post(SHOWSCORE_URL, entry.str);
    string_kill(&entry);
}

static void set_score(ShowScore *self) {
    log_info("showscore set_score page %i", self->L.page);
    Highscore *h = self->L.highscore;
    if(!h)
        return;
    int pages = 1 + (h->entries_size-1) / ROWS;
    assume(self->L.page>=0 && self->L.page<pages, "wtf");
    
    int own_text_begin = 0;
    int own_text_len = 0;
    
    char text[ROWS*(LINE_LEN+1)+1];
    size_t used = 0;
    for(int i=0; i<ROWS; i++) {
        int e = self->L.page * ROWS + i;
        if(e >= h->entries_size)
            break;
        char line[LINE_LEN+1]; // + null
        snprintf(line, sizeof line, "#%-2i %i %.*s", 
                e+1, 
                h->entries[e].score,
                HIGHSCORE_NAME_MAX_LENGTH, 
                h->entries[e].name);
        int line_len = snprintf(text + used, sizeof text - used, "%s\n", line);
        used += line_len;
        
        if(strcmp(self->name_ref, h->entries[e].name) == 0) {
            own_text_len = line_len;
            
        } else if(own_text_len == 0) {
            own_text_begin += line_len;
        }
    }
    
    ro_text_set_text(&self->L.score, text);
    
    if(own_text_len==0) {
        ro_text_set_color(&self->L.score, SCORE_COLOR);
    } else {
        for(int i=0; i<self->L.score.ro.num; i++) {
            vec4 col = i<own_text_begin || i>=own_text_begin+own_text_len ? 
                    SCORE_COLOR : SCORE_OWN_COLOR;
            self->L.score.ro.rects[i].color = col;
        }
        ro_batch_update(&self->L.score.ro);
    }
}

ShowScore *showscore_new(const char *name, int score) {
    log_info("showscore_new, URL=<%s>", SHOWSCORE_URL);

    ShowScore *self = rhc_calloc(sizeof *self);
    
    self->name_ref = name;
    self->user_score = score;
    
    self->L.title = ro_text_new_font55(10);
    ro_text_set_text(&self->L.title, "Haiscore:");
    ro_text_set_color(&self->L.title, (vec4) {{0, 0, 0, 0.5}});
    
    self->L.score = ro_text_new_font85(LINE_LEN*ROWS);
    self->L.score.offset.y = 10;
    ro_text_set_color(&self->L.score, SCORE_COLOR);
    
    self->L.btns = ro_batch_new(3, r_texture_new_file(2, 3, "res/showscore_btns.png"));
    for(int i=0; i<3; i++)
        self->L.btns.rects[i].sprite.y = i;
    
    fetch_score(self);
    
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
    bool error = 0;
    String res = u_fetch_check_response(&self->L.fetch, &error);
    if(string_valid(res)) {
        if(self->L.highscore)
            highscore_kill(self->L.highscore);
        else
            self->L.highscore = rhc_malloc(sizeof *self->L.highscore);
        *self->L.highscore = highscore_new_msg(res.str);
        
        // find page
        self->L.page = 0;
        for(int i=0; i<self->L.highscore->entries_size; i++) {
            if(strcmp(self->name_ref, 
                    self->L.highscore->entries[i].name
                    ) == 0) {
                self->L.page = i / ROWS;
                break;
            }
        }
        
        set_score(self);
    } else if(error) {
        if(self->L.highscore) {
            highscore_kill(self->L.highscore);
            rhc_free(self->L.highscore);
            self->L.highscore = NULL;
        }

        ro_text_set_text(&self->L.score, "connection error :(");
        
        // test 
        /*
        Highscore *h = rhc_calloc(sizeof *h);
        h->entries_size = 256;
        h->entries = rhc_calloc(sizeof *h->entries * h->entries_size);
        for(int i=0; i<h->entries_size; i++) {
            h->entries[i].score = i*1100;
            if(i == 107) {
                strcpy(h->entries[i].name, self->name_ref);
                continue;
            }
            char name[32];
            sprintf(name, "Test %i", i);
            strcpy(h->entries[i].name, name);
        }
        self->L.highscore = h;
        // find page
        self->L.page = 0;
        for(int i=0; i<self->L.highscore->entries_size; i++) {
            if(strcmp(self->name_ref, 
                    self->L.highscore->entries[i].name
                    ) == 0) {
                self->L.page = i / ROWS;
                break;
            }
        }
        set_score(self);
        */
        
    }
    string_kill(&res);
    
    
    // set poses
    self->L.title.pose = u_pose_new_aa(self->in.pos.x, self->in.pos.y, 
            2, 2);
    self->L.score.pose = u_pose_new_aa(self->in.pos.x, self->in.pos.y - 18, 
            1*CAM_SIZE_FACTOR, 1*CAM_SIZE_FACTOR);
    
    
    Highscore *h = self->L.highscore;
    int pages = !h? 0 : 1 + (h->entries_size-1) / ROWS;
    
    
    if(!h && !self->L.fetch)
        self->L.btns.rects[0].pose = u_pose_new_aa(self->in.pos.x + 140, self->in.pos.y, 
                32, 16);
    else 
        self->L.btns.rects[0].pose = u_pose_new_hidden();
        
        
    if(h && self->L.page>0) 
        self->L.btns.rects[1].pose = u_pose_new_aa(self->in.pos.x + 140, self->in.pos.y,
                32, 16);
    else
        self->L.btns.rects[1].pose = u_pose_new_hidden();
    
    if(h && self->L.page<pages-1) 
        self->L.btns.rects[2].pose = u_pose_new_aa(self->in.pos.x + 180, self->in.pos.y, 
                32, 16);
    else
        self->L.btns.rects[2].pose = u_pose_new_hidden();
        
}


void showscore_render(const ShowScore *self, const mat4 *hudcam_mat) {
    ro_text_render(&self->L.title, hudcam_mat);
    ro_text_render(&self->L.score, hudcam_mat);
    ro_batch_render(&self->L.btns, hudcam_mat, true);
}

void showscore_pointer(ShowScore *self, ePointer_s pointer) {
    
    // retry
    if(button_clicked(&self->L.btns.rects[0], pointer)) {
        fetch_score(self);
    }
    
    
    Highscore *h = self->L.highscore;
    if(!h)
        return;
    int pages = 1 + (h->entries_size-1) / ROWS;
    
    
    // page up (to the top, so --)
    if(button_clicked(&self->L.btns.rects[1], pointer)) {
        self->L.page = isca_max(0, self->L.page-1);
        set_score(self);
    }
    
    // page down
    if(button_clicked(&self->L.btns.rects[2], pointer)) {
        self->L.page = isca_min(pages-1, self->L.page+1);
        set_score(self);
    }
}
