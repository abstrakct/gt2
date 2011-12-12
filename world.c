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
                        world->out[x][y].type = d(1,6);
                }
        }
}
