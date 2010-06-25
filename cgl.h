/* cgl.h - data structures representing a single CGL level file and the tile
 * structure - the main data structure of the game simulation and graphics
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

#ifndef CGL_H
#define CGL_H

#include "mathgeom.h"
#include "gfx.h"
#include <stdio.h>
#include <stdint.h>

#define CGL_MAGIC "\xe1\xd2\xc3\xb4"
enum cgl_sizes {
	/* side length in pixels of the smallest game unit */
	UNIT = 4,
	CGL_BLOCK_SIZE = (8 * UNIT),
	BLOCK_SIZE = 32,
	/* size of section headers */
	CGL_SHDR_SIZE = 4,
	CGL_MAGIC_SIZE = 4,
	SOBS_TILE_SIZE = 4,
	VENT_NUM_SHORTS = 18,
	VENT_HDR_SIZE = 2,
	MAGN_NUM_SHORTS = 18,
	MAGN_HDR_SIZE = 2,
	DIST_NUM_SHORTS = 18,
	DIST_HDR_SIZE = 2,
	CANO_NUM_SHORTS = 22,
	CANO_HDR_SIZE = 3,
	PIPE_NUM_SHORTS = 4,
	PIPE_HDR_SIZE = 16,
	ONEW_HDR_SIZE = 1,
	ONEW_NUM_SHORTS = 32,
	LPTS_HDR_SIZE = 1,
	LPTS_NUM_SHORTS = 6,
	LPTS_NUM_STUFF = 10
};
/* All dynamic tiles (not in SOBS) are placed above the rest */
#define DYN_TILES_Z 0.1
/* Some dynamic tiles consist of 2 layers. Second layer's Z */
#define DYN_TILES_OVERLAY_Z 0.2
enum error_codes {
	EBADHDR = 1,
	EBADSHDR,
	EBADSIZE,
	EBADSOIN,
	EBADSOBS,
	EBADVENT,
	EBADMAGN,
	EBADDIST,
	EBADCANO,
	EBADPIPE,
	EBADONEW,
	EBADBARR,
	EBADLPTS,
	/* ... */

	EBADSHORT,
	EBADINT
};
/* The main tile data structure. Used to represent all objects in the game. */
struct tile {
	/* origin */
	short x, y;
	/* dimensions */
	unsigned short w, h;
	/* texture position - assume the same dimensions of texture */
	short tex_x, tex_y;
	/* z-value - from 0 (lowest) to 1 (highest) */
	double z;
	enum type {
		/* drawn normally */
		Simple = 0,
		/* not drawn */
		Transparent,
		/* Blinking (for gate lights) */
		Blink
	} type;
	/* This is the type of collision test to be performed on a tile */
	enum collision_test {
		/* The whole rectangular area defined by points (x, y) and
		 * (x + w, y + h) is used to detect collisions */
		Rect = 0,
		/* As above, but check whether the center of ship intersects,
		 * not the whole ship */
		RectPoint,
		/* A rectangular part of collision map defined by points
		 * (img_x, img_y), (img_x + w, img_y + h) is used to detect
		 * collisions in a rectabgular tile */
		Bitmap,
		/* For transparent or special tiles */
		NoCollision,
		/* Collisions are detected using a special function */
		Cannon
	} collision_test;
	/* What to do if there's a collision */
	enum collision_type {
		Kaboom = 0,
		AirgenAction,
		GateAction,
		LGateAction,
		AirportAction,
		FanAction,
		MagnetAction
	} collision_type;
	/* necessary for renderer, the number of the most recent frame in
	 * which the tile was rendered */
	unsigned int lframe;
	/* additional data necessary for collision detection */
	void *data;
};

struct fan {
	enum {
		Hi = 0,
		Low
	} power;
	enum dir dir;
	struct tile *base,
		    *pipes,
		    *act;
	/* x position of the primary texture */
	int tex_x;
	double modifier;
};
struct magnet {
	enum dir dir;
	struct tile *base,
		    *magn,
		    *act;
	/* x position of the primary texture */
	int tex_x;
	double modifier;
};
struct airgen {
	enum {
		CCW = 0,
		CW
	} spin;
	enum dir dir;
	struct tile *base,
		    *pipes,
		    *act;
	/* x position of the primary texture */
	int tex_x;
	int active;
};
struct cannon {
	enum dir dir;
	int fire_rate;
	int speed_x, speed_y;
	struct tile *beg_base,
		    *beg_cano,
		    *end_base,
		    *end_catch;
	vector beg,
	       end;
};
enum orientation {
	Vertical = 0,
	Horizontal
};
struct bar {
	enum {
		Constant = 0,
		Variable
	} gap_type;
	enum orientation orientation;
	double flen, slen;
	double fspeed, sspeed;
	double fnext_change, snext_change;
	int len;
	int gap;
	int min_s, max_s;
	int freq;
	struct tile *beg,
		    *end,
		    *fbar,
		    *sbar;
	int btex_x, etex_x;
};
enum gate_type{
	GateLeft = 0,
	GateTop,
	GateRight,
	GateBottom
};
struct gate {
	struct tile *base[5],
		    *bar,
		    *arrow,
		    *act;
	enum gate_type type;
	enum orientation orient;
	int dir;
	int has_end;
	double max_len, len;
	int active;
};
struct lgate {
	struct tile *base[5],
		    *bar,
		    *light[4],
		    *act;
	enum gate_type type;
	enum orientation orient;
	int keys[4];
	int has_end;
	double max_len, len;
	int active;
	int open;
};
struct freight {
	enum {
		Freight1 = 0,
		Freight2,
		Freight3,
		Freight4
	} f;
	struct airport *ap;
};
struct airport {
	struct tile *base,
		    *stripe[2],
		    *arrow[2],
		    *cargo[10];
	struct rect lbbox;
	enum {
		Homebase = 1,
		Key,
		Fuel,
		Freight,
		Extras,
	} type;
	int has_left_arrow,
	    has_right_arrow;
	size_t num_cargo;
	double transfer_time;
	int sched_cargo_transfer;
	int ship_touched;
	union {
		int key;
		struct freight freight[10];
		enum {
			Turbo = 0,
			Life,
			Cargo
		} extras[10];
	} c /* common */;
};
typedef struct tile **block;
/* cgl level contents */
struct cgl {
	enum {
		Full,
		Demo
	} type;
	size_t num_all_freight;
	size_t width, height;
	size_t ntiles;
	struct tile *tiles;
	size_t nfans;
	struct fan *fans;
	size_t nmagnets;
	struct magnet *magnets;
	size_t nairgens;
	struct airgen *airgens;
	size_t ncannons;
	struct cannon *cannons;
	size_t nbars;
	struct bar *bars;
	size_t ngates;
	struct gate *gates;
	size_t nlgates;
	struct lgate *lgates;
	size_t nairports;
	struct airport *airports;
	struct airport *hb;
	block **blocks;

	double time;
	struct ship *ship;
	collision_map cmap;
};

struct cgl *read_cgl(const char*, uint8_t**);
void cgl_preprocess(struct cgl*);
void free_cgl(struct cgl*);

#endif
