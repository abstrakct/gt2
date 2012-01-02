/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */

#define _XOPEN_SOURCE_EXTENDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <time.h>
#include <signal.h>
#include <libconfig.h>
#include <stdbool.h>

#ifdef GT_USE_NCURSES
#include <curses.h>
#endif

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "you.h"
#include "display.h"
#include "debug.h"
#include "gt.h"

char *otypestrings[50] = {
        "Weapon",
        "Armor",
        "Ring",
        "Card",
        "Wand",
        "Thing",
        "Gold",
        "",
        "",
        "",
        "",
        "Body armor"
};

// Important global variables
monster_t *monsterdefs;
obj_t *objdefs;
game_t *game;
world_t *world;
actor_t *player;
struct actionqueue *aq;
gt_config_t gtconfig;
long actionnum;
FILE *messagefile;
int mapcx, mapcy;
bool mapchanged;
int tempxsize, tempysize;

// Messages
message_t m[500];
int currmess, maxmess;

#ifdef GT_USE_NCURSES
// Ncurses stuff
WINDOW *wall;
WINDOW *wstat;
WINDOW *winfo;
WINDOW *wmap;
#endif


void init_variables()
{
        monsterdefs = (monster_t *) gtmalloc(sizeof(monster_t));
        memset(monsterdefs, 0, sizeof(monster_t));
        monsterdefs->head = monsterdefs;

        objdefs = (obj_t *) gtmalloc(sizeof(obj_t));
        memset(objdefs, 0, sizeof(obj_t));
        objdefs->head = objdefs;

        aq = (struct actionqueue *) gtmalloc(sizeof(struct actionqueue));
        aq->head = aq;
        aq->next = 0;
        aq->action = ACTION_NOTHING;
        actionnum = 0;

        world = (world_t *) gtmalloc(sizeof(world_t));
        memset(world, 0, sizeof(world_t));

        world->dng = gtcalloc(4, sizeof(level_t));    // allocate 4 levels, 0 = outside, 1..n = dungeons
        world->out = world->dng;                      // i.e. it points to world->dng[0]
        world->out->xsize = XSIZE;
        world->out->ysize = YSIZE;
        init_level(world->out);

        game = (game_t *) gtmalloc(sizeof(game_t));
        game->dead = 0;
        game->seed = time(0);
        srand(game->seed);
        game->wizardmode = false;
fprintf(stderr, "DEBUG: %s:%d - Random seed is %d\n", __FILE__, __LINE__, game->seed);
        
        player = (actor_t *) gtmalloc(sizeof(actor_t));
}

/*********************************************
* Description - initialize player
* Author - RK
* Date - Dec 14 2011
* *******************************************/
void init_player()
{
        plx = game->mapw / 2;
        ply = game->maph / 2;
        ppx = plx - game->mapw / 2;
        ppy = ply - game->maph / 2;
        mapcx = game->mapw + 2;
        mapcy = game->maph + 2;
        player->viewradius = 50;
}

void shutdown_gt()
{
        monster_t *n, *m;

        n = monsterdefs->head->next;
        while(n) {
                m = n->next;
                if(n)
                        free(n);
                n = m;
        }
        
        //free(monsterdefs->head);
        free(world->village);
        free(world->city);
        free(world->forest);

        fclose(messagefile);
}

/*********************************************
* Description - Do an action specified by parameter action
* Author - RK
* Date - Dec 14 2011
* *******************************************/
void do_action(int action)
{
        switch(action) {
                case ACTION_PLAYER_MOVE_DOWN:
                        if(passable(ply+1, plx))
                                ply++;
         //               if(ply >= world->curlevel->ysize)
         //                       ply = world->curlevel->ysize-1;
                        if(ply >= (ppy + (mapcy/6*5))) {
                                mapchanged = true;
                                ppy++;
                        }
                        if(ppy >= world->curlevel->ysize-mapcy) {
                                mapchanged = true;
                                ppy = world->curlevel->ysize - mapcy - 1;
                        }
                        if(ppy < 0)
                                ppy = 0;
                        break;
                case ACTION_PLAYER_MOVE_UP:
                        if(passable(ply-1,plx))
                                ply--;
                        if(ply < 0)
                                ply = 0;
                        if(ply <= (ppy + (mapcy/6))) {
                                mapchanged = true;
                                ppy--;
                        }
                        if(ppy < 0)
                                ppy = 0;
                        break;
                case ACTION_PLAYER_MOVE_LEFT:
                        if(passable(ply, plx-1))
                                plx--;
                        if(plx < 0)
                                plx = 0;
                        if(plx <= (ppx+(mapcx/6))) {
                                mapchanged = true;
                                ppx--;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case ACTION_PLAYER_MOVE_RIGHT:
                        if(passable(ply,plx+1))
                                plx++;
        //                if(plx >= world->curlevel->xsize)
        //                        plx = world->curlevel->xsize-1;
                        if(plx >= (ppx+(mapcx/6*5))) {
                                mapchanged = true;
                                ppx++;
                        }
                        if(ppx >= world->curlevel->xsize-mapcx) {
                                mapchanged = true;
                                ppx = world->curlevel->xsize-mapcx-1;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case ACTION_PLAYER_MOVE_NW:
                        if(passable(ply-1,plx-1)) {
                                ply--;
                                plx--;
                        }
                        if(ply < 0)
                                ply = 0;
                        if(ply <= (ppy + (mapcy/6))) {
                                mapchanged = true;
                                ppy--;
                        }
                        if(ppy < 0)
                                ppy = 0;

                        if(plx < 0)
                                plx = 0;
                        if(plx <= (ppx + (mapcx/6))) {
                                mapchanged = true;
                                ppx--;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case ACTION_PLAYER_MOVE_NE:
                        if(passable(ply-1,plx+1)) {
                                ply--; plx++;
                        }
                        
                        if(plx >= world->curlevel->xsize)
                                plx = world->curlevel->xsize-1;
                        if(plx >= (ppx+(mapcx/6*5))) {
                                mapchanged = true;
                                ppx++;
                        }
                        if(ppx >= world->curlevel->xsize-mapcx) {
                                mapchanged = true;
                                ppx = world->curlevel->xsize-mapcx-1;
                        }
                        if(ppx < 0)
                                ppx = 0;

                        if(ply < 0)
                                ply = 0;
                        if(ply <= (ppy+(mapcy/6))) {
                                mapchanged = true;
                                ppy--;
                        }
                        if(ppy < 0)
                                ppy = 0;
                        break;
                case ACTION_PLAYER_MOVE_SW:
                        if(passable(ply+1, plx-1)) {
                                ply++; plx--;
                        }

                        if(ply >= world->curlevel->ysize)
                                ply = world->curlevel->ysize-1;
                        if(ply >= (ppy+(mapcy/6*5))) {
                                mapchanged = true;
                                ppy++;
                        }
                        if(ppy >= world->curlevel->ysize-mapcy) {
                                mapchanged = true;
                                ppy = world->curlevel->ysize-mapcy-1;
                        }
                        if(ppy < 0)
                                ppy = 0;

                        //if(plx < 0)
                        //        plx = 0;
                        if(plx <= (ppx+(mapcx/6))) {
                                mapchanged = true;
                                ppx--;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case ACTION_PLAYER_MOVE_SE:
                        if(passable(ply+1, plx+1)) {
                                ply++; plx++;
                        }
                        if(ply >= world->curlevel->ysize)
                                ply = world->curlevel->ysize-1;
                        if(ply >= (ppy+(mapcy/6*5))) {
                                mapchanged = true;
                                ppy++;
                        }
                        if(ppy >= world->curlevel->ysize - mapcy) {
                                mapchanged = true;
                                ppy = world->curlevel->ysize - mapcy - 1;
                        }
                        if(ppy < 0)
                                ppy = 0;

                        if(plx >= world->curlevel->xsize)
                                plx = world->curlevel->xsize - 1;
                        if(plx >= (ppx+(mapcx/6*5))) {
                                mapchanged = true;
                                ppx++;
                        }
                        if(ppx >= world->curlevel->xsize - mapcx) {
                                mapchanged = true;
                                ppx = world->curlevel->xsize - mapcx-1;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case ACTION_NOTHING:
                        break;
                default:
                        fprintf(stderr, "DEBUG: %s:%d - Unknown action %d attemted!\n", __FILE__, __LINE__, action);
                        break;
        }
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
}

/*********************************************
* Description - Process the first action in the action queue
* Author - RK
* Date - Dec 14 2011
* *******************************************/
void do_one_thing_in_queue() // needs a better name..
{
        struct actionqueue *tmp;

        tmp = aq->next;

        if(tmp) {
                do_action(tmp->action);
                aq->next = tmp->next;
                free(tmp);
        }
}

void do_all_things_in_queue() // needs a better name..
{
        struct actionqueue *tmp;

        tmp = aq->next;

        while(tmp) {
                do_one_thing_in_queue();
                tmp = tmp->next;
        }
}

int main(int argc, char *argv[])
{
        int c, x;

        if(!setlocale(LC_ALL, ""))
                die("couldn't set locale.");

        messagefile = fopen("messages.txt", "w");
        printf("Gullible's Travails v%s\n", get_version_string());

        init_variables();

        printf("Reading data files...\n");
        if(parse_data_files())
                die("Couldn't parse data files.");

        generate_world();
        world->cmap = world->out->c;

        init_display();
        init_player();

        world->curlevel = world->out;
        game->context = CONTEXT_OUTSIDE;
        draw_world(world->curlevel);
        initial_update_screen();

        do {
                bool do_all = false;

                c = gtgetch();
                mapchanged = false;
                player->oldx = plx;
                player->oldy = ply;

                switch(c) {
                        case 'q':
                                queue(ACTION_NOTHING);
                                game->dead = 1;
                                break;
                        case 'd':
                                queue(ACTION_NOTHING);
                                if(world->cmap == world->out->c) {
                                        world->cmap = world->dng[1].c;
                                        world->curlevel = &(world->dng[1]);
                                        game->context = CONTEXT_DUNGEON;

                                        ply = ri((world->curlevel->ysize/2) - 25, (world->curlevel->ysize/2) + 25);
                                        plx = ri((world->curlevel->xsize/2) - 25, (world->curlevel->xsize/2) + 25);
                                        while(world->cmap[ply][plx].type != DNG_FLOOR) {
                                                ply = ri((world->curlevel->ysize/2) - 25, (world->curlevel->ysize/2) + 25);
                                                plx = ri((world->curlevel->xsize/2) - 25, (world->curlevel->xsize/2) + 25);
                                        }

                                        ppy = ply - (game->maph / 2);
                                        ppx = plx - (game->mapw / 2);
                                        
                                        if(ppy <= 0)
                                                ppy = 0;
                                        if(ppx <= 0)
                                                ppx = 0;
                                        
                                        player->viewradius = 5;
                                } else {
                                        world->cmap = world->out->c;
                                        world->curlevel = world->out;
                                        game->context = CONTEXT_OUTSIDE;
                                        player->viewradius = 50;
                                }
                                break;
                        case 'f':
                                // this should ensure floodfill working every time!
                                x = ri(11,111);
                                while(world->dng[1].c[x][x].type != DNG_FLOOR) {
                                        x = ri(11,111);
                                }
                                gtprintf("floodfilling from %d, %d\n", x, x);
                                floodfill(x, x);
                                queue(ACTION_NOTHING);
                                break;
                        case 'j': queue(ACTION_PLAYER_MOVE_DOWN); break;
                        case 'k': queue(ACTION_PLAYER_MOVE_UP); break;
                        case 'h': queue(ACTION_PLAYER_MOVE_LEFT); break;
                        case 'l': queue(ACTION_PLAYER_MOVE_RIGHT); break;
                        case 'y': queue(ACTION_PLAYER_MOVE_NW); break;
                        case 'u': queue(ACTION_PLAYER_MOVE_NE); break;
                        case 'b': queue(ACTION_PLAYER_MOVE_SW); break;
                        case 'n': queue(ACTION_PLAYER_MOVE_SE); break;
                        case 'J':
                                for(x=0;x<20;x++)
                                        queue(ACTION_PLAYER_MOVE_DOWN);
                                do_all = true;
                                break;
                        case 'K':
                                for(x=0;x<20;x++)
                                        queue(ACTION_PLAYER_MOVE_UP);
                                do_all = true;
                                break;
                        case 'H':
                                for(x=0;x<20;x++)
                                        queue(ACTION_PLAYER_MOVE_LEFT);
                                do_all = true;
                                break;
                        case 'L':
                                for(x=0;x<20;x++)
                                        queue(ACTION_PLAYER_MOVE_RIGHT);
                                do_all = true;
                                break;
                        case 'v':
                                set_all_visible(); queue(ACTION_NOTHING); break;
                        case 's':
                                spawn_monster_at(plx+5, ply+5, ri(0, game->monsterdefs), world->curlevel->monsters, world->curlevel);
                                dump_monsters(world->curlevel->monsters);
                                queue(ACTION_NOTHING);
                                break;
                        case 'w':
                                game->wizardmode = (game->wizardmode ? false : true); queue(ACTION_NOTHING); break;
                        case 'a': dump_action_queue();
                        default: queue(ACTION_NOTHING); break;
                }

                if(!do_all) {
                        do_one_thing_in_queue();
                } else {
                        do_all_things_in_queue();
                        do_all = false;
                }

                draw_world(world->curlevel);
                gtprintf("player x,y = %d, %d\tpx,py = %d, %d\tmapcx,y = %d,%d", plx, ply, ppx, ppy, mapcx, mapcy);
                update_screen();
        } while(!game->dead);

        shutdown_display();
        shutdown_gt();

        return 0;
}
