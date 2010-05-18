#include "cg.h"
#include "graphics.h"
#include <stdlib.h>
#include <math.h>

struct cg *cg_init(struct cgl *level)
{
	struct cg *cg = calloc(1, sizeof(*cg));
	cg->level = level;
	cg->time = 0.0;
	return cg;
}

void cg_step(struct cg *cg, double time)
{
	double dt = time - cg->time;

	for (size_t i = 0; i < cg->level->nmagnets; ++i)
		cg_animate_magnet(cg->level->magnets[i].magn, time);
	for (size_t i = 0; i < cg->level->nfans; ++i)
		cg_animate_fan(cg->level->fans[i].base, time);
	for (size_t i = 0; i < cg->level->nairgens; ++i)
		cg_animate_airgen(cg->level->airgens[i].base, time);
	cg->time = time;
}

void ship_step(struct ship* r, double dt)
{
	r->vx += r->ax * dt;
	r->vy += r->ay * dt;
	r->x += r->vx * dt;
	r->y += r->vy * dt;
}
