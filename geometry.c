#include "opencg.h"
#include "cg.h"
#include "cgl.h"

void ship_to_tile(struct ship *s, struct tile *t)
{
	t->w = s->w, t->h = s->h;
	t->x = (int)s->x, t->y = (int)s->y;
	if (s->engine) {
		t->img_x = SHIP_ON_IMG_X;
		t->img_y = SHIP_ON_IMG_Y;
	} else {
		t->img_x = SHIP_OFF_IMG_X;
		t->img_y = SHIP_OFF_IMG_Y;
	}
}
