/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _DISPALY_H
#define _DISPALY_H

#define COLOR_PLAIN    AREA_PLAIN
#define COLOR_FOREST   AREA_FOREST
#define COLOR_CITY     AREA_CITY
#define COLOR_MOUNTAIN AREA_MOUNTAIN
#define COLOR_VILLAGE  COLOR_CITY
#define COLOR_DUNGEON  AREA_DUNGEON
#define COLOR_LAKE     AREA_LAKE

#define COLOR_WARNING 10
#define COLOR_GOOD    COLOR_FOREST    // Green
#define COLOR_BAD     COLOR_WARNING
#define COLOR_INFO    13
#define COLOR_PLAYER  COLOR_BAD
#define COLOR_NORMAL  COLOR_PLAIN

void init_display();
void shutdown_display();
void draw_world();
void gtmapaddch(int x, int y, int color, char c);
void update_screen();
void initial_update_screen();
char gtgetch();
void domess();
void scrollmessages();
void mess(char *message);
void messc(int color, char *message);



#endif
