/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _GT_H
#define _GT_H

#define GT_VERSION_MAJ 0
#define GT_VERSION_MIN 0
#define GT_VERSION_REV 2

#define MAIN_DATA_FILE "data/data.cfg"

#define DEVELOPMENT_MODE

typedef struct {                              // gt_config_t
        int  minf, maxf;
        int  minc, maxc;
        int  minv, maxv;
        int  mind, maxd;
        int  dxsize, dysize;
        int  compress_savefile;                // compress the savefile?
        char autopickup[10];
        bool ap[10];                          // adjust later, match object type
} gt_config_t;

typedef struct {                              // game_t
        char         version[20];
        short        width, height;           // width, height of screen
        short        mapw, maph;              // width, height of map window
        int          mapcx, mapcy;
        bool         dead;                    // is the game/player dead?
        short        context;                 // which context are we in? see CONTEXT_ defines
        short        currentlevel;            // what's the current level?
        int          turn;                    // count turns
        long long    tick;
        unsigned int seed;                    // random seed
        short        monsterdefs;             // number of monster definitions
        short        objdefs;                 // number of object definitions
        short        createddungeons;         // number of dungeons which have been created
        int          num_objects;             // number of spawned objects
        int          num_monsters;            // number of spawned monsters
        bool         wizardmode;              // yay!
        char         savefile[255];           // filename of the save file for this game
        obj_t       *objects[2000];
} game_t;

typedef struct {                              // message_t
        int color;
        char text[250];
} message_t;

struct actionqueue {                          // struct actionqueue
        struct actionqueue *head;
        struct actionqueue *next;
        int action;
        long num;
};

typedef struct coord {
        int y;
        int x;
} co;

#define ACTION_NOTHING            0
#define ACTION_PLAYER_MOVE_LEFT   1
#define ACTION_PLAYER_MOVE_RIGHT  2
#define ACTION_PLAYER_MOVE_UP     3
#define ACTION_PLAYER_MOVE_DOWN   4
#define ACTION_PLAYER_MOVE_NW     5
#define ACTION_PLAYER_MOVE_NE     6
#define ACTION_PLAYER_MOVE_SW     7
#define ACTION_PLAYER_MOVE_SE     8
#define ACTION_PICKUP             9
#define ACTION_ATTACK            10
#define ACTION_MOVE_MONSTERS     11
#define ACTION_ENTER_DUNGEON     12
#define ACTION_GO_DOWN_STAIRS    13
#define ACTION_GO_UP_STAIRS      14
#define ACTION_FIX_VIEW          15
#define ACTION_WIELDWEAR         16
#define ACTION_UNWIELDWEAR       17
#define ACTION_HEAL_PLAYER       18
#define ACTION_MAKE_DISTANCEMAP  19
#define ACTION_DROP              20

#define TICKS_MOVEMENT  1000
#define TICKS_ATTACK    1000
#define TICKS_WIELDWEAR  333

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

bool do_next_thing_in_queue();
void queue(int action);
void shutdown_gt();

#endif
