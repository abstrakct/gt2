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
#include "monsters.h"
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "you.h"
#include "display.h"
#include "actor.h"
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
FILE *messagefile;
int mapcx, mapcy;

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

        world = (world_t *) gtmalloc(sizeof(world_t));
        memset(world, 0, sizeof(world_t));

        game = (game_t *) gtmalloc(sizeof(game_t));
        game->dead = 0;
        game->seed = time(0);
        game->vx = game->vy = 0;
        srand(game->seed);
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
        player->x = game->mapw / 2;
        player->y = game->maph / 2;
        player->px = player->x - game->mapw / 2;
        player->py = player->y - game->maph / 2;
        mapcx = game->mapw + 2;
        mapcy = game->maph + 2;
        player->viewradius = 10;
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
                case ACTION_NOTHING:
                        break;
                case ACTION_PLAYER_MOVE_DOWN:
                        player->y++;
                        if(world->cmap[player->y][player->x].type == DNG_WALL)
                                player->y--;
                        if(player->y >= YSIZE-6)
                                player->y = YSIZE-7;
                        if(player->y >= (player->py + (mapcy/6*5)))
                                player->py++;
                        if(player->py >= YSIZE-mapcy-2)
                                player->py = YSIZE - mapcy - 3;
                        break;
                case ACTION_PLAYER_MOVE_UP:
                        player->y--;
                        if(world->cmap[player->y][player->x].type == DNG_WALL)
                                player->y++;
                        if(player->y < 3)
                                player->y = 3;
                        if(player->y <= (player->py + (mapcy/6)))
                                player->py--;
                        if(player->py < 0)
                                player->py = 0;
                        break;
                case ACTION_PLAYER_MOVE_LEFT:
                        player->x--;
                        if(world->cmap[player->y][player->x].type == DNG_WALL)
                                player->x++;
                        if(player->x < 3)
                                player->x = 3;
                        if(player->x <= (player->px+(mapcx/6)))
                                player->px--;
                        if(player->px < 0)
                                player->px = 0;
                        break;
                case ACTION_PLAYER_MOVE_RIGHT:
                        player->x++;
                        if(world->cmap[player->y][player->x].type == DNG_WALL)
                                player->x--;
                        if(player->x >= XSIZE-4)
                                player->x = XSIZE-5;
                        if(player->x >= (player->px+(mapcx/6*5)))
                                player->px++;
                        if(player->px >= XSIZE-mapcx)
                                player->px = XSIZE-mapcx-1;
                        break;
                case ACTION_PLAYER_MOVE_NW:
                        if(pt(ply-1,plx-1) != DNG_WALL) {
                                ply--;
                                plx--;
                        }
                        if(ply < 3)
                                ply = 3;
                        if(ply <= (player->py + (mapcy/6)))
                                player->py--;
                        if(player->py < 0)
                                player->py = 0;

                        if(plx < 3)
                                plx = 3;
                        if(plx <= (player->px + (mapcx/6)))
                                player->px--;
                        if(player->px < 0)
                                player->px = 0;
                        break;
                case ACTION_PLAYER_MOVE_NE:
                        if(passable(pt(ply-1,plx+1))) {
                                ply--; plx++;
                        }
                        
                        if(plx >= XSIZE-4)
                                plx = XSIZE-5;
                        if(plx >= (ppx+(mapcx/6*5)))
                                ppx++;
                        if(ppx >= XSIZE-mapcx)
                                ppx = XSIZE-mapcx-1;

                        if(ply < 3)
                                ply = 3;
                        if(ply <= (ppy+(mapcy/6)))
                                ppy--;
                        if(ppy < 0)
                                ppy = 0;
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

        do_action(tmp->action);

        aq->next = tmp->next;
        free(tmp);
}

void do_all_things_in_queue() // needs a better name..
{
        struct actionqueue *tmp;

        tmp = aq->next;

        while(tmp) {
                if(tmp)
                        do_action(tmp->action);
                tmp = tmp->next;
                if(tmp)
                        free(tmp);
        }
}

int main(int argc, char *argv[])
{
        int c;
        int x;
        monster_t *monster;

        if(!setlocale(LC_ALL, ""))
                die("couldn't set locale.");

        messagefile = fopen("messages.txt", "w");
        printf("Gullible's Travails v%s\n", get_version_string());

        init_variables();

        printf("Reading data files...\n");
        if(parse_data_files())
                die("Couldn't parse data files.");

        generate_world();
        world->cmap = world->out;

        init_display();
        init_player();
        draw_world();

        gtprintf("player x,y = %d, %d - px,py = %d, %d", player->x, player->y, player->px, player->py);
        monster = get_monsterdef(ri(1, game->monsterdefs));
        gtprintf("monster: %s", monster->name);
        monster->x = player->x + ri(5,15);
        monster->y = player->y + ri(5,15);
        world->cmap[monster->y][monster->x].monster = monster;
        game->context = CONTEXT_OUTSIDE;

        initial_update_screen();

        do {
                c = gtgetch();
                switch(c) {
                        case 'q':
                                queue(ACTION_NOTHING);
                                game->dead = 1;
                                break;
                        case 'd':
                                queue(ACTION_NOTHING);
                                if(world->cmap == world->out) {
                                        world->cmap = world->dng;
                                        game->context = CONTEXT_DUNGEON;
                                        while(world->cmap[player->y][player->x].type != DNG_FLOOR) {
                                                player->y = ri(15, DUNGEON_SIZE);
                                                player->x = ri(15, DUNGEON_SIZE);
                                        }
                                        player->py = player->y - (game->maph / 2);
                                        player->px = player->x - (game->mapw / 2);
                                        if(player->py <= 0)
                                                player->py = 0;
                                        if(player->px <= 0)
                                                player->px = 0;


                                        player->viewradius = 5;
                                } else {
                                        world->cmap = world->out;
                                        game->context = CONTEXT_OUTSIDE;
                                        player->viewradius = 10;
                                }
                                break;
                        case 'f':
                                // this should ensure floodfill working every time!
                                x = ri(11,111);
                                while(world->dng[x][x].type != DNG_FLOOR) {
                                        x = ri(11,111);
                                }
                                gtprintf("floodfilling from %d, %d\n", x, x);
                                floodfill(x, x);
                                queue(ACTION_NOTHING);
                                break;
                        case 'j':
                                queue(ACTION_PLAYER_MOVE_DOWN);
                                break;
                        case 'k':
                                queue(ACTION_PLAYER_MOVE_UP);
                                break;
                        case 'h':
                                queue(ACTION_PLAYER_MOVE_LEFT);
                                break;
                        case 'l':
                                queue(ACTION_PLAYER_MOVE_RIGHT);
                                break;
                        case 'y':
                                queue(ACTION_PLAYER_MOVE_NW);
                                break;
                        case 'u':
                                queue(ACTION_PLAYER_MOVE_NE);
                                break;
                        case 'b':
                                queue(ACTION_PLAYER_MOVE_SW);
                                break;
                        case 'n':
                                queue(ACTION_PLAYER_MOVE_SE);
                                break;
                        default:
                                queue(ACTION_NOTHING);
                                break;
                }

                do_one_thing_in_queue();
                do_all_things_in_queue();
                if(monster->ai)
                        monster->ai(monster);

                draw_world();
                gtprintf("player x,y = %d, %d - px,py = %d, %d", player->x, player->y, player->px, player->py);
                update_screen();
        } while(!game->dead);

        shutdown_display();
        shutdown_gt();

        return 0;
}
