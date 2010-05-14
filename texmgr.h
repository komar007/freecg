#ifndef TEXMGR_H
#define TEXMGR_H

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

enum config {
	TEX_FILTER = GL_LINEAR,
};

struct texture {
	GLuint no;	/* opengl texture number */
	size_t refcount;
};

struct texture_manager {
	const SDL_Surface *img;
	struct texture *lookup_table;
};

void tm_init(const SDL_Surface *);
GLuint tm_request_texture(size_t x, size_t y, size_t w, size_t h);

extern struct texture_manager texmgr;

#endif
