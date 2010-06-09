#ifndef CG_H
#define CG_H

#include "gfx.h"
#include "cgl.h"
#include <stdint.h>

enum {
	BAR_SPEED_CHANGE_INTERVAL = 4,
	BAR_MIN_LEN = 2
};

struct ship {
	double x, y;
	double vx, vy;
	double ax, ay;
	int rot;
	int engine;
	/*temporary*/ double switchoff;
};
struct cg {
	struct cgl *level;
	struct ship *ship;
	double time;
	collision_map cmap;
};

struct cg *cg_init(struct cgl *level);
void cg_step(struct cg *cg, double dt);


#endif
