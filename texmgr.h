#ifndef TEXMGR_H
#define TEXMGR_H

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
	unsigned real_hash;
	size_t refcount;
};

struct texture_manager {
	const SDL_Surface *img;
	struct texture *lookup_table;
};

void tm_init(const SDL_Surface *);
struct texture *tm_request_texture(int x, int y, size_t w, size_t h);

inline void tm_coord_tl(struct texture __attribute__((unused)) *tex)
{
	glTexCoord2f(0.0, 0.0);
}
inline void tm_coord_bl(struct texture *tex)
{
	glTexCoord2f(0.0, tex->h_ratio);
}
inline void tm_coord_br(struct texture *tex)
{
	glTexCoord2f(tex->w_ratio, tex->h_ratio);
}
inline void tm_coord_tr(struct texture *tex)
{
	glTexCoord2f(tex->w_ratio, 0.0);
}

extern struct texture_manager texmgr;

#endif
