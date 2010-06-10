#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "cg.h"
#include "cgl.h"
#include "basic_types.h"
#include <assert.h>

inline int max(int a, int b)
{
	return a > b ? a : b;
}
inline int min(int a, int b)
{
	return a < b ? a : b;
}
inline int sgn(double a)
{
	return a < 0 ? -1 : a == 0 ? 0 : 1;
}
inline int rand_range(int min_n, int max_n)
{
	assert(min_n <= max_n);
	return rand() % (max_n - min_n + 1) + min_n;
}
inline int rand_sign()
{
	return 2 * rand_range(0, 1) - 1;
}

void ship_to_tile(const struct ship*, struct tile*);
int tiles_intersect(const struct tile*, const struct tile*, struct rect*);

#endif