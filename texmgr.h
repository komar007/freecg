#ifndef TEXMGR_H
#define TEXMGR_H

#include "cgl.h"
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

struct texmgr {
	double w, h;
	GLuint texno;
};

inline void tm_coord_tl(struct texmgr *tm, int x, int y, int w, int h)
{
	glTexCoord2f((double)x / tm->w,
			(double)y / tm->h);
}
inline void tm_coord_bl(struct texmgr *tm, int x, int y, int w, int h)
{
	glTexCoord2f((double)x / tm->w,
			(double)(y + h) / tm->h);
}
inline void tm_coord_br(struct texmgr *tm, int x, int y, int w, int h)
{
	glTexCoord2f((double)(x + w) / tm->w,
			(double)(y + h) / tm->h);
}
inline void tm_coord_tr(struct texmgr *tm, int x, int y, int w, int h)
{
	glTexCoord2f((double)(x + w) / tm->w,
			(double)y / tm->h);
}
inline void tm_bind_texture(struct texmgr *tm)
{
	glBindTexture(GL_TEXTURE_2D, tm->texno);
}

struct texmgr *tm_request_texture(const SDL_Surface*);

#endif
