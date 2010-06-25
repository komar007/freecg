/* cg.h - basic game logic data structures and definitions of simulation
 * constants
 * Copyright (C) 2010 Michal Trybus.
 *
 * This file is part of FreeCG.
 *
 * FreeCG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * FreeCG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FreeCG. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CG_H
#define CG_H

#include "cgl.h"
#include "gfx.h"
#include <stdint.h>

#define AIRGEN_ROT_SPEED 4.97
#define ROT_UP 18
#define AIR_RESISTANCE 0.3
#define FUEL_SPEED 0.2143
enum {
	BAR_SPEED_CHANGE_INTERVAL = 4,
	BAR_MIN_LEN = 2,
	GATE_BAR_MIN_LEN = 2,
	GATE_BAR_SPEED = 23,
	ENGINE_ACCEL = 130,
	GRAVITY = 23,
	MAX_FUEL = 16,
	FUEL_BARREL = 6,
	DEFAULT_LIFE = 5
};
struct ship {
	double x, y;
	double vx, vy;
	double rot, rot_speed;
	int engine;
	int keys[4];
	/* the size of cargo hold */
	size_t max_freight;
	/* freight currently on board */
	size_t num_freight;
	struct freight *freight;
	double max_vx, max_vy;
	/* reference to the airport on which the ship is waiting */
	struct airport *airport;
	double fuel;
	int has_turbo;
	int dead;
	int life;
};

void cg_init(struct cgl*);
void cg_step(struct cgl*, double);
void cg_ship_set_engine(struct ship*, int);
void cg_ship_rotate(struct ship*, double);
size_t cg_freight_remaining(struct cgl*);


#endif
