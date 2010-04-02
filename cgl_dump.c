#include "cgl.h"
#include "gfx.h"
#include <SDL/SDL_error.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define ARR_SZ(x) (sizeof(x) / sizeof(*(x)))

enum sections {
	SIZE = '1',
	SOIN,
	SOBS,
	VENT,
	MAGN,
	DIST,
	CANO,
	PIPE
	/* to be continued */
};

struct option opts[] = {
	{"all", no_argument, NULL, 'a'},
	{"size", no_argument, NULL, -SIZE},
	{"no-size", no_argument, NULL, -SIZE},
	{"soin", no_argument, NULL, -SOIN},
	{"no-soin", no_argument, NULL, -SOIN},
	{"sobs", no_argument, NULL, -SOBS},
	{"no-sobs", no_argument, NULL, -SOBS},
	{NULL, 0, NULL, 0}
};
char *optstr = "a123";
int which[] = {-1, -1, -1};

int main(int argc, char *argv[])
{
	int ret;
	while ((ret = getopt_long(argc, argv, optstr, opts, NULL)) != -1) {
		if (ret == '?')
			continue;
		if (ret == 'a') {
			for (size_t i = 0; i < ARR_SZ(which); ++i)
				if (which[i] == -1)
					which[i] = 1;
		} else if (ret > 0) {
			if (which[ret - '1'] == -1)
				which[ret - '1'] = 1;
		} else if (ret < 0) {
			which[-ret - '1'] = 0;
		}
	}
	for (size_t i = 0; i < ARR_SZ(which); ++i)
		if (which[i] == -1)
			which[i] = 0;
	struct cgl *cgl = read_cgl(argv[optind]);
	if (!cgl) {
		fprintf(stderr, "read_cgl: %s\n", SDL_GetError());
		return -1;
	}

	printf("CGL1 (%s level)\n",
			cgl->type == DEMO ? "demo" : "full version");
	if (which[SIZE - '1']) {
		printf("section SIZE\n");
		printf("\twidth = %d\n\theight = %d\n", cgl->width, cgl->height);
	}

	if (which[SOIN - '1']) {
		printf("section SOIN\n");
		printf("\tnumber of blocks = %d (%d x %d)\nblock dump:\n",
				cgl->width * cgl->height, cgl->width, cgl->height);
		for (size_t j = 0; j < cgl->height; ++j) {
			printf("\t%d", cgl->blocks[j][0].size);
			for (size_t i = 1; i < cgl->width; ++i)
				printf(" %d", cgl->blocks[j][i].size);
			printf("\n");
		}
	}

	return 0;
}
