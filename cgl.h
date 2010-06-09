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
	PIPE_HDR_SIZE = 16
};
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
	/* ... */

	EBADSHORT,
	EBADINT
};
/* basic tile, max. 8x8 units = 32x32 px */
struct tile {
	short x, y;	/* tile's origin */
	unsigned short w, h;
	short tex_x, tex_y;
	unsigned short tex_w, tex_h;
	unsigned short img_x, img_y; /* relative position of image in texture */
	enum {
		Simple = 0,
	} type;
	/* This is the type of collision test to be performed on a tile */
	enum {
		/* The whole rectangular area defined by points (x, y) and
		 * (x + w, y + h) is used to detect collisions */
		Rect = 0,
		/* A rectangular part of collision map defined by points
		 * (img_x, img_y), (img_x + w, img_y + h) is used to detect
		 * collisions in a rectabgular tile */
		Bitmap,
		/* Collisions are detected using a special function */
		Cannon
	} collision_test;
	/* necessary for renderer, the number of the most recent frame in
	 * which the tile was rendered */
	unsigned int lframe;
	void *data;
};
/* 8x8 unit = 32x32 px square tile containter */
struct block {
	size_t size;
	struct tile *tiles;
};

struct fan {
	enum {
		Hi = 0,
		Low
	} power;
	enum dir dir;
	struct tile *base,
		    *pipes; /* unused */
	struct rect bbox, /* unused */
		    range; /* unused */
};
struct magnet {
	enum dir dir;
	struct tile *base, /* unused */
		    *magn;
	struct rect bbox, /* unused */
		    range; /* unused */
};
struct airgen {
	enum {
		CCW = 0,
		CW
	} spin;
	enum dir dir;
	struct tile *base,
		    *pipes; /* unused */
	struct rect bbox, /* unused */
		    range; /* unused */
};
struct cannon {
	enum dir dir;
	int fire_rate;
	int speed_x, speed_y;
	struct tile *beg_base,
		    *beg_cano,
		    *end_base,
		    *end_catch;
	struct rect bbox;
	vector beg,
	       end;
};
struct bar {
	enum {
		Constant = 0,
		Variable
	} gap_type;
	enum {
		Vertical = 0,
		Horizontal
	} orientation;
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
};
/* cgl level contents */
struct cgl {
	enum {
		Full,
		Demo
	} type;
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
	struct tile ****blocks;
};

struct cgl *read_cgl(const char *, uint8_t**);
void cgl_preprocess(struct cgl *);
void free_cgl(struct cgl *);

#endif
