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

#include "objects.h"
#include "monsters.h"
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "you.h"
#include "display.h"
#include "actor.h"
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
        '.'   //foreest nohouse
};

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
                                        world->out[fy][fx].type = AREA_FOREST_NOTREE;
                                        world->out[fy][fx].color = COLOR_NORMAL;
                                        break;
                                case AREA_VILLAGE:
                                        world->village[i].x1 = tx;
                                        world->village[i].y1 = ty;
                                        world->village[i].x2 = tx+xsize-1;
                                        world->village[i].y2 = ty+ysize-1;
                                        if(world->out[fy][fx].type == AREA_PLAIN) {
                                                world->out[fy][fx].type = AREA_VILLAGE_NOHOUSE;
                                                world->out[fy][fx].color = COLOR_NORMAL;
                                        }
                                        break;
                                case AREA_CITY:
                                        world->city[i].x1 = tx;
                                        world->city[i].y1 = ty;
                                        world->city[i].x2 = tx+xsize-1;
                                        world->city[i].y2 = ty+ysize-1;
                                        if(world->out[fy][fx].type == AREA_PLAIN) {
                                                world->out[fy][fx].type = AREA_CITY_NOHOUSE;
                                                world->out[fy][fx].color = COLOR_NORMAL;
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
                                world->out[fy][fx].type = type;
                                switch(type) {
                                        case AREA_FOREST:
                                                world->out[fy][fx].color = COLOR_FOREST;
                                                break;
                                        case AREA_VILLAGE:
                                        case AREA_CITY:
                                                world->out[fy][fx].color = COLOR_CITY;
                                                break;
                                }
                                /*
                                switch(type) {
                                        case FOREST:
                                                color = ri(0,4);
                                                world->out[fy][fx].color = forestcolors[color];
                                                break;
                                        case VILLAGE:
                                                color = ri(0,1); 
                                                world->out[fy][fx].color = citycolors[color];
                                                world->village[i].houses++;
                                                break;
                                        case CITY:
                                                color = ri(0,1);
                                                world->out[fy][fx].color = citycolors[color];
                                                world->city[i].houses++;
                                                break;
                                        case DUNGEON:
                                                color = ri(0,2);
                                                world->out[fy][fx].color = dungeoncolors[color];
                                                world->dungeons++;
                                                world->out[fy][fx].flags |= HAS_DUNGEON;
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
* Description - One big function which should
* take care of all world generation stuff.
* Author - RK
* Date - Dec 12 2011
* *******************************************/
void generate_world()
{
        int x, y;

        for(x = 0; x < XSIZE; x++) {
                for(y = 0; y < YSIZE; y++) {
                        world->out[y][x].type = AREA_PLAIN;
                        world->out[y][x].flags = 0;
                        world->out[y][x].color = COLOR_PLAIN;
                        world->out[y][x].monster = NULL;
                        world->out[y][x].inventory = NULL;
                }
        }

        world->forests  = ri(game->c.minf, game->c.maxf);
        world->cities   = ri(game->c.minc, game->c.maxc);
        world->villages = ri(game->c.minv, game->c.maxv);
        world->dungeons = ri(game->c.mind, game->c.maxd);

        
fprintf(stderr, "DEBUG: %s:%d - Generating %d forests\n", __FILE__, __LINE__, world->forests);
        world->forest = (forest_t *) gtcalloc((size_t)world->forests, sizeof(forest_t));
        generate_forest(world->forests);

        world->city = gtcalloc((size_t)world->cities, sizeof(city_t));
        generate_city(world->cities);

        world->village = gtcalloc((size_t)world->villages, sizeof(city_t));
        generate_village(world->villages);
}