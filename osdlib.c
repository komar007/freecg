/* osdlib.c - simple OSD widget library used by FreeCG - main source
 * Copyright (C) 2010 Michal Trybus.
 *
 * This file is part of FreeCG.
 *
 * FreeCG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * FreeCG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FreeCG. If not, see <http://www.gnu.org/licenses/>.
 */

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
		e->ch[i].tr = TransparentSubtree;
	if (!init)
		return;
	va_list ptrs;
	va_start(ptrs, init);
	for (size_t i = 0; i < num; ++i)
		*va_arg(ptrs, struct osd_element**) = &e->ch[i];
	va_end(ptrs);
}

void osdlib_make_text(struct osd_element *e, const struct osdlib_font *font,
		const char *str)
{
	size_t len = strlen(str);
	e->w = len * font->w;
	e->h = font->h;
	osdlib_make_children(e, len, 0);
	for (size_t i = 0; i < len; ++i) {
		int tx = font->tex_x + font->w*(str[i] - font->offset);
		e->ch[i] = _o(font->w*i, 0, font->w, font->h, e->a, tx, font->tex_y,
				font->w, font->h, 0, font->t);
	}
}

void center_on_screen(struct osd_element *e)
{
	e->x = gl.win_w/2 - e->w/2,
	e->y = gl.win_h/2 - e->h/2;
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
	x = e->rx >= 0 ? px + e->rx : px + pw + e->rx - w;
	y = e->ry >= 0 ? py + e->ry : py + ph + e->ry - h;
	z = pz + 0.01 + e->z;
	if (e->tr == Opaque) {
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
	if (e->tr != TransparentSubtree) {
		/* recurse into the children */
		for (size_t i = 0; i < e->nch; ++i)
			osdlib_draw(&e->ch[i], x, y, w, h, z);
	}
}
