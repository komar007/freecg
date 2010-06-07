#include "texmgr.h"
#include "gfx.h"
#include <math.h>

struct texture_manager texmgr;

void tm_init(const SDL_Surface *image)
{
	texmgr.img = image;
	texmgr.lookup_table = calloc(texmgr.img->w/4 * texmgr.img->h/4,
			sizeof(*texmgr.lookup_table));
}

inline unsigned real_hash(int x, int y, size_t w, size_t h)
{
	return ((x/4) << 24) + ((y/4) << 16) + ((w/4) << 8) + (h/4);
}

struct texture *tm_request_texture(int x, int y, size_t w, size_t h)
{
	extern GLuint tm_load_texture(SDL_Surface *);
	struct texture *lt = texmgr.lookup_table;
	unsigned rh = real_hash(x, y, w, h);
	int hash = x/4 + y/4 * texmgr.img->w/4;
	for (; lt[hash].refcount != 0 && lt[hash].real_hash != rh; ++hash);
	if (texmgr.lookup_table[hash].refcount++ == 0) {
		int tex_w = 1 << (int)ceil(log2(w)),
		    tex_h = 1 << (int)ceil(log2(h));
		SDL_Surface *tile = SDL_CreateRGBSurface(0, tex_w, tex_h, 32,
				RMASK, GMASK, BMASK, AMASK);
		SDL_Rect rect = {
			.x = x,
			.y = y,
			.w = w,
			.h = h
		};
		if (SDL_MUSTLOCK(tile))
			SDL_LockSurface(tile);
		SDL_BlitSurface((SDL_Surface*)texmgr.img, &rect,
				tile, NULL);
		texmgr.lookup_table[hash].no = tm_load_texture(tile);
		if (SDL_MUSTLOCK(tile))
			SDL_UnlockSurface(tile);
		lt[hash].real_hash = rh;
		lt[hash].w_ratio = (double)w / tex_w;
		lt[hash].h_ratio = (double)h / tex_h;
		SDL_FreeSurface(tile);
	}
	return &lt[hash];
}

GLuint tm_load_texture(SDL_Surface *image)
{
	GLuint texno;
	glGenTextures(1, &texno);
	glBindTexture(GL_TEXTURE_2D, texno);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TEX_FILTER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TEX_FILTER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	return texno;
}
