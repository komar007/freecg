/* osd.c - On Screen Display routines used to draw all the indicators on top
 * of the game field
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
 * along with FreeCG; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "osd.h"
#include "graphics.h"
#include <math.h>

static struct cg_osd osd;

void osd_fuel_init(struct osd_fuel *f, struct osd_element *container,
		double x, double y)
{
	*container  = _o(x, y,  16, 64,  0.8,  0, 90,  1, 1,  1, gl.ttm);
	osdlib_make_children(container, 16, 0);
	f->bars = container->ch;
	f->bars[15] = _o(0, 0,  16, 3,  0.8,  388, 387,  16, 3, 0, gl.ttm);
	for (int i = 14; i >= 10; --i)
		f->bars[i] = _ro(&f->bars[i+1], 0, -1,  16, 3,  0.8,  388, 387,  16, 3, 0, gl.ttm);
	for (int i = 9; i >= 4; --i)
		f->bars[i] = _ro(&f->bars[i+1], 0, -1,  16, 3,  0.8,  388, 390,  16, 3, 0, gl.ttm);
	for (int i = 3; i >= 0; --i)
		f->bars[i] = _ro(&f->bars[i+1], 0, -1,  16, 3,  0.8,  388, 393,  16, 3, 0, gl.ttm);
}
void osd_velocity_init(struct osd_velocity *v, struct osd_element *container,
		double x, double y)
{
	*container = _o(x,  y,  64, 64,  0.8,    0, 400,  64, 64,  0, gl.ttm);
	osdlib_make_children(container, 2, 1, &v->xbar, &v->ybar);
	*v->xbar   = _o(31, 16,   2, 32,  0.8,  384, 366,   2, 32,  0, gl.ttm);
	*v->ybar   = _o(16, 31,  32,  2,  0.8,  384, 398,  32,  2,  0, gl.ttm);
}
void osd_keys_init(struct osd_keys *k, struct osd_element *container,
		double x, double y)
{
	*container = _o(x, y,  16, 64,  0.8,    0,  90,   1,  1,  1, gl.ttm);
	osdlib_make_children(container, 4, 0);
	k->keys = container->ch;
	for (int i = 0; i < 4; ++i)
		k->keys[i] = _o(0, 17*i,  16, 16,  0.2,  256, 360+16*i,  16, 16,  0, gl.ttm);
}
void osd_freigh_init(struct osd_freigh *f, struct osd_element *container,
		double x, double y, int tex_x, int tex_y)
{
	f->container = container;
	f->max_freigh = gl.cg->level->num_all_freigh;
	int shelf_pos = 44;
	struct osd_element *img, *shelf;
	/* containter's width is updated in real-time, thus w = 0 */
	*container = _o(x, y, 0, 18,  1,  0, 0, 0, 0, 1, gl.ttm);
	osdlib_make_children(container, 2, 1, &img, &shelf);
	*img = _o(0, 0,  36, 18,  0.8,  tex_x, tex_y,  48, 24,  0, gl.ttm);
	*shelf = _o(shelf_pos, 6, -shelf_pos, 12,  0.2,  4, 302, 1, 1, 0, gl.ttm);
	osdlib_make_children(shelf, f->max_freigh, 0);
	f->freigh = shelf->ch;
	f->freigh[0] = _o(4, -2,  16, 16,  0.8,  80, 392,  16, 16, 1, gl.ttm);
	for (size_t i = 1; i < f->max_freigh; ++i)
		f->freigh[i] = _ro(&f->freigh[i-1], -3, 0,  16, 16,  0.8,  80, 392,  16, 16, 1, gl.ttm);
}
void osd_init()
{
	osd.root = _o(0, 0, gl.win_w, gl.win_h, 1, 0, 0, 0, 0, 1, gl.ttm);
	osdlib_make_children(&osd.root, 3, 1, &osd.rect, &osd.panel, &osd.pause);
	/* left rect */
	*osd.rect = _o(0,  -.1,  144, 80,  0.8,  0, 90,  1, 1,  0, gl.ttm);
	struct osd_element *fuel_cont, *cross, *key_cont;
	osdlib_make_children(osd.rect, 3, 1, &fuel_cont, &cross, &key_cont);
	osd_fuel_init(&osd.fuel, fuel_cont, 12, 8);
	osd_velocity_init(&osd.velocity, cross, 40, 8);
	osd_keys_init(&osd.keys, key_cont, -12, 8);
	/* panel */
	*osd.panel = _o(142, -.1, -142, 32,  0.8,  0, 90,  1, 1,  0, gl.ttm);
	struct osd_element *lfreigh, *sfreigh, *hbfreigh;
	osdlib_make_children(osd.panel, 3, 1, &lfreigh, &sfreigh, &hbfreigh);
	osd_freigh_init(&osd.freigh_level, lfreigh, 8, 8, 384, 400);
	osd_freigh_init(&osd.freigh_ship, sfreigh, -12, 0, 432, 400);
	sfreigh->rel = lfreigh;
	osd_freigh_init(&osd.freigh_hb, hbfreigh, -12, 0, 480, 400);
	hbfreigh->rel = sfreigh;
	int pause_x = gl.win_w/2 - 264/2,
	    pause_y = gl.win_h/2 - 120/2;
	*osd.pause = _o(pause_x,pause_y, 264, 120, 0.8, 0, 90, 1, 1, 0, gl.ttm);
	struct osd_element *pause_img;
	osdlib_make_children(osd.pause, 1, 1, &pause_img);
	*pause_img = _o(48, 42, 168, 36, 0.55, 64, 408, 168, 36, 0, gl.ttm);
}

void osd_fuel_step(struct osd_fuel *f, double fuel)
{
	for (size_t i = 0; i < (size_t)ceil(fuel); ++i)
		f->bars[i].a = 0.8;
	for (size_t i = (size_t)ceil(fuel); i < 16; ++i)
		f->bars[i].a = 0.1;
}
void osd_velocity_step(struct osd_velocity *v, double vx, double vy)
{
	v->xbar->x = fmin(64, fmax(0, vx/3 + 31));
	v->ybar->y = fmin(64, fmax(0, vy/3 + 31));
}
void osd_keys_step(struct osd_keys *k, const int *keys)
{
	for (size_t i = 0; i < 4; ++i) {
		if (!keys[i]) {
			k->keys[i].a = 0.2;
			k->keys[i].tex_x = 256;
		} else {
			k->keys[i].tex_x = 256 + 16 * ((int)(gl.cg->time*KEY_ANIM_SPEED + i) % 8);
			k->keys[i].a = 0.8;
		}
	}
}
void osd_freigh_step(struct osd_freigh *f, const enum freigh *flist, size_t nfreigh)
{
	for (size_t i = 0; i < nfreigh; ++i) {
		f->freigh[i].transparent = 0;
		f->freigh[i].tex_x = 80 + 16*flist[i];
	}
	for (size_t i = nfreigh; i < f->max_freigh; ++i)
		f->freigh[i].transparent = 1;
	/* shelf length = number of freights + number of gaps + 2x margin(4) */
	int shelf_len = f->max_freigh*16 + (f->max_freigh-1)*3 + 8;
	f->container->w = shelf_len + 44;
}
void osd_step()
{
	struct ship *ship = gl.cg->ship;
	osd_fuel_step(&osd.fuel, ship->fuel);
	osd_velocity_step(&osd.velocity, ship->vx, ship->vy);
	osd_keys_step(&osd.keys, ship->keys);
	size_t nfreigh = cg_freigh_remaining(gl.cg);
	enum freigh freigh[nfreigh];
	size_t k = 0;
	for (size_t i = 0; i < gl.cg->level->nairports; ++i) {
		if (gl.cg->level->airports[i].type == Freigh) {
			struct airport *a = &gl.cg->level->airports[i];
			for (size_t j = 0; j < a->num_cargo; ++j)
				freigh[k++] = a->c.freigh[j];
		}
	}
	osd_freigh_step(&osd.freigh_level, freigh, nfreigh);
	osd.freigh_ship.max_freigh = ship->max_freigh;
	osd_freigh_step(&osd.freigh_ship, ship->freigh, ship->num_freigh);
	struct airport *hb = gl.cg->level->hb;
	osd_freigh_step(&osd.freigh_hb, hb->c.freigh, hb->num_cargo);
}
void osd_draw()
{
	osdlib_draw(&osd.root, 0, 0, gl.win_w, gl.win_h, 0);
}
