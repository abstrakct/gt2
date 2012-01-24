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
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "you.h"
#include "display.h"
#include "gt.h"

#define MAXFORESTSIZE 70
#define LARGECITYSIZE 50
#define VILLAGESIZE   20

char mapchars[50] = {
        ' ',  //nothing
        '.',  //plain
        '~',  //mountain
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
        
        if(!level->c) {
                level->c = gtmalloc(level->ysize * (sizeof(cell_t)));
        } else {
                gtfree(level->c);
                for(i = 0; i<level->xsize; i++)
                        gtfree(level->c[i]);
                level->c = gtmalloc(level->ysize * (sizeof(cell_t)));
        }

        for(i = 0; i<level->ysize; i++)
                level->c[i] = gtmalloc(level->xsize * (sizeof(cell_t)));

        level->monsters = gtmalloc(sizeof(monster_t));
}

/*********************************************
* Description - Generate a dungeon, labyrinthine (or perhaps more like a cavern?)
* maxsize = well, max size
* Author - RK
* Date - Dec 12 2011
* *******************************************/
void generate_dungeon_labyrinthine(int d)
{
        int tx, ty, xsize, ysize; 
        int fx, fy;
        int a, chance = 0;
        int csx, cex, csy, cey;
        float outerlinesx, outerlinesy;
        int edgex, edgey;
        level_t *l;
        //int color;

        l = &world->dng[d];
        tx = 0; //ri(0, 10);  // starting X
        ty = 0; //ri(0, 10);  // starting y
        xsize = world->dng[d].xsize - tx;  // total size X
        ysize = world->dng[d].ysize - ty;  // total size Y - rather uneccessary these two, eh?

fprintf(stderr, "DEBUG: %s:%d - tx,ty = %d,%d xsize,ysize = %d,%d\n", __FILE__, __LINE__, tx, ty, xsize, ysize);
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

        for(tx=0;tx<l->xsize-4;tx++) {
                addwall(&world->dng[d], 1, tx);
                addwall(&world->dng[d], l->ysize-1, tx);
                addwall(&world->dng[d], l->ysize-2, tx);
                addwall(&world->dng[d], l->ysize-3, tx);
                addwall(&world->dng[d], l->ysize-4, tx);
                addwall(&world->dng[d], l->ysize-5, tx);
        }
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

struct room {
        int y1, x1, y2, x2, sx, sy;
};

void generate_dungeon_normal(int d)
{
        struct room *r;        
        int numrooms, nrx, nry, i, j;
        int x1, y1, sy, sx, x2, y2, rindex;
        level_t *l;
                
        l = &world->dng[d];
        nrx = l->xsize / 60;
        nry = l->ysize / 60;
        numrooms = nrx * nry;
        r = gtcalloc(numrooms+1, sizeof(struct room));
        rindex = 0;

        printf("Generating %d x %d = %d rooms (levelsize = %d x %d)\n", nrx, nry, numrooms, l->xsize, l->ysize);
        
        for(i = 1; i <= nrx; i++) {
                for(j = 1; j <= nry; j++) {
                        y1 = ri(5, (l->ysize/(nry+1)*j)) + ri(2,10);
                        x1 = ri(5, (l->xsize/(nrx+1)*i)) + ri(2,10);
                        sy = ri(5,15);
                        sx = ri(15,25);
                        y2 = y1 + sy;
                        x2 = x1 + sx;
                        printf("painting room %d from %d,%d to %d,%d\n", rindex, y1,x1,y2,x2);
                        paint_room(l, y1, x1, sy, sx, 1);
                        r[rindex].y1 = y1;
                        r[rindex].x1 = x1;
                        r[rindex].y2 = y2;
                        r[rindex].x2 = x2;
                        rindex++;
                }
        }

        for(i = 0; i < numrooms; i++) {
                if(i == (numrooms - 1))
                        break;



                //paint_corridor(l, r[i].y1, r[i].x1, r[i+1].y2, r[i+1].x2);
        }
        


        gtfree(r);

        /*paint_room(&world->dng[1], 10, 10, 10, 10, 0);
        paint_room(&world->dng[1], 30, 30, 10, 10, 0);
        paint_corridor(&world->dng[1], 15, 20, 35, 30);*/
}

void generate_dungeon_normal2(int d)
{
        struct room **r;        
        int numrooms, maxroomsizex, maxroomsizey, nrx, nry, i, j;
        int x1, y1, sy, sx, x2, y2, ty, tx;
        int starty, startx, endy, endx;
        level_t *l;
                
        l = &world->dng[d];

        maxroomsizey = 15;
        maxroomsizex = 25;

        nrx = l->xsize / maxroomsizex;
        nry = l->ysize / maxroomsizey;
        numrooms = nrx * nry;


        r = gtmalloc(sizeof(struct room) * (nry+1));
        for(i=0;i<nry+1;i++)
                r[i] = gtmalloc(sizeof(struct room) * (nrx+1));

        printf("Generating %d x %d = %d rooms (levelsize = %d x %d)\n", nry, nrx, numrooms, l->ysize, l->xsize);
        
        for(i = 1; i <= nrx; i++) {
                for(j = 1; j <= nry; j++) {
                        maxroomsizey = 15;
                        maxroomsizex = 25;
                        do {
                                y1 = ((j-1) * maxroomsizey) + ri(0,5);
                                x1 = ((i-1) * maxroomsizex) + ri(0,5);
                                sy = ri(5,  maxroomsizey - 5);
                                sx = ri(15, maxroomsizex - 5);
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
                        if(perc(25))
                                adddoor(l, starty, r[j][i].x2, false);

                        if(starty < r[j][i+1].y1) {
                                //printf("2corridor from room %d,%d to %d,%d\n", j,i,j,i+1);
                                paint_corridor_vertical(l, starty, r[j][i+1].y1, endx);
                        }

                        if(starty > r[j][i+1].y2) {
                                //printf("3corridor from room %d,%d to %d,%d\n", j,i,j,i+1);
                                paint_corridor_vertical(l, starty, r[j][i+1].y2, endx);
                        }

                        startx = r[j][i].x1 + ri(2, r[j][i].sx-1);
                        endy   = r[j+1][i].y1 + ri(2, r[j+1][i].sy-1);

                        //printf("4corridor from room %d,%d to %d,%d\n", j,i,j+1,i);
                        paint_corridor_vertical(l, r[j][i].y2, endy, startx);
                        if(perc(25))
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

        //printf("PASS2 connecting last row\n");

        for(i = nrx; i > 2; i--) {
                starty = r[nry][i].y1 + ri(2, r[nry][i].sy-1);
                endx   = r[nry][i-1].x1 - ri(2, r[nry][i-1].sx-1);

                //printf("1corridor from room %d,%d to %d,%d\n", nry,i,nry,i-1);
                paint_corridor_horizontal(l, starty, r[nry][i].x1, endx);


                /*
                if(starty < r[nry][i-1].y1) {
                        //printf("2corridor from room %d,%d to %d,%d\n", nry,i,nry,i-1);
                        paint_corridor_vertical(l, starty, r[nry][i-1].y2, endx);       // not sure if I intended to comment out these, but it seems to work fine!?!
                }

                if(starty > r[nry][i-1].y2) {
                        //printf("3corridor from room %d,%d to %d,%d\n", nry,i,nry,i-1);
                        paint_corridor_vertical(l, starty, r[nry][i-1].y1, endx);
                }*/

                /*startx = r[nry][i].x1 + ri(2, r[nry][i].sx-1);
                endy   = r[nry-1][i].y2 - ri(2, r[nry-1][i].sy-1);

                printf("4corridor from room %d,%d to %d,%d\n", nry,i,nry-1,i);
                paint_corridor_vertical(l, r[nry][i].y1, endy, startx);

                if(startx < r[nry-1][i].x1) {
                        printf("5corridor from room %d,%d to %d,%d\n", nry,i,nry-1,i);
                        paint_corridor_horizontal(l, startx, r[nry-1][i].x2, endy);
                }
                if(startx > r[nry-1][i].x2) {
                        printf("6corridor from room %d,%d to %d,%d\n", nry,i,nry-1,i);
                        paint_corridor_horizontal(l, startx, r[nry-1][i].x1, endy);
                }*/

        }

        for(j = nry; j > 2; j--) {
                startx = r[j][nrx].x1 + ri(2, r[j][nrx].sx-1);
                endy   = r[j-1][nrx].y2 - ri(2, r[j-1][nrx].sy-1);

                //printf("4corridor from room %d,%d to %d,%d\n", j,nrx,j-1,nrx);
                paint_corridor_vertical(l, r[j][nrx].y1, endy, startx);

                if(startx < r[j-1][nrx].x1) {
                        //printf("5corridor from room %d,%d to %d,%d\n", j,nrx,j-1,nrx);
                        paint_corridor_horizontal(l, endy, startx, r[j-1][nrx].x2);
                }
                if(startx > r[j-1][nrx].x2) {
                        //printf("6corridor from room %d,%d to %d,%d\n", j,nrx,j-1,nrx);
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

        tx = ri(0, XSIZE-1);  // starting X
        ty = ri(0, YSIZE-1);  // starting y
        xsize = ri(modifier, maxsize-1+modifier);  // total size X
        ysize = ri(modifier, maxsize-1+modifier);  // total size Y

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

/*********************************************
* Description - Flood fill to test dungeon gen
* Author - RK
* Date - Dec 14 2011
* *******************************************/
void floodfill(level_t *l, int y, int x)
{
//fprintf(stderr, "DEBUG: %s:%d - entering floodfill! x,y = %d,%d\n", __FILE__, __LINE__, x, y);
        if(l->c[y][x].type == DNG_FLOOR) {
                l->c[y][x].type = DNG_FILL;
                l->c[y][x].color = COLOR_LAKE;
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

void set_level_visited(level_t *l)
{
        int x,y;

        for(y=0;y<l->ysize;y++) {
                for(x=0;x<l->xsize;x++) {
                        l->c[y][x].flags |= CF_VISITED;
                }
        }
}

void addfloor(level_t *l, float y, float x)
{
        if((int)y >= l->ysize || (int)x >= l->xsize || (int)y < 0 || (int)x < 0)
                return;

        l->c[(int)y][(int)x].type = DNG_FLOOR;
        l->c[(int)y][(int)x].color = COLOR_SHADE;
}

void addwall(level_t *l, int y, int x)
{
        l->c[y][x].type  = DNG_WALL;
        l->c[y][x].color = COLOR_SHADE;
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
        if(type == DNG_WALL)
                return false;
        if(type == AREA_LAKE)
                return false;
        if(type == AREA_WALL)
                return false;
        if(type == AREA_NOTHING)
                return false;

        return true;
}

bool monster_passable(level_t *l, int y, int x)
{
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
        else if(l->c[y][x].monster)
                return false;
        else if(y == ply && x == plx)
                return false;               // replace with attack code or something later
        else if(l->c[y][x].flags & CF_HAS_DOOR_CLOSED)
                return false;
        else
                return passable(l, y, x);
}

/*********************************************
* Description - One big function which should
* take care of all world generation stuff.
* Author - RK
* Date - Dec 12 2011
* *******************************************/
void generate_world()
{
        int x, y;

        /*
         * Generate the outside world first.
         */
        for(x = 0; x < world->out->xsize; x++) {
                for(y = 0; y < world->out->ysize; y++) {
                        world->out->c[y][x].type = AREA_PLAIN;
                        world->out->c[y][x].flags = 0;
                        world->out->c[y][x].color = COLOR_PLAIN;
                        world->out->c[y][x].monster = NULL;
                        world->out->c[y][x].inventory = NULL;
                        world->out->c[y][x].visible = 0;
                }
        }

        world->forests  = ri(gtconfig.minf, gtconfig.maxf);
        world->cities   = ri(gtconfig.minc, gtconfig.maxc);
        world->villages = ri(gtconfig.minv, gtconfig.maxv);
        world->dungeons = ri(gtconfig.mind, gtconfig.maxd);

        fprintf(stderr, "DEBUG: %s:%d - Generating %d forests\n", __FILE__, __LINE__, world->forests);
        world->forest = (forest_t *) gtcalloc((size_t)world->forests, sizeof(forest_t));
        generate_forest(world->forests);

        fprintf(stderr, "DEBUG: %s:%d - Generating %d cities\n", __FILE__, __LINE__, world->cities);
        world->city = gtcalloc((size_t)world->cities, sizeof(city_t));
        generate_city(world->cities);

        fprintf(stderr, "DEBUG: %s:%d - Generating %d villages\n", __FILE__, __LINE__, world->villages);
        world->village = gtcalloc((size_t)world->villages, sizeof(city_t));
        generate_village(world->villages);
        spawn_monsters(100, world->out, 100); 
        spawn_objects(ri(world->out->xsize/8, world->out->ysize/4), world->out);

        //fprintf(stderr, "DEBUG: %s:%d - Generating dungeon!!\n", __FILE__, __LINE__);
        //generate_dungeon_labyrinthine(1);
        
        zero_level(&world->dng[1]);
        generate_dungeon_normal2(1);
        spawn_monsters(20, &world->dng[1], 100);
        spawn_objects(ri(world->dng[1].xsize/10, world->dng[1].xsize/5), &world->dng[1]);
        game->createddungeons++;

        // create the edge of the world
        for(x=0; x<world->out->xsize; x++) {
                world->out->c[1][x].type = AREA_WALL;
                world->out->c[2][x].type = AREA_WALL;
                world->out->c[794][x].type = AREA_WALL;
                world->out->c[795][x].type = AREA_WALL;
        }
        for(y=0; y<world->out->ysize; y++) {
                world->out->c[y][1].type = AREA_WALL;
                world->out->c[y][2].type = AREA_WALL;
                world->out->c[y][794].type = AREA_WALL;
                world->out->c[y][795].type = AREA_WALL;
        }
}
