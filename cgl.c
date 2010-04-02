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
 * 	   version of Crazy Gravity.
 * 	SOBS (width * height * 4 bytes)
 * 		width * height 4-byte structures describing tiles.
 * 		Single tile description:
 * 		offset	(length)
 * 		0x0	(1) position (first half - x, second - y)
 * 		0x1	(1) dimensions (as above)
 * 		0x2	(2) position in gfx file (first byte - x, second - y)
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
 * 	FIXME: others
 * 	-- after the last section a magic 4-byte string may appear which means
 * 	   the level was created by a user in the level editor.
 */

void free_cgl(struct cgl *cgl)
{
	if (!cgl)
		return;
	if (cgl->blocks) {
		for (size_t j = 0; j < cgl->height; ++j) {
			for (size_t i = 0; i < cgl->width; ++i)
				free(cgl->blocks[j][i].tiles);
			free(cgl->blocks[j]);
		}
		free(cgl->blocks);
	}
	free(cgl);
	return;
}

struct cgl *read_cgl(const char *path)
{
	extern int cgl_read_section_header(const char*, FILE*),
		   cgl_read_header(struct cgl*, FILE*),
		   cgl_read_size(struct cgl*, FILE*),
	           cgl_read_soin(struct cgl*, FILE*),
		   cgl_read_magic(struct cgl*, FILE*),
		   cgl_read_sobs(struct cgl *, FILE *);
	struct cgl *cgl;
	FILE *fp;
	fp = fopen(path, "r");
	if (!fp) {
		SDL_SetError("fopen: %s", strerror(errno));
		return NULL;
	}
	cgl = malloc(sizeof(*cgl));
	memset(cgl, 0, sizeof(*cgl));
	if (cgl_read_section_header("CGL1", fp) != 0)
		goto error;
	if (cgl_read_size(cgl, fp) != 0)
		goto error;
	if (cgl_read_soin(cgl, fp) != 0)
		goto error;
	if (cgl_read_magic(cgl, fp) != 0)
		goto error;
	if (cgl_read_sobs(cgl, fp) != 0)
		goto error;

	fclose(fp);
	return cgl;
error:
	fclose(fp);
	free_cgl(cgl);
	return NULL;
}

int cgl_read_section_header(const char *name, FILE *fp)
{
	size_t nread;
	char hdr[4];

	nread = fread(hdr, 1, CGL_SHDR_SIZE, fp);
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
	size_t nread;

	int err = cgl_read_section_header("SIZE", fp);
	if (err)
		return err;
	nread = fread(dims, 2, sizeof(uint32_t), fp);
	if (nread < 2) {
		SDL_SetError("cgl SIZE section corrupted (incomplete)");
		return -EBADSIZE;
	}
	cgl->width  = dims[0];
	cgl->height = dims[1];
	return 0;
}

int cgl_read_soin(struct cgl *cgl, FILE *fp)
{
	size_t nread,
	       nblocks = cgl->height * cgl->width;
	uint8_t *nums, *pnum;

	int err = cgl_read_section_header("SOIN", fp);
	if (err)
		return err;
	pnum = nums = calloc(nblocks, sizeof(*nums));
	nread = fread(nums, 1, nblocks, fp);
	if (nread < nblocks) {
		free(nums);
		SDL_SetError("cgl SOIN section corrupted (incomplete)");
		return -EBADSOIN;
	}
	cgl->blocks = calloc(cgl->height, sizeof(*cgl->blocks));
	for (size_t j = 0; j < cgl->height; ++j) {
		cgl->blocks[j] = calloc(cgl->width,
				sizeof(*cgl->blocks[j]));
		for (size_t i = 0; i < cgl->width; ++i) {
			cgl->blocks[j][i].size = *pnum++ & 0x7f;
			cgl->blocks[j][i].tiles = calloc(cgl->blocks[j][i].size,
					sizeof(*cgl->blocks[j][i].tiles));
		}
	}
	free(nums);
	return 0;
}

int cgl_read_magic(struct cgl *cgl, FILE *fp)
{
	size_t nread;
	uint8_t mgc[4];

	nread = fread(mgc, 1, CGL_MAGIC_SIZE, fp);
	if (nread < CGL_MAGIC_SIZE) {
		fseek(fp, -nread, SEEK_CUR);
		SDL_SetError("cgl corrupted after SOIN section");
		return -EBADSOBS;
	} else if (memcmp(mgc, CGL_MAGIC, CGL_MAGIC_SIZE) != 0) {
		/* Magic absent, level works only in registered version */
		fseek(fp, -nread, SEEK_CUR);
		cgl->type = FULL;
	} else {
		/* Magic present, level works also in unregistered version */
		cgl->type = DEMO;
	}
	return 0;
}

int cgl_read_sobs(struct cgl *cgl, FILE *fp)
{
	extern int read_block(struct block*, FILE*);
	int err = cgl_read_section_header("SOBS", fp);

	if (err)
		return err;
	for (size_t j = 0; j < cgl->height; ++j) {
		for (size_t i = 0; i < cgl->width; ++i) {
			err = read_block(&cgl->blocks[j][i], fp);
			if (err)
				return err;
		}
	}
	return 0;
}

int read_block(struct block *block, FILE* fp)
{
	uint8_t buf[4];
	int nread;

	for (size_t k = 0; k < block->size; ++k) {
		nread = fread(buf, 1, SOBS_TILE_SIZE, fp);
		if (nread < SOBS_TILE_SIZE) {
			SDL_SetError("cgl SOBS section corrupted "
					"(incomplete)");
			return -EBADSOBS;
		}
		block->tiles[k].offs_x = UNIT * (buf[0] >> 4);
		block->tiles[k].offs_y = UNIT * (buf[0] & 0x0f);
		block->tiles[k].width  = UNIT * (buf[1] >> 4);
		block->tiles[k].height = UNIT * (buf[1] & 0x0f);
		block->tiles[k].img_x  = UNIT * buf[2];
		block->tiles[k].img_y  = UNIT * buf[3];
	}
	return 0;
}

int cgl_read_vent(struct cgl *cgl, FILE *fp)
{
	int err = cgl_read_section_header("VENT", fp);

	if (err)
		return err;


}
