/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * This file deals with world generation.
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "quest.h"
#include "npc.h"
#include "world.h"
#include "datafiles.h"
#include "io.h"
#include "gt.h"
#include "utils.h"

#include "fractmod.h"

#define MAXFORESTSIZE 70
#define LARGECITYSIZE 50
#define VILLAGESIZE   20

char mapchars[50] = {
        ' ',  //nothing
        '.',  //plain
        '^',  //mountain
        'T',  //forest
        '#',  //city/house
        'o',  //village
        '>',  //dungeon
        '.',  //city    nohouse
        '.',  //village nohouse
        '.',  //foreest nohouse
        '~',  //lake
        '.',  //lake nohouse
        '.',  //dungeonfloor
        '#'   //dungeonwall
};

/*********************************************
* Description - Initialize a level_t struct.
* This function will allocate memory for xsize*ysize*sizeof(cell_t)
* If memory already has been allocated, it will be free'd and reallocated!
* Author - RK
* Date - Jan 02 2012
* *******************************************/
void init_level(level_t *level)
{
        int i;
        
        if(level->c) {
                gtfree(level->c);
                for(i = 0; i<level->xsize; i++)
                        gtfree(level->c[i]);
        }

        level->c = (cell_t **) gtmalloc2d(level->ysize, level->xsize, sizeof(cell_t));
        level->monsters = gtmalloc(sizeof(monster_t));
}

void zero_level(level_t *l)
{
        int x, y;

        for(y = 0; y < l->ysize; y++) {
                for(x = 0; x < l->xsize; x++) {
                        addwall(l, y, x);
                }
        }
}

void clear_area(level_t *l, int y1, int x1, int y2, int x2)
{
        int i, j;

        for(i = 0; i < (y2-y1); i++) {
                for(j = 0; j < (x2-x1); j++) {
                        l->c[y1+i][x1+j].type = DNG_WALL;
                        l->c[y1+i][x1+j].flags = 0;
                        if(l->c[y1+i][x1+j].monster)
                                kill_monster(l, l->c[y1+i][x1+j].monster, 0);
                }
        }
}

/*
 * Check if everything the player can reach has been reached.
 * It goes like this:
 * Do a floodfill from player x,y (only necessary to do when entering level, right?)
 * Then count if the number of filled cells equal the number of visited cells.
 * If the player has seen it all, then those two numbers should be equal.
 */
/*bool check_if_all_explored(level_t *l)
{
        int x, y;
        bool result;
        int floors, visited;

        floors = visited = 0;
        for(x = 0; x < l->xsize; x++) {
                for(y = 0; y < l->ysize; y++) {
                        if(hasbit(cf(y, x), CF_FLOODFILLED)) {
                                floors++;
                                if(hasbit(cf(y, x), CF_VISITED))
                                        visited++;
                        }
                }
        }

        if(floors == visited)
                result = true;
        else
                result = false;

        return result;
}*/

bool check_if_all_explored(level_t *l)
{
        //gtprintf("walkable = %d     visited = %d", l->walkable, l->visited);
        if(l->visited == l->walkable)
                return true;
        else
                return false;
}

/*
 * this function cleans the dungeon for stuff made by the dungeon generator which shouldn't be there.
 */
void cleanup_dungeon(level_t *l)
{
        int y, x;

        for(y = 0; y < l->ysize; y++) {
                for(x = 0; x < l->xsize; x++) {
                        if(hasbit(l->c[y][x].flags, CF_HAS_DOOR_CLOSED) && l->c[y][x].type == DNG_WALL)
                                l->c[y][x].type = DNG_FLOOR;

                        if(hasbit(l->c[y][x].flags, CF_HAS_DOOR_CLOSED)) {
                                if(l->c[y+1][x].type == DNG_WALL && l->c[y-1][x].type == DNG_WALL)
                                        break;
                                if(l->c[y][x+1].type == DNG_WALL && l->c[y][x-1].type == DNG_WALL)
                                        break;

                                clearbit(l->c[y][x].flags, CF_HAS_DOOR_CLOSED);
                        }
                }
        }
}

void insert_roomdef_at(level_t *l, int y, int x)
{
        int i, j;

        for(i = 0; i < r.y; i++) {
                for(j = 0; j < r.x; j++) {
                        switch(r.c[i][j].type) {
                                case DNG_WALL:
                                        addwall(l, y+i, x+j);
                                        break;
                                case DNG_FLOOR: 
                                        addfloor(l, y+i, x+j); 
                                        if(hasbit(r.c[i][j].flags, CF_HAS_DOOR_CLOSED))
                                                adddoor(l, y+i, x+j, false);
                                        break;
                                default:
                                        break;
                        }
                }
        }
}

void generate_dungeon_type_1(int d)
{
        struct room **r;        
        int maxroomsizex, maxroomsizey, nrx, nry, i, j;
        int x1, y1, sy, sx, x2, y2, ty, tx;
        int starty, startx, endy, endx;
        int minroomsizex, minroomsizey;
        level_t *l;
                
        l = &world->dng[d];

        minroomsizey = 3;
        minroomsizex = 4;
        maxroomsizey = 9;
        maxroomsizex = 16;

        nry = l->ysize / maxroomsizey;
        nrx = l->xsize / maxroomsizex;
        //numrooms = nrx * nry;

        r = (struct room **) gtmalloc2d(nry+1, nrx+1, sizeof(struct room));

        //printf("Generating %d x %d = %d rooms (levelsize = %d x %d)\n", nry, nrx, numrooms, l->ysize, l->xsize);
        
        for(i = 1; i <= nrx; i++) {
                for(j = 1; j <= nry; j++) {
                        do {
                                y1 = ((j-1) * maxroomsizey) + ri(0,5);
                                x1 = ((i-1) * maxroomsizex) + ri(0,5);
                                sy = ri(minroomsizey, maxroomsizey);
                                sx = ri(minroomsizex, maxroomsizex);
                                y2 = y1 + sy;
                                x2 = x1 + sx;
                                if(y2 >= l->ysize)
                                        maxroomsizey--;
                                if(x2 >= l->xsize)
                                        maxroomsizex--;
                        } while(y2 >= l->ysize || x2 >= l->xsize);

                        //printf("painting room [%d][%d] from %d,%d to %d,%d\n", j, i, y1,x1,y2,x2);
                        paint_room(l, y1, x1, sy, sx, 0);
                        r[j][i].y1 = y1;
                        r[j][i].x1 = x1;
                        r[j][i].sx = sx;
                        r[j][i].sy = sy;
                        r[j][i].y2 = y2;
                        r[j][i].x2 = x2;
                }
         }

        for(i = 1; i < nrx; i++) {
                for(j = 1; j < nry; j++) {

                        starty = r[j][i].y1 + ri(2, r[j][i].sy-1);
                        endx   = r[j][i+1].x1 + ri(2, r[j][i+1].sx-1);

                        //printf("1corridor from room %d,%d to %d,%d\n", j,i,j,i+1);
                        paint_corridor_horizontal(l, starty, r[j][i].x2, endx);
                        if(perc(30))
                                adddoor(l, starty, r[j][i].x2, false);

                        if(starty < r[j][i+1].y1) {
                                //printf("2corridor from room %d,%d to %d,%d\n", j,i,j,i+1);
                                paint_corridor_vertical(l, starty, r[j][i+1].y1, endx);
                                if(perc(30))
                                                adddoor(l, r[j][i+1].y1, endx, false);

                        }

                        if(starty > r[j][i+1].y2) {
                                //printf("3corridor from room %d,%d to %d,%d\n", j,i,j,i+1);
                                paint_corridor_vertical(l, starty, r[j][i+1].y2, endx);
                                if(perc(30))
                                                adddoor(l, r[j][i+1].y2, endx, false);
                        }

                        startx = r[j][i].x1 + ri(2, r[j][i].sx-1);
                        endy   = r[j+1][i].y1 + ri(2, r[j+1][i].sy-1);

                        //printf("4corridor from room %d,%d to %d,%d\n", j,i,j+1,i);
                        paint_corridor_vertical(l, r[j][i].y2, endy, startx);
                        if(perc(30))
                                adddoor(l, r[j][i].y2, startx, false);

                        if(startx < r[j+1][i].x1) {
                                //printf("5corridor from room %d,%d to %d,%d\n", j,i,j+1,i);
                                paint_corridor_horizontal(l, endy, startx, r[j+1][i].x1);
                        }
                        if(startx > r[j+1][i].x2) {
                                //printf("6corridor from room %d,%d to %d,%d\n", j,i,j+1,i);
                                paint_corridor_horizontal(l, endy, startx, r[j+1][i].x2);
                        }
                }

        }

        for(i = nrx; i > 2; i--) {
                starty = r[nry][i].y1 + ri(2, r[nry][i].sy-1);
                endx   = r[nry][i-1].x1 - ri(2, r[nry][i-1].sx-1);

                paint_corridor_horizontal(l, starty, r[nry][i].x1, endx);
        }

        for(j = nry; j > 2; j--) {
                startx = r[j][nrx].x1 + ri(2, r[j][nrx].sx-1);
                endy   = r[j-1][nrx].y2 - ri(2, r[j-1][nrx].sy-1);

                paint_corridor_vertical(l, r[j][nrx].y1, endy, startx);

                if(startx < r[j-1][nrx].x1) {
                        paint_corridor_horizontal(l, endy, startx, r[j-1][nrx].x2);
                }
                if(startx > r[j-1][nrx].x2) {
                        paint_corridor_horizontal(l, endy, startx, r[j-1][nrx].x1);
                }

        }

        // the edges
        for(ty=0;ty<l->ysize;ty++) {
                addwall(&world->dng[d], ty, 1);
                addwall(&world->dng[d], ty, l->xsize-1);
                addwall(&world->dng[d], ty, l->xsize-2);
                addwall(&world->dng[d], ty, l->xsize-3);
                addwall(&world->dng[d], ty, l->xsize-4);
                addwall(&world->dng[d], ty, l->xsize-5);
        }

        for(tx=0;tx<l->xsize-4;tx++) {
                addwall(&world->dng[d], 1, tx);
                addwall(&world->dng[d], l->ysize-1, tx);
                addwall(&world->dng[d], l->ysize-2, tx);
                addwall(&world->dng[d], l->ysize-3, tx);
                addwall(&world->dng[d], l->ysize-4, tx);
                addwall(&world->dng[d], l->ysize-5, tx);
        }

        // And finally, do some cleaning:
        cleanup_dungeon(&world->dng[d]);
}

/*********************************************
* Description - Generate a dungeon, labyrinthine (or perhaps more like a cavern?)
* maxsize = well, max size
* Author - RK
* Date - Dec 12 2011
* *******************************************/
void generate_dungeon_type_2(int d)
{
        int tx, ty, xsize, ysize; 
        int fx, fy;
        int a, chance = 0;
        int csx, cex, csy, cey;
        float outerlinesx, outerlinesy;
        int edgex, edgey;
        level_t *l;
        //int color;

        //printf("Generating dungeon type 2...\n");
        l = &world->dng[d];
        tx = 0; //ri(0, 10);  // starting X
        ty = 0; //ri(0, 10);  // starting y
        xsize = world->dng[d].xsize;  // total size X
        ysize = world->dng[d].ysize;  // total size Y - rather uneccessary these two, eh?

        //fprintf(stderr, "DEBUG: %s:%d - tx,ty = %d,%d xsize,ysize = %d,%d\n", __FILE__, __LINE__, tx, ty, xsize, ysize);
        // let's not go over the edge
        if(tx+xsize >= XSIZE)
                xsize = XSIZE-tx;
        if(ty+ysize >= YSIZE)
                ysize = YSIZE-ty;

        // now let's find center and edges 
        csx = tx + (xsize/2) - (xsize/4);
        cex = tx + (xsize/2) + (xsize/4);
        csy = ty + (ysize/2) - (ysize/4);
        cey = ty + (ysize/2) + (ysize/4);
        outerlinesx = ((float) xsize / 100) * 10;   // outer 10%
        outerlinesy = ((float) ysize / 100) * 10;
        edgex = (int) outerlinesx;
        edgey = (int) outerlinesy;
        if(edgex <= 0)
                edgex = 1;
        if(edgey <= 0)
                edgey = 1;

        for(fy = ty; fy < ysize; fy++) {
                for(fx = tx; fx < xsize; fx++) {
                        addwall(&world->dng[d], fy, fx);
                        world->dng[d].c[fy][fx].visible = 0;
                }
        }

        for(fy=ty;fy<ty+ysize;fy++) {
                for(fx=tx;fx<tx+xsize;fx++) {
                        a = ri(1,100);
                        chance = 60;
                        // ensure less chance of trees at the edge of the forest and greater chance around the center
                        //if(((fx == (tx-(xsize/2))) || (fx == (tx-(xsize/2)+1))) && ((fy == (ty-(ysize/2))) || (fy == (ty-(ysize/2)+1))))
                        
                        if(fx <= tx+edgex)
                                chance = 90;
                        if(fy <= ty+edgey)
                                chance = 90;
                        if(fy >= ty+ysize-edgey)
                                chance = 90;
                        if(fx >= tx+xsize-edgex)
                                chance = 90;
                        if(fx >= csx && fx <= cex && fy >= csy && fy <= cey) {
                                chance = 20;
                        }

                        // testing testing chance percentage
                        chance = 25;

                        if(a >= chance && fy != ty && fy != (ty+ysize) && fx != tx && fx != (tx+xsize)) {
                                world->dng[d].c[fy][fx].type = DNG_FLOOR;
                                world->dng[d].c[fy][fx].color = COLOR_NORMAL;
                        }
                }
        }

        for(ty=0;ty<l->ysize;ty++) {
                addwall(&world->dng[d], ty, 1);
                addwall(&world->dng[d], ty, l->xsize-1);
                addwall(&world->dng[d], ty, l->xsize-2);
                addwall(&world->dng[d], ty, l->xsize-3);
                addwall(&world->dng[d], ty, l->xsize-4);
                addwall(&world->dng[d], ty, l->xsize-5);
        }

        for(tx=0;tx<l->xsize;tx++) {
                addwall(&world->dng[d], 1, tx);
                addwall(&world->dng[d], l->ysize-1, tx);
                addwall(&world->dng[d], l->ysize-2, tx);
                addwall(&world->dng[d], l->ysize-3, tx);
                addwall(&world->dng[d], l->ysize-4, tx);
                addwall(&world->dng[d], l->ysize-5, tx);
        }
}

void generate_dungeon_type_3(int d)
{
        int i, j, x, y, q, r, num;
        level_t *l;

        //printf("Generating dungeon type 3...\n");
        l = &world->dng[d];
        q = ri(80, l->ysize);
        r = ri(100, l->xsize);

        for(i = 2; i < q; i++) {
                x = l->xsize / 2;
                y = l->ysize / 2;
                for(j = 2; j < r; j++) {
                        num = dice(1, 4, 0);
                        switch(num) {
                                case 1: x++; break;
                                case 2: x--; break;
                                case 3: y++; break;
                                case 4: y--; break;
                        }

                        addfloor(l, y, x);
                }
        }
}

/*
 * Check if a rectangular area is OK for generating
 * a forest, city or village.
 * It is OK if it has no mountain or lake.
 * Only works on outside world so far.
 */
bool area_is_ok(int y1, int x1, int y2, int x2)
{
        int i, j;

        if(y1 == 0 || y2 == 0 || x1 == 0 || x2 == 0)
                return false;

        for(i = y1; i < y2; i++) {
                for(j = x1; j < x2; j++) {
                        if(world->out->c[i][j].type == AREA_LAKE)
                                return false;
                        if(world->out->c[i][j].type == AREA_MOUNTAIN)
                                return false;
                }
        }

        return true;
}

/*********************************************
* Description - Generate an area
* i = index into array
* type = Type of area (see AREA_defines)
* modifier = modifier for maxsize.
* maxsize = well, max size
* Author - RK
* Date - Dec 12 2011
* *******************************************/
void generate_area(int i, int type, int modifier, int maxsize)
{
        int tx, ty, xsize, ysize; 
        int fx, fy;
        int a, chance = 0;
        int csx, cex, csy, cey;
        float outerlinesx, outerlinesy;
        int edgex, edgey;
        //int color;

        tx = ty = xsize = ysize = 0;

        while(!area_is_ok(ty, tx, ty+ysize, tx+xsize)) {
                tx = ri(1, world->out->xsize - maxsize - 5);  // starting X
                ty = ri(1, world->out->ysize - maxsize - 5);  // starting y
                xsize = ri(modifier, maxsize-1+modifier);  // total size X
                ysize = ri(modifier, maxsize-1+modifier);  // total size Y
        }

        // let's not go over the edge
        if(tx+xsize >= XSIZE)
                xsize = XSIZE-tx;
        if(ty+ysize >= YSIZE)
                ysize = YSIZE-ty;

        // now let's find center and edges 
        csx = tx + (xsize/2) - (xsize/4);
        cex = tx + (xsize/2) + (xsize/4);
        csy = ty + (ysize/2) - (ysize/4);
        cey = ty + (ysize/2) + (ysize/4);
        outerlinesx = ((float) xsize / 100) * 10;   // outer 10%
        outerlinesy = ((float) ysize / 100) * 10;
        edgex = (int) outerlinesx;
        edgey = (int) outerlinesy;
        if(edgex <= 0)
                edgex = 1;
        if(edgey <= 0)
                edgey = 1;

        for(fy=ty;fy<(ty+ysize);fy++) {
                for(fx=tx;fx<(tx+xsize);fx++) {
                        switch(type) {
                                // TODO: add namegens here
                                case AREA_FOREST:
                                        world->forest[i].x1 = tx;
                                        world->forest[i].y1 = ty;
                                        world->forest[i].x2 = tx+xsize-1;
                                        world->forest[i].y2 = ty+ysize-1;
                                        world->forest[i].flags = 0;;
                                        world->out->c[fy][fx].type = AREA_FOREST_NOTREE;
                                        world->out->c[fy][fx].color = COLOR_NORMAL;
                                        break;
                                case AREA_VILLAGE:
                                        world->village[i].x1 = tx;
                                        world->village[i].y1 = ty;
                                        world->village[i].x2 = tx+xsize-1;
                                        world->village[i].y2 = ty+ysize-1;
                                        if(world->out->c[fy][fx].type == AREA_PLAIN) {
                                                world->out->c[fy][fx].type = AREA_VILLAGE_NOHOUSE;
                                                world->out->c[fy][fx].color = COLOR_NORMAL;
                                        }
                                        break;
                                case AREA_CITY:
                                        world->city[i].x1 = tx;
                                        world->city[i].y1 = ty;
                                        world->city[i].x2 = tx+xsize-1;
                                        world->city[i].y2 = ty+ysize-1;
                                        if(world->out->c[fy][fx].type == AREA_PLAIN) {
                                                world->out->c[fy][fx].type = AREA_CITY_NOHOUSE;
                                                world->out->c[fy][fx].color = COLOR_NORMAL;
                                        }
                                        break;
                        }                        
                }
        }

        for(fy=ty;fy<ty+ysize;fy++) {
                for(fx=tx;fx<tx+xsize;fx++) {

                        a = ri(1,100);
                        chance = 60;

                        // ensure less chance of trees at the edge of the forest and greater chance around the center
                        //if(((fx == (tx-(xsize/2))) || (fx == (tx-(xsize/2)+1))) && ((fy == (ty-(ysize/2))) || (fy == (ty-(ysize/2)+1))))
                        
                        if(fx <= tx+edgex)
                                chance = 90;
                        if(fy <= ty+edgey)
                                chance = 90;
                        if(fy >= ty+ysize-edgey)
                                chance = 90;
                        if(fx >= tx+xsize-edgex)
                                chance = 90;
                        if(fx >= csx && fx <= cex && fy >= csy && fy <= cey) {
                                // REMOVE AREA_ for interesting effect! possible dungeon gen!
                                // eller -- switch plain/forest ...?
                                if(type == AREA_CITY || type == AREA_VILLAGE) {
                                        chance = 0;
                                } else if(type == AREA_FOREST) {
                                        chance = 20;
                                }
                        }

                        if(a >= chance) {
                                world->out->c[fy][fx].type = type;
                                switch(type) {
                                        case AREA_FOREST:
                                                world->out->c[fy][fx].color = COLOR_FOREST;
                                                break;
                                        case AREA_VILLAGE:
                                        case AREA_CITY:
                                                world->out->c[fy][fx].color = COLOR_CITY;
                                                break;
                                }
                                /*
                                switch(type) {
                                        case FOREST:
                                                color = ri(0,4);
                                                world->out->c[fy][fx].color = forestcolors[color];
                                                break;
                                        case VILLAGE:
                                                color = ri(0,1); 
                                                world->out->c[fy][fx].color = citycolors[color];
                                                world->village[i].houses++;
                                                break;
                                        case CITY:
                                                color = ri(0,1);
                                                world->out->c[fy][fx].color = citycolors[color];
                                                world->city[i].houses++;
                                                break;
                                        case DUNGEON:
                                                color = ri(0,2);
                                                world->out->c[fy][fx].color = dungeoncolors[color];
                                                world->dungeons++;
                                                world->out->c[fy][fx].flags |= HAS_DUNGEON;
                                                break;
                                }*/
                        }
                }
        }
}

/*********************************************
* Description - Wrapper function for generating %num forests
* Author - RK
* Date - Dec 12 2011
* *******************************************/
void generate_forest(int num)
{
        int i;

        for(i = 0; i < num; i++)
                generate_area(i, AREA_FOREST, 4, MAXFORESTSIZE);
}

/*
 * TODO: Make this work as forest generator!
 */
void generate_forest2(int number)
{
        int i, j, x, y, q, r, num;
        level_t *l;

        l = world->out;
        q = ri(2, MAXFORESTSIZE);
        r = ri(2, MAXFORESTSIZE);

        for(i = 2; i < q; i++) {
                x = l->xsize / 2;
                y = l->ysize / 2;
                for(j = 2; j < r; j++) {
                        num = dice(1, 4, 0);
                        switch(num) {
                                case 1: x++; break;
                                case 2: x--; break;
                                case 3: y++; break;
                                case 4: y--; break;
                        }

                        addfloor(l, y, x);
                }
        }
}

/*********************************************
* Description - Wrapper function for generating %num cities
* Author - RK
* Date - Dec 12 2011
* *******************************************/
void generate_city(int num)
{
        int i;

        for(i = 0; i < num; i++)
                generate_area(i, AREA_CITY, 2, LARGECITYSIZE);
}

/*********************************************
* Description - Wrapper function for generating %num villages
* Author - RK
* Date - Dec 12 2011
* *******************************************/
void generate_village(int num)
{
        int i;

        for(i = 0; i < num; i++)
                generate_area(i, AREA_VILLAGE, 1, VILLAGESIZE);
}

void pathfinder(level_t *l, int y1, int x1, int y2, int x2)
{
        float x, y, xinc, yinc, dx, dy;
        int k, step;

        dx = x2 - x1;
        dy = y2 - y1;
        if(abs(dx) > abs(dy))
                step = abs(dx);
        else
                step = abs(dy);

        xinc = dx / step;
        yinc = dy / step;

        x = (float) x1;
        y = (float) y1;

        l->c[(int)y][(int)x].color = COLOR_LAKE;
        for(k = 1; k <= step; k++) {
                x += xinc;
                y += yinc;
                l->c[(int)y][(int)x].color = COLOR_LAKE;
        }
}

/*********************************************
* Description - Flood fill to test dungeon gen
* Author - RK
* Date - Dec 14 2011
* *******************************************/
void floodfill(level_t *l, int y, int x)
{
        if(l->c[y][x].type == DNG_FLOOR && !hasbit(l->c[y][x].flags, CF_FLOODFILLED)) {
                setbit(l->c[y][x].flags, CF_FLOODFILLED);
                l->walkable++;
                //l->c[y][x].color = COLOR_LAKE;
                floodfill(l, y-1, x);
                floodfill(l, y+1, x);
                floodfill(l, y,   x-1);
                floodfill(l, y,   x+1);
                floodfill(l, y+1, x+1);
                floodfill(l, y+1, x-1);
                floodfill(l, y-1, x+1);
                floodfill(l, y-1, x-1);
        }
}

void set_cell_visited(level_t *l, int y, int x)
{
        if(!hasbit(l->c[y][x].flags, CF_VISITED)) {
                setbit(l->c[y][x].flags, CF_VISITED);
                if(l->c[y][x].type == DNG_FLOOR)
                        l->visited++;
        }
}

void set_level_visited(level_t *l)
{
        int x,y;

        for(y=0;y<l->ysize;y++) {
                for(x=0;x<l->xsize;x++) {
                        set_cell_visited(l, y, x);
                }
        }
}

void addfloor(level_t *l, float y, float x)
{
        if((int)y >= l->ysize || (int)x >= l->xsize || (int)y < 0 || (int)x < 0)
                return;

        if(l->c[(int)y][(int)x].type == DNG_WALL) {
                l->c[(int)y][(int)x].type = DNG_FLOOR;
                l->c[(int)y][(int)x].color = COLOR_SHADE;
                l->c[(int)y][(int)x].litcolor = COLOR_SHADE;
        }
}

void addwall(level_t *l, int y, int x)
{
        l->c[y][x].type  = DNG_WALL;
        l->c[y][x].color = COLOR_SHADE;
        l->c[y][x].litcolor = COLOR_YELLOW;
}

void adddoor(level_t *l, int y, int x, bool secret)
{
        if(secret)
                l->c[y][x].flags |= CF_HAS_DOOR_SECRET;
        else
                l->c[y][x].flags |= CF_HAS_DOOR_CLOSED;
}

/*********************************************
* Description - Paint a room in a dungeon
* Author - RK
* Date - Dec 14 2011
* *******************************************/
void paint_room(level_t *l, int y, int x, int sy, int sx, int join_overlapping)
{
        int i, j;

        if(((x+sx) >= l->xsize) || ((y+sy) >= l->ysize))
                return;

        for(i = x; i <= (x+sx); i++) {
                for(j = y; j <= (y+sy); j++) {
                        if((j == y) || (j == y+sy) || (i == x) || (i == x+sx)) {
                                if(join_overlapping) {
                                        if(l->c[j][i].type != DNG_FLOOR) {
                                                addwall(l, j, i);
                                        }
                                } else {
                                        addwall(l, j, i);
                                }
                        } else {
                                addfloor(l, j, i);
                        }
                }
        }
}

#define paint_corridor_left(a, b, c, d) paint_corridor_horizontal(a, b, c, d)
#define paint_corridor_right(a, b, c, d) paint_corridor_horizontal(a, b, c, d)
#define paint_corridor_up(a, b, c, d) paint_corridor_vertical(a, b, c, d)
#define paint_corridor_down(a, b, c, d) paint_corridor_vertical(a, b, c, d)

void paint_corridor_horizontal(level_t *l, int y, int x1, int x2)
{
        int i;

        //printf("horizontal corridor from %d,%d to %d,%d\n", y, x1, y, x2);
        if(x1 < x2) {
                for(i = x1; i < x2; i++)
                        addfloor(l, y, i);
        }

        if(x1 > x2) {
                for(i = x1; i >= x2; i--)
                        addfloor(l, y, i);
        }
}

void paint_corridor_vertical(level_t *l, int y1, int y2, int x)
{
        int i;

        //printf("vertical corridor from %d,%d to %d,%d\n", y1, x, y2, x);
        if(y1 < y2) {
                for(i = y1; i < y2; i++)
                        addfloor(l, i, x);
        }

        if(y1 > y2) {
                for(i = y1; i >= y2; i--)
                        addfloor(l, i, x);
        }
}

void paint_corridor(level_t *l, int y1, int x1, int y2, int x2)
{
        float x, y, xinc, yinc, dx, dy;
        int k, step;

        dx = x2 - x1;
        dy = y2 - y1;
        if(abs(dx) > abs(dy))
                step = abs(dx);
        else
                step = abs(dy);

        xinc = dx / step;
        yinc = dy / step;

        x = (float) x1;
        y = (float) y1;

        addfloor(l, y, x);
        for(k = 1; k <= step; k++) {
                x += xinc;
                y += yinc;
                addfloor(l, y, x);
        }
}

bool passable(level_t *l, int y, int x)
{
        int type;

        if(y < 0)
                return false;
        if(x < 0)
                return false;

        if(x >= l->xsize)
                return false;
        if(y >= l->ysize)
                return false;

        if(game->wizardmode)   // if we are in wizard mode, anything goes!
                return true;

        type = l->c[y][x].type;

        /*if(type == DNG_WALL && l->c[y][x].color == COLOR_LAKE)
                return true;*/
        if(type == DNG_WALL)
                return false;
        if(type == AREA_LAKE)
                return false;
        if(type == AREA_MOUNTAIN)
                return false;
        if(type == AREA_WALL)
                return false;
        if(type == AREA_NOTHING)
                return false;

        if(l->c[y][x].npc && !hasbit(l->c[y][x].npc->flags, MF_ISDEAD))         // change here if e.g bump to chat???
                return false;

        return true;
}

bool monster_passable(level_t *l, int y, int x)
{ 
        int type;

        if(l->c[y][x].monster)
                return false;
        
        if(y < 0)
                return false;
        if(x < 0)
                return false;

        if(x >= l->xsize)
                return false;
        if(y >= l->ysize)
                return false;

        if(l->c[y][x].type == AREA_VILLAGE || l->c[y][x].type == AREA_CITY)
                return false;
        if(l->c[y][x].monster)
                return false;
        if(l->c[y][x].npc)
                return false;
        if(l->c[y][x].flags & CF_HAS_DOOR_CLOSED)
                return false;
        
        type = l->c[y][x].type;

        if(type == DNG_WALL)
                return false;
        if(type == AREA_LAKE)
                return false;
        if(type == AREA_WALL)
                return false;
        if(type == AREA_NOTHING)
                return false;
        if(type == AREA_MOUNTAIN)
                return false;
        if(type == AREA_LAKE)
                return false;

        return true;
}

void generate_stairs_outside()
{
        /* Here's what we're gonna do:
         * - Choose 3 destinations in dng[1]
         * - Make about 30 downstairs on outside level, each leading to one of 3 destinations
         * - Make the 3 upstairs in dng[1]
         * - Then, use another function for generating stairs in each dungeon.
         */

        struct dest {
                int y, x;
        } d[3];
        int i, s, j;
        int sy, sx;

        
        for(i = 0; i < 3; i++) {
                d[i].y = ri(0, world->dng[1].ysize-5);
                d[i].x = ri(0, world->dng[1].xsize-5);
                while(!passable(&world->dng[1], d[i].y, d[i].x)) {
                        d[i].y = ri(0, world->dng[1].ysize-5);
                        d[i].x = ri(0, world->dng[1].xsize-5);
                }

                setbit(world->dng[1].c[d[i].y][d[i].x].flags, CF_HAS_STAIRS_UP);
        }

        s = ri(35, 70);
        for(i = 0; i < s; i++) {
                sy = ri(5, world->out->ysize-5);
                sx = ri(5, world->out->xsize-5);
                while(!passable(world->out, sy, sx)) {
                        sy = ri(5, world->out->ysize-5);
                        sx = ri(5, world->out->xsize-5);
                }
                j  = ri(0, 2);

                setbit(world->dng[0].c[sy][sx].flags, CF_HAS_STAIRS_DOWN);
                world->dng[0].c[sy][sx].desty = d[j].y;
                world->dng[0].c[sy][sx].destx = d[j].x;
                world->dng[1].c[d[j].y][d[j].x].desty = sy;
                world->dng[1].c[d[j].y][d[j].x].destx = sx;
        }

}

void create_stairs(int num, int s, int d)
{
        level_t *a, *b;
        int x1, y1, x2, y2;
        int i;

        a = &world->dng[s];
        b = &world->dng[d];

        for(i = 0; i < num; i++) {
                y1 = ri(5, a->ysize-5);
                x1 = ri(5, a->xsize-5);
                while(!passable(a, y1, x1)) {
                        y1 = ri(5, a->ysize-5);
                        x1 = ri(5, a->xsize-5);
                }

                y2 = ri(5, b->ysize-5);
                x2 = ri(5, b->xsize-5);
                while(!passable(b, y2, x2)) {
                        y2 = ri(5, b->ysize-5);
                        x2 = ri(5, b->xsize-5);
                }

                setbit(a->c[y1][x1].flags, CF_HAS_STAIRS_DOWN);
                a->c[y1][x1].desty = y2;
                a->c[y1][x1].destx = x2;

                setbit(b->c[y2][x2].flags, CF_HAS_STAIRS_UP);
                b->c[y2][x2].desty = y1;
                b->c[y2][x2].destx = x1;
        }
}

void meta_generate_dungeon(int d, int type)
{
        if(type && type <= 3) {
                int num_monsters, mino, maxo;

                if(type == 3) {
                        world->dng[d].ysize = ri(400, 700);
                        world->dng[d].xsize = ri(450, 750);
                } else {
                        world->dng[d].xsize = (ri(55, 100));  // let's start within reasonable sizes!
                        world->dng[d].ysize = (ri(40, 80));
                }

                world->dng[d].level = d;
                world->dng[d].type  = type;
                
                init_level(&world->dng[d]);
                zero_level(&world->dng[d]);

                if(type == 1)
                        generate_dungeon_type_1(d);   // classic
                if(type == 2)
                        generate_dungeon_type_2(d);   // "labyrinthine"
                if(type == 3)
                        generate_dungeon_type_3(d);   // "cave"

                if(type == 3) {
                        num_monsters = (((world->dng[d].xsize + world->dng[d].ysize) / 100) * d) / 2;
                        mino = (world->dng[d].ysize + world->dng[d].xsize) / 80;
                        maxo = (world->dng[d].ysize + world->dng[d].xsize) / 60;
                } else {
                        num_monsters = (world->dng[d].xsize + world->dng[d].ysize) / 20;
                        mino = (world->dng[d].ysize + world->dng[d].xsize) / 20;
                        maxo = (world->dng[d].ysize + world->dng[d].xsize) / 10;
                }

                fov_initmap(&world->dng[d]);

                spawn_monsters(num_monsters, d+2, &world->dng[d]);
                spawn_gold_with_maxtotal(75 + ri(0, d*15), &world->dng[d]);
                spawn_objects(ri(mino, maxo), &world->dng[d]);

                game->createddungeons++;
        } else {
                die("trying to generate dungeon of unknown type!");
        }
}

void generate_stairs()
{
        int i;

        generate_stairs_outside();

        for(i = 1; i < game->createddungeons; i++) {
                create_stairs(3, i, i+1);
        }
}

void generate_heightmap()
{
        float *f;
        int y, x;

        f = alloc2DFractArray(XSIZE);
        fill2DFractArray(f, XSIZE, game->seed, 1.0, 0.9);
        for(y = 0; y < YSIZE; y++) {
                for(x = 0; x < XSIZE; x++) {
                        float h;
                        h = f[y*YSIZE + x] * 100.0f;
                        world->out->c[y][x].height = (int) h;
                        world->out->zero += world->out->c[y][x].height;
                        //fprintf(stderr, "DEBUG: %s:%d - cellheight set to %d\n", __FILE__, __LINE__, world->out->c[y][x].height);
                }
        }

        world->out->zero /= (YSIZE*XSIZE);
        
fprintf(stderr, "DEBUG: %s:%d - zero is defined as %d !\n", __FILE__, __LINE__, world->out->zero);
}
        
void generate_terrain(int visible)
{
        int y, x;
        for(x = 0; x < world->out->xsize; x++) {
                for(y = 0; y < world->out->ysize; y++) {
                        if(world->out->c[y][x].height >= world->out->zero - world->out->lakelimit && world->out->c[y][x].height <= world->out->zero + world->out->lakelimit) {
                                world->out->c[y][x].type = AREA_PLAIN;
                                world->out->c[y][x].color = COLOR_PLAIN;
                                world->out->c[y][x].monster = NULL;
                                world->out->c[y][x].inventory = NULL;
                                world->out->c[y][x].visible = visible;
                        }
                        if(world->out->c[y][x].height < world->out->zero - world->out->lakelimit) {
                                world->out->c[y][x].type = AREA_LAKE;
                                world->out->c[y][x].color = COLOR_BLUE;
                                world->out->c[y][x].monster = NULL;
                                world->out->c[y][x].inventory = NULL;
                                world->out->c[y][x].visible = visible;
                        }
                        if(world->out->c[y][x].height > world->out->zero + world->out->lakelimit) {
                                world->out->c[y][x].type = AREA_MOUNTAIN;
                                world->out->c[y][x].color = COLOR_MOUNTAIN;
                                world->out->c[y][x].monster = NULL;
                                world->out->c[y][x].inventory = NULL;
                                world->out->c[y][x].visible = visible;
                        }
                }
        }
}

/*********************************************
* Description - One big function which should
* take care of all world generation stuff.
* Author - RK
* Date - Dec 12 2011
* *******************************************/
void generate_world()
{
        int x, y, i;

        /*
         * Generate the outside world first.
         */

        printf("Generating outside world...");
        generate_heightmap();
        world->out->lakelimit = 4;
        generate_terrain(0);

        world->forests  = ri(gtconfig.minf, gtconfig.maxf);
        world->cities   = ri(gtconfig.minc, gtconfig.maxc);
        world->villages = ri(gtconfig.minv, gtconfig.maxv);

        //fprintf(stderr, "DEBUG: %s:%d - Generating %d forests\n", __FILE__, __LINE__, world->forests);
        world->forest = (forest_t *) gtcalloc((size_t)world->forests, sizeof(forest_t));
        generate_forest(world->forests);

        //fprintf(stderr, "DEBUG: %s:%d - Generating %d villages\n", __FILE__, __LINE__, world->villages);
        world->village = gtcalloc((size_t)world->villages, sizeof(city_t));
        generate_village(world->villages);

        //fprintf(stderr, "DEBUG: %s:%d - Generating %d cities\n", __FILE__, __LINE__, world->cities);
        world->city = gtcalloc((size_t)world->cities, sizeof(city_t));
        generate_city(world->cities);

        fov_initmap(world->out);

        spawn_monsters(ri(75,125), 3, world->out); 
        spawn_golds(ri(75,125), 100, world->out);
        spawn_objects(ri(world->out->xsize/4, world->out->ysize/4), world->out);

        printf(" done.\n");

        printf("Generating dungeons.");
        meta_generate_dungeon(1, 1);
        printf(".");
        //clear_area(&world->dng[1], 6, 6, r.y+6, r.x+6);
        //insert_roomdef_at(&world->dng[1], 6, 6);

        for(i = 2; i <= 10; i++) {
                int p;

                p = ri(1,100);
                if(p <= 66)
                        meta_generate_dungeon(i, 1);
                if(p > 66 && p < 82)
                        meta_generate_dungeon(i, 2);
                if(p >= 82)
                        meta_generate_dungeon(i, 3);

                printf(".");
        }
        printf("done.\n");

        generate_stairs();

        spawn_predef_npcs();

        // create the edge of the world
        for(x=0; x<world->out->xsize; x++) {
                world->out->c[1][x].type   = AREA_WALL;
                world->out->c[2][x].type   = AREA_WALL;
                world->out->c[XSIZE-6][x].type = AREA_WALL;
                world->out->c[XSIZE-5][x].type = AREA_WALL;
        }
        for(y=0; y<world->out->ysize; y++) {
                world->out->c[y][1].type   = AREA_WALL;
                world->out->c[y][2].type   = AREA_WALL;
                world->out->c[y][YSIZE-6].type = AREA_WALL;
                world->out->c[y][YSIZE-5].type = AREA_WALL;
        }


fprintf(stderr, "DEBUG: %s:%d - Generated a total of %d objects.\n", __FILE__, __LINE__, game->num_objects);
fprintf(stderr, "DEBUG: %s:%d - Generated a total of %d monsters.\n", __FILE__, __LINE__, game->num_monsters);
}
