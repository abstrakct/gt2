/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _DISPALY_H
#define _DISPALY_H

#define COLOR_PLAIN    0
#define COLOR_FOREST   1
#define COLOR_CITY     2
#define COLOR_VILLAGE  2
#define COLOR_DUNGEON  2
#define COLOR_MOUNTAIN 3
#define COLOR_LAKE     4
#define COLOR_PLAYER   5

#define COLOR_WARNING 10
#define COLOR_BAD     COLOR_WARNING
#define COLOR_GOOD    1               // Green
#define COLOR_NORMAL  COLOR_PLAIN     // White

#define COLOR_INVISIBLE 15            // ???

void init_display();
void shutdown_display();
void draw_world(level_t *level);
void draw_wstat();

void gtmapaddch(int y, int x, int color, char c);
void update_screen();
void update_player();
void initial_update_screen();
int  gtgetch();

void domess();
void scrollmessages();
void mess(char *message);
void messc(int color, char *message);

bool blocks_light(int type);


#endif
