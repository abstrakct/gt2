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
#include <math.h>
#include <stdbool.h>

#include "objects.h"
#include "monsters.h"
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "you.h"
#include "display.h"
#include "actor.h"
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
        init_pair(COLOR_INVISIBLE, COLOR_BLACK, COLOR_BLACK);

        game->width = COLS;
        game->height = LINES;
        game->mapw = ((COLS/4)*3);
        game->maph = ((LINES/3)*2)-1;

        wall  = newwin(0, 0, 0, 0);                                                                                                                                                                                                                                                                                         
        wmap  = subwin(wall, game->maph, game->mapw, 0, 0);               //øverst venstre                                                                                                                                                                                                                                          
        wstat = subwin(wall, (LINES/3)*2-1, (COLS/4), 0, COLS-((COLS/4)));  //øverst høyre                                                                                                                                                                                                                                    
        winfo = subwin(wall, LINES/3, COLS, LINES-(LINES/3)-1, 0);          //nederst                                                                                                                                                                                                                                                 
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

bool blocks_light(int type)
{
        if(type != AREA_PLAIN && type != DNG_WALL && type != AREA_CITY_NOHOUSE && type != AREA_FOREST_NOTREE && type != AREA_CITY && type != AREA_FOREST && type != AREA_VILLAGE && type != AREA_WALL)
                return true;
        
        return false;
}

void clear_map_to_invisible()
{
        int x, y;

        for(y = 0; y < game->height; y++) {
                for(x = 0; x < game->width; x++) {
                        world->cmap[y][x].visible = 0;
                }
        }
}

/*
 * The next two functions are about FOV.
 * Stolen/adapted from http://roguebasin.roguelikedevelopment.org/index.php/Eligloscode
 */

void dofov(float x, float y)
{
        int i;
        float ox, oy;

        ox = (float) player->x + 0.5f;
        oy = (float) player->y + 0.5f;

        for(i = 0; i < player->viewradius; i++) {
                if((int)oy >= 0 && (int)ox >= 0 && (int)oy < YSIZE && (int)ox < XSIZE) {
                        world->cmap[(int)oy][(int)ox].visible = 1;
                        if(game->context == CONTEXT_DUNGEON)
                                if(!blocks_light(world->cmap[(int)oy][(int)ox].type))
                                        return;

                        if(game->context == CONTEXT_OUTSIDE) {
                                if(blocks_light(world->cmap[(int)oy][(int)ox].type))
                                        return;
                        }

                        ox += x;
                        oy += y;
                }
        }
}

void FOV()
{
        float x, y;
        int i;

        //clear_map_to_invisible();
        for(i = 0; i < 360; i++) {
                x = cos((float) i * 0.01745f);
                y = sin((float) i * 0.01745f);
                dofov(x, y);
        }
}

void draw_world()
{
        int i,j;
        int dx, dy;  // coordinates on screen!

        werase(wmap);
        FOV();
        for(j = player->py, dy = 0; j < (player->py + game->maph); j++, dy++) {
                for(i = player->px, dx = 0; i < (player->px + game->mapw); i++, dx++) {
                        /*
                         * in this function, (j,i) are the coordinates on the map,
                         * dx,dy = coordinates on screen.
                         * so, player->py/px describes the upper left corner of the map
                         */

                        if(world->cmap[j][i].visible) {
                                gtmapaddch(dx, dy, world->cmap[j][i].color, mapchars[(int)world->cmap[j][i].type]);
                                if(world->cmap[j][i].monster)
                                        gtmapaddch(dx, dy, COLOR_RED, (char) world->cmap[j][i].monster->c);
                        }

                        if(world->cmap[j][i].type == AREA_WALL) {
                                gtmapaddch(dx, dy, COLOR_PLAIN, mapchars[DNG_WALL]);
                        }
                        
                        if(j == player->y && i == player->x)
                                gtmapaddch(dx, dy, COLOR_PLAYER, '@');
                }
        }

        wattron(wmap, COLOR_PAIR(COLOR_NORMAL));
        box(wmap, ACS_VLINE, ACS_HLINE);
        //wcolor_set(wmap, 0, 0);
}

void gtmapaddch(int x, int y, int color, char c)
{
        wattron(wmap, COLOR_PAIR(color));
        mvwaddch(wmap, y, x, c);
        wattroff(wmap, COLOR_PAIR(color));
}

void update_screen()
{
        doupdate();
}

void initial_update_screen()
{
        wnoutrefresh(wmap);
        wnoutrefresh(winfo);
        wnoutrefresh(wstat);
        doupdate();
}

char gtgetch()
{
        char c;
        c = wgetch(wmap);
        return c;
}

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

        fprintf(messagefile, "%d %s\n", m[currmess-1].color, m[currmess-1].text);
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

void gtmapaddch(int x, int y, int color, char c)
{
        printf("%c", c);
}

void initial_update_screen()
{
        printf("%s:%d - initial_update_screen\n", __FILE__, __LINE__);
}

void update_screen()
{
        printf("%s:%d - update_screen\n", __FILE__, __LINE__);
}

char gtgetch()
{
        return (char) ri(97,122);
}

void domess()
{
        printf("%s:%d - domess\n", __FILE__, __LINE__);
}

void scrollmessages()
{
        printf("%s:%d - scrollmessages\n", __FILE__, __LINE__);
}

void mess(char *message)
{
        printf("%s:%d - %s\n", __FILE__, __LINE__, message);
}

void messc(int color, char *message)
{
        printf("%s:%d - %s\n", __FILE__, __LINE__, message);
}

#endif
