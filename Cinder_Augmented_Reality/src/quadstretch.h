#ifndef _QUADSTRETCH_H_
#define QUADSTRETCH_H_

namespace jttoolkit{

/**
  functions to help with digital project keystoning. Can skew and deskew a point.
  @author jtnimoy
*/

void intersect_lines2(float x0,float y0,float x1,float y1,float x2,float y2,float x3,float y3,float &xi,float &yi);

void interpolatePoints2(float x1,float y1,float x2,float y2,float i,float &x,float &y);

void quadstretch(float x,float y,float l,float t,
		 float w,float h,float *qx,float *qy,float *ret);


void quadStretch2(float x,float y,float l,float t,float w,float h,float *q,float &newx,float &newy);

void interpolatePoints(float x1,float y1,
		       float x2,float y2,float i,float *ret);

void intersect_lines(float x0,float y0,float x1,
		     float y1,float x2,float y2,
		     float x3,float y3,float *ret);

int intersectLineSegs(float x1,float y1,
		      float x2,float y2,
		      float x3,float y3,
		      float x4,float y4);

};

#endif
