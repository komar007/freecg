#ifndef TEXMGR_H
#define TEXMGR_H

#include "cgl.h"
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

struct texmgr {
	double w, h;
	GLuint texno;
};

static inline void tm_coord_tl(struct texmgr *tm, int x, int y,
		__attribute__((unused)) int w, __attribute__((unused)) int h)
{
	glTexCoord2f((double)x / tm->w,
			(double)y / tm->h);
}
static inline void tm_coord_bl(struct texmgr *tm, int x, int y, __attribute__((unused)) int w, int h)
{
	glTexCoord2f((double)x / tm->w,
			(double)(y + h) / tm->h);
}
static inline void tm_coord_br(struct texmgr *tm, int x, int y, int w, int h)
{
	glTexCoord2f((double)(x + w) / tm->w,
			(double)(y + h) / tm->h);
}
static inline void tm_coord_tr(struct texmgr *tm, int x, int y, int w, __attribute__((unused)) int h)
{
	glTexCoord2f((double)(x + w) / tm->w,
			(double)y / tm->h);
}

struct texmgr *tm_request_texture(const SDL_Surface*);

#endif
