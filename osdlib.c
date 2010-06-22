#include "osdlib.h"
#include "graphics.h"
#include "texmgr.h"
#include <stdarg.h>
#include <float.h>

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

/* recursively count position relative to sibling */
void count_rel(struct osd_element *e)
{
	if (e->rx != DBL_MAX)
		return;
	if (e->rel) {
		count_rel(e->rel);
		e->rx = e->x >= 0 ? e->x + e->rel->rx : e->rel->rx + e->rel->w - e->x;
		e->ry = e->y >= 0 ? e->y + e->rel->ry : e->rel->ry + e->rel->h - e->y;
	} else {
		e->rx = e->x;
		e->ry = e->y;
	}
}
void osdlib_draw(struct osd_element *e, double px, double py,
		double pw, double ph, double pz)
{
	double x, y, z, w, h;
	w = e->w > 0 ? e->w : pw + e->w;
	h = e->h > 0 ? e->h : ph + e->h;
	count_rel(e);
	x = e->rx >= 0 ? px + e->rx : px + pw + e->rx;
	y = e->ry >= 0 ? py + e->ry : py + ph + e->ry;
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
	/* clean memoized vaules for children */
	for (size_t i = 0; i < e->nch; ++i)
		e->ch[i].rx = e->ch[i].ry = DBL_MAX;
	/* recurse into the children */
	for (size_t i = 0; i < e->nch; ++i)
		osdlib_draw(&e->ch[i], x, y, w, h, z);
}
