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
	extern void print_size(const struct cgl*),
	            print_soin(const struct cgl*),
		    print_sobs(const struct cgl*);
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
	if (which[SIZE - '1'])
		print_size(cgl);
	if (which[SOIN - '1'])
		print_soin(cgl);
	if (which[SOBS - '1'])
		print_sobs(cgl);
	return 0;
}

void print_size(const struct cgl *cgl)
{
	printf("section SIZE\n");
	printf("\twidth = %d\n\theight = %d\n", cgl->width, cgl->height);
}

void print_soin(const struct cgl *cgl)
{
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

void print_sobs(const struct cgl *cgl)
{
	extern void print_sobs_block(const struct block*, int, int);

	printf("section SOBS\n");
	for (size_t j = 0; j < cgl->height; ++j)
		for (size_t i = 0; i < cgl->width; ++i)
			print_sobs_block(&cgl->blocks[j][i], i, j);
}

void print_sobs_block(const struct block *b, int x, int y)
{
	printf("\tblock at (%d, %d):\n", x, y);
	for (size_t k = 0; k < b->size; ++k) {
		printf("\t\ttile %d:" "\t" "size" "\t= " "(%d, %d)\n"
				"\t\t\t" "offset" "\t= " "(%d, %d)\n"
				"\t\t\t" "img_pos" "\t= " "(%d, %d)\n", k,
				b->tiles[k].width, b->tiles[k].height,
				b->tiles[k].offs_x, b->tiles[k].offs_y,
				b->tiles[k].img_x, b->tiles[k].img_y);
	}
}
