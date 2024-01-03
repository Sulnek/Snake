#include "Segment.h"

Segment::Segment(int prev, int next, int x_cord, int y_cord, float r, float g, float b) {
	red = r;
	green = g;
	blue = b;
	next_side = next;
	prev_side = prev;
	x = x_cord;
	y = y_cord;
}

HRESULT Segment::draw(Paint* paint) {
	if (prev_side % 2 == next_side % 2) {
		// Straight segment
		return paint->drawStraightSegment(x, y, next_side, D2D1::ColorF(red, green, blue));
	}
	// Curved segment:

	// If turning left:
	int orientation = next_side;
	if ((next_side + 1) % 4 == prev_side) {
		// If turning right
		orientation = prev_side;
	}
	return paint->drawCurvedSegment(x, y, orientation, D2D1::ColorF(red, green, blue));
}

int Segment::move(int prev) {
	int ret = prev_side;
	if (prev_side == 0) {
		x--;
	}
	if (prev_side == 1) {
		y++;
	}
	if (prev_side == 2) {
		x++;
	}
	if (prev_side == 3) {
		y--;
	}

	next_side = (2 + prev_side) % 4; // we're noting where we came from
	prev_side = prev;
	return ret;
}