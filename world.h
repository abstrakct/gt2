/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef WORLD_H
#define WORLD_H

#define PLAIN		1
#define MOUNTAIN	2
#define FOREST		3
#define CITY		4
#define PLAYER		7
#define INFO            8
#define NORMAL          9

#define YSIZE 800
#define XSIZE 800

typedef struct {
        char type;
        int flags;
        obj_t *inventory;
} cell_t;

typedef struct {
        cell_t cell[YSIZE][XSIZE];
} world_t;

#endif
