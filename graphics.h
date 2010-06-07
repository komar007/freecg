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

void gl_init(struct cg*);
void gl_change_viewport(double, double, double, double);
void gl_draw_scene();

extern struct glengine gl;

/* Animators */
enum graphics_config {
	FAN_ANIM_INTERVAL = 66,
	AIRGEN_ANIM_INTERVAL = 50,
	MAGNET_ANIM_INTERVAL = 66,
	BAR_ANIM_INTERVAL = 750,
	BAR_TEX_OFFSET = 28
};
static const int magnet_anim_order[] = {0, 1, 2, 1};
static const int fan_anim_order[] = {0, 1, 2};
static const int airgen_anim_order[] = {0, 1, 2, 3, 4, 5, 6, 7};
static const int bar_anim_order[][2] = {{0, 1}, {1, 0}};

#endif
