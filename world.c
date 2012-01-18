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
        //int color;

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
                        world->dng[d].c[fy][fx].type = DNG_WALL;
                        world->dng[d].c[fy][fx].color = COLOR_NORMAL;
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

        // the edges
        for(ty=0;ty<ysize;ty++) {
                world->dng[d].c[ty][1].type = DNG_WALL;
                world->dng[d].c[ty][xsize-1].type = DNG_WALL;
        }

        for(tx=0;tx<xsize;tx++) {
                world->dng[d].c[1][tx].type = DNG_WALL;
                world->dng[d].c[ysize-5][tx].type = DNG_WALL;
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
void floodfill(int y, int x)
{
//fprintf(stderr, "DEBUG: %s:%d - entering floodfill! x,y = %d,%d\n", __FILE__, __LINE__, x, y);
        if(world->cmap[y][x].type == DNG_FLOOR) {
                world->cmap[y][x].type = DNG_FILL;
                world->cmap[y][x].color = COLOR_LAKE;
                floodfill(x-1, y);
                floodfill(x+1, y);
                floodfill(x, y-1);
                floodfill(x, y+1);
                floodfill(x+1, y+1);
                floodfill(x+1, y-1);
                floodfill(x-1, y+1);
                floodfill(x-1, y-1);
        }
}

void set_all_visible()
{
        int x,y;

        for(y=0;y<world->curlevel->ysize;y++) {
                for(x=0;x<world->curlevel->xsize;x++) {
                        world->curlevel->c[y][x].visible = true;
                }
        }
}

/*********************************************
* Description - Paint a room in a dungeon
* Author - RK
* Date - Dec 14 2011
* *******************************************/
void paint_room(level_t *l, int y, int x, int sy, int sx, int join_overlapping)
{
        int i, j;
        int door;

        if(((x+sx) >= l->xsize) || ((y+sy) >= l->ysize))
                return;

        for(i = x; i <= (x+sx); i++) {
                for(j = y; j <= (y+sy); j++) {
                        if((j == y) || (j == y+sy) || (i == x) || (i == x+sx)) {
                                if(join_overlapping) {
                                        if(l->c[j][i].type != DNG_FLOOR) {
                                                l->c[j][i].type = DNG_WALL;
                                                l->c[j][i].color = COLOR_NORMAL;
                                        }
                                } else {
                                        l->c[j][i].type = DNG_WALL;
                                        l->c[j][i].color = COLOR_NORMAL;
                                }
                        } else {
                                l->c[j][i].type = DNG_FLOOR;
                                l->c[j][i].color = COLOR_NORMAL;
                        }
                        door = ri(1,100);
                        if(door >= 99)
                                l->c[j][i].type = DNG_FLOOR;
                }
        }
}

bool passable(int y, int x)
{
        int type;

        if(game->wizardmode)   // if we are in wizard mode, ignore this! dangerous, but useful
                return true;

        if(x >= world->curlevel->xsize)
                return false;
        if(y >= world->curlevel->ysize)
                return false;

        type = world->curlevel->c[y][x].type;
        if(type == DNG_WALL)
                return false;
        if(type == AREA_LAKE)
                return false;
        if(type == AREA_WALL)
                return false;

        return true;
}

bool monster_passable(int y, int x)
{
        if(ct(y,x) == AREA_VILLAGE || ct(y,x) == AREA_CITY)
                return false;
        else if(cm(y,x))
                return false;
        else
                return passable(y, x);
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

        fprintf(stderr, "DEBUG: %s:%d - Generating dungeon!!\n", __FILE__, __LINE__);
        generate_dungeon_labyrinthine(1);
        game->createddungeons++;
        paint_room(&world->dng[1], 10, 10, 10, 10, 0);

        // create the edge of the world
        for(x=0; x<world->out->xsize; x++) {
                world->out->c[1][x].type = AREA_WALL;
                world->out->c[2][x].type = AREA_WALL;
                world->out->c[792][x].type = AREA_WALL;
                world->out->c[793][x].type = AREA_WALL;
        }
        for(y=0; y<world->out->ysize; y++) {
                world->out->c[y][1].type = AREA_WALL;
                world->out->c[y][2].type = AREA_WALL;
                world->out->c[y][794].type = AREA_WALL;
                world->out->c[y][795].type = AREA_WALL;
        }
}
