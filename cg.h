#ifndef CG_H
#define CG_H

#include "cgl.h"

struct ship {
	double x, y;
	double vx, vy;
	double ax, ay;
	int engine;
	unsigned char collision_map[2][SHIP_ANIM_LEN][SHIP_H][SHIP_W];
};

struct cg {
	struct cgl *level;
	struct ship *ship;
	double time;
};

struct cg *cg_init(struct cgl *level);
void cg_step(struct cg *cg, double dt);

#endif
