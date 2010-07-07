/* osd.c - On Screen Display routines used to draw all the indicators on top
 * of the game field
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

#include "osd.h"
#include "graphics.h"
#include <math.h>

static struct cg_osd osd;

void osd_fuel_init(struct osd_fuel *f, struct osd_element *container)
{
	o_dim(container, 16, 64, TransparentElement);
	osdlib_make_children(container, 16, 0);
	f->bars = container->ch;
	o_set(&f->bars[15], NULL, pad(L,0), pad(T,0), 16, 3, O);
	o_img(&f->bars[15], gl.ttm, 0.8, 388, 387, 16, 3);
	for (int i = 14; i >= 0; --i)
		o_set(&f->bars[i], &f->bars[i+1], pad(L,0), margin(B,1),
				16, 3, O);
	for (int i = 14; i >= 10; --i)
		o_img(&f->bars[i], gl.ttm, 0.8, 388, 387, 16, 3);
	for (int i = 9; i >= 4; --i)
		o_img(&f->bars[i], gl.ttm, 0.8, 388, 390, 16, 3);
	for (int i = 3; i >= 0; --i)
		o_img(&f->bars[i], gl.ttm, 0.8, 388, 393, 16, 3);
}
void osd_velocity_init(struct osd_velocity *v, struct osd_element *container)
{
	o_dim(container, 64, 64, Opaque);
	o_img(container, gl.ttm, 0.8, 0, 400, 64, 64);
	osdlib_make_children(container, 5, 1,
			&v->xbar, &v->ybar, &v->mxbar1, &v->mxbar2, &v->mybar);
	o_set(v->xbar,   NULL, center(), center(), 2, 32, O);
	o_set(v->ybar,   NULL, center(), center(), 32, 2, O);
	o_set(v->mxbar1, NULL, center(), center(), 2, 14, O);
	o_set(v->mxbar2, NULL, center(), center(), 2, 14, O);
	o_set(v->mybar,  NULL, center(), center(), 14, 2, O);
	o_img(v->xbar,   gl.ttm, 0.8, 384, 366, 2, 32);
	o_img(v->ybar,   gl.ttm, 0.8, 384, 398, 32, 2);
	o_img(v->mxbar1, gl.ttm, 0.8, 386, 382, 2, 14);
	o_img(v->mxbar2, gl.ttm, 0.8, 386, 382, 2, 14);
	o_img(v->mybar,  gl.ttm, 0.8, 386, 396, 14, 2);
}
void osd_keys_init(struct osd_keys *k, struct osd_element *container)
{
	o_dim(container, 16, 64, TransparentElement);
	osdlib_make_children(container, 4, 0);
	k->keys = container->ch;
	o_set(&k->keys[0], NULL, pad(L,0), pad(T,0), 16, 16, O);
	for (int i = 1; i < 4; ++i)
		o_set(&k->keys[i], &k->keys[i-1], pad(L,0), margin(B,1),
				16, 16, O);
	for (int i = 0; i < 4; ++i)
		o_img(&k->keys[i], gl.ttm, 0.2, 256, 360+16*i, 16, 16);
}
void osd_freight_init(struct osd_freight *f, struct osd_element *container,
		int tex_x, int tex_y)
{
	f->max_freight = gl.l->num_all_freight;
	f->container = container;
	struct osd_element *img, *shelf;
	/* containter's width is updated in real-time, thus w = 0 */
	o_dim(container, 0, 18, TransparentElement);
	osdlib_make_children(container, 2, 1, &img, &shelf);
	o_set(img, NULL, pad(L,0), pad(T,0), 36, 18, O);
	o_img(img, gl.ttm, 0.6, tex_x, tex_y, 48, 24);
	o_set(shelf, img, margin(R,8), pad(B,0), -44, 12, O);
	o_img(shelf, gl.ttm, 0.2, 4, 302, 1, 1);
	osdlib_make_children(shelf, f->max_freight, 0);
	f->freight = shelf->ch;
	o_set(&f->freight[0], NULL, pad(L,4), pad(B,2), 16, 16, TE);
	for (size_t i = 1; i < f->max_freight; ++i)
		o_set(&f->freight[i], &f->freight[i-1], margin(R,3), pad(T,0),
				16, 16, TE);
	for (size_t i = 0; i < f->max_freight; ++i)
		o_img(&f->freight[i], gl.ttm, 0.8, 80, 392, 16, 16);
}
void osd_life_init(struct osd_life *l, struct osd_element *container)
{
	l->max_life = gl.l->num_1ups + DEFAULT_LIFE;
	int cwidth  = (l->max_life-1)*14 + SHIP_W + 2*2;
	int cheight = SHIP_H;
	o_dim(container, cwidth, cheight, TransparentElement);
	osdlib_make_children(container, l->max_life, 0);
	l->ships = container->ch;
	o_set(&l->ships[0], NULL, pad(R,0), pad(T,0), SHIP_W, SHIP_H, O);
	for (size_t i = 1; i < l->max_life; ++i)
		o_set(&l->ships[i], &l->ships[i-1], margin(L,-10), pad(T,0),
				SHIP_W, SHIP_H, O);
	for (size_t i = 0; i < l->max_life; ++i) {
		o_img(&l->ships[i], gl.ttm, 0.5,
				SHIP_ON_IMG_X + 12*SHIP_W*(i%2), SHIP_ON_IMG_Y,
				SHIP_W, SHIP_H);
		l->ships[i].z = i*0.01;
	}
}
void osd_timer_init(struct osd_timer *t, struct osd_element *container, int w)
{
	struct osd_element *bleft, *bright, *bcenter;
	osdlib_make_children(container, 4, 1,
			&bleft, &bright, &bcenter, &t->time);
	o_dim(container, w, 24, TE);
	o_set(bleft,   NULL, pad(L,0), pad(T,0),   8, 24, O);
	o_set(bright,  NULL, pad(R,0), pad(T,0),   8, 24, O);
	o_set(bcenter, NULL, center(), pad(T,0), -16, 24, O);
	o_pos(t->time, NULL, center(), center());
	o_img(bleft,   gl.otm, 1.0,  0, 81, 8, 24);
	o_img(bcenter, gl.otm, 1.0,  8, 81, 1, 24);
	o_img(bright,  gl.otm, 1.0, 13, 81, 8, 24);
	o_txt(t->time, &osd.font, "18:12");
}
void osd_init()
{
	const struct osdlib_font f = {
		.tm = gl.ftm,
		.w  = 16,
		.h  = 16,
		.tex_x = 0,
		.tex_y = 0,
		.offset = 32
	};
	osd.font = f;
	struct osd_element *orect, *opanel, *otimer, *ogameover, *ovictory;
	osd.root = calloc(1, sizeof(*osd.root));
	o_init(osd.root); o_set(osd.root, NULL, pad(L,0), pad(T,0), 0, 0, TE);
	osdlib_make_children(osd.root, 5, 1,
		&orect, &opanel, &otimer, &ogameover, &ovictory);
	/* left rect */
	o_set(orect, NULL, pad(L,0), pad(B,0), 150, 80, O);
	o_img(orect, gl.otm, 1.0, 0, 0, 150, 80);
	struct osd_element *fuel_cont, *cross, *key_cont;
	osdlib_make_children(orect, 3, 1, &fuel_cont, &cross, &key_cont);
	o_pos(fuel_cont, NULL, pad(L,8), pad(T,10));
	osd_fuel_init(&osd.fuel, fuel_cont);
	o_pos(cross, NULL, center(), pad(T,8));
	osd_velocity_init(&osd.velocity, cross);
	o_pos(key_cont, NULL, pad(R,14), pad(T,8));
	osd_keys_init(&osd.keys, key_cont);
	/* panel */
	o_set(opanel, orect, margin(R,0), pad(B,0), -150, 32, O);
	o_img(opanel, gl.otm, 1, 152, 48, 2, 32);
	struct osd_element *lfreight, *sfreight, *hbfreight, *life;
	osdlib_make_children(opanel, 4, 1,
			&lfreight, &sfreight, &hbfreight, &life);
	o_pos(lfreight, NULL, pad(L,8), c(C,C,1));
	osd_freight_init(&osd.freight_level, lfreight, 384, 400);
	o_pos(sfreight, lfreight, margin(R,12),pad(T,0));
	osd_freight_init(&osd.freight_ship, sfreight, 432, 400);
	o_pos(hbfreight, sfreight, margin(R,12), pad(T,0));
	osd_freight_init(&osd.freight_hb, hbfreight, 480, 400);
	o_pos(life, NULL, pad(R,8), pad(T,6));
	osd_life_init(&osd.life, life);
	/* timer */
	o_pos(otimer, NULL, center(), pad(T,0));
	osd_timer_init(&osd.timer, otimer, 96);

	struct animation t = {
		.val_start = 32,
		.val_end = opanel->y.v,
		.time_start = 0,
		.time_end = 1.5,
		.val = &opanel->y.v,
		.e = ease_atan,
		.running = 1
	};
	osd.test_anim = t;

	/* DEPRECATED (labels will go to menu) */
	_o(ogameover, 0, 0, 160, 32, 0.8, 0, 90, 1, 1, TS, gl.ttm);
	o_pos(ogameover, NULL, center(), center());
	struct osd_element *gameover_img;
	osdlib_make_children(ogameover, 1, 1, &gameover_img);
	_o(gameover_img, 8, 8, 0, 0, 0.8, 0, 0, 0, 0, TE, gl.ttm);
	o_txt(gameover_img, &osd.font, "GAME OVER");
	_o(ovictory, 0, 0, 144, 32, 0.8, 0, 90, 1, 1, TS, gl.ttm);
	o_pos(ovictory, NULL, center(), center());
	struct osd_element *victory_img;
	osdlib_make_children(ovictory, 1, 1, &victory_img);
	_o(victory_img, 8, 8, 0, 0, 0.8, 0, 0, 0, 0, TE, gl.ttm);
	o_txt(victory_img, &osd.font, "VICTORY!");
	osd.victory = ovictory;
	osd.gameover = ogameover;
	/* /DEPRECATED */
}

void osd_fuel_step(struct osd_fuel *f, double fuel)
{
	for (size_t i = 0; i < (size_t)ceil(fuel); ++i)
		f->bars[i].a = 0.8;
	for (size_t i = (size_t)ceil(fuel); i < 16; ++i)
		f->bars[i].a = 0.1;
}
void osd_velocity_step(struct osd_velocity *v, double vx, double vy,
		double max_vx, double max_vy)
{
	v->xbar->x.v   = fmin(32, fmax(-32,  vx/3));
	v->ybar->y.v   = fmin(32, fmax(-32,  vy/3));
	v->mxbar1->x.v = fmin(32, fmax(-32,  max_vx/3));
	v->mxbar2->x.v = fmin(32, fmax(-32, -max_vx/3));
	v->mybar->y.v  = fmin(32, fmax(-32,  max_vy/3));
}
void osd_keys_step(struct osd_keys *k, const int *keys)
{
	for (size_t i = 0; i < 4; ++i) {
		if (!keys[i]) {
			k->keys[i].a = 0.2;
			k->keys[i].tex_x = 256;
		} else {
			k->keys[i].tex_x = 256 + 16 * ((int)(gl.l->time*KEY_ANIM_SPEED + i) % 8);
			k->keys[i].a = 0.8;
		}
	}
}
void osd_freight_step(struct osd_freight *f, const struct freight *flist, size_t nfreight)
{
	for (size_t i = 0; i < nfreight; ++i) {
		f->freight[i].tr = Opaque;
		f->freight[i].tex_x = 80 + 16*flist[i].f;
	}
	for (size_t i = nfreight; i < f->max_freight; ++i)
		f->freight[i].tr = TransparentElement;
	/* shelf length = number of freights + number of gaps + 2x margin(4) */
	int shelf_len = f->max_freight*16 + (f->max_freight-1)*3 + 8;
	f->container->w = shelf_len + 44;
}
void osd_life_step(struct osd_life *l, size_t life)
{
	for (size_t i = 0; i < life; ++i)
		l->ships[i].tr = Opaque;
	for (size_t i = life; i < l->max_life; ++i)
		l->ships[i].tr = TransparentElement;
}
void osd_timer_step(struct osd_timer *t, double time)
{
	char time_str[8];
	int min = (int)time / 60,
	    sec = (int)time % 60;
	sprintf(time_str, "%.2d:%.2d", min, sec);
	o_txt(t->time, &osd.font, time_str);
}
void osd_step(double time)
{
	struct ship *ship = gl.l->ship;
	osd_fuel_step(&osd.fuel, ship->fuel);
	osd_velocity_step(&osd.velocity, ship->vx, ship->vy,
			ship->max_vx, ship->max_vy);
	osd_keys_step(&osd.keys, ship->keys);
	size_t nfreight = cg_freight_remaining(gl.l);
	struct freight freight[nfreight];
	cg_get_freight_airports(gl.l, freight);
	osd_freight_step(&osd.freight_level, freight, nfreight);
	osd.freight_ship.max_freight = ship->max_freight;
	osd_freight_step(&osd.freight_ship, ship->freight, ship->num_freight);
	struct airport *hb = gl.l->hb;
	osd_freight_step(&osd.freight_hb, hb->c.freight, hb->num_cargo);
	osd_life_step(&osd.life, max(0, gl.l->ship->life));
	/* FIXME: use real time, not game time */
	osd_timer_step(&osd.timer, gl.l->time);
	if (gl.l->status == Victory)
		osd.victory->tr = Opaque;
	if (gl.l->status == Lost)
		osd.gameover->tr = Opaque;
	animation_step(&osd.test_anim, time);
}
void osd_draw()
{
	osdlib_draw(osd.root);
}

void osd_free()
{
	osdlib_free(osd.root);
}
