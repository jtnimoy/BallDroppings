/*
 *  EditLine.cpp
 *  BallDroppings
 *
 *  Created by Josh Nimoy on 9/6/10.
 *
 */

#include "EditLine.h"

EditLine::EditLine(){
	p2 = p1 = Vec2d(0,0);
}

bool EditLine::checkBallCollide(Ball ball){

	Vec2d lineLocalVec = Vec2d(p2.x-p1.x, p2.y-p1.y);
	
	//get the angle between the ball and one end of the wall
	double angleCurrent1 = checkAngle(ball.x , ball.y , p1.x,p1.y, lineLocalVec);  
	double angleCurrent2 = checkAngle(ball.x , ball.y , p2.x,p2.y, lineLocalVec);  
	
	//lets get the angle between the ball and one end of the wall
	double angleFuture1 = checkAngle(ball.x+ball.force.x, ball.y+ball.force.y
									   ,p1.x,p1.y, lineLocalVec);
	double angleFuture2 = checkAngle(ball.x+ball.force.x, ball.y+ball.force.y
									   ,p2.x,p2.y, lineLocalVec);
	
	if(diffSign(angleCurrent1,angleFuture1) && diffSign(angleCurrent2,angleFuture2)){
	    double d1x = ball.x - p1.x;
	    double d1y = ball.y - p1.y;
	    double d2x = ball.x - p2.x;
	    double d2y = ball.y - p2.y;
	    double wallLength = lineLocalVec.length();
	    if( (sqrt(d1x*d1x + d1y*d1y) < wallLength) && (sqrt(d2x*d2x + d2y*d2y) < wallLength)){
			return true;
	    }
	    else return false;
	}
	else return false;
}


bool EditLine::diffSign(double v1,double v2){
	if( (v1 >= 0 && v2 < 0 ) || (v2 >= 0 && v1 < 0 ) )return true;
	else return false;
}


double EditLine::checkAngle(  double point_x,  double point_y, double line_x,  double line_y,  Vec2d lineVec){
	Vec2d vec = Vec2d(line_x - point_x, line_y - point_y);
	Vec2d vecline = Vec2d(0,0);
	vecline = Vec2d(lineVec.x,lineVec.y);
	
	vecline = Vec2d(vecline.y , -vecline.x);//getRightNormal
	
	vec.normalize();
	vecline.normalize();
	return vec.dot(vecline);
	
}
