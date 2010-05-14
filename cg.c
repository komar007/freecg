#include "cg.h"
#include <stdlib.h>

struct cg *cg_init(struct cgl *level)
{
	struct cg *cg = calloc(1, sizeof(*cg));
	cg->level = level;
	return cg;
}

void cg_step(struct cg *cg, double dt)
{
	extern void rocket_step(struct rocket*, double);
	rocket_step(cg->rocket, dt);
}

void rocket_step(struct rocket* r, double dt)
{
	r->vx += r->ax * dt;
	r->vy += r->ay * dt;
	r->x += r->vx * dt;
	r->y += r->vy * dt;
}
