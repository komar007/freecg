#include "glengine.h"
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
	SDL_Surface *screen = SDL_SetVideoMode(2560, 1024, 32,
			SDL_OPENGL|SDL_FULLSCREEN);
	init_opengl();
	SDL_Surface *gfx = read_gfx("data/GRAVITY.GFX");
	struct cgl *cgl = read_cgl("data/LEVEL12.CGL", NULL);
	assert(gfx);
	GLuint texno = load_texture(gfx);
	change_viewport(0, 0, 2560, 1024);
	test_draw(cgl, gfx);
	SDL_Delay(5000);
	return 0;
}
