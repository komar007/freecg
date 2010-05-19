#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

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
