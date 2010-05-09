CC=gcc
WARN=-Wall -Wextra
LIBS=-lm `sdl-config --libs` -lGL
CFLAGS=`sdl-config --cflags` -O2 -pedantic -std=c99 -pg
SOURCES=cgl.c cgl_dump.c gfx.c main.c glengine.c
HEADERS=cgl.h gfx.h
FILES=$(SOURCES) $(HEADERS)

all:
	make dep
	make opencg
	make cgl_dump

dep:
	@echo -en > Makefile.dep
	@for s in $(SOURCES); do \
		gcc -M $$s >> Makefile.dep; \
	done

-include Makefile.dep

cgl_dump: cgl_dump.o cgl.o
	$(CC) $(LIBS) -o cgl_dump $^

opencg: main.o cgl.o gfx.o glengine.o
	$(CC) $(LIBS) -o opencg $^

clean:
	rm -fr *.o cgl_dump opencg

*.o:
	$(CC) $*.c -c $(WARN) $(CFLAGS)


