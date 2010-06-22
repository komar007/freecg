#ifndef OSD_H
#define OSD_H

#include "osdlib.h"

struct cg_osd {
	size_t num;
	struct osd_element *els;
	struct osd_element *vxbar,
			   *vybar,
			   *fuel,
			   *keys,
			   *freigh,
			   *sfreigh;
};

void osd_init();
void osd_step();
void osd_draw();

#endif
