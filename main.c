#include "glengine.h"
#include "texmgr.h"
#include "gfx.h"

#include <stdio.h>
#include <assert.h>
#include <SDL/SDL.h>
#include <math.h>

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s file.cgl\n", argv[0]);
		exit(-1);
	}
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL failed: %s\n", SDL_GetError());
		abort();
	}
	SDL_SetVideoMode(1024, 768, 32,
			SDL_OPENGL);
	SDL_Surface *gfx = read_gfx("data/GRAVITY.GFX");
	init_texture_manager(gfx);
	struct cgl *cgl = read_cgl(argv[1], NULL);
	cgl_preprocess(cgl);
	struct cg *cg = cg_init(cgl);
	gl_init(cg);
	assert(gfx);
	//int i;
	//for (double x = 0; x < 4*M_PI; x += 0.005) {
	//	change_viewport(cos(x) *500, sin(x)*500, 1024, 768);
	//	test_draw(cgl, cos(x)*500, sin(x)*500, cos(x)*500 + 1024, sin(x)*500 + 768);
	//	i++
	//}
	int i = 0;
	int t = SDL_GetTicks();
	for (double x = 0; x < 200; x += 0.1) {
		gl_change_viewport(x, 0, 1024, 768);
		gl_draw_scene();
		++i;
	}
	t = SDL_GetTicks() - t;
	printf("%d frames in %d ms. %f fps\n", i, t, (float)i / t * 1000);
	free_cgl(cgl);
	return 0;
}
