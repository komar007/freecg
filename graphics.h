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

/* Animators */
enum graphics_config {
	FAN_ANIM_SPEED = 15,
	AIRGEN_ANIM_SPEED = 20,
	MAGNET_ANIM_SPEED = 15,
	BAR_ANIM_SPEED = 1,
	BAR_TEX_OFFSET = 28,
	KEY_ANIM_SPEED = 20,
};
struct osd_element {
	int rel;
	int x, y;
	int w, h;
	double z;
	double a;
	int texrel;
	int tex_x, tex_y;
	int tex_w, tex_h;
	struct texmgr *tm;
};

#define BLINK_SPEED 1.8

#endif
