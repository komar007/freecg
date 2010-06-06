#include "cg.h"
#include "geometry.h"
#include <stdlib.h>
#include <math.h>

void cg_init_ship(struct ship *s)
{
	s->engine = 1;
	s->rot = 0;
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

void cg_ship_step(struct ship* s, double dt)
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
	extern int cg_collision_bitmap(struct cg*, struct rect*,
			int, int, struct tile*),
	           cg_collision_rect(struct cg*, struct rect*,
			int, int, struct tile*);
	struct rect r;
	struct tile stile;
	ship_to_tile(cg->ship, &stile);
	for (size_t i = 0; blocks[i] != NULL; ++i) {
		if (!tiles_intersect(&stile, blocks[i], &r))
			continue;
		int img_x = stile.img_x + (r.x - stile.x),
		    img_y = stile.img_y + (r.y - stile.y);
		int coll = 0;
		switch (blocks[i]->collision_test) {
		case Rect:
			coll = cg_collision_rect(cg, &r, img_x, img_y, blocks[i]);
			break;
		case Bitmap:
			coll = cg_collision_bitmap(cg, &r, img_x, img_y, blocks[i]);
			break;
		case Cannon:
			/* FIXME */
			coll = 1;
			break;
		}
		if (coll)
			printf("Collision %i %i %i %i, %i %i %i %i\n",
				stile.x, stile.y, stile.w, stile.h,
				blocks[i]->x, blocks[i]->y,
				blocks[i]->w, blocks[i]->h);
	}
}

void cg_step(struct cg *cg, double time)
{
	double dt = time - cg->time;
	cg_ship_step(cg->ship, dt);
	cg_detect_collisions(cg);
	cg->time = time;
}

/* Collision detectors */
int cg_collision_rect(struct cg *cg, struct rect *r,
		int img_x, int img_y, __attribute__((unused)) struct tile *t)
{
	for (unsigned j = 0; j < r->h; ++j)
		for (unsigned i = 0; i < r->w; ++i)
			if (cg->cmap[img_y + j][img_x + i])
				return 1;
	return 0;
}
int cg_collision_bitmap(struct cg *cg, struct rect *r,
	int img_x, int img_y, struct tile *t)
{
	int tile_img_x = t->img_x + (r->x - t->x),
	    tile_img_y = t->img_y + (r->y - t->y);
	for (unsigned j = 0; j < r->h; ++j)
		for (unsigned i = 0; i < r->w; ++i)
			if (cg->cmap[img_y + j][img_x + i] &&
			    cg->cmap[tile_img_y + j][tile_img_x + i])
				return 1;
	return 0;
}
