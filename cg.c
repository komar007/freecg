#include "cg.h"
#include "graphics.h"
#include "geometry.h"
#include <stdlib.h>
#include <math.h>

struct cg *cg_init(struct cgl *level)
{
	struct cg *cg = calloc(1, sizeof(*cg));
	cg->level = level;
	cg->time = 0.0;
	cg->ship = calloc(1, sizeof(*cg->ship));
	//cg->ship->vx = 10;
	//cg->ship->vy = 10;
	cg->ship->engine = 1;
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
	extern void cg_detect_collisions_block(struct cg*, struct tile**);
	size_t x = max(0, cg->ship->x / BLOCK_SIZE),
	       y = max(0, cg->ship->y / BLOCK_SIZE);
	int end_x = min(cg->ship->x + SHIP_W,
			cg->level->width * BLOCK_SIZE),
	    end_y = min(cg->ship->y + SHIP_H,
			cg->level->height * BLOCK_SIZE);
	for (size_t j = y; (signed)j*BLOCK_SIZE < end_y; ++j)
		for (size_t i = x; (signed)i*BLOCK_SIZE < end_x; ++i)
			cg_detect_collisions_block(cg, cg->level->blocks[j][i]);
}

void cg_detect_collisions_block(struct cg *cg, struct tile **blocks)
{
	struct rect r;
	struct tile shipt;
	for (size_t i = 0; blocks[i] != NULL; ++i) {
		ship_to_tile(cg->ship, &shipt);
		if (tiles_intersect(&shipt, blocks[i], &r)) {
			printf("1\n");
			printf("Collision %i %i %i %i, %i %i %i %i\n",
					shipt.x, shipt.y, shipt.w, shipt.h,
					blocks[i]->x, blocks[i]->y,
					blocks[i]->w, blocks[i]->h);
		} else
			printf("0\n");
	}
}

void cg_step(struct cg *cg, double time)
{
	double dt = time - cg->time;

	for (size_t i = 0; i < cg->level->nmagnets; ++i)
		cg_animate_magnet(&cg->level->magnets[i], time);
	for (size_t i = 0; i < cg->level->nfans; ++i)
		cg_animate_fan(&cg->level->fans[i], time);
	for (size_t i = 0; i < cg->level->nairgens; ++i)
		cg_animate_airgen(&cg->level->airgens[i], time);
	ship_step(cg->ship, dt);
	cg_detect_collisions(cg);
	cg->time = time;
}
