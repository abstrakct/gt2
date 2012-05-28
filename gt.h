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

#define GAME_NAME "Gullible's Travails"

#define MAIN_DATA_FILE "data/data.cfg"

#define DEVELOPMENT_MODE

#ifdef GT_USE_LIBTCOD
#include <libtcod/libtcod.h>
#endif

/**
 * @struct gt_config_t
 * @brief Various config options.
 */
typedef struct {                              // gt_config_t
        int  minf, maxf;                      //!< minimum and maximum number of forests
        int  minc, maxc;                      //!< minimum and maximum number of cities
        int  minv, maxv;
        int  mind, maxd;
        int  dxsize, dysize;
        int  compress_savefile;               //!< compress the savefile?
        char autopickup[10];                  //!< Chars of objects to autopickup
        bool ap[10];                          //!< adjust later, match object type
        int  rows, cols;                      //!< Window size
} gt_config_t;

#ifdef GT_USE_LIBTCOD
typedef struct {
        int x, y, w, h;
        TCOD_console_t c;
} win_t;
#endif

/**
 * @brief Variables related to the running game.
 */
typedef struct {                              // game_t
        char         version[20];             //!< Version of the game, in string format
        short        width, height;           //!< width, height of screen
        short        mapw, maph;              //!< width, height of map window
        int          mapcx, mapcy;
        bool         dead;                    //!< is the game/player dead?
        short        context;                 //!< which context are we in? see CONTEXT_ defines
        short        currentlevel;            //!< what's the current level?
        int          turn;                    //!< count turns
        int          tick, prevtick;
        unsigned int seed;                    //!< random seed
        short        monsterdefs;             //!< number of monster definitions
        short        objdefs;                 //!< number of object definitions
        short        createddungeons;         //!< number of dungeons which have been created
        int          num_objects;             //!< number of spawned objects
        int          num_monsters;            //!< number of spawned monsters
        bool         wizardmode;              //!< wizardmode! yay!
        char         savefile[255];           //!< filename of the save file for this game
        obj_t       *objects[2000];           //!< Pointers to every object currently existing in the game. 2000 might have to be adjusted.
#ifdef GT_USE_LIBTCOD
        win_t        map, messages, left, right;
#endif
} game_t;

typedef struct {                              // message_t
        gtcolor_t color;
        char text[250];
} message_t;

struct actionqueue {                          // struct actionqueue
        struct actionqueue *head;             //!< Pointer to the head of the actionqueue
        struct actionqueue *next;             //!< Pointer to the next item in the queue
        int action;                           //!< Which action is it? See \ref group_actions "ACTION_defines"
        long num;                             //!< In the head, this contains the number of actions in the queue, for a specific action it signifies which number the action is overall (e.g. if it's the tenth action performed in the game, the two-thousandth, or whatever).
        int  tick;                            //!< At which tick should this action be performed?
        monster_t *monster;
        obj_t *object;
};

typedef struct action {
        int        action;
        int        tick;
//        bool       completed;
        monster_t *monster;
        obj_t     *object;
} action_t;

#define MAXACT 100

/** @defgroup group_actions Group of action defines
 * @{
 */
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
#define ACTION_QUAFF             21
#define ACTION_MOVE_MONSTER      22
#define ACTION_PLAYER_NEXTMOVE   23

#define ACTION_FREESLOT          -577
/** @} */

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

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

// global variables
extern world_t            *world;
extern game_t             *game;
extern monster_t          *monsterdefs;
extern obj_t              *objdefs;
extern actor_t            *player;
extern int                 mapcx, mapcy;
extern FILE               *messagefile;
extern message_t           messages[500];
extern int                 currmess, maxmess;
extern struct actionqueue *aq;
extern gt_config_t         gtconfig;
extern int                 tempxsize, tempysize;
extern int                 actionlength[100];


/* function prototypes */

//struct actionqueue* schedule_action(int action, actor_t *actor);
//struct actionqueue* schedule_action_delayed(int action, actor_t *actor, int delay);

int get_next_free_action_slot();
int schedule_action(int action, actor_t *actor);
int schedule_action_delayed(int action, actor_t *actor, int delay);
void schedule_monster(monster_t *m);
void unschedule_action(int index);

int do_next_thing_in_queue();
void queue(int action);
void queuemany(actor_t *actor, int first, ...);
void shutdown_gt();
void do_one_action(int action);
bool do_action(action_t *aqe);
void do_everything_at_tick(int tick);

void do_turn();
void process_player_input();


#endif
