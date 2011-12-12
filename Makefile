CC = gcc
CFLAGS = -Wall -g -ggdb -I. 
LIBS = -lconfig -lncursesw
LDFLAGS = -Wl $(LIBS)  # ,-rpath=lib 

SOURCES = gt.c utils.c datafiles.c you.c world.c
HEADERS = gt.h utils.h monsters.h datafiles.h you.h world.h
OBJS    = gt.o utils.o datafiles.o you.o world.o

#MKOBJS = objects.o makeobjdefs.o
#MKSRCS = objects.c makeobjdefs.c
#MKHDRS = objects.h objlist.h

gt: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

gt.: $(SOURCES) $(HEADERS)

#makeobjdefs: $(MKOBJS) $(MKHDRS) $(MKSRCS) 
#	$(CC) -o $@ $(MKOBJS) 
#	./makeobjdefs

#makeobjdefs.: $(MKSRCS) $(MKHDRS)

clean:
	rm *.o gt #makeobjdefs


all: gt
#makeobjdefs
