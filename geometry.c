#include "geometry.h"
#include "cg.h"
#include "cgl.h"
#include <math.h>

void ship_to_tile(const struct ship *s, struct tile *t)
{
	t->w = SHIP_W, t->h = SHIP_H;
	t->x = (int)s->x, t->y = (int)s->y;
	if (s->engine) {
		t->img_x = SHIP_ON_IMG_X;
		t->img_y = SHIP_ON_IMG_Y;
	} else {
		t->img_x = SHIP_OFF_IMG_X;
		t->img_y = SHIP_OFF_IMG_Y;
	}
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
