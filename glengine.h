#include "cgl.h"
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

enum {
	BLOCK_SIZE = 32,
};

void init_opengl(void);
void change_viewport(double, double, double, double);
void test_draw(struct cgl *, SDL_Surface *);
GLuint load_texture(SDL_Surface *);
