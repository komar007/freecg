#ifndef CG_H
#define CG_H

#include "cgl.h"
#include <stdint.h>

enum {
	CMAP_W = 588,
	CMAP_H = 464
};

typedef uint8_t collision_map[CMAP_H][CMAP_W];

struct ship {
	double x, y;
	double vx, vy;
	double ax, ay;
	int rot;
	int engine;
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
