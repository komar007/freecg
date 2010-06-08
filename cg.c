#include "cg.h"
#include "geometry.h"
#include <stdlib.h>
#include <math.h>
#include <assert.h>

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

void cg_step_ship(struct ship* s, double dt)
{
	s->vx += s->ax * dt;
	s->vy += s->ay * dt;
	s->x += s->vx * dt;
	s->y += s->vy * dt;
}

void cg_step_objects(struct cg *cg, double time, double dt)
{
	extern void cg_step_bar(struct bar*, double, double);
	for (size_t i = 0; i < cg->level->nbars; ++i)
		cg_step_bar(&cg->level->bars[i], time, dt);
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
	cg_step_ship(cg->ship, dt);
	cg_step_objects(cg, time, dt);
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

/* Object steppers */
void update_sliding_tile(enum dir dir, struct tile *t, int len)
{
	switch (dir) {
	case Right:
		t->img_x -= len - t->w;
		t->w = len;
		break;
	case Left:
		t->x -= len - t->w;
		t->w = len;
		break;
	case Down:
		t->img_y -= len - t->h;
		t->h = len;
		break;
	case Up:
		t->y -= len - t->h;
		t->h = len;
		break;
	}
}
inline int rand_range(int min_n, int max_n)
{
	assert(min_n <= max_n);
	return rand() % (max_n - min_n + 1) + min_n;
}
void cg_step_bar(struct bar *bar, double time, double dt)
{
	switch (bar->gap_type) {
	case Constant:
		if (bar->fbar_len <= 0 || bar->sbar_len <= 0) {
			bar->speed = -sgn(bar->speed) *
				bar_speeds[rand_range(bar->min_s, bar->max_s)];
			bar->fbar_len = fmax(0, bar->fbar_len);
			bar->sbar_len = fmax(0, bar->sbar_len);
		} else if (bar->freq && bar->next_change <= time) {
			bar->speed = (2*rand_range(0, 1) - 1) *
				bar_speeds[rand_range(bar->min_s, bar->max_s)];
			bar->next_change = time + (rand()/(float)RAND_MAX + 0.5) *
				(bar->len - bar->gap)/abs(bar->speed);
		}
		bar->fbar_len += bar->speed * dt;
		bar->sbar_len = bar->len - bar->fbar_len - bar->gap;
		/* Make sure sth negative is not drawn */
		bar->fbar_len = fmax(0, bar->fbar_len);
		bar->sbar_len = fmax(0, bar->sbar_len);
		break;
	}
	switch (bar->orientation) {
	case Vertical:
		update_sliding_tile(Down, bar->fbar, (int)bar->fbar_len);
		update_sliding_tile(Up, bar->sbar, (int)bar->sbar_len);
		break;
	case Horizontal:
		update_sliding_tile(Right, bar->fbar, (int)bar->fbar_len);
		update_sliding_tile(Left, bar->sbar, (int)bar->sbar_len);
		break;
	}
}
