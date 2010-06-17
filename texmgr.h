#ifndef TEXMGR_H
#define TEXMGR_H

#include "cgl.h"
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

struct texmgr {
	int w, h;
	GLuint texno;
};
struct texmgr texm;

inline void tm_coord_tl(const struct tile *tile)
{
	glTexCoord2f((double)tile->tex_x / texm.w,
			(double)tile->tex_y / texm.h);
}
inline void tm_coord_bl(const struct tile *tile)
{
	glTexCoord2f((double)tile->tex_x / texm.w,
			(double)(tile->tex_y + tile->h) / texm.h);
}
inline void tm_coord_br(const struct tile *tile)
{
	glTexCoord2f((double)(tile->tex_x + tile->w) / texm.w,
			(double)(tile->tex_y + tile->h) / texm.h);
}
inline void tm_coord_tr(const struct tile *tile)
{
	glTexCoord2f((double)(tile->tex_x + tile->w) / texm.w,
			(double)tile->tex_y / texm.h);
}

void tm_request_texture(const SDL_Surface*);

#endif
