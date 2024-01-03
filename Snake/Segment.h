#pragma once
#ifndef SEGMENT_H
#define SEGMENT_H

#include "Paint.h"

class Segment {
public:
	int prev_side;
	int next_side;

	int x;
	int y;

	float red;
	float green;
	float blue;

	Segment(int prev, int next, int x_cord, int y_cord, float r, float g, float b);

	HRESULT draw(Paint* paint);

	int move(int prev);
};



#endif
