CC = gcc 
CFLAGS = -Wall -g -ggdb3 -I. -DGT_USE_NCURSES
#DEFINES = -DGT_USE_DUMMY
LIBS = -lm -lconfig -lncursesw
LDFLAGS = -Wl $(LIBS)  # ,-rpath=lib 
 
SOURCES = gt.c utils.c monsters.c datafiles.c world.c display.c debug.c saveload.c commands.c actor.c objects.c o_effects.c cards.c fractmod.c
HEADERS = gt.h utils.h monsters.h datafiles.h world.h display.h debug.h saveload.h commands.h actor.h objects.h o_effects.h cards.h fractmod.h
OBJS    = gt.o utils.o monsters.o datafiles.o world.o display.o debug.o saveload.o commands.o actor.o objects.o o_effects.o cards.o fractmod.o

gt: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

gt.: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $(SOURCES)

#makeobjdefs: $(MKOBJS) $(MKHDRS) $(MKSRCS) 
#	$(CC) -o $@ $(MKOBJS) 
#	./makeobjdefs

#makeobjdefs.: $(MKSRCS) $(MKHDRS)

clean:
	rm -f *.o gt core 

depend:
	gcc -MM *.c > .deps

all: gt
#makeobjdefs

include .deps
