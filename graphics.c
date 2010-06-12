#include "graphics.h"
#include "mathgeom.h"
#include "texmgr.h"
#include <assert.h>
#include <math.h>

struct glengine gl;
void gl_draw_sprite(double, double, const struct tile*, const struct texture*);

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
	extern void gl_dispatch_drawing(const struct tile*);
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
	struct texture *tex = tm_request_texture(&tile);
	gl_draw_sprite(gl.cg->ship->x, gl.cg->ship->y, &tile, tex);
}

/* this function uses x and y as coordinates instead of tile's x and y, to
 * support subpixel rendering */
void gl_draw_sprite(double x, double y, const struct tile *tile,
		const struct texture *tex)
{
	glBindTexture(GL_TEXTURE_2D, tex->no);
	glBegin(GL_QUADS);
	tm_coord_tl(tex);
	glVertex2f(x, y);
	tm_coord_bl(tex);
	glVertex2f(x, y + tile->h);
	tm_coord_br(tex);
	glVertex2f(x + tile->w, y + tile->h);
	tm_coord_tr(tex);
	glVertex2f(x + tile->w, y);
	glEnd();
}

inline void gl_draw_simple_tile(const struct tile *tile,
		const struct texture *tex)
{
	gl_draw_sprite(tile->x, tile->y, tile, tex);
}
inline void gl_draw_blinking_tile(const struct tile *tile,
		const struct texture *tex)
{
	int phase = round(gl.cg->time * BLINK_SPEED);
	if (phase % 2 == 0)
		gl_draw_sprite(tile->x, tile->y, tile, tex);
}

void gl_dispatch_drawing(const struct tile *tile)
{
	struct texture *tex;
	switch (tile->type) {
	case Transparent:
		break;
	case Simple:
		tex = tm_request_texture(tile);
		gl_draw_simple_tile(tile, tex);
		break;
	case Blink:
		tex = tm_request_texture(tile);
		gl_draw_blinking_tile(tile, tex);
	}
}

/* Animators */
static const int magnet_anim_order[] = {0, 1, 2, 1};
static const int fan_anim_order[] = {0, 1, 2};
static const int airgen_anim_order[] = {0, 1, 2, 3, 4, 5, 6, 7};
static const int bar_anim_order[][2] = {{0, 1}, {1, 0}};

void animate_fan(struct fan *fan, double time)
{
	int phase = round(time * FAN_ANIM_SPEED);
	int cur_tex = fan_anim_order[phase % 3];
	fan->base->tex_x = fan->tex_x + cur_tex * fan->base->w;
}
void animate_magnet(struct magnet *magnet, double time)
{
	int phase = round(time * MAGNET_ANIM_SPEED);
	int cur_tex = magnet_anim_order[phase % 4];
	magnet->magn->tex_x = magnet->tex_x + cur_tex * magnet->magn->w;
}
void animate_airgen(struct airgen *airgen, double time)
{
	int phase = round(time * AIRGEN_ANIM_SPEED);
	int cur_tex = airgen_anim_order[phase % 8];
	airgen->base->tex_x = airgen->tex_x + cur_tex * airgen->base->w;
}
void animate_bar(struct bar *bar, double time)
{
	int phase = round(time * BAR_ANIM_SPEED);
	int cur_tex = bar_anim_order[0][phase % 2];
	bar->beg->tex_x = bar->btex_x + cur_tex * BAR_TEX_OFFSET;
	cur_tex = bar_anim_order[1][phase % 2];
	bar->end->tex_x = bar->etex_x + cur_tex * BAR_TEX_OFFSET;
}
