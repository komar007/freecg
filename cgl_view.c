/*
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
 * along with FreeCG; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "graphics.h"
#include "texmgr.h"
#include "gfx.h"

#include <stdio.h>
#include <assert.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <math.h>

#define MODE SDL_OPENGL
#define SCREEN_W 1024
#define SCREEN_H 768
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
			gl.cam.x -= e->motion.xrel/gl.cam.scale;
			gl.cam.y -= e->motion.yrel/gl.cam.scale;
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
		case SDLK_LEFT:
			gl.cg->ship->rots = -5.5;
			break;
		case SDLK_RIGHT:
			gl.cg->ship->rots = 5.5;
			break;
		case SDLK_UP:
			cg_ship_set_engine(gl.cg->ship, 1);
			break;
		default:
			break;
		}
		break;
	case SDL_KEYUP:
		switch(e->key.keysym.sym) {
		case SDLK_UP:
			cg_ship_set_engine(gl.cg->ship, 0);
		case SDLK_LEFT:
			if (gl.cg->ship->rots == -5.5)
				gl.cg->ship->rots = 0;
			break;
		case SDLK_RIGHT:
			if (gl.cg->ship->rots == 5.5)
				gl.cg->ship->rots = 0;
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
	gl_resize_viewport(screen->w, screen->h);
	struct texmgr *ttm = tm_request_texture(gfx);
	gl_init(cg, ttm);
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
		cg_step(cg, time / 1000.0);
		if (nt > 100) {
			printf("%d frames in %d ms - %.1f fps\n",
					gl.frame - fr, nt, (float)(gl.frame - fr) / nt * 1000);
			printf("\rF %.1lf k[", cg->ship->fuel);
			for (size_t i = 0; i < 4; ++i)
				printf("%c", cg->ship->keys[i] ? i + '0' : ' ');
			printf("] sh[");
			for (size_t i = 0; i < cg->ship->num_freigh; ++i)
				printf("%d", cg->ship->freigh[i]);
			for (size_t i = 0; i < cg->ship->max_freigh - cg->ship->num_freigh; ++i)
				printf(" ");
			printf("] hb[");
			for (size_t i = 0; i < cg->level->hb->num_cargo; ++i)
				printf("%d", cg->level->hb->c.freigh[i]);
			printf("]%d/%d ", cg->level->hb->num_cargo,
					cg->level->num_all_freigh);
			if (cg->ship->has_turbo)
				printf("T ");
			if (cg->ship->dead) {
				if (cg->level->hb->num_cargo == cg->level->num_all_freigh)
					printf("You won!");
				else
					printf("Dead. Game over!");
			}
			fflush(stdout);

			t += nt;
			fr = gl.frame;
		}
		gl.cam.x = cg->ship->x;
		gl.cam.y = cg->ship->y;
		gl_update_window();
	}
	free_cgl(cgl);
	return 0;
}
