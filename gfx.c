#include "gfx.h"
#include <errno.h>
#include <assert.h>

SDL_Surface *read_gfx(const char *path)
{
	FILE *fp;
	uint8_t *buffer;
	size_t size;
	SDL_RWops *rw;
	SDL_Surface *bmp = NULL,
		    *gfx = NULL;

	fp = fopen(path, "rb");
	if (!fp) {
		SDL_SetError("fopen: %s", strerror(errno));
		return NULL;
	}
	(void)fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	if (size < 2) {
		SDL_SetError("file is corrupted");
		fclose(fp);
		return NULL;
	}
	(void)fseek(fp, 0, SEEK_SET);
	buffer = calloc(size, sizeof(*buffer));
	assert(buffer != NULL);
	if (fread(buffer, sizeof(*buffer), size, fp) < size) {
		SDL_SetError("fread: %s", strerror(errno));
		goto cleanup;
	}
	if (memcmp(buffer, "CG", 2) != 0) {
		SDL_SetError("wrong CG header");
		goto cleanup;
	}
	memcpy(buffer, "BM", 2);
	rw = SDL_RWFromMem(buffer, size);
	assert(rw != NULL);
	bmp = SDL_LoadBMP_RW(rw, 0);
	if (!bmp) {
		SDL_FreeRW(rw);
		goto cleanup;
	};
	SDL_SetColorKey(bmp, SDL_SRCCOLORKEY, 0);
	SDL_FreeRW(rw);
	gfx = SDL_CreateRGBSurface(0, bmp->w, bmp->h, 32,
			0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	SDL_BlitSurface(bmp, NULL, gfx, NULL);
	SDL_FreeSurface(bmp);
cleanup:
	fclose(fp);
	free(buffer);
	return gfx;
}
