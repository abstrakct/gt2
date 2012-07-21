/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */

#define _XOPEN_SOURCE_EXTENDED

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <signal.h>
#include <libconfig.h>
#include <getopt.h>

#ifdef GT_USE_NCURSES
#include <curses.h>
#endif

#ifdef GT_USE_LIBTCOD
#include <libtcod/libtcod.h>
#endif

#include "cards.h"
#include "objects.h"
#include "o_effects.h"
#include "actor.h"
#include "monsters.h"
#include "quest.h"
#include "npc.h"
#include "world.h"
#include "gt.h"
#include "datafiles.h"
#include "io.h"
#include "debug.h"
#include "saveload.h"
#include "commands.h"
#include "utils.h"

char *otypestrings[50] = {
        "",
        "Gold",
        "Weapon",
        "Armor",
        "Bracelet",
        "Amulet",
        "Card",
        "Wand",
        "Potion",
        "",
        "",
        "",
        "",
        ""
};

// Important global variables
monster_t   *monsterdefs;
obj_t       *objdefs;
game_t      *game;
world_t     *world;
actor_t     *player;
gt_config_t gtconfig;
long        eventnum;
FILE        *messagefile;
bool        mapchanged;
int         tempxsize, tempysize;
bool        loadgame;
void        *eventdata;
actor_t     *a_attacker, *a_victim;
event_t    *eventlist;

// Messages
message_t messages[500];
int currmess, maxmess;

/**
 * @brief Command line options
 */
struct option gt_options[] = {
        { "seed",    1,   0, 's' },
        { "load",    1,   0, 'l' },
        { "version", 0,   0, 'v' },
        { NULL,      0, NULL, 0  }
};

/**
 * @brief Initialize important variables and data structures.
 */
void init_variables()
{
        int i;

        garbageindex = 0;

        monsterdefs = (monster_t *) gtmalloc(sizeof(monster_t));
        monsterdefs->head = monsterdefs;
        mid_counter = 1000;

        objdefs = (obj_t *) gtmalloc(sizeof(obj_t));
        objdefs->head = objdefs;

        eventlist = (event_t *) gtmalloc(sizeof(event_t) * MAXEVENTS);
        for(i=0;i<MAXEVENTS;i++)
                eventlist[i].event = EVENT_FREESLOT;

        world = (world_t *) gtmalloc(sizeof(world_t));

        world->dng = gtcalloc(26, sizeof(level_t));    // allocate n levels, 0 = outside, 1..n = dungeons
        world->out = world->dng;                      // i.e. it points to world->dng[0]
        world->out->xsize = XSIZE;
        world->out->ysize = YSIZE;

        game = (game_t *) gtmalloc(sizeof(game_t));
        game->dead = 0;
        game->seed = time(0);
        srand(game->seed);
        game->createddungeons = 0;
        generate_savefilename(game->savefile);
        loadgame = false;
        gtconfig.rows = ROWS;
        gtconfig.cols = COLS;

        game->wizardmode = false;
        player = (actor_t *) gtmalloc(sizeof(actor_t));
}

/**
 * @brief Initialize the player struct. Should probably later deal with character generation.
 */
void init_player()
{
        // TODO: Character generation!!
        //plx = game->map.w / 2;
        //ply = game->map.h / 2;
        ppx = plx - game->map.w / 2;
        ppy = ply - game->map.h / 2;
        game->mapcx = game->map.w - 2;
        game->mapcy = game->map.h - 2;
        player->viewradius = 12;
        player->level = 1;

        player->attr.str  = dice(3, 6, 0);
        player->attr.dex  = dice(3, 6, 0);
        player->attr.phy  = dice(3, 6, 0);
        player->attr.wis  = dice(3, 6, 0);
        player->attr.cha  = dice(3, 6, 0);
        player->attr.intl = dice(3, 6, 0);

        player->speed = 9;

        // TODO: Starting HP - FIX according to race etc.
        player->hp = player->maxhp = (dice(1, 10, 7)) + ability_modifier(player->attr.phy);

        player->id = PLAYER_ID;
        strcpy(player->name, "Whiskeyjack");
}

/**
 * @brief Close open files and free memory.
 */
void shutdown_gt()
{
        int i;

        i = garbageindex;
        while(i >= 0) {
               i--;
               if(garbage[i])
                       free((void *)garbage[i]);
        }
        
        if(messagefile)
                fclose(messagefile);
}

/**
 * @brief Parse the command line options. Most of this code is taken from getopt's wikipedia page
 *
 * @param argc Arg count
 * @param argv Arguments
 */
void parse_commandline(int argc, char **argv)
{
        int option_index = 0;
        int c;
        char s[15];

        while((c = getopt_long(argc, argv, "s:v", gt_options, &option_index)) != -1) {
                switch(c) {
                        case 's': game->seed = atoi(optarg);
                                  srand(game->seed);
                                  generate_savefilename(game->savefile);
                                  fprintf(stderr, "DEBUG: %s:%d - set random seed to %d (parse_commandline)\n", __FILE__, __LINE__, game->seed);
                                  break;
                        case 'v': get_version_string(s); printf("Gullible's Travails v%s\n", s); exit(0); break;
                        case 'l': strcpy(game->savefile, optarg);
                                  loadgame = true;
                                  break;
                        default:  printf("Unknown command line option 0%o -- ignoring!\n", c);
                }
        }
}

/**
 * @brief Fix the viewport variables so that the map is drawn correctly.
 */
void fixview()
{
        ppx = plx - (game->map.w / 2);
        ppy = ply - (game->map.h / 2);

        if(plx < 0)
                plx = 0;
        if(plx <= (ppx+(game->mapcx/6)))
                ppx--;
        if(plx >= (ppx+(game->mapcx/6*5)))
                ppx++;
        if(ppx >= world->curlevel->xsize-game->mapcx)
                ppx = world->curlevel->xsize-game->mapcx-1;
        if(ppx < 0)
                ppx = 0;

        if(ply < 0)
                ply = 0;
        if(ply <= (ppy + (game->mapcy/6)))
                ppy--;
        if(ply >= (ppy + (game->mapcy/6*5)))
                ppy++;
        if(ppy >= world->curlevel->ysize-game->mapcy)
                ppy = world->curlevel->ysize - game->mapcy - 1;
        if(ppy < 0)
                ppy = 0;
}

/*! \brief Open a door.
 *  \param x The X coordinate of the door.
 *  \param y The Y coordinate of the door.
 */
void open_door(int y, int x)
{
        clearbit(cf(y, x), CF_HAS_DOOR_CLOSED);
        setbit(cf(y, x), CF_HAS_DOOR_OPEN);
#ifdef GT_USE_LIBTCOD
        TCOD_map_set_properties(world->curlevel->map, x, y, true, true);
#endif

        if(hasbit(cf(y+1,x), CF_HAS_DOOR_CLOSED))
                open_door(y+1,x);
        if(hasbit(cf(y-1,x), CF_HAS_DOOR_CLOSED))
                open_door(y-1,x);
        if(hasbit(cf(y,x+1), CF_HAS_DOOR_CLOSED))
                open_door(y,x+1);
        if(hasbit(cf(y,x-1), CF_HAS_DOOR_CLOSED))
                open_door(y,x-1);
}

/*! \brief Setup attack - that is, do what's needed to perform an attack by the player.
 *
 * TODO: Do we need this???
 */
void setup_attack()
{
        schedule_event(EVENT_ATTACK, player);
}

void do_one_event(int event)
{
        event_t a;

        a.event = event;
        a.tick = game->tick;

        do_event(&a);
}

/**
 * @brief Do an event.
 *
 */
bool do_event(event_t *ev)
{
        int oldy, oldx;
        int tmpy, tmpx;
        bool fullturn;
        obj_t *o;
        int i;

        oldy = ply; oldx = plx;
        fullturn = true;
        //gtprintf("Doing event %s %s", event_name[ev->event], ev ? (ev->monster ? ev->monster->name : " ") : " ");

        switch(ev->event) {
                case EVENT_PLAYER_MOVE_DOWN:
                        if(passable(world->curlevel, ply+1, plx)) {
                                if(world->curlevel->c[ply+1][plx].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply+1][plx].monster;
                                        setup_attack();
                                        break;
                                } else
                                        ply++;
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(ply >= (ppy + (game->mapcy/6*5))) {
                                mapchanged = true;
                                ppy++;
                        }
                        if(ppy >= world->curlevel->ysize-game->mapcy) {
                                mapchanged = true;
                                ppy = world->curlevel->ysize - game->mapcy - 1;
                        }
                        if(ppy < 0)
                                ppy = 0;
                        break;
                case EVENT_PLAYER_MOVE_UP:
                        if(passable(world->curlevel, ply-1,plx)) {
                                if(world->curlevel->c[ply-1][plx].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply-1][plx].monster;
                                        setup_attack();
                                        break;
                                } else
                                        ply--;
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(ply < 0)
                                ply = 0;
                        if(ply <= (ppy + (game->mapcy/6))) {
                                mapchanged = true;
                                ppy--;
                        }
                        if(ppy < 0)
                                ppy = 0;
                        break;
                case EVENT_PLAYER_MOVE_LEFT:
                        if(passable(world->curlevel, ply, plx-1)) {
                                if(world->curlevel->c[ply][plx-1].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply][plx-1].monster;
                                        setup_attack();
                                        break;
                                } else
                                        plx--;
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(plx < 0)
                                plx = 0;
                        if(plx <= (ppx+(game->mapcx/6))) {
                                mapchanged = true;
                                ppx--;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case EVENT_PLAYER_MOVE_RIGHT:
                        if(passable(world->curlevel, ply,plx+1)) {
                                if(world->curlevel->c[ply][plx+1].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply][plx+1].monster;
                                        setup_attack();
                                        break;
                                } else
                                        plx++;
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(plx >= (ppx+(game->mapcx/6*5))) {
                                mapchanged = true;
                                ppx++;
                        }
                        if(ppx >= world->curlevel->xsize-game->mapcx) {
                                mapchanged = true;
                                ppx = world->curlevel->xsize-game->mapcx-1;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case EVENT_PLAYER_MOVE_NW:
                        if(passable(world->curlevel, ply-1,plx-1)) {
                                if(world->curlevel->c[ply-1][plx-1].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply-1][plx-1].monster;
                                        setup_attack();
                                        break;
                                } else {
                                        ply--;
                                        plx--;
                                }
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(ply < 0)
                                ply = 0;
                        if(ply <= (ppy + (game->mapcy/6))) {
                                mapchanged = true;
                                ppy--;
                        }
                        if(ppy < 0)
                                ppy = 0;

                        if(plx < 0)
                                plx = 0;
                        if(plx <= (ppx + (game->mapcx/6))) {
                                mapchanged = true;
                                ppx--;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case EVENT_PLAYER_MOVE_NE:
                        if(passable(world->curlevel, ply-1,plx+1)) {
                                if(world->curlevel->c[ply-1][plx+1].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply-1][plx+1].monster;
                                        setup_attack();
                                        break;
                                } else {
                                        ply--; plx++;
                                }
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(plx >= world->curlevel->xsize)
                                plx = world->curlevel->xsize-1;
                        if(plx >= (ppx+(game->mapcx/6*5))) {
                                mapchanged = true;
                                ppx++;
                        }
                        if(ppx >= world->curlevel->xsize-game->mapcx) {
                                mapchanged = true;
                                ppx = world->curlevel->xsize-game->mapcx-1;
                        }
                        if(ppx < 0)
                                ppx = 0;

                        if(ply < 0)
                                ply = 0;
                        if(ply <= (ppy+(game->mapcy/6))) {
                                mapchanged = true;
                                ppy--;
                        }
                        if(ppy < 0)
                                ppy = 0;
                        break;
                case EVENT_PLAYER_MOVE_SW:
                        if(passable(world->curlevel, ply+1, plx-1)) {
                                if(world->curlevel->c[ply+1][plx-1].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply+1][plx-1].monster;
                                        setup_attack();
                                        break;
                                } else {
                                        ply++; plx--;
                                }
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(ply >= world->curlevel->ysize)
                                ply = world->curlevel->ysize-1;
                        if(ply >= (ppy+(game->mapcy/6*5))) {
                                mapchanged = true;
                                ppy++;
                        }
                        if(ppy >= world->curlevel->ysize-game->mapcy) {
                                mapchanged = true;
                                ppy = world->curlevel->ysize-game->mapcy-1;
                        }
                        if(ppy < 0)
                                ppy = 0;

                        if(plx <= (ppx+(game->mapcx/6))) {
                                mapchanged = true;
                                ppx--;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case EVENT_PLAYER_MOVE_SE:
                        if(passable(world->curlevel, ply+1, plx+1)) {
                                if(world->curlevel->c[ply+1][plx+1].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply+1][plx+1].monster;
                                        setup_attack();
                                        break;
                                } else {
                                        ply++; plx++;
                                }
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(ply >= world->curlevel->ysize)
                                ply = world->curlevel->ysize-1;
                        if(ply >= (ppy+(game->mapcy/6*5))) {
                                mapchanged = true;
                                ppy++;
                        }
                        if(ppy >= world->curlevel->ysize - game->mapcy) {
                                mapchanged = true;
                                ppy = world->curlevel->ysize - game->mapcy - 1;
                        }
                        if(ppy < 0)
                                ppy = 0;

                        if(plx >= world->curlevel->xsize)
                                plx = world->curlevel->xsize - 1;
                        if(plx >= (ppx+(game->mapcx/6*5))) {
                                mapchanged = true;
                                ppx++;
                        }
                        if(ppx >= world->curlevel->xsize - game->mapcx) {
                                mapchanged = true;
                                ppx = world->curlevel->xsize - game->mapcx-1;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case EVENT_PICKUP:
                        if(ci(ply, plx) && ci(ply, plx)->gold > 0) {
                                player->inventory->gold += ci(ply, plx)->gold;
                                ci(ply, plx)->gold -= ci(ply, plx)->gold;
                                youc(COLOR_INFO, "now have %d gold pieces.", player->inventory->gold);
                        }

                        if(ci(ply, plx) && ci(ply, plx)->num_used > 0) {
                                int slot;
                                slot = get_first_used_slot(ci(ply, plx));
                                pick_up(ci(ply, plx)->object[slot], player);
                                ci(ply, plx)->object[slot] = NULL;
                                ci(ply, plx)->num_used--;
                        }
                        fullturn = false;
                        break;
                case EVENT_ATTACK:
                        attack(a_attacker, a_victim);
                        break;
                case EVENT_MOVE_MONSTERS:
#ifdef GT_USE_NCURSES
                        makedistancemap(player->y, player->x);
#endif
                        break;
                case EVENT_GO_DOWN_STAIRS:
                        if(game->currentlevel < game->createddungeons) {
                                tmpy = world->curlevel->c[ply][plx].desty;
                                tmpx = world->curlevel->c[ply][plx].destx;
                                if(game->currentlevel == 0) {
                                        world->dng[1].c[tmpy][tmpx].desty = ply;
                                        world->dng[1].c[tmpy][tmpx].destx = plx;
                                        gtprintf("setting return destination to %d,%d", ply, plx);
                                }

                                game->currentlevel++;
                                world->cmap = world->dng[game->currentlevel].c;
                                world->curlevel = &(world->dng[game->currentlevel]);
                                if(game->currentlevel > 0)
                                        game->context = CONTEXT_DUNGEON;

                                ply = tmpy;
                                plx = tmpx;

                                if(world->curlevel->type == 1)
                                        player->viewradius = 12;
                                else
                                        player->viewradius = 12;
                        }
                        init_pathfinding(player);
                        break;
                case EVENT_GO_UP_STAIRS:
                        tmpy = ply; tmpx = plx;
                        ply = world->curlevel->c[tmpy][tmpx].desty;
                        plx = world->curlevel->c[tmpy][tmpx].destx;

                        if(game->currentlevel > 0)
                                game->currentlevel--;
                        world->cmap = world->dng[game->currentlevel].c;
                        world->curlevel = &(world->dng[game->currentlevel]);

                        if(game->currentlevel == 0) {
                                game->context = CONTEXT_OUTSIDE;
                                player->viewradius = 24;
                        }
                        init_pathfinding(player);
                        //floodfill(world->curlevel, player->y, player->x);
                        break;
                case EVENT_WIELDWEAR:
                        o = (obj_t *) eventdata;
                        if(o)
                                wieldwear(player, o);
                        else
                                gtprintf("You don't have that!");
                        break;
                case EVENT_UNWIELDWEAR:
                        o = (obj_t *) eventdata;
                        if(o)
                                unwieldwear(player, o);
                        else
                                gtprintf("You don't have that!");

                        break;
                case EVENT_QUAFF:
                        o = (obj_t *) eventdata;
                        if(o)
                                quaff(player, o);
                        else
                                gtprintf("You don't have that!");
                        break;
                case EVENT_DROP:
                        o = (obj_t *) eventdata;
                        if(o)
                                drop(player, o);
                        else
                                gtprintf("You don't have that!");
                        break;  
                case EVENT_FIX_VIEW:
                        fixview();
                        break;
                case EVENT_HEAL_PLAYER:
                        if(player->hp < player->maxhp) {
                                int x;
                                if(player->hp <= 0) {
                                        // add potentially life saving stuff here?
                                        break;
                                }

                                x = 17 - player->attr.phy;
                                if(x <= 0)
                                        x = 1;
                                if(game->tick % x) {
                                        if(perc(40 + player->attr.phy)) {
                                                int j;

                                                j = ability_modifier(player->attr.phy);
                                                if(j < 1)
                                                        j = 1;
                                                increase_hp(player, j);
                                                gtprintf("You feel a little better!");
                                        }
                                }
                        }
                        break;
                case EVENT_MAKE_DISTANCEMAP:
                        makedistancemap(player->y, player->x);
                        break;
                case EVENT_MOVE_MONSTER:
                        if(ev)
                                move_monster(ev->monster);
                        break;
                case EVENT_PLAYER_NEXTMOVE:
                        process_player_input();

                        i = 17 - pphy;
                        if(i <= 0)
                                i = 1;

                        if(!(game->tick % i)) {
                                if(perc(40+pphy))
                                        schedule_event(EVENT_HEAL_PLAYER, player);
                        }
                        
                        schedule_event_delayed(EVENT_PLAYER_NEXTMOVE, player, 0, 1);
                        break;
                case EVENT_NOTHING:
                        break;
                default:
                        fprintf(stderr, "DEBUG: %s:%d - Unknown event %d attempted!\n", __FILE__, __LINE__, ev->event);
                        gtprintf("DEBUG: %s:%d - Unknown event %d attempted!\n", __FILE__, __LINE__, ev->event);
                        break;
        }

        if(cf(ply, plx) & CF_HAS_DOOR_CLOSED) {
                open_door(ply, plx);
                you("open the door!");
                ply = oldy; plx = oldx;
        }

        return fullturn;
}

int get_next_free_event_slot()
{
        int i;

        for(i = 1; i < MAXEVENTS; i++) {
                if(eventlist[i].event == EVENT_FREESLOT)
                        return i;
        }

        return 0;
}

int schedule_event(int event, actor_t *actor)
{
        int i;

        i = get_next_free_event_slot();
        if(!i)
                die("fatal! no free slots in event queue!");

        eventlist[i].event = event;
        eventlist[i].tick = game->tick + actor->speed;
        eventlist[i].actor = actor;
        //gtprintfc(COLOR_SKYBLUE, "Scheduled event %s at tick %d!", event_name[event], eventlist[i].tick);

        return i;
}

int schedule_event_delayed(int event, actor_t *actor, obj_t *object, int delay)
{
        int i;

        i = get_next_free_event_slot();
        if(!i)
                die("fatal! no free slots in event queue!");

        eventlist[i].event = event;
        eventlist[i].tick = game->tick + actor->speed + delay;
        eventlist[i].actor = actor;
        eventlist[i].object = object;
        //gtprintfc(COLOR_SKYBLUE, "Scheduled delayed event %s at tick %d!", event_name[event], eventlist[i].tick);

        return i;
}

int schedule_event_immediately(int event, actor_t *actor)
{
        int i;
        event_t a;

        i = get_next_free_event_slot();
        if(!i)
                die("fatal! no free slots in event queue!");

        a.event = event;
        a.tick = game->tick;


        //gtprintfc(COLOR_SKYBLUE, "Doing event immediately - %s at tick %d (game->tick = %d)!", event_name[event], eventlist[i].tick, game->tick);
        do_event(&a);

        return i;
}

void unschedule_event(int index)
{
        eventlist[index].event  = EVENT_FREESLOT;
        eventlist[index].tick    = 0;
        eventlist[index].monster = 0;
        eventlist[index].object  = 0;
}

void schedule_monster(monster_t *m)
{
        int i;

        i = schedule_event(EVENT_MOVE_MONSTER, m);
        eventlist[i].monster = m;

        //gtprintfc(COLOR_SKYBLUE, "Scheduled monster %s at tick %d", m->name, eventlist[i].tick);
}

void unschedule_all_monsters()
{
        int i;

        for(i = 0; i < MAXEVENTS; i++) {
                if(eventlist[i].event == EVENT_MOVE_MONSTERS || eventlist[i].event == EVENT_MOVE_MONSTER)
                        unschedule_event(i);
        }
}

/**
 * @brief Queue more than one instance of the same event.
 *
 * @param num How many instances.
 * @param event Which event to queue - see \ref group_events "EVENT-defines" in gt.h
 */
void schedule_eventx(int num, int event, actor_t *actor)
{
        int i;

        for(i=0; i<num; i++)
                schedule_event(event, actor);
}

/**
 * @brief Add several, possibly different, events to the event queue.
 *
 * @param first The first event to add (see \ref group_events "EVENT-defines")
 * @param ... Additional events to add. The last argument has to be ENDOFLIST.
 */
void queuemany(actor_t *actor, int first, ...)
{
        va_list args;
        int i;

        va_start(args, first);
        schedule_event(first, actor);

        i = va_arg(args, int);
        while(i != ENDOFLIST) {
                schedule_event(i, actor);
                i = va_arg(args, int);
        }
        va_end(args);
}

void process_autopickup()
{
        if(ci(ply, plx)) {
                if(ci(ply, plx) && ci(ply, plx)->gold) {
                        if(gtconfig.ap[OT_GOLD])
                                schedule_event_immediately(EVENT_PICKUP, player);
                }

                if(ci(ply, plx)->num_used > 0) {
                        if(ci(ply, plx)->num_used == 1) {
                                int slot;
                                obj_t *ob;

                                slot = get_first_used_slot(ci(ply, plx));
                                if(slot < 0)
                                        return;

                                ob = ci(ply, plx)->object[slot];
                                if(gtconfig.ap[ob->type] && !hasbit(ob->flags, OF_DONOTAP)) {
                                        schedule_event_immediately(EVENT_PICKUP, player);
                                }
                        }
                }
        }
}

/**
 * @brief Take a look at the player's current position, and tell him what he sees.
 *
 */
void look()
{
        //char *stairmat[] = { "stone", "wood", "bone", "marble", "metal" };

        if(cf(ply, plx) & CF_HAS_STAIRS_DOWN) {
                if(game->currentlevel == 0)
                        gtprintf("There is a portal to the underworld here!");
                else                                
                        gtprintf("There is a staircase leading down here.");
        }

        if(cf(ply, plx) & CF_HAS_STAIRS_UP) {
                if(game->currentlevel == 1) 
                        gtprintf("There is a portal to the outside world here!");
                else
                        gtprintf("There is a staircase leading up here.");
        }

        if(ci(ply, plx)) {
                if(ci(ply, plx) && ci(ply, plx)->gold) {
                                gtprintf("There is %d gold %s here.", ci(ply, plx)->gold, (ci(ply, plx)->gold > 1) ? "pieces" : "piece");
                }

                if(ci(ply, plx)->num_used > 0) {
                        if(ci(ply, plx)->num_used == 1) {
                                int slot;
                                obj_t *ob;

                                slot = get_first_used_slot(ci(ply, plx));
                                if(slot < 0)
                                        return;

                                ob = ci(ply, plx)->object[slot];
                                gtprintf("There is %s here.", a_an(pair(ob)));
                        }

                        if(ci(ply, plx)->num_used == 2) {
                                int slot, slot2;

                                slot  = get_first_used_slot(ci(ply, plx));
                                slot2 = get_next_used_slot_after(slot, ci(ply, plx));
                                if(slot < 0)
                                        return;
                                if(slot2 < 0)
                                        return;

                                gtprintf("There is %s and %s here.", a_an(pair(ci(ply, plx)->object[slot])), a_an(pair(ci(ply, plx)->object[slot2])));
                        }

                        if(ci(ply, plx)->num_used > 2) {
                                int i;

                                gtprintfc(COLOR_INFO, "There are several things here:");
                                for(i=0;i<52;i++) {
                                        if(ci(ply, plx)->object[i])
                                                gtprintf("%s", a_an(pair(ci(ply, plx)->object[i])));
                                }
                        }
                }
        }
}

void do_everything_at_tick(int tick)
{
        int i;

        for(i = 0; i < MAXEVENTS; i++) {
                if(eventlist[i].event >= 0) {
                        if(eventlist[i].tick == tick) {
                                do_event(&eventlist[i]);
                                unschedule_event(i);
                        }
                }
        }
}

void increase_ticks(int i)
{
        game->tick += i;
        update_ticks();
}

/**
 * @brief Do a complete turn. This should take into account the player's speed, and schedule other events accordingly.
 */
void do_turn()
{
        int i;

        //gtprintf("Started do_turn. ---------------------------------------");

        //dump_event_queue();
        //update_screen();
        //look();
        
        look_for_monsters();
        update_screen();

        if(is_autoexploring) {
                if(gt_checkforkeypress()) {
                        gtprintf("Autoexplore interrupted...");
                        clearbit(player->flags, PF_AUTOEXPLORING);
                }
        }

        if(game->dead)
                return;

        for(i = 0; i < 10; i++) {
                do_everything_at_tick(game->tick);
                look_for_monsters();
                //update_screen();
                increase_ticks(1);
        }

        process_temp_effects(player);
        update_screen();

        //gtprintf("Ended do_turn. -----------------------------------------");
}

/**
 * @brief Catch a signal and perform an event (currently, that event is to exit).
 */
void catchsignal()
{
        game->dead = true;
        shutdown_display();
        shutdown_gt();
        fprintf(stderr, "Caught signal - exiting.\n");
        exit(0);
}

/**
 * @brief Get a command from the player, and schedule the appropriate event.
 *
 */
void process_player_input()
{
        int c, x, y, l, i;
        npc_t *npc;

        look();
        process_autopickup();
        update_screen();
        c = 0;
        while(!c)
                c = get_command();

        mapchanged = false;
        player->oldx = plx;
        player->oldy = ply;

        switch(c) {
                case CMD_QUIT:
                        game->dead = 1;
                        break;
                case CMD_DOWN:  schedule_event(EVENT_PLAYER_MOVE_DOWN, player); break;
                case CMD_UP:    schedule_event(EVENT_PLAYER_MOVE_UP, player); break;
                case CMD_LEFT:  schedule_event(EVENT_PLAYER_MOVE_LEFT, player); break;
                case CMD_RIGHT: schedule_event(EVENT_PLAYER_MOVE_RIGHT, player); break;
                case CMD_NW:    schedule_event(EVENT_PLAYER_MOVE_NW, player); break;
                case CMD_NE:    schedule_event(EVENT_PLAYER_MOVE_NE, player); break;
                case CMD_SW:    schedule_event(EVENT_PLAYER_MOVE_SW, player); break;
                case CMD_SE:    schedule_event(EVENT_PLAYER_MOVE_SE, player); break;
                case CMD_WIELDWEAR:
                                l = ask_char("Which item would you like to wield/wear?");
                                eventdata = (void *) get_object_from_letter(l, player->inventory);
                                schedule_event(EVENT_WIELDWEAR, player);
                                break;
                case CMD_UNWIELDWEAR:
                                l = ask_char("Which item would you like to remove/unwield?");
                                eventdata = (void *) get_object_from_letter(l, player->inventory);
                                schedule_event(EVENT_UNWIELDWEAR, player);
                                break;
                case CMD_DROP:
                                l = ask_char("Which item would you like to drop?");
                                eventdata = (void *) get_object_from_letter(l, player->inventory);
                                schedule_event(EVENT_DROP, player);
                                break;
                case CMD_LONGDOWN:
                                schedule_eventx(20, EVENT_PLAYER_MOVE_DOWN, player);
                                break;
                case CMD_LONGUP:
                                schedule_eventx(20, EVENT_PLAYER_MOVE_UP, player);
                                break;
                case CMD_LONGLEFT:
                                schedule_eventx(20, EVENT_PLAYER_MOVE_LEFT, player);
                                break;
                case CMD_LONGRIGHT:
                                schedule_eventx(20, EVENT_PLAYER_MOVE_RIGHT, player);
                                break;
                case CMD_TOGGLEFOV:
                                gtprintf("Setting all cells to visible.");
                                set_level_visited(world->curlevel);
                                schedule_event(EVENT_NOTHING, player);
                                break;
                case CMD_SPAWNMONSTER:
                                spawn_monster_at(ply+5, plx+5, ri(1, game->monsterdefs), world->curlevel->monsters, world->curlevel, 100);
                                //dump_monsters(world->curlevel->monsters);
                                schedule_event(EVENT_NOTHING, player);
                                break;
                case CMD_WIZARDMODE:
                                game->wizardmode = (game->wizardmode ? false : true); schedule_event(EVENT_NOTHING, player);
                                gtprintf("Wizard mode %s!", game->wizardmode ? "on" : "off");
                                break;
                case CMD_SAVE:
                                save_game(game->savefile);
                                schedule_event(EVENT_NOTHING, player);
                                break;
                case CMD_LOAD:
                                if(!load_game(game->savefile, 1))
                                        gtprintf("Loading failed!");
                                else
                                        gtprintf("Loading successful!");
                                schedule_event(EVENT_NOTHING, player);
                                break;
                case CMD_DUMPOBJECTS:
                                dump_objects(world->curlevel->c[ply][plx].inventory);
                                schedule_event(EVENT_NOTHING, player);
                                break;
                case CMD_INCFOV:
                                //player->viewradius++;
                                player->speed--;
                                //world->out->lakelimit++;
                                //generate_terrain(1);
                                //gtprintf("lakelimit = %d", world->out->lakelimit);
                                schedule_event(EVENT_NOTHING, player);
                                break;
                case CMD_DECFOV:
                                //player->viewradius--;
                                player->speed++;
                                //world->out->lakelimit--;
                                //generate_terrain(1);
                                //gtprintf("lakelimit = %d", world->out->lakelimit);
                                schedule_event(EVENT_NOTHING, player);
                                break;
                case CMD_FLOODFILL:
                                x = ri(11, world->curlevel->xsize);
                                y = ri(11, world->curlevel->ysize);
                                while(world->curlevel->c[y][x].type != DNG_FLOOR) {
                                        x = ri(11, world->curlevel->xsize);
                                        y = ri(11, world->curlevel->ysize);
                                }
                                gtprintf("floodfilling from %d, %d\n", y, x);
                                floodfill(world->curlevel, y, x);
                                schedule_event(EVENT_NOTHING, player);
                                break;
                case CMD_INVENTORY:
                                schedule_event(EVENT_NOTHING, player);
                                dump_objects(player->inventory);
                                break;
                case CMD_PICKUP:
                                schedule_event(EVENT_PICKUP, player);
                                break;
                case CMD_QUAFF:
                                l = ask_char("Which potion would you like to drink?");
                                eventdata = (void *) get_object_from_letter(l, player->inventory);
                                schedule_event(EVENT_QUAFF, player);
                                break;
                case CMD_DESCEND:
                                if(hasbit(cf(ply,plx), CF_HAS_STAIRS_DOWN)) {
                                        schedule_event(EVENT_GO_DOWN_STAIRS, player);
                                        schedule_event(EVENT_FIX_VIEW, player);
                                        unschedule_all_monsters();
                                } else {
                                        gtprintf("You can't go up here!");
                                }
                                break;
                case CMD_ASCEND:
                                if(hasbit(cf(ply,plx), CF_HAS_STAIRS_UP)) {
                                        schedule_event(EVENT_GO_UP_STAIRS, player);
                                        schedule_event(EVENT_FIX_VIEW, player);
                                        unschedule_all_monsters();
                                } else {
                                        gtprintf("You can't go up here!");
                                }
                                break;
                case CMD_CHAT:
                                i = get_number_of_npcs_nearby(player);
                                if(!i)
                                        gtmsgbox(" ... ", "There's no one to talk to nearby!");
                                else {
                                        npc = get_nearest_npc(player);
                                        if(npc->has_quest)
                                                npc->quest->initiate();
                                        else
                                                npc->chat(npc);
                                }
                                break;
                case CMD_REST:
                                schedule_event(EVENT_NOTHING, player);
                                break;
                case CMD_PATHFINDER: break;
                case CMD_AUTOEXPLORE:
                                     autoexplore();
                                     break;
                case CMD_DUMPAQ:
                                     dump_event_queue();
                                     break;
                default:
                                     schedule_event(EVENT_NOTHING, player);
                                     break;
        }
}

void game_loop()
{
        while(!game->dead) {
                do_turn();
        };
}

int main(int argc, char *argv[])
{
        char messagefilename[50];
        //char found;
        //bool done;

        signal(SIGINT,  catchsignal);
        signal(SIGKILL, catchsignal);
        
        if(!setlocale(LC_ALL, ""))
                die("couldn't set locale.");

        init_variables();

        get_version_string(game->version);
        printf("Gullible's Travails v%s\n", game->version);

        parse_commandline(argc, argv);

        if(!loadgame) {
                init_objects();
                generate_deck();
                printf("Reading data files...\n");
                if(parse_data_files(0))
                        die("Couldn't parse data files.");
                start_autopickup();
        }

        if(loadgame) {
                init_player();
                parse_data_files(ONLY_CONFIG);
                if(!load_game(game->savefile, 0))
                        die("Loading game failed! Filename: %s\n", game->savefile);

                sprintf(messagefilename, "%s/messages.%d.gtsave", SAVE_DIRECTORY, game->seed);
                messagefile = fopen(messagefilename, "a");
                
                init_display();
                
                // these next should be loaded by load_game?!
                world->cmap = world->dng[game->currentlevel].c;
                world->curlevel = &world->dng[game->currentlevel];

                fov_initmap(world->curlevel);
                fov_updatemap(world->curlevel);
        } else {
                init_level(world->out);
                generate_world();

                sprintf(messagefilename, "%s/messages.%d.gtsave", SAVE_DIRECTORY, game->seed);
                messagefile = fopen(messagefilename, "a");

                init_display();
                init_player();
                player->inventory = init_inventory();

                world->cmap = world->out->c;
                world->curlevel = world->out;
                game->context = CONTEXT_OUTSIDE;

                // then move down a level...
                move_player_to_stairs_down(0);
                do_one_event(EVENT_GO_DOWN_STAIRS);
                fixview();
        }

        init_commands();
        init_pathfinding(player);
        initial_update_screen();
        schedule_event_delayed(EVENT_PLAYER_NEXTMOVE, player, 0, 1);

        game_loop();

        shutdown_display();
        shutdown_gt();

        return 0;
}

// vim: fdm=syntax guifont=Terminus\ 8
