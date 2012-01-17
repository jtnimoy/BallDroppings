/*
 *  Ball.cpp
 *  BallDroppings
 *
 *  Created by Josh Nimoy on 9/2/10.
 *
 */

#include "Ball.h"
#include <iostream>

using namespace std;

Vec2d Ball::gravity = Vec2d(0,0.3);
double Ball::friction = 0.999;

Ball::Ball(float xx,float yy):Vec2d(xx,yy){
	radius = 3;
	dead = false;
	bounceTimer = 0;
	force = Vec2d(0,0);
}

void Ball::step(){
	(*this) += force;
	force += gravity;
	force *= friction;
	radius--;
	if(radius<3)radius = 3;
	
	
	//kill if bouncing too often
	if(bounces.size()>5)bounces.pop_front();
	if(bounces.size()==5){
		int accum = 0;
		deque<int>::iterator it=bounces.begin();
		for(;it!=bounces.end();it++){
			accum += (*it);
		}
		if(accum < 10){
			dead = true;
		}
	}

	bounceTimer++;
}


void Ball::bounce( double x1, double y1, double x2, double y2 , ofSoundPlayer &snd){
	//Thank you to Theo Watson for helping me out here.
	//V
	Vec2d v = Vec2d( force.x,force.y );
	
	//N
	Vec2d n = Vec2d(x2-x1,y2-y1);
	n = Vec2d(-n.y , n.x);
	n.normalize();
	
	//2 * V [dot] N
	double dotVec = 2 * v.dot(n);
	
	// ( 2 * V [dot] N ) N
	n *= dotVec;
	
	//V - ( 2 * V [dot] N ) N
	//change direction
	Vec2d mvn = Vec2d(v.x - n.x, v.y - n.y);//minVecNew

	force.x = mvn.x;
	force.y = mvn.y;
	
	//enlarge the ball
	
	radius = sqrt(force.x*force.x + force.y*force.y);

	snd.setSpeed(force.length()*0.2);
	snd.play();
	
	bounces.push_back(bounceTimer);
	bounceTimer=0;
}

