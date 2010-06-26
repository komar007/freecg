/* graphics.c - screen drawing routines
 * Copyright (C) 2010 Michal Trybus.
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
#include "mathgeom.h"
#include "texmgr.h"
#include <assert.h>
#include <math.h>

struct glengine gl;
void gl_draw_sprite(double, double, const struct tile*);

void gl_init(struct cgl* l, struct texmgr *ttm, struct texmgr *ftm)
{
	gl.ttm = ttm;
	gl.ftm = ftm;
	gl.frame = 0;
	gl.l = l;
	gl.cam.scale = 1;
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.1, 0.1, 0.1, 1);
	osd_init();
}
void gl_resize_viewport(double w, double h)
{
	gl.win_w = w, gl.win_h = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1, -1, 1);
	glOrtho(0, w, 0, h, -5, 5);
	glMatrixMode(GL_MODELVIEW);
}
void gl_look_at(double x, double y, double scale)
{
	gl.viewport.w = gl.win_w/scale;
	gl.viewport.h = gl.win_h/scale;
	gl.viewport.x = fmin(gl.l->width*BLOCK_SIZE - gl.viewport.w,
			fmax(0, x - gl.viewport.w/2));
	gl.viewport.y = fmin(gl.l->height*BLOCK_SIZE - gl.viewport.h,
			fmax(0, y - gl.viewport.h/2));
	glScalef(scale, scale, 1);
	glTranslated(-gl.viewport.x, -gl.viewport.y, 0);
}

void gl_draw_scene()
{
	extern void fix_lframes(struct cgl*),
	            gl_draw_block(struct tile *[]),
		    gl_draw_ship(void),
		    animate_tiles();
	animate_tiles();
	if (gl.frame == 0)
		fix_lframes(gl.l);
	double x1 = fmax(0, gl.viewport.x),
	       y1 = fmax(0, gl.viewport.y),
	       x2 = fmin(gl.viewport.x + gl.viewport.w,
			       gl.l->width * BLOCK_SIZE),
	       y2 = fmin(gl.viewport.y + gl.viewport.h,
			       gl.l->height * BLOCK_SIZE);
	glColor4f(1, 1, 1, 1);
	gl_bind_texture(gl.ttm);
	gl_draw_ship();
	glPushMatrix();
	glTranslated(0, 0, 0.1);
	glBegin(GL_QUADS);
	for (size_t j = y1/BLOCK_SIZE; j*BLOCK_SIZE < y2; ++j)
		for (size_t i = x1/BLOCK_SIZE; i*BLOCK_SIZE < x2; ++i)
			gl_draw_block(gl.l->blocks[j][i]);
	glEnd();
	glPopMatrix();
	gl.frame++;
}
void fix_lframes(struct cgl *level)
{
	for (size_t i = 0; i < level->ntiles; ++i)
		level->tiles[i].lframe = 0;
	gl.frame = 1;
}
void gl_draw_ship(void)
{
	struct tile tile;
	ship_to_tile(gl.l->ship, &tile); /* to get tex coordinates */
	glBegin(GL_QUADS);
	gl_draw_sprite(gl.l->ship->x, gl.l->ship->y, &tile);
	glEnd();
}
/* this function uses x and y as coordinates instead of tile's x and y, to
 * support subpixel rendering */
void gl_draw_sprite(double x, double y, const struct tile *tile)
{
	tm_coord_tl(gl.ttm, tile->tex_x, tile->tex_y, tile->w, tile->h);
	glVertex3d(x, y, tile->z);
	tm_coord_bl(gl.ttm, tile->tex_x, tile->tex_y, tile->w, tile->h);
	glVertex3d(x, y + tile->h, tile->z);
	tm_coord_br(gl.ttm, tile->tex_x, tile->tex_y, tile->w, tile->h);
	glVertex3d(x + tile->w, y + tile->h, tile->z);
	tm_coord_tr(gl.ttm, tile->tex_x, tile->tex_y, tile->w, tile->h);
	glVertex3d(x + tile->w, y, tile->z);
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
inline void gl_draw_simple_tile(const struct tile *tile)
{
	gl_draw_sprite(tile->x, tile->y, tile);
}
inline void gl_draw_blinking_tile(const struct tile *tile)
{
	int phase = round(gl.l->time * BLINK_SPEED);
	if (phase % 2 == 0)
		gl_draw_sprite(tile->x, tile->y, tile);
}
void gl_dispatch_drawing(const struct tile *tile)
{
	switch (tile->type) {
	case Transparent:
		break;
	case Simple:
		gl_draw_simple_tile(tile);
		break;
	case Blink:
		gl_draw_blinking_tile(tile);
	}
}

void gl_draw_osd()
{
	osd_step();
	osd_draw();
}

void gl_update_window()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gl_look_at(gl.cam.x, gl.cam.y, gl.cam.scale);
	gl_draw_scene();
	glLoadIdentity();
	glTranslated(0, 0, 2);
	gl_draw_osd();
	SDL_GL_SwapBuffers();
}

void animate_tiles()
{
	extern void animate_fan(struct fan*, double),
	            animate_magnet(struct magnet*, double),
	            animate_airgen(struct airgen*, double),
	            animate_bar(struct bar*, double),
	            animate_key(struct airport*, double);
	for (size_t i = 0; i < gl.l->nmagnets; ++i)
		animate_magnet(&gl.l->magnets[i], gl.l->time);
	for (size_t i = 0; i < gl.l->nfans; ++i)
		animate_fan(&gl.l->fans[i], gl.l->time);
	for (size_t i = 0; i < gl.l->nairgens; ++i)
		animate_airgen(&gl.l->airgens[i], gl.l->time);
	for (size_t i = 0; i < gl.l->nbars; ++i)
		animate_bar(&gl.l->bars[i], gl.l->time);
	for (size_t i = 0; i < gl.l->nairports; ++i)
		animate_key(&gl.l->airports[i], gl.l->time);
}
/* Animators */
static const int magnet_anim_order[] = {0, 1, 2, 1};
static const int fan_anim_order[] = {0, 1, 2};
static const int airgen_anim_order[] = {0, 1, 2, 3, 4, 5, 6, 7};
static const int bar_anim_order[][2] = {{0, 1}, {1, 0}};
static const int key_anim_order[] = {0, 1, 2, 3, 4, 5, 6, 7};
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
void animate_key(struct airport *airport, double time)
{
	if (airport->type != Key)
		return;
	int phase = round(time * KEY_ANIM_SPEED);
	int cur_tex = key_anim_order[phase % 8];
	airport->cargo[0]->tex_x = KEY_TEX_X + cur_tex * airport->cargo[0]->w;
}
