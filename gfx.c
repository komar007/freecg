/* gfx.c - GFX file parsing routines
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

#include "gfx.h"
#include <errno.h>
#include <assert.h>
#include <SDL/SDL_image.h>

SDL_Surface *load_gfx(const char *path)
{
	extern void fix_transparency(SDL_Surface*, int, int, int, int),
	            fix_stripe(SDL_Surface*, int, int, int, int, int, int);
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
	SDL_SetColorKey(bmp, SDL_SRCCOLORKEY,
			SDL_MapRGB(bmp->format, 179, 179, 0));
	SDL_FreeRW(rw);
	gfx = SDL_CreateRGBSurface(SDL_SWSURFACE,
			TILESET_W+160+STRIPE_END_W, TILESET_H,
			32, RMASK, GMASK, BMASK, AMASK);
	SDL_BlitSurface(bmp, NULL, gfx, NULL);
	/* Make sure all next blits copy all channels, including alpha */
	SDL_SetAlpha(gfx, 0, 255);
	fix_transparency(gfx, 0, 0, TILESET_W, TILESET_H);
	/* attach fixed airport stripes to the right */
	for (int i = 0; i < 8; ++i)
		fix_stripe(gfx, STRIPE_ORYG_X, STRIPE_ORYG_Y + STRIPE_H*i,
				84, 8, TILESET_W, STRIPE_H*i);
	SDL_FreeSurface(bmp);
cleanup:
	fclose(fp);
	free(buffer);
	return gfx;
}

SDL_Surface *load_png(const char *path)
{
	SDL_RWops *rw = SDL_RWFromFile(path, "rb");
	SDL_Surface *png = IMG_LoadPNG_RW(rw);
	if (!png) {
		SDL_SetError("IMG_LoadPNG_RW: %s", IMG_GetError());
		goto cleanup;
	}
	SDL_SetAlpha(png, 0, 255);
cleanup:
	SDL_FreeRW(rw);
	return png;
}

void blitntimes(SDL_Surface *surf, int src_x, int src_y, int w, int h,
		int dst_x, int dst_y, int n)
{
	SDL_Rect srect = {
		.x = src_x,
		.y = src_y,
		.w = w,
		.h = h
	};
	SDL_Rect drect = {
		.x = dst_x,
		.y = dst_y
	};
	for (int i = 0; i < n; ++i) {
		drect.x = dst_x + i*w;
		SDL_BlitSurface(surf, &srect, surf, &drect);
	}
}
void fix_stripe(SDL_Surface *surf, int src_x, int src_y, int w, int h,
		int dst_x, int dst_y)
{
	blitntimes(surf, src_x, src_y, STRIPE_END_W, STRIPE_H,
			dst_x, dst_y, 1);
	blitntimes(surf, src_x + STRIPE_END_W, src_y, w - 2*STRIPE_END_W, h,
			dst_x + STRIPE_END_W, dst_y, 5);
}

void fix_transparency(SDL_Surface *gfx, int _x, int _y, int w, int h)
{
	for (int y = _y; y < _y + h; ++y) {
		uint32_t *pixel = (uint32_t*)((uint8_t*)gfx->pixels +
				y*gfx->pitch);
		for (int x = _x; x < _x + w; ++x, ++pixel) {
			if ((*pixel & (~AMASK)) == 0) /* Black */
				*pixel = 0;
		}
	}
}

int make_collision_map(const SDL_Surface *gfx, collision_map cmap)
{
	for (int y = 0; y < TILESET_H; ++y) {
		uint32_t *pixel = (uint32_t*)((uint8_t*)gfx->pixels +
				y*gfx->pitch);
		for (int x = 0; x < TILESET_W; ++x, ++pixel)
			cmap[y][x] = *pixel & AMASK ? 1 : 0;
	}
	return 0;
}
