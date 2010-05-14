CC=gcc
WARN=-Wall -Wextra
LIBS=-lm `sdl-config --libs` -lGL
CFLAGS=`sdl-config --cflags` -O2 -pedantic -std=c99 $(WARN)
SOURCES=cgl.c cgl_dump.c gfx.c main.c glengine.c texmgr.c cg.c
HEADERS=cgl.h gfx.h texmgr.h glengine.h cg.h opencg.h
FILES=$(SOURCES) $(HEADERS)

all: dep
	make opencg cgl_dump

dep:
	@echo -en > Makefile.dep
	@for s in $(SOURCES); do \
		gcc -M $$s >> Makefile.dep; \
		echo -e '\t'@echo CC $$s >> Makefile.dep; \
		echo -e '\t'@$(CC) $(CFLAGS) -c $$s >> Makefile.dep; \
	done

-include Makefile.dep

cgl_dump: cgl_dump.o cgl.o
	@echo LINK cgl_dump
	@$(CC) $(LIBS) -o cgl_dump $^

opencg: main.o cgl.o gfx.o glengine.o texmgr.o cg.o
	@echo LINK opencg
	@$(CC) $(LIBS) -o opencg $^

clean:
	rm -fr *.o cgl_dump opencg
