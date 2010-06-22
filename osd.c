#include "osd.h"
#include "graphics.h"
#include <math.h>

static struct osd_element *osd;
static size_t num_osd = 25;

void osd_init()
{
	osd = calloc(num_osd, sizeof(*osd));
        /*                x    y     w    h     z      a   tr    tx    ty   tw   th */
	osd[0]  = _o(-1,  0, -24,    0,  24,    0,   0.6,  -1,    0,  179,   1,   1, gl.ttm); /* gp 1 */
	osd[1]  = _o(-1,  0, -80,  142,  56,    0,   0.6,  -1,    0,  179,   1,   1, gl.ttm); /* gp 2 */
	osd[2]  = _o( 1, 40,   8,   64,  64,  0.1,   0.8,  -1,    0,  400,  64,  64, gl.ttm); /* cross*/
	osd[3]  = _o( 2, 31,  16,    2,  32,  0.2,   0.8,  -1,  384,  366,   2,  32, gl.ttm); /* hbar */
	osd[4]  = _o( 2, 16,  31,   32,   2,  0.2,   0.8,  -1,  384,  398,  32,   2, gl.ttm); /* vbar */
	osd[5]  = _o( 1, 16,  69,   16,   3,  0.1,   0.8,  -1,  388,  393,  16,   3, gl.ttm); /* red fb */
	osd[6]  = _o( 5,  0,  -4,    0,   0,  0.1,   0.8,   5,    0,    0,   0,   0, gl.ttm);
	osd[7]  = _o( 6,  0,  -4,    0,   0,  0.1,   0.8,   5,    0,    0,   0,   0, gl.ttm);
	osd[8]  = _o( 7,  0,  -4,    0,   0,  0.1,   0.8,   5,    0,    0,   0,   0, gl.ttm);
	osd[9]  = _o( 8,  0,  -4,    0,   0,  0.1,   0.8,  -1,  388,  390,  16,   3, gl.ttm); /* yellow fb */
	osd[10] = _o( 9,  0,  -4,    0,   0,  0.1,   0.8,   9,    0,    0,   0,   0, gl.ttm);
	osd[11] = _o(10,  0,  -4,    0,   0,  0.1,   0.8,   9,    0,    0,   0,   0, gl.ttm);
	osd[12] = _o(11,  0,  -4,    0,   0,  0.1,   0.8,   9,    0,    0,   0,   0, gl.ttm);
	osd[13] = _o(12,  0,  -4,    0,   0,  0.1,   0.8,   9,    0,    0,   0,   0, gl.ttm);
	osd[14] = _o(13,  0,  -4,    0,   0,  0.1,   0.8,   9,    0,    0,   0,   0, gl.ttm);
	osd[15] = _o(14,  0,  -4,    0,   0,  0.1,   0.8,  -1,  388,  387,  16,   3, gl.ttm); /* green fb */
	osd[16] = _o(15,  0,  -4,    0,   0,  0.1,   0.8,  15,    0,    0,   0,   0, gl.ttm);
	osd[17] = _o(16,  0,  -4,    0,   0,  0.1,   0.8,  15,    0,    0,   0,   0, gl.ttm);
	osd[18] = _o(17,  0,  -4,    0,   0,  0.1,   0.8,  15,    0,    0,   0,   0, gl.ttm);
	osd[19] = _o(18,  0,  -4,    0,   0,  0.1,   0.8,  15,    0,    0,   0,   0, gl.ttm);
	osd[20] = _o(19,  0,  -4,    0,   0,  0.1,   0.8,  15,    0,    0,   0,   0, gl.ttm);
	osd[21] = _o( 1,112,   6,   16,  16,  0.1,   0.6,  -1,  256,  360,  16,  16, gl.ttm); /* keys */
	osd[22] = _o(21,  0,  17,    0,   0,  0.1,   0.8,  -1,  256,  376,  16,  16, gl.ttm);
	osd[23] = _o(22,  0,  17,    0,   0,  0.1,   0.8,  -1,  256,  392,  16,  16, gl.ttm);
	osd[24] = _o(23,  0,  17,    0,   0,  0.1,   0.8,  -1,  256,  408,  16,  16, gl.ttm);
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
		fbars[i].a = 0.1;
	for (size_t i = 0; i < 4; ++i) {
		if (!gl.cg->ship->keys[i]) {
			keys[i].a = 0.2;
			keys[i].tex_x = 256;
		} else {
			keys[i].tex_x = 256 + 16 * ((int)(gl.cg->time*KEY_ANIM_SPEED + i) % 8);
			keys[i].a = 0.8;
		}
	}
}
void osd_draw()
{
	osdlib_draw(osd, num_osd);
}
