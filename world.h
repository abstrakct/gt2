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
#define DNG_NOTHING         AREA_NOTHING

#define AREA_WALL           100

#define YSIZE 800
#define XSIZE 800
#define DUNGEON_SIZE 200

#define ct(a,b) world->cmap[a][b].type
#define cv(a,b) world->cmap[a][b].visible
#define cc(a,b) world->cmap[a][b].color
#define cm(a,b) world->cmap[a][b].monster
#define ci(a,b) world->cmap[a][b].inventory

typedef struct {
        char  name[50];
        short x1, y1, x2, y2;     // start/end coordinates
        short alignment;          // not sure what this is for.... good/evil alignment??
        short houses;
} city_t;

typedef struct {
        char  name[50];
        short x1, y1, x2, y2;
        short flags;              // not sure yet what this is for...
} forest_t;

typedef struct {                 // cell_t
        char       type;
        int        flags;
        short      desty, destx;       // for stairs and portals; destination y,x
        short      color;
        bool       visible;
        monster_t *monster;
        obj_t     *inventory;
} cell_t;

struct levelstruct {
        short      xsize, ysize;
        cell_t     **c;
        monster_t  *monsters;      // point to head of linked lists of monsters on this level
};

typedef struct levelstruct level_t;
typedef cell_t** map_ptr;

typedef struct {
        level_t  *out;               // shall point to dng[0]
        level_t  *dng;
        level_t  *curlevel;          // needed?
        city_t   *city;
        city_t   *village;
        forest_t *forest;
        cell_t   **cmap;
        short    villages, cvillage;     // num of villages, current village
        short    cities, ccity;
        short    forests, cforest;
        short    dungeons;
} world_t;

// CELLFLAGS

#define CF_HAS_STAIRS_DOWN (1<<0)
#define CF_HAS_STAIRS_UP   (1<<1)
#define CF_LIT             (1<<2)
#define CF_VISITED         (1<<3)

void generate_world();
void floodfill(level_t *l, int y, int x);
bool passable(level_t *l, int y, int x);
bool monster_passable(level_t *l, int y, int x);
void init_level(level_t *level);
void set_all_visible();
void set_floor(level_t *l, float y, float x);
void addwall(level_t *l, int y, int x);
void paint_room(level_t *l, int y, int x, int sy, int sx, int join_overlapping);
void paint_corridor(level_t *l, int y1, int x1, int y2, int x2);
void paint_corridor_vertical(level_t *l, int y1, int y2, int x);
void paint_corridor_horizontal(level_t *l, int y, int x1, int x2);

extern char mapchars[];

#endif
