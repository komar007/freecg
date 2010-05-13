#ifndef CGL_H
#define CGL_H

#include <stdio.h>
#include <stdint.h>

/* side length in pixels of the smallest game unit */
#define UNIT 4
#define TILE_SIZE (8 * UNIT)
/* size of section header */
#define CGL_SHDR_SIZE 4
#define CGL_MAGIC_SIZE 4
#define CGL_MAGIC "\xe1\xd2\xc3\xb4"
#define SOBS_TILE_SIZE 4
#define VENT_NUM_SHORTS 18

enum {
	EBADHDR = 1,
	EBADSHDR,
	EBADSIZE,
	EBADSOIN,
	EBADSOBS,
	EBADVENT,
	/* ... */

	EBADSHORT,
	EBADINT
};

struct rect {
	int x, y;
	unsigned int w, h;
};
/* basic tile, max. 8x8 units = 32x32 px */
struct tile {
	int x, y;	/* tile's origin */
	unsigned int w, h;
	unsigned int img_x, img_y;	/* position of image in gfx file */
	enum {
		Normal = 0,
		Special
	} type;

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
	struct tile *base,
		    *pipes;
	struct rect bbox,
		    range;
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
};

struct cgl *read_cgl(const char *path, uint8_t**);
void free_cgl(struct cgl *cgl);

#endif
