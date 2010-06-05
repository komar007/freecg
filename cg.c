#include "cg.h"
#include "geometry.h"
#include <stdlib.h>
#include <math.h>

void cg_init_ship(struct ship *s)
{
	s->engine = 1;
	s->rot = 5;
}
struct cg *cg_init(struct cgl *level)
{
	struct cg *cg = calloc(1, sizeof(*cg));
	cg->level = level;
	cg->time = 0.0;
	cg->ship = calloc(1, sizeof(*cg->ship));
	cg_init_ship(cg->ship);
	return cg;
}

void ship_step(struct ship* s, double dt)
{
	s->vx += s->ax * dt;
	s->vy += s->ay * dt;
	s->x += s->vx * dt;
	s->y += s->vy * dt;
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
	extern int cg_detect_collision_bitmap(struct cg*, struct tile*);
	struct rect r;
	struct tile shipt;
	ship_to_tile(cg->ship, &shipt);
	for (size_t i = 0; blocks[i] != NULL; ++i) {
		if (tiles_intersect(&shipt, blocks[i], &r)) {
			int collision = 0;
			switch (blocks[i]->collision_test) {
			case Rect:
				collision = 1;
				break;
			case Bitmap:
				collision = cg_detect_collision_bitmap(cg,
						blocks[i]);
				break;
			}
			if (collision)
				printf("Collision %i %i %i %i, %i %i %i %i\n",
					shipt.x, shipt.y, shipt.w, shipt.h,
					blocks[i]->x, blocks[i]->y,
					blocks[i]->w, blocks[i]->h);
		}
	}
}

void cg_step(struct cg *cg, double time)
{
	double dt = time - cg->time;

	ship_step(cg->ship, dt);
	cg_detect_collisions(cg);
	cg->time = time;
}

/* Collision detectors */
int cg_detect_collision_bitmap(struct cg *cg, struct tile *tile)
{
	return 1;
}
