/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef GT_H
#define GT_H

#define GT_VERSION_MAJ 0
#define GT_VERSION_MIN 0
#define GT_VERSION_REV 1

#define MAIN_DATA_FILE "data/data.cfg"

typedef struct {
        int width, height;
        int dead;
        unsigned int seed;
} game_t;

typedef struct {
        int color;
        char text[250];
} message_t;

// #define MAX_MESSAGES 100

extern monster_t *monsterdefs;
extern     obj_t *objdefs;

#define COLOR_WARNING 10
#define COLOR_GOOD    11
#define COLOR_BAD     12

/* function prototypes */

void mess(char *message);
void messc(int color, char *message);

#endif
