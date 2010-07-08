/* osdlib.h - simple OSD widget library used by FreeCG - header file
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

#ifndef OSDLIB_H
#define OSDLIB_H

#include "mathgeom.h"
#include <stdlib.h>
#include <float.h>

enum transparency_model {
	Opaque = 0,
	TransparentElement,
	TransparentSubtree
};
enum side {
	Begin = 0,	/* top or left */
	Center,
	End		/* bottom or right */
};
struct coord {
	enum side orig,
		  rel;
	double v;
};
typedef double (*ease_function)(double);
struct animation {
	int running;
	double val_start, val_end;
	double time_start, time_end;
	ease_function e;
	double *val;
};
struct osd_element {
	enum transparency_model tr;
	struct coord x, y;
	double w, h;
	double a;
	double z;
	struct texmgr *t;
	int tex_x, tex_y;
	int tex_w, tex_h;
	size_t nch;
	struct osd_element *ch;
	struct osd_element *parent,
			   *rel;
	/* memoized x, y, w, h, z */
	double rx, ry;
	double rw, rh;
	double rz;
};
struct osdlib_font {
	struct texmgr *tm;
	int tex_x, tex_y;
	int w, h;
	size_t offset;
};
struct osd_layer {
	double w, h;
	struct osd_element *root;
};
struct coord c(enum side, enum side, double);
struct coord margin(enum side, double);
struct coord pad(enum side, double);
struct coord center();
void osdlib_init(struct osd_layer*, double, double);
void osdlib_draw(struct osd_layer*);
void osdlib_free(struct osd_layer*);
void osdlib_make_children(struct osd_element*, size_t, int, ...);
void o_init(struct osd_element*);
void o_img(struct osd_element*, struct texmgr*, double,
		int, int, int, int);
void o_pos(struct osd_element*, struct osd_element*, struct coord, struct coord);
void o_dim(struct osd_element*, double, double, enum transparency_model);
void o_set(struct osd_element*, struct osd_element*, struct coord, struct coord,
		double, double, enum transparency_model);
void o_txt(struct osd_element*, const struct osdlib_font*, const char*);

void animation_step(struct animation*, double);
double ease_sin(double x);
double ease_linear(double);
double ease_atan(double);

/* DEPRECATED create absolutely positioned element (relatively to the parent) */
static inline void _o(struct osd_element *e, double x, double y,
		double w, double h, double a, double tx, double ty,
		double tw, double th, enum transparency_model tr, struct texmgr *t)
{
	e->x.v = x, e->y.v = y, e->w = w, e->h = h, e->a = a, e->tex_x = tx,
		e->tex_y = ty, e->tex_w = tw, e->tex_h = th, e->t = t;
	if (x < 0) {
		e->x.orig = End;
		e->x.rel = End;
	}
	if (y < 0) {
		e->y.orig = End;
		e->y.rel = End;
	}
	e->tr = tr;
}

/* DEPRECATED create element relatively positioned to the sibling */
static inline void _ro(struct osd_element *e, struct osd_element *s,
		double x, double y, double w, double h, double a,
		double tx, double ty, double tw, double th,
		enum transparency_model tr, struct texmgr *t)
{
	_o(e, x, y, w, h, a, tx, ty, tw, th, tr, t);
	if (x < 0) {
		e->x.orig = Begin;
		e->x.rel = End;
		e->x.v = -e->x.v;
	}
	if (y < 0) {
		e->y.orig = Begin;
		e->y.rel = End;
		e->y.v = -e->y.v;
	}
	e->rel = s;
}

enum shortcuts {
       O = Opaque,
       TE = TransparentElement,
       TS = TransparentSubtree,
       L = Begin,
       T = Begin,
       C = Center,
       R = End,
       B = End,
};

#endif
