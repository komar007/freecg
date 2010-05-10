#include "glengine.h"
#include "texmgr.h"
#include <assert.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

void init_opengl(void)
{
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
}

void change_viewport(double x, double y, double w, double h)
{
	assert(w > 0 && h > 0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1, -1, 1);
	glOrtho(x, x+w, y, y+h, 0, 1);
	glMatrixMode(GL_MODELVIEW);
}

void test_draw(struct cgl *cgl, double x1, double y1, double x2, double y2)
{
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
	GLuint texno;
	for (size_t i = 0; i < cgl->ntiles; ++i) {
		if (cgl->tiles[i].x < x1 || cgl->tiles[i].x > x2 ||
				cgl->tiles[i].y < y1 || cgl->tiles[i].y > y2)
			continue;
		texno = tm_request_texture(cgl->tiles[i].img_x, cgl->tiles[i].img_y,
				cgl->tiles[i].w, cgl->tiles[i].h);
		glBindTexture(GL_TEXTURE_2D, texno);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex2f(cgl->tiles[i].x, cgl->tiles[i].y);
		glTexCoord2f(0.0, 1.0);
		glVertex2f(cgl->tiles[i].x, cgl->tiles[i].y + cgl->tiles[i].h);
		glTexCoord2f(1.0, 1.0);
		glVertex2f(cgl->tiles[i].x + cgl->tiles[i].w, cgl->tiles[i].y + cgl->tiles[i].h);
		glTexCoord2f(1.0, 0.0);
		glVertex2f(cgl->tiles[i].x + cgl->tiles[i].w, cgl->tiles[i].y);
		glEnd();
	}
	SDL_GL_SwapBuffers();
}

