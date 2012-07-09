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

typedef struct {
        int x, y, w, h;
#ifdef GT_USE_LIBTCOD
        TCOD_console_t c;
#endif
#ifdef GT_USE_NCURSES
        WINDOW *c;
#endif
} win_t;

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
        win_t        map, messages, left, right;
} game_t;

typedef struct {                              // message_t
        gtcolor_t color;
        char text[250];
} message_t;

struct eventqueue {                          // struct eventqueue
        struct eventqueue *head;             //!< Pointer to the head of the eventqueue
        struct eventqueue *next;             //!< Pointer to the next item in the queue
        int event;                           //!< Which event is it? See \ref group_events "EVENT_defines"
        long num;                             //!< In the head, this contains the number of events in the queue, for a specific event it signifies which number the event is overall (e.g. if it's the tenth event performed in the game, the two-thousandth, or whatever).
        int  tick;                            //!< At which tick should this event be performed?
        monster_t *monster;
        obj_t *object;
};

typedef struct event {
        int        event;
        int        tick;
        monster_t *monster;
        obj_t     *object;
        actor_t   *actor;
        int        gain;                      // for temporary effects
//        bool       completed;
} event_t;

#define MAXACT 100

/** @defgroup group_events Group of event defines
 * @{
 */
#define EVENT_NOTHING            0
#define EVENT_PLAYER_MOVE_LEFT   1
#define EVENT_PLAYER_MOVE_RIGHT  2
#define EVENT_PLAYER_MOVE_UP     3
#define EVENT_PLAYER_MOVE_DOWN   4
#define EVENT_PLAYER_MOVE_NW     5
#define EVENT_PLAYER_MOVE_NE     6
#define EVENT_PLAYER_MOVE_SW     7
#define EVENT_PLAYER_MOVE_SE     8
#define EVENT_PICKUP             9
#define EVENT_ATTACK            10
#define EVENT_MOVE_MONSTERS     11
#define EVENT_ENTER_DUNGEON     12
#define EVENT_GO_DOWN_STAIRS    13
#define EVENT_GO_UP_STAIRS      14
#define EVENT_FIX_VIEW          15
#define EVENT_WIELDWEAR         16
#define EVENT_UNWIELDWEAR       17
#define EVENT_HEAL_PLAYER       18
#define EVENT_MAKE_DISTANCEMAP  19
#define EVENT_DROP              20
#define EVENT_QUAFF             21
#define EVENT_MOVE_MONSTER      22
#define EVENT_PLAYER_NEXTMOVE   23
//#define EVENT_DECREASE_TEMP_EFFECT 24
#define EVENT_DECREASE_INVISIBILITY 24
#define EVENT_DECREASE_TEMP_CHARISMA 25
#define EVENT_DECREASE_TEMP_STRENGTH 26
#define EVENT_DECREASE_TEMP_INTELLIGENCE 27
#define EVENT_DECREASE_TEMP_PHYSIQUE 28
#define EVENT_DECREASE_TEMP_WISDOM 29
#define EVENT_DECREASE_TEMP_DEXTERITY 30

#define EVENT_FREESLOT          -577
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

#define PLAYER_ID -577

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
extern gt_config_t         gtconfig;
extern int                 tempxsize, tempysize;
extern event_t            *eventlist;


/* function prototypes */

//struct eventqueue* schedule_event(int event, actor_t *actor);
//struct eventqueue* schedule_event_delayed(int event, actor_t *actor, int delay);

int get_next_free_event_slot();
int schedule_event(int event, actor_t *actor);
int schedule_event_delayed(int event, actor_t *actor, obj_t *object, int delay);
void schedule_monster(monster_t *m);
void unschedule_event(int index);

void queuemany(actor_t *actor, int first, ...);
void shutdown_gt();
void do_one_event(int event);
bool do_event(event_t *aqe);
void do_everything_at_tick(int tick);

void do_turn();
void process_player_input();


#endif
