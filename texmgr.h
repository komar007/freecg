/* texmgr.h - a simple opengl texture manager
 * Copyright (C) 2010 Michal Trybus.
 *
 * This file is part of FreeCG.
 *
 * FreeCG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * FreeCG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FreeCG. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEXMGR_H
#define TEXMGR_H

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
