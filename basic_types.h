#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

enum dir {
	Down = 0,
	Up,
	Left,
	Right
};
typedef struct vector {
	int x,
	    y;
} vector;
struct rect {
	int x, y;
	unsigned int w, h;
};
struct drect {
	double x, y;
	double w, h;
};

#endif
