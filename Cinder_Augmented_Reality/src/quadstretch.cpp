#include "quadstretch.h"  

namespace jttoolkit{
  
  void intersect_lines2(float x0,float y0,float x1,float y1,float x2,float y2,float x3,float y3,float &xi,float &yi){
    //THANK YOU PETE CONOLLY
    float infinity_approx = 999999999999999999999.0f;
    float a1=0.0;
    float b1=0.0;
    float c1=0.0; // constants of linear equations
    float a2=0.0;
    float b2=0.0;
    float c2=0.0;
    float det_inv=0.0;  // the inverse of the determinant of the coefficient
    float m1=0.0;
    float m2=0.0;    // the slopes of each line
    
    if(x1!=x0)m1=(y1-y0)/(x1-x0);
    else m1 = infinity_approx;
    
    if(x3!=x2)m2=(y3-y2)/(x3-x2);
    else m2 = infinity_approx;
    a1=m1;
    a2=m2;
    b1=-1;
    b2=-1;
    c1=(y0-m1*x0);
    c2=(y2-m2*x2);
    // compute the inverse of the determinate
    det_inv = 1/(a1*b2 - a2*b1);
    // use Kramers rule to compute xi and yi
    xi=((b1*c2 - b2*c1)*det_inv);
    yi=((a2*c1 - a1*c2)*det_inv);
  }
  
  
  void quadStretch2(float x,float y,float l,float t,float w,float h,float *q,float &newx,float &newy){
    /*
      --x & y are the original point
      -- l & t are left and top of original rect
      -- w & h are width and height of original rect
      --q is an array of 4 points describing the new quad.
      
      --1. strip it down to floats between 0 and 1
      --   so we know where it is, relative to the boundaries.
    */
    float xF = (x-l)/w;
    float yF = (y-t)/h;
    //2. generate "vertical"
    
    float interpTop[2];
    float interpBottom[2];
    float interpLeft[2];
    float interpRight[2];
    
    interpolatePoints2(	q[0*2+0],q[0*2+1],
			q[1*2+0],q[1*2+1],xF,
			interpTop[0],interpTop[1]);
    
    interpolatePoints2(	q[3*2+0],q[3*2+1],
			q[2*2+0],q[2*2+1],xF,
			interpBottom[0],interpBottom[1]);
    
    //3. generate "horizontal"
    interpolatePoints2(	q[0*2+0],q[0*2+1],
			q[3*2+0],q[3*2+1],yF,
			interpLeft[0],interpLeft[1]);
    
    interpolatePoints2(q[1*2+0],q[1*2+1],
		       q[2*2+0],q[2*2+1],yF,
		       interpRight[0],interpRight[1]);
    
    intersect_lines2(	interpTop[0],		interpTop[1],
			interpBottom[0],	interpBottom[1],
			interpLeft[0],		interpLeft[1],
			interpRight[0],		interpRight[1],
			newx,newy);
  }
  
  
  void interpolatePoints2(float x1,float y1,float x2,float y2,float i,float &x,float &y){
    //i is a float between 0 and 1
    x=x1+(x2-x1)*i;
    y=y1+(y2-y1)*i;
  }
  
  
  void interpolatePoints(float x1,float y1,
			 float x2,float y2,float i,float *ret){
    //   --i is a float between 0 and 1
    ret[0] = x1 + (x2-x1)*i;
    ret[1] = y1 + (y2-y1)*i;
  }
  
  //-----------------------------------------------------------------------------
  void intersect_lines(float x0,float y0,float x1,
		       float y1,float x2,float y2,
		       float x3,float y3,float *ret){
    //   --THANK YOU PETE CONOLLY
#define infinity_approx 999999999999999999999.0
    float a1=0.0;
    float b1=0.0;
    float c1=0.0; // constants of linear equations
    float a2=0.0;
    float b2=0.0;
    float c2=0.0;
    float det_inv=0.0;  // the inverse of the determinant of the coefficient
    float m1=0.0;
    float m2=0.0;    // the slopes of each line
    if (x1 != x0) {
      m1 = (y1-y0)/(x1-x0);
    } else {
      m1 = infinity_approx;
    }
    if (x3 != x2) {
      m2 = (y3-y2)/(x3-x2);
    } else {
      m2 = infinity_approx;
    }
    a1 = m1;
    a2 = m2;
    b1 = -1;
    b2 = -1;
    c1 = (y0-m1*x0);
    c2 = (y2-m2*x2);
    // compute the inverse of the determinate
    det_inv = 1/(a1*b2 - a2*b1);
    // use Kramers rule to compute xi and yi
    float xi=((b1*c2 - b2*c1)*det_inv);
    float yi=((a2*c1 - a1*c2)*det_inv);
    ret[0] = xi;
    ret[1] = yi;
  }
  
  //-----------------------------------------------------------------------------
  
    /**
	 --x & y are the original point
	 -- l & t are left and top of original rect
	 -- w & h are width and height of original rect
	 --qx and qy are arrays of 4 points describing the new quad.
	*/

	void quadstretch(float x,float y,float l,float t,
		   float w,float h,float *qx,float *qy,float *ret){
    
    
    float xF = (x-l)/float(w);
    float yF = (y-t)/float(h);
    // 2. generate "vertical"
    float interpTop[2];
    interpolatePoints(qx[0],qy[0],qx[1],qy[1],xF,interpTop);
    
    float interpBottom[2];
    interpolatePoints(qx[3],qy[3],qx[2],qy[2],xF,interpBottom);
    //3. generate "horizontal"
    float interpLeft[2];
    interpolatePoints(qx[0],qy[0],qx[3],qy[3],yF,interpLeft);
    
    float interpRight[2];
    interpolatePoints(qx[1],qy[1],qx[2],qy[2],yF,interpRight);
    
    intersect_lines(interpTop   [0],   interpTop[1],
		    interpBottom[0],interpBottom[1],
		    interpLeft  [0],  interpLeft[1],
		    interpRight [0], interpRight[1],ret);
  }
  
  //-----------------------------------------------------------------------------
  
#define CMPPNTS(a,b,c,d) ((a==c)&&(b==d))
  
  int intersectLineSegs(float x1,float y1,
			float x2,float y2,
			float x3,float y3,
			float x4,float y4){
    float ix=0;
    float iy=0;
    float m1=0;
    float m2=0;
    float b1=0;
    float b2=0;
    // this code was taken from virtual origami icm final from itp
    // dumbed down only to return true or false, instead of a 
    // full geometric relationships report.
    
    // warning: Z not implemented
    
    if(CMPPNTS(x1,y1,x3,y3)||CMPPNTS(x1,y1,x4,y4)||
       CMPPNTS(x2,y2,x3,y3)||CMPPNTS(x2,y2,x4,y4)){
      //any vertexes have a straight match?
      return true;
    }
    
    //vertical check
    if(x1==x2){//vertical solutions
      m2 = (y3 - y4)/(x3 - x4);
      b2 = y3 - m2 * x3;
      ix = x1;
      iy = m2*x1 + b2;
      
    }else if(x3==x4){//vertical solutions
      m1 = (y1 - y2)/(x1 - x2);
      b1 = y1 - m1 * x1;
      ix = x3;
      iy = m1*x3 + b1;
      
    }else{//otherwise, do it like you would
      // slopes
      m1 = (y1 - y2)/(x1 - x2);
      m2 = (y3 - y4)/(x3 - x4);
      
      if(m1==m2){
	//ir.parallel = true;
	//return ir;
	return false;
      }
      // y-intercepts
      b1 = y1 - m1 * x1;
      b2 = y3 - m2 * x3;
      
      ix = (b1-b2)/(m2-m1);
      iy = m1*ix + b1;
    }
    
    //and finally, check to see if it is within both segments.
    //first, the Xs
    
    if(x1>x2){
      if(ix > x1 || ix < x2)return false;//ir.outside = true;
    }else{
      if(ix < x1 || ix > x2)return false;//ir.outside = true;
    }
    
    if(x3>x4){
      if(ix > x3 || ix < x4)return false;//ir.outside = true;
    }else{
      if(ix < x3 || ix > x4)return false;//ir.outside = true;
    }
    
    //then the Y's
    
    if(y1>y2){
      if(iy > y1 || iy < y2)return false;//ir.outside = true;
    }else{
      if(iy < y1 || iy > y2)return false;//ir.outside = true;
    }
    
    if(y3>y4){
      if(iy > y3 || iy < y4)return false;//ir.outside = true;
    }else{
      if(iy < y3 || iy > y4)return false;//ir.outside = true;
    }
    
    //  if(ir.outside){
    // return false;
    //return ir;
    // }else{
    //ir.ok = true;
    return true;
    //return ir;
    //}
  }
};
