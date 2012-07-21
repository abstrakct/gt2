#
# Gullible's Travails
# Makefile
#

#
# Uncomment the appropriate line to compile with ncurses or libtcod
#

DISPLAY = libtcod
#DISPLAY = ncurses

ifeq ($(DISPLAY),ncurses)
CFLAGS = -Wall -g -ggdb3 -I. -DGT_USE_NCURSES # -pg
LIBS = -lm -lconfig -lncursesw
else
CFLAGS = -Wall -g -ggdb3 -I. -DGT_USE_LIBTCOD # -pg
LIBS = -lm -lconfig -ltcod
endif

CC = gcc 
DEFINES = #-DGT_USE_DUMMY
LDFLAGS = $(LIBS)  # -pg ,-rpath=lib 

SOURCES   = gt.c utils.c monsters.c datafiles.c world.c debug.c saveload.c actor.c objects.c o_effects.c cards.c fractmod.c npc.c quest.c io_ncurses.c io_libtcod.c 
HEADERS   = gt.h utils.h monsters.h datafiles.h world.h debug.h saveload.h actor.h objects.h o_effects.h cards.h fractmod.h npc.h quest.h io_ncurses.h io_libtcod.h commands.h
NCOBJS    = gt.o utils.o monsters.o datafiles.o world.o debug.o saveload.o actor.o objects.o o_effects.o cards.o fractmod.o npc.o quest.o io_ncurses.o 
TCOBJS    = gt.o utils.o monsters.o datafiles.o world.o debug.o saveload.o actor.o objects.o o_effects.o cards.o fractmod.o npc.o quest.o io_libtcod.o 

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
