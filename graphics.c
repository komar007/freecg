#include "opencg.h"
#include "graphics.h"
#include "texmgr.h"
#include <assert.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

struct glengine gl;

void gl_init(struct cg* cg)
{
	gl.frame = 0;
	gl.cg = cg;
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
}

void gl_change_viewport(double x, double y, double w, double h)
{
	assert(w > 0 && h > 0);
	gl.viewport.x = x, gl.viewport.y = y;
	gl.viewport.w = w, gl.viewport.h = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1, -1, 1);
	glOrtho(x, x+w, y, y+h, 0, 1);
	glMatrixMode(GL_MODELVIEW);
}

void gl_draw_scene()
{
	extern void fix_lframes(struct cgl*);
	extern void draw_block(struct tile *[]);
	if (gl.frame == 0)
		fix_lframes(gl.cg->level);
	double x1 = fmax(gl.viewport.x, 0),
	       y1 = fmax(gl.viewport.y, 0),
	       x2 = fmin(gl.viewport.x + gl.viewport.w,
			       gl.cg->level->width * BLOCK_SIZE),
	       y2 = fmin(gl.viewport.y + gl.viewport.h,
			       gl.cg->level->height * BLOCK_SIZE);
	struct cgl *l = gl.cg->level;
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
	for (size_t j = y1/BLOCK_SIZE; j*BLOCK_SIZE < y2; ++j)
		for (size_t i = x1/BLOCK_SIZE; i*BLOCK_SIZE < x2; ++i)
			draw_block(l->blocks[j][i]);
	SDL_GL_SwapBuffers();
	gl.frame++;
}

void fix_lframes(struct cgl *level)
{
	for (size_t i = 0; i < level->ntiles; ++i)
		level->tiles[i].lframe = 0;
	gl.frame = 1;
}

void draw_block(struct tile *tiles[])
{
	extern void dispatch_drawing(struct tile*);
	for (size_t i = 0; tiles[i]; ++i) {
		if (tiles[i]->lframe != gl.frame) {
			dispatch_drawing(tiles[i]);
			tiles[i]->lframe = gl.frame;
		}
	}
}

void draw_simple_tile(struct tile *tile, int img_x, int img_y)
{
	struct texture *tex = tm_request_texture(img_x, img_y,
			tile->w, tile->h);
	glBindTexture(GL_TEXTURE_2D, tex->no);
	glBegin(GL_QUADS);
	tm_coord_tl(tex);
	glVertex2f(tile->x, tile->y);
	tm_coord_bl(tex);
	glVertex2f(tile->x, tile->y + tile->h);
	tm_coord_br(tex);
	glVertex2f(tile->x + tile->w, tile->y + tile->h);
	tm_coord_tr(tex);
	glVertex2f(tile->x + tile->w, tile->y);
	glEnd();
}

void dispatch_drawing(struct tile *tile)
{
	void draw_animated_tile(struct tile*);
	switch (tile->type) {
	case Static:
		draw_simple_tile(tile, tile->img_x, tile->img_y); break;
	case Animated:
		draw_animated_tile(tile); break;
	}
}

void draw_animated_tile(struct tile *tile)
{
	size_t img_x = tile->img_x + tile->dyn.cur_tex * tile->w;
	draw_simple_tile(tile, img_x, tile->img_y);
}

/* Animators called from cg */

void cg_animate_fan(struct tile *tile, double time)
{
	int phase = round(time * 1000 / FAN_ANIM_INTERVAL);
	tile->dyn.cur_tex = fan_anim_order[phase % 3];
}
void cg_animate_magnet(struct tile *tile, double time)
{
	int phase = round(time * 1000 / MAGNET_ANIM_INTERVAL);
	tile->dyn.cur_tex = magnet_anim_order[phase % 4];
}
void cg_animate_airgen(struct tile *tile, double time)
{
	int phase = round(time * 1000 / AIRGEN_ANIM_INTERVAL);
	tile->dyn.cur_tex = airgen_anim_order[phase % 8];
}
