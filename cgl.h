#ifndef CGL_H
#define CGL_H

#include "basic_types.h"
#include <stdio.h>
#include <stdint.h>

enum {
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
	CANO_HDR_SIZE = 3
};
#define CGL_MAGIC "\xe1\xd2\xc3\xb4"

enum {
	SHIP_ON_IMG_X = 0,
	SHIP_ON_IMG_Y = 92,
	SHIP_OFF_IMG_X = 0,
	SHIP_OFF_IMG_Y = 115
};

enum {
	EBADHDR = 1,
	EBADSHDR,
	EBADSIZE,
	EBADSOIN,
	EBADSOBS,
	EBADVENT,
	EBADMAGN,
	EBADDIST,
	EBADCANO,
	/* ... */

	EBADSHORT,
	EBADINT
};
/* basic tile, max. 8x8 units = 32x32 px */
struct tile {
	int x, y;	/* tile's origin */
	unsigned int w, h;
	unsigned int img_x, img_y;	/* position of image in gfx file */
	enum {
		Simple = 0,
		Animated
	} type;
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

enum dir {
	Down = 0,
	Up,
	Left,
	Right
};

struct fan {
	enum {
		Hi = 0,
		Low
	} power;
	enum dir dir;
	int img_x; /* x position of primary texture */
	struct tile *base,
		    *pipes; /* unused */
	struct rect bbox, /* unused */
		    range; /* unused */
};
struct magnet {
	enum dir dir;
	int img_x; /* x position of primary texture */
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
	int img_x; /* position of primary texture */
	struct tile *base,
		    *pipes; /* unused */
	struct rect bbox, /* unused */
		    range; /* unused */
};
struct cannon {
	enum dir dir;
	int fire_rate;
	int speed_x, speed_y;
	struct tile *begin_base,
		    *begin_cano,
		    *end_base,
		    *end_catch;
	struct rect bbox;
	vector begin,
	       end;
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
	struct tile ****blocks;
};

struct cgl *read_cgl(const char *, uint8_t**);
void cgl_preprocess(struct cgl *);
void free_cgl(struct cgl *);

#endif
