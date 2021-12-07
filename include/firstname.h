#ifndef GMTKJAM21_FIRSTNAME_H
#define GMTKJAM21_FIRSTNAME_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "rhc/error.h"

#define FIRSTNAME_MAX_LENGTH 16

// out_name should be a buffer with at least FIRSTNAME_MAX_LENGTH+1 bytes (null terminator)
static void firstname_generate(char *out_name) {
    const char *pre[] = {
            "Wolfgang",
            "Hermann",
            "Seppl",
            "Thommy",
            "Artur",
            "Hannes",
            "SUPER",
            "ULTRA",
            "MEGA",
            "mini",
            "micro",
            "Lutz",
            "Klaas",
            "Zuzzy",
            "Bingo",
            "Frau",
            "Billy",
            "Andy",
            "Kati",
            "Chris",
            "Bomba"
    };
    size_t pre_len = sizeof pre / sizeof *pre;
    for (int i = 0; i < pre_len; i++)
        assume(strlen(pre[i]) <= 8, "wtf");


    const char *mid[] = {
            "Ballo",
            "Missi",
            "Mammu",
            "-XxX-",
            " x32 ",
            "(.I.)",
            "_____",
            "_-_-_",
            "23456",
            "85858",
            "77779",
            "00002",
            " ",
            " H ",
            " G ",
            " loli",
            "_jjj",
            "Mummy",
            "Daddy",
            "Uncle",
            "SAM",
            "Denis",
            "Juerg",
            "Joach",
            "Detlf",
            " Korn",
            " RUM",
            " fish",
            "Shark",
            " gin-",
            "-ALCY",
            "PLUS",
            "67000",
            "  9  ",
            " 8p",
            " ina",
            ":in",
            "MAN",
            ""
    };
    size_t mid_len = sizeof mid / sizeof *mid;
    for (int i = 0; i < mid_len; i++)
        assume(strlen(mid[i]) <= 5, "wtf");


    const char *post[] = {
            "_0",
            "lol",
            "TUF",
            "hey",
            "666",
            "93",
            "92",
            "91",
            "94",
            "001",
            "007",
            "x32",
            "x64",
            "x86",
            "GoG",
            "-_-",
            "o_o",
            " :D",
            " ;D",
            " xD",
            " :P",
            " xP",
            " ;P",
            " 8",
            " 99",
            "0 0",
            "Zzz",
            "Hmm",
            ""
    };
    size_t post_len = sizeof post / sizeof *post;
    for (int i = 0; i < post_len; i++)
        assume(strlen(post[i]) <= 3, "wtf");

    // random choice
    srand(time(NULL));

    const char *used_pre = pre[rand() % pre_len];
    const char *used_mid = mid[rand() % mid_len];
    const char *used_post = post[rand() % post_len];

    snprintf(out_name, FIRSTNAME_MAX_LENGTH + 1, "%s%s%s", used_pre, used_mid, used_post);
}

#endif //GMTKJAM21_FIRSTNAME_H
