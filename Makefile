#
# Gullible's Travails
# Makefile
#

#
# Uncomment the appropriate line to compile with ncurses or libtcod
#
#DISPLAY = libtcod
DISPLAY = ncurses

ifeq ($(DISPLAY),ncurses)
CFLAGS = -Wall -g -ggdb3 -I. -DGT_USE_NCURSES
LIBS = -lm -lconfig -lncursesw
else
CFLAGS = -Wall -g -ggdb3 -I. -DGT_USE_LIBTCOD
LIBS = -lm -lconfig -ltcod
endif

CC = gcc 
DEFINES = #-DGT_USE_DUMMY
LDFLAGS = -Wl $(LIBS)  # ,-rpath=lib 

SOURCES   = gt.c utils.c monsters.c datafiles.c world.c io_ncurses.c io_libtcod.c debug.c saveload.c commands.c actor.c objects.c o_effects.c cards.c fractmod.c
HEADERS   = gt.h utils.h monsters.h datafiles.h world.h io_ncurses.h io_libtcod.h debug.h saveload.h commands.h actor.h objects.h o_effects.h cards.h fractmod.h
NCOBJS    = gt.o utils.o monsters.o datafiles.o world.o io_ncurses.o debug.o saveload.o commands.o actor.o objects.o o_effects.o cards.o fractmod.o
TCOBJS    = gt.o utils.o monsters.o datafiles.o world.o io_libtcod.o debug.o saveload.o commands.o actor.o objects.o o_effects.o cards.o fractmod.o

#gt: $(OBJS)
#	$(CC) $(LDFLAGS) -o $@ $(OBJS)

#gt.: $(SOURCES) $(HEADERS)
#	$(CC) $(CFLAGS) $(DEFINES) -o $@ $(SOURCES)

ifeq ($(DISPLAY),ncurses)
gt: $(NCOBJS)
	$(CC) $(LDFLAGS) -o gt $(NCOBJS)
else
gt: $(TCOBJS)
	$(CC) $(LDFLAGS) -o gt $(TCOBJS)
endif


clean:
	rm -f *.o gt core 

depend:
	gcc -MM *.c > .deps

all: gt

include .deps
