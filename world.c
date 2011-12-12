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
#include "gt.h"

char mapchars[50] = {
        ' ',
        '.',
        '~',
        'T',
        '#',
        'v',
        '>'
};

void generate_world()
{
        int x, y;

        for(x = 0; x < XSIZE; x++) {
                for(y = 0; y < YSIZE; y++) {
                        world->out[y][x].type = d(1,6);
                        world->out[y][x].flags = 0;
                        world->out[y][x].color = COLOR_GOOD;
                        world->out[y][x].monster = NULL;
                        world->out[y][x].inventory = NULL;
                }
        }

        world->forests  = ri(game->c.minf, game->c.maxf);
        world->cities   = ri(game->c.minc, game->c.maxc);
        world->villages = ri(game->c.minv, game->c.maxv);
        world->dungeons = ri(game->c.mind, game->c.maxd);
}
