/* graphics.h - renderer's data structures and animation constants
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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "cg.h"
#include "texmgr.h"
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

struct camera {
	double x, y;
	double scale;
};
struct glengine {
	struct texmgr *ttm,
		      *ctm;
	struct drect viewport;
	struct camera cam;
	double win_w, win_h;
	struct cgl *l;
	unsigned int frame;
	GLuint curtex;
};
extern struct glengine gl;

void gl_init(struct cgl*, struct texmgr*, struct texmgr*);
void gl_resize_viewport(double, double);
void gl_update_window();

static inline void gl_bind_texture(struct texmgr *tm)
{
	if (gl.curtex != tm->texno) {
		glBindTexture(GL_TEXTURE_2D, tm->texno);
		gl.curtex = tm->texno;
	}
}

/* Animators */
enum graphics_config {
	FAN_ANIM_SPEED = 15,
	AIRGEN_ANIM_SPEED = 20,
	MAGNET_ANIM_SPEED = 15,
	BAR_ANIM_SPEED = 1,
	BAR_TEX_OFFSET = 28,
	KEY_ANIM_SPEED = 20,
};

#define BLINK_SPEED 1.8

#endif
