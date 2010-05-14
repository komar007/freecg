#ifndef CG_H
#define CG_H

#include "cgl.h"

struct rocket {
	double x, y;
	double vx, vy;
	double ax, ay;
};

struct cg {
	struct cgl *level;
	struct rocket *rocket;
};

struct cg *cg_init(struct cgl *level);

#endif
