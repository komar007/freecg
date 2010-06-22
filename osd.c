#include "osd.h"
#include "graphics.h"
#include <math.h>

static struct cg_osd osd;

void osd_init()
{
	osd.num = 2;
	osd.els = calloc(osd.num, sizeof(*osd.els));
	/* left rect */
	osd.els[0] = _o(0,  -80,   144,  80,   0.8,    0,  90,  1,  1, 0, gl.ttm);
	struct osd_element *fuel_cont, *cross, *key_cont;
	osdlib_make_children(&osd.els[0], 3, 1, &fuel_cont, &cross, &key_cont);
	*fuel_cont = _o(16,   8,    16,  64,   0.8,    0,  90,  1,  1, 1, gl.ttm);
	*cross     = _o(40,   8,    64,  64,   0.8,    0, 400, 64, 64, 0, gl.ttm);
	*key_cont  = _o(-32,  8,    16,  64,   0.8,    0,  90,  1,  1, 1, gl.ttm);
	osdlib_make_children(fuel_cont, 16, 0);
	osd.fuel = fuel_cont->ch;
	for (int i = 0; i < 4; ++i)
		fuel_cont->ch[i] = _o(0, -4*(i+1),  16, 3,  0.8,  388, 393,  16, 3, 0, gl.ttm);
	for (int i = 4; i < 10; ++i)
		fuel_cont->ch[i] = _o(0, -4*(i+1),  16, 3,  0.8,  388, 390,  16, 3, 0, gl.ttm);
	for (int i = 10; i < 16; ++i)
		fuel_cont->ch[i] = _o(0, -4*(i+1),  16, 3,  0.8,  388, 387,  16, 3, 0, gl.ttm);
	osdlib_make_children(cross, 2, 1, &osd.vxbar, &osd.vybar);
	*osd.vxbar = _o(31,  16,     2,  32,   0.8,  384, 366,  2, 32, 0, gl.ttm);
	*osd.vybar = _o(16,  31,    32,   2,   0.8,  384, 398, 32,  2, 0, gl.ttm);
	osdlib_make_children(key_cont, 4, 0);
	osd.keys = key_cont->ch;
	for (int i = 0; i < 4; ++i)
		osd.keys[i] = _o(0, 17*i,  16, 16,  0.2,  256, 360+16*i,  16, 16, 0, gl.ttm);
	/* panel */
	osd.els[1] = _o(142,-32,     0,  32,   0.8,    0,  90,  1,  1, 0, gl.ttm);
	struct osd_element *freigh_cont, *freigh_img, *sfreigh_cont, *sfreigh_img;
	osdlib_make_children(&osd.els[1], 4, 1, &freigh_cont, &freigh_img,
			&sfreigh_cont, &sfreigh_img);

	*freigh_img   = _o(4,  10, 36, 18, 0.8, 384, 400, 48, 24, 0, gl.ttm);
	size_t num_freigh = gl.cg->level->num_all_freigh;
	int fcont_len = num_freigh * 19 + 8 - 3;
	int fcont_pos = 44;
	*freigh_cont = _o(fcont_pos, 16, fcont_len, 12, 0.2, 4, 302, 1, 1, 0, gl.ttm);
	osdlib_make_children(freigh_cont, num_freigh, 0);
	osd.freigh = freigh_cont->ch;

	fcont_pos += fcont_len + 12;
	*sfreigh_img  = _o(fcont_pos, 10, 36, 18, 0.8, 432, 400, 48, 24, 0, gl.ttm);
	num_freigh = gl.cg->level->num_all_freigh; /**/
	fcont_len = num_freigh * 19 + 8 - 3;
	fcont_pos += 44;
	*sfreigh_cont = _o(fcont_pos, 16, fcont_len, 12, 0.2, 4, 302, 1, 1, 0, gl.ttm);
	/*osdlib_make_children(sfreigh_cont, num_freigh, 0);
	osd.sfreigh = sfreigh_cont->ch;*/
}
void osd_step()
{
	osd.vxbar->x = fmin(64, fmax(0, gl.cg->ship->vx/3 + 31));
	osd.vybar->y = fmin(64, fmax(0, gl.cg->ship->vy/3 + 31));
	for (size_t i = 0; i < (size_t)ceil(gl.cg->ship->fuel); ++i)
		osd.fuel[i].a = 0.8;
	for (size_t i = (size_t)ceil(gl.cg->ship->fuel); i < 16; ++i)
		osd.fuel[i].a = 0.1;
	for (size_t i = 0; i < 4; ++i) {
		if (!gl.cg->ship->keys[i]) {
			osd.keys[i].a = 0.2;
			osd.keys[i].tex_x = 256;
		} else {
			osd.keys[i].tex_x = 256 + 16 * ((int)(gl.cg->time*KEY_ANIM_SPEED + i) % 8);
			osd.keys[i].a = 0.8;
		}
	}
	size_t nfreigh = cg_freigh_remaining(gl.cg);
	size_t k = 0;
	for (size_t i = 0; i < gl.cg->level->nairports; ++i) {
		if (gl.cg->level->airports[i].type == Freigh) {
			struct airport *a = &gl.cg->level->airports[i];
			for (size_t j = 0; j < a->num_cargo; ++j) {
				osd.freigh[k] = _o(4 + k*19, -18, 16, 16, 0.8,
						80+a->c.freigh[j]*16, 392, 16, 16, 0, gl.ttm);
				++k;
			}
		}
	}
	for (size_t i = k; i < nfreigh; ++i)
		osd.freigh[i].transparent = 1;
}
void osd_draw()
{
	for (size_t i = 0; i < osd.num; ++i)
		osdlib_draw(&osd.els[i], 0, 0, gl.win_w, gl.win_h, 0);
}
