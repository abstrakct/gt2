#ifndef _IO_H
#define _IO_H

#ifdef GT_USE_NCURSES 

typedef int gtcolor_t;
typedef int gtkey;

#include <curses.h>
#include "io_ncurses.h"



#else



#include <libtcod/libtcod.h>

typedef struct {
        TCOD_color_t fore, back;
} gtcolor_t;

typedef TCOD_key_t gtkey;

#include "io_libtcod.h"

#endif

#endif
