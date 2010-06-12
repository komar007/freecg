#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "cg.h"
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

struct glengine {
	struct drect viewport;
	struct cg *cg;
	unsigned int frame;
};
extern struct glengine gl;

void gl_init(struct cg*);
void gl_change_viewport(double, double, double, double);
void gl_draw_scene();

/* Animators */
enum graphics_config {
	FAN_ANIM_SPEED = 15,
	AIRGEN_ANIM_SPEED = 20,
	MAGNET_ANIM_SPEED = 15,
	BAR_ANIM_SPEED = 1,
	BAR_TEX_OFFSET = 28,
};
#define BLINK_SPEED 1.8

#endif
