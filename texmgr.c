#include "texmgr.h"

struct texture_manager texmgr;

void init_texture_manager(const SDL_Surface *image)
{
	texmgr.img = image;
	texmgr.lookup_table = calloc(texmgr.img->w/4 * texmgr.img->h/4,
			sizeof(*texmgr.lookup_table));
}

GLuint tm_request_texture(size_t x, size_t y, size_t w, size_t h)
{
	extern GLuint load_texture(SDL_Surface *);
	int hash = x/4 + y/4 * texmgr.img->w/4;
	if (texmgr.lookup_table[hash].refcount++ == 0) {
		SDL_Surface *tile = SDL_CreateRGBSurface(0, w, h, 32,
				0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
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
		texmgr.lookup_table[hash].no = load_texture(tile);
		if (SDL_MUSTLOCK(tile))
			SDL_UnlockSurface(tile);
		SDL_FreeSurface(tile);
	}
	return texmgr.lookup_table[hash].no;
}

GLuint load_texture(SDL_Surface *image)
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
