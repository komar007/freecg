/* Copyright (C) 2010 Michal Trybus.
 *
 * This file is part of FreeCG.
 *
 * FreeCG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * FreeCG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FreeCG. If not, see <http://www.gnu.org/licenses/>.
 */

#include "graphics.h"
#include "osd.h"
#include "texmgr.h"
#include "gfx.h"
#include "cg.h"

#include <stdio.h>
#include <assert.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <math.h>

#define MODE SDL_OPENGL
#define SCREEN_W 800
#define SCREEN_H 600
#define SCALE_STEP 0.2
#define SCALE_ASTEP 0.01

int mouse, running;

void process_event(SDL_Event *e)
{
	switch (e->type) {
	case SDL_QUIT:
		running = 0;
		break;
	case SDL_MOUSEMOTION:
		if (mouse) {
			gl.cam.nx -= e->motion.xrel/gl.cam.scale;
			gl.cam.ny -= e->motion.yrel/gl.cam.scale;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		switch (e->button.button) {
		case 1:
			mouse = 0;
			break;
		case 4:
			gl.cam.scale += 0.2;
//			nextscale += SCALE_STEP;
//			if (nextscale > 10)
//				nextscale = 10;
			break;
		case 5:
			gl.cam.scale -= 0.2;
//			nextscale -= SCALE_STEP;
//			if (nextscale < 0.4)
//				nextscale = 0.4;
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
			gl.l->ship->keys[0] = !gl.l->ship->keys[0];
			break;
		case SDLK_2:
			gl.l->ship->keys[1] = !gl.l->ship->keys[1];
			break;
		case SDLK_3:
			gl.l->ship->keys[2] = !gl.l->ship->keys[2];
			break;
		case SDLK_4:
			gl.l->ship->keys[3] = !gl.l->ship->keys[3];
			break;
		case SDLK_LEFT:
			gl.l->ship->rot_speed = -5.5;
			break;
		case SDLK_RIGHT:
			gl.l->ship->rot_speed = 5.5;
			break;
		case SDLK_UP:
			cg_ship_set_engine(gl.l->ship, 1);
			break;
		case SDLK_o:
			osd_toggle();
			break;
		default:
			break;
		}
		break;
	case SDL_KEYUP:
		switch(e->key.keysym.sym) {
		case SDLK_UP:
			cg_ship_set_engine(gl.l->ship, 0);
		case SDLK_LEFT:
			if (gl.l->ship->rot_speed == -5.5)
				gl.l->ship->rot_speed = 0;
			break;
		case SDLK_RIGHT:
			if (gl.l->ship->rot_speed == 5.5)
				gl.l->ship->rot_speed = 0;
			break;
		default:
			break;
		}
		break;
	}
}

int main(int argc, char *argv[])
{
	if (!(argc == 2 || argc == 4)) {
		printf("Usage: %s file.cgl [width height]\n", argv[0]);
		exit(-1);
	}
	SDL_Surface *screen;
	SDL_Surface *gfx = load_gfx("data/GRAVITY.GFX");
	if (!gfx) {
		fprintf(stderr, "read_gfx: %s\n", SDL_GetError());
		abort();
	}
	SDL_Surface *png = load_png("font.png");
	if (!png) {
		fprintf(stderr, "load_png: %s\n", SDL_GetError());
		abort();
	}
	SDL_Surface *osd = load_png("osd.png");
	if (!osd) {
		fprintf(stderr, "load_png: %s\n", SDL_GetError());
		abort();
	}
	struct cgl *cgl = read_cgl(argv[1], NULL);
	if (!cgl) {
		fprintf(stderr, "read_cgl: %s\n", SDL_GetError());
		abort();
	}
	cgl_preprocess(cgl);
	cg_init(cgl);
	make_collision_map(gfx, cmap);
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL failed: %s\n", SDL_GetError());
		abort();
	}
	if (argc == 4) {
		int w = atoi(argv[2]),
		    h = atoi(argv[3]);
		if (w && h)
			screen = SDL_SetVideoMode(w, h, 0, MODE);
		else
			fprintf(stderr, "Wrong resolution");
	} else {
		screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 0, MODE);
	}
	gl_resize_viewport(screen->w, screen->h);
	struct texmgr *ttm = tm_request_texture(gfx);
	struct texmgr *fnt = tm_request_texture(png);
	struct texmgr *otm = tm_request_texture(osd);
	gl_init(cgl, ttm, fnt, otm);
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
		time = SDL_GetTicks();
		nt = time - t;
		cg_step(cgl, time / 1000.0);
		if (nt > 5000) {
			printf("%d frames in %d ms - %.1f fps\n",
					gl.frame - fr, nt, (float)(gl.frame - fr) / nt * 1000);
			if (cgl->status == Lost)
				printf("Dead. Game over!");
			if (cgl->status == Victory)
				printf("You won!");
			fflush(stdout);

			t += nt;
			fr = gl.frame;
		}
		gl.cam.nx = cgl->ship->x + SHIP_W/2.0;
		gl.cam.ny = cgl->ship->y + SHIP_H/2.0;
		gl_update_window(time / 1000.0);
	}
	free_cgl(cgl);
	return 0;
}
