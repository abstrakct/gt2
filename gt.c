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
#include <curses.h>
#include <signal.h>
#include <libconfig.h>

#include "objects.h"
#include "monsters.h"
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "you.h"
#include "display.h"
#include "actor.h"
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
FILE *messagefile;
int mapcx, mapcy;

// Messages
message_t m[500];
int currmess, maxmess;

// Ncurses stuff
WINDOW *wall;
WINDOW *wstat;
WINDOW *winfo;
WINDOW *wmap;

void dump_monsters()
{
        monster_t *m, *n;
        int i;

        n = monsterdefs->head;
        for(i=0;i<monsterdefs->head->x;i++) {
                m = n->next;
                printf("%s\t%c\nstr\t%d\tphys\t%d\tintl\t%d\tknow\t%d\tdex\t%d\tcha\t%d\n", m->name, m->c, m->attr.str, m->attr.phys, m->attr.intl, m->attr.know, m->attr.dex, m->attr.cha);
                printf("hp\t%d\tthac0\t%d\tlevel\t%d\tspeed\t%.1f\n", m->hp, m->thac0, m->level, m->speed);
                printf("Can use weapon: %s\tCan use armor: %s\tCan have gold: %s\n", m->flags & MF_CANUSEWEAPON ? "Yes" : "No", m->flags & MF_CANUSEARMOR ? "Yes" : "No", m->flags & MF_CANHAVEGOLD ? "Yes" : "No");
                printf("\n");
                n = m;
        }
}

void dump_objects()
{
        obj_t *o, *n;
        int i;

        n = objdefs->head;
        for(i=0; i<objdefs->head->ddice; i++) {
                o = n->next;
                printf("Basename: %s\n", o->basename);
                printf("Type:     %s\n", otypestrings[o->type]);
                if(isarmor(o))
                        printf("AC:       %d\n", o->ac);
                printf("\n");
                n = o;
        }
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

void init_variables()
{
        monsterdefs = (monster_t *) gtmalloc(sizeof(monster_t));
        memset(monsterdefs, 0, sizeof(monster_t));
        monsterdefs->head = monsterdefs;

        objdefs = (obj_t *) gtmalloc(sizeof(obj_t));
        memset(objdefs, 0, sizeof(obj_t));
        objdefs->head = objdefs;

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
        player->x = XSIZE/2;
        player->y = YSIZE/2;
        player->px = player->x - game->mapw / 2;
        player->py = player->y - game->maph / 2;
        mapcx = game->mapw + 2;
        mapcy = game->maph + 2;
}

int main(int argc, char *argv[])
{
        int c;

        if(!setlocale(LC_ALL, ""))
                die("couldn't set locale.");

        messagefile = fopen("messages.txt", "w");

        printf("Gullible's Travails v%s\n", get_version_string());

        init_variables();

        printf("Reading data files...\n");
        if(parse_data_files())
                die("Couldn't parse data files.");

        generate_world();

        init_display();
        init_player();
        draw_world();

        initial_update_screen();

        do {
                c = gtgetch();
                switch(c) {
                        case 'q':
                                game->dead = 1;
                                break;
                        case 'j':
                                player->y++;
                                if(player->y >= YSIZE-6)
                                        player->y = YSIZE-7;
                                if(player->y >= (player->py + (mapcy/6*5)))
                                        player->py++;
                                if(player->py >= YSIZE-mapcy-2)
                                        player->py = YSIZE - mapcy - 3;
                                break;
                        case 'k':
                                player->y--;
                                if(player->y < 3)
                                        player->y = 3;
                                if(player->y <= (player->py + (mapcy/6)))
                                        player->py--;
                                if(player->py < 2)
                                        player->py = 2;
                                break;
                        case 'h':
                                player->x--;
                                if(player->x < 3)
                                        player->x = 3;
                                if(player->x <= (player->px+(mapcx/6)))
                                        player->px--;
                                if(player->px < 2)
                                        player->px = 2;
                                break;
                        case 'l':
                                player->x++;
                                if(player->x >= XSIZE-4)
                                        player->x = XSIZE-5;
                                if(player->x >= (player->px+(mapcx/6*5)))
                                        player->px++;
                                if(player->px >= XSIZE-mapcx)
                                        player->px = XSIZE-mapcx-1;
                                break;
                        default:
                                break;
                }

                // TODO: make all display drawing functions library/output independent
                draw_world();
                update_screen();
        } while(!game->dead);

        shutdown_display();
        shutdown_gt();

        return 0;
}

        /*  dump/debug stuff
         *  dump_monsters();
         *  dump_objects();
         * sleep(1);
         */
        
