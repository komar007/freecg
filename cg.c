#include "cg.h"
#include "graphics.h"
#include <stdlib.h>
#include <math.h>

struct cg *cg_init(struct cgl *level)
{
	struct cg *cg = calloc(1, sizeof(*cg));
	cg->level = level;
	cg->time = 0.0;
	cg->ship = calloc(1, sizeof(*cg->ship));
	cg->ship->w = SHIP_W;
	cg->ship->h = SHIP_H;
	return cg;
}

void ship_step(struct ship* r, double dt)
{
	r->vx += r->ax * dt;
	r->vy += r->ay * dt;
	r->x += r->vx * dt;
	r->y += r->vy * dt;
}

void cg_detect_collisions(struct cg *cg)
{
	extern void cg_detect_collisions_block(struct tile**);
	size_t x = cg->ship->x / BLOCK_SIZE,
	       y = cg->ship->y / BLOCK_SIZE;
	unsigned end_x = cg->ship->x + cg->ship->w,
		 end_y = cg->ship->y + cg->ship->h;
	for (size_t j = y; j*BLOCK_SIZE < end_y; ++j)
		for (size_t i = x; i*BLOCK_SIZE < end_x; ++i)
			cg_detect_collisions_block(cg->level->blocks[j][i]);
}

void cg_detect_collisions_block(struct tile **blocks)
{
	for (size_t i = 0; blocks[i] != NULL; ++i) {
		
	}
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
	ship_step(cg->ship, dt);
	cg_detect_collisions(cg);
	cg->time = time;
}
