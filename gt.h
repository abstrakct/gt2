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
        int minf, maxf;
        int minc, maxc;
        int minv, maxv;
        int mind, maxd;
} gt_config_t;

typedef struct {            // game_t
        int width, height;
        int mapw, maph;     // width, height of map window
        int dead;           // is the game/player dead?
        int context;        // which context are we in? see CONTEXT_ defines
        unsigned int seed;  // random seed
        int monsterdefs;    // number of monster definitions
} game_t;

typedef struct {       // message_t
        int color;
        char text[250];
} message_t;

struct actionqueue {
        struct actionqueue *head;
        struct actionqueue *next;
        int action;
        long num;
};

#define ACTION_NOTHING            0
#define ACTION_PLAYER_MOVE_LEFT   1
#define ACTION_PLAYER_MOVE_RIGHT  2
#define ACTION_PLAYER_MOVE_UP     3
#define ACTION_PLAYER_MOVE_DOWN   4
#define ACTION_PLAYER_MOVE_NW     5
#define ACTION_PLAYER_MOVE_NE     6
#define ACTION_PLAYER_MOVE_SW     7
#define ACTION_PLAYER_MOVE_SE     8

#define CONTEXT_OUTSIDE 0
#define CONTEXT_DUNGEON 1

#define FALSE 0
#define TRUE 1
// #define MAX_MESSAGES 100

// define some shortcuts
#define ply player->y
#define plx player->x
#define ppx player->px
#define ppy player->py
#define pyxt world->cmap[player->y][player->x].type

#define ct(a,b) world->cmap[a][b].type
#define cv(a,b) world->cmap[a][b].visible
#define cc(a,b) world->cmap[a][b].color
#define cm(a,b) world->cmap[a][b].monster
#define ci(a,b) world->cmap[a][b].inventory


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
extern struct actionqueue *aq;
extern gt_config_t gtconfig;

/*extern WINDOW *wall;
extern WINDOW *wstat;
extern WINDOW *winfo;
extern WINDOW *wmap;*/


/* function prototypes */

void queue(int action);

#endif
