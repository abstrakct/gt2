/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _GT_H
#define _GT_H

#define GT_VERSION_MAJ 0
#define GT_VERSION_MIN 0
#define GT_VERSION_REV 1

#define MAIN_DATA_FILE "data/data.cfg"

typedef struct {
        int width, height;
        int mapw, maph;    // width, height of map window
        int dead;
        unsigned int seed;
        struct {
                int minf, maxf;
                int minc, maxc;
                int minv, maxv;
                int mind, maxd;
        } c;
} game_t;

typedef struct {
        int color;
        char text[250];
} message_t;

// #define MAX_MESSAGES 100

// global variables
extern world_t *world;
extern monster_t *monsterdefs;
extern obj_t *objdefs;
extern game_t *game;
extern monster_t *monsterdefs;
extern obj_t *objdefs;

#define COLOR_NORMAL  1
#define COLOR_PLAYER  2
#define COLOR_INFO    3
#define COLOR_WARNING 10
#define COLOR_GOOD    11
#define COLOR_BAD     12

/* function prototypes */

void mess(char *message);
void messc(int color, char *message);


#endif
