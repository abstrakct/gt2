/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * This file deals with diplaying output!
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "world.h"
#include "datafiles.h"
#include "io.h"
#include "gt.h"
#include "utils.h"
#include "commands.h"

#include <libtcod/libtcod.h>

#define RGB_BLACK {0,0,0}
#define RGB_WHITE {255,255,255}
#define RGB_RED   {255,0,0}
#define RGB_GREEN {0,255,0}
#define RGB_BLUE  {0,0,255}
#define RGB_YELLOW {255,255,0}
#define RGB_MAGENTA {255,0,191}
#define RGB_CYAN {0,255,255}

gtcolor_t colors[] = { 
        { RGB_BLACK,    RGB_BLACK },
        { RGB_RED,      RGB_BLACK },
        { RGB_GREEN,    RGB_BLACK },
        { RGB_YELLOW,   RGB_BLACK },
        { RGB_BLUE,     RGB_BLACK },
        { RGB_MAGENTA,  RGB_BLACK },
        { RGB_CYAN,     RGB_BLACK },
        { RGB_WHITE,    RGB_BLACK },
        { RGB_BLACK,    RGB_WHITE },
        { RGB_RED,      RGB_WHITE },
        { RGB_GREEN,    RGB_WHITE },
        { RGB_YELLOW,   RGB_WHITE },
        { RGB_BLUE,     RGB_WHITE },
        { RGB_MAGENTA,  RGB_WHITE },
        { RGB_CYAN,     RGB_WHITE },
        { RGB_WHITE,    RGB_WHITE }
};


extern int maxmess;

int numcommands;
cmd_t *curcommands;

/*           keycode         char pressed lalt lctrl ralt rctrl shift */
cmd_t outsidecommands[] = {
        { { TCODK_DOWN,         0,   1,     0,   0,   0,    0,    0 }, CMD_DOWN,        "Move down" },
        { { TCODK_CHAR,       'j',   1,     0,   0,   0,    0,    0 }, CMD_DOWN,        "Move down" },
        { { TCODK_UP,           0,   1,     0,   0,   0,    0,    0 }, CMD_UP,          "Move up" },
        { { TCODK_CHAR,       'k',   1,     0,   0,   0,    0,    0 }, CMD_UP,          "Move up" },
        { { TCODK_LEFT,         0,   1,     0,   0,   0,    0,    0 }, CMD_LEFT,        "Move left" },
        { { TCODK_CHAR,       'h',   1,     0,   0,   0,    0,    0 }, CMD_LEFT,        "Move left" },
        { { TCODK_RIGHT,        0,   1,     0,   0,   0,    0,    0 }, CMD_RIGHT,       "Move right" },
        { { TCODK_CHAR,       'l',   1,     0,   0,   0,    0,    0 }, CMD_RIGHT,       "Move right" },
        { { TCODK_CHAR,       'y',   1,     0,   0,   0,    0,    0 }, CMD_NW,          "Move up-left" },
        { { TCODK_CHAR,       'u',   1,     0,   0,   0,    0,    0 }, CMD_NE,          "Move up-right" },
        { { TCODK_CHAR,       'b',   1,     0,   0,   0,    0,    0 }, CMD_SW,          "Move down-left" },
        { { TCODK_CHAR,       'n',   1,     0,   0,   0,    0,    0 }, CMD_SE,          "Move down-right" },
        { { TCODK_CHAR,       'w',   0,     0,   0,   0,    0,    0 }, CMD_WIELDWEAR,   "Wield or wear an item" },
        { { TCODK_CHAR,       'r',   0,     0,   0,   0,    0,    0 }, CMD_UNWIELDWEAR, "Remove or unwield an item" },
        { { TCODK_CHAR,       ',',   1,     0,   0,   0,    0,    0 }, CMD_PICKUP,      "Pick up something" },
        { { TCODK_CHAR,       '.',   1,     0,   0,   0,    0,    0 }, CMD_REST,        "Rest one turn" },
        { { TCODK_CHAR,       '<',   1,     0,   0,   0,    0,    0 }, CMD_ASCEND,      "Go up stairs" },
        { { TCODK_CHAR,       '>',   1,     0,   0,   0,    0,    0 }, CMD_DESCEND,     "Go down stairs" },
        { { TCODK_CHAR,       'd',   0,     0,   0,   0,    0,    0 }, CMD_DROP,        "Drop an object" },
        //{ { TCODK_CHAR,       'i', 1, 0, 0, 0, 0, 0 }, CMD_INVENTORY,   "Show inventory" },
        //{ TCODK_F5,  CMD_SAVE,        "Save" },
        //{ TCODK_F6,  CMD_LOAD,        "Load" },
#ifdef DEVELOPMENT_MODE
        { { TCODK_F1,           0, 1, 0, 0, 0, 0, 0 }, CMD_WIZARDMODE,  "Toggle wizard mode" },
        //{ TCODK_F2,  CMD_INCTIME,     "Time travel!?" },
        { { TCODK_CHAR,       '+', 1, 0, 0, 0, 0, 0 }, CMD_INCFOV,      "Increase FOV" },
        { { TCODK_CHAR,       '-', 1, 0, 0, 0, 0, 0 }, CMD_DECFOV,      "Decrease FOV" },
        { { TCODK_CHAR,       'f', 1, 0, 0, 0, 0, 0 }, CMD_FLOODFILL,   "Floodfill (debug)" },
        { { TCODK_CHAR,       's', 1, 0, 0, 0, 0, 0 }, CMD_SPAWNMONSTER,"Spawn monster" },
        { { TCODK_PAGEDOWN,    0,  1, 0, 0, 0, 0, 0 }, CMD_LONGDOWN,    "" },
        { { TCODK_PAGEUP,      0,  1, 0, 0, 0, 0, 0 }, CMD_LONGUP,      "" },
        { { TCODK_HOME,        0,  1, 0, 0, 0, 0, 0 }, CMD_LONGLEFT,    "" },
        { { TCODK_END,         0,  1, 0, 0, 0, 0, 0 }, CMD_LONGRIGHT,   "" },

        /*{ { TCODK_CHAR,       'K', 1, 0, 0, 0, 0, 0 }, CMD_LONGUP,      "" },
        { { TCODK_CHAR,       'J', 1, 0, 0, 0, 0, 0 }, CMD_LONGDOWN,    "" },
        { { TCODK_CHAR,       'H', 1, 0, 0, 0, 0, 0 }, CMD_LONGLEFT,    "" },
        { { TCODK_CHAR,       'L', 1, 0, 0, 0, 0, 0 }, CMD_LONGRIGHT,   "" },*/
        { { TCODK_CHAR,       'v', 1, 0, 0, 0, 0, 0 }, CMD_TOGGLEFOV,   "Toggle FOV" },
        //{ KEY_F(4),  CMD_DUMPOBJECTS, "Dump objects" },
        { { TCODK_CHAR,       'o', 1, 0, 0, 0, 0, 0 }, CMD_DUMPOBJECTS, "" },
        { { TCODK_CHAR,       'c', 1, 0, 0, 0, 0, 0 }, CMD_DUMPCOLORS, "" },
        { { TCODK_CHAR,       'p', 1, 0, 0, 0, 0, 0 }, CMD_PATHFINDER, "" },
#endif
        //{ , CMD_IDENTIFYALL, "Identify everything" },
        //{ , CMD_SKILLSCREEN, "Show skills" },
};

bool cmp_keystruct(gtkey a, gtkey b)
{
        if((a.vk == b.vk) &&
           (a.pressed == b.pressed) &&
           (a.c == b.c) &&
           (a.lalt == b.lalt) &&
           (a.lctrl == b.lctrl) &&
           (a.ralt == b.ralt) &&
           (a.rctrl == b.rctrl) &&
           (a.shift == b.shift))
                return true;                              /* they're the same */
        else
                return false;
}

int get_command()
{
        int i;
        gtkey key;

        TCOD_console_flush();

        key = gtgetch();
        if(key.vk == TCODK_NONE)
                return 0;

        if(key.vk == TCODK_ESCAPE)
                return CMD_QUIT;       // easy exit even if C&C breaks down!

        for(i=0; i<numcommands; i++) {
                if(cmp_keystruct(curcommands[i].key, key))
                        return curcommands[i].cmd;
        }

        return 0;
}

void init_commands()
{
        curcommands = outsidecommands;
        numcommands = (sizeof(outsidecommands) / sizeof(cmd_t));
}

char ask_char(char *question)
{
        gtkey c;

        gtprintf(question);
        update_screen();
        c = gtgetch();
        return c.c;
}

char ask_for_hand()
{
        gtkey c;
        
        while(1) {
                gtprintf("Which hand - (l)eft or (r)ight?");
                update_screen();
                c = gtgetch();
//fprintf(stderr, "DEBUG: %s:%d - you pressed key with decimal value %d\n", __FILE__, __LINE__, c);
                if(c.c == 13 || c.c == 27)         // ENTER or ESCAPE
                        return 0;
                else if(c.c == 'l' || c.c == 'r')
                        return c.c;
                else
                        gtprintf("Only (l)eft or (r)ight, please.");
        }

}

bool yesno(char *fmt, ...)
{
        va_list argp;
        char s[1000];
        gtkey c;

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        strcat(s, " (y/n)?");
        mess(s);

        update_screen();
        c = gtgetch();
        if(c.c == 'y' || c.c == 'Y')
                return true;
        if(c.c == 'n' || c.c == 'N')
                return false;

        return false;
}

void more()
{
        gtkey c;

        gtprintfc(COLOR_WHITE, "-- more --");
        while(1) {
                c = gtgetch();
                if(c.c == 13 || c.c == 32) {
                        delete_last_message();
                        return;
                }
        }
}

void init_display()
{
        /* font selection code is stolen from brogue! */
	char font[60];
	int fontsize = -1;
	
	int screenwidth, screenheight;
	int fontwidths[13] = {112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304}; // widths of the font graphics (divide by 16 to get individual character width)
	int fontheights[13] = {176, 208, 240, 272, 304, 336, 368, 400, 432, 464, 496, 528, 528}; // heights of the font graphics (divide by 16 to get individual character height)

	TCOD_sys_get_current_resolution(&screenwidth, &screenheight);

	// adjust for title bars and whatever -- very approximate, but better than the alternative
	screenwidth -= 6;
	screenheight -= 48;

	gtconfig.rows = ROWS;
	gtconfig.cols = COLS;

	if (fontsize < 1 || fontsize > 13) {
		for (fontsize = 13; fontsize > 1 && (fontwidths[fontsize - 1] * gtconfig.cols / 16 >= screenwidth || fontheights[fontsize - 1] * gtconfig.rows / 16 >= screenheight); fontsize--);
	}

        if(screenwidth <= 1024) {
        	sprintf(font, "fonts/ds.png");
        	//gtconfig.rows *= 2;
        } else
                sprintf(font, "fonts/df.png");

	//sprintf(font, "fonts/font-%i.png", fontsize);
        TCOD_console_set_custom_font(font, /*TCOD_FONT_TYPE_GREYSCALE |*/ TCOD_FONT_LAYOUT_ASCII_INROW, 16, 16);

        TCOD_console_init_root(gtconfig.cols, gtconfig.rows, GAME_NAME, false, TCOD_RENDERER_SDL);
	TCOD_console_map_ascii_codes_to_font(0, 255, 0, 0);
	//TCOD_console_set_keyboard_repeat(350, 60);

        game->width = gtconfig.cols;
        game->height = gtconfig.rows;

        game->left.w = gtconfig.cols/4;
        game->left.h = (gtconfig.rows/3) * 2;
        game->left.x = 0;
        game->left.y = 0;
        game->left.c = TCOD_console_new(game->left.w, game->left.h);

        game->map.w = gtconfig.cols/2;
        game->map.h = (gtconfig.rows/3) * 2;
        game->map.x = game->left.w + 1;
        game->map.y = 0;
        game->map.c = TCOD_console_new(game->map.w, game->map.h);

        game->right.w = gtconfig.cols/4;
        game->right.h = (gtconfig.rows/3) * 2;
        game->right.x = game->map.x + game->map.w + 1;
        game->right.y = 0;
        game->right.c = TCOD_console_new(game->left.w, game->left.h);

        game->messages.w = gtconfig.cols;
        game->messages.h = gtconfig.rows / 3;
        game->messages.x = 0;
        game->messages.y = game->left.h + 1;
        game->messages.c = TCOD_console_new(game->messages.w, game->messages.h);
        TCOD_console_set_default_foreground(game->messages.c, TCOD_white);
        TCOD_console_set_default_background(game->messages.c, TCOD_black);
        maxmess = game->messages.h - 2;
}

void shutdown_display()
{
        printf("Shutting down!\n");
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

void draw_world()
{
}

void draw_wstat()
{
/*        obj_t *o;
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
        mvwprintw(wleft, 6, 5, "%d/%d (%.1f%%)", player->hp, player->maxhp, ((float)(100/(float)player->maxhp) * (float)player->hp));
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
                                mvwprintw(wstat, i, 1, "%c)   %s %s", slot_to_letter(j), a_an(o->fullname), is_bracelet(o) ? (o == pw_leftbracelet ? "[<]" : "[>]") : "\0");
                                wattron(wstat, COLOR_PAIR(COLOR_GREEN));
                                mvwprintw(wstat, i, 4, "*"); 
                                wattroff(wstat, COLOR_PAIR(COLOR_GREEN));
                        } else {
                                mvwprintw(wstat, i, 1, "%c)   %s", slot_to_letter(j), a_an(o->fullname));
                        }
                        i++;
                }
        }*/
}

void update_player()
{
        gtmapaddch(player->oldy, player->oldx, cc(player->oldy, player->oldx), mapchars[(int) ct(player->oldy, player->oldx)]);
        gtmapaddch(ply, plx, COLOR_PLAYER, '@');
}

void gtmapaddch(int y, int x, gtcolor_t color, char c)
{
        TCOD_console_put_char_ex(game->map.c, x, y, c, color.fore, color.back);
}

void update_screen()
{
}

void initial_update_screen()
{
}

// Input and messages

gtkey gtgetch()
{
        TCOD_key_t key;

        TCOD_console_flush();
        key = TCOD_console_wait_for_keypress(false);
        
        //key = TCOD_console_check_for_keypress(TCOD_KEY_PRESSED);

        return key;
}

void domess()
{
        int i;

        TCOD_console_set_default_foreground(game->messages.c, TCOD_white);
        TCOD_console_clear(game->messages.c);
        currmess++;
        for(i = maxmess-1; i > 0; i--) {
                TCOD_console_set_default_foreground(game->messages.c, messages[i].color.fore);
                TCOD_console_set_default_background(game->messages.c, messages[i].color.back);
                TCOD_console_print(game->messages.c, 1, i, "%s", messages[i].text);
        }
        TCOD_console_blit(game->messages.c, 0, 0, game->messages.w, game->messages.h, NULL, game->messages.x, game->messages.y, 1.0, 1.0);
        TCOD_console_flush();

        fprintf(messagefile, "%d - %s\n", game->turn, messages[currmess-1].text);
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
        //if(!strcmp(message, messages[currmess-1].text))
                //return;

        scrollmessages();
        messages[currmess].color = COLOR_NORMAL;
        strcpy(messages[currmess].text, message);
        domess();
}

void delete_last_message()
{
        messages[currmess-1].color = COLOR_NORMAL;
        messages[currmess-1].text[0] = '\0';
        domess();
}

void messc(gtcolor_t color, char *message)
{
        //if(!strcmp(message, messages[currmess-1].text))
                //return;

        scrollmessages();
        messages[currmess].color = color;
        strcpy(messages[currmess].text, message);
        domess();
}

// vim: fdm=syntax guifont=Terminus\ 8