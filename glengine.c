#include "glengine.h"
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

void test_draw(struct cgl *cgl, SDL_Surface *gfx)
{
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_Surface *tile;
	GLuint texno;
	SDL_Rect rect;
	for (size_t i = 0; i < cgl->ntiles; ++i) {
		tile = SDL_CreateRGBSurface(0, cgl->tiles[i].w, cgl->tiles[i].h,
				32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
		rect.x = cgl->tiles[i].img_x;
		rect.y = cgl->tiles[i].img_y;
		rect.w = cgl->tiles[i].w;
		rect.h = cgl->tiles[i].h;
		SDL_BlitSurface(gfx, &rect, tile, NULL);
		texno = load_texture(tile);
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
		SDL_FreeSurface(tile);
	}
	SDL_GL_SwapBuffers();
}

GLuint load_texture(SDL_Surface *image)
{
	GLuint texno;
	glGenTextures(1, &texno);
	glBindTexture(GL_TEXTURE_2D, texno);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	return texno;
}
