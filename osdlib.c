/* osdlib.c - simple OSD widget library used by FreeCG - main source
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

#include "osdlib.h"
#include "graphics.h"
#include "texmgr.h"
#include <stdarg.h>
#include <float.h>
#include <assert.h>
#include <math.h>

/* ==================== Positioning ==================== */
/* General relative coordinate constructor
 * rel  - relation to parent/sibling
 * orig - relation to self
 */
struct coord c(enum side rel, enum side orig, double val)
{
	struct coord r = {
		.rel = rel,
		.orig = orig,
		.v = val
	};
	return r;
}
/* relative to sibling with margin */
struct coord margin(enum side s, double m)
{
	assert(s != Center);
	if (s == Begin)
		return c(Begin, End,  -m);
	else
		return c(End,   Begin, m);
}
struct coord pad(enum side s, double m)
{
	assert(s != Center);
	if (s == Begin)
		return c(Begin, Begin, m);
	else
		return c(End,   End,  -m);
}
struct coord center()
{
	return c(C,C,0);
}

/* ==================== Element setters ==================== */
/* Inits an osd_element. Must be called on each root element */
void o_init(struct osd_element *e)
{
	e->tr  = Opaque;
	e->rx  = DBL_MAX;
	e->z   = 0;
	e->nch = 0;
	e->rel = NULL;
	e->parent = NULL;
}
void o_img(struct osd_element *e, struct texmgr *tm, double a,
		int x, int y, int w, int h)
{
	e->t = tm;
	e->a = a;
	e->tex_x = x, e->tex_y = y;
	e->tex_w = w, e->tex_h = h;
}
void o_pos(struct osd_element *e, struct osd_element *rel,
		struct coord x, struct coord y)
{
	e->x = x, e->y = y;
	e->rel = rel;
}
void o_dim(struct osd_element *e, double w, double h,
		enum transparency_model tr)
{
	e->w = w, e->h = h;
	e->tr = tr;
}
void o_set(struct osd_element *e, struct osd_element *rel,
		struct coord x, struct coord y,
		double w, double h, enum transparency_model tr)
{
	o_pos(e, rel, x, y);
	o_dim(e, w, h, tr);
}

/* ==================== Osdlib functions ==================== */
void osdlib_init(struct osd_layer *layer, double w, double h)
{
	layer->w = w, layer->h = h;
	layer->root = calloc(1, sizeof(*layer->root));
	o_init(layer->root); o_set(layer->root, NULL, pad(L,0), pad(T,0), 0, 0, TE);
	layer->animation_list = NULL;
}

/* Creates children positioned relatively to the parent and inits them */
void osdlib_make_children(struct osd_element *e, size_t num, int init, ...)
{
	e->nch = num;
	e->ch  = calloc(num, sizeof(*e->ch));
	for (size_t i = 0; i < num; ++i) {
		o_init(&e->ch[i]);
		e->ch[i].parent = e;
	}
	if (!init)
		return;
	va_list ptrs;
	va_start(ptrs, init);
	for (size_t i = 0; i < num; ++i)
		*va_arg(ptrs, struct osd_element**) = &e->ch[i];
	va_end(ptrs);
}
void osdlib_step(struct osd_layer *l, double time)
{
	extern void osdlib_animations_step(struct osd_layer*, double);
	osdlib_animations_step(l, time);
	l->time = time;
}
double relative_dimension(double dim, double pdim)
{
	if (dim > 0)
		return dim;
	else
		return pdim + dim;
}
double relative_coord(struct coord coord, double dim,
		double pcoord, double pdim)
{
	double rel;
	rel = pcoord;
	if (coord.rel == End)
		rel += pdim;
	else if (coord.rel == Center)
		rel += pdim/2;
	if (coord.orig == End)
		rel -= dim;
	else if (coord.orig == Center)
		rel -= dim/2;
	return rel + coord.v;
}
void osdlib_count_absolute(struct osd_layer *l, struct osd_element *e)
{
	if (e->rx != DBL_MAX)
		return;
	double pw, ph, px, py, pz;
	if (e->parent) {
		osdlib_count_absolute(l, e->parent);
		pw = e->parent->rw, ph = e->parent->rh;
		if (e->rel) {
			osdlib_count_absolute(l, e->rel);
			px = e->rel->rx, py = e->rel->ry;
			pz = e->rel->rz;
		} else {
			px = e->parent->rx, py = e->parent->ry;
			pz = e->parent->rz + 0.01;
		}
	} else {
		pw = l->w, ph = l->h;
		px = 0, py = 0;
		pz = 0;
	}
	e->rw = relative_dimension(e->w, pw);
	e->rh = relative_dimension(e->h, ph);
	e->rx = relative_coord(e->x, e->rw, px, e->rel ? e->rel->rw : pw);
	e->ry = relative_coord(e->y, e->rh, py, e->rel ? e->rel->rh : ph);
	e->rz = pz + e->z;
	/* FIXME: consider a */
}
void osdlib_draw_rec(struct osd_layer *l, struct osd_element *e)
{
	osdlib_count_absolute(l, e);
	if (e->tr == Opaque) {
		gl_bind_texture(e->t);
		glBegin(GL_QUADS);
		glColor4f(1, 1, 1, e->a);
		tm_coord_tl(e->t, e->tex_x, e->tex_y,
				e->tex_w, e->tex_h);
		glVertex3d(e->rx, e->ry, e->rz);
		tm_coord_bl(e->t, e->tex_x, e->tex_y,
				e->tex_w, e->tex_h);
		glVertex3d(e->rx, e->ry + e->rh, e->rz);
		tm_coord_br(e->t, e->tex_x, e->tex_y,
				e->tex_w, e->tex_h);
		glVertex3d(e->rx + e->rw, e->ry + e->rh, e->rz);
		tm_coord_tr(e->t, e->tex_x, e->tex_y,
				e->tex_w, e->tex_h);
		glVertex3d(e->rx + e->rw, e->ry, e->rz);
		glEnd();
	}
	if (e->tr != TransparentSubtree) {
		/* recurse into the children */
		for (size_t i = 0; i < e->nch; ++i)
			osdlib_draw_rec(l, &e->ch[i]);
	}
}
void mark_as_unvisited(struct osd_element *e)
{
	e->rx = DBL_MAX;
	for (size_t i = 0; i < e->nch; ++i)
		mark_as_unvisited(&e->ch[i]);
}
void osdlib_draw(struct osd_layer *l)
{
	mark_as_unvisited(l->root);
	osdlib_draw_rec(l, l->root);
}

void osdlib_free_rec(struct osd_element *e)
{
	for (size_t i = 0; i < e->nch; ++i)
		osdlib_free_rec(&e->ch[i]);
	if (e->nch > 0)
		free(e->ch);
}
void osdlib_free(struct osd_layer *l)
{
	osdlib_free_rec(l->root);
	free(l->root);
	free(l);
}

/* ==================== Animations ==================== */
void osdlib_add_animation(struct osd_layer *l, struct animation *anim)
{
	anim->next = l->animation_list;
	l->animation_list = anim;
}
void animation_step(struct animation *anim, double time)
{
	if (!anim->running || time < anim->time_start)
		return;
	if (time > anim->time_end) {
		*anim->val = anim->val_end;
		anim->running = 0;
		return;
	}
	double length = anim->time_end - anim->time_start,
	       delta  = anim->val_end - anim->val_start,
	       phase  = time - anim->time_start;
	*anim->val = anim->val_start + delta * anim->e(phase/length);
}
void osdlib_animations_step(struct osd_layer *l, double time)
{
	struct animation dummy = {
		.next = l->animation_list
	};
	struct animation *cur  = dummy.next,
			 *prev = &dummy;
	while (cur != NULL) {
		animation_step(cur, time);
		if (!cur->running) {
			prev->next = cur->next;
			free(cur);
			cur = prev->next;
		} else {
			prev = cur;
			cur = cur->next;
		}
	}
	l->animation_list = dummy.next;
}
struct animation *anim(enum anim_mode mode, double *var, ease_function e,
		double vstart, double vend, double tstart, double tend)
{
	struct animation *a = calloc(1, sizeof(*a));
	a->val = var;
	a->val_start  = vstart, a->val_end  = vend;
	a->time_start = tstart, a->time_end = tend;
	a->e = e;
	a->mode = mode;
	a->running = 1;
	return a;
}

/* ==================== Higher level functions ==================== */
void o_txt(struct osd_element *e, const struct osdlib_font *font,
		const char *str)
{
	osdlib_free_rec(e);
	size_t len = strlen(str);
	o_dim(e, len*font->w, font->h, TE);
	osdlib_make_children(e, len, 0);
	struct osd_element *chr = e->ch;
	o_set(&chr[0], NULL, pad(L,0), pad(T,0), font->w, font->h, O);
	for (size_t i = 1; i < len; ++i)
		o_set(&chr[i], &chr[i-1], margin(R,0), pad(T,0),
				font->w, font->h, O);
	for (size_t i = 0; i < len; ++i) {
		int tx = font->tex_x + font->w*(str[i] - font->offset);
		o_img(&chr[i], font->tm, 1.0, tx, font->tex_y,
				font->w, font->h);
	}
}

/* ==================== Ease functions ==================== */
double ease_sin(double x)
{
	return sin(M_PI*(x-0.5))/2 + 0.5;
}
double ease_linear(double x)
{
	return x;
}
double ease_atan(double x)
{
	const double coef = 7;
	return atan((x-0.5)*coef)/atan(coef/2)/2 + 0.5;
}
