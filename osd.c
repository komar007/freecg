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
	o_img(&f->bars[15], gl.ttm, 0.2, 388, 387, 16, 3);
	for (int i = 14; i >= 0; --i)
		o_set(&f->bars[i], &f->bars[i+1], pad(L,0), margin(B,1),
				16, 3, O);
	for (int i = 14; i >= 10; --i)
		o_img(&f->bars[i], gl.ttm, 0.2, 388, 387, 16, 3);
	for (int i = 9; i >= 4; --i)
		o_img(&f->bars[i], gl.ttm, 0.2, 388, 390, 16, 3);
	for (int i = 3; i >= 0; --i)
		o_img(&f->bars[i], gl.ttm, 0.2, 388, 393, 16, 3);
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
	int cont_width = 5 + 44;
	o_dim(container, cont_width, 18, TransparentElement);
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
	for (size_t i = 0; i < l->max_life; ++i) {
		o_set(&l->ships[i], NULL, pad(R, (signed)i*(SHIP_W-10)), pad(T,0),
				SHIP_W, SHIP_H, O);
		o_img(&l->ships[i], gl.ttm, 0,
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
	osd.visible = 0;
	struct osd_element *orect, *opanel, *otimer, *ogameover, *ovictory;
	osd.layer = calloc(1, sizeof(*osd.layer));
	osdlib_init(osd.layer, gl.win_w, gl.win_h);
	osdlib_make_children(osd.layer->root, 5, 1,
		&orect, &opanel, &otimer, &ogameover, &ovictory);
	osd.shipinfo.container = orect;
	osd.panel.container = opanel;
	osd.timer.container = otimer;
	/* left rect */
	o_set(orect, NULL, pad(L,0), pad(B,80)/*hidden*/, 151, 80, O);
	o_img(orect, gl.otm, 1.0, 0, 0, 151, 80);
	struct osd_element *fuel_cont, *cross, *key_cont;
	osdlib_make_children(orect, 3, 1, &fuel_cont, &cross, &key_cont);
	o_pos(fuel_cont, NULL, pad(L,8), pad(T,10));
	osd_fuel_init(&osd.shipinfo.fuel, fuel_cont);
	o_pos(cross, NULL, center(), pad(T,8));
	osd_velocity_init(&osd.shipinfo.velocity, cross);
	o_pos(key_cont, NULL, pad(R,14), pad(T,8));
	osd_keys_init(&osd.shipinfo.keys, key_cont);
	/* panel */
	o_set(opanel, orect, margin(R,0), pad(B,0), -151, 32, O);
	o_img(opanel, gl.otm, 1, 152, 48, 2, 32);
	struct osd_element *lfreight, *sfreight, *hbfreight, *life;
	osdlib_make_children(opanel, 4, 1,
			&lfreight, &sfreight, &hbfreight, &life);
	o_pos(lfreight, NULL, pad(L,8), c(C,C,1));
	osd_freight_init(&osd.panel.lfreight, lfreight, 384, 400);
	o_pos(sfreight, lfreight, margin(R,12),pad(T,0));
	osd_freight_init(&osd.panel.sfreight, sfreight, 432, 400);
	o_pos(hbfreight, sfreight, margin(R,12), pad(T,0));
	osd_freight_init(&osd.panel.hbfreight, hbfreight, 480, 400);
	o_pos(life, NULL, pad(R,8), pad(T,6));
	life->z = 0.01;
	osd_life_init(&osd.panel.life, life);
	/* timer */
	o_pos(otimer, NULL, center(), pad(T,-32));
	osd_timer_init(&osd.timer, otimer, 96);
	osd_show();

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
	size_t nfuel = (size_t)ceil(fuel);
	double t = osd.layer->time;
	if (nfuel == f->old_nfuel)
		return;
	else if (nfuel > f->old_nfuel)
		for (size_t i = f->old_nfuel; i < nfuel; ++i) {
			int dt = i - f->old_nfuel;
			struct animation *a = anim(Rel, Abs,
					&f->bars[i].a, ease_atan,
					0, 0.8, t+0.125*dt, t+0.125*dt+0.25);
			osdlib_add_animation(osd.layer, a);
		}
	else
		for (size_t i = nfuel; i < f->old_nfuel; ++i) {
			int dt = i - nfuel;
			struct animation *a = anim(Rel, Abs,
					&f->bars[i].a, ease_atan,
					0, 0.2, t+0.125*dt, t+0.125*dt+0.5);
			osdlib_add_animation(osd.layer, a);
		}
	f->old_nfuel = nfuel;
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
	if (f->max_freight > f->old_max_freight) {
		double t = osd.layer->time;
		int shelf_delta = (f->max_freight-f->old_max_freight)*(16+3);
		struct animation *a = anim(Rel, Rel, &f->container->w, ease_atan,
				0, shelf_delta, t, t+0.5);
		osdlib_add_animation(osd.layer, a);
		f->old_max_freight = f->max_freight;
	}
}
void osd_life_step(struct osd_life *l, size_t life)
{
	double t = osd.layer->time;
	if (life == l->old_life)
		return;
	else if (life > l->old_life)
		for (size_t i = l->old_life; i < life; ++i) {
			int dt = i - l->old_life;
			struct animation *a = anim(Rel, Abs,
					&l->ships[i].a, ease_atan,
					0, 0.5, t+0.25*dt, t+0.25*dt+0.25);
			osdlib_add_animation(osd.layer, a);
			l->ships[i].a = 0;
			a = anim(Abs, Rel, &l->ships[i].x.v, ease_atan,
					0, 0, t+0.25*dt, t+0.25*dt+0.25);
			osdlib_add_animation(osd.layer, a);
		}
	else
		for (size_t i = life; i < l->old_life; ++i) {
			int dt = i - life;
			struct animation *a = anim(Rel, Abs,
					&l->ships[i].a, ease_atan,
					0, 0, t+0.125*dt, t+0.125*dt+0.5);
			osdlib_add_animation(osd.layer, a);
		}
	l->old_life = life;
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
	osd_fuel_step(&osd.shipinfo.fuel, ship->fuel);
	osd_velocity_step(&osd.shipinfo.velocity, ship->vx, ship->vy,
			ship->max_vx, ship->max_vy);
	osd_keys_step(&osd.shipinfo.keys, ship->keys);
	size_t nfreight = cg_freight_remaining(gl.l);
	struct freight freight[nfreight];
	cg_get_freight_airports(gl.l, freight);
	osd_freight_step(&osd.panel.lfreight, freight, nfreight);
	osd.panel.sfreight.max_freight = ship->max_freight;
	osd_freight_step(&osd.panel.sfreight, ship->freight, ship->num_freight);
	struct airport *hb = gl.l->hb;
	osd_freight_step(&osd.panel.hbfreight, hb->c.freight, hb->num_cargo);
	osd_life_step(&osd.panel.life, max(0, gl.l->ship->life));
	osd_timer_step(&osd.timer, time);
	if (gl.l->status == Victory)
		osd.victory->tr = Opaque;
	if (gl.l->status == Lost)
		osd.gameover->tr = Opaque;
	osdlib_step(osd.layer, time);
}
void osd_draw()
{
	osdlib_draw(osd.layer);
}

void osd_free()
{
	osdlib_free(osd.layer);
}
void osd_show()
{
	if (osd.visible)
		return;
	double t = osd.layer->time;
	struct animation *a;
	a = anim(Abs, Abs, &osd.shipinfo.container->y.v, ease_atan,
			osd.shipinfo.container->h, 0, t, t+0.5);
	osdlib_add_animation(osd.layer, a);
	a = anim(Abs, Abs, &osd.timer.container->y.v, ease_atan,
			-osd.timer.container->h, 0, t+0.25, t+0.75);
	osdlib_add_animation(osd.layer, a);
	osd.visible = 1;
}
void osd_hide()
{
	if (!osd.visible)
		return;
	double t = osd.layer->time;
	struct animation *a;
	a = anim(Abs, Abs, &osd.shipinfo.container->y.v, ease_atan,
			0, osd.shipinfo.container->h, t+0.25, t+0.75);
	osdlib_add_animation(osd.layer, a);
	a = anim(Abs, Abs, &osd.timer.container->y.v, ease_atan,
			0, -osd.timer.container->h, t, t+0.5);
	osdlib_add_animation(osd.layer, a);
	osd.visible = 0;
}
void osd_toggle()
{
	if (osd.visible)
		osd_hide();
	else
		osd_show();
}
