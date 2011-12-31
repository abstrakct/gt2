/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include "actor.h"
#include "monsters.h"
#include "objects.h"
#include "world.h"
#include "display.h"
#include "gt.h"
#include "you.h"

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

