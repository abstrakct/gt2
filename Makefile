CC = gcc
CFLAGS = -Wall -g -ggdb3 -I. -DGT_USE_NCURSES # -O2
#DEFINES = -DGT_USE_DUMMY
LIBS = -lm -lconfig -lncursesw
LDFLAGS = -Wl $(LIBS)  # ,-rpath=lib 

SOURCES = gt.c utils.c monsters.c datafiles.c you.c world.c display.c debug.c saveload.c commands.c
HEADERS = gt.h utils.h monsters.h datafiles.h you.h world.h display.h debug.h saveload.h commands.h
OBJS    = gt.o utils.o monsters.o datafiles.o you.o world.o display.o debug.o saveload.o commands.o

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

all: gt
#makeobjdefs

#dependencies: 
commands.o: commands.c objects.h actor.h monsters.h utils.h datafiles.h \
 world.h you.h display.h debug.h saveload.h commands.h gt.h
datafiles.o: datafiles.c actor.h monsters.h objects.h utils.h datafiles.h \
 world.h gt.h
debug.o: debug.c objects.h actor.h monsters.h utils.h datafiles.h world.h \
 you.h display.h debug.h gt.h
display.o: display.c objects.h actor.h monsters.h utils.h datafiles.h \
 world.h you.h display.h gt.h
gt.o: gt.c objects.h actor.h monsters.h utils.h datafiles.h world.h you.h \
 display.h debug.h saveload.h commands.h gt.h
monsters.o: monsters.c objects.h actor.h monsters.h utils.h datafiles.h \
 world.h you.h display.h gt.h
saveload.o: saveload.c objects.h actor.h monsters.h utils.h datafiles.h \
 world.h you.h display.h debug.h saveload.h gt.h
utils.o: utils.c actor.h monsters.h objects.h utils.h world.h gt.h
world.o: world.c objects.h actor.h monsters.h utils.h datafiles.h world.h \
 you.h display.h gt.h
you.o: you.c actor.h monsters.h objects.h world.h display.h gt.h you.h
