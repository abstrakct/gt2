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
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "display.h"
#include "gt.h"

#ifdef GT_USE_NCURSES
#include <curses.h>

extern WINDOW *wall;
extern WINDOW *wstat;
extern WINDOW *winfo;
extern WINDOW *wmap;
extern WINDOW *wleft;

extern int maxmess;

// Stolen from DCSS!
void setup_color_pairs()
{
        short i, j;

        
        for (i = 0; i < 8; i++)
                for (j = 0; j < 8; j++) {
                        if ((i > 0) || (j > 0))
                                init_pair(i * 8 + j, j, i);
                }

        init_pair(63, COLOR_BLACK, COLOR_BLACK);
}

void init_display()
{
        initscr();
        if(!has_colors()) {
                endwin();
                die("Your terminal has no colors!");
        }
        start_color();

	/*init_pair(COLOR_PLAIN,   COLOR_WHITE,  COLOR_BLACK);
	init_pair(COLOR_FOREST,  COLOR_GREEN,  COLOR_BLACK);
	init_pair(COLOR_CITY,    COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_WARNING, COLOR_RED,    COLOR_BLACK);
	init_pair(COLOR_PLAYER,  COLOR_BLUE,   COLOR_BLACK);
	init_pair(COLOR_LIGHT,   COLOR_YELLOW, COLOR_BLACK);
	init_pair(COLOR_SHADE,   COLOR_WHITE, COLOR_BLACK);

        init_pair(COLOR_INVISIBLE, COLOR_BLACK, COLOR_BLACK);*/

        setup_color_pairs();

        game->width = COLS;
        game->height = LINES;
        game->mapw = ((COLS/4)*3) - (COLS/4);
        game->maph = ((LINES/3)*2)-1;

        wall  = newwin(0, 0, 0, 0);
        wleft = subwin(wall, game->maph, (COLS/4), 0, 0);
        wmap  = subwin(wall, game->maph, game->mapw, 0, (COLS/4));               //øverst venstre                                                                                                                                                                                                                                          
        wstat = subwin(wall, (LINES/3)*2-1, (COLS/4), 0, COLS-((COLS/4)));  //øverst høyre                                                                                                                                                                                                                                    
        winfo = subwin(wall, LINES/3, COLS, LINES-(LINES/3)-1, 0);          //nederst                                                                                                                                                                                                                                                 
        maxmess = (LINES/3)-2;

        box(wmap,  ACS_VLINE, ACS_HLINE);
        box(wstat, ACS_VLINE, ACS_HLINE);                                                                                                                                                                                                                                                                         
        box(winfo, ACS_VLINE, ACS_HLINE);
        box(wleft, ACS_VLINE, ACS_HLINE);


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

        mess("*** Welcome to Gullible's Travails ***");
        mess("Press q to exit.");

        touchwin(wmap);
        touchwin(wstat);
        touchwin(winfo);
        touchwin(wleft);
}

void shutdown_display()
{
        endwin();
}

bool blocks_light(int y, int x)
{
        level_t *l = world->curlevel;

        if(l->c[y][x].flags & CF_HAS_DOOR_CLOSED)
                return true;

        switch(l->c[y][x].type) {
                case AREA_NOTHING:
                case AREA_MOUNTAIN:
                case AREA_CITY:
                case AREA_FOREST:
                case AREA_VILLAGE:
                case AREA_WALL:
                case DNG_WALL:
                       return true;
                default:       
                       return false;
        }

        // shouldn't be reached...
        return false;
}

void clear_map_to_invisible(level_t *l)
{
        int x, y;

        for(y = ppy; y < (ppy+game->maph); y++) {
                for(x = ppx; x < (ppx+game->mapw); x++) {
                        if(x >= 0 && y >= 0 && x < l->xsize && y < l->ysize)
                                l->c[y][x].visible = 0;
                }
        }
}

void clear_map_to_unlit(level_t *l)
{
        int x, y;

        for(y = 0; y < l->ysize; y++) {
                for(x = 0; x < l->xsize; x++) {
                        l->c[y][x].flags &= ~(CF_LIT);   // TODO: make setbit/clearbit macros!
                }
        }
}

/*
 * The next two functions are about FOV.
 * Stolen/adapted from http://roguebasin.roguelikedevelopment.org/index.php/Eligloscode
 */

void dofov(actor_t *actor, level_t *l, float x, float y)
{
        int i;
        float ox, oy;

        ox = (float) actor->x + 0.5f;
        oy = (float) actor->y + 0.5f;

        for(i = 0; i < actor->viewradius; i++) {
                if((int)oy >= 0 && (int)ox >= 0 && (int)oy < l->ysize && (int)ox < l->xsize) {
                        l->c[(int)oy][(int)ox].visible = 1;
                        l->c[(int)oy][(int)ox].flags  |= CF_VISITED;
                        if(blocks_light((int) oy, (int) ox)) {
                                return;
                        }/* else {  //SCARY MODE! 
                                if(perc((100-actor->viewradius)/3))
                                        return;
                        }*/


                        ox += x;
                        oy += y;
                }
        }
}

void FOV(actor_t *a, level_t *l)
{
        float x, y;
        int i;
        //signed int tmpx,tmpy;

        // if dark dungeon
        clear_map_to_invisible(l);

        for(i = 0; i < 360; i++) {
                x = cos((float) i * 0.01745f);
                y = sin((float) i * 0.01745f);
                dofov(a, l, x, y);
        }
}

void dofovlight(actor_t *actor, level_t *l, float x, float y)
{
        int i;
        float ox, oy;

        ox = (float) actor->x + 0.5f;
        oy = (float) actor->y + 0.5f;


        for(i = 0; i < (actor->viewradius/2); i++) {       // TODO: add a lightradius in actor_t, calculate it based on stuff
                if((int)oy >= 0 && (int)ox >= 0 && (int)oy < l->ysize && (int)ox < l->xsize) {
                        if(l->c[(int)oy][(int)ox].type == DNG_WALL)
                                l->c[(int)oy][(int)ox].flags |= CF_LIT;
                        if(blocks_light((int) oy, (int) ox))
                                return;

                        ox += x;
                        oy += y;
                }
        }
}

void FOVlight(actor_t *a, level_t *l)
{
        float x, y;
        int i;

        clear_map_to_unlit(l);
        for(i = 0; i < 360; i++) {
                x = cos((float) i * 0.01745f);
                y = sin((float) i * 0.01745f);
                dofovlight(a, l, x, y);
        }
}

// The actual drawing on screen

void draw_world(level_t *level)
{
        int i,j;
        int dx, dy;  // coordinates on screen!
        int color;

        werase(wmap);
        FOV(player, level);
        if(game->context == CONTEXT_DUNGEON)
                FOVlight(player, level);     // only necessary in dungeon

        /*
         * in this function, (j,i) are the coordinates on the map,
         * dx,dy = coordinates on screen.
         * so, player->py/px describes the upper left corner of the map
         */
        for(i = ppx, dx = 0; i <= (ppx + game->mapw); i++, dx++) {
                for(j = ppy, dy = 0; j <= (ppy + game->maph); j++, dy++) {
                        if(j < level->ysize && i < level->xsize) {
                                if(level->c[j][i].flags & CF_VISITED) {
                                        color = cc(j,i);
                                        if(game->context == CONTEXT_DUNGEON)
                                                wattron(wmap, A_BOLD);

                                        if(level->c[j][i].flags & CF_LIT) {
                                                wattroff(wmap, A_BOLD);
                                                color = COLOR_CITY;
                                        }

                                        if(level->c[j][i].flags & CF_HAS_DOOR_CLOSED)
                                                gtmapaddch(dy, dx, color, '+');
                                        else if(level->c[j][i].flags & CF_HAS_DOOR_OPEN)
                                                gtmapaddch(dy, dx, color, '\'');
                                        else
                                                gtmapaddch(dy, dx, color, mapchars[(int) level->c[j][i].type]);

                                        if(level->c[j][i].visible && level->c[j][i].inventory) {
                                                if(level->c[j][i].inventory->quantity > 0) {
                                                        wattron(wmap, A_BOLD);
                                                        gtmapaddch(dy, dx, COLOR_YELLOW, objchars[OT_GOLD]);
                                                        wattroff(wmap, A_BOLD);
                                                } else {                                                         // TODO ADD OBJECT COLORS!!!
                                                        if(level->c[j][i].inventory->next) {
                                                                gtmapaddch(dy, dx, COLOR_BLUE, objchars[level->c[j][i].inventory->next->type]);
                                                        }
                                                }
                                        }
                                }


                                if(level->c[j][i].visible && level->c[j][i].monster /*&& actor_in_lineofsight(player, level->c[j][i].monster)*/)
                                        gtmapaddch(dy, dx, COLOR_RED, (char) level->c[j][i].monster->c);

                                if(level->c[j][i].type == AREA_WALL) {
                                        gtmapaddch(dy, dx, COLOR_PLAIN, mapchars[DNG_WALL]);
                                }
                        if(j == ply && i == plx)
                                gtmapaddch(dy, dx, COLOR_PLAYER, '@');
                        }
                }
        }

        wattron(wmap, COLOR_PAIR(COLOR_NORMAL));
        box(wmap, ACS_VLINE, ACS_HLINE);
        //wcolor_set(wmap, 0, 0);
}

void draw_wstat()
{
        obj_t *o;
        int i;
        int color;

        mvwprintw(wleft, 1, 1, "Name:");
        mvwprintw(wleft, 2, 1, "Turn:   %d", game->turn);
        mvwprintw(wleft, 3, 1, "(y,x)   (%d,%d)     ", ply, plx);
        mvwprintw(wleft, 4, 1, "(py,px) (%d,%d)     ", ppy, ppx);
        mvwprintw(wleft, 5, 1, "viewradius: %d      ", player->viewradius);
        if(player->hp >= (player->maxhp/4*3))
                color = COLOR_PAIR(COLOR_GREEN);
        else if(player->hp >= (player->maxhp/4) && player->hp < (player->maxhp/4*3))
                color = COLOR_PAIR(COLOR_YELLOW);
        else if(player->hp < (player->maxhp/4))
                color = COLOR_PAIR(COLOR_RED);
        mvwprintw(wleft, 6, 1, "HP:");
        wattron(wleft, color);
        mvwprintw(wleft, 6, 5, "%d   ", player->hp);
        wattroff(wleft, color);

        mvwprintw(wstat, 1, 1, "Inventory:");
        mvwprintw(wstat, 2, 1, "Gold: %d            ", player->inventory->quantity);
        o = player->inventory->next;
        i = 3;
        while(o) {
                mvwprintw(wstat, i, 1, "  * %s             ", o->basename);
                i++;
                o = o->next;
        }
}

void update_player()
{
        gtmapaddch(player->oldy, player->oldx, cc(player->oldy, player->oldx), mapchars[(int) ct(player->oldy, player->oldx)]);
        gtmapaddch(ply, plx, COLOR_PLAYER, '@');
}

void gtmapaddch(int y, int x, int color, char c)
{
        wattron(wmap, COLOR_PAIR(color));
        mvwaddch(wmap, y, x, c);
        wattroff(wmap, COLOR_PAIR(color));
}

void update_screen()
{
        wnoutrefresh(wstat);
        wnoutrefresh(wleft);
        doupdate();
}

void initial_update_screen()
{
        wnoutrefresh(wmap);
        wnoutrefresh(winfo);
        wnoutrefresh(wstat);
        wnoutrefresh(wleft);
        doupdate();
}

// Input and messages

int gtgetch()
{
        int c;
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
                wattron(winfo, COLOR_PAIR(messages[i].color));
                mvwprintw(winfo, i+1, 1, messages[i].text);
                wattroff(winfo, COLOR_PAIR(messages[i].color));
        }

        fprintf(messagefile, "%d\t%s\n", messages[currmess-1].color, messages[currmess-1].text);
        wnoutrefresh(winfo);
}

void scrollmessages()
{
        int i;

        if (currmess >= maxmess) {
                currmess = maxmess - 1;
                for(i = 0; i <= currmess; i++) {
                        messages[i].color = messages[i+1].color;
                        strcpy(messages[i].text, messages[i+1].text);
                }
        }
}

void mess(char *message)
{
        /* optionally insert check for duplicate messages here! */

        scrollmessages();
        messages[currmess].color = COLOR_NORMAL;
        strcpy(messages[currmess].text, message);
        domess();
}

void messc(int color, char *message)
{
        /* optionally insert check for duplicate messages here! */

        scrollmessages();
        messages[currmess].color = color;
        strcpy(messages[currmess].text, message);
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

void gtmapaddch(int y, int x, int color, char c)
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
