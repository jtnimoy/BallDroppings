/*
 *  EditLine.h
 *  BallDroppings
 *
 *  Created by Josh Nimoy on 9/6/10.
 *
 */

#pragma once

#include "cinder/Vector.h"
#include "Ball.h"

using namespace ci;

class EditLine{
public:
	EditLine();
	Vec2d p1;
	Vec2d p2;
	bool checkBallCollide(Ball);
	bool diffSign(double v1,double v2);
	double checkAngle(  double point_x,  double point_y, double line_x,  double line_y,  Vec2d lineVec);

};

