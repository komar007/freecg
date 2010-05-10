#include "glengine.h"
#include "texmgr.h"
#include "gfx.h"

#include <stdio.h>
#include <assert.h>
#include <SDL/SDL.h>
#include <math.h>

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL failed: %s\n", SDL_GetError());
		abort();
	}
	SDL_SetVideoMode(1024, 768, 32,
			SDL_OPENGL);
	init_opengl();
	SDL_Surface *gfx = read_gfx("data/GRAVITY.GFX");
	init_texture_manager(gfx);
	struct cgl *cgl = read_cgl("data/LEVEL14.CGL", NULL);
	assert(gfx);
	for (double x = 0; x < 100; x += 0.1) {
		change_viewport(x, 0, 1024, 768);
		test_draw(cgl, x, 0, x + 1024, 0 + 768);
	}
	return 0;
}
