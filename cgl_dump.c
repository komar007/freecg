#include "cgl.h"
#include "gfx.h"
#include <SDL/SDL_error.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(-1);
	}
	struct cgl *cgl = read_cgl(argv[1]);
	if (!cgl) {
		fprintf(stderr, "read_cgl: %s\n", SDL_GetError());
		return -1;
	}

	printf("CGL1\n");
	printf("section SIZE\n");
	printf("\twidth = %d\n\theight = %d\n", cgl->width, cgl->height);

	return 0;
}
