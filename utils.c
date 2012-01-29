/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "world.h"
#include "display.h"
#include "gt.h"


void *garbage[MAX_GARBAGE];
int garbageindex;

int ri(int a, int b) 
{
        int result;
        result = (a + (rand() % (b-a+1)));

        return result;
}

void get_version_string(char *s)
{
        sprintf(s, "%d.%d.%d", GT_VERSION_MAJ, GT_VERSION_MIN, GT_VERSION_REV);
}

void die(char *m, ...)
{
        va_list argp;
        char s[1000];
        char s2[1020];

        sprintf(s2, "FATAL ERROR: ");

        va_start(argp, m);
        vsprintf(s, m, argp);
        va_end(argp);

        strcat(s2, s);
        fprintf(stderr, "%s", s2);

        shutdown_display();
        shutdown_gt();
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

int perc(int i)
{
        int x;

        x = ri(1, 100);
        if(x <= i)
                return TRUE;
        else
                return FALSE;
}

void *gtmalloc(size_t size)
{
        void *p;

        p = malloc(size);
        if(!p)
                die("Memory allocation in gtmalloc for size %d failed! Exiting.\n", (int) size);

        memset(p, 0, size);
        garbage[garbageindex] = p;
        garbageindex++;

        return p;
}

void *gtcalloc(size_t num, size_t size)
{
        void *p;

        p = calloc(num, size);
        if(!p)
                die("calloc %d * %d (total %d) in gtcalloc failed! Exiting.\n", (int) num, (int) size, (int) (num*size));

        memset(p, 0, num*size);
        garbage[garbageindex] = p;
        garbageindex++;

        return p;
}

void gtfree(void *ptr)
{
        int i;

        for(i=0;i<MAX_GARBAGE;i++)
                if(garbage[i] == ptr)
                        break;

        garbage[i] = NULL;
        free(ptr);
}

void you(char *fmt, ...)
{
        va_list argp;
        char s[1000];
        char s2[1020];

        sprintf(s2, "You ");

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        strcat(s2, s);
        mess(s2);
}

void youc(int color, char *fmt, ...)
{
        va_list argp;
        char s[1000];
        char s2[1020];

        sprintf(s2, "You ");

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        strcat(s2, s);
        messc(color, s2);
}

void yousee(char *fmt, ...)
{
        va_list argp;
        char s[1000];
        char s2[1020];

        sprintf(s2, "You see ");

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        strcat(s2, s);
        mess(s2);
}

void gtprintf(char *fmt, ...)
{
        va_list argp;
        char s[1000];

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        mess(s);
}

void gtprintfc(int color, char *fmt, ...)
{
        va_list argp;
        char s[1000];

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        messc(color, s);
}

