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
#include <curses.h>

#include <libconfig.h>

#include "utils.h"
#include "monsters.h"
#include "datafiles.h"
#include "gt.h"

monster_t *monsterdefs;


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

void clean_up_the_mess()
{
        monster_t *n, *m;

        n = monsterdefs->head->next;
        while(n) {
                m = n->next;
                if(n)
                        free(n);
                n = m;
        }
        free(monsterdefs->head);
}

void init_display()
{
        initscr();
        start_color();
        cbreak();
        // raw(); bruk denne istedet hvis vi skal takle interrupt handling.
        noecho();
        keypad(stdscr, TRUE);
        scrollok(stdscr, FALSE);
        nonl();
        curs_set(0);
        meta(stdscr, TRUE);
}

void shutdown_display()
{
        endwin();
}

int main(int argc, char *argv[])
{
        int x,y;

        if(!setlocale(LC_ALL, ""))
                die("couldn't set locale.");

        printf("Gullible's Travails v%s\n", get_version_string());
        printf("Reading data files...\n");

        monsterdefs = (monster_t *) gtmalloc(sizeof(monster_t));
        memset(monsterdefs, 0, sizeof(monster_t));
        monsterdefs->head = monsterdefs;

        if(parse_data_files())
                die("Couldn't parse data files.");

        dump_monsters();
        sleep(1);
        
        init_display();
        getmaxyx(stdscr, y, x);
        mvprintw(y/2, x/2, "ROMPE!");
        refresh();
        sleep(1);

        shutdown_display();
        clean_up_the_mess();

        return 0;
}
