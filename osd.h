/* osd.h - OSD data structures
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

#ifndef OSD_H
#define OSD_H

#include "osdlib.h"

struct osd_fuel {
	struct osd_element *bars;
};
struct osd_velocity {
	struct osd_element *xbar,
			   *ybar,
			   *mxbar1,
			   *mxbar2,
			   *mybar;
};
struct osd_keys {
	struct osd_element *keys;
};
struct osd_shipinfo {
	struct osd_element *container;
	struct osd_fuel     fuel;
	struct osd_velocity velocity;
	struct osd_keys     keys;
};
struct osd_freight {
	size_t max_freight;
	size_t old_max_freight;
	struct osd_element *freight;
	struct osd_element *container;
};
struct osd_life {
	struct osd_element *ships;
	size_t max_life;
};
struct osd_panel {
	struct osd_element *container;
	struct osd_freight lfreight,
			   sfreight,
			   hbfreight;
	struct osd_life    life;
};
struct osd_timer {
	struct osd_element *container;
	struct osd_element *time;
};
struct cg_osd {
	int visible;
	struct osd_layer *layer;
	struct osdlib_font font;

	struct osd_shipinfo shipinfo;
	struct osd_panel    panel;
	struct osd_timer    timer;

	/* deprecated */
	struct osd_element *victory,
			   *gameover;
};

void osd_init();
void osd_step();
void osd_draw();
void osd_free();
void osd_show();
void osd_hide();
void osd_toggle();

#endif
