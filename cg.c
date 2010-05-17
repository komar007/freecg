#include "cg.h"
#include <stdlib.h>
#include <math.h>

static const int magnet_anim_order[] = {0, 1, 2, 1};
static const int fan_anim_order[] = {0, 1, 2};
static const int airgen_anim_order[] = {0, 1, 2, 3, 4, 5, 6, 7};

struct cg *cg_init(struct cgl *level)
{
	struct cg *cg = calloc(1, sizeof(*cg));
	cg->level = level;
	cg->time = 0.0;
	return cg;
}

void cg_step(struct cg *cg, double time)
{
	extern void cg_animate_fan(struct tile*, double);
	extern void cg_animate_magnet(struct tile*, double);
	extern void cg_animate_airgen(struct tile*, double);
	double dt = time - cg->time;

	for (size_t i = 0; i < cg->level->nmagnets; ++i)
		cg_animate_magnet(cg->level->magnets[i].magn, time);
	for (size_t i = 0; i < cg->level->nfans; ++i)
		cg_animate_fan(cg->level->fans[i].base, time);
	for (size_t i = 0; i < cg->level->nairgens; ++i)
		cg_animate_airgen(cg->level->airgens[i].base, time);
	cg->time = time;
}

void cg_animate_fan(struct tile *tile, double time)
{
	int phase = round(time * 1000 / FAN_ANIM_INTERVAL);
	tile->dyn.cur_tex = fan_anim_order[phase % 3];
}
void cg_animate_magnet(struct tile *tile, double time)
{
	int phase = round(time * 1000 / MAGNET_ANIM_INTERVAL);
	tile->dyn.cur_tex = magnet_anim_order[phase % 4];
}
void cg_animate_airgen(struct tile *tile, double time)
{
	int phase = round(time * 1000 / AIRGEN_ANIM_INTERVAL);
	tile->dyn.cur_tex = airgen_anim_order[phase % 8];
}

void ship_step(struct ship* r, double dt)
{
	r->vx += r->ax * dt;
	r->vy += r->ay * dt;
	r->x += r->vx * dt;
	r->y += r->vy * dt;
}
