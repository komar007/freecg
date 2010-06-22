#ifndef OSDLIB_H
#define OSDLIB_H

#include <stdlib.h>

struct osd_element {
	int rel;
	int x, y;
	int w, h;
	double z;
	double a;
	int texrel;
	int tex_x, tex_y;
	int tex_w, tex_h;
	struct texmgr *t;
};
void osdlib_draw(const struct osd_element*, size_t);

inline struct osd_element _o(int rel, double x, double y, double w, double h, double z,
		double a, int texrel, double tx, double ty, double tw,
		double th, struct texmgr *t)
{
	struct osd_element e;
	e.rel = rel, e.x = x, e.y = y, e.w = w, e.h = h, e.z = z, e.a = a,
		e.texrel = texrel, e.tex_x = tx, e.tex_y = ty, e.tex_w = tw,
		e.tex_h = th, e.t = t;
	return e;
}

#endif
