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

inline long long real_hash(int x, int y, size_t w, size_t h)
{
	return ((long long)x << 30) + ((long long)y << 20) +
		((long long)w << 10) + (long long)h;
}

struct texture *tm_request_texture(struct tile *t)
{
	static int asd = 0;
	extern GLuint tm_load_texture(SDL_Surface *);
	struct texture *lt = texmgr.lookup_table;
	long long rh = real_hash(t->tex_x, t->tex_y, t->tex_w, t->tex_h);
	int hash = t->tex_x/4 + t->tex_y/4 * texmgr.img->w/4;
	for (; lt[hash].refcount != 0 && lt[hash].real_hash != rh; ++hash);
	if (texmgr.lookup_table[hash].refcount++ == 0) {
		int tex_w = 1 << (int)ceil(log2(t->tex_w)),
		    tex_h = 1 << (int)ceil(log2(t->tex_h));
		SDL_Surface *tile = SDL_CreateRGBSurface(0, tex_w, tex_h, 32,
				RMASK, GMASK, BMASK, AMASK);
		SDL_Rect rect = {
			.x = t->tex_x,
			.y = t->tex_y,
			.w = t->tex_w,
			.h = t->tex_h
		};
		if (SDL_MUSTLOCK(tile))
			SDL_LockSurface(tile);
		SDL_BlitSurface((SDL_Surface*)texmgr.img, &rect,
				tile, NULL);
		texmgr.lookup_table[hash].no = tm_load_texture(tile);
		if (SDL_MUSTLOCK(tile))
			SDL_UnlockSurface(tile);
		lt[hash].real_hash = rh;
		lt[hash].w_ratio = (double)t->w / tex_w;
		lt[hash].h_ratio = (double)t->h / tex_h;
		SDL_FreeSurface(tile);
		printf("tex: %i\n", ++asd);
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
