/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * This file deals with diplaying output!
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "objects.h"
#include "monsters.h"
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "you.h"
#include "display.h"
#include "gt.h"

#ifdef GT_USE_NCURSES
#include <curses.h>

extern WINDOW *wall;
extern WINDOW *wstat;
extern WINDOW *winfo;
extern WINDOW *wmap;
extern int maxmess;

void init_display()
{
        initscr();
        if(!has_colors()) {
                endwin();
                die("Your terminal has no colors!");
        }
        start_color();

	init_pair(COLOR_FOREST, COLOR_GREEN, COLOR_BLACK);
	// TODO: ncurses w/ 256 colors!
	//init_color(50, 200, 200, 200); // a grey color
	init_pair(COLOR_PLAIN,   COLOR_WHITE, COLOR_BLACK);
	init_pair(COLOR_FOREST,  COLOR_GREEN, COLOR_BLACK);
	init_pair(COLOR_CITY,    COLOR_YELLOW, COLOR_BLACK);
	init_pair(COLOR_PLAYER,  COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_INFO,    COLOR_BLUE, COLOR_BLACK);
        init_pair(COLOR_WARNING, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_LAKE,    COLOR_BLUE, COLOR_BLACK);

        game->width = COLS;
        game->height = LINES;
        game->mapw = ((COLS/4)*3);
        game->maph = ((LINES/3)*2);

        wall  = newwin(0, 0, 0, 0);                                                                                                                                                                                                                                                                                         
        wmap  = subwin(wall, game->maph, game->mapw, 0, 0);               //øverst venstre                                                                                                                                                                                                                                          
        wstat = subwin(wall, (LINES/3)*2, (COLS/4), 0, COLS-((COLS/4)));  //øverst høyre                                                                                                                                                                                                                                    
        winfo = subwin(wall, LINES/3, COLS, LINES-(LINES/3), 0);          //nederst                                                                                                                                                                                                                                                 
        maxmess = (LINES/3)-2;

        box(wmap, ACS_VLINE, ACS_HLINE);                                                                                                                                                                                                                                                                                    
        box(wstat, ACS_VLINE, ACS_HLINE);                                                                                                                                                                                                                                                                                   
        box(winfo, ACS_VLINE, ACS_HLINE);          

        cbreak();
        // raw(); bruk denne istedet hvis vi skal takle interrupt handling.
        noecho();
        keypad(wmap, TRUE);
        scrollok(wall, FALSE);
        nonl();
        nodelay(wall, TRUE);
        curs_set(0);
        meta(wall, TRUE);
        intrflush(wmap, FALSE);

        //mvwprintw(winfo, 1, 1, "*** Welcome to Gullible's Travails v.%s ***", get_version_string());
        //mvwprintw(winfo, 2, 1, "Press q to exit.");
        mess("*** Welcome to Gullible's Travails ***");
        mess("Press q to exit.");

        touchwin(wmap);
        touchwin(wstat);
        touchwin(winfo);
}

void shutdown_display()
{
        endwin();
}

void draw_world()
{
        int i,j;

        werase(wmap);
        for(i=1;i<game->mapw;i++) {
                for(j=1;j<game->maph;j++) {
                        wattron(wmap, COLOR_PAIR(world->out[j][i].color));
                        mvwaddch(wmap, j, i, mapchars[(int)world->out[j][i].type]);
                        wattroff(wmap, COLOR_PAIR(world->out[j][i].color));
                }
        }

        wattron(wmap, COLOR_PAIR(COLOR_NORMAL));
        box(wmap, ACS_VLINE, ACS_HLINE);
        //wcolor_set(wmap, 0, 0);
}

#else

void init_display()
{
        printf("Dummy display driver reporting for duty!\n");
}

void shutdown_display()
{
        printf("Shutting down dummy display driver!\n");
}

void draw_world()
{
        printf("Imagine a beautiful landscape... Trees, mountains, birds...\n");
}
#endif
