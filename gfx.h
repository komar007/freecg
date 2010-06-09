#ifndef GFX_H
#define GFX_H

#include <SDL/SDL.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xff000000
#define GMASK 0x00ff0000
#define BMASK 0x0000ff00
#define AMASK 0x000000ff
#else
#define RMASK 0x000000ff
#define GMASK 0x0000ff00
#define BMASK 0x00ff0000
#define AMASK 0xff000000
#endif

enum gfx_consts {
	TILESET_W = 588,
	TILESET_H = 464,
	SHIP_W = 23,
	SHIP_H = 23,
	SHIP_ON_IMG_X = 0,
	SHIP_ON_IMG_Y = 0,
	SHIP_OFF_IMG_X = 0,
	SHIP_OFF_IMG_Y = SHIP_H,
	SHIP_NUM_ANGLES = 24
};
typedef uint8_t collision_map[TILESET_H][TILESET_W];

SDL_Surface *read_gfx(const char *path);
int make_collision_map(const SDL_Surface *gfx, collision_map);

#endif
