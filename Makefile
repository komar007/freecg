all: cgl_dump

%: %.c cgl.c cgl.h gfx.c gfx.h
	gcc $< gfx.c cgl.c -o $@ -Wall -Wextra -lm -std=c99 -pedantic -DDEBUG -O2 `sdl-config --libs`

%.run: %
	./$<

%.rin: % %.in
	./$< <$<.in

