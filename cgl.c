#include "cgl.h"
#include <SDL/SDL_error.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

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
	extern int cgl_read_header(struct cgl*, FILE*),
	           cgl_read_soin(struct cgl*, FILE*),
		   cgl_read_magic(struct cgl*, FILE*);
	struct cgl *cgl;
	FILE *fp;
	fp = fopen(path, "r");
	if (!fp) {
		SDL_SetError("fopen: %s", strerror(errno));
		return NULL;
	}
	cgl = malloc(sizeof(*cgl));
	memset(cgl, 0, sizeof(*cgl));
	if (cgl_read_header(cgl, fp) != 0)
		goto error;
	if (cgl_read_soin(cgl, fp) != 0)
		goto error;
	if (cgl_read_magic(cgl, fp) != 0)
		goto error;

	fclose(fp);
	return cgl;
error:
	fclose(fp);
	free_cgl(cgl);
	return NULL;
}

/*
 * These functions fill the cgl structure with data read from a file pointer.
 * On error SDL_SetError(...) is called and non-zero error code is returned.
 * 0 is returned on success.
 */
int cgl_read_header(struct cgl *cgl, FILE *fp)
{
	uint8_t buf[16];
	size_t nread;

	nread = fread(buf, 1, CGL_HEADER_SIZE, fp);
	if (nread < CGL_HEADER_SIZE) {
		SDL_SetError("incomplete cgl header");
		return -EBADHDR;
	} else if (memcmp(buf, "CGL1SIZE", 8) != 0) {
		SDL_SetError("cgl header corrupted");
		return -EBADHDR;
	}
	uint32_t *dims = (uint32_t*)(buf + 8);
	cgl->width  = dims[0];
	cgl->height = dims[1];
	return 0;
}

int cgl_read_soin(struct cgl *cgl, FILE *fp)
{
	size_t nread,
	       nblocks = cgl->height * cgl->width;
	uint8_t *nums, *pnum;
	char hdr[4];

	nread = fread(hdr, 1, CGL_SOIN_HDR_SIZE, fp);
	if (nread < CGL_SOIN_HDR_SIZE) {
		SDL_SetError("cgl SOIN header incomplete");
		return -EBADSOIN;
	} else if (memcmp(hdr, "SOIN", CGL_SOIN_HDR_SIZE) != 0) {
		SDL_SetError("cgl SOIN header corrupted");
		return -EBADSOIN;
	}
	pnum = nums = calloc(nblocks, sizeof(*nums));
	nread = fread(nums, 1, nblocks, fp);
	if (nread < nblocks) {
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

