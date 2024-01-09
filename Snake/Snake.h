#ifndef SNAKE_H
#define SNAKE_H

#include <list>
#include <unordered_set>
#include <utility>
#include <random>
#include <ctime>

#include "Paint.h"
#include "Segment.h"

class Snake {
private:
	// Hash function for pairs of integers
	struct PairHash {
		inline size_t operator()(const std::pair<int, int>& p) const {
			// Combine hashes of the two integers using bitwise operations
			return (size_t)p.first * 809 + (size_t)p.second;
		}
	};

	Paint* paint;
	/************************************************************************
	*					0 : Snake going up the screen						*
	*					1 : Snake going right								*
	*					2 : Snake going down								*
	*					3 : Snake going left								*
	************************************************************************/
	int tail_orientation;

	std::pair<int, int> head_cords;
	std::pair<int, int> tail_cords;

	float candy_r;
	float candy_g;
	float candy_b;
	float eating_animation_r;
	float eating_animation_g;
	float eating_animation_b;

	std::list<Segment> segments;
	std::unordered_set<std::pair<int, int>, PairHash> freeSpots;

	std::pair<int, int> determineNewCords();
	void getLastSegmentCords(int& x, int& y);
	void randomizeCandy();
	void drawCandy();
	HRESULT drawEatingAnimation();

public:
	bool running;
	int len;
	int orientation;
	int new_orientation;
	bool orientation_changed;
	bool eating_animation;

	std::pair<int, int> candy;


	// when was the last time the snake was drawn
	long long last_time;


	Snake(Paint * p);
	HRESULT draw();
	void moveOneStep();
	void restart();
	void eatCandy(int prev, int last_x, int last_y);
};

#endif