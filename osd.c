#include "osd.h"
#include "graphics.h"
#include <math.h>

static struct cg_osd osd;

void osd_init()
{
	extern void osd_fuel_init(struct osd_fuel*, struct osd_element*),
	            osd_velocity_init(struct osd_velocity*, struct osd_element*),
		    osd_keys_init(struct osd_keys*, struct osd_element*);
	osd.root = _o(0, 0, gl.win_w, gl.win_h, 1, 0, 0, 0, 0, 1, gl.ttm);
	osdlib_make_children(&osd.root, 2, 1, &osd.rect, &osd.panel);
	/* left rect */
	*osd.rect = _o(0,   -80,  144, 80,  0.8,  0, 90,  1, 1,  0, gl.ttm);
	struct osd_element *fuel_cont, *cross, *key_cont;
	osdlib_make_children(osd.rect, 3, 1, &fuel_cont, &cross, &key_cont);
	osd_fuel_init(&osd.fuel, fuel_cont);
	osd_velocity_init(&osd.velocity, cross);
	osd_keys_init(&osd.keys, key_cont);
	/* panel */
	int panel_w = gl.win_w - 142;
	*osd.panel = _o(142, -32, panel_w, 32,  0.8,  0, 90,  1, 1,  0, gl.ttm);
	struct osd_element *freigh_cont, *freigh_img, *hbfreigh_cont, *hbfreigh_img;
	osdlib_make_children(osd.panel, 4, 1, &freigh_cont, &freigh_img,
			&hbfreigh_cont, &hbfreigh_img);

	*freigh_img   = _o(4,  10, 36, 18, 0.8, 384, 400, 48, 24, 0, gl.ttm);
	size_t num_freigh = gl.cg->level->num_all_freigh;
	int fcont_len = num_freigh * 19 + 8 - 3;
	int fcont_pos = 44;
	*freigh_cont = _o(fcont_pos, 16, fcont_len, 12, 0.2, 4, 302, 1, 1, 0, gl.ttm);
	osdlib_make_children(freigh_cont, num_freigh, 0);
	osd.freigh = freigh_cont->ch;

	fcont_pos += fcont_len + 12;
	*hbfreigh_img  = _o(fcont_pos, 10, 36, 18, 0.8, 480, 400, 48, 24, 0, gl.ttm);
	fcont_pos += 44;
	*hbfreigh_cont = _o(fcont_pos, 16, fcont_len, 12, 0.2, 4, 302, 1, 1, 0, gl.ttm);
	osdlib_make_children(hbfreigh_cont, num_freigh, 0);
	osd.hbfreigh = hbfreigh_cont->ch;
}
void osd_fuel_init(struct osd_fuel *f, struct osd_element *container)
{
	*container  = _o(16, 8,  16, 64,  0.8,  0, 90,  1, 1,  1, gl.ttm);
	osdlib_make_children(container, 16, 0);
	f->bars = container->ch;
	f->bars[15] = _o(0, 0,  16, 3,  0.8,  388, 387,  16, 3, 0, gl.ttm);
	for (int i = 14; i >= 10; --i)
		f->bars[i] = _ro(&f->bars[i+1], 0, -1,  16, 3,  0.8,  388, 387,  16, 3, 0, gl.ttm);
	for (int i = 9; i >= 4; --i)
		f->bars[i] = _ro(&f->bars[i+1], 0, -1,  16, 3,  0.8,  388, 390,  16, 3, 0, gl.ttm);
	for (int i = 3; i >= 0; --i)
		f->bars[i] = _ro(&f->bars[i+1], 0, -1,  16, 3,  0.8,  388, 393,  16, 3, 0, gl.ttm);
}
void osd_velocity_init(struct osd_velocity *v, struct osd_element *container)
{
	*container = _o(40,  8,  64, 64,  0.8,    0, 400,  64, 64,  0, gl.ttm);
	osdlib_make_children(container, 2, 1, &v->xbar, &v->ybar);
	*v->xbar   = _o(31, 16,   2, 32,  0.8,  384, 366,   2, 32,  0, gl.ttm);
	*v->ybar   = _o(16, 31,  32,  2,  0.8,  384, 398,  32,  2,  0, gl.ttm);
}
void osd_keys_init(struct osd_keys *k, struct osd_element *container)
{
	*container = _o(-32, 8,  16, 64,  0.8,    0,  90,   1,  1,  1, gl.ttm);
	osdlib_make_children(container, 4, 0);
	k->keys = container->ch;
	for (int i = 0; i < 4; ++i)
		k->keys[i] = _o(0, 17*i,  16, 16,  0.2,  256, 360+16*i,  16, 16,  0, gl.ttm);
}
void osd_step()
{
	osd.velocity.xbar->x = fmin(64, fmax(0, gl.cg->ship->vx/3 + 31));
	osd.velocity.ybar->y = fmin(64, fmax(0, gl.cg->ship->vy/3 + 31));
	for (size_t i = 0; i < (size_t)ceil(gl.cg->ship->fuel); ++i)
		osd.fuel.bars[i].a = 0.8;
	for (size_t i = (size_t)ceil(gl.cg->ship->fuel); i < 16; ++i)
		osd.fuel.bars[i].a = 0.1;
	for (size_t i = 0; i < 4; ++i) {
		if (!gl.cg->ship->keys[i]) {
			osd.keys.keys[i].a = 0.2;
			osd.keys.keys[i].tex_x = 256;
		} else {
			osd.keys.keys[i].tex_x = 256 + 16 * ((int)(gl.cg->time*KEY_ANIM_SPEED + i) % 8);
			osd.keys.keys[i].a = 0.8;
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
	for (size_t i = k; i < gl.cg->level->num_all_freigh; ++i)
		osd.freigh[i].transparent = 1;

	for (size_t i = 0; i < gl.cg->level->hb->num_cargo; ++i)
		osd.hbfreigh[i] = _o(4 + i*19, -18, 16, 16, 0.9, 80+gl.cg->level->hb->c.freigh[i]*16, 392, 16, 16, 0, gl.ttm);
	for (size_t i = gl.cg->level->hb->num_cargo; i < gl.cg->level->num_all_freigh; ++i)
		osd.hbfreigh[i].transparent = 1;
}
void osd_draw()
{
	osdlib_draw(&osd.root, 0, 0, gl.win_w, gl.win_h, 0);
}
