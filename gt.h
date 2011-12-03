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


extern monster_t *monsterdefs;
extern     obj_t *objdefs;

#endif
