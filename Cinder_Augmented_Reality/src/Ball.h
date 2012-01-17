/*
 *  Ball.h
 *  BallDroppings
 *
 *  Created by Josh Nimoy on 9/2/10.
 *
 */

#pragma once

#include "cinder/Vector.h"
#include "cinder/audio/Output.h"
#include "cinder/audio/Io.h"
#include "Resources.h"
#include "jofSoundPlayer.h"

#include <deque>


using namespace ci;
using namespace std;

class Ball:public Vec2d{
public:
	Ball(float x, float y);
	Vec2d force;
	static Vec2d gravity;
	static double friction;
	void step();
	float radius;
	void bounce( double x1, double y1, double x2, double y2 , ofSoundPlayer &snd);
	bool dead;
	deque<int>bounces;
	int bounceTimer;
};

