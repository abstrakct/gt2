/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _DISPALY_H
#define _DISPALY_H


// Prototypes
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
void delete_last_message();

bool blocks_light(int y, int x);

// Color definitions
#define C_BLACK_BLACK           0
#define C_BLACK_RED             1
#define C_BLACK_GREEN           2
#define C_BLACK_YELLOW          3
#define C_BLACK_BLUE            4
#define C_BLACK_MAGENTA         5
#define C_BLACK_CYAN            6
#define C_BLACK_WHITE           7
#define C_RED_BLACK             8
#define C_RED_RED               9
#define C_RED_GREEN             10
#define C_RED_YELLOW            11
#define C_RED_BLUE              12
#define C_RED_MAGENTA           13
#define C_RED_CYAN              14
#define C_RED_WHITE             15
#define C_GREEN_BLACK           16
#define C_GREEN_RED             17
#define C_GREEN_GREEN           18
#define C_GREEN_YELLOW          19
#define C_GREEN_BLUE            20
#define C_GREEN_MAGENTA         21
#define C_GREEN_CYAN            22
#define C_GREEN_WHITE           23
#define C_YELLOW_BLACK          24
#define C_YELLOW_RED            25
#define C_YELLOW_GREEN          26
#define C_YELLOW_YELLOW         27
#define C_YELLOW_BLUE           28
#define C_YELLOW_MAGENTA        29
#define C_YELLOW_CYAN           30
#define C_YELLOW_WHITE          31
#define C_BLUE_BLACK            32
#define C_BLUE_RED              33
#define C_BLUE_GREEN            34
#define C_BLUE_YELLOW           35
#define C_BLUE_BLUE             36
#define C_BLUE_MAGENTA          37
#define C_BLUE_CYAN             38
#define C_BLUE_WHITE            39
#define C_MAGENTA_BLACK         40
#define C_MAGENTA_RED           41
#define C_MAGENTA_GREEN         42
#define C_MAGENTA_YELLOW        43
#define C_MAGENTA_BLUE          44
#define C_MAGENTA_MAGENTA       45
#define C_MAGENTA_CYAN          46
#define C_MAGENTA_WHITE         47
#define C_CYAN_BLACK            48
#define C_CYAN_RED              49
#define C_CYAN_GREEN            50
#define C_CYAN_YELLOW           51
#define C_CYAN_BLUE             52
#define C_CYAN_MAGENTA          53
#define C_CYAN_CYAN             54
#define C_CYAN_WHITE            55
#define C_WHITE_BLACK           56
#define C_WHITE_RED             57
#define C_WHITE_GREEN           58
#define C_WHITE_YELLOW          59
#define C_WHITE_BLUE            60
#define C_WHITE_MAGENTA         61
#define C_WHITE_CYAN            62
#define C_WHITE_WHITE           63


#define COLOR_PLAIN    C_BLACK_WHITE
#define COLOR_FOREST   C_BLACK_GREEN
#define COLOR_CITY     C_BLACK_YELLOW
#define COLOR_VILLAGE  COLOR_CITY
#define COLOR_DUNGEON  COLOR_CITY
#define COLOR_MOUNTAIN C_BLACK_YELLOW
#define COLOR_LAKE     C_BLACK_BLUE
#define COLOR_PLAYER   56

#define COLOR_WARNING C_BLACK_RED
#define COLOR_BAD     C_BLACK_RED
#define COLOR_GOOD    C_BLACK_GREEN
#define COLOR_NORMAL  C_BLACK_WHITE
#define COLOR_INFO    C_BLACK_YELLOW
#define COLOR_VISIBLE C_BLACK_YELLOW

#define COLOR_SHADE   63
#define COLOR_LIGHT   62

#define COLOR_INVISIBLE 63

#endif
