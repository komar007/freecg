#include "cgl.h"
#include <SDL/SDL_error.h>
#include <stdio.h>
#include <math.h>
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
 * 	-- directly before headers of all following sections a magic 4-byte
 * 	   string may follow whose presence means the level is available in
 * 	   unregistered version of Crazy Gravity (demo level).
 * 	SOBS (width * height * 4 bytes)
 * 		width * height 4-byte structures describing tiles.
 * 		Single tile description:
 * 		offset	(length)
 * 		0x0	(1) position (first half - x, second - y)
 * 		0x1	(1) dimensions (as above)
 * 		0x2	(2) position in gfx file (first byte - y, second - x)
 * 		All values are given in units, so a conversion to px is
 * 		necessary (multiply by 4).
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
 * 	MAGN (4 + nmagnets * 38 bytes)
 * 		Magnets description. All like in VENT.
 * 		Single magnet description:
 * 		offset	(length)
 * 		0x00	(2) magnet type
 * 			First byte: direction, values as in VENT.
 * 			Second byte: unknown.
 * 		next as in VENT
 * 	DIST (4 + nairgens * 38 bytes)
 * 		Air current generators. Like VENT.
 * 		First byte:
 *		first half: turning direction: 0 - CCW, 1 - CW
 *		second half: direction (as in VENT)
 *		Second byte: unknown
 * 		The rest as in VENT
 *	CANO (4 + ncannons * 51 bytes)
 *		Single cannon:
 *		offset	(length)
 *		0x00	(3) header - first byte: direction, rest - unknown
 *		0x03	(2) a short: fire rate
 *		0x05	(2) 2 signed bytes: speed_x, speed_y
 *		0x07	(8) 4 shorts: coordinates of begning and end of the
 *			bullet track
 * 		0x0f	(4) 2 shorts: coordinates of cannon's base
 *		0x11	(8) 4 shorts: coords nd gfx coords of cannon tile
 *		0x1b	(4) 2 shorts: coords of cannon's end
 *		0x1f	(12)6 shorts: full description of catcher's tile
 *		0x2b	(8) 4 shorts: bounding box
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
	if (cgl->blocks) {
		for (size_t j = 0; j < cgl->height; ++j) {
			for (size_t i = 0; i < cgl->width; ++i)
				free(cgl->blocks[j][i]);
			free(cgl->blocks[j]);
		}
		free(cgl->blocks);
	}
	free(cgl);
}

#define FIX_PTRS(what, tile, howmany, arr)\
	for (size_t i = 0; i < howmany; ++i) \
		what[i].tile = cgl->tiles + cgl->ntiles + (what[i].tile - arr);
struct cgl *read_cgl(const char *path, uint8_t **out_soin)
{
	extern int cgl_read_section_header(const char*, FILE*),
		   cgl_read_header(struct cgl*, FILE*),
		   cgl_read_size(struct cgl*, FILE*),
	           cgl_read_soin(struct cgl*, uint8_t*, FILE*),
		   cgl_read_magic(struct cgl*, FILE*),
		   cgl_read_sobs(struct cgl*, const uint8_t*, FILE*),
		   /* dynamic element reading functions: */
		   cgl_read_vent(struct cgl*, struct tile**, size_t*, FILE*),
		   cgl_read_magn(struct cgl*, struct tile**, size_t*, FILE*),
		   cgl_read_dist(struct cgl*, struct tile**, size_t*, FILE*),
		   cgl_read_cano(struct cgl*, struct tile**, size_t*, FILE*),
		   cgl_read_pipe(struct cgl*, struct tile**, size_t*, FILE*);
	struct cgl *cgl;
	FILE *fp;
	uint8_t *soin = NULL;
	fp = fopen(path, "rb");
	if (!fp) {
		SDL_SetError("fopen: %s", strerror(errno));
		return NULL;
	}
	cgl = calloc(1, sizeof(*cgl));
	cgl->tiles = NULL;
	cgl->fans = NULL;
	cgl->magnets = NULL;
	cgl->airgens = NULL;
	cgl->cannons = NULL;
	cgl->bars = NULL;
	cgl->blocks = NULL;
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
	struct tile *vent_tiles, *magn_tiles, *dist_tiles, *cano_tiles,
		    *pipe_tiles;
	size_t nvent_tiles, nmagn_tiles, ndist_tiles, ncano_tiles,
	       npipe_tiles;
	if (cgl_read_magic(cgl, fp) != 0)
		goto error;
	if (cgl_read_vent(cgl, &vent_tiles, &nvent_tiles, fp) != 0)
		goto error;
	if (cgl_read_magic(cgl, fp) != 0)
		goto error;
	if (cgl_read_magn(cgl, &magn_tiles, &nmagn_tiles, fp) != 0)
		goto error;
	if (cgl_read_magic(cgl, fp) != 0)
		goto error;
	if (cgl_read_dist(cgl, &dist_tiles, &ndist_tiles, fp) != 0)
		goto error;
	if (cgl_read_magic(cgl, fp) != 0)
		goto error;
	if (cgl_read_cano(cgl, &cano_tiles, &ncano_tiles, fp) != 0)
		goto error;
	if (cgl_read_magic(cgl, fp) != 0)
		goto error;
	if (cgl_read_pipe(cgl, &pipe_tiles, &npipe_tiles, fp) != 0)
		goto error;
	/* join extra tiles from the other sections with those from SOBS,
	 * fix pointers to point to the new memory */
	size_t num_tiles = cgl->ntiles + (nvent_tiles + nmagn_tiles +
			ndist_tiles + ncano_tiles + npipe_tiles/* + ... */);
	cgl->tiles = realloc(cgl->tiles, num_tiles * sizeof(*cgl->tiles));
	memcpy(cgl->tiles + cgl->ntiles, vent_tiles,
			nvent_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->fans, base, cgl->nfans, vent_tiles)
	FIX_PTRS(cgl->fans, pipes, cgl->nfans, vent_tiles)
	cgl->ntiles += nvent_tiles;
	free(vent_tiles);
	memcpy(cgl->tiles + cgl->ntiles, magn_tiles,
			nmagn_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->magnets, base, cgl->nmagnets, magn_tiles)
	FIX_PTRS(cgl->magnets, magn, cgl->nmagnets, magn_tiles)
	cgl->ntiles += nmagn_tiles;
	free(magn_tiles);
	memcpy(cgl->tiles + cgl->ntiles, dist_tiles,
			ndist_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->airgens, base, cgl->nairgens, dist_tiles)
	FIX_PTRS(cgl->airgens, pipes, cgl->nairgens, dist_tiles)
	cgl->ntiles += ndist_tiles;
	free(dist_tiles);
	memcpy(cgl->tiles + cgl->ntiles, cano_tiles,
			ncano_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->cannons, beg_base, cgl->ncannons, cano_tiles)
	FIX_PTRS(cgl->cannons, beg_cano, cgl->ncannons, cano_tiles)
	FIX_PTRS(cgl->cannons, end_base, cgl->ncannons, cano_tiles)
	FIX_PTRS(cgl->cannons, end_catch, cgl->ncannons, cano_tiles)
	cgl->ntiles += ncano_tiles;
	free(cano_tiles);
	memcpy(cgl->tiles + cgl->ntiles, pipe_tiles,
			npipe_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->bars, beg, cgl->nbars, pipe_tiles)
	FIX_PTRS(cgl->bars, end, cgl->nbars, pipe_tiles)
	FIX_PTRS(cgl->bars, fbar, cgl->nbars, pipe_tiles)
	FIX_PTRS(cgl->bars, sbar, cgl->nbars, pipe_tiles)
	cgl->ntiles += npipe_tiles;
	free(pipe_tiles);

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
		SDL_SetError("cgl corrupted (error reading magic/header");
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
					i * CGL_BLOCK_SIZE, j * CGL_BLOCK_SIZE, fp);

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

/* Auxilliary functions to extract a single rectangle or tile description from
 * an array of int16_t */
void parse_point(int16_t *data, vector *a, vector *b)
{
	a->x = data[0], a->y = data[1];
	b->x = data[2], b->y = data[3];
}
void parse_rect(int16_t *data, struct rect *rect)
{
	rect->x = data[0], rect->y = data[1];
	rect->w = data[2], rect->h = data[3];
}
void parse_tile(int16_t *data, struct tile *tile)
{
	tile->x = data[0], tile->y = data[1];
	tile->w = data[2], tile->h = data[3];
	tile->img_x = data[4], tile->img_y = data[5];
}
void parse_tile_simple(int16_t *data, struct tile *tile,
		unsigned width, unsigned height)
{
	tile->x = data[0], tile->y = data[1];
	tile->img_x = data[2], tile->img_y = data[3];
	tile->w = width, tile->h = height;
}
void parse_tile_very_simple(int16_t *data, struct tile *tile,
		unsigned w, unsigned h, unsigned img_x, unsigned img_y)
{
	tile->x = data[0], tile->y = data[1];
	tile->img_x = img_x, tile->img_y = img_y;
	tile->w = w, tile->h = h;
}

/*
 * Each of these functions reads one section of dynamic objects from the cgl
 * file. They allocate space for tiles needed by these objects, place the
 * objects there and return a pointer through the pointer in the second argument.
 */

/*
 * All following functions share the same scaffold:
 */
#define BEGIN_CGL_READ_X(what, hdr, obj, howmany) \
int cgl_read_##what(struct cgl *cgl, struct tile **out_tiles,               \
		size_t *ntiles, FILE *fp)                                   \
{                                                                           \
	extern int cgl_read_one_##what(struct obj*, FILE*);                 \
	uint32_t num;                                                       \
	int err;                                                            \
	err = cgl_read_section_header(#hdr, fp);                            \
	if (err)                                                            \
		return err;                                                 \
	err = read_integer((int32_t*)&num, 1, fp);                          \
	if (err)                                                            \
		goto error;                                                 \
	cgl->n##obj##s = num;                                               \
	*ntiles = howmany * cgl->n##obj##s;                                 \
	cgl->obj##s = calloc(num, sizeof(*cgl->obj##s));                    \
	struct tile *tiles = calloc(howmany * num, sizeof(*tiles));         \
	*out_tiles = tiles;                                                 \
	for (size_t i = 0; i < num; ++i) {

#define END_CGL_READ_X(what, hdr, obj, howmany) \
		err = cgl_read_one_##what(&cgl->obj##s[i], fp);             \
		if (err)                                                    \
			goto error;                                         \
	}                                                                   \
	return 0;                                                           \
error:                                                                      \
	SDL_SetError("cgl " #hdr " section corrupted (incomplete)");        \
	return -EBAD##hdr;                                                  \
}

BEGIN_CGL_READ_X(vent, VENT, fan, 2)
	cgl->fans[i].base  = &tiles[2*i + 0];
	cgl->fans[i].pipes = &tiles[2*i + 1];
END_CGL_READ_X(vent, VENT, fan, 2)

int cgl_read_one_vent(struct fan *fan, FILE *fp)
{
	int err;
	uint8_t buf[VENT_HDR_SIZE];
	int16_t buf2[VENT_NUM_SHORTS];
	size_t nread;

	nread = fread(buf, sizeof(uint8_t), VENT_HDR_SIZE, fp);
	if (nread < VENT_HDR_SIZE)
		return -EBADVENT;
	err = read_short((int16_t*)buf2, VENT_NUM_SHORTS, fp);
	if (err)
		return -EBADVENT;
	fan->power = (buf[0] >> 4) & 0x01;
	fan->dir = buf[0] & 0x03;
	parse_tile_simple(buf2 + 0x00, fan->base, 48, 48);
	parse_tile(buf2 + 0x04, fan->pipes);
	fan->pipes->collision_test = Bitmap;
	fan->img_x = fan->base->img_x;
	parse_rect(buf2 + 0x0a, &fan->bbox);
	parse_rect(buf2 + 0x0e, &fan->range);
	return 0;
}

BEGIN_CGL_READ_X(magn, MAGN, magnet, 2)
	cgl->magnets[i].base = &tiles[2*i + 0];
	cgl->magnets[i].magn = &tiles[2*i + 1];
END_CGL_READ_X(magn, MAGN, magnet, 2)

int cgl_read_one_magn(struct magnet *magnet, FILE *fp)
{
	int err;
	uint8_t buf[MAGN_HDR_SIZE];
	int16_t buf2[MAGN_NUM_SHORTS];
	size_t nread;

	nread = fread(buf, sizeof(uint8_t), MAGN_HDR_SIZE, fp);
	if (nread < MAGN_HDR_SIZE)
		return -EBADMAGN;
	err = read_short((int16_t*)buf2, MAGN_NUM_SHORTS, fp);
	if (err)
		return -EBADMAGN;
	magnet->dir = buf[0] & 0x03;
	parse_tile_simple(buf2 + 0x00, magnet->base, 32, 32);
	parse_tile(buf2 + 0x04, magnet->magn);
	magnet->magn->collision_test = Bitmap;
	magnet->img_x = magnet->magn->img_x;
	parse_rect(buf2 + 0x0a, &magnet->bbox);
	parse_rect(buf2 + 0x0e, &magnet->range);
	return 0;
}

BEGIN_CGL_READ_X(dist, DIST, airgen, 2)
	cgl->airgens[i].base  = &tiles[2*i + 0];
	cgl->airgens[i].pipes = &tiles[2*i + 1];
END_CGL_READ_X(dist, DIST, airgen, 2)

int cgl_read_one_dist(struct airgen *airgen, FILE *fp)
{
	int err;
	uint8_t buf[DIST_HDR_SIZE];
	int16_t buf2[DIST_NUM_SHORTS];
	size_t nread;

	nread = fread(buf, sizeof(uint8_t), DIST_HDR_SIZE, fp);
	if (nread < DIST_HDR_SIZE)
		return -EBADDIST;
	err = read_short((int16_t*)buf2, DIST_NUM_SHORTS, fp);
	if (err)
		return -EBADDIST;
	airgen->spin = (buf[0] >> 4) & 0x01;
	airgen->dir = buf[0] & 0x03;
	parse_tile_simple(buf2 + 0x00, airgen->base, 40, 40);
	parse_tile(buf2 + 0x04, airgen->pipes);
	airgen->pipes->collision_test = Bitmap;
	airgen->img_x = airgen->base->img_x;
	parse_rect(buf2 + 0x0a, &airgen->bbox);
	parse_rect(buf2 + 0x0e, &airgen->range);
	return 0;
}

BEGIN_CGL_READ_X(cano, CANO, cannon, 4)
	cgl->cannons[i].beg_base  = &tiles[4*i + 0];
	cgl->cannons[i].beg_cano  = &tiles[4*i + 1];
	cgl->cannons[i].end_base  = &tiles[4*i + 2];
	cgl->cannons[i].end_catch = &tiles[4*i + 3];
END_CGL_READ_X(cano, CANO, cannon, 4)

int cgl_read_one_cano(struct cannon *cannon, FILE *fp)
{
	int err;
	uint8_t buf[CANO_HDR_SIZE];
	int16_t buf2[CANO_NUM_SHORTS];
	size_t nread;

	nread = fread(buf, sizeof(uint8_t), CANO_HDR_SIZE, fp);
	if (nread < CANO_HDR_SIZE)
		return -EBADCANO;
	err = read_short((int16_t*)buf2, 1, fp);
	if (err)
		return -EBADCANO;
	cannon->fire_rate = buf2[0];
	nread = fread(buf, sizeof(uint8_t), 2, fp);
	if (nread < 2)
		return -EBADCANO;
	cannon->speed_x = (int8_t)buf[0];
	cannon->speed_y = (int8_t)buf[1];
	err = read_short((int16_t*)buf2, CANO_NUM_SHORTS, fp);
	if (err)
		return -EBADCANO;
	cannon->dir = buf[0] & 0x03;
	parse_point(buf2 + 0x00, &cannon->beg, &cannon->end);
	parse_tile_very_simple(buf2 + 0x04, cannon->beg_base,
			24, 24, 512, 188);
	parse_tile_simple(buf2 + 0x06, cannon->beg_cano,
			16, 16);
	cannon->beg_cano->collision_test = Bitmap;
	parse_tile_very_simple(buf2 + 0x0a, cannon->end_base,
			16, 16, 472, 196);
	parse_tile(buf2 + 0x0c, cannon->end_catch);
	cannon->end_catch->collision_test = Bitmap;
	parse_rect(buf2 + 0x12, &cannon->bbox);
	return 0;
}

BEGIN_CGL_READ_X(pipe, PIPE, bar, 4)
	cgl->bars[i].beg  = &tiles[4*i + 0];
	cgl->bars[i].end  = &tiles[4*i + 1];
	cgl->bars[i].fbar = &tiles[4*i + 2];
	cgl->bars[i].sbar = &tiles[4*i + 3];
END_CGL_READ_X(pipe, PIPE, bar, 4)

int cgl_read_one_pipe(struct bar *bar, FILE *fp)
{
	int err;
	uint8_t buf[PIPE_HDR_SIZE];
	int16_t buf2[PIPE_NUM_SHORTS];
	size_t nread;

	nread = fread(buf, sizeof(uint8_t), PIPE_HDR_SIZE, fp);
	if (nread < PIPE_HDR_SIZE)
		return -EBADPIPE;
	err = read_short((int16_t*)buf2, PIPE_NUM_SHORTS, fp);
	if (err)
		return -EBADPIPE;
	bar->gap_type = (buf[0] >> 4) & 0x01;
	bar->orientation = buf[0] & 0x01;
	bar->gap = buf[2];
	bar->min_s = buf[6] - 1;
	bar->max_s = buf[7] - 1;
	bar->speed = 1; /* anything, not 0 */
	bar->freq = buf[10] & 0x01;
	int width = buf2[2],
	    height = buf2[3];
	switch (bar->orientation) {
	case Vertical:
		parse_tile_very_simple(buf2, bar->beg,
				20, 24, 496, 56);
		bar->bimg_x = 496;
		bar->eimg_x = 496;
		bar->end->x = bar->beg->x;
		bar->end->y = bar->beg->y + height - 24;
		bar->end->w = bar->beg->w; bar->end->h = bar->beg->h;
		bar->end->img_y = 52;
		/* fbar and sbar must take the whole space available, so that
		 * cgl_preprocess assigns them to all blocks where they may
		 * appear */
		bar->fbar->x = bar->beg->x + 4;
		bar->fbar->y = bar->beg->y + 24;
		bar->fbar->w = 12, bar->fbar->h = height - 2*24;
		bar->fbar->img_x = 552, bar->fbar->img_y = 308 - bar->fbar->h;
		bar->sbar->x = bar->beg->x + 4;
		bar->sbar->y = bar->beg->y + 24;
		bar->sbar->w = 12, bar->sbar->h = height - 2*24;
		bar->sbar->img_x = 552, bar->sbar->img_y = 0;
		bar->len = height - 2*24;
		break;
	case Horizontal:
		parse_tile_very_simple(buf2, bar->beg,
				24, 20, 496, 56);
		bar->bimg_x = 496;
		bar->eimg_x = 492;
		bar->end->x = bar->beg->x + width - 24;
		bar->end->y = bar->beg->y;
		bar->end->w = bar->beg->w; bar->end->h = bar->beg->h;
		bar->end->img_y = 56;
		/* Same as in case Vertical */
		bar->fbar->x = bar->beg->x + 24;
		bar->fbar->y = bar->beg->y + 4;
		bar->fbar->w = width - 2*24, bar->fbar->h = 12;
		bar->fbar->img_x = 548 - bar->fbar->w, bar->fbar->img_y = 80;
		bar->sbar->x = bar->beg->x + 24;
		bar->sbar->y = bar->beg->y + 4;
		bar->sbar->w = width - 2*24, bar->sbar->h = 12;
		bar->sbar->img_x = 240, bar->sbar->img_y = 80;
		bar->len = width - 2*24;
		break;
	}
	switch (bar->gap_type) {
	case Constant:
		bar->fbar_len = 0;
		break;
	}
	bar->beg->collision_test = bar->end->collision_test = Bitmap;
	return 0;
}

/* ------------------------------------------------------------------------*/

void cgl_preprocess(struct cgl *cgl)
{
	unsigned width_px = cgl->width * CGL_BLOCK_SIZE,
		 height_px = cgl->height * CGL_BLOCK_SIZE;
	/* express the dimensions of level in new block units (BLOCK_SIZE
	 * instead of CGL_BLOCK_SIZE */
	cgl->width = (size_t)ceil((double)width_px / BLOCK_SIZE);
	cgl->height = (size_t)ceil((double)height_px / BLOCK_SIZE);
	size_t *sizes = calloc(cgl->width * cgl->height, sizeof(*sizes)),
	       *is = calloc(cgl->width * cgl->height, sizeof(*is));
	for (size_t k = 0; k < cgl->ntiles; ++k) {
		size_t x = cgl->tiles[k].x / BLOCK_SIZE,
		       y = cgl->tiles[k].y / BLOCK_SIZE;
		assert(x < cgl->width);
		assert(y < cgl->height);
		for (size_t j = y; j*BLOCK_SIZE < cgl->tiles[k].y +
				cgl->tiles[k].h; ++j)
			for (size_t i = x; i*BLOCK_SIZE < cgl->tiles[k].x +
					cgl->tiles[k].w; ++i)
				sizes[i + j * cgl->width]++;
	}
	cgl->blocks = calloc(cgl->height, sizeof(*cgl->blocks));
	for (size_t j = 0; j < cgl->height; ++j) {
		cgl->blocks[j] = calloc(cgl->width, sizeof(**cgl->blocks));
		for (size_t i = 0; i < cgl->width; ++i)
			cgl->blocks[j][i] = calloc(sizes[i + j*cgl->width] + 1,
					sizeof(**cgl->blocks));
	}
	for (size_t k = 0; k < cgl->ntiles; ++k) {
		size_t x = cgl->tiles[k].x / BLOCK_SIZE,
		       y = cgl->tiles[k].y / BLOCK_SIZE;
		for (size_t j = y; j*BLOCK_SIZE < cgl->tiles[k].y +
				cgl->tiles[k].h; ++j)
			for (size_t i = x; i*BLOCK_SIZE < cgl->tiles[k].x +
					cgl->tiles[k].w; ++i)
				cgl->blocks[j][i][is[i + j*cgl->width]++] = &cgl->tiles[k];
	}
	for (size_t j = 0; j < cgl->height; ++j)
		for (size_t i = 0; i < cgl->width; ++i)
			cgl->blocks[j][i][is[i + j*cgl->width]] = NULL;
	free(sizes);
	free(is);
}
