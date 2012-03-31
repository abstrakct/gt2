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

#include "cards.h"
#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "world.h"
#include "datafiles.h"
#include "display.h"
#include "debug.h"
#include "saveload.h"
#include "commands.h"
#include "gt.h"

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
monster_t *monsterdefs;
obj_t     *objdefs;
game_t    *game;
world_t   *world;
actor_t   *player;
struct actionqueue *aq;
gt_config_t gtconfig;
long actionnum;
FILE *messagefile;
bool mapchanged;
int tempxsize, tempysize;
bool loadgame;
void *actiondata;

actor_t *a_attacker, *a_victim;

// Messages
message_t messages[500];
int currmess, maxmess;

#ifdef GT_USE_NCURSES
// Ncurses stuff
WINDOW *wall;
WINDOW *wstat;
WINDOW *winfo;
WINDOW *wmap;
WINDOW *wleft;
#endif

struct option gt_options[] = {
        { "seed",    1,   0, 's' },
        { "load",    1,   0, 'l' },
        { "version", 0,   0, 'v' },
        { NULL,      0, NULL, 0  }
};

void init_variables()
{
        garbageindex = 0;

        monsterdefs = (monster_t *) gtmalloc(sizeof(monster_t));
        monsterdefs->head = monsterdefs;
        mid_counter = 1000;

        objdefs = (obj_t *) gtmalloc(sizeof(obj_t));
        objdefs->head = objdefs;

        aq = (struct actionqueue *) gtmalloc(sizeof(struct actionqueue));
        aq->head = aq;
        aq->next = 0;
        aq->action = ACTION_NOTHING;
        actionnum = 0;

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

        game->wizardmode = false;
        player = (actor_t *) gtmalloc(sizeof(actor_t));
}

/*********************************************
* Description - initialize player
* Author - RK
* Date - Dec 14 2011
* *******************************************/
void init_player()
{
        // TODO: Character generation!!
        plx = game->mapw / 2;
        ply = game->maph / 2;
        ppx = plx - game->mapw / 2;
        ppy = ply - game->maph / 2;
        game->mapcx = game->mapw + 2;
        game->mapcy = game->maph + 2;
        player->viewradius = 16;
        player->level = 1;

        player->attr.str  = dice(3, 6, 0);
        player->attr.dex  = dice(3, 6, 0);
        player->attr.phy  = dice(3, 6, 0);
        player->attr.wis  = dice(3, 6, 0);
        player->attr.cha  = dice(3, 6, 0);
        player->attr.intl = dice(3, 6, 0);

        // TODO: Starting HP - FIX according to race etc.
        player->hp = player->maxhp = (dice(1, 10, 7)) + ability_modifier(player->attr.phy);

        strcpy(player->name, "Whiskeyjack");
}

void shutdown_gt()
{
        int i;

        i = garbageindex;
        while(i >= 0) {
               i--;
               if(garbage[i])
                       free((void *)garbage[i]);
        }
        
        fclose(messagefile);
}

/*
 * The following (parse_commandline) is muchly stolen
 * from getopt's wikipedia page
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

void fixview()
{
        ppx = plx - (game->mapw / 2);
        ppy = ply - (game->maph / 2);

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

/*********************************************
* Description - Do an action specified by parameter action
* Author - RK
* Date - Dec 14 2011
* *******************************************/
bool do_action(int action)
{
        int oldy, oldx;
        int tmpy, tmpx;
        bool fullturn;
        obj_t *o;
        int i;

        oldy = ply; oldx = plx;
        fullturn = true;

        switch(action) {
                case ACTION_PLAYER_MOVE_DOWN:
                        if(passable(world->curlevel, ply+1, plx)) {
                                if(world->curlevel->c[ply+1][plx].monster) {
                                        //gtprintf("You attack the %s!", world->curlevel->c[ply+1][plx].monster->name);
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply+1][plx].monster;
                                        queue(ACTION_ATTACK);
                                        fullturn = false;
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
                        player->ticks -= TICKS_MOVEMENT;
                        break;
                case ACTION_PLAYER_MOVE_UP:
                        if(passable(world->curlevel, ply-1,plx)) {
                                if(world->curlevel->c[ply-1][plx].monster) {
                                        //gtprintf("You attack the %s!", world->curlevel->c[ply-1][plx].monster->name);
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply-1][plx].monster;
                                        queue(ACTION_ATTACK);
                                        fullturn = false;
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
                        player->ticks -= TICKS_MOVEMENT;
                        break;
                case ACTION_PLAYER_MOVE_LEFT:
                        if(passable(world->curlevel, ply, plx-1)) {
                                if(world->curlevel->c[ply][plx-1].monster) {
                                        //gtprintf("You attack the %s!", world->curlevel->c[ply][plx-1].monster->name);
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply][plx-1].monster;
                                        queue(ACTION_ATTACK);
                                        fullturn = false;
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
                        player->ticks -= TICKS_MOVEMENT;
                        break;
                case ACTION_PLAYER_MOVE_RIGHT:
                        if(passable(world->curlevel, ply,plx+1)) {
                                if(world->curlevel->c[ply][plx+1].monster) {
                                        //gtprintf("You attack the %s!", world->curlevel->c[ply][plx+1].monster->name);
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply][plx+1].monster;
                                        queue(ACTION_ATTACK);
                                        fullturn = false;
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
                        player->ticks -= TICKS_MOVEMENT;
                        break;
                case ACTION_PLAYER_MOVE_NW:
                        if(passable(world->curlevel, ply-1,plx-1)) {
                                if(world->curlevel->c[ply-1][plx-1].monster) {
                                        //gtprintf("You attack the %s!", world->curlevel->c[ply-1][plx-1].monster->name);
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply-1][plx-1].monster;
                                        queue(ACTION_ATTACK);
                                        fullturn = false;
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
                        player->ticks -= TICKS_MOVEMENT;
                        break;
                case ACTION_PLAYER_MOVE_NE:
                        if(passable(world->curlevel, ply-1,plx+1)) {
                                if(world->curlevel->c[ply-1][plx+1].monster) {
                                        //gtprintf("You attack the %s!", world->curlevel->c[ply-1][plx+1].monster->name);
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply-1][plx+1].monster;
                                        queue(ACTION_ATTACK);
                                        fullturn = false;
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
                        player->ticks -= TICKS_MOVEMENT;
                        break;
                case ACTION_PLAYER_MOVE_SW:
                        if(passable(world->curlevel, ply+1, plx-1)) {
                                if(world->curlevel->c[ply+1][plx-1].monster) {
                                        //gtprintf("You attack the %s!", world->curlevel->c[ply+1][plx-1].monster->name);
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply+1][plx-1].monster;
                                        queue(ACTION_ATTACK);
                                        fullturn = false;
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
                        player->ticks -= TICKS_MOVEMENT;
                        break;
                case ACTION_PLAYER_MOVE_SE:
                        if(passable(world->curlevel, ply+1, plx+1)) {
                                if(world->curlevel->c[ply+1][plx+1].monster) {
                                        //gtprintf("You attack the %s!", world->curlevel->c[ply+1][plx+1].monster->name);
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply+1][plx+1].monster;
                                        queue(ACTION_ATTACK);
                                        fullturn = false;
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
                        player->ticks -= TICKS_MOVEMENT;
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
                        player->ticks -= TICKS_MOVEMENT;
                        break;
                case ACTION_ATTACK:
                        attack(a_attacker, a_victim);
                        player->ticks -= TICKS_ATTACK;
                        break;
                case ACTION_MOVE_MONSTERS:
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
                                        player->viewradius = 16;
                                else
                                        player->viewradius = 8;
                        }
                        player->ticks -= TICKS_MOVEMENT;
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
                                player->viewradius = 45;
                        }
                        player->ticks -= TICKS_MOVEMENT;
                        break;
                case ACTION_WIELDWEAR:
                        o = (obj_t *) actiondata;
                        if(o)
                                wieldwear(o);
                        else
                                gtprintf("HUH????????????????????");
                        player->ticks -= TICKS_WIELDWEAR;
                        break;
                case ACTION_UNWIELDWEAR:
                        o = (obj_t *) actiondata;
                        if(o)
                                unwieldwear(o);
                        else
                                gtprintf("HUH????????????????????");

                        player->ticks -= TICKS_WIELDWEAR;
                        break;
                case ACTION_QUAFF:
                        o = (obj_t *) actiondata;
                        if(o)
                                quaff(o, player);
                        else
                                gtprintf("Huh? I don't understand.");
                        player->ticks -= TICKS_MOVEMENT;
                        break;
                case ACTION_DROP:
                        o = (obj_t *) actiondata;
                        if(o)
                                drop(o, player);
                        else
                                gtprintf("Drop what?");
                        player->ticks -= TICKS_WIELDWEAR;
                        break;  
                case ACTION_FIX_VIEW:
                        fixview();
                        break;
                case ACTION_HEAL_PLAYER:
                        i = 25 - pphy;
                        if(i < 0)
                                i = 1;

                        if(!(game->turn % i)) {
                                if(perc(40+pphy))
                                        increase_hp(player, 1);
                        }

                        fullturn = false;
                        break;
                case ACTION_MAKE_DISTANCEMAP:
                        makedistancemap(player->y, player->x);
                        fullturn = false;
                        break;
                case ACTION_NOTHING:
                        //updatescreen = false;
                        break;
                default:
                        fprintf(stderr, "DEBUG: %s:%d - Unknown action %d attempted!\n", __FILE__, __LINE__, action);
                        gtprintf("DEBUG: %s:%d - Unknown action %d attempted!\n", __FILE__, __LINE__, action);
                        fullturn = false;
                        //updatescreen = false;
                        break;
        }

        if(cf(ply, plx) & CF_HAS_DOOR_CLOSED) {
                clearbit(cf(ply, plx), CF_HAS_DOOR_CLOSED);       // move to its own funtcion?!"¤&¤%"%
                setbit(cf(ply, plx), CF_HAS_DOOR_OPEN);
                you("open the door!");
                ply = oldy; plx = oldx;
        }


        return fullturn;
}

/*********************************************
* Description - Add action to action queue
* Parameters: int action = ACTION_#define to add
* Author - RK
* Date - Dec 14 2011
* *******************************************/
void queue(int action)
{
        struct actionqueue *tmp, *prev;

        prev = aq;
        tmp = aq->next;
        
        while(tmp) {
                prev = tmp;
                tmp = tmp->next; 
        }

        tmp = (struct actionqueue *) gtmalloc(sizeof(struct actionqueue));
        tmp->head = aq;
        tmp->next = 0;
        tmp->action = action;
        actionnum++;
        tmp->num = actionnum;
        prev->next = tmp;
        aq->num++;
}

/*
 *  Queue num actions
 */
void queuex(int num, int action)
{
        int i;

        for(i=0; i<num; i++)
                queue(action);
}

/*
 * Queue up many actions. Argument list must{

 * end with ENDOFLIST
 */
void queuemany(int first, ...)
{
        va_list args;
        int i;

        va_start(args, first);
        queue(first);

        i = va_arg(args, int);
        while(i != ENDOFLIST) {
                queue(i);
                i = va_arg(args, int);
        }
        va_end(args);
}

/*********************************************
* Description - Process the first action in the action queue
* Author - RK
* Date - Dec 14 2011
* *******************************************/
bool do_next_thing_in_queue() // needs a better name..
{
        bool ret;
        struct actionqueue *tmp;

        tmp = aq->next;
        ret = false;

        if(tmp) {
                ret = do_action(tmp->action);
                aq->num--;
                aq->next = tmp->next;
                gtfree(tmp);
        }

        return ret;
}

bool do_all_things_in_queue() // needs a better name..
{
        struct actionqueue *tmp;
        bool ret;

        tmp = aq->next;
        ret = false;

        while(tmp) {
                ret = do_next_thing_in_queue();
                tmp = tmp->next;
        }

        return ret;
}

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
                        if(gtconfig.ap[OT_GOLD])
                                do_action(ACTION_PICKUP);
                        else
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
                                if(gtconfig.ap[ob->type] && !hasbit(ob->flags, OF_DONOTAP)) {
                                        do_action(ACTION_PICKUP);
                                } else {
                                        if(is_pair(ob))
                                                gtprintf("There is a pair of %s here.", ob->fullname);
                                        else
                                                gtprintf("There is %s here.", a_an(ob->fullname));
                                }
                        }

                        if(ci(ply, plx)->num_used == 2) {
                                int slot, slot2;

                                slot  = get_first_used_slot(ci(ply, plx));
                                slot2 = get_next_used_slot_after(slot, ci(ply, plx));
                                if(slot < 0)
                                        return;
                                if(slot2 < 0)
                                        return;

                                /*if(is_pair(ci(ply, plx)->object[slot]))
                                        gtprintf("There is a pair of %s here.", ci(ply, plx)->object[slot]->fullname);
                                else*/
                                        gtprintf("There is %s and %s here.", a_an(ci(ply, plx)->object[slot]->fullname), a_an(ci(ply, plx)->object[slot2]->fullname));
                        }

                        if(ci(ply, plx)->num_used > 2) {
                                int i;

                                gtprintfc(COLOR_INFO, "There are several things here:");
                                for(i=0;i<52;i++) {
                                        if(ci(ply, plx)->object[i])
                                                gtprintf("%s", a_an(ci(ply, plx)->object[i]->fullname));
                                }
                        }
                }
        }
}

void do_turn()
{
       // bool fullturn;
        int i, ret;

        player->ticks += 1000;


        if(game->currentlevel)
                queue(ACTION_MAKE_DISTANCEMAP);

        queue(ACTION_MOVE_MONSTERS);
        
        if(game->turn % 5)                      // TODO: Better condition... based on physique etc.
                queue(ACTION_HEAL_PLAYER);

        i = aq->num;

        while(i) {
                i = aq->num;

                ret = do_next_thing_in_queue();
                        
                if(ret) {
                        game->turn++;
                        look();
                }

                draw_world(world->curlevel);
                draw_wstat();
                update_screen();
        }
}

int main(int argc, char *argv[])
{
        int c, x, y, l;
        char messagefilename[50];

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
                do_action(ACTION_GO_DOWN_STAIRS);
                fixview();
        }

        init_commands();

        draw_world(world->curlevel);
        draw_wstat();
        initial_update_screen();

        do {
                c = get_command();

                mapchanged = false;
                player->oldx = plx;
                player->oldy = ply;

                switch(c) {
                        case CMD_QUIT:
                                queue(ACTION_NOTHING);
                                game->dead = 1;
                                break;
                        case CMD_DOWN:  queue(ACTION_PLAYER_MOVE_DOWN); break;
                        case CMD_UP:    queue(ACTION_PLAYER_MOVE_UP); break;
                        case CMD_LEFT:  queue(ACTION_PLAYER_MOVE_LEFT); break;
                        case CMD_RIGHT: queue(ACTION_PLAYER_MOVE_RIGHT); break;
                        case CMD_NW:    queue(ACTION_PLAYER_MOVE_NW); break;
                        case CMD_NE:    queue(ACTION_PLAYER_MOVE_NE); break;
                        case CMD_SW:    queue(ACTION_PLAYER_MOVE_SW); break;
                        case CMD_SE:    queue(ACTION_PLAYER_MOVE_SE); break;
                        case CMD_WIELDWEAR:
                                       l = ask_char("Which item would you like to wield/wear?");
                                       actiondata = (void *) get_object_from_letter(l, player->inventory);
                                       queue(ACTION_WIELDWEAR);
                                       break;
                        case CMD_UNWIELDWEAR:
                                       l = ask_char("Which item would you like to remove/unwield?");
                                       actiondata = (void *) get_object_from_letter(l, player->inventory);
                                       queue(ACTION_UNWIELDWEAR);
                                       break;
                        case CMD_DROP:
                                       l = ask_char("Which item would you like to drop?");
                                       actiondata = (void *) get_object_from_letter(l, player->inventory);
                                       queue(ACTION_DROP);
                                       break;
                        case CMD_LONGDOWN:
                                queuex(20, ACTION_PLAYER_MOVE_DOWN);
                                break;
                        case CMD_LONGUP:
                                queuex(20, ACTION_PLAYER_MOVE_UP);
                                break;
                        case CMD_LONGLEFT:
                                queuex(20, ACTION_PLAYER_MOVE_LEFT);
                                break;
                        case CMD_LONGRIGHT:
                                queuex(20, ACTION_PLAYER_MOVE_RIGHT);
                                break;
                        case CMD_TOGGLEFOV:
                                gtprintf("Setting all cells to visible.");
                                set_level_visited(world->curlevel);
                                queue(ACTION_NOTHING);
                                break;
                        case CMD_SPAWNMONSTER:
                                spawn_monster_at(ply+5, plx+5, ri(1, game->monsterdefs), world->curlevel->monsters, world->curlevel, 100);
                                //dump_monsters(world->curlevel->monsters);
                                queue(ACTION_NOTHING);
                                break;
                        case CMD_WIZARDMODE:
                                game->wizardmode = (game->wizardmode ? false : true); queue(ACTION_NOTHING);
                                gtprintf("Wizard mode %s!", game->wizardmode ? "on" : "off");
                                break;
                        case CMD_SAVE:
                                save_game(game->savefile);
                                queue(ACTION_NOTHING);
                                break;
                        case CMD_LOAD:
                                if(!load_game(game->savefile, 1))
                                        gtprintf("Loading failed!");
                                else
                                        gtprintf("Loading successful!");
                                queue(ACTION_NOTHING);
                                break;
                        case CMD_DUMPOBJECTS:
                                dump_objects(world->curlevel->c[ply][plx].inventory);
                                queue(ACTION_NOTHING);
                                break;
                        case CMD_INCFOV:
                                player->viewradius++;
                                //world->out->lakelimit++;
                                //generate_terrain(1);
                                //gtprintf("lakelimit = %d", world->out->lakelimit);
                                queue(ACTION_NOTHING);
                                break;
                        case CMD_DECFOV:
                                player->viewradius--;
                                //world->out->lakelimit--;
                                //generate_terrain(1);
                                //gtprintf("lakelimit = %d", world->out->lakelimit);
                                queue(ACTION_NOTHING);
                                break;
                        case CMD_DUMPCOLORS:
                                for(x = 0;  x < 64; x++) {
                                        /*gtprintfwc(wstat, x, "This is color %d  ", x);
                                        wattron(wstat, A_BOLD);
                                        gtprintfwc(wstat, x, "This is BOLD color %d  ", x);
                                        wattroff(wstat, A_BOLD);*/
                                }
                                queue(ACTION_NOTHING);
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
                                queue(ACTION_NOTHING);
                                break;
                        case CMD_INVENTORY:
                                queue(ACTION_NOTHING);
                                dump_objects(player->inventory);
                                break;
                        case CMD_PICKUP:
                                queue(ACTION_PICKUP);
                                break;
                        case CMD_QUAFF:
                                l = ask_char("Which potion would you like to drink?");
                                actiondata = (void *) get_object_from_letter(l, player->inventory);
                                queue(ACTION_QUAFF);
                                break;
                        case CMD_DESCEND:
                                if(hasbit(cf(ply,plx), CF_HAS_STAIRS_DOWN)) {
                                        queue(ACTION_GO_DOWN_STAIRS);
                                        queue(ACTION_FIX_VIEW);
                                } else {
                                        gtprintf("You can't go up here!");
                                        queue(ACTION_NOTHING);
                                }
                                break;
                        case CMD_ASCEND:
                                if(hasbit(cf(ply,plx), CF_HAS_STAIRS_UP)) {
                                        queue(ACTION_GO_UP_STAIRS);
                                        queue(ACTION_FIX_VIEW);
                                } else {
                                        gtprintf("You can't go up here!");
                                        queue(ACTION_NOTHING);
                                }
                                break;
                        case CMD_REST:
                                queue(ACTION_NOTHING);
                                break;
                        case CMD_PATHFINDER:
                                pathfinder(world->curlevel, player->y, player->x, player->y + ri(-15,15), player->x + ri(-15,15));
                                queue(ACTION_NOTHING);
                                break;
                        default:
                                queue(ACTION_NOTHING);
                                game->turn--;
                                break;
                }

                do_turn();
        } while(!game->dead);

        shutdown_display();
        shutdown_gt();

        return 0;
}
