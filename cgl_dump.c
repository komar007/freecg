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
	{"help", no_argument, NULL, 'h'},
	{"all", no_argument, NULL, 'a'},
	{"size", no_argument, NULL, SIZE},
	{"no-size", no_argument, NULL, -SIZE},
	{"soin", no_argument, NULL, SOIN},
	{"no-soin", no_argument, NULL, -SOIN},
	{"sobs", no_argument, NULL, SOBS},
	{"no-sobs", no_argument, NULL, -SOBS},
	{"vent", no_argument, NULL, VENT},
	{"no-vent", no_argument, NULL, -VENT},
	{NULL, 0, NULL, 0}
};
char *optstr = "ha1234";
int which[] = {-1, -1, -1, -1};

void print_help(const char *name)
{
	printf("Dump CGL file\n\n" "Usage: %s [options] <file>\n\n"
			"Options:\n"
			"  -h, --help\t* print this help and exit\n"
			"  -a, --all\t* print all sections of file\n"
			"  --SECT\t* print section SECT\n"
			"  --no-SECT\t* don't print section SECT "
			"(overrides --SECT)\n"
			"Where SECT can be one of: size, soin, sobs, vent, "
			"magn, dist,\ncano, pipe, onew, barr, lpts, lvin.\n\n"
			"Example:\n"
			"%s --all --no-sobs level.cgl\n",
			name, name);
}

int main(int argc, char *argv[])
{
	extern void print_size(const struct cgl*),
	            print_soin(const struct cgl*, const uint8_t *),
		    print_sobs(const struct cgl*, const uint8_t *),
		    print_vent(const struct cgl*);
	int ret;

	while ((ret = getopt_long(argc, argv, optstr, opts, NULL)) != -1) {
		if (ret == '?')
			continue;
		if (ret == 'h') {
			print_help(argv[0]);
			exit(0);
		} else if (ret == 'a') {
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
	if (optind >= argc) {
		print_help(argv[0]);
		exit(-1);
	}
	uint8_t *soin;
	struct cgl *cgl = read_cgl(argv[optind], &soin);
	if (!cgl) {
		fprintf(stderr, "read_cgl: %s\n", SDL_GetError());
		return -1;
	}

	printf("CGL1 (%s level)\n",
			cgl->type == Demo ? "demo" : "full version");
	if (which[SIZE - '1'])
		print_size(cgl);
	if (which[SOIN - '1'])
		print_soin(cgl, soin);
	if (which[SOBS - '1'])
		print_sobs(cgl, soin);
	if (which[VENT - '1'])
		print_vent(cgl);
	return 0;
}

void print_size(const struct cgl *cgl)
{
	printf("section SIZE\n");
	printf("\twidth = %zu\n\theight = %zu\n", cgl->width, cgl->height);
}

void print_soin(const struct cgl *cgl, const uint8_t *soin)
{
	printf("section SOIN\n");
	printf("\tnumber of blocks = %lu (%zu x %zu)\nblock dump:\n",
			cgl->width * cgl->height, cgl->width, cgl->height);
	for (size_t j = 0; j < cgl->height; ++j) {
		printf("\t%hhu", soin[j*cgl->width]);
		for (size_t i = 1; i < cgl->width; ++i)
			printf(" %hhu", soin[i + j*cgl->width]);
		printf("\n");
	}
}

void print_sobs(const struct cgl *cgl, const uint8_t *soin)
{
	extern void print_sobs_block(const struct tile*, size_t, int, int);

	struct tile *cur_tile = cgl->tiles;
	printf("section SOBS\n");
	for (size_t j = 0; j < cgl->height; ++j) {
		for (size_t i = 0; i < cgl->width; ++i) {
			print_sobs_block(cur_tile,
					(size_t)soin[i + j*cgl->width], i, j);
			cur_tile += soin[i + j*cgl->width];
		}
	}
}

void print_sobs_block(const struct tile *tiles, size_t num, int x, int y)
{
	printf("\tblock at (%d, %d):\n", x, y);
	for (size_t k = 0; k < num; ++k) {
		printf("\t\ttile %zu:\tsize\t= (%d, %d)\n"
				"\t\t\tpos\t= (%d, %d)\n"
				"\t\t\timg_pos\t= (%d, %d)\n", k,
				tiles[k].w, tiles[k].h,
				tiles[k].x, tiles[k].y,
				tiles[k].img_x, tiles[k].img_y);
	}
}

void print_vent(const struct cgl *cgl)
{
	extern void print_one_vent(struct fan *fan, int);

	printf("section VENT\n");
	for (size_t i = 0; i < cgl->nfans; ++i)
		print_one_vent(&cgl->fans[i], i);
}

void print_one_vent(struct fan *fan, int num)
{
	printf("\tfan %d: power = %s, dir = %s\n", num,
			fan->power == Hi ? "hi" : "low",
			fan->dir == Down ? "Down" : fan->dir == Up ? "Up" :
			fan->dir == Left ? "Left" : "Right");
	printf("\t\tbase:\tpos\t= (%d, %d)\n"
			"\t\t\timg_pos\t= (%d, %d)\n",
			fan->base->x, fan->base->y,
			fan->base->img_x, fan->base->img_y);
	printf("\t\tpipes:\tsize\t= (%d, %d)\n"
			"\t\t\tpos\t= (%d, %d)\n"
			"\t\t\timg_pos\t= (%d, %d)\n",
			fan->pipes->w, fan->pipes->h,
			fan->pipes->x, fan->pipes->y,
			fan->pipes->img_x, fan->pipes->img_y);
	printf("\t\tbbox\t= (%d, %d, %d, %d)\n"
			"\t\trange\t= (%d, %d, %d, %d)\n",
			fan->bbox.x, fan->bbox.y,
			fan->bbox.w, fan->bbox.h,
			fan->range.x, fan->range.y,
			fan->range.w, fan->range.h);
}
