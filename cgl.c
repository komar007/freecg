#include "cgl.h"
#include <SDL/SDL_error.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

/*
 * CGL file format
 *
 * The map is divided into width x height blocks. Each block is 8x8 units
 * (32x32px). 4x4px units are used in some places for memory optimization and
 * in the level editor. Unless stated otherwise, dimensions and coordinates
 * are given in px. The map consists of tiles, each tile is anchored to
 * a block and represented by offsets from the origin of its block. Each tile
 * has 2 dimensions and coordinates of its graphical representation in GFX
 * file. Some special tiles may not have dimensions specified in the CGL file.
 * Such dimensions are static for a particular special tile.
 *
 * The CGL file consists of a 4-byte header containing an ASCII string "CGL1"
 * and 12 sections, each containing a 4-byte section header representing
 * section's name in ASCII and data specific for a section.
 * Where the name `integer' is used, a 32-bit unsigned little-endian int is
 * meant. `Short' refers to 16-bit unsigned little-endian short.
 *
 * Section names with brief descriptions (sizes do not include header):
 * 	SIZE (8 bytes)
 * 		2 integers, width and height of map, respectively, in
 * 		32x32 blocks
 * 	SOIN (width * height bytes)
 * 		width * height 1-byte integers holding the number of tiles
 * 		anchored in each block. Only 7 LSBs count. The MSB should be
 * 		omitted. The order is left to right, top to bottom.
 * 	-- between the SOIN and SOBS sections a magic 4-byte string may follow
 * 	   whose presence means the level is available in unregistered
 * 	   version of Crazy Gravity (demo level).
 * 	SOBS (width * height * 4 bytes)
 * 		width * height 4-byte structures describing tiles.
 * 		Single tile description:
 * 		offset	(length)
 * 		0x0	(1) position (first half - x, second - y)
 * 		0x1	(1) dimensions (as above)
 * 		0x2	(2) position in gfx file (first byte - x, second - y)
 * 		All values are given in units, so a conversion to px is
 * 		necessary (multiply by 4).
 * 	-- the same magic 4-byte string appears here iif the level is a demo
 * 	   level.
 * 	VENT (4 + nfans * 38 bytes)
 * 		Fans description. The first 4 bytes is an integer - the number
 * 		of fans. Then descriptions of fans follow, each 38 bytes long.
 *		A fan consists of a 48x48 base part and a 16x48 or 48x16
 *		pipes part. Single fan description:
 * 		offset	(length)
 * 		0x00	(2) fan type.
 * 			First byte:
 * 			first half: 0 - hi-powered, 1 - low-powered
 * 			second half: blowing direction, 0, 1, 2 or 3 meaning
 * 				respectively down, up, left or right.
 * 			Second byte: unknown.
 * 		0x02	(4) - two shorts, coordinates of fan's base
 * 		0x06	(4) - two shorts, coordinates of base's
 * 			graphics in gfx file
 * 		0x0a	(4) - two shorts, coordinates of pipes' tile
 * 		0x0e	(4) - two shorts, dimensions of pipes' tile
 * 		0x12	(4) - two shorts, coordinates of pipes' graphics in
 * 			gfx file
 * 		0x16	(8) - four shorts, bounding box coordinates and
 * 			dimensions
 * 		0x1e	(8) - four shorts, coordinates and dimensions of fan's
 * 			area of interaction.
 * 	FIXME: others
 * 	-- after the last section a magic 4-byte string may appear which means
 * 	   the level was created by a user in the level editor (or maybe in
 * 	   the full version of it?)
 */

void free_cgl(struct cgl *cgl)
{
	if (!cgl)
		return;
	free(cgl->tiles);
	free(cgl->fans);
	free(cgl);
}

struct cgl *read_cgl(const char *path, uint8_t **out_soin)
{
	extern int cgl_read_section_header(const char*, FILE*),
		   cgl_read_header(struct cgl*, FILE*),
		   cgl_read_size(struct cgl*, FILE*),
	           cgl_read_soin(struct cgl*, uint8_t*, FILE*),
		   cgl_read_magic(struct cgl*, FILE*),
		   cgl_read_sobs(struct cgl*, const uint8_t*, FILE*),
		   /* dynamic element reading functions: */
		   cgl_read_vent(struct cgl*, struct tile **, size_t*, FILE*);
	struct cgl *cgl;
	FILE *fp;
	uint8_t *soin = NULL;
	fp = fopen(path, "r");
	if (!fp) {
		SDL_SetError("fopen: %s", strerror(errno));
		return NULL;
	}
	cgl = calloc(1, sizeof(*cgl));
	cgl->tiles = NULL;
	cgl->fans = NULL;
	if (cgl_read_section_header("CGL1", fp) != 0)
		goto error;
	if (cgl_read_size(cgl, fp) != 0)
		goto error;
	soin = calloc(cgl->width * cgl->height, sizeof(*soin));
	if (cgl_read_soin(cgl, soin, fp) != 0)
		goto error;
	if (cgl_read_magic(cgl, fp) != 0)
		goto error;
	if (cgl_read_sobs(cgl, soin, fp) != 0)
		goto error;
	if (cgl->type == Demo && cgl_read_magic(cgl, fp) != 0)
		goto error;
	struct tile *vent_tiles;
	size_t nvent_tiles;
	if (cgl_read_vent(cgl, &vent_tiles, &nvent_tiles, fp) != 0)
		goto error;
	/* join extra tiles from the other sections with those from SOBS */
	size_t num_tiles = cgl->ntiles + (nvent_tiles /* + ... */);
	cgl->tiles = realloc(cgl->tiles, num_tiles * sizeof(*cgl->tiles));
	mempcpy(cgl->tiles + cgl->ntiles, vent_tiles,
			nvent_tiles * sizeof(*cgl->tiles));
	cgl->ntiles += nvent_tiles;
	free(vent_tiles);

	if (out_soin)
		*out_soin = soin;
	else
		free(soin);
	fclose(fp);
	return cgl;
error:
	fclose(fp);
	if (soin)
		free(soin);
	free_cgl(cgl);
	return NULL;
}

int read_short(int16_t arr[], size_t num, FILE *fp)
{
	/* FIXME: Add big-endian support */
	uint8_t buf[2];
	int nread;

	for(size_t i = 0; i < num; ++i) {
		nread = fread(buf, sizeof(uint8_t), 2, fp);
		if (nread < 2)
			return -EBADSHORT;
		arr[i] = buf[0] | buf[1] << 8;
	}
	return 0;
}

int read_integer(int32_t arr[], size_t num, FILE *fp)
{
	/* FIXME: Add big-endian support */
	int16_t buf[2];
	int err;

	for (size_t i = 0; i < num; ++i) {
		err = read_short(buf, 2, fp);
		if (err)
			return -EBADINT;
		arr[i] = buf[0] + buf[1] * 65536;
	}
	return 0;
}

int cgl_read_section_header(const char *name, FILE *fp)
{
	size_t nread;
	char hdr[4];

	nread = fread(hdr, sizeof(char), CGL_SHDR_SIZE, fp);
	if (nread < CGL_SHDR_SIZE) {
		SDL_SetError("cgl %s header incomplete", name);
		return -EBADSHDR;
	} else if (memcmp(hdr, name, CGL_SHDR_SIZE) != 0) {
		SDL_SetError("cgl %s header corrupted", name);
		return -EBADSOIN;
	}
	return 0;
}

/*
 * These functions fill the cgl structure with data read from a file pointer.
 * On error SDL_SetError(...) is called and non-zero error code is returned.
 * 0 is returned on success.
 */

int cgl_read_size(struct cgl *cgl, FILE *fp)
{
	uint32_t dims[2];
	int err;

	err = cgl_read_section_header("SIZE", fp);
	if (err)
		return err;
	err = read_integer((int32_t*)dims, 2, fp);
	if (err) {
		SDL_SetError("cgl SIZE section corrupted (incomplete)");
		return -EBADSIZE;
	}
	cgl->width  = dims[0];
	cgl->height = dims[1];
	return 0;
}

int cgl_read_soin(struct cgl *cgl, uint8_t *nums, FILE *fp)
{
	size_t nread,
	       nblocks = cgl->height * cgl->width;

	int err = cgl_read_section_header("SOIN", fp);
	if (err)
		return err;
	nread = fread(nums, sizeof(uint8_t), nblocks, fp);
	if (nread < nblocks) {
		SDL_SetError("cgl SOIN section corrupted (incomplete)");
		return -EBADSOIN;
	}
	cgl->ntiles = 0;
	for (size_t j = 0; j < cgl->height; ++j)
		for (size_t i = 0; i < cgl->width; ++i)
			cgl->ntiles += (*nums++ &= 0x7f);
	return 0;
}

int cgl_read_magic(struct cgl *cgl, FILE *fp)
{
	size_t nread;
	uint8_t mgc[4];

	nread = fread(mgc, sizeof(uint8_t), CGL_MAGIC_SIZE, fp);
	if (nread < CGL_MAGIC_SIZE) {
		fseek(fp, -nread, SEEK_CUR);
		SDL_SetError("cgl corrupted after SOIN section");
		return -EBADSOBS;
	} else if (memcmp(mgc, CGL_MAGIC, CGL_MAGIC_SIZE) != 0) {
		/* Magic absent, level works only in registered version */
		fseek(fp, -nread, SEEK_CUR);
		cgl->type = Full;
	} else {
		/* Magic present, level works also in unregistered version */
		cgl->type = Demo;
	}
	return 0;
}

int cgl_read_sobs(struct cgl *cgl, const uint8_t *soin, FILE *fp)
{
	extern int read_block(struct tile*, size_t, int, int, FILE*);
	int err;

	err = cgl_read_section_header("SOBS", fp);
	if (err)
		return err;
	cgl->tiles = calloc(cgl->ntiles, sizeof(*cgl->tiles));
	struct tile *tile_ptr = cgl->tiles;
	size_t cur_block = 0;
	for (size_t j = 0; j < cgl->height; ++j) {
		for (size_t i = 0; i < cgl->width; ++i, ++cur_block) {
			err = read_block(tile_ptr, (size_t)soin[cur_block],
					i * TILE_SIZE, j * TILE_SIZE, fp);

			if (err)
				return err;
			tile_ptr += soin[cur_block];
		}
	}
	return 0;
}

int read_block(struct tile *tiles, size_t num, int x, int y, FILE* fp)
{
	uint8_t buf[4];
	int nread;

	for (size_t k = 0; k < num; ++k) {
		nread = fread(buf, sizeof(uint8_t), SOBS_TILE_SIZE, fp);
		if (nread < SOBS_TILE_SIZE) {
			SDL_SetError("cgl SOBS section corrupted "
					"(incomplete)");
			return -EBADSOBS;
		}
		tiles[k].x = x + UNIT * (buf[0] >> 4);
		tiles[k].y = y + UNIT * (buf[0] & 0x0f);
		tiles[k].w = UNIT * (buf[1] >> 4);
		tiles[k].h = UNIT * (buf[1] & 0x0f);
		tiles[k].img_y = UNIT * buf[2];
		tiles[k].img_x = UNIT * buf[3];
	}
	return 0;
}

/*
 * Each of these functions reads one section of dynamic objects from the cgl
 * file. They allocate space for tiles needed by these objects, place the
 * objects there and return a pointer through the pointer in the second argument.
 */

int cgl_read_vent(struct cgl *cgl, struct tile **out_tiles, size_t *ntiles,
		FILE *fp)
{
	extern int cgl_read_one_vent(struct fan*, FILE*);
	uint32_t num;
	int err;

	err = cgl_read_section_header("VENT", fp);
	if (err)
		return err;
	err = read_integer((int32_t*)&num, 1, fp);
	if (err)
		goto error;
	cgl->nfans = num;
	*ntiles = 2 * cgl->nfans;
	cgl->fans = calloc(num, sizeof(*cgl->fans));
	struct tile *tiles = calloc(num, 2 * sizeof(*tiles));
	*out_tiles = tiles;
	for (size_t i = 0; i < num; ++i) {
		/* prepare pointers to tiles */
		cgl->fans[i].base = &tiles[2*i];
		cgl->fans[i].pipes = &tiles[2*i + 1];
		err = cgl_read_one_vent(&cgl->fans[i], fp);
		if (err)
			goto error;
	}
	return 0;
error:
	SDL_SetError("cgl VENT section corrupted (incomplete)");
	return -EBADVENT;
}

int cgl_read_one_vent(struct fan *fan, FILE *fp)
{
	int err;
	uint8_t buf[2];
	int16_t buf2[VENT_NUM_SHORTS];
	size_t nread;

	nread = fread(buf, sizeof(uint8_t), 2, fp);
	if (nread < 2)
		return -EBADVENT;
	err = read_short((int16_t*)buf2, VENT_NUM_SHORTS, fp);
	if (err)
		return -EBADVENT;
	fan->power = (buf[0] >> 4) & 0x01;
	fan->dir = buf[0] & 0x03;
	fan->base->x = buf2[0], fan->base->y = buf2[1];
	fan->base->w = fan->base->h = 48;
	fan->base->img_x = buf2[2], fan->base->img_y = buf2[3];
	fan->pipes->x = buf2[4], fan->pipes->y = buf2[5];
	fan->pipes->w = buf2[6], fan->pipes->h = buf2[7];
	fan->pipes->img_x = buf2[8], fan->pipes->img_y = buf2[9];
	fan->bbox.x = buf2[10], fan->bbox.y = buf2[11];
	fan->bbox.w = buf2[12], fan->bbox.h = buf2[13];
	fan->range.x = buf2[14], fan->range.y = buf2[15];
	fan->range.w = buf2[16], fan->range.h = buf2[17];
	return 0;
}
