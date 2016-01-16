CC = gcc 
CFLAGS = -Wall -g -ggdb3 -I. -Iinclude #-DDS_USE_LIBTCOD
#DEFINES = -DGT_USE_DUMMY
LIBS = -lm -lconfig -ltcod -Llib -Wl,-rpath=lib
LDFLAGS = $(LIBS)     #,-rpath=lib 
 
SOURCES = dsrl.c utils.c monsters.c datafiles.c world.c display.c debug.c saveload.c commands.c actor.c objects.c o_effects.c fractmod.c dstime.c npc-names.c npc.c player.c
HEADERS = dsrl.h utils.h monsters.h datafiles.h world.h display.h debug.h saveload.h commands.h actor.h objects.h o_effects.h fractmod.h dstime.h npc-names.h npc.h player.h
OBJS    = dsrl.o utils.o monsters.o datafiles.o world.o display.o debug.o saveload.o commands.o actor.o objects.o o_effects.o fractmod.o dstime.o npc-names.o npc.o player.o

ds: $(OBJS)
	$(CC) $(DEFINES) $(LDFLAGS) -o $@ $(OBJS)

ds.: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $(SOURCES)

#makeobjdefs: $(MKOBJS) $(MKHDRS) $(MKSRCS) 
#	$(CC) -o $@ $(MKOBJS) 
#	./makeobjdefs

#makeobjdefs.: $(MKSRCS) $(MKHDRS)

clean:
	rm -f *.o dsrl core 

depend:
	gcc -MM *.c > .deps

all: gt
#makeobjdefs

include .deps
