#include "graphics.h"
#include "geometry.h"
#include "texmgr.h"
#include <assert.h>
#include <math.h>

struct glengine gl;
void draw_sprite(double, double, double, double, int, int);

void gl_init(struct cg* cg)
{
	gl.frame = 0;
	gl.cg = cg;
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
	extern void draw_ship(void);
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
	draw_ship();
	SDL_GL_SwapBuffers();
	gl.frame++;
}

void fix_lframes(struct cgl *level)
{
	for (size_t i = 0; i < level->ntiles; ++i)
		level->tiles[i].lframe = 0;
	gl.frame = 1;
}
/* Each tile may be referenced by many blocks. This function makes sure each
 * tile is drawn to the buffer only once */
void draw_block(struct tile *tiles[])
{
	extern void dispatch_drawing(struct tile*);
	for (size_t i = 0; tiles[i]; ++i) {
		/* if the tile has not been drawn in current frame yet, draw
		 * and update tile's frame number */
		if (tiles[i]->lframe != gl.frame) {
			dispatch_drawing(tiles[i]);
			tiles[i]->lframe = gl.frame;
		}
	}
}

void draw_ship(void)
{
	struct tile tile;
	ship_to_tile(gl.cg->ship, &tile); /* to get tex coordinates */
	draw_sprite(gl.cg->ship->x, gl.cg->ship->y,
			SHIP_W, SHIP_H, tile.img_x, tile.img_y);
}

void draw_sprite(double x, double y, double w, double h, int img_x, int img_y)
{
	struct texture *tex = tm_request_texture(img_x, img_y, (int)w, (int)h);
	glBindTexture(GL_TEXTURE_2D, tex->no);
	glBegin(GL_QUADS);
	tm_coord_tl(tex);
	glVertex2f(x, y);
	tm_coord_bl(tex);
	glVertex2f(x, y + h);
	tm_coord_br(tex);
	glVertex2f(x + w, y + h);
	tm_coord_tr(tex);
	glVertex2f(x + w, y);
	glEnd();
}

void draw_simple_tile(struct tile *tile, int img_x, int img_y)
{
	draw_sprite(tile->x, tile->y, tile->w, tile->h, img_x, img_y);
}

void dispatch_drawing(struct tile *tile)
{
	switch (tile->type) {
	case Simple:
		draw_simple_tile(tile, tile->img_x, tile->img_y); break;
	}
}

/* Animators called from cg */

void cg_animate_fan(struct fan *fan, double time)
{
	int phase = round(time * 1000 / FAN_ANIM_INTERVAL);
	int cur_tex = fan_anim_order[phase % 3];
	fan->base->img_x = fan->img_x + cur_tex * fan->base->w;
}
void cg_animate_magnet(struct magnet *magnet, double time)
{
	int phase = round(time * 1000 / MAGNET_ANIM_INTERVAL);
	int cur_tex = magnet_anim_order[phase % 4];
	magnet->magn->img_x = magnet->img_x + cur_tex * magnet->magn->w;
}
void cg_animate_airgen(struct airgen *airgen, double time)
{
	int phase = round(time * 1000 / AIRGEN_ANIM_INTERVAL);
	int cur_tex = airgen_anim_order[phase % 8];
	airgen->base->img_x = airgen->img_x + cur_tex * airgen->base->w;
}
