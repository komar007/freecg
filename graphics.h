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
	struct texmgr *ttm;
	struct drect viewport;
	struct camera cam;
	double win_w, win_h;
	struct cg *cg;
	unsigned int frame;
	GLuint curtex;
};
extern struct glengine gl;

void gl_init(struct cg*, struct texmgr*);
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
