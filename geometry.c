/* geometry.c - basig geometry routines
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

#include "mathgeom.h"
#include "gfx.h"
#include "cg.h"
#include <math.h>

int normalize_angle(double *a)
{
	if (*a < 0)
		*a += 2*M_PI;
	if (*a >= 2*M_PI)
		*a -= 2*M_PI;
	assert(*a < 2*M_PI);
	assert(*a >= 0);
	return 0;
}

void ship_to_tile(const struct ship *s, struct tile *t)
{
	t->w = SHIP_W, t->h = SHIP_H;
	t->x = (int)round(s->x), t->y = (int)round(s->y);
	if (s->engine) {
		t->tex_x = SHIP_ON_IMG_X;
		t->tex_y = SHIP_ON_IMG_Y;
	} else {
		t->tex_x = SHIP_OFF_IMG_X;
		t->tex_y = SHIP_OFF_IMG_Y;
	}
	double drot = s->rot + M_PI/2;
	normalize_angle(&drot);
	int rot = drot / (2*M_PI) * 24;
	t->tex_x += rot * SHIP_W;
}

int tiles_intersect(const struct tile *t1, const struct tile *t2,
		struct rect *r)
{
	r->x = max(t1->x, t2->x);
	r->y = max(t1->y, t2->y);
	int x2 = min(t1->x + t1->w, t2->x + t2->w),
	    y2 = min(t1->y + t1->h, t2->y + t2->h);
	int w = x2 - r->x,
	    h = y2 - r->y;
	r->w = w;
	r->h = h;
	if (w > 0 && h > 0)
		return 1;
	else
		return 0;
}

void rect_to_tile(const struct rect *r, struct tile *t)
{
	t->x = r->x, t->y = r->y;
	t->w = r->w, t->h = r->h;
}

int discrete_rot(double rot)
{
	return (int)(rot/(2*M_PI) * 360) / 15;
}
