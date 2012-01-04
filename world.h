/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _WORLD_H
#define _WORLD_H

#include <stdbool.h>

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

#define DNG_FLOOR           12
#define DNG_WALL            13
#define DNG_FILL            DNG_WALL

#define AREA_WALL           100

#define YSIZE 800
#define XSIZE 800
#define DUNGEON_SIZE 200

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
        char type;
        int flags;
        int color;
        short visible;
        monster_t *monster;
        obj_t     *inventory;
} cell_t;

struct levelstruct {
        cell_t **c;
        int xsize, ysize;
        monster_t *monsters;      // point to head of linked lists of monsters on this level
};

typedef struct levelstruct level_t;
typedef cell_t** map_ptr;

typedef struct {
        level_t *out;               // shall point to dng[0]
        level_t *dng;
        level_t *curlevel;          // needed?
        int villages, cvillage;     // num of villages, current village
        int cities, ccity;
        int forests, cforest;
        int dungeons;
        city_t *city;
        city_t *village;
        forest_t *forest;
        cell_t **cmap;
} world_t;


void generate_world();
void floodfill(int x, int y);
bool passable(int y, int x);
void init_level(level_t *level);
void set_all_visible();

extern char mapchars[];

#endif
