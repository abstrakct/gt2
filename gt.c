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
monster_t          *monsterdefs;
obj_t              *objdefs;
game_t             *game;
world_t            *world;
actor_t            *player;
struct actionqueue *aq;
gt_config_t         gtconfig;
long                actionnum;
FILE               *messagefile;
bool                mapchanged;
int                 tempxsize, tempysize;
bool                loadgame;
void               *actiondata;
actor_t            *a_attacker, *a_victim;
action_t           *act;

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

        /*aq = (struct actionqueue *) gtmalloc(sizeof(struct actionqueue));
        aq->head = aq;
        aq->next = 0;
        aq->action = ACTION_NOTHING;
        actionnum = 0;*/

        act = (action_t *) gtmalloc(sizeof(action_t) * MAXACT);
        for(i=0;i<MAXACT;i++)
                act[i].action = ACTION_FREESLOT;

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
        TCOD_map_set_properties(world->curlevel->map, x, y, true, true);

        if(hasbit(cf(y+1,x), CF_HAS_DOOR_CLOSED))
                open_door(y+1,x);
        if(hasbit(cf(y-1,x), CF_HAS_DOOR_CLOSED))
                open_door(y-1,x);
        if(hasbit(cf(y,x+1), CF_HAS_DOOR_CLOSED))
                open_door(y,x+1);
        if(hasbit(cf(y,x-1), CF_HAS_DOOR_CLOSED))
                open_door(y,x-1);
}

/*! \brief Clear the actionqueue 
void clear_aq()
{
        struct actionqueue *tmp;

        while(aq->num) {
                tmp = aq->next;
                aq->next = tmp->next;
                gtfree(tmp);
                aq->num--;
        }
}*/

/*! \brief Setup attack - that is, do what's needed to perform an attack by the player.
 */
void setup_attack()
{
        schedule_action(ACTION_ATTACK, player);
}

void do_one_action(int action)
{
        action_t a;

        a.action = action;
        a.tick = game->tick;

        do_action(&a);
}

/**
 * @brief Do an action. This really should be split into functions.
 *
 * @param action Which action to perform - see \ref group_actions "ACTION-defines" in gt.h
 *
 * @return True if action took a full turn, false if not. (This is currently becoming obsolete.) 
 */
bool do_action(action_t *aqe)
{
        int oldy, oldx;
        int tmpy, tmpx;
        bool fullturn;
        obj_t *o;
        int i;

        oldy = ply; oldx = plx;
        fullturn = true;
        //gtprintf("Doing action %s %s", action_name[aqe->action], aqe ? (aqe->monster ? aqe->monster->name : " ") : " ");

        switch(aqe->action) {
                case ACTION_PLAYER_MOVE_DOWN:
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
                case ACTION_PLAYER_MOVE_UP:
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
                case ACTION_PLAYER_MOVE_LEFT:
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
                case ACTION_PLAYER_MOVE_RIGHT:
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
                case ACTION_PLAYER_MOVE_NW:
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
                case ACTION_PLAYER_MOVE_NE:
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
                case ACTION_PLAYER_MOVE_SW:
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
                case ACTION_PLAYER_MOVE_SE:
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
                case ACTION_PICKUP:
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
                case ACTION_ATTACK:
                        attack(a_attacker, a_victim);
                        break;
                case ACTION_MOVE_MONSTERS:
#ifdef GT_USE_NCURSES
                        do_action(ACTION_MAKE_DISTANCEMAP);
#endif
                        move_monsters();
                        fullturn = false;
                        break;
                case ACTION_GO_DOWN_STAIRS:
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
                        fullturn = false;
                        break;
                case ACTION_GO_UP_STAIRS:
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
                        fullturn = false;
                        break;
                case ACTION_WIELDWEAR:
                        o = (obj_t *) actiondata;
                        if(o)
                                wieldwear(player, o);
                        else
                                gtprintf("You don't have that!");
                        fullturn = false;
                        break;
                case ACTION_UNWIELDWEAR:
                        o = (obj_t *) actiondata;
                        if(o)
                                unwieldwear(player, o);
                        else
                                gtprintf("You don't have that!");
                        fullturn = false;

                        break;
                case ACTION_QUAFF:
                        o = (obj_t *) actiondata;
                        if(o)
                                quaff(player, o);
                        else
                                gtprintf("You don't have that!");
                        fullturn = false;
                        break;
                case ACTION_DROP:
                        o = (obj_t *) actiondata;
                        if(o)
                                drop(player, o);
                        else
                                gtprintf("You don't have that!");
                        fullturn = false;
                        break;  
                case ACTION_FIX_VIEW:
                        fixview();
                        fullturn = false;
                        break;
                case ACTION_HEAL_PLAYER:
                        increase_hp(player, 1);
                        fullturn = false;
                        break;
                case ACTION_MAKE_DISTANCEMAP:
                        makedistancemap(player->y, player->x);
                        fullturn = false;
                        break;
                case ACTION_MOVE_MONSTER:
                        if(aqe)
                                move_monster(aqe->monster);
                        break;
                case ACTION_PLAYER_NEXTMOVE:
                        process_player_input();

                        i = 17 - pphy;
                        if(i <= 0)
                                i = 1;

                        if(!(game->tick % i)) {
                                if(perc(40+pphy))
                                        schedule_action(ACTION_HEAL_PLAYER, player);
                        }
                        
                        schedule_action_delayed(ACTION_PLAYER_NEXTMOVE, player, 0, 1);

                        // also, schedule some future actions
                        //if(!game->tick % 50) {
                                //schedule_action_delayed(ACTION_PLAYER_NEXTMOVE, player, player->speed);
                                //schedule_action_delayed(ACTION_PLAYER_NEXTMOVE, player, player->speed * 2);
                                //schedule_action_delayed(ACTION_PLAYER_NEXTMOVE, player, player->speed * 3);
                                //schedule_action_delayed(ACTION_PLAYER_NEXTMOVE, player, player->speed * 4);
                        //}

                        break;
                case ACTION_DECREASE_INVISIBILITY:
                        aqe->actor->temp[TEMP_INVISIBLE] -= 1;
                        if(aqe->actor->temp[TEMP_INVISIBLE] == 0)
                                oe_invisibility(aqe->actor, NULL, 0, false);
                        break;
                //case ACTION_DECREASE_TEMP_CHARISMA:
                        //aqe->actor->temp[TEMP_CHARISMA] -= 1;
                        //if(aqe->actor->temp[TEMP_CHARISMA] == 0)
                                //oe_charisma(aqe->actor, 
                        //break; 
                case ACTION_DECREASE_TEMP_STRENGTH:
                        aqe->actor->temp[TEMP_STRENGTH] -= 1;
                        if(aqe->actor->temp[TEMP_STRENGTH] == 0)
                                oe_strength(aqe->actor, aqe->object, aqe->gain, false);
                        break;
                case ACTION_NOTHING:
                        fullturn = false;
                        //updatescreen = false;
                        break;
                default:
                        fprintf(stderr, "DEBUG: %s:%d - Unknown action %d attempted!\n", __FILE__, __LINE__, aqe->action);
                        gtprintf("DEBUG: %s:%d - Unknown action %d attempted!\n", __FILE__, __LINE__, aqe->action);
                        fullturn = false;
                        //updatescreen = false;
                        break;
        }

        if(cf(ply, plx) & CF_HAS_DOOR_CLOSED) {
                open_door(ply, plx);
                you("open the door!");
                ply = oldy; plx = oldx;
        }

        return fullturn;
}

int get_next_free_action_slot()
{
        int i;

        for(i = 1; i < MAXACT; i++) {
                if(act[i].action == ACTION_FREESLOT)
                        return i;
        }

        return 0;
}

int schedule_action(int action, actor_t *actor)
{
        int i;

        i = get_next_free_action_slot();
        if(!i)
                die("fatal! no free slots in action queue!");

        act[i].action = action;
        act[i].tick = game->tick + actor->speed;
        act[i].actor = actor;
        //gtprintfc(COLOR_SKYBLUE, "Scheduled action %s at tick %d!", action_name[action], act[i].tick);

        return i;
}

int schedule_action_delayed(int action, actor_t *actor, obj_t *object, int delay)
{
        int i;

        i = get_next_free_action_slot();
        if(!i)
                die("fatal! no free slots in action queue!");

        act[i].action = action;
        act[i].tick = game->tick + actor->speed + delay;
        act[i].actor = actor;
        act[i].object = object;
        //gtprintfc(COLOR_SKYBLUE, "Scheduled delayed action %s at tick %d!", action_name[action], act[i].tick);

        return i;
}

int schedule_action_immediately(int action, actor_t *actor)
{
        int i;
        action_t a;

        i = get_next_free_action_slot();
        if(!i)
                die("fatal! no free slots in action queue!");

        a.action = action;
        a.tick = game->tick;


        //gtprintfc(COLOR_SKYBLUE, "Doing action immediately - %s at tick %d (game->tick = %d)!", action_name[action], act[i].tick, game->tick);
        do_action(&a);

        return i;
}

void unschedule_action(int index)
{
        act[index].action  = ACTION_FREESLOT;
        act[index].tick    = 0;
        act[index].monster = 0;
        act[index].object  = 0;
}

void schedule_monster(monster_t *m)
{
        int i;

        i = schedule_action(ACTION_MOVE_MONSTER, m);
        act[i].monster = m;

        //gtprintfc(COLOR_SKYBLUE, "Scheduled monster %s at tick %d", m->name, act[i].tick);
}

void unschedule_all_monsters()
{
        int i;

        for(i = 0; i < MAXACT; i++) {
                if(act[i].action == ACTION_MOVE_MONSTERS || act[i].action == ACTION_MOVE_MONSTER)
                        unschedule_action(i);
        }
}

/**
 * @brief Queue more than one instance of the same action.
 *
 * @param num How many instances.
 * @param action Which action to queue - see \ref group_actions "ACTION-defines" in gt.h
 */
void schedule_actionx(int num, int action, actor_t *actor)
{
        int i;

        for(i=0; i<num; i++)
                schedule_action(action, actor);
}

/**
 * @brief Add several, possibly different, actions to the action queue.
 *
 * @param first The first action to add (see \ref group_actions "ACTION-defines")
 * @param ... Additional actions to add. The last argument has to be ENDOFLIST.
 */
void queuemany(actor_t *actor, int first, ...)
{
        va_list args;
        int i;

        va_start(args, first);
        schedule_action(first, actor);

        i = va_arg(args, int);
        while(i != ENDOFLIST) {
                schedule_action(i, actor);
                i = va_arg(args, int);
        }
        va_end(args);
}

void process_autopickup()
{
        if(ci(ply, plx)) {
                if(ci(ply, plx) && ci(ply, plx)->gold) {
                        if(gtconfig.ap[OT_GOLD])
                                schedule_action_immediately(ACTION_PICKUP, player);
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
                                        schedule_action_immediately(ACTION_PICKUP, player);
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

bool action_at_tick(int tick)
{
        struct actionqueue *tmp;

        tmp = aq->next;
        while(tmp) {
                if(tmp->tick == tick)
                        return true;

                tmp = tmp->next;
        }

        return false;
}

void do_everything_at_tick(int tick)
{
        int i;

        for(i = 0; i < MAXACT; i++) {
                if(act[i].action >= 0) {
                        if(act[i].tick == tick) {
                                do_action(&act[i]);
                                unschedule_action(i);
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
 * @brief Do a complete turn. This should take into account the player's speed, and schedule other actions accordingly.
 */
void do_turn()
{
        int i;

        //gtprintf("Started do_turn. ---------------------------------------");

        //dump_action_queue();
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

        //if(player->temp)
        //        process_temp_effects(player);

        update_screen();

        //gtprintf("Ended do_turn. -----------------------------------------");
}

/**
 * @brief Catch a signal and perform an action (currently, that action is to exit).
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
 * @brief Get a command from the player, and schedule the appropriate action.
 *
 */
void process_player_input()
{
        int c, x, y, l;

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
                case CMD_DOWN:  schedule_action(ACTION_PLAYER_MOVE_DOWN, player); break;
                case CMD_UP:    schedule_action(ACTION_PLAYER_MOVE_UP, player); break;
                case CMD_LEFT:  schedule_action(ACTION_PLAYER_MOVE_LEFT, player); break;
                case CMD_RIGHT: schedule_action(ACTION_PLAYER_MOVE_RIGHT, player); break;
                case CMD_NW:    schedule_action(ACTION_PLAYER_MOVE_NW, player); break;
                case CMD_NE:    schedule_action(ACTION_PLAYER_MOVE_NE, player); break;
                case CMD_SW:    schedule_action(ACTION_PLAYER_MOVE_SW, player); break;
                case CMD_SE:    schedule_action(ACTION_PLAYER_MOVE_SE, player); break;
                case CMD_WIELDWEAR:
                                l = ask_char("Which item would you like to wield/wear?");
                                actiondata = (void *) get_object_from_letter(l, player->inventory);
                                schedule_action(ACTION_WIELDWEAR, player);
                                break;
                case CMD_UNWIELDWEAR:
                                l = ask_char("Which item would you like to remove/unwield?");
                                actiondata = (void *) get_object_from_letter(l, player->inventory);
                                schedule_action(ACTION_UNWIELDWEAR, player);
                                break;
                case CMD_DROP:
                                l = ask_char("Which item would you like to drop?");
                                actiondata = (void *) get_object_from_letter(l, player->inventory);
                                schedule_action(ACTION_DROP, player);
                                break;
                case CMD_LONGDOWN:
                                schedule_actionx(20, ACTION_PLAYER_MOVE_DOWN, player);
                                break;
                case CMD_LONGUP:
                                schedule_actionx(20, ACTION_PLAYER_MOVE_UP, player);
                                break;
                case CMD_LONGLEFT:
                                schedule_actionx(20, ACTION_PLAYER_MOVE_LEFT, player);
                                break;
                case CMD_LONGRIGHT:
                                schedule_actionx(20, ACTION_PLAYER_MOVE_RIGHT, player);
                                break;
                case CMD_TOGGLEFOV:
                                gtprintf("Setting all cells to visible.");
                                set_level_visited(world->curlevel);
                                schedule_action(ACTION_NOTHING, player);
                                break;
                case CMD_SPAWNMONSTER:
                                spawn_monster_at(ply+5, plx+5, ri(1, game->monsterdefs), world->curlevel->monsters, world->curlevel, 100);
                                //dump_monsters(world->curlevel->monsters);
                                schedule_action(ACTION_NOTHING, player);
                                break;
                case CMD_WIZARDMODE:
                                game->wizardmode = (game->wizardmode ? false : true); schedule_action(ACTION_NOTHING, player);
                                gtprintf("Wizard mode %s!", game->wizardmode ? "on" : "off");
                                break;
                case CMD_SAVE:
                                save_game(game->savefile);
                                schedule_action(ACTION_NOTHING, player);
                                break;
                case CMD_LOAD:
                                if(!load_game(game->savefile, 1))
                                        gtprintf("Loading failed!");
                                else
                                        gtprintf("Loading successful!");
                                schedule_action(ACTION_NOTHING, player);
                                break;
                case CMD_DUMPOBJECTS:
                                dump_objects(world->curlevel->c[ply][plx].inventory);
                                schedule_action(ACTION_NOTHING, player);
                                break;
                case CMD_INCFOV:
                                //player->viewradius++;
                                player->speed--;
                                //world->out->lakelimit++;
                                //generate_terrain(1);
                                //gtprintf("lakelimit = %d", world->out->lakelimit);
                                schedule_action(ACTION_NOTHING, player);
                                break;
                case CMD_DECFOV:
                                //player->viewradius--;
                                player->speed++;
                                //world->out->lakelimit--;
                                //generate_terrain(1);
                                //gtprintf("lakelimit = %d", world->out->lakelimit);
                                schedule_action(ACTION_NOTHING, player);
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
                                schedule_action(ACTION_NOTHING, player);
                                break;
                case CMD_INVENTORY:
                                schedule_action(ACTION_NOTHING, player);
                                dump_objects(player->inventory);
                                break;
                case CMD_PICKUP:
                                schedule_action(ACTION_PICKUP, player);
                                break;
                case CMD_QUAFF:
                                l = ask_char("Which potion would you like to drink?");
                                actiondata = (void *) get_object_from_letter(l, player->inventory);
                                schedule_action(ACTION_QUAFF, player);
                                break;
                case CMD_DESCEND:
                                if(hasbit(cf(ply,plx), CF_HAS_STAIRS_DOWN)) {
                                        schedule_action(ACTION_GO_DOWN_STAIRS, player);
                                        schedule_action(ACTION_FIX_VIEW, player);
                                        unschedule_all_monsters();
                                } else {
                                        gtprintf("You can't go up here!");
                                }
                                break;
                case CMD_ASCEND:
                                if(hasbit(cf(ply,plx), CF_HAS_STAIRS_UP)) {
                                        schedule_action(ACTION_GO_UP_STAIRS, player);
                                        schedule_action(ACTION_FIX_VIEW, player);
                                        unschedule_all_monsters();
                                } else {
                                        gtprintf("You can't go up here!");
                                }
                                break;
                case CMD_REST:
                                schedule_action(ACTION_NOTHING, player);
                                break;
                case CMD_PATHFINDER: break;
                case CMD_AUTOEXPLORE:
                                     autoexplore();
                                     break;
                case CMD_DUMPAQ:
                                     dump_action_queue();
                                     break;
                default:
                                     schedule_action(ACTION_NOTHING, player);
                                     break;
        }

}

void game_loop()
{
        do {
                do_turn();
        } while(!game->dead);
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
                        die("Couldn't open file %s\n", game->savefile);

                sprintf(messagefilename, "%s/messages.%d.gtsave", SAVE_DIRECTORY, game->seed);
                messagefile = fopen(messagefilename, "a");
                
                init_display();
                
                // these next should be loaded by load_game?!
                world->cmap = world->dng[game->currentlevel].c;
                world->curlevel = &world->dng[game->currentlevel];
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
                do_one_action(ACTION_GO_DOWN_STAIRS);
                fixview();
        }

        init_commands();
        init_pathfinding(player);
        initial_update_screen();
        player->ticks = 0;
        schedule_action_delayed(ACTION_PLAYER_NEXTMOVE, player, 0, 1);
        //schedule_action_delayed(ACTION_PLAYER_NEXTMOVE, player, player->speed);
        //schedule_action_delayed(ACTION_PLAYER_NEXTMOVE, player, player->speed * 2);
        //schedule_action_delayed(ACTION_PLAYER_NEXTMOVE, player, player->speed * 3);
        //schedule_action_delayed(ACTION_PLAYER_NEXTMOVE, player, player->speed * 4);

        game_loop();

        shutdown_display();
        shutdown_gt();

        return 0;
}
