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
        int vx, vy;        // upper left corner of viewport
        int dead;
        unsigned int seed;
        struct {
                int minf, maxf;
                int minc, maxc;
                int minv, maxv;
                int mind, maxd;
        } c;
} game_t;

typedef struct {       // message_t
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
extern actor_t *player;
extern int mapcx, mapcy;
extern FILE *messagefile;
extern message_t m[500];
extern int currmess, maxmess;

/*extern WINDOW *wall;
extern WINDOW *wstat;
extern WINDOW *winfo;
extern WINDOW *wmap;*/


/* function prototypes */

void mess(char *message);
void messc(int color, char *message);


#endif
