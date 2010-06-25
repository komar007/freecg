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

struct osd_element {
	int transparent;
	double x, y;
	double w, h;
	double a;
	int tex_x, tex_y;
	int tex_w, tex_h;
	struct texmgr *t;
	size_t nch;
	struct osd_element *ch;
	struct osd_element *rel;
	/* memoized x, y */
	double rx, ry;
};
void osdlib_draw(struct osd_element*, double, double, double, double, double);

/* create absolutely positioned element (relatively to the parent) */
static inline struct osd_element _o(double x, double y, double w, double h,
		double a, double tx, double ty, double tw, double th,
		int tr, struct texmgr *t)
{
	struct osd_element e;
	e.x = x, e.y = y, e.w = w, e.h = h, e.a = a, e.tex_x = tx,
		e.tex_y = ty, e.tex_w = tw, e.tex_h = th, e.t = t;
	e.rx = e.ry = DBL_MAX;
	e.nch = 0;
	e.ch = NULL;
	e.rel = NULL;
	e.transparent = tr;
	return e;
}

/* create element relatively positioned to the sibling */
static inline struct osd_element _ro(struct osd_element *s, double x, double y,
		double w, double h, double a, double tx, double ty,
		double tw, double th, int tr, struct texmgr *t)
{
	struct osd_element e = _o(x, y, w, h, a, tx, ty, tw, th, tr, t);
	e.rel = s;
	return e;
}

void osdlib_make_children(struct osd_element*, size_t, int, ...);

#endif
