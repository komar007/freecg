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

int cgl_read_sobs(struct cgl *cgl, FILE *fp)
{
	
}

