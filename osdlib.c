#include "osdlib.h"
#include "graphics.h"
#include "texmgr.h"

void osdlib_draw(const struct osd_element *e, size_t size)
{
	/* convert relative coords to absolute */
	struct osd_element *el;
	gl_bind_texture(gl.ttm);
	glBegin(GL_QUADS);
	el = calloc(size, sizeof(*el));
	memcpy(el, e, size * sizeof(*el));
	for (size_t i = 0; i < size; ++i) {
		if (el[i].rel == -1) {
			el[i].x = el[i].x >= 0 ? el[i].x : el[i].x + gl.win_w;
			el[i].y = el[i].y >= 0 ? el[i].y : el[i].y + gl.win_h;
			el[i].w = el[i].w >  0 ? el[i].w : el[i].w + gl.win_w;
			el[i].h = el[i].h >  0 ? el[i].h : el[i].h + gl.win_h;
		} else {
			el[i].x = el[i].x + el[el[i].rel].x;
			el[i].y = el[i].y + el[el[i].rel].y;
			el[i].w = el[i].w >  0 ? el[i].w : el[i].w + el[el[i].rel].w;
			el[i].h = el[i].h >  0 ? el[i].h : el[i].h + el[el[i].rel].h;
		}
		if(el[i].texrel != -1) {
			el[i].tex_x = el[i].tex_x + el[el[i].texrel].tex_x;
			el[i].tex_y = el[i].tex_y + el[el[i].texrel].tex_y;
			el[i].tex_w = el[i].tex_w >  0 ? el[i].tex_w :
				el[i].tex_w + el[el[i].texrel].tex_w;
			el[i].tex_h = el[i].tex_h >  0 ? el[i].tex_h :
				el[i].tex_h + el[el[i].texrel].tex_h;
		}
		glColor4f(1, 1, 1, el[i].a);
		tm_coord_tl(el[i].t, el[i].tex_x, el[i].tex_y,
				el[i].tex_w, el[i].tex_h);
		glVertex3d(el[i].x, el[i].y, el[i].z);
		tm_coord_bl(el[i].t, el[i].tex_x, el[i].tex_y,
				el[i].tex_w, el[i].tex_h);
		glVertex3d(el[i].x, el[i].y + el[i].h, el[i].z);
		tm_coord_br(el[i].t, el[i].tex_x, el[i].tex_y,
				el[i].tex_w, el[i].tex_h);
		glVertex3d(el[i].x + el[i].w, el[i].y + el[i].h, el[i].z);
		tm_coord_tr(el[i].t, el[i].tex_x, el[i].tex_y,
				el[i].tex_w, el[i].tex_h);
		glVertex3d(el[i].x + el[i].w, el[i].y, el[i].z);
	}
	glEnd();
}
