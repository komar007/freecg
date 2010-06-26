/* gfx.h - definitions of texture coordinates and GFX parser functions
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
	SHIP_ON_IMG_Y = 92,
	SHIP_OFF_IMG_X = 0,
	SHIP_OFF_IMG_Y = 92 + SHIP_H,
	SHIP_NUM_ANGLES = 24,
	FAN_NUM_TEXTURES = 3,
	MAGNET_NUM_TEXTURES = 3,
	AIRGEN_NUM_TEXTURES = 8,
	BAR_TEX_LEN = 308,
	BAR_THICKNESS = 12,
	VBAR_TEX_X = 552,
	VBAR_TEX_Y = 0,
	HBAR_TEX_X = 240,
	HBAR_TEX_Y = 80,
	BAR_BASE_W = 24,
	BAR_BASE_H = 20,
	BAR_BEG_TEX_X = 496,
	BAR_BEG_TEX_Y = 56,
	VBAR_END_TEX_X = 496,
	VBAR_END_TEX_Y = 52,
	HBAR_END_TEX_X = 492,
	HBAR_END_TEX_Y = 56,
	GATE_BAR_THICKNESS = 32,
	GATE_BAR_LEN = 160,
	VGATE_TEX_X = 488,
	VGATE_TEX_Y = 228,
	HGATE_TEX_X = 232,
	HGATE_TEX_Y = 296,
	ARROW_TEX_Y = 384,
	ARROW_SIDE = 16,
	ARROW_OFFSET = 8,
	LVGATE_TEX_X = 520,
	LVGATE_TEX_Y = 228,
	LHGATE_TEX_X = 232,
	LHGATE_TEX_Y = 328,
	LIGHTS_TEX_X = 64,
	LIGHTS_TEX_Y = 384,
	STUFF_SIZE = 16,
	STUFF_TEX_X = 64,
	STUFF_TEX_Y = 392,
	KEY_TEX_X = 256,
	KEY_TEX_Y = 360,
	STRIPE_ORYG_X = 392,
	STRIPE_ORYG_Y = 316,
	STRIPE_ORYG_W = 84,
	STRIPE_END_W = 26,
	STRIPE_H = 8,
	STRIPE_OFFS = 6
};
typedef uint8_t collision_map[TILESET_H][TILESET_W];

SDL_Surface *load_gfx(const char*);
SDL_Surface *load_png(const char*);
int make_collision_map(const SDL_Surface *gfx, collision_map);

#endif
