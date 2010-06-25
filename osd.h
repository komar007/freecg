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
			   *ybar;
};
struct osd_keys {
	struct osd_element *keys;
};
struct osd_freight {
	size_t max_freight;
	struct osd_element *freight;
	struct osd_element *container;
};
struct cg_osd {
	struct osd_element root;
	struct osd_fuel fuel;
	struct osd_velocity velocity;
	struct osd_keys keys;
	struct osd_freight freight_level,
			  freight_ship,
			  freight_hb;
	struct osd_element *rect,
			   *panel,
			   *pause;
};

void osd_init();
void osd_step();
void osd_draw();

#endif
