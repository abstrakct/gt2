#ifndef _IO_H
#define _IO_H

#ifdef GT_USE_NCURSES 
#include "io_ncurses.h"
#else
#include "io_libtcod.h"
#endif

#endif
