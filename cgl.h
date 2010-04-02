#ifndef CGL_H
#define CGL_H

#include <stdio.h>
#include <stdint.h>

#define CGL_HEADER_SIZE 16
#define CGL_SOIN_HDR_SIZE 4
#define CGL_MAGIC_SIZE 4
#define CGL_MAGIC "\xe1\xd2\xc3\xb4"

enum {
	EBADHDR = 1,
	EBADSOIN,
	EBADSOBS
};

/* basic tile, max. 8x8 units = 32x32 px */
struct tile {
	uint8_t	width, height;
	uint8_t offs_x, offs_y;	/* offset of tile from segment's origin */
	uint16_t img_x, img_y;	/* position of image in gfx file */
};
/* 8x8 unit = 32x32 px square tile containter */
struct block {
	size_t size;
	struct tile *tiles;
};
/* cgl level contents */
struct cgl {
	enum {
		FULL,
		DEMO
	} type;
	size_t width, height;
	struct block **blocks;
};

struct cgl *read_cgl(const char *path);
void free_cgl(struct cgl *cgl);

#endif
