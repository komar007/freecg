#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

enum config {
	TEX_FILTER = GL_NEAREST,
};

struct texture {
	GLuint no;	/* opengl texture number */
	size_t refcount;
};

struct texture_manager {
	const SDL_Surface *img;
	struct texture *lookup_table;
};

void init_texture_manager(const SDL_Surface *);
GLuint tm_request_texture(SDL_Rect*);

extern struct texture_manager texmgr;
