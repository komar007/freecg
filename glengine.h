#ifndef GLENGINE_H
#define GLENGINE_H

#include "cgl.h"
#include "cg.h"
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

enum {
	BLOCK_SIZE = 32,
};

struct glengine {
	struct drect viewport;
	struct cg *cg;
	unsigned int frame;
};

void gl_init(struct cg*);
void gl_change_viewport(double, double, double, double);
void gl_draw_scene();

extern struct glengine gl;

#endif
