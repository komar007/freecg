#ifndef TEXMGR_H
#define TEXMGR_H

#include "cgl.h"
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

enum tm_config {
	TEX_FILTER = GL_NEAREST,
};

struct texture {
	GLuint no;	/* opengl texture number */
	/* How much of an opengl texture is used to hold actual graphics
	 * (textures must have power-of-two dimensions) */
	double w_ratio, h_ratio;
	double x, y, w, h;
	long long real_hash;
	size_t refcount;
};

struct texture_manager {
	const SDL_Surface *img;
	struct texture *lookup_table;
};

void tm_init(const SDL_Surface *);
struct texture *tm_request_texture(struct tile*);

inline void tm_coord_tl(struct texture __attribute__((unused)) *tex)
{
	glTexCoord2f(tex->x, tex->y);
}
inline void tm_coord_bl(struct texture *tex)
{
	glTexCoord2f(tex->x, tex->y + tex->h);
}
inline void tm_coord_br(struct texture *tex)
{
	glTexCoord2f(tex->x + tex->w, tex->y + tex->h);
}
inline void tm_coord_tr(struct texture *tex)
{
	glTexCoord2f(tex->x + tex->w, tex->y);
}

extern struct texture_manager texmgr;

#endif
