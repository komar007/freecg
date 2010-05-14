#include "opencg.h"
#include "glengine.h"
#include "texmgr.h"
#include <assert.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

struct glengine gl;

void gl_init(struct cg* cg)
{
	gl.frame = 0;
	gl.cg = cg;
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
}

void gl_change_viewport(double x, double y, double w, double h)
{
	assert(w > 0 && h > 0);
	gl.viewport.x = x, gl.viewport.y = y;
	gl.viewport.w = w, gl.viewport.h = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1, -1, 1);
	glOrtho(x, x+w, y, y+h, 0, 1);
	glMatrixMode(GL_MODELVIEW);
}

void gl_draw_simple_tile(struct tile *tile)
{
	GLuint texno = tm_request_texture(tile->img_x, tile->img_y,
			tile->w, tile->h);
	glBindTexture(GL_TEXTURE_2D, texno);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(tile->x, tile->y);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(tile->x, tile->y + tile->h);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(tile->x + tile->w, tile->y + tile->h);
	glTexCoord2f(1.0, 0.0);
	glVertex2f(tile->x + tile->w, tile->y);
	glEnd();
}

void gl_draw_scene()
{
	extern void fix_lframes(struct cgl*);
	extern void draw_block(struct tile *[]);
	if (gl.frame == 0)
		fix_lframes(gl.cg->level);
	double x1 = gl.viewport.x,
	       y1 = gl.viewport.y,
	       x2 = gl.viewport.x + gl.viewport.w,
	       y2 = gl.viewport.y + gl.viewport.h;
	struct cgl *l = gl.cg->level;
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
	for (size_t j = y1/BLOCK_SIZE; j*BLOCK_SIZE < y2; ++j)
		for (size_t i = x1/BLOCK_SIZE; i*BLOCK_SIZE < x2; ++i)
			draw_block(l->blocks[j][i]);
	SDL_GL_SwapBuffers();
	gl.frame++;
}

void fix_lframes(struct cgl *level)
{
	for (size_t i = 0; i < level->ntiles; ++i)
		level->tiles[i].lframe = 0;
	gl.frame = 1;
}

void draw_block(struct tile *tiles[])
{
	for (size_t i = 0; tiles[i]; ++i) {
		if (tiles[i]->lframe != gl.frame) {
			gl_draw_simple_tile(tiles[i]);
			tiles[i]->lframe = gl.frame;
		}
	}
}
