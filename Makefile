SHELL=/bin/bash

CC=gcc -g -ggdb
WARN=-Wall -Wextra
LIBS=-lm `sdl-config --libs` -lGL -lSDL_image
CFLAGS=`sdl-config --cflags` -O2 -pedantic -std=c99 $(WARN)
SOURCES=cgl.c gfx.c cgl_view.c graphics.c texmgr.c cg.c geometry.c osd.c osdlib.c
HEADERS=cgl.h gfx.h texmgr.h graphics.h cg.h mathgeom.h basic_types.h osd.h osdlib.h
FILES=$(SOURCES) $(HEADERS)

all: dep
	make cgl_view

dep:
	@echo -en > Makefile.dep
	@for s in $(SOURCES); do \
		gcc -M $$s >> Makefile.dep; \
		echo -e '\t'@echo CC $$s >> Makefile.dep; \
		echo -e '\t'@$(CC) $(CFLAGS) -c $$s >> Makefile.dep; \
	done

-include Makefile.dep

cgl_view: cgl_view.o cgl.o gfx.o graphics.o texmgr.o cg.o geometry.o osd.o osdlib.o
	@echo LINK freecg
	@$(CC) -o cgl_view $^ $(LIBS)

clean:
	rm -fr *.o cgl_view
