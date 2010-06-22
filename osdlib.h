#ifndef OSDLIB_H
#define OSDLIB_H

#include <stdlib.h>

struct osd_element {
	int transparent;
	int x, y;
	int w, h;
	double a;
	int tex_x, tex_y;
	int tex_w, tex_h;
	struct texmgr *t;
	size_t nch;
	struct osd_element *ch;
};
void osdlib_draw(const struct osd_element*, double, double, double, double, double);

inline struct osd_element _o(double x, double y, double w, double h,
		double a, double tx, double ty, double tw, double th,
		int tr, struct texmgr *t)
{
	struct osd_element e;
	e.x = x, e.y = y, e.w = w, e.h = h, e.a = a, e.tex_x = tx,
		e.tex_y = ty, e.tex_w = tw, e.tex_h = th, e.t = t;
	e.nch = 0;
	e.ch = NULL;
	e.transparent = tr;
	return e;
}

void osdlib_make_children(struct osd_element*, size_t, int, ...);

#endif
