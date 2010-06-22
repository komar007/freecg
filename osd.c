#include "osd.h"
#include "graphics.h"
#include <math.h>

static struct osd_element *osd;
static size_t num_osd;

void osd_init()
{
	num_osd = 26 + gl.cg->level->num_all_freigh;
	osd = calloc(num_osd, sizeof(*osd));
        /*                x    y     w    h     z      a   tr    tx    ty   tw   th */
	osd[0]  = _o(-1,142, -32,    0,  32,    0,   0.6,  -1,    0,  179,   1,   1, gl.ttm); /* gp 1 */
	osd[1]  = _o(-1,  0, -80,  142,  80,    0,   0.6,  -1,    0,  179,   1,   1, gl.ttm); /* gp 2 */
	osd[2]  = _o( 1, 40,   8,   64,  64,  0.1,   0.8,  -1,    0,  400,  64,  64, gl.ttm); /* cross*/
	osd[3]  = _o( 2, 31,  16,    2,  32,  0.2,   0.8,  -1,  384,  366,   2,  32, gl.ttm); /* hbar */
	osd[4]  = _o( 2, 16,  31,   32,   2,  0.2,   0.8,  -1,  384,  398,  32,   2, gl.ttm); /* vbar */
	/* red fuel */
	osd[5]  = _o( 1, 16,  69,   16,   3,  0.1,   0.8,  -1,  388,  393,  16,   3, gl.ttm);
for (size_t i = 6; i <= 8; ++i)
	osd[i]  = _o(i-1, 0,  -4,    0,   0,  0.1,   0.8,   5,    0,    0,   0,   0, gl.ttm);
	/* yellow fuel */
	osd[9]  = _o( 8,  0,  -4,    0,   0,  0.1,   0.8,  -1,  388,  390,  16,   3, gl.ttm);
for (size_t i = 10; i <= 14; ++i)
	osd[i]  = _o(i-1, 0,  -4,    0,   0,  0.1,   0.8,   9,    0,    0,   0,   0, gl.ttm);
	/* green fuel */
	osd[15] = _o(14,  0,  -4,    0,   0,  0.1,   0.8,  -1,  388,  387,  16,   3, gl.ttm);
for (size_t i = 16; i <= 20; ++i)
	osd[i]  = _o(i-1, 0,  -4,    0,   0,  0.1,   0.8,  15,    0,    0,   0,   0, gl.ttm);
	/* keys */
	osd[21] = _o( 1,112,   6,   16,  16,  0.1,   0.6,  -1,  256,  360,  16,  16, gl.ttm);
for (size_t i = 1; i < 4; ++i)
	osd[i+21]=_o(i+20,0,  17,    0,   0,  0.1,   0.8,  -1,  256,360+16*i,16, 16, gl.ttm);
	osd[25] = _o(0,   8,   8,   32,  16,  0.1,   0.8,  -1,  384,  400,  48,  24, gl.ttm);
for (size_t i = 0; i < gl.cg->level->num_all_freigh; ++i)
	osd[26+i].rel = osd[26+i].texrel = -1;
}
void osd_step()
{
	struct osd_element *bars  = osd+3,
			   *fbars = osd+5,
			   *keys  = osd+21;
	bars[0].x = fmin(64, fmax(0, gl.cg->ship->vx/3 + 31));
	bars[1].y = fmin(64, fmax(0, gl.cg->ship->vy/3 + 31));
	for (size_t i = 0; i < (size_t)ceil(gl.cg->ship->fuel); ++i)
		fbars[i].a = 0.8;
	for (size_t i = (size_t)ceil(gl.cg->ship->fuel); i < 16; ++i)
		fbars[i].a = 0.2;
	for (size_t i = 0; i < 4; ++i) {
		if (!gl.cg->ship->keys[i]) {
			keys[i].a = 0.2;
			keys[i].tex_x = 256;
		} else {
			keys[i].tex_x = 256 + 16 * ((int)(gl.cg->time*KEY_ANIM_SPEED + i) % 8);
			keys[i].a = 0.8;
		}
	}
	size_t nfreigh = 0;
	for (size_t i = 0; i < gl.cg->level->nairports; ++i) {
		if (gl.cg->level->airports[i].type == Freigh)
			nfreigh += gl.cg->level->airports[i].num_cargo;
	}
	int freigh[nfreigh];
	int k = 0;
	for (size_t i = 0; i < gl.cg->level->nairports; ++i) {
		if (gl.cg->level->airports[i].type == Freigh) {
			struct airport *a = &gl.cg->level->airports[i];
			for (size_t j = 0; j < a->num_cargo; ++j)
				freigh[k++] = a->c.freigh[j];
		}
	}
	osd[26] = _o(0,  48,   8,   16,  16,  0.1,   0.8,  -1,   80+freigh[0]*16,  392,  16,  16, gl.ttm);
	for (size_t i = 1; i < nfreigh; ++i)
		osd[26+i]=_o(25+i,17,  0,    0,   0,  0.1,   0.8,  -1,   80+freigh[i]*16,  392,  16,  16, gl.ttm);
	for (size_t i = nfreigh; i < gl.cg->level->num_all_freigh; ++i)
		osd[26+i]=_o(-1, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, gl.ttm);
}
void osd_draw()
{
	osdlib_draw(osd, num_osd);
}
