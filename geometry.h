#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "cg.h"
#include "cgl.h"
#include "basic_types.h"

enum {
	SHIP_W = 23,
	SHIP_H = 23
};

void ship_to_tile(const struct ship*, struct tile*);
int tiles_intersect(const struct tile*, const struct tile*, struct rect*);

#endif
