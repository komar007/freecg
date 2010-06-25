/* cgl.c - CGL level parser for FreeCG
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

#include "cgl.h"
#include "cg.h"
#include "mathgeom.h"
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
		   cgl_read_pipe(struct cgl*, struct tile**, size_t*, FILE*),
		   cgl_read_onew(struct cgl*, struct tile**, size_t*, FILE*),
		   cgl_read_barr(struct cgl*, struct tile**, size_t*, FILE*),
		   cgl_read_lpts(struct cgl*, struct tile**, size_t*, FILE*);
	struct cgl *cgl;
	FILE *fp;
	uint8_t *soin = NULL;
	fp = fopen(path, "rb");
	if (!fp) {
		SDL_SetError("fopen: %s", strerror(errno));
		return NULL;
	}
	cgl = calloc(1, sizeof(*cgl));
	cgl->tiles   = NULL;
	cgl->fans    = NULL;
	cgl->magnets = NULL;
	cgl->airgens = NULL;
	cgl->cannons = NULL;
	cgl->bars    = NULL;
	cgl->gates   = NULL;
	cgl->blocks  = NULL;
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
		    *pipe_tiles, *onew_tiles, *barr_tiles, *lpts_tiles;
	size_t nvent_tiles, nmagn_tiles, ndist_tiles, ncano_tiles,
	       npipe_tiles, nonew_tiles, nbarr_tiles, nlpts_tiles;
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
	if (cgl_read_magic(cgl, fp) != 0)
		goto error;
	if (cgl_read_onew(cgl, &onew_tiles, &nonew_tiles, fp) != 0)
		goto error;
	if (cgl_read_magic(cgl, fp) != 0)
		goto error;
	if (cgl_read_barr(cgl, &barr_tiles, &nbarr_tiles, fp) != 0)
		goto error;
	if (cgl_read_magic(cgl, fp) != 0)
		goto error;
	if (cgl_read_lpts(cgl, &lpts_tiles, &nlpts_tiles, fp) != 0)
		goto error;
	/* join extra tiles from the other sections with those from SOBS,
	 * fix pointers to point to the new memory */
	size_t num_tiles = cgl->ntiles + (nvent_tiles + nmagn_tiles +
			ndist_tiles + ncano_tiles + npipe_tiles +
			nonew_tiles + nbarr_tiles + nlpts_tiles);
	cgl->tiles = realloc(cgl->tiles, num_tiles * sizeof(*cgl->tiles));
	memcpy(cgl->tiles + cgl->ntiles, vent_tiles,
			nvent_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->fans,    base,      cgl->nfans,    vent_tiles)
	FIX_PTRS(cgl->fans,    pipes,     cgl->nfans,    vent_tiles)
	FIX_PTRS(cgl->fans,    act,       cgl->nfans,    vent_tiles)
	cgl->ntiles += nvent_tiles;
	free(vent_tiles);
	memcpy(cgl->tiles + cgl->ntiles, magn_tiles,
			nmagn_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->magnets, base,      cgl->nmagnets, magn_tiles)
	FIX_PTRS(cgl->magnets, magn,      cgl->nmagnets, magn_tiles)
	FIX_PTRS(cgl->magnets, act,       cgl->nmagnets, magn_tiles)
	cgl->ntiles += nmagn_tiles;
	free(magn_tiles);
	memcpy(cgl->tiles + cgl->ntiles, dist_tiles,
			ndist_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->airgens, base,      cgl->nairgens, dist_tiles)
	FIX_PTRS(cgl->airgens, pipes,     cgl->nairgens, dist_tiles)
	FIX_PTRS(cgl->airgens, act,       cgl->nairgens, dist_tiles)
	cgl->ntiles += ndist_tiles;
	free(dist_tiles);
	memcpy(cgl->tiles + cgl->ntiles, cano_tiles,
			ncano_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->cannons, beg_base,  cgl->ncannons, cano_tiles)
	FIX_PTRS(cgl->cannons, beg_cano,  cgl->ncannons, cano_tiles)
	FIX_PTRS(cgl->cannons, end_base,  cgl->ncannons, cano_tiles)
	FIX_PTRS(cgl->cannons, end_catch, cgl->ncannons, cano_tiles)
	cgl->ntiles += ncano_tiles;
	free(cano_tiles);
	memcpy(cgl->tiles + cgl->ntiles, pipe_tiles,
			npipe_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->bars,    beg,       cgl->nbars,    pipe_tiles)
	FIX_PTRS(cgl->bars,    end,       cgl->nbars,    pipe_tiles)
	FIX_PTRS(cgl->bars,    fbar,      cgl->nbars,    pipe_tiles)
	FIX_PTRS(cgl->bars,    sbar,      cgl->nbars,    pipe_tiles)
	cgl->ntiles += npipe_tiles;
	free(pipe_tiles);
	memcpy(cgl->tiles + cgl->ntiles, onew_tiles,
			nonew_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->gates,   base[0],  cgl->ngates,    onew_tiles)
	FIX_PTRS(cgl->gates,   base[1],  cgl->ngates,    onew_tiles)
	FIX_PTRS(cgl->gates,   base[2],  cgl->ngates,    onew_tiles)
	FIX_PTRS(cgl->gates,   base[3],  cgl->ngates,    onew_tiles)
	FIX_PTRS(cgl->gates,   base[4],  cgl->ngates,    onew_tiles)
	FIX_PTRS(cgl->gates,   bar,      cgl->ngates,    onew_tiles)
	FIX_PTRS(cgl->gates,   arrow,    cgl->ngates,    onew_tiles)
	FIX_PTRS(cgl->gates,   act,      cgl->ngates,    onew_tiles)
	cgl->ntiles += nonew_tiles;
	free(onew_tiles);
	memcpy(cgl->tiles + cgl->ntiles, barr_tiles,
			nbarr_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->lgates,  base[0],  cgl->nlgates,   barr_tiles)
	FIX_PTRS(cgl->lgates,  base[1],  cgl->nlgates,   barr_tiles)
	FIX_PTRS(cgl->lgates,  base[2],  cgl->nlgates,   barr_tiles)
	FIX_PTRS(cgl->lgates,  base[3],  cgl->nlgates,   barr_tiles)
	FIX_PTRS(cgl->lgates,  base[4],  cgl->nlgates,   barr_tiles)
	FIX_PTRS(cgl->lgates,  bar,      cgl->nlgates,   barr_tiles)
	FIX_PTRS(cgl->lgates,  light[0], cgl->nlgates,   barr_tiles)
	FIX_PTRS(cgl->lgates,  light[1], cgl->nlgates,   barr_tiles)
	FIX_PTRS(cgl->lgates,  light[2], cgl->nlgates,   barr_tiles)
	FIX_PTRS(cgl->lgates,  light[3], cgl->nlgates,   barr_tiles)
	FIX_PTRS(cgl->lgates,  act,      cgl->nlgates,   barr_tiles)
	cgl->ntiles += nbarr_tiles;
	free(barr_tiles);
	memcpy(cgl->tiles + cgl->ntiles, lpts_tiles,
			nlpts_tiles * sizeof(*cgl->tiles));
	FIX_PTRS(cgl->airports,  base,      cgl->nairports,   lpts_tiles)
	FIX_PTRS(cgl->airports,  stripe[0], cgl->nairports,   lpts_tiles)
	FIX_PTRS(cgl->airports,  stripe[1], cgl->nairports,   lpts_tiles)
	FIX_PTRS(cgl->airports,  arrow[0],  cgl->nairports,   lpts_tiles)
	FIX_PTRS(cgl->airports,  arrow[1],  cgl->nairports,   lpts_tiles)
	for (size_t k = 0; k < 10; ++k)
		FIX_PTRS(cgl->airports, cargo[k], cgl->nairports, lpts_tiles);
	cgl->ntiles += nlpts_tiles;
	free(lpts_tiles);
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
		tiles[k].tex_y = UNIT * buf[2];
		tiles[k].tex_x = UNIT * buf[3];
	}
	return 0;
}

/* Auxilliary functions to extract a single rectangle or tile description from
 * an array of int16_t */
inline void set_dims(struct tile *t, int x, int y, int w, int h,
		int tex_x, int tex_y)
{
	t->x = x, t->y = y;
	t->w = w, t->h = h;
	t->tex_x = tex_x, t->tex_y = tex_y;
}
inline void set_type(struct tile *t, enum type type, enum collision_test test,
		enum collision_type action, void *data)
{
	t->type = type;
	t->collision_test = test;
	t->collision_type = action;
	t->data = data;
}
inline void parse_point(const int16_t *data, vector *a, vector *b)
{
	a->x = data[0], a->y = data[1];
	b->x = data[2], b->y = data[3];
}
inline void parse_rect(const int16_t *data, struct rect *rect)
{
	rect->x = data[0], rect->y = data[1];
	rect->w = data[2], rect->h = data[3];
}
inline void parse_tile_normal(const int16_t *data, struct tile *tile)
{
	tile->x = data[0], tile->y = data[1];
	tile->w = data[2], tile->h = data[3];
	tile->tex_x = data[4], tile->tex_y = data[5];
}
inline void parse_tile_simple(const int16_t *data, struct tile *tile,
		unsigned width, unsigned height)
{
	tile->x = data[0], tile->y = data[1];
	tile->tex_x = data[2], tile->tex_y = data[3];
	tile->w = width, tile->h = height;
}
inline void parse_tile_minimal(const int16_t *data, struct tile *tile,
		unsigned w, unsigned h, unsigned tex_x, unsigned tex_y)
{
	tile->x = data[0], tile->y = data[1];
	tile->tex_x = tex_x, tile->tex_y = tex_y;
	tile->w = w, tile->h = h;
}

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
	for (size_t i = 0; i < howmany * num; ++i)                          \
		tiles[i].z = DYN_TILES_Z;                                             \
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

/*
 * Each of these functions reads one section of dynamic objects from the cgl
 * file. They allocate space for tiles needed by these objects, place the
 * objects there and return a pointer through the pointer in the second argument.
 */
BEGIN_CGL_READ_X(vent, VENT, fan, 3)
	cgl->fans[i].base  = &tiles[3*i + 0];
	cgl->fans[i].pipes = &tiles[3*i + 1];
	cgl->fans[i].act   = &tiles[3*i + 2];
END_CGL_READ_X(vent, VENT, fan, 3)

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
	fan->dir   = (buf[0] >> 0) & 0x03;
	fan->power = (buf[0] >> 4) & 0x01;
	parse_tile_simple(buf2 + 0x00, fan->base, 48, 48);
	fan->tex_x = fan->base->tex_x;
	parse_tile_normal(buf2 + 0x04, fan->pipes);
	fan->pipes->collision_test = Bitmap;
	struct rect r;
	parse_rect(buf2 + 0x0e, &r);
	rect_to_tile(&r, fan->act);
	set_type(fan->act, Transparent, RectPoint, FanAction, fan);
	return 0;
}

BEGIN_CGL_READ_X(magn, MAGN, magnet, 3)
	cgl->magnets[i].base = &tiles[3*i + 0];
	cgl->magnets[i].magn = &tiles[3*i + 1];
	cgl->magnets[i].act  = &tiles[3*i + 2];
END_CGL_READ_X(magn, MAGN, magnet, 3)

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
	parse_tile_normal(buf2 + 0x04, magnet->magn);
	magnet->tex_x = magnet->magn->tex_x;
	magnet->magn->collision_test = Bitmap;
	struct rect r;
	parse_rect(buf2 + 0x0e, &r);
	rect_to_tile(&r, magnet->act);
	set_type(magnet->act, Transparent, RectPoint, MagnetAction, magnet);
	return 0;
}

BEGIN_CGL_READ_X(dist, DIST, airgen, 3)
	cgl->airgens[i].base  = &tiles[3*i + 0];
	cgl->airgens[i].pipes = &tiles[3*i + 1];
	cgl->airgens[i].act   = &tiles[3*i + 2];
END_CGL_READ_X(dist, DIST, airgen, 3)

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
	airgen->dir  = (buf[0] >> 0) & 0x03;
	airgen->spin = (buf[0] >> 4) & 0x01;
	parse_tile_simple(buf2 + 0x00, airgen->base, 40, 40);
	airgen->tex_x = airgen->base->tex_x;
	parse_tile_normal(buf2 + 0x04, airgen->pipes);
	airgen->pipes->collision_test = Bitmap;
	struct rect r;
	parse_rect(buf2 + 0x0e, &r);
	rect_to_tile(&r, airgen->act);
	set_type(airgen->act, Transparent, RectPoint, AirgenAction, airgen);
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
	parse_tile_minimal(buf2 + 0x04, cannon->beg_base,
			24, 24, 512, 188);
	parse_tile_simple(buf2 + 0x06, cannon->beg_cano,
			16, 16);
	cannon->beg_cano->collision_test = Bitmap;
	parse_tile_minimal(buf2 + 0x0a, cannon->end_base,
			16, 16, 472, 196);
	parse_tile_normal(buf2 + 0x0c, cannon->end_catch);
	cannon->end_catch->collision_test = Bitmap;
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
	bar->orientation = (buf[0] >> 0) & 0x01;
	bar->gap_type    = (buf[0] >> 4) & 0x01;
	bar->gap    = buf[2];
	bar->min_s  = buf[6] - 1;
	bar->max_s  = buf[7] - 1;
	bar->freq   = buf[10] & 0x01;
	int width   = buf2[2],
	    height  = buf2[3];
	/* prepare beg, end, fbar and sbar tiles */
	switch (bar->orientation) {
	case Vertical:
		parse_tile_minimal(buf2, bar->beg,
			BAR_BASE_H, BAR_BASE_W, BAR_BEG_TEX_X, BAR_BEG_TEX_Y);
		set_dims(bar->end,
			bar->beg->x, bar->beg->y + height - BAR_BASE_W,
			bar->beg->w, bar->beg->h,
			VBAR_END_TEX_X, VBAR_END_TEX_Y);
		/* fbar and sbar must take the whole space available, so that
		 * cgl_preprocess assigns them to all blocks where they may
		 * appear */
		set_dims(bar->fbar,
			bar->beg->x + (BAR_BASE_H - BAR_THICKNESS)/2,
			bar->beg->y + BAR_BASE_W,
			BAR_THICKNESS, height - 2*BAR_BASE_W,
			VBAR_TEX_X, VBAR_TEX_Y + BAR_TEX_LEN - (height - 2*BAR_BASE_W));
		set_dims(bar->sbar,
			bar->beg->x + (BAR_BASE_H - BAR_THICKNESS)/2,
			bar->beg->y + BAR_BASE_W,
			BAR_THICKNESS, height - 2*BAR_BASE_W,
			VBAR_TEX_X, VBAR_TEX_Y),
		bar->len = height - 2*BAR_BASE_W;
		break;
	case Horizontal:
		parse_tile_minimal(buf2, bar->beg,
				BAR_BASE_W, BAR_BASE_H, BAR_BEG_TEX_X, BAR_BEG_TEX_Y);
		set_dims(bar->end,
			bar->beg->x + width - BAR_BASE_W, bar->beg->y,
			bar->beg->w, bar->beg->h,
			HBAR_END_TEX_X, HBAR_END_TEX_Y);
		/* Same as in case Vertical */
		set_dims(bar->fbar,
			bar->beg->x + BAR_BASE_W,
			bar->beg->y + (BAR_BASE_H - BAR_THICKNESS)/2,
			width - 2*BAR_BASE_W, BAR_THICKNESS,
			HBAR_TEX_X + BAR_TEX_LEN - (width - 2*BAR_BASE_W), HBAR_TEX_Y);
		set_dims(bar->sbar,
			bar->beg->x + BAR_BASE_W,
			bar->beg->y + (BAR_BASE_H - BAR_THICKNESS)/2,
			width - 2*BAR_BASE_W, BAR_THICKNESS,
			HBAR_TEX_X, HBAR_TEX_Y);
		bar->len = width - 2*BAR_BASE_W;
		break;
	}
	bar->btex_x = bar->beg->tex_x;
	bar->etex_x = bar->end->tex_x;
	bar->slen = BAR_MIN_LEN;
	bar->flen = BAR_MIN_LEN;
	bar->beg->collision_test = bar->end->collision_test = Bitmap;
	return 0;
}

BEGIN_CGL_READ_X(onew, ONEW, gate, 8)
	cgl->gates[i].base[0]  = &tiles[8*i + 0];
	cgl->gates[i].base[1]  = &tiles[8*i + 1];
	cgl->gates[i].base[2]  = &tiles[8*i + 2];
	cgl->gates[i].base[3]  = &tiles[8*i + 3];
	cgl->gates[i].base[4]  = &tiles[8*i + 4];
	cgl->gates[i].bar      = &tiles[8*i + 5];
	cgl->gates[i].arrow    = &tiles[8*i + 6];
	cgl->gates[i].act      = &tiles[8*i + 7];
END_CGL_READ_X(onew, ONEW, gate, 8)

inline void parse_packed_tiles(const int16_t *data, size_t num, struct tile *tiles[],
		const vector *dims)
{
	for (size_t i = 0; i < num; ++i)
		tiles[i]->x = data[0*num + i];
	for (size_t i = 0; i < num; ++i)
		tiles[i]->y = data[1*num + i];
	for (size_t i = 0; i < num; ++i)
		tiles[i]->tex_x = data[2*num + i];
	for (size_t i = 0; i < num; ++i)
		tiles[i]->tex_y = data[3*num + i];
	for (size_t i = 0; i < num; ++i) {
		tiles[i]->w = dims[i].x;
		tiles[i]->h = dims[i].y;
	}
}

int cgl_read_one_onew(struct gate *gate, FILE *fp)
{
	static const vector base_dims[][5] = {
		{{32, 32}, {32, 16}, {32, 32}, {40, 4}, {32, 20}},
		{{32, 32}, {16, 32}, {32, 32}, {4, 40}, {20, 32}}
	};
	int err;
	uint8_t buf[ONEW_HDR_SIZE];
	int16_t buf2[ONEW_NUM_SHORTS];
	size_t nread;

	nread = fread(buf, sizeof(uint8_t), ONEW_HDR_SIZE, fp);
	if (nread < ONEW_HDR_SIZE)
		return -EBADONEW;
	err = read_short((int16_t*)buf2, ONEW_NUM_SHORTS, fp);
	if (err)
		return -EBADONEW;
	/* fill with header information */
	gate->type    = (buf[0] >> 0) & 0x03;
	gate->dir     = (buf[0] >> 2) & 0x01;
	gate->has_end = (buf[0] >> 3) & 0x01;
	gate->orient  = (buf[0] >> 4) & 0x01;
	assert(buf2[0] == buf2[1]);
	gate->len = gate->max_len = buf2[0];
	parse_packed_tiles(buf2 + 0x02, 5, gate->base, base_dims[gate->orient]);
	/* prepare gate's bar */
	switch (gate->orient) {
	case Vertical:
		parse_tile_minimal(buf2 + 0x16, gate->bar,
				GATE_BAR_THICKNESS, gate->len,
				VGATE_TEX_X, VGATE_TEX_Y);
		break;
	case Horizontal:
		parse_tile_minimal(buf2 + 0x16, gate->bar,
				gate->len, GATE_BAR_THICKNESS,
				HGATE_TEX_X, HGATE_TEX_Y);
		break;
	}
	gate->bar->collision_test = Bitmap;
	struct rect r;
	parse_rect(buf2 + 0x1c, &r);
	rect_to_tile(&r, gate->act);
	set_type(gate->act, Transparent, RectPoint, GateAction, gate);
	if (!gate->has_end)
		set_type(gate->base[4], Transparent, NoCollision, 0, NULL);
	if (gate->type == GateLeft)
		gate->bar->tex_x += GATE_BAR_LEN - gate->len;
	if (gate->type == GateTop)
		gate->bar->tex_y += GATE_BAR_LEN - gate->len;
	/* add blue arrow */
	gate->arrow->w = 16;
	gate->arrow->h = 16;
	switch (gate->dir) {
	case 0:
		gate->arrow->x = gate->base[0]->x + ARROW_OFFSET;
		gate->arrow->y = gate->base[0]->y + ARROW_OFFSET;
		break;
	case 1:
		gate->arrow->x = gate->base[2]->x + ARROW_OFFSET;
		gate->arrow->y = gate->base[2]->y + ARROW_OFFSET;
		break;
	}
	int arrow_dir = gate->type - 1;
	if (gate->dir == 1)
		arrow_dir += 2;
	arrow_dir = (arrow_dir + 4) % 4;
	gate->arrow->tex_y = ARROW_TEX_Y;
	gate->arrow->tex_x = ARROW_SIDE * arrow_dir;
	gate->arrow->z = DYN_TILES_OVERLAY_Z;
	return 0;
}

BEGIN_CGL_READ_X(barr, BARR, lgate, 11)
	cgl->lgates[i].base[0]  = &tiles[11*i + 0];
	cgl->lgates[i].base[1]  = &tiles[11*i + 1];
	cgl->lgates[i].base[2]  = &tiles[11*i + 2];
	cgl->lgates[i].base[3]  = &tiles[11*i + 3];
	cgl->lgates[i].base[4]  = &tiles[11*i + 4];
	cgl->lgates[i].bar      = &tiles[11*i + 5];
	cgl->lgates[i].light[0] = &tiles[11*i + 6];
	cgl->lgates[i].light[1] = &tiles[11*i + 7];
	cgl->lgates[i].light[2] = &tiles[11*i + 8];
	cgl->lgates[i].light[3] = &tiles[11*i + 9];
	cgl->lgates[i].act      = &tiles[11*i + 10];
END_CGL_READ_X(barr, BARR, lgate, 11)

void set_light_tile(struct tile *light, int num, int x, int y)
{
	set_dims(light, x, y, 8, 8, LIGHTS_TEX_X + num*8, LIGHTS_TEX_Y);
	set_type(light, Transparent, NoCollision, 0, NULL);
	light->z = DYN_TILES_OVERLAY_Z;
}
int cgl_read_one_barr(struct lgate *lgate, FILE *fp)
{
	static const vector base_dims[][5] = {
		{{32, 32}, {24, 16}, {0, 0}, {40, 4}, {32, 20}},
		{{32, 32}, {16, 24}, {0, 0}, {4, 40}, {20, 32}}
	};
	int err;
	uint8_t buf[ONEW_HDR_SIZE];
	int16_t buf2[ONEW_NUM_SHORTS];
	size_t nread;

	nread = fread(buf, sizeof(uint8_t), ONEW_HDR_SIZE, fp);
	if (nread < ONEW_HDR_SIZE)
		return -EBADBARR;
	err = read_short((int16_t*)buf2, ONEW_NUM_SHORTS, fp);
	if (err)
		return -EBADBARR;
	/* fill with header information */
	lgate->type    = (buf[0] >> 0) & 0x03;
	lgate->has_end = (buf[0] >> 2) & 0x01;
	lgate->keys[0] = (buf[0] >> 7) & 0x01;
	lgate->keys[1] = (buf[0] >> 6) & 0x01;
	lgate->keys[2] = (buf[0] >> 5) & 0x01;
	lgate->keys[3] = (buf[0] >> 4) & 0x01;
	if (lgate->type == GateTop || lgate->type == GateBottom)
		lgate->orient = Vertical;
	else
		lgate->orient = Horizontal;
	assert(buf2[0] == buf2[1]);
	lgate->len = lgate->max_len = buf2[0];
	parse_packed_tiles(buf2 + 0x02, 5, lgate->base, base_dims[lgate->orient]);
	/* prepare lgate's bar */
	switch (lgate->orient) {
	case Vertical:
		parse_tile_minimal(buf2 + 0x16, lgate->bar,
				GATE_BAR_THICKNESS, lgate->len,
				LVGATE_TEX_X, LVGATE_TEX_Y);
		break;
	case Horizontal:
		parse_tile_minimal(buf2 + 0x16, lgate->bar,
				lgate->len, GATE_BAR_THICKNESS,
				LHGATE_TEX_X, LHGATE_TEX_Y);
		break;
	}
	lgate->bar->collision_test = Bitmap;
	struct rect r;
	parse_rect(buf2 + 0x1c, &r);
	rect_to_tile(&r, lgate->act);
	set_type(lgate->act, Transparent, RectPoint, LGateAction, lgate);
	if (!lgate->has_end)
		set_type(lgate->base[4], Transparent, NoCollision, 0, NULL);
	if (lgate->type == GateLeft)
		lgate->bar->tex_x += GATE_BAR_LEN - lgate->len;
	if (lgate->type == GateTop)
		lgate->bar->tex_y += GATE_BAR_LEN - lgate->len;
	set_light_tile(lgate->light[0], 0,
			lgate->base[0]->x + 6,  lgate->base[0]->y + 6);
	set_light_tile(lgate->light[1], 1,
			lgate->base[0]->x + 18, lgate->base[0]->y + 6);
	set_light_tile(lgate->light[2], 2,
			lgate->base[0]->x + 6,  lgate->base[0]->y + 18);
	set_light_tile(lgate->light[3], 3,
			lgate->base[0]->x + 18, lgate->base[0]->y + 18);
	return 0;
}

BEGIN_CGL_READ_X(lpts, LPTS, airport, 15)
	cgl->airports[i].base      = &tiles[15*i + 0];
	cgl->airports[i].stripe[0] = &tiles[15*i + 1];
	cgl->airports[i].stripe[1] = &tiles[15*i + 2];
	cgl->airports[i].arrow[0]  = &tiles[15*i + 3];
	cgl->airports[i].arrow[1]  = &tiles[15*i + 4];
	for (size_t k = 0; k < 10; ++k)
		cgl->airports[i].cargo[k] = &tiles[15*i + 5+k];
END_CGL_READ_X(lpts, LPTS, airport, 15)

int cgl_read_one_lpts(struct airport *airport, FILE *fp)
{
	static const int16_t larrow_data[] = {0, 0, 24, 32, 232, 360},
		             rarrow_data[] = {0, 0, 24, 32, 232, 392};
	int err;
	uint8_t buf[LPTS_NUM_STUFF*3];
	int16_t buf2[LPTS_NUM_SHORTS];
	size_t nread;

	nread = fread(buf, sizeof(uint8_t), LPTS_HDR_SIZE, fp);
	if (nread < LPTS_HDR_SIZE)
		return -EBADLPTS;
	airport->type = buf[0] & 0x0f;
	if (airport->type == Key) {
		airport->c.key = buf[0] >> 4;
	} else {
		airport->has_left_arrow  = (buf[0] >> 4) & 0x01;
		airport->has_right_arrow = (buf[0] >> 5) & 0x01;
	}
	err = read_short((int16_t*)buf2, LPTS_NUM_SHORTS, fp);
	if (err)
		return -EBADLPTS;
	int stripe_tex_y = buf2[LPTS_NUM_SHORTS - 1];
	parse_tile_minimal(buf2, airport->base,
			buf2[2]*32, 20, buf2[3], buf2[4]);
	set_type(airport->base, Simple, Rect, AirportAction, airport);
	airport->base->y += 32;
	set_dims(airport->stripe[0],
		airport->base->x + STRIPE_OFFS,
		airport->base->y + STRIPE_OFFS,
		airport->base->w - 2*STRIPE_OFFS - STRIPE_END_W, STRIPE_H,
		TILESET_W, stripe_tex_y - STRIPE_ORYG_Y);
	airport->stripe[0]->z = DYN_TILES_OVERLAY_Z;
	set_dims(airport->stripe[1],
		airport->stripe[0]->x + airport->stripe[0]->w,
		airport->stripe[0]->y,
		STRIPE_END_W, STRIPE_H,
		STRIPE_ORYG_X + STRIPE_ORYG_W - STRIPE_END_W, stripe_tex_y);
	airport->stripe[1]->z = DYN_TILES_OVERLAY_Z;
	if (airport->has_left_arrow) {
		parse_tile_normal(larrow_data, airport->arrow[0]);
		airport->arrow[0]->x = airport->base->x;
		airport->arrow[0]->y = airport->base->y - 32;
		set_type(airport->arrow[0], Simple, Bitmap, Kaboom, NULL);
	}
	if (airport->has_right_arrow) {
		parse_tile_normal(rarrow_data, airport->arrow[1]);
		airport->arrow[1]->x = airport->base->x +
			airport->base->w - 24;
		airport->arrow[1]->y = airport->base->y - 32;
		set_type(airport->arrow[1], Simple, Bitmap, Kaboom, NULL);
	}
	nread = fread(buf, sizeof(uint8_t), 1, fp);
	if (nread < 1)
		return -EBADLPTS;
	airport->num_cargo = buf[0];
	nread = fread(buf, sizeof(uint8_t), LPTS_NUM_STUFF*3, fp);
	if (nread < LPTS_NUM_STUFF*3)
		return -EBADLPTS;
	for (size_t i = 0; i < airport->num_cargo; ++i) {
		set_dims(airport->cargo[i],
			airport->base->x + buf[i],
			airport->base->y - 32 + buf[10+i],
			STUFF_SIZE, STUFF_SIZE,
			STUFF_TEX_X + buf[20+i]*16, STUFF_TEX_Y);
		switch (airport->type) {
		case Freigh:
			airport->c.freigh[i] = buf[20+i] - 1;
			break;
		case Extras:
			airport->c.extras[i] = buf[20+i] - 5;
			break;
		case Key:
			airport->cargo[i]->tex_x = KEY_TEX_X;
			airport->cargo[i]->tex_y = KEY_TEX_Y +
				airport->c.key*STUFF_SIZE;
			break;
		case Homebase:
			/* This should never happen - homebase has no cargo */
			assert(airport->type != Homebase);
			break;
		case Fuel:
			/* Do nothing - no special treatment of fuel */
			break;
		}
	}
	err = read_short((int16_t*)buf2, 4, fp);
	if (err)
		return -EBADLPTS;
	parse_rect(buf2, &airport->lbbox);
	return 0;
}

/* ------------------------------------------------------------------------*/

void cgl_preprocess(struct cgl *cgl)
{
	/* below are the expected dimensions of the gamefield. Unfortunately,
	 * some levels have some tiles standing out of the gamefield, thus, a
	 * correction is necessary afterwards */
	unsigned width_px = cgl->width * CGL_BLOCK_SIZE,
		 height_px = cgl->height * CGL_BLOCK_SIZE;
	for (size_t k = 0; k < cgl->ntiles; ++k) {
		width_px = max(width_px, cgl->tiles[k].x + cgl->tiles[k].w);
		height_px = max(height_px, cgl->tiles[k].y + cgl->tiles[k].h);
	}
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
		for (size_t j = y; j*BLOCK_SIZE < (size_t)cgl->tiles[k].y +
				cgl->tiles[k].h; ++j)
			for (size_t i = x; i*BLOCK_SIZE < (size_t)cgl->tiles[k].x +
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
		for (size_t j = y; j*BLOCK_SIZE < (size_t)cgl->tiles[k].y +
				cgl->tiles[k].h; ++j)
			for (size_t i = x; i*BLOCK_SIZE < (size_t)cgl->tiles[k].x +
					cgl->tiles[k].w; ++i)
				cgl->blocks[j][i][is[i + j*cgl->width]++] = &cgl->tiles[k];
	}
	for (size_t j = 0; j < cgl->height; ++j)
		for (size_t i = 0; i < cgl->width; ++i)
			cgl->blocks[j][i][is[i + j*cgl->width]] = NULL;
	free(sizes);
	free(is);
	for (size_t i = 0; i < cgl->nairports; ++i) {
		switch (cgl->airports[i].type) {
		case Homebase:
			cgl->hb = &cgl->airports[i];
			break;
		case Freigh:
			cgl->num_all_freigh += cgl->airports[i].num_cargo;
			break;
		default:
			break;
		}
	}
}
