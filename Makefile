CC = gcc 
CFLAGS = -Wall -g -ggdb3 -I. -DGT_USE_NCURSES # -O2
#DEFINES = -DGT_USE_DUMMY
LIBS = -lm -lconfig -lncursesw
LDFLAGS = -Wl $(LIBS)  # ,-rpath=lib 

SOURCES = gt.c utils.c monsters.c datafiles.c you.c world.c display.c debug.c saveload.c commands.c actor.c objects.c
HEADERS = gt.h utils.h monsters.h datafiles.h you.h world.h display.h debug.h saveload.h commands.h actor.h objects.h
OBJS    = gt.o utils.o monsters.o datafiles.o you.o world.o display.o debug.o saveload.o commands.o actor.o objects.o

#MKOBJS = objects.o makeobjdefs.o
#MKSRCS = objects.c makeobjdefs.c
#MKHDRS = objects.h objlist.h

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
