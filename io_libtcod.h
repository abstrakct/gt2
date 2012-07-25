/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _IOLIBTCOD_H
#define _IOLIBTCOD_H

typedef struct coord {
        int y;
        int x;
} co;

// Prototypes
void init_display();
void shutdown_display();

void gtmapaddch(int y, int x, gtcolor_t color, char c);
void update_screen();
void update_player();
void update_ticks();
void initial_update_screen();
gtkey gtgetch();

void domess();
void scrollmessages();
void mess(char *message);
void messc(gtcolor_t color, char *message);
void delete_last_message();

bool blocks_light(void *l, int y, int x);
void fov_initmap(void *l);
void fov_updatemap(void *level);
void init_pathfinding(void *a);
co get_next_step(void *actor);

int get_command();
void init_commands();
bool cmp_keystruct(gtkey a, gtkey b);
char ask_char(char *question);
char ask_for_hand();
bool yesno(char *fmt, ...);
void more();
bool gt_checkforkeypress();

void gtmsgbox(char *header, char *message);

extern  gtcolor_t colors[];

#define COLS  180                          // x
#define ROWS   60                          // y

// Color definitions
#define C_BLACK_BLACK           0
#define C_BLACK_RED             1
#define C_BLACK_GREEN           2
#define C_BLACK_YELLOW          3
#define C_BLACK_BLUE            4
#define C_BLACK_MAGENTA         5
#define C_BLACK_CYAN            6
#define C_BLACK_WHITE           7
#define C_WHITE_BLACK           8
#define C_WHITE_RED             9
#define C_WHITE_GREEN           10
#define C_WHITE_YELLOW          11
#define C_WHITE_BLUE            12
#define C_WHITE_MAGENTA         13
#define C_WHITE_CYAN            14
#define C_WHITE_WHITE           15
#define C_BLACK_DARKERGREY      16
#define C_BLACK_GOLD            17
#define C_BLACK_AMBER           18
#define C_BLACK_ORANGE          19
#define C_BLACK_LIMEGREEN       20
#define C_BLACK_SKYBLUE         21
#define C_BLACK_VIOLET          22
#define C_BLACK_CRIMSON         23
#define C_BLACK_AZURE           24
#define C_BLACK_DARKRED         25
#define C_BLACK_SEAGREEN        26
#define C_BLACK_PURPLE          27
#define C_BLACK_BROWN           28


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

#define COLOR_YELLOW    colors[C_BLACK_YELLOW]
#define COLOR_BLUE      colors[C_BLACK_BLUE]
#define COLOR_GREEN     colors[C_BLACK_GREEN]
#define COLOR_RED       colors[C_BLACK_RED]
#define COLOR_WHITE     colors[C_BLACK_WHITE]
#define COLOR_MAGENTA   colors[C_BLACK_MAGENTA]
#define COLOR_AMBER     colors[C_BLACK_AMBER]
#define COLOR_ORANGE    colors[C_BLACK_ORANGE]
#define COLOR_LIMEGREEN colors[C_BLACK_LIMEGREEN]
#define COLOR_VIOLET    colors[C_BLACK_VIOLET]
#define COLOR_CRIMSON   colors[C_BLACK_CRIMSON]
#define COLOR_CYAN      colors[C_BLACK_CYAN]
#define COLOR_AZURE     colors[C_BLACK_AZURE]
#define COLOR_SKYBLUE   colors[C_BLACK_SKYBLUE]
#define COLOR_DARKRED   colors[C_BLACK_DARKRED]
#define COLOR_SEAGREEN  colors[C_BLACK_SEAGREEN]
#define COLOR_PURPLE    colors[C_BLACK_PURPLE]
#define COLOR_BROWN     colors[C_BLACK_BROWN]

// some special potion "colors"
#define COLOR_FIZZY    colors[C_WHITE_BLACK]
#define COLOR_CLOUDY   colors[C_WHITE_CYAN]

#define COLOR_PLAIN    colors[C_BLACK_WHITE]
#define COLOR_FOREST   colors[C_BLACK_GREEN]
#define COLOR_CITY     colors[C_BLACK_YELLOW]
#define COLOR_VILLAGE  colors[C_BLACK_YELLOW]
#define COLOR_DUNGEON  colors[C_BLACK_YELLOW]
#define COLOR_MOUNTAIN colors[C_BLACK_RED]
#define COLOR_LAKE     colors[C_BLACK_BLUE]
#define COLOR_PLAYER   colors[C_BLACK_BLUE]
#define COLOR_PINVIS   colors[C_WHITE_BLUE]

#define COLOR_WARNING  colors[C_BLACK_RED]
#define COLOR_BAD      colors[C_BLACK_RED]
#define COLOR_GOOD     colors[C_BLACK_GREEN]
#define COLOR_NORMAL   colors[C_BLACK_WHITE]
#define COLOR_INFO     colors[C_BLACK_YELLOW]
#define COLOR_VISIBLE  colors[C_BLACK_YELLOW]
#define COLOR_GOLD     colors[C_BLACK_GOLD]
#define COLOR_LIT_WALL colors[C_BLACK_AMBER]

#define COLOR_SHADE    colors[C_BLACK_DARKERGREY]
#define COLOR_LIGHT    colors[C_BLACK_YELLOW]

#define COLOR_INVISIBLE 63

#endif
// vim: fdm=syntax guifont=Terminus\ 8
