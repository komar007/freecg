#include "graphics.h"
#include "mathgeom.h"
#include "texmgr.h"
#include <assert.h>
#include <math.h>

struct glengine gl;
void gl_draw_sprite(double, double, double, double, int, int);

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

void animate_tiles()
{
	extern void animate_fan(struct fan*, double),
	            animate_magnet(struct magnet*, double),
	            animate_airgen(struct airgen*, double),
	            animate_bar(struct bar*, double);
	for (size_t i = 0; i < gl.cg->level->nmagnets; ++i)
		animate_magnet(&gl.cg->level->magnets[i], gl.cg->time);
	for (size_t i = 0; i < gl.cg->level->nfans; ++i)
		animate_fan(&gl.cg->level->fans[i], gl.cg->time);
	for (size_t i = 0; i < gl.cg->level->nairgens; ++i)
		animate_airgen(&gl.cg->level->airgens[i], gl.cg->time);
	for (size_t i = 0; i < gl.cg->level->nbars; ++i)
		animate_bar(&gl.cg->level->bars[i], gl.cg->time);
}

void gl_draw_scene()
{
	extern void fix_lframes(struct cgl*);
	extern void gl_draw_block(struct tile *[]);
	extern void gl_draw_ship(void);
	animate_tiles();
	if (gl.frame == 0)
		fix_lframes(gl.cg->level);
	double x1 = fmax(0, gl.viewport.x),
	       y1 = fmax(0, gl.viewport.y),
	       x2 = fmin(gl.viewport.x + gl.viewport.w,
			       gl.cg->level->width * BLOCK_SIZE),
	       y2 = fmin(gl.viewport.y + gl.viewport.h,
			       gl.cg->level->height * BLOCK_SIZE);
	struct cgl *l = gl.cg->level;
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
	for (size_t j = y1/BLOCK_SIZE; j*BLOCK_SIZE < y2; ++j)
		for (size_t i = x1/BLOCK_SIZE; i*BLOCK_SIZE < x2; ++i)
			gl_draw_block(l->blocks[j][i]);
	gl_draw_ship();
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
void gl_draw_block(struct tile *tiles[])
{
	extern void gl_dispatch_drawing(struct tile*);
	for (size_t i = 0; tiles[i]; ++i) {
		/* if the tile has not been drawn in current frame yet, draw
		 * and update tile's frame number */
		if (tiles[i]->lframe != gl.frame) {
			gl_dispatch_drawing(tiles[i]);
			tiles[i]->lframe = gl.frame;
		}
	}
}

void gl_draw_ship(void)
{
	struct tile tile;
	ship_to_tile(gl.cg->ship, &tile); /* to get tex coordinates */
	gl_draw_sprite(gl.cg->ship->x, gl.cg->ship->y, SHIP_W, SHIP_H,
			tile.img_x, tile.img_y);
}

void gl_draw_sprite(double x, double y, double w, double h, int img_x, int img_y)
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

void gl_draw_simple_tile(struct tile *tile, int img_x, int img_y)
{
	gl_draw_sprite(tile->x, tile->y, tile->w, tile->h, img_x, img_y);
}

void gl_dispatch_drawing(struct tile *tile)
{
	switch (tile->type) {
	case Simple:
		gl_draw_simple_tile(tile, tile->img_x, tile->img_y); break;
	}
}

/* Animators */
void animate_fan(struct fan *fan, double time)
{
	int phase = round(time * 1000 / FAN_ANIM_INTERVAL);
	int cur_tex = fan_anim_order[phase % 3];
	fan->base->img_x = fan->img_x + cur_tex * fan->base->w;
}
void animate_magnet(struct magnet *magnet, double time)
{
	int phase = round(time * 1000 / MAGNET_ANIM_INTERVAL);
	int cur_tex = magnet_anim_order[phase % 4];
	magnet->magn->img_x = magnet->img_x + cur_tex * magnet->magn->w;
}
void animate_airgen(struct airgen *airgen, double time)
{
	int phase = round(time * 1000 / AIRGEN_ANIM_INTERVAL);
	int cur_tex = airgen_anim_order[phase % 8];
	airgen->base->img_x = airgen->img_x + cur_tex * airgen->base->w;
}
void animate_bar(struct bar *bar, double time)
{
	int phase = round(time * 1000 / BAR_ANIM_INTERVAL);
	int cur_tex = bar_anim_order[0][phase % 2];
	bar->beg->img_x = bar->bimg_x + cur_tex * BAR_TEX_OFFSET;
	cur_tex = bar_anim_order[1][phase % 2];
	bar->end->img_x = bar->eimg_x + cur_tex * BAR_TEX_OFFSET;
}
