#include "cinder/app/AppBasic.h"
#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include <deque>
#include <fstream>
#include "Ball.h"
#include "EditLine.h"
#include "quadstretch.h"

#include "cinder/audio/Output.h"
#include "cinder/audio/Io.h"
#include "Resources.h"
#include "jofSoundPlayer.h"


using namespace ci;
using namespace ci::app;
using namespace std;
using namespace jttoolkit;

#define CAMW 320
#define CAMH 240

#define SETTINGS_FILENAME "/Contents/Resources/balldroppingsAR_Settings.txt"

class BallDroppingsApp : public AppBasic {
 public:
	void setup();
	void mouseDrag( MouseEvent event );
	void mouseMove( MouseEvent event );
	void keyDown( KeyEvent event );
	void mouseDown( MouseEvent event );
	void mouseUp( MouseEvent event );
	
	void draw();
	void update();
	Vec2d emitterPos;
	Capture cam;
	deque<Ball> balls;
	int framesPerBall;
	unsigned char*camPixels;
	Surface camSurface;
	double collideLimit;
	deque<EditLine> lines;
	bool mouseIsDown;
	Vec2i mousePos;
	bool drawCam;
	deque<Vec2i> quadPoints;
	bool showQuad;
	int quadPointGrabbed;
	bool showThresh;
	ofSoundPlayer ball_sound;
	bool getCamPixel(int x,int y);
	void getPixelEdge(int x,int y, Vec2f &retVec);
};


void BallDroppingsApp::getPixelEdge(int x,int y, Vec2f &retVec){
	int steps = 30;
	float inc = (M_PI*2)/steps;
	int radSteps = 10;
	float radInc = 0.5;
	deque<float> thetas;
	
	//loop through 1 PI
	float readingsAverage = 0;
	for(int i=0;i<steps;i++){
		int accum = 0;
		//loop through radii and log pixel values
		for(int j = 0 ; j < radSteps ; j++){
			int xx = x + (j*radInc) * cos(inc*i);
			int yy = y + (j*radInc) * sin(inc*i);
			//push back the pixel readings
			accum += getCamPixel(xx,yy);
		}
		readingsAverage += accum;
	}
	
	//get average of readings
	readingsAverage /= steps;
	
	//loop through 1 PI
	for(int i=0;i<steps;i++){
		int accum = 0;
		//loop through radii and log pixel values
		for(int j = 0 ; j < radSteps ; j++){
			int xx = x + (j*radInc) * cos(inc*i);
			int yy = y + (j*radInc) * sin(inc*i);
			//push back the pixel readings
			accum += getCamPixel(xx,yy);
		}
		if(accum<readingsAverage)thetas.push_back(inc*i);
	}
	
	
	//average results
	if(thetas.size()>1){
		float averageTheta = 0;
		deque<float>::iterator it = thetas.begin();
		for(;it!=thetas.end();it++)averageTheta += (*it);
		averageTheta /= thetas.size();
		
		//perpendicular
		averageTheta += (M_PI/2);
		
		retVec.x = cos(averageTheta);
		retVec.y = sin(averageTheta);
	}else{
		Rand rnd;
		retVec.x = 10;
		retVec.y = rnd.nextFloat(-5, 5);
	}
}


bool BallDroppingsApp::getCamPixel(int x,int y){
	if(!camSurface)return false;
	int cW = cam.getWidth();
	int pixInc = camSurface.getPixelInc();
	if(x < CAMW && x >= 0 && y < CAMH && y >= 0){
		try{
			float r = camPixels[((int)y*cW+(int)x)*pixInc  ]/255.0;
			float g = camPixels[((int)y*cW+(int)x)*pixInc+1]/255.0;
			float b = camPixels[((int)y*cW+(int)x)*pixInc+2]/255.0;
			return (r+g+b)/3.0 < collideLimit;
		}catch(...){
			return false;
		}
	}else return false;
}

void BallDroppingsApp::setup(){
	
	setWindowSize(640,480);
	//setFullScreen(true);
	emitterPos = Vec2d( 100,50 );
	
	//start video capture
	try{
		cam = Capture(CAMW,CAMH,Capture::findDeviceByName("unibrain Fire-i"));
		//cam = Capture(CAMW,CAMH);
		
		//list the devices
		/*
		std::vector<Capture::Device> devices = cam.getDevices();
		cout << "----- begin device listing -----" << endl;
		for(int i=0;i<devices.size();i++){
			cout << devices[i].getName() << endl;
		}
		cout << "----- end device listing -----" << endl;
		 */
		cam.start();
	}catch(...){
		console() << "cam failed to start" << endl;
	}
	
	framesPerBall = 100;
	collideLimit = 0.2;
	mouseIsDown = false;
	camPixels = new unsigned char[CAMW*CAMH*3];
	drawCam = false;
	showQuad = false;
	quadPoints.push_back(Vec2d(100,100));
	quadPoints.push_back(Vec2d(400,150));
	quadPoints.push_back(Vec2d(350,400));
	quadPoints.push_back(Vec2d(90,400));
	quadPointGrabbed = 0;
	showThresh = false;
	
	//load from settings file
	ifstream infile((getAppPath() + SETTINGS_FILENAME).c_str());
	if(infile.good()){
		infile >> quadPoints[0].x;
		infile >> quadPoints[0].y;
		infile >> quadPoints[1].x;
		infile >> quadPoints[1].y;
		infile >> quadPoints[2].x;
		infile >> quadPoints[2].y;
		infile >> quadPoints[3].x;
		infile >> quadPoints[3].y;
		infile >> emitterPos.x;
		infile >> emitterPos.y;
		infile >> collideLimit;
		int fullscreenFlag;
		infile >> fullscreenFlag;
		setFullScreen(fullscreenFlag);
		
	}else{
		cout << "settings file not found." << endl;
	}	
	
	ball_sound.loadSound(getAppPath() + "/Contents/Resources/sineStereo.wav");
	ball_sound.setMultiPlay(true);

	gl::enableAlphaBlending(true);
}

void BallDroppingsApp::mouseDrag( MouseEvent event ){
	mousePos = event.getPos();
	
	if(quadPointGrabbed){
		quadPoints[quadPointGrabbed-1] = mousePos;
	}else{
		lines.back().p2 = mousePos;	
	}
}

void BallDroppingsApp::mouseMove( MouseEvent event ){
	mousePos = event.getPos();
}


void BallDroppingsApp::keyDown( KeyEvent event ){
	switch(event.getChar()){
		case 27:
			quit();
		case 'f':
			setFullScreen(!isFullScreen());
			break;
		case '-':
			collideLimit -= 0.01;
			console() << "collide limit: " << collideLimit << endl;
			break;
		case '=':
			collideLimit += 0.01;
			console() << "collide limit: " << collideLimit << endl;
			break;
		case ' ':
			balls.clear();
			lines.clear();
			break;
		case 'c':
			drawCam = !drawCam;
			break;
		case 'q':
			showQuad = !showQuad;
			break;
		case 'Q':
			quadPoints[0].x = 0;
			quadPoints[0].y = 0;
			quadPoints[1].x = getWindowWidth();
			quadPoints[1].y = 0;
			quadPoints[2].x = getWindowWidth();
			quadPoints[2].y = getWindowHeight();
			quadPoints[3].x = 0;
			quadPoints[3].y = getWindowHeight();
			
			break;
		case 'b':
			emitterPos = mousePos;
			break;
		case 'x':
			showThresh = !showThresh;
			break;
		case 's':
			ofstream outfile( (getAppPath() + SETTINGS_FILENAME).c_str() );
			outfile << quadPoints[0].x << ' ';
			outfile << quadPoints[0].y << ' ';
			outfile << quadPoints[1].x << ' ';
			outfile << quadPoints[1].y << ' ';
			outfile << quadPoints[2].x << ' ';
			outfile << quadPoints[2].y << ' ';
			outfile << quadPoints[3].x << ' ';
			outfile << quadPoints[3].y << endl;
			outfile << emitterPos.x << ' ';
			outfile << emitterPos.y << endl;
			outfile << collideLimit << endl;
			outfile << (isFullScreen()?1:0) << endl;
			outfile.close();
			cout << "settings saved" << endl;
			break;
	}
}

void BallDroppingsApp::update(){
	
	//kill dead balls
	while(balls.front().dead){
		if(balls.size()>0)balls.pop_front();
		else break;
	}
	
	//read a new frame from cam
	if(cam && cam.checkNewFrame()){
		camSurface = cam.getSurface();
		camPixels = camSurface.getData();
	}
	
	//drop another ball drom emitter
	if(getElapsedFrames()%framesPerBall==0){
		balls.push_back(Ball(emitterPos.x,emitterPos.y));
	}
	
	
	//collide balls
	
	Rand rnd;
	for(deque<Ball>::iterator it = balls.begin();it!=balls.end();it++){
		
		float x = it->x;
		float y = it->y;

		//quad stretch
		float qx[4];
		float qy[4];
		for(int i=0;i<4;i++){
			qx[i] = quadPoints[i].x;
			qy[i] = quadPoints[i].y;
		}
		float ret[2];
		quadstretch(x, y, 0, 0, getWindowWidth(), getWindowHeight(), qx, qy, ret);
		x = ret[0] / ((double)getWindowWidth()/CAMW);
		y = ret[1] / ((double)getWindowHeight()/CAMH);
		
		
		if(camSurface){
		   if(x < CAMW && x >= 0 && y < CAMH && y >= 0){
				if(getCamPixel(x,y)){
					Vec2f edge;
					getPixelEdge(x,y,edge);
					it->bounce(0,0,edge.x,edge.y,ball_sound);
				}
		   }else {
			   it->dead = true;
		   }
		}
	}

	
	//step the ball physics
	for(deque<Ball>::iterator it = balls.begin();it!=balls.end();it++){
		it->step();
	}
	
	//collide edit lines and balls
	for(int i=0;i<balls.size();i++){
		for(int j=0;j<lines.size();j++){
			if(lines[j].checkBallCollide(balls[i])){
				balls[i].bounce(lines[j].p1.x,lines[j].p1.y,
								lines[j].p2.x,lines[j].p2.y,ball_sound);
				
				break;
			}
		}		
	}
	
}

void BallDroppingsApp::draw(){
	
	gl::clear( Color(1,1,1) );
	
	glPushMatrix();
	
	//draw camera
	if(camSurface && drawCam){
		glPushMatrix();
		gl::color( Color(1,1,1) );
		gl::draw( gl::Texture(camSurface) , 
				 Area(0,0,getWindowWidth(),getWindowHeight()) );
		glPopMatrix();
	}


	//threshold picture for limit troubleshooting
	//gl::color( Color(0,1,0) );
	if(showThresh){
		glPushMatrix();
		glScaled((double)getWindowWidth()/CAMW,(double)getWindowHeight()/CAMH,1);
		for(int y=0;y<cam.getHeight();y++){
			glBegin(GL_TRIANGLE_STRIP);
			for(int x=0;x<cam.getWidth();x++){

				if(getCamPixel(x,y)){
					gl::color( Color(0,1,0) );
				}else{
					glColor4f(0,0,0,0);
				}
				glVertex2f(x,y);
				glVertex2f(x,y+1);
					
			}
			glEnd();
		}
		glPopMatrix();
	}

	gl::color( Color(0,0,0) );

	
	//draw emitter
	gl::drawStrokedCircle(emitterPos,7,20);
	glPopMatrix();
	
	//step the ball physics
	for(deque<Ball>::iterator it = balls.begin();it!=balls.end();it++){
		glPushMatrix();
		glTranslated(it->x,it->y,0);
		gl::drawSolidCircle(Vec2f(0,0),it->radius,20);
		glPopMatrix();
	}
	
	//draw lines
	glBegin(GL_LINES);
	for(int i=0;i<lines.size();i++){
		gl::vertex(lines[i].p1);
		gl::vertex(lines[i].p2);
	}
	glEnd();
	
	//draw quad
	if(showQuad){
		glColor3f(1,0,0);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(3,0xCCCCCC);
		glBegin(GL_LINE_LOOP);
		for(int i=0;i<4;i++){
			gl::vertex( quadPoints[i] );
		}
		glEnd();
		glDisable(GL_LINE_STIPPLE);
		
		//draw quad points
		for(int i=0;i<4;i++){
			if(quadPoints[i].distance(mousePos)<5){
				gl::drawStrokedCircle(quadPoints[i],10,20);
			}
		}
		
	}
}



void BallDroppingsApp::mouseDown(MouseEvent event){

	quadPointGrabbed = 0;
	for(int i=0;i<4;i++){
		if(quadPoints[i].distance(mousePos) < 5 && showQuad){
			quadPointGrabbed = i+1;
			break;
		}
	}
	
	
	if(!quadPointGrabbed){
		lines.push_back(EditLine());
		mousePos = lines.back().p1 = lines.back().p2 = event.getPos();
	}
	
	mouseIsDown = true;
}

void BallDroppingsApp::mouseUp(MouseEvent event){
	if(!quadPointGrabbed){
		lines.back().p2 = event.getPos();
	}
	
	quadPointGrabbed = false;
	mouseIsDown = false;
}

CINDER_APP_BASIC( BallDroppingsApp, RendererGl )

