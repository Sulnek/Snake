#include "Snake.h"


Snake::Snake(Paint * p) {
	this->restart();
	paint = p;
}

void Snake::restart() {
	orientation = 1;
	new_orientation = orientation;
	orientation_changed = false;
	tail_orientation = 1;
	len = 2;
	eating_animation = false;

	head_cords = std::pair<int, int>(0, 1);
	tail_cords = std::pair<int, int>(0, 0);

	segments.clear();

	running = true;
	for (int x = 0; x < GRID_HEIGHT; x++) {
		for (int y = 0; y < GRID_WIDTH; y++) {
			if (!(x == head_cords.first && y == head_cords.second) &&
				!(x == tail_cords.first && y == tail_cords.second)) {
				freeSpots.insert(std::pair<int, int>(x, y));
			}
		}
	}
	randomizeCandy();
}

HRESULT Snake::draw() {
	HRESULT hr;
	for (Segment& segment : segments) {
		hr = segment.draw(paint);
		if (FAILED(hr)) {
			return hr;
		}
	}
	hr = paint->drawHead(head_cords.first, head_cords.second, orientation);
	if (FAILED(hr)) {
		return hr;
	}
	hr = paint->drawTail(tail_cords.first, tail_cords.second, tail_orientation);
	if (FAILED(hr)) {
		return hr;
	}
	if (eating_animation) {
		hr = drawEatingAnimation();
		if (FAILED(hr)) {
			return hr;
		}
	}
	drawCandy();
	return S_OK;
}

bool checkIfOutOfBounds(const std::pair<int, int>& p) {
	if (p.first < 0 || p.first >= GRID_HEIGHT || p.second < 0 || p.second >= GRID_WIDTH) {
		return true;
	}
	return false;
}

std::pair<int, int> Snake::determineNewCords() {
	int new_x = head_cords.first;
	int new_y = head_cords.second;
	if (orientation == 0) {
		new_x--;
	}
	else if (orientation == 1) {
		new_y++;
	}
	else if (orientation == 2) {
		new_x++;
	}
	else {
		new_y--;
	}
	return std::pair<int, int>(new_x, new_y);
}

void Snake::getLastSegmentCords(int& x, int& y) {
	if (len > 2) {
		x = segments.back().x;
		y = segments.back().y;
	}
	else {
		x = head_cords.first;
		y = head_cords.second;
	}
}

void Snake::drawCandy() {
	paint->drawCandy(candy.first, candy.second, D2D1::ColorF(candy_r, candy_g, candy_b));
}

void Snake::randomizeCandy() {
	std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
	std::uniform_real_distribution<float> distrib_f(0.0f, 1.0f);
	std::uniform_int_distribution<int> distrib_i(0, (int) freeSpots.size() - 1);

	candy_r = distrib_f(rng);
	candy_g = distrib_f(rng);
	candy_b = distrib_f(rng);

	int pos = distrib_i(rng);
	int i = 0;
	for (auto it : freeSpots) {
		i++;
		if (i == pos) {
			candy.first = it.first;
			candy.second = it.second;
			return;
		}
	}
}

void Snake::eatCandy(int prev, int last_x, int last_y) {
	eating_animation = true;
	eating_animation_r = candy_r;
	eating_animation_g = candy_g;
	eating_animation_b = candy_b;

	segments.push_back(Segment(
		prev,
		(tail_orientation + 2) % 4,
		last_x, last_y,
		candy_r, candy_g, candy_b));
	len++;
	randomizeCandy();
}

HRESULT Snake::drawEatingAnimation() {
	return paint->drawEatingAnimation(
		head_cords.first,
		head_cords.second,
		orientation,
		D2D1::ColorF(eating_animation_r, eating_animation_g, eating_animation_b));
}

void Snake::moveOneStep() {
	eating_animation = false;
	orientation = new_orientation;
	std::pair<int, int> new_head_cords = determineNewCords();
	bool lengthen = (candy.first == new_head_cords.first && candy.second == new_head_cords.second);
	if (!lengthen) {
		freeSpots.insert(tail_cords);
	}

	auto it = freeSpots.find(new_head_cords);
	if (checkIfOutOfBounds(new_head_cords) || freeSpots.end() == it) {
		running = false;
	}
	else {
		int last_segment_x, last_segment_y;
		getLastSegmentCords(last_segment_x, last_segment_y);
		head_cords.first = new_head_cords.first;
		head_cords.second = new_head_cords.second;
		freeSpots.erase(it);
		int prev_element = orientation; // tells each segment where the previous one went
		for (Segment& seg : segments) {
			prev_element = seg.move(prev_element);
		}
		if (lengthen) {
			eatCandy(prev_element, last_segment_x, last_segment_y);
		}
		else {
			tail_orientation = prev_element;
			tail_cords = std::pair<int, int>(last_segment_x, last_segment_y);
		}
	}
	orientation_changed = false;
}