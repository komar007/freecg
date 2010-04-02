#include "cgl.h"
#include "gfx.h"
#include <SDL/SDL.h>
#include <SDL/SDL_error.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(-1);
	}
	struct cgl *cgl = read_cgl(argv[1]);
	if (!cgl) {
		fprintf(stderr, "read_cgl: %s\n", SDL_GetError());
		return -1;
	}

	SDL_Surface *screen;
	SDL_Surface *one;
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);
	screen = SDL_SetVideoMode(800, 600, 32, SDL_DOUBLEBUF);

	one = read_gfx("data/GRAVITY.GFX");
	if (!one) {
		fprintf(stderr, "read_gfx: %s\n", SDL_GetError());
		return -1;
	}


	/*
	SDL_Rect rect;
	rect.x = rect.y = 0;
	rect.w = one->w;
	rect.h = one->h;
	SDL_BlitSurface(one, NULL, screen, &rect);
	SDL_Flip(screen);

	SDL_Delay(5000);
	*/
	return 0;
}
