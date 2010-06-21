#ifndef TEXMGR_H
#define TEXMGR_H

#include "cgl.h"
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

struct texmgr {
	double w, h;
	GLuint texno;
};

inline void tm_coord_tl(struct texmgr *tm, const struct tile *tile)
{
	glTexCoord2f((double)tile->tex_x / tm->w,
			(double)tile->tex_y / tm->h);
}
inline void tm_coord_bl(struct texmgr *tm, const struct tile *tile)
{
	glTexCoord2f((double)tile->tex_x / tm->w,
			(double)(tile->tex_y + tile->h) / tm->h);
}
inline void tm_coord_br(struct texmgr *tm, const struct tile *tile)
{
	glTexCoord2f((double)(tile->tex_x + tile->w) / tm->w,
			(double)(tile->tex_y + tile->h) / tm->h);
}
inline void tm_coord_tr(struct texmgr *tm, const struct tile *tile)
{
	glTexCoord2f((double)(tile->tex_x + tile->w) / tm->w,
			(double)tile->tex_y / tm->h);
}
inline void tm_bind_texture(struct texmgr *tm)
{
	glBindTexture(GL_TEXTURE_2D, tm->texno);
}

struct texmgr *tm_request_texture(const SDL_Surface*);

#endif
