#include "glengine.h"
#include "texmgr.h"
#include "gfx.h"

#include <stdio.h>
#include <assert.h>
#include <SDL/SDL.h>
#include <math.h>

#define MODE SDL_OPENGL
#define SCREEN_W 1024
#define SCREEN_H 768
#define SCALE_STEP 0.2
#define SCALE_ASTEP 0.01

int mouse, running;
double scale = 1;
double nextscale = 1;

void scale_viewport(double nscale)
{
	double offs_x, offs_y;
	int x, y;
	SDL_GetMouseState(&x, &y);
	offs_x = (nscale - scale) / (scale*nscale) * x;
	offs_y = (nscale - scale) / (scale*nscale) * y;
	scale = nscale;
	gl_change_viewport(gl.viewport.x + offs_x, gl.viewport.y + offs_y,
			SCREEN_W/scale,
			SCREEN_H/scale);
}

void process_event(SDL_Event *e)
{
	switch (e->type) {
	case SDL_QUIT:
		running = 0;
		break;
	case SDL_MOUSEMOTION:
		if (mouse)
			gl_change_viewport(gl.viewport.x - e->motion.xrel/scale,
				gl.viewport.y - e->motion.yrel/scale,
				gl.viewport.w, gl.viewport.h);
		break;
	case SDL_MOUSEBUTTONUP:
		switch (e->button.button) {
		case 1:
			mouse = 0;
			break;
		case 4:
			nextscale += SCALE_STEP;
			if (nextscale > 2.5)
				nextscale = 2.5;
			break;
		case 5:
			nextscale -= SCALE_STEP;
			if (nextscale < 0.4)
				nextscale = 0.4;
			break;
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		switch(e->button.button) {
		case 1:
			mouse = 1;
			break;
		}
		break;
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s file.cgl\n", argv[0]);
		exit(-1);
	}
	SDL_Surface *screen;
	SDL_Surface *gfx = read_gfx("data/GRAVITY.GFX");
	if (!gfx) {
		fprintf(stderr, "read_gfx: %s\n", SDL_GetError());
		abort();
	}
	struct cgl *cgl = read_cgl(argv[1], NULL);
	if (!cgl) {
		fprintf(stderr, "read_cgl: %s\n", SDL_GetError());
		abort();
	}
	cgl_preprocess(cgl);
	struct cg *cg = cg_init(cgl);
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL failed: %s\n", SDL_GetError());
		abort();
	}
	screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 0, MODE);
	gl_change_viewport(0, 0, screen->w/scale, screen->h/scale);
	tm_init(gfx);
	gl_init(cg);
	int t = SDL_GetTicks();
	running = 1;
	mouse = 0;
	SDL_Event e;
	while (running) {
		while (SDL_PollEvent(&e))
			process_event(&e);
		if (nextscale < scale) {
			if (scale - SCALE_ASTEP < nextscale)
				scale_viewport(nextscale);
			else
				scale_viewport(scale - SCALE_ASTEP);
		} else if (nextscale > scale) {
			if (scale + SCALE_ASTEP > nextscale)
				scale_viewport(nextscale);
			else
				scale_viewport(scale + SCALE_ASTEP);
		}
		cg_step(cg, (SDL_GetTicks() - t) / 1000.0);
		gl_draw_scene();
	}
	t = SDL_GetTicks() - t;
	printf("%d frames in %d ms. %f fps\n", gl.frame, t, (float)gl.frame / t * 1000);
	free_cgl(cgl);
	return 0;
}
