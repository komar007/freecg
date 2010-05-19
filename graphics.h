#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "cgl.h"
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
	MAGNET_ANIM_INTERVAL = 66
};
static const int magnet_anim_order[] = {0, 1, 2, 1};
static const int fan_anim_order[] = {0, 1, 2};
static const int airgen_anim_order[] = {0, 1, 2, 3, 4, 5, 6, 7};

void cg_animate_fan(struct fan*, double);
void cg_animate_magnet(struct magnet*, double);
void cg_animate_airgen(struct airgen*, double);

#endif
