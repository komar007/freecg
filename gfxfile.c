#include "gfxfile.h"
#include <SDL/SDL_error.h>
#include <errno.h>

SDL_Surface *read_gfx(const char *path)
{
	FILE *fp;
	uint8_t *buffer;
	size_t size;
	SDL_RWops *rw;
	SDL_Surface *gfx;

	fp = fopen(path, "r");
	if (!fp) {
		SDL_SetError("fopen: %s", strerror(errno));
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	if (size < 2) {
		SDL_SetError("file is corrupted");
		goto error;
	}
	fseek(fp, 0, SEEK_SET);
	buffer = calloc(size, sizeof(*buffer));
	if (fread(buffer, sizeof(*buffer), size, fp) != size) {
		SDL_SetError("fread: %s", strerror(errno));
		goto error;
	}
	if (memcmp(buffer, "CG", 2) != 0) {
		SDL_SetError("wrong CG header");
		goto error;
	}
	fclose(fp);
	memcpy(buffer, "BM", 2);
	rw = SDL_RWFromMem(buffer, size);
	assert(rw != NULL);
	gfx = SDL_LoadBMP_RW(rw, 0);
	SDL_FreeRW(rw);
	free(buffer);
	return gfx;
error:
	fclose(fp);
	return NULL;
}

