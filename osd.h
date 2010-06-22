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
struct cg_osd {
	struct osd_element root;
	struct osd_fuel fuel;
	struct osd_velocity velocity;
	struct osd_keys keys;
	struct osd_element *rect,
			   *panel;


	struct osd_element *freigh,
			   *hbfreigh;
};

void osd_init();
void osd_step();
void osd_draw();

#endif
