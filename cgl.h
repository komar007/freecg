#ifndef CGL_H
#define CGL_H

#include "basic_types.h"
#include "gfx.h"
#include <stdio.h>
#include <stdint.h>

#define CGL_MAGIC "\xe1\xd2\xc3\xb4"
enum cgl_sizes {
	/* side length in pixels of the smallest game unit */
	UNIT = 4,
	CGL_BLOCK_SIZE = (8 * UNIT),
	BLOCK_SIZE = 32,
	/* size of section header */
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
#define DYN_TILES_OVERLAY_Z 0.2
#define DYN_TILES_Z 0.1
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
/* basic tile, max. 8x8 units = 32x32 px */
struct tile {
	short x, y;	/* tile's origin */
	double z;
	unsigned short w, h;
	short tex_x, tex_y;
	enum {
		Simple = 0,
		Transparent,
		Blink
	} type;
	/* This is the type of collision test to be performed on a tile */
	enum {
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
		NoCollision,
		/* Collisions are detected using a special function */
		Cannon
	} collision_test;
	enum {
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
	void *data;
};

struct fan {
	enum {
		Hi = 0,
		Low
	} power;
	enum dir dir;
	struct tile *base,
		    *pipes, /* unused */
		    *act;
	int tex_x;
	double modifier;
};
struct magnet {
	enum dir dir;
	struct tile *base, /* unused */
		    *magn,
		    *act;
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
		    *pipes, /* unused */
		    *act;
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
enum freigh {
	Freigh1 = 0,
	Freigh2,
	Freigh3,
	Freigh4
};
struct airport {
	struct tile *base[2],
		    *arrow[2],
		    *cargo[10];
	struct rect lbbox;
	enum {
		Homebase = 1,
		Key,
		Fuel,
		Freigh,
		Extras,
	} type;
	size_t num_cargo;
	double transfer_time;
	int sched_cargo_transfer;
	int ship_touched;
	union {
		int key;
		enum freigh freigh[10];
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
	size_t num_all_freigh;
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
};

struct cgl *read_cgl(const char*, uint8_t**);
void cgl_preprocess(struct cgl*);
void free_cgl(struct cgl*);

#endif
