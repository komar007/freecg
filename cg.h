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

static const double bar_speeds[] = {5.65, 7.43, 10.83, 21.67, 43.33, 69.33};
enum {
	BAR_SPEED_CHANGE_INTERVAL = 4,
	BAR_MIN_LEN = 2
};

#endif
