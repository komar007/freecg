#ifndef CG_H
#define CG_H

#include "cgl.h"

enum cg_config {
	FAN_ANIM_INTERVAL = 66,
	AIRGEN_ANIM_INTERVAL = 50,
	MAGNET_ANIM_INTERVAL = 66
};

struct ship {
	double x, y;
	double vx, vy;
	double ax, ay;
};

struct cg {
	struct cgl *level;
	struct ship *ship;
	double time;
};

struct cg *cg_init(struct cgl *level);
void cg_step(struct cg *cg, double dt);

#endif
