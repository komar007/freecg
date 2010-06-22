#include "osdlib.h"
#include "graphics.h"
#include "texmgr.h"
#include <stdarg.h>

void osdlib_make_children(struct osd_element *e, size_t num, int init, ...)
{
	e->ch = calloc(num, sizeof(*e->ch));
	e->nch = num;
	for (size_t i = 0; i < num; ++i)
		e->ch[i].transparent = 1;
	if (!init)
		return;
	va_list ptrs;
	va_start(ptrs, init);
	for (size_t i = 0; i < num; ++i)
		*va_arg(ptrs, struct osd_element**) = &e->ch[i];
	va_end(ptrs);
}

void osdlib_draw(const struct osd_element *e, double px, double py,
		double pw, double ph, double pz)
{
	double x, y, z, w, h;
	x = e->x >= 0 ? px + e->x : px + pw + e->x;
	y = e->y >= 0 ? py + e->y : py + ph + e->y;
	w = e->w > 0 ? e->w : pw + e->w;
	h = e->h > 0 ? e->h : ph + e->h;
	z = pz + 0.01;
	if (!e->transparent) {
		gl_bind_texture(e->t);
		glBegin(GL_QUADS);
		glColor4f(1, 1, 1, e->a);
		tm_coord_tl(e->t, e->tex_x, e->tex_y,
				e->tex_w, e->tex_h);
		glVertex3d(x, y, z);
		tm_coord_bl(e->t, e->tex_x, e->tex_y,
				e->tex_w, e->tex_h);
		glVertex3d(x, y + h, z);
		tm_coord_br(e->t, e->tex_x, e->tex_y,
				e->tex_w, e->tex_h);
		glVertex3d(x + w, y + h, z);
		tm_coord_tr(e->t, e->tex_x, e->tex_y,
				e->tex_w, e->tex_h);
		glVertex3d(x + w, y, z);
		glEnd();
	}
	for (size_t i = 0; i < e->nch; ++i)
		osdlib_draw(&e->ch[i], x, y, w, h, z);
}
