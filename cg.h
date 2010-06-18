#ifndef CG_H
#define CG_H

#include "gfx.h"
#include "cgl.h"
#include <stdint.h>

#define AIRGEN_ROT_SPEED 4.97
#define ROT_UP 18
enum {
	BAR_SPEED_CHANGE_INTERVAL = 4,
	BAR_MIN_LEN = 2,
	GATE_BAR_MIN_LEN = 2,
	GATE_BAR_SPEED = 23
};

struct ship {
	double x, y;
	double vx, vy;
	double rot, rots;
	int engine;
	int keys[4];
	int dead;
};
struct cg {
	struct cgl *level;
	struct ship *ship;
	double time;
	collision_map cmap;
};

struct cg *cg_init(struct cgl*);
void cg_step(struct cg*, double);
void cg_ship_rotate(struct ship*, double);


#endif
