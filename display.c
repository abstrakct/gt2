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

        gtprintf("*** Welcome to Gullible's Travails v%s ***", game->version);
        gtprintf("Press q to exit.");

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

        if(hasbit(l->c[y][x].flags, CF_HAS_DOOR_CLOSED))
                return true;

        if(l->c[y][x].type == AREA_FOREST || l->c[y][x].type == AREA_CITY || l->c[y][x].type == AREA_VILLAGE) {    // trees and houses can be "see through" (e.g. if they are small)
                if(perc(20))
                        return false;
                else
                        return true;
        }

        switch(l->c[y][x].type) {
                case AREA_NOTHING:
                case AREA_MOUNTAIN:
                //case AREA_CITY:
                //case AREA_VILLAGE:
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
                        clearbit(l->c[y][x].flags, CF_LIT);
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
                        setbit(l->c[(int)oy][(int)ox].flags, CF_VISITED);
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


        //gtprintf("\tentering dofovlight");
        for(i = 0; i < (actor->viewradius/2); i++) {       // TODO: add a lightradius in actor_t, calculate it based on stuff
                if((int)oy >= 0 && (int)ox >= 0 && (int)oy < l->ysize && (int)ox < l->xsize) {
                        //gtprintf("\t\tchecking cell %d,%d", (int)oy, (int)ox);
                        if(hasbit(l->c[(int)oy][(int)ox].flags, CF_LIT))
                                return;

                        if(l->c[(int)oy][(int)ox].type == DNG_WALL) {
                                setbit(l->c[(int)oy][(int)ox].flags, CF_LIT);
                        }

                        if(blocks_light((int) oy, (int) ox)) {
                                //gtprintf("cell %d,%d blocks light", (int)oy, (int)ox);
                                return;
                        }

                        ox += x;
                        oy += y;
                }
        }
}

void FOVlight(actor_t *a, level_t *l)
{
        float x, y;
        int i;

        //gtprintf("entering FOVlight..");
        clear_map_to_unlit(l);
        for(i = 0; i < 360; i++) {
                x = cos((float) i * 0.01745f);
                y = sin((float) i * 0.01745f);
                //fprintf(stderr, "DEBUG: %s:%d - now going to dofovlight i = %d y = %.4f x = %.4f\n", __FILE__, __LINE__, i, y, x);
                dofovlight(a, l, x, y);
        }
}

// The actual drawing on screen

void draw_world(level_t *level)
{
        int i,j, slot;
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
                                if(hasbit(level->c[j][i].flags, CF_VISITED)) {
                                        color = cc(j,i);
                                        if(game->context == CONTEXT_DUNGEON)
                                                wattron(wmap, A_BOLD);

                                        if(hasbit(level->c[j][i].flags, CF_LIT)) {
                                                wattroff(wmap, A_BOLD);
                                                color = COLOR_CITY;
                                        }

                                        gtmapaddch(dy, dx, color, mapchars[(int) level->c[j][i].type]);

                                        if(level->c[j][i].inventory) {
                                                wattroff(wmap, A_BOLD);
                                                if(level->c[j][i].inventory->gold > 0) {
                                                        wattron(wmap, A_BOLD);
                                                        gtmapaddch(dy, dx, COLOR_YELLOW, objchars[OT_GOLD]);
                                                        wattroff(wmap, A_BOLD);
                                                } else {                                                         // TODO ADD OBJECT COLORS!!!
                                                        slot = get_first_used_slot(level->c[j][i].inventory);
                                                        if(level->c[j][i].inventory->num_used > 0 && slot >= 0 && level->c[j][i].inventory->object[slot]) {
                                                                gtmapaddch(dy, dx, COLOR_BLUE, objchars[level->c[j][i].inventory->object[slot]->type]);
                                                        }
                                                }
                                        }

                                        if(hasbit(level->c[j][i].flags, CF_HAS_DOOR_CLOSED))
                                                gtmapaddch(dy, dx, color, '+');
                                        else if(hasbit(level->c[j][i].flags, CF_HAS_DOOR_OPEN))
                                                gtmapaddch(dy, dx, color, '\'');
                                        else if(hasbit(level->c[j][i].flags, CF_HAS_STAIRS_DOWN))
                                                gtmapaddch(dy, dx, color, '>');
                                        else if(hasbit(level->c[j][i].flags, CF_HAS_STAIRS_UP))
                                                gtmapaddch(dy, dx, color, '<');
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
        int i, j;
        int color;

        werase(wleft);
        werase(wstat);
        box(wleft, ACS_VLINE, ACS_HLINE);                                                                                                                                                                                                                                                                         
        box(wstat, ACS_VLINE, ACS_HLINE);                                                                                                                                                                                                                                                                         

        mvwprintw(wleft, 1, 1, "Name:");
        mvwprintw(wleft, 2, 1, "Turn:   %d", game->turn);
        mvwprintw(wleft, 3, 1, "Weapon: %s", player->weapon ? player->weapon->fullname : "bare hands");
        //mvwprintw(wleft, 3, 1, "y,x     %d,%d", ply, plx);
        //mvwprintw(wleft, 4, 1, "(py,px) (%d,%d)", ppy, ppx);
        mvwprintw(wleft, 5, 1, "viewradius: %d", player->viewradius);
        if(player->hp >= (player->maxhp/4*3))
                color = COLOR_PAIR(COLOR_GREEN);
        else if(player->hp >= (player->maxhp/4) && player->hp < (player->maxhp/4*3))
                color = COLOR_PAIR(COLOR_YELLOW);
        else if(player->hp < (player->maxhp/4))
                color = COLOR_PAIR(COLOR_RED);
        mvwprintw(wleft, 6, 1, "HP:");
        wattron(wleft, color);
        mvwprintw(wleft, 6, 5, "%d (%.1f%%)", player->hp, ((float)(100/(float)player->maxhp) * (float)player->hp));
        wattroff(wleft, color);
        mvwprintw(wleft, 7, 1, "Player level: %d", player->level);
        mvwprintw(wleft, 8, 1, "AC: %d", player->ac);
        mvwprintw(wleft, 9, 1, "Dungeon level: %d (out of %d)", game->currentlevel, game->createddungeons);
        mvwprintw(wleft, 10, 1, "STR:   %d", player->attr.str);
        mvwprintw(wleft, 11, 1, "DEX:   %d", player->attr.dex);
        mvwprintw(wleft, 12, 1, "PHY:   %d", player->attr.phy);
        mvwprintw(wleft, 13, 1, "INT:   %d", player->attr.intl);
        mvwprintw(wleft, 14, 1, "WIS:   %d", player->attr.wis);
        mvwprintw(wleft, 15, 1, "CHA:   %d", player->attr.cha);
        mvwprintw(wleft, 16, 1, "XP:    %d", player->xp);
        mvwprintw(wleft, 17, 1, "Level: %d", player->level);


        mvwprintw(wstat, 1, 1, "== INVENTORY ==");
        mvwprintw(wstat, 2, 1, "Gold: %d", player->inventory->gold);
        
        i = 3;
        for(j = 0; j < 52; j++) {
                if(player->inventory->object[j]) {
                        //o = get_object_from_letter(slot_to_letter(j), player->inventory);
                        o = player->inventory->object[j];
                        if(is_worn(o)) {
                                mvwprintw(wstat, i, 1, "%c)   %s %s", slot_to_letter(j), a_an(o->fullname), is_ring(o) ? (o == pw_leftring ? "[<]" : "[>]") : "\0");
                                wattron(wstat, COLOR_PAIR(COLOR_GREEN));
                                mvwprintw(wstat, i, 4, "*"); 
                                wattroff(wstat, COLOR_PAIR(COLOR_GREEN));
                        } else {
                                mvwprintw(wstat, i, 1, "%c)   %s", slot_to_letter(j), a_an(o->fullname));
                        }
                        i++;
                }
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

void setup_color_pairs()
{
}

bool blocks_light(int y, int x)
{
        level_t *l = world->curlevel;

        if(hasbit(l->c[y][x].flags, CF_HAS_DOOR_CLOSED))
                return true;

        if(l->c[y][x].type == AREA_FOREST || l->c[y][x].type == AREA_CITY || l->c[y][x].type == AREA_VILLAGE) {    // trees and houses can be "see through" (e.g. if they are small)
                if(perc(20))
                        return false;
                else
                        return true;
        }

        switch(l->c[y][x].type) {
                case AREA_NOTHING:
                case AREA_MOUNTAIN:
                //case AREA_CITY:
                //case AREA_VILLAGE:
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
                        clearbit(l->c[y][x].flags, CF_LIT);
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
                        setbit(l->c[(int)oy][(int)ox].flags, CF_VISITED);
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


        //gtprintf("\tentering dofovlight");
        for(i = 0; i < (actor->viewradius/2); i++) {       // TODO: add a lightradius in actor_t, calculate it based on stuff
                if((int)oy >= 0 && (int)ox >= 0 && (int)oy < l->ysize && (int)ox < l->xsize) {
                        //gtprintf("\t\tchecking cell %d,%d", (int)oy, (int)ox);
                        if(hasbit(l->c[(int)oy][(int)ox].flags, CF_LIT))
                                return;

                        if(l->c[(int)oy][(int)ox].type == DNG_WALL) {
                                setbit(l->c[(int)oy][(int)ox].flags, CF_LIT);
                        }

                        if(blocks_light((int) oy, (int) ox)) {
                                //gtprintf("cell %d,%d blocks light", (int)oy, (int)ox);
                                return;
                        }

                        ox += x;
                        oy += y;
                }
        }
}

void FOVlight(actor_t *a, level_t *l)
{
        float x, y;
        int i;

        //gtprintf("entering FOVlight..");
        clear_map_to_unlit(l);
        for(i = 0; i < 360; i++) {
                x = cos((float) i * 0.01745f);
                y = sin((float) i * 0.01745f);
                //fprintf(stderr, "DEBUG: %s:%d - now going to dofovlight i = %d y = %.4f x = %.4f\n", __FILE__, __LINE__, i, y, x);
                dofovlight(a, l, x, y);
        }
}

void init_display()
{
        printf("Dummy display driver reporting for duty!\n");
}

void shutdown_display()
{
        printf("Shutting down dummy display driver!\n");
}

void draw_world(level_t *level)
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

int gtgetch()
{
        return ri(97,122);
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
