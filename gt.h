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

typedef struct {                    // config
        int minf, maxf;
        int minc, maxc;
        int minv, maxv;
        int mind, maxd;
        int dxsize, dysize;
        int compress_savefile;      // compress the savefile?
} gt_config_t;

typedef struct {                              // game_t
        short        width, height;           // width, height of screen
        short        mapw, maph;              // width, height of map window
        bool         dead;                    // is the game/player dead?
        short        context;                 // which context are we in? see CONTEXT_ defines
        short        currentlevel;            // what's the current level?
        int          turn;                    // count turns
        unsigned int seed;                    // random seed
        short        monsterdefs;             // number of monster definitions
        short        objdefs;                 // number of object definitions
        short        createddungeons;         // number of dungeons which have been created
        bool         wizardmode;              // yay!
        char         savefile[255];           // filename of the save file for this game
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
#define ENDOFLIST -577

// define some shortcuts
#define ply player->y
#define plx player->x
#define ppx player->px
#define ppy player->py
#define pyxt world->cmap[player->y][player->x].type



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
extern message_t messages[500];
extern int currmess, maxmess;
extern struct actionqueue *aq;
extern gt_config_t gtconfig;
extern int tempxsize, tempysize;

/*extern WINDOW *wall;
extern WINDOW *wstat;
extern WINDOW *winfo;
extern WINDOW *wmap;*/


/* function prototypes */

void queue(int action);
void shutdown_gt();

#endif
