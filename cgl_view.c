#include "graphics.h"
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
		if (mouse) {
			gl_change_viewport(gl.viewport.x - e->motion.xrel/scale,
				gl.viewport.y - e->motion.yrel/scale,
				gl.viewport.w, gl.viewport.h);
		} else {
			gl.cg->ship->x += e->motion.xrel/scale;
			gl.cg->ship->y += e->motion.yrel/scale;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		switch (e->button.button) {
		case 1:
			mouse = 0;
			break;
		case 4:
			nextscale += SCALE_STEP;
			if (nextscale > 10)
				nextscale = 10;
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
	case SDL_KEYDOWN:
		switch (e->key.keysym.sym) {
		case SDLK_ESCAPE:
			running = 0;
			break;
		case SDLK_1:
			gl.cg->ship->keys[0] = !gl.cg->ship->keys[0];
			break;
		case SDLK_2:
			gl.cg->ship->keys[1] = !gl.cg->ship->keys[1];
			break;
		case SDLK_3:
			gl.cg->ship->keys[2] = !gl.cg->ship->keys[2];
			break;
		case SDLK_4:
			gl.cg->ship->keys[3] = !gl.cg->ship->keys[3];
			break;
		default:
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
	make_collision_map(gfx, cg->cmap);
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL failed: %s\n", SDL_GetError());
		abort();
	}
	screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 0, MODE);
	gl_change_viewport(0, 0, screen->w/scale, screen->h/scale);
	tm_init(gfx);
	gl_init(cg);
	int t = SDL_GetTicks(),
	    nt = t,
	    time = t,
	    fr = 0;
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
		time = SDL_GetTicks();
		nt = time - t;
		cg_step(cg, time / 1000.0);
		if (nt > 1000) {
			printf("%d frames in %d ms - %.1f fps\n",
					gl.frame - fr, nt, (float)(gl.frame - fr) / nt * 1000);
			t += nt;
			fr = gl.frame;
		}
		gl_draw_scene();
	}
	free_cgl(cgl);
	return 0;
}
