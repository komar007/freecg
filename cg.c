#include "cg.h"
#include "mathgeom.h"
#include <stdlib.h>
#include <math.h>
#include <assert.h>

void cg_init_ship(struct ship *s)
{
	s->engine = 0;
	s->rot = 3/2.0 * M_PI;
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

void cg_step_ship(struct ship* s, double time, double dt)
{
	double ax = 0, ay = 0;
	cg_ship_rotate(s, s->rots*dt);
	double rot = ((int)(s->rot / (2*M_PI) * 360) / 15) * 15 / 360.0 * 2*M_PI;
	if (s->engine) {
		ax = cos(rot)*100;
		ay = sin(rot)*100;
	}
	ax += -s->vx*0.2;
	ay += -s->vy*0.2;
	ay += 20;
	s->vx += ax * dt;
	s->vy += ay * dt;
	s->x += s->vx * dt;
	s->y += s->vy * dt;
}

void cg_ship_rotate(struct ship *s, double delta)
{
	s->rot += delta;
	normalize_angle(&s->rot);
}

/* perform logic simulation of all objects */
void cg_step_objects(struct cg *cg, double time, double dt)
{
	extern void cg_step_airgen(struct airgen*, struct cg*, double, double),
	            cg_step_bar(struct bar*, double, double),
	            cg_step_gate(struct gate*, double, double),
	            cg_step_lgate(struct ship*, struct lgate*, double, double);
	for (size_t i = 0; i < cg->level->nairgens; ++i)
		cg_step_airgen(&cg->level->airgens[i], cg, time, dt);
	for (size_t i = 0; i < cg->level->nbars; ++i)
		cg_step_bar(&cg->level->bars[i], time, dt);
	for (size_t i = 0; i < cg->level->ngates; ++i)
		cg_step_gate(&cg->level->gates[i], time, dt);
	for (size_t i = 0; i < cg->level->nlgates; ++i)
		cg_step_lgate(cg->ship, &cg->level->lgates[i], time, dt);
}

/* ==================== Collision detectors ==================== */
/* check if ship's center is inside the tile */
int cg_collision_rect_point(const struct tile *ship, const struct tile *tile)
{
	double x = ship->x + ship->w / 2,
	       y = ship->y + ship->h / 2;
	if (tile->x <= x && x <= tile->x + tile->w &&
	    tile->y <= y && y <= tile->y + tile->h)
		return 1;
	return 0;
}
/* check if tile t's bounding box collides with the ship within rectangle r,
 * knowing that r's origin in collision map is (img_x, img_y) */
int cg_collision_rect(const struct cg *cg, const struct rect *r,
		int img_x, int img_y, __attribute__((unused)) const struct tile *t)
{
	for (unsigned j = 0; j < r->h; ++j)
		for (unsigned i = 0; i < r->w; ++i)
			if (cg->cmap[img_y + j][img_x + i])
				return 1;
	return 0;
}
/* check if tile t collides with the ship within the rectangle r, knowing
 * that r's origin in collision map is (img_x, img_y) */
int cg_collision_bitmap(const struct cg *cg, const struct rect *r,
	int img_x, int img_y, const struct tile *t)
{
	int tile_img_x = t->tex_x + (r->x - t->x),
	    tile_img_y = t->tex_y + (r->y - t->y);
	for (unsigned j = 0; j < r->h; ++j)
		for (unsigned i = 0; i < r->w; ++i)
			if (cg->cmap[img_y + j][img_x + i] &&
			    cg->cmap[tile_img_y + j][tile_img_x + i])
				return 1;
	return 0;
}
/* ==================== /Collision detectors ==================== */

void cg_handle_collisions(struct cg *cg)
{
	extern void cg_handle_collisions_block(struct cg*, block);
	size_t x = max(0, cg->ship->x / BLOCK_SIZE),
	       y = max(0, cg->ship->y / BLOCK_SIZE);
	int end_x = min(cg->ship->x + SHIP_W,
			cg->level->width * BLOCK_SIZE),
	    end_y = min(cg->ship->y + SHIP_H,
			cg->level->height * BLOCK_SIZE);
	for (size_t j = y; (signed)j*BLOCK_SIZE < end_y; ++j)
		for (size_t i = x; (signed)i*BLOCK_SIZE < end_x; ++i)
			cg_handle_collisions_block(cg, cg->level->blocks[j][i]);
}
void cg_handle_collisions_block(struct cg *cg, block blk)
{
	extern void cg_call_collision_handler(struct cg*, struct tile*);
	struct rect r;
	struct tile stile;
	ship_to_tile(cg->ship, &stile);
	for (size_t i = 0; blk[i] != NULL; ++i) {
		if (!tiles_intersect(&stile, blk[i], &r))
			continue;
		int img_x = stile.tex_x + (r.x - stile.x),
		    img_y = stile.tex_y + (r.y - stile.y);
		int coll = 0;
		switch (blk[i]->collision_test) {
		case RectPoint:
			coll = cg_collision_rect_point(&stile, blk[i]);
			break;
		case Rect:
			coll = cg_collision_rect(cg, &r, img_x, img_y, blk[i]);
			break;
		case Bitmap:
			coll = cg_collision_bitmap(cg, &r, img_x, img_y, blk[i]);
			break;
		case Cannon:
			/* FIXME */
			coll = 1;
			break;
		case NoCollision:
			break;
		}
		if (coll)
			cg_call_collision_handler(cg, blk[i]);
	}
}
void cg_call_collision_handler(struct cg *cg, struct tile *tile)
{
	extern void cg_handle_collision_gate(struct gate*),
	            cg_handle_collision_lgate(struct ship*, struct lgate*),
	            cg_handle_collision_airgen(struct airgen*);
	switch (tile->collision_type) {
	case GateAction:
		cg_handle_collision_gate((struct gate*)tile->data);
		break;
	case LGateAction:
		cg_handle_collision_lgate(cg->ship, (struct lgate*)tile->data);
		break;
	case AirgenAction:
		cg_handle_collision_airgen((struct airgen*)tile->data);
		break;
	case Kaboom:
		break;
	}
}

void cg_step(struct cg *cg, double time)
{
	double dt = time - cg->time;
	cg_step_ship(cg->ship, time, dt);
	cg_handle_collisions(cg);
	cg_step_objects(cg, time, dt);
	cg->time = time;
}

/* ==================== Collision handlers ==================== */
void cg_handle_collision_gate(struct gate *gate)
{
	gate->active = 1;
}
void cg_handle_collision_lgate(struct ship *ship, struct lgate *lgate)
{
	lgate->active = 1;
	lgate->open = 1;
	for (size_t i = 0; i < 4; ++i)
		if (lgate->keys[i] && !ship->keys[i])
			lgate->open = 0;
}
void cg_handle_collision_airgen(struct airgen *airgen)
{
	airgen->active = 1;
}
/* ==================== /Collision handlers ==================== */

/* ==================== Object simulators ==================== */
/* auxilliary function used by all sliding tiles */
void update_sliding_tile(enum dir dir, struct tile *t, int len)
{
	switch (dir) {
	case Right:
		t->tex_x -= len - t->w;
		t->w = len;
		break;
	case Left:
		t->x -= len - t->w;
		t->w = len;
		break;
	case Down:
		t->tex_y -= len - t->h;
		t->h = len;
		break;
	case Up:
		t->y -= len - t->h;
		t->h = len;
		break;
	}
}

static const double bar_speeds[] = {5.65, 7.43, 10.83, 21.67, 43.33, 69.33};
static inline double bar_rand_speed(const struct bar *bar)
{
	return bar_speeds[rand_range(bar->min_s, bar->max_s)];
}
inline double bar_next_change(double time)
{
	return time + (rand()/(float)RAND_MAX + 0.5) *
		BAR_SPEED_CHANGE_INTERVAL;
}
void cg_step_bar(struct bar *bar, double time, double dt)
{
	if (bar->flen + bar->slen > bar->len) {
		bar->slen = bar->len - bar->flen;
		bar->fspeed = -bar_rand_speed(bar);
		bar->sspeed = -bar_rand_speed(bar);
	} else if (bar->flen <= BAR_MIN_LEN) {
		bar->fspeed = bar_rand_speed(bar);
	} else if (bar->gap_type == Constant && bar->slen <= BAR_MIN_LEN) {
		bar->fspeed = -bar_rand_speed(bar);
	} else if (bar->freq && bar->fnext_change <= time) {
		bar->fspeed = rand_sign() * bar_rand_speed(bar);
		bar->fnext_change = bar_next_change(time);
	}
	bar->flen += bar->fspeed * dt;
	bar->flen = fmin(bar->len, fmax(BAR_MIN_LEN, bar->flen));
	switch (bar->gap_type) {
	case Constant:
		bar->slen = bar->len - bar->flen - bar->gap;
		break;
	case Variable:
		if (bar->slen <= BAR_MIN_LEN) {
			bar->sspeed = bar_rand_speed(bar);
		} else if (bar->freq && bar->snext_change <= time) {
			bar->sspeed = rand_sign() * bar_rand_speed(bar);
			bar->snext_change = bar_next_change(time);
		}
		bar->slen += bar->sspeed * dt;
		break;
	}
	bar->slen = fmin(bar->len, fmax(BAR_MIN_LEN, bar->slen));
	switch (bar->orientation) {
	case Vertical:
		update_sliding_tile(Down, bar->fbar, (int)bar->flen);
		update_sliding_tile(Up, bar->sbar, (int)bar->slen);
		break;
	case Horizontal:
		update_sliding_tile(Right, bar->fbar, (int)bar->flen);
		update_sliding_tile(Left, bar->sbar, (int)bar->slen);
		break;
	}
}

void update_gate_bar(enum gate_type type, struct tile *bar, int len)
{
	switch (type) {
	case GateTop:
		update_sliding_tile(Down,  bar, len);
		break;
	case GateBottom:
		update_sliding_tile(Up,    bar, len);
		break;
	case GateLeft:
		update_sliding_tile(Right, bar, len);
		break;
	case GateRight:
		update_sliding_tile(Left,  bar, len);
		break;
	}
}

void cg_step_gate(struct gate *gate, __attribute__((unused)) double time, double dt)
{
	if (!gate->active && gate->len < gate->max_len)
		gate->len = fmin(gate->max_len,
				gate->len + GATE_BAR_SPEED * dt);
	if (gate->active && gate->len > 0)
		gate->len = fmax(GATE_BAR_MIN_LEN,
				gate->len - GATE_BAR_SPEED * dt);
	update_gate_bar(gate->type, gate->bar, (int)gate->len);
	gate->active = 0;
}

void cg_step_lgate(struct ship *ship, struct lgate *lgate,
		__attribute__((unused)) double time, double dt)
{
	for (size_t i = 0; i < 4; ++i) {
		if (!lgate->active) {
			lgate->light[i]->type = Transparent;
		} else {
			if (lgate->keys[i] && !ship->keys[i])
				lgate->light[i]->type = Blink;
			else if (lgate->keys[i])
				lgate->light[i]->type = Simple;
			else
				lgate->light[i]->type = Transparent;
		}
	}
	if (!lgate->open && lgate->len < lgate->max_len)
		lgate->len = fmin(lgate->max_len,
				lgate->len + GATE_BAR_SPEED * dt);
	if (lgate->open && lgate->len > 0)
		lgate->len = fmax(GATE_BAR_MIN_LEN,
				lgate->len - GATE_BAR_SPEED * dt);
	update_gate_bar(lgate->type, lgate->bar, (int)lgate->len);
	lgate->open = 0;
	lgate->active = 0;
}

void cg_step_airgen(struct airgen *airgen, struct cg *cg,
		__attribute__((unused)) double time, double dt)
{
	if (!airgen->active)
		return;
	switch (airgen->spin) {
	case CW:
		cg_ship_rotate(cg->ship, AIRGEN_ROT_SPEED * dt);
		break;
	case CCW:
		cg_ship_rotate(cg->ship, -AIRGEN_ROT_SPEED * dt);
		break;
	}
	airgen->active = 0;
}
/* ==================== /Object simulators ==================== */
