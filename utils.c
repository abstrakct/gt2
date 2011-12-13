/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>

#include "monsters.h"
#include "objects.h"
#include "utils.h"
#include "world.h"
#include "gt.h"

char *get_version_string()
{
        char *s;

        s = gtmalloc((sizeof(int))*10);
        sprintf(s, "%d.%d.%d", GT_VERSION_MAJ, GT_VERSION_MIN, GT_VERSION_REV);

        return s;
}

void die(char *m)
{
        fprintf(stderr, "FATAL ERROR: %s\n", m);
        exit(1);
}

int dice(int num, int sides, signed int modifier)
{
        int i, result;

        result = modifier;
        for(i=0;i<num;i++) {
                result += 1 + (rand() % sides);
        }

        return result;
}

void *gtmalloc(size_t size)
{
        void *p;

        p = malloc(size);
        if(!p) {
                char m[100];
                sprintf(m, "Memory allocation in gtmalloc for size %d failed! Exiting.\n", (int) size);
                die(m);
        }

        return p;
}

void *gtcalloc(size_t num, size_t size)
{
        void *p;

        p = calloc(num, size);
        if(!p) {
                char m[200];
                sprintf(m, "calloc %d * %d (total %d) in gtcalloc failed! Exiting.\n", (int) num, (int) size, (int) (num*size));
                die(m);
        }

        return p;
}

int isarmor(obj_t *o)
{
        int retval = 0;

        if(o->type == OT_ARMOR)
                retval = 1;
        if(o->type == OT_BODYARMOR)
                retval = 1;

        return retval;
}
