/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _WORLD_H
#define _WORLD_H

#define AREA_NOTHING         0
#define AREA_PLAIN	     1
#define AREA_MOUNTAIN	     2
#define AREA_FOREST	     3
#define AREA_CITY	     4
#define AREA_VILLAGE         5
#define AREA_DUNGEON         6
#define AREA_CITY_NOHOUSE    7
#define AREA_VILLAGE_NOHOUSE 8
#define AREA_FOREST_NOTREE   9
#define AREA_LAKE           10
#define AREA_LAKE_NOWATER   11

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
        int x1, y1, x2, y2;     // start/end coordinates
        char name[50];
        int alignment;          // not sure what this is for.... good/evil alignment??
        int houses;
} city_t;

typedef struct {
        int x1, y1, x2, y2;
        char name[50];
        int flags;              // not sure yet what this is for...
} forest_t;

typedef struct {
        cell_t out[YSIZE][XSIZE];
        int villages, cvillage;   // num of villages, current village|
        int cities, ccity;
        int forests, cforest;
        int dungeons;
        city_t *city;
        city_t *village;
        forest_t *forest;
} world_t;


void generate_world();

extern char mapchars[];

#endif
