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
FILE *messagefile;

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

/* 
 * Message window handling!
 */

void domess()
{
        int i;

        // There might be a better way to clean the window, but this works.
        werase(winfo);
        box(winfo, ACS_VLINE, ACS_HLINE);          

        currmess++;
        for(i = maxmess-1; i >= 0; i--) {
                wattron(winfo, COLOR_PAIR(m[i].color));
                mvwprintw(winfo, i+1, 1, m[i].text);
                wattroff(winfo, COLOR_PAIR(m[i].color));
        }

        fprintf(messagefile, "%s\n", m[currmess-1].text);
        wnoutrefresh(winfo);
//        doupdate();
}

void scrollmessages()
{
        int i;

        if (currmess >= maxmess) {
                currmess = maxmess - 1;
                for(i = 0; i <= currmess; i++) {
                        m[i].color = m[i+1].color;
                        strcpy(m[i].text, m[i+1].text);
                }
        }
}

void mess(char *message)
{
        /* optionally insert check for duplicate messages here! */

        scrollmessages();
        m[currmess].color = COLOR_NORMAL;
        strcpy(m[currmess].text, message);
        domess();
}

void messc(int color, char *message)
{
        /* optionally insert check for duplicate messages here! */

        scrollmessages();
        m[currmess].color = color;
        strcpy(m[currmess].text, message);
        domess();
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
}

int main(int argc, char *argv[])
{
        int c;
        int i = 0;
        char s[50];

        if(!setlocale(LC_ALL, ""))
                die("couldn't set locale.");

        messagefile = fopen("messages.txt", "w");

        game = (game_t *) gtmalloc(sizeof(game_t));
        game->dead = 0;
        game->seed = time(0);
        srand(game->seed);
fprintf(stderr, "DEBUG: %s:%d - Random seed is %d\n", __FILE__, __LINE__, game->seed);

        printf("Gullible's Travails v%s\n", get_version_string());
        printf("Reading data files...\n");

        init_variables();

        if(parse_data_files())
                die("Couldn't parse data files.");

        /*  dump/debug stuff
         *  dump_monsters();
         *  dump_objects();
         * sleep(1);
         */
        
        generate_world();

        init_display();
        draw_world();

        wnoutrefresh(wmap);
        wnoutrefresh(winfo);
        wnoutrefresh(wstat);
        doupdate();

        do {
                c = wgetch(wmap);
                if(c == 'q')
                        game->dead = 1;
                i++;
                sprintf(s, "Keypress number %d", i);
                messc(COLOR_GOOD, s);

                draw_world();
                // TODO: make all display drawing functions library/output independent
                doupdate();
        } while(!game->dead);

        shutdown_display();
        shutdown_gt();

        return 0;
}
