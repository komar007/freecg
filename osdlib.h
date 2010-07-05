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

#include <stdlib.h>
#include <float.h>

enum transparency_model {
	Opaque = 0,
	TransparentElement,
	TransparentSubtree
};
enum side {
	Begin = 0,	/* top or left */
	End		/* bottom or right */
};
struct coord {
	enum side orig,
		  rel;
	double v;
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
	struct texmgr *t;
	int tex_x, tex_y;
	int w, h;
	int offset;
};
void osdlib_draw(struct osd_element*);
void osdlib_make_text(struct osd_element*, const struct osdlib_font*, const char*);
void center_on_screen(struct osd_element*);

/* create absolutely positioned element (relatively to the parent) */
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
	e->rx = e->ry = DBL_MAX;
	e->nch = 0;
	e->ch = NULL;
	e->rel = NULL;
	e->tr = tr;
	e->z = 0;
}

/* create element relatively positioned to the sibling */
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

void osdlib_make_children(struct osd_element*, size_t, int, ...);

enum shortcuts {
	O = Opaque,
	T = TransparentElement,
	TS = TransparentSubtree,
	B = Begin,
	E = End
};

#endif
