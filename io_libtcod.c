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

#define RGB_BLACK       {  0,  0,  0}
#define RGB_WHITE       {255,255,255}
#define RGB_RED         {255,  0,  0}
#define RGB_GREEN       {  0,255,  0}
#define RGB_BLUE        {  0,  0,255}
#define RGB_YELLOW      {255,255,  0}
#define RGB_MAGENTA     {255,  0,191}
#define RGB_CYAN        {  0,255,255}
#define RGB_DARKER_GREY { 63, 63, 63}
#define RGB_GOLD        {221,191,  0}
#define RGB_AMBER       {255,191,  0}
#define RGB_ORANGE      {255,127,  0}
#define RGB_LIMEGREEN   {191,255,  0}
#define RGB_CYAN        {  0,255,255}
#define RGB_SKYBLUE     {  0,191,255}
#define RGB_VIOLET      {127,  0,255}
#define RGB_CRIMSON     {255,  0, 63}
#define RGB_AZURE       {  0,127,255}

gtcolor_t colors[] = { 
        { RGB_BLACK,       RGB_BLACK },
        { RGB_RED,         RGB_BLACK },
        { RGB_GREEN,       RGB_BLACK },
        { RGB_YELLOW,      RGB_BLACK },
        { RGB_BLUE,        RGB_BLACK },
        { RGB_MAGENTA,     RGB_BLACK },
        { RGB_CYAN,        RGB_BLACK },
        { RGB_WHITE,       RGB_BLACK },
        { RGB_BLACK,       RGB_WHITE },
        { RGB_RED,         RGB_WHITE },
        { RGB_GREEN,       RGB_WHITE },
        { RGB_YELLOW,      RGB_WHITE },
        { RGB_BLUE,        RGB_WHITE },
        { RGB_MAGENTA,     RGB_WHITE },
        { RGB_CYAN,        RGB_WHITE },
        { RGB_WHITE,       RGB_WHITE },
        { RGB_DARKER_GREY, RGB_BLACK },
        { RGB_GOLD,        RGB_BLACK },
        { RGB_AMBER,       RGB_BLACK },
        { RGB_ORANGE,      RGB_BLACK },
        { RGB_LIMEGREEN,   RGB_BLACK },
        { RGB_SKYBLUE,     RGB_BLACK },
        { RGB_VIOLET,      RGB_BLACK },
        { RGB_CRIMSON,     RGB_BLACK },
        { RGB_AZURE,       RGB_BLACK },
};

extern int maxmess;

int numcommands;
cmd_t *curcommands;

/*           keycode         char pressed lalt lctrl ralt rctrl shift */
cmd_t normalcommands[] = {
        // movement
        { { TCODK_DOWN,         0,   1,     0,   0,   0,    0,    0 }, CMD_DOWN,        "Move down" },
        { { TCODK_CHAR,       'j',   1,     0,   0,   0,    0,    0 }, CMD_DOWN,        "Move down" },
        { { TCODK_KP2,          0,   1,     0,   0,   0,    0,    0 }, CMD_DOWN,        "Move down" },
        { { TCODK_UP,           0,   1,     0,   0,   0,    0,    0 }, CMD_UP,          "Move up" },
        { { TCODK_CHAR,       'k',   1,     0,   0,   0,    0,    0 }, CMD_UP,          "Move up" },
        { { TCODK_KP8,          0,   1,     0,   0,   0,    0,    0 }, CMD_UP,          "Move up" },
        { { TCODK_LEFT,         0,   1,     0,   0,   0,    0,    0 }, CMD_LEFT,        "Move left" },
        { { TCODK_CHAR,       'h',   1,     0,   0,   0,    0,    0 }, CMD_LEFT,        "Move left" },
        { { TCODK_KP4,          0,   1,     0,   0,   0,    0,    0 }, CMD_LEFT,        "Move left" },
        { { TCODK_RIGHT,        0,   1,     0,   0,   0,    0,    0 }, CMD_RIGHT,       "Move right" },
        { { TCODK_CHAR,       'l',   1,     0,   0,   0,    0,    0 }, CMD_RIGHT,       "Move right" },
        { { TCODK_KP6,          0,   1,     0,   0,   0,    0,    0 }, CMD_RIGHT,       "Move right" },
        { { TCODK_CHAR,       'y',   1,     0,   0,   0,    0,    0 }, CMD_NW,          "Move up-left" },
        { { TCODK_KP7,          0,   1,     0,   0,   0,    0,    0 }, CMD_NW,          "Move up-left" },
        { { TCODK_CHAR,       'u',   1,     0,   0,   0,    0,    0 }, CMD_NE,          "Move up-right" },
        { { TCODK_KP9,          0,   1,     0,   0,   0,    0,    0 }, CMD_NE,          "Move up-right" },
        { { TCODK_CHAR,       'b',   1,     0,   0,   0,    0,    0 }, CMD_SW,          "Move down-left" },
        { { TCODK_KP1,          0,   1,     0,   0,   0,    0,    0 }, CMD_SW,          "Move down-left" },
        { { TCODK_CHAR,       'n',   1,     0,   0,   0,    0,    0 }, CMD_SE,          "Move down-right" },
        { { TCODK_KP3,          0,   1,     0,   0,   0,    0,    0 }, CMD_SE,          "Move down-right" },

        // actions
        { { TCODK_CHAR,       'w',   0,     0,   0,   0,    0,    0 }, CMD_WIELDWEAR,   "Wield or wear an item" },
        { { TCODK_CHAR,       'r',   0,     0,   0,   0,    0,    0 }, CMD_UNWIELDWEAR, "Remove or unwield an item" },
        { { TCODK_CHAR,       ',',   1,     0,   0,   0,    0,    0 }, CMD_PICKUP,      "Pick up something" },
        { { TCODK_CHAR,       '.',   1,     0,   0,   0,    0,    0 }, CMD_REST,        "Rest one turn" },
        { { TCODK_CHAR,       '<',   1,     0,   0,   0,    0,    0 }, CMD_ASCEND,      "Go up stairs" },
        { { TCODK_CHAR,       '>',   1,     0,   0,   0,    0,    1 }, CMD_DESCEND,     "Go down stairs" },
        { { TCODK_CHAR,       'd',   0,     0,   0,   0,    0,    0 }, CMD_DROP,        "Drop an object" },
        { { TCODK_CHAR,       'q',   0,     0,   0,   0,    0,    0 }, CMD_QUAFF,       "Quaff a potion" },
        { { TCODK_CHAR,       'o',   1,     0,   0,   0,    0,    0 }, CMD_AUTOEXPLORE, "Autoexplore" },
        //{ { TCODK_CHAR,       'i', 1, 0, 0, 0, 0, 0 }, CMD_INVENTORY,   "Show inventory" },
        //{ TCODK_F5,  CMD_SAVE,        "Save" },
        //{ TCODK_F6,  CMD_LOAD,        "Load" },
#ifdef DEVELOPMENT_MODE
        { { TCODK_F1,           0, 1, 0, 0, 0, 0, 0 }, CMD_WIZARDMODE,  "Toggle wizard mode" },
        { { TCODK_F2,           0, 1, 0, 0, 0, 0, 0 }, CMD_DUMPOBJECTS, "" },
        { { TCODK_F5,           0, 1, 0, 0, 0, 0, 0 }, CMD_SAVE,        "Save game" },
        { { TCODK_CHAR,       '+', 1, 0, 0, 0, 0, 0 }, CMD_INCFOV,      "Increase FOV" },
        { { TCODK_CHAR,       '-', 1, 0, 0, 0, 0, 0 }, CMD_DECFOV,      "Decrease FOV" },
        { { TCODK_CHAR,       'f', 1, 0, 0, 0, 0, 0 }, CMD_FLOODFILL,   "Floodfill (debug)" },
        { { TCODK_CHAR,       's', 1, 0, 0, 0, 0, 0 }, CMD_SPAWNMONSTER,"Spawn monster" },
        { { TCODK_PAGEDOWN,    0,  1, 0, 0, 0, 0, 0 }, CMD_LONGDOWN,    "" },
        { { TCODK_PAGEUP,      0,  1, 0, 0, 0, 0, 0 }, CMD_LONGUP,      "" },
        { { TCODK_HOME,        0,  1, 0, 0, 0, 0, 0 }, CMD_LONGLEFT,    "" },
        { { TCODK_END,         0,  1, 0, 0, 0, 0, 0 }, CMD_LONGRIGHT,   "" },

        { { TCODK_CHAR,       'K', 1, 0, 0, 0, 0, 1 }, CMD_LONGUP,      "" },
        { { TCODK_CHAR,       'J', 1, 0, 0, 0, 0, 1 }, CMD_LONGDOWN,    "" },
        { { TCODK_CHAR,       'H', 1, 0, 0, 0, 0, 1 }, CMD_LONGLEFT,    "" },
        { { TCODK_CHAR,       'L', 1, 0, 0, 0, 0, 1 }, CMD_LONGRIGHT,   "" },
        { { TCODK_CHAR,       'v', 1, 0, 0, 0, 0, 0 }, CMD_TOGGLEFOV,   "Toggle FOV" },
        { { TCODK_CHAR,       'c', 1, 0, 0, 0, 0, 0 }, CMD_DUMPCOLORS, "" },
        { { TCODK_CHAR,       'a', 1, 0, 0, 0, 0, 0 }, CMD_DUMPAQ, "" },
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
        bool b;

//TCOD_console_flush();

        key = gtgetch();
        if(key.vk == TCODK_NONE)
                return 0;

        if(key.vk == TCODK_ESCAPE) {
                b = yesno("Are you sure you want to quit?");
                if(b)
                        return CMD_QUIT;
        }

        for(i=0; i<numcommands; i++) {
                if(cmp_keystruct(curcommands[i].key, key))
                        return curcommands[i].cmd;
        }

        return 0;
}

void init_commands()
{
        curcommands = normalcommands;
        numcommands = (sizeof(normalcommands) / sizeof(cmd_t));
}

// Pathfinding

void init_pathfinding(void *a)
{
        actor_t *actor;

        actor = (actor_t*)a;
        actor->path = TCOD_path_new_using_map(world->curlevel->map, 1.0f);
}

void update_path(void *actor)
{
        actor_t *a;

        a = (actor_t*)actor;
        TCOD_path_compute(a->path, a->x, a->y, a->goalx, a->goaly);
}

co get_next_step(void *actor)
{
        actor_t *a;
        co c;
        int x, y;

        a = (actor_t*)actor;
        
        TCOD_path_compute(a->path, a->x, a->y, a->goalx, a->goaly);
        if(!TCOD_path_walk(a->path, &x, &y, true)) {
                c.x = 0;
                c.y = 0;
        } else {
                c.x = x;
                c.y = y;
        }

        return c;
}

// General stuff

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

	if (fontsize < 1 || fontsize > 13) {
		for (fontsize = 13; fontsize > 1 && (fontwidths[fontsize - 1] * gtconfig.cols / 16 >= screenwidth || fontheights[fontsize - 1] * gtconfig.rows / 16 >= screenheight); fontsize--);
	}

        if(screenwidth <= 1024) {
        	sprintf(font, "fonts/ds.png");
        } else
                sprintf(font, "fonts/df.png");

	//sprintf(font, "fonts/font-%i.png", fontsize);
	//sprintf(font, "fonts/terminal16x16_gs_ro.png");

	//sprintf(font, "fonts/terminal8x12_gs_ro.png");
	sprintf(font, "fonts/terminal8x14_gs_ro.png");
	//sprintf(font, "fonts/terminal10x16_gs_ro.png");

        if(screenwidth <= 1024) {
                gtconfig.rows = screenheight / 8;
                gtconfig.rows -= 30;
                gtconfig.cols = screenwidth  / 8;
        } else {
                gtconfig.rows = screenheight / 15;
                gtconfig.cols = screenwidth  / 15;
        } 
                

        TCOD_console_set_custom_font(font, TCOD_FONT_TYPE_GREYSCALE | TCOD_FONT_LAYOUT_ASCII_INROW, 16, 16);

        TCOD_console_init_root(gtconfig.cols, gtconfig.rows, GAME_NAME, false, TCOD_RENDERER_SDL);
	TCOD_console_map_ascii_codes_to_font(0, 255, 0, 0);
	TCOD_console_set_keyboard_repeat(350, 70);

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

        TCOD_console_set_window_title(GAME_NAME);
}

void shutdown_display()
{
        printf("Shutting down!\n");
}

// LOS, FOV.

bool blocks_light(void *level, int y, int x)
{
        level_t *l;
        l = (level_t *)level;

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

void donewfov(actor_t *a, level_t *l)
{
        TCOD_map_compute_fov(l->map, a->x, a->y, a->viewradius, true, FOV_SHADOW);
}

void fov_updatemap(void *level)
{
        int x, y;
        bool trans, walk;
        level_t *l;

        l = (level_t *)level;

        for(x = 1; x < l->xsize; x++) {
                for(y = 1; y < l->ysize; y++) {
                        if(blocks_light(l, y, x))
                                trans = false;
                        else
                                trans = true;

                        if(passable(l, y, x))
                                walk = true;
                        else
                                walk = false;


                        TCOD_map_set_properties(l->map, x, y, trans, walk);
                }
        }
}

void fov_initmap(void *level)
{
        level_t *l;

        l = (level_t *)level;

        l->map = TCOD_map_new(l->xsize, l->ysize);
        fov_updatemap(l);
}

// The actual drawing on screen

void draw_map()
{
        int i,j, slot;
        int dx, dy;  // coordinates on screen!
        gtcolor_t color;
        level_t *level;

        level = world->curlevel;

        donewfov(player, level);

        /*
         * in this function, (j,i) are the coordinates on the map,
         * dx,dy = coordinates on screen.
         * so, player->py/px describes the upper left corner of the map
         */
        for(i = ppx, dx = 1; i < (ppx + game->map.w - 2); i++, dx++) {
                for(j = ppy, dy = 1; j < (ppy + game->map.h - 2); j++, dy++) {
                        if(j < level->ysize && i < level->xsize) {
                                if(TCOD_map_is_in_fov(level->map, i, j)) {
                                        set_cell_visited(level, j, i);
                                  /*      if(level->c[j][i].monster) {
                                                if(!hasbit(level->c[j][i].monster->flags, MF_SEENBYPLAYER)) {
                                                        setbit(level->c[j][i].monster->flags, MF_SEENBYPLAYER);
                                                        gtprintfc(COLOR_RED, "%s comes into view!", Upper(a_an(level->c[j][i].monster->name)));
                                                }
                                        }*/
                                        if(level->c[j][i].inventory && level->c[j][i].inventory->object[0]) {
                                                if(!hasbit(level->c[j][i].inventory->object[0]->flags, OF_SEENBYPLAYER)) {
                                                        setbit(level->c[j][i].inventory->object[0]->flags, OF_SEENBYPLAYER);
                                                        gtprintf("You found %s.", a_an(pair(level->c[j][i].inventory->object[0])));
                                                }
                                        }
                                }

                                if(hasbit(level->c[j][i].flags, CF_VISITED)) {
                                        if(TCOD_map_is_in_fov(level->map, i, j)) {
                                                if(ct(j, i) == DNG_WALL)
                                                        color = COLOR_LIT_WALL;
                                                else if(ct(j, i) == DNG_FLOOR)
                                                        color = COLOR_SHADE;
                                                else 
                                                        color = level->c[j][i].color;
                                        } else {
                                                color = COLOR_SHADE;
                                        }
                                }

                                if(hasbit(level->c[j][i].flags, CF_VISITED)) {
                                        gtmapaddch(dy, dx, color, mapchars[(int) level->c[j][i].type]);

                                        if(level->c[j][i].inventory) {
                                                if(level->c[j][i].inventory->gold > 0) {
                                                        gtmapaddch(dy, dx, COLOR_GOLD, objchars[OT_GOLD]);
                                                } else {                                                         // TODO ADD OBJECT COLORS!!!
                                                        slot = get_first_used_slot(level->c[j][i].inventory);
                                                        if(level->c[j][i].inventory->num_used > 0 && slot >= 0 && level->c[j][i].inventory->object[slot]) {
                                                                color = level->c[j][i].inventory->object[slot]->color;
                                                                gtmapaddch(dy, dx, color, objchars[level->c[j][i].inventory->object[slot]->type]);
                                                        }
                                                }
                                        }

                                        if(hasbit(level->c[j][i].flags, CF_HAS_DOOR_CLOSED)) {
                                                //dsprintf("closed door at %d,%d", dx, dy);
                                                gtmapaddch(dy, dx, color, '+');
                                        }
                                        if(hasbit(level->c[j][i].flags, CF_HAS_DOOR_OPEN)) {
                                                //dsprintf("open door at %d,%d", dx, dy);
                                                gtmapaddch(dy, dx, color, '\'');
                                        }
                                        
                                        if(hasbit(level->c[j][i].flags, CF_HAS_STAIRS_DOWN))
                                                gtmapaddch(dy, dx, COLOR_WHITE, '>');
                                        if(hasbit(level->c[j][i].flags, CF_HAS_STAIRS_UP))
                                                gtmapaddch(dy, dx, COLOR_WHITE, '<');
                                        if(TCOD_map_is_in_fov(level->map, i, j) && level->c[j][i].monster /*&& actor_in_lineofsight(player, level->c[j][i].monster)*/)
                                                gtmapaddch(dy, dx, COLOR_RED, (char) level->c[j][i].monster->c);

                                        if(hasbit(player->flags, MF_INVISIBLE))
                                                color = COLOR_PINVIS;
                                        else
                                                color = COLOR_PLAYER;

                                        if(j == ply && i == plx)
                                                gtmapaddch(dy, dx, color, '@');
                                }
                        }
                }
        }
}

void draw_map_old()
{
        int i,j, slot;
        int dx, dy;  // coordinates on screen!
        gtcolor_t color;
        level_t *level;

        level = world->curlevel;             // make sure world->curlevel is correct!

        /*
         * in this function, (j,i) are the coordinates on the map,
         * dx,dy = coordinates on screen.
         * so, player->py/px describes the upper left corner of the map
         */
        for(i = ppx, dx = 0; i <= (ppx + game->mapw - 2); i++, dx++) {
                for(j = ppy, dy = 0; j <= (ppy + game->maph - 2); j++, dy++) {
                        if(j < level->ysize && i < level->xsize) {
                                if(hasbit(level->c[j][i].flags, CF_VISITED)) {
                                        color = cc(j,i);

                                        if(hasbit(level->c[j][i].flags, CF_LIT))
                                                color = level->c[j][i].litcolor;

                                        gtmapaddch(dy, dx, color, mapchars[(int) level->c[j][i].type]);

                                        /*
                                        if(level->c[j][i].height < 0) {
                                                color = COLOR_RED;
                                                c = 48+(0 - level->c[j][i].height);
                                        } else {
                                                color = COLOR_BLUE;
                                                c = 48+level->c[j][i].height;
                                        }

                                        gtmapaddch(dy, dx, color, c);
                                        */

                                        if(level->c[j][i].inventory) {
                                                if(level->c[j][i].inventory->gold > 0) {
                                                        gtmapaddch(dy, dx, COLOR_YELLOW, objchars[OT_GOLD]);
                                                } else {                                                         // TODO ADD OBJECT COLORS!!!
                                                        slot = get_first_used_slot(level->c[j][i].inventory);
                                                        if(level->c[j][i].inventory->num_used > 0 && slot >= 0 && level->c[j][i].inventory->object[slot]) {
                                                                color = level->c[j][i].inventory->object[slot]->color;
                                                                gtmapaddch(dy, dx, color, objchars[level->c[j][i].inventory->object[slot]->type]);
                                                        }
                                                }
                                        }

                                        if(hasbit(level->c[j][i].flags, CF_HAS_DOOR_CLOSED))
                                                gtmapaddch(dy, dx, color, '+');
                                        else if(hasbit(level->c[j][i].flags, CF_HAS_DOOR_OPEN))
                                                gtmapaddch(dy, dx, color, '\'');
                                        else if(hasbit(level->c[j][i].flags, CF_HAS_STAIRS_DOWN))
                                                gtmapaddch(dy, dx, COLOR_WHITE, '>');
                                        else if(hasbit(level->c[j][i].flags, CF_HAS_STAIRS_UP))
                                                gtmapaddch(dy, dx, COLOR_WHITE, '<');
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
}

void draw_left()
{
        int i;
        TCOD_color_t color;
        float turn;

        turn = (float) game->tick / 10.0f;

        i = 0;

        //TCOD_console_set_alignment(game->left.c, TCOD_CENTER);
        TCOD_console_set_default_foreground(game->left.c, TCOD_light_blue);
        TCOD_console_print(game->left.c, (game->left.w/2)-(strlen(player->name)/2), i+1, "* %s *", player->name);

        i++;
        TCOD_console_set_default_foreground(game->left.c, TCOD_white);
        TCOD_console_print(game->left.c, 1, i+3, "Weapon: %s", player->weapon ? player->weapon->displayname : "bare hands");
        //TCOD_console_print(game->left.c, 1, i+4, "viewradius: %d", player->viewradius);
        
        /* Hitpoints */
        if(player->hp >= (player->maxhp/4*3))
                color = TCOD_green;
        else if(player->hp >= (player->maxhp/4) && player->hp < (player->maxhp/4*3))
                color = TCOD_yellow;
        else if(player->hp < (player->maxhp/4))
                color = TCOD_red;

        TCOD_console_print(game->left.c, 1, i+4, "HP:");
        TCOD_console_set_default_foreground(game->left.c, color);
        TCOD_console_print(game->left.c, 5, i+4, "%d/%d (%.1f%%)", player->hp, player->maxhp, ((float)(100/(float)player->maxhp) * (float)player->hp));

        TCOD_console_set_default_foreground(game->left.c, TCOD_white);
        TCOD_console_print(game->left.c, 1, i+6,  "Turn:  %.1f", player->speed, turn);
        TCOD_console_print(game->left.c, 1, i+7,  "STR:   %d", get_strength(player));
        TCOD_console_print(game->left.c, 1, i+8,  "DEX:   %d", get_dexterity(player));
        TCOD_console_print(game->left.c, 1, i+9,  "PHY:   %d", get_physique(player));
        TCOD_console_print(game->left.c, 1, i+10, "INT:   %d", get_intelligence(player));
        TCOD_console_print(game->left.c, 1, i+11, "WIS:   %d", get_wisdom(player));
        TCOD_console_print(game->left.c, 1, i+12, "CHA:   %d", get_charisma(player));
        TCOD_console_print(game->left.c, 1, i+13, "AC:    %d", player->ac);
        TCOD_console_print(game->left.c, 1, i+14, "XP:    %d", player->xp);
        TCOD_console_print(game->left.c, 1, i+15, "Level: %d", player->level);
        TCOD_console_print(game->left.c, 1, i+16, "Speed: %d", player->speed);

        if(player->temp[TEMP_INVISIBLE]) {
                TCOD_console_set_default_foreground(game->left.c, TCOD_sea);
                TCOD_console_print(game->left.c, 1, i+18, "Invisible");
        }

        if(player->temp[TEMP_STRENGTH]) {
                TCOD_console_set_default_foreground(game->left.c, TCOD_green);
                TCOD_console_print(game->left.c, 1, i+18, "Strong");
        }

        if(player->temp[TEMP_WISDOM]) {
                TCOD_console_set_default_foreground(game->left.c, TCOD_green);
                TCOD_console_print(game->left.c, 1, i+18, "Wise");
        }

        
        //TCOD_console_print(game->left.c, 1, i+9, 1, "Dungeon level: %d (out of %d)", game->currentlevel, game->createdareas);
        //mvwprintw(wleft, 3, 1, "y,x     %d,%d", ply, plx);
        //mvwprintw(wleft, 4, 1, "(py,px) (%d,%d)", ppy, ppx);
/*        obj_t *o;
        int i, j;
        int color;

        werase(wleft);
        werase(wstat);
        box(wleft, ACS_VLINE, ACS_HLINE);                                                                                                                                                                                                                                                                         
        box(wstat, ACS_VLINE, ACS_HLINE);                                                                                                                                                                                                                                                                         

        mvwprintw(wleft, 1, 1, "Name:");
        mvwprintw(wleft, 2, 1, "Turn:   %d", game->turn);
        mvwprintw(wleft, 3, 1, "Weapon: %s", player->weapon ? player->weapon->displayname : "bare hands");
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
                                mvwprintw(wstat, i, 1, "%c)   %s %s", slot_to_letter(j), a_an(o->displayname), is_bracelet(o) ? (o == pw_leftbracelet ? "[<]" : "[>]") : "\0");
                                wattron(wstat, COLOR_PAIR(COLOR_GREEN));
                                mvwprintw(wstat, i, 4, "*"); 
                                wattroff(wstat, COLOR_PAIR(COLOR_GREEN));
                        } else {
                                mvwprintw(wstat, i, 1, "%c)   %s", slot_to_letter(j), a_an(o->displayname));
                        }
                        i++;
                }
        }*/
}

void draw_right()
{
        obj_t *o;
        int i, j;

        TCOD_console_set_default_foreground(game->right.c, TCOD_light_blue);
        TCOD_console_print(game->right.c, (game->right.w/2)-7, 1, "* INVENTORY *");

        TCOD_console_set_default_foreground(game->right.c, TCOD_gold);
        TCOD_console_print(game->right.c, 1, 3, "Gold: %d", player->inventory->gold);


        TCOD_console_set_default_foreground(game->right.c, TCOD_white);
        
        i = 4;
        for(j = 0; j < 52; j++) {
                if(player->inventory->object[j]) {
                        //o = get_object_from_letter(slot_to_letter(j), player->inventory);
                        //
                        //TODO:SIMPLIFY
                        o = player->inventory->object[j];
                        if(is_worn(o)) {
                                TCOD_console_print(game->right.c, 1, i, "%c", slot_to_letter(j));
                                TCOD_console_put_char_ex(game->right.c, 3, i, '*', TCOD_light_green, TCOD_black);
                                TCOD_console_set_default_foreground(game->right.c, o->color.fore);
                                TCOD_console_set_default_background(game->right.c, o->color.back);
                                TCOD_console_print(game->right.c, 5, i, "%s %s", a_an(pair(o)), is_bracelet(o) ? (o == pw_leftbracelet ? "[<]" : "[>]") : "\0");
                                TCOD_console_set_default_foreground(game->right.c, TCOD_white);
                                TCOD_console_set_default_background(game->right.c, TCOD_black);
                        } else {
                                TCOD_console_print(game->right.c, 1, i, "%c", slot_to_letter(j));
                                TCOD_console_put_char_ex(game->right.c, 3, i, '-', TCOD_white, TCOD_black);
                                TCOD_console_set_default_foreground(game->right.c, o->color.fore);
                                TCOD_console_set_default_background(game->right.c, o->color.back);
                                TCOD_console_print(game->right.c, 5, i, "%s", a_an(pair(o)));
                                TCOD_console_set_default_foreground(game->right.c, TCOD_white);
                                TCOD_console_set_default_background(game->right.c, TCOD_black);
                        }
                        i++;
                }
        }
}

void update_ticks()
{
        float turn;

        turn = (float) game->tick / 10.0f;
        TCOD_console_print(game->left.c, 1, 7,  "Turn: %.1f", player->speed, turn);
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
        TCOD_console_clear(game->map.c);
        TCOD_console_clear(game->left.c);
        TCOD_console_clear(game->right.c);

        //TCOD_console_rect(game->map.c, game->map.x, game->map.y, game->map.w, game->map.h, true, TCOD_BKGND_NONE);
        /*TCOD_console_print_frame(game->map.c, 0, 0, game->map.w, game->map.h, true, TCOD_BKGND_NONE, "Map");
        TCOD_console_print_frame(game->left.c, 0, 0, game->left.w, game->left.h, true, TCOD_BKGND_NONE, "You");
        TCOD_console_print_frame(game->right.c, 0, 0, game->right.w - 2, game->right.h, true, TCOD_BKGND_NONE, "Inventory");*/

        draw_map();
        draw_left();
        draw_right();

        TCOD_console_blit(game->map.c, 0, 0, game->map.w, game->map.h, NULL, game->map.x, game->map.y, 1.0, 1.0);
        TCOD_console_blit(game->messages.c, 0, 0, game->messages.w, game->messages.h, NULL, game->messages.x, game->messages.y, 1.0, 1.0);
        TCOD_console_blit(game->left.c, 0, 0, game->left.w, game->left.h, NULL, game->left.x, game->left.y, 1.0, 1.0);
        TCOD_console_blit(game->right.c, 0, 0, game->right.w, game->right.h, NULL, game->right.x, game->right.y, 1.0, 1.0);

        TCOD_console_flush();
}

void initial_update_screen()
{
        gtprintf("Welcome to %s v%s!", GAME_NAME, game->version);
        update_screen();
}

// Input and messages

char ask_char(char *question)
{
        gtkey key;

        gtprintf(question);
        update_screen();

        TCOD_console_flush();
        key = TCOD_console_wait_for_keypress(true);
        key = TCOD_console_wait_for_keypress(true);
        //key = TCOD_console_check_for_keypress(TCOD_KEY_PRESSED);

        if(key.shift && key.c >= 'a' && key.c <= 'z')
                key.c += 'A' - 'a';

        return key.c;
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
        while(1) { 
                if(c.c == 'y' || c.c == 'Y')
                        return true;
                if(c.c == 'n' || c.c == 'N')
                        return false;
                
                c = gtgetch();
        }

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

bool gt_checkforkeypress()
{
        TCOD_key_t key;

        //TCOD_console_flush();
        key = TCOD_console_check_for_keypress(TCOD_KEY_PRESSED);
        if(key.vk == TCODK_NONE)
                return false;
        else
                return true;
}

gtkey gtgetch()
{
        TCOD_key_t key;

        TCOD_console_flush();
        key = TCOD_console_wait_for_keypress(false);
        //key = TCOD_console_check_for_keypress(TCOD_KEY_PRESSED);

        if(key.shift && key.c >= 'a' && key.c <= 'z')
                key.c += 'A' - 'a';
        if(key.shift && key.c == '<')
                key.c = '>';

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

        fprintf(messagefile, "%d - %s\n", game->tick, messages[currmess-1].text);
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

        char debugmessage[1000];
        sprintf(debugmessage, "%d: %s", game->tick, message);
        //fprintf(stderr, "%s", debugmessage);

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

        char debugmessage[1000];
        sprintf(debugmessage, "%d: %s", game->tick, message);
        //fprintf(stderr, "%s", debugmessage);

        scrollmessages();
        messages[currmess].color = color;
        strcpy(messages[currmess].text, message);
        domess();
}

//


// vim: fdm=syntax guifont=Terminus\ 8
