#ifndef MATHGEOM_H
#define MATHGEOM_H

/* mathgeom.h - simple math/geometry functions and the most basic geometry
 * data structures
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

#include <assert.h>
#include <stdlib.h>

#define ARRSZ(a) (sizeof(a)/sizeof(*(a)))
enum dir {
	Down = 0,
	Up,
	Left,
	Right
};
typedef struct vector {
	int x,
	    y;
} vector;
struct rect {
	int x, y;
	unsigned int w, h;
};
struct drect {
	double x, y;
	double w, h;
};

static inline int max(int a, int b)
{
	return a > b ? a : b;
}
static inline int min(int a, int b)
{
	return a < b ? a : b;
}
static inline int sgn(double a)
{
	return a < 0 ? -1 : a == 0 ? 0 : 1;
}
static inline int rand_range(int min_n, int max_n)
{
	assert(min_n <= max_n);
	return rand() % (max_n - min_n + 1) + min_n;
}
static inline int rand_sign()
{
	return 2 * rand_range(0, 1) - 1;
}

struct tile;
struct ship;
int normalize_angle(double*);
void ship_to_tile(const struct ship*, struct tile*);
int tiles_intersect(const struct tile*, const struct tile*, struct rect*);
int discrete_rot(double);
void rect_to_tile(const struct rect*, struct tile*);

#endif
