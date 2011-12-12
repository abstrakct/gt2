/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _WORLD_H
#define _WORLD_H

#define NOTHING         0
#define PLAIN		1
#define MOUNTAIN	2
#define FOREST		3
#define CITY		4
#define VILLAGE		5
#define DUNGEON         6

#define YSIZE 800
#define XSIZE 800

typedef struct {
        char type;
        int flags;
        int color;
        monster_t *monster;
        obj_t     *inventory;
} cell_t;

typedef struct {
        cell_t out[YSIZE][XSIZE];
        int villages, cvillage;   // num of villages, current village|
        int cities, ccity;
        int forests, cforest;
        int dungeons;
} world_t;


void generate_world();

extern char mapchars[];

#endif
