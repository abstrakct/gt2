/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "monsters.h"
#include "objects.h"
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

