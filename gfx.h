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

SDL_Surface *read_gfx(const char *path);
uint8_t gfx_get_alpha(SDL_Surface*, int, int);

#endif
