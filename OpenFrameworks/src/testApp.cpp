/**
@author jtnimoy
*/
#include "testApp.h"

//statics
float testApp::friction;
float testApp::frequencyRange;

//--------------------------------------------------------------

void testApp::setup(){	 
	
	//ofEnableSmoothing();
	ofToggleFullscreen();
	
	//defaults
	friction = 0.99997f;
	frequencyRange = 50000;
	fullScreenMode=true;	
	oldMillis=0;
	clickCount=0;
	ball_drop_rate = 3000;
	emptyBalls=0;
	closestLine=0;
	closestLineVertex=0;
	closestLine_maxPickingDistance = 20;
	closestLineDistance = 0;
	mousestate_draggingvert=0;
	undoables=0;
	closestLine_beginMove.copyFrom(0,0,0);
	
	hole.copyFrom(400,100);
	holeLag.copyFrom(400,100);

	undoBusy=false;
	paused = false;
	gravity = 0.01f;
	ballcolor=1;
	oldMouse.copyFrom(-1,-1,-1);

	undoables = new LinkedList();
	balls = new LinkedList();
	lines = new LinkedList();
	emptyBalls = new LinkedList(); //make a new queue for recyclable ball spots.
	
	//load a new ball.
	Ball *b = new Ball(hole, balls->size());
	balls->push((long)b);
	ball_drop_rate = 3000;
	
	font.loadFont("Verdana.ttf",8);
	displayAlpha = 0;
}

//--------------------------------------------------------------
void testApp::update(){

}

//--------------------------------------------------------------

void testApp::draw(){

	if(!paused)step(); //animation and physics are seperated out into a seperate routine

	ofBackground(!ballcolor*255,!ballcolor*255,!ballcolor*255);

	//draw balldropper
	ofSetColor(51,51,51);
	ofCircle( holeLag.x,holeLag.y ,4);


	//draw the mouse line in-progress
	if(clickCount%2==1 && mousestate_draggingvert==0){		
		lineGradient(lastMouseDown.x,lastMouseDown.y,mouseX,mouseY,76.5,0);
	}

	//draw the regular lines
	ofSetColor(ballcolor*255,ballcolor*255,ballcolor*255);
	for(int i=0;i<lines->size();i++){
		EditLine *thisLine = (EditLine*)(lines->get(i));
		ofLine(thisLine->x1, thisLine->y1, thisLine->x2,thisLine->y2 );
	}

	if ( mousestate_draggingvert==0){//am i free roaming, or am i dragging a vertex?

		//draw closest vertex line.
		if(closestLine!=0 && closestLineDistance < closestLine_maxPickingDistance){
			glPushMatrix();
			
			//ok, which point to be drawn?
			if(closestLineVertex==0){
				glTranslatef(closestLine->x1,closestLine->y1,0);
			} else {
				glTranslatef(closestLine->x2,closestLine->y2,0);
			}
			
			//draw a little square
			ofNoFill();
			ofRect(-3,-3,6,6);
			glPopMatrix();
		}
	} else {

		//then draw the vertex as you pull it. 
		glPushMatrix();
		if(closestLineVertex==0){
			glTranslatef(closestLine->x1,closestLine->y1,0);
		} else {
			glTranslatef(closestLine->x2,closestLine->y2,0);
		}
		ofRect(-3,-3,6,6);
		glPopMatrix();
	}

	ofFill();

	//draw all the balls
	for(int i=0;i<balls->size();i++){
		if(balls->get(i)!=0){
			Ball *b = (Ball*)(balls->get(i));
			ofCircle(b->x,b->y, b->jitter*5.0f+2);
		}
	}
	
	//draw the text info display
	if(displayAlpha>0){
		ofSetColor(ballcolor*255,ballcolor*255,ballcolor*255,  displayAlpha );
		char displayString[512];
		sprintf(displayString,"ball drop rate: %i [- +] \nfriction: %f [f F]\ngravity: %f [g G]\npitch range: %f [r R]\n\n\
shift-space clears balls\nctrl-space clears lines\nspace alone clears all\nctrl-z to undo line editing\nctrl-f toggles fullscreen\n\
w inverts video\nb moves balldropper to mouse\ns saves world state to disk\no loads world state from disk\ne,del deletes point under mouse\np pause",
		ball_drop_rate, friction, gravity, frequencyRange);
		
		font.drawString(displayString,20,20);
	}
}

//--------------------------------------------------------------

void testApp::step(){

	int i;//for loop variable to be reused

	holeLag.lerpSelfTo(hole,0.1);


	//release a ball on a timer
	if(glutGet(GLUT_ELAPSED_TIME)-oldMillis > ball_drop_rate ){
		createBall();
		oldMillis = glutGet(GLUT_ELAPSED_TIME);
	}

	updateClosestPickingLine();
	//for all the balls . . .
	for(i=0;i<balls->size();i++){
		if(balls->get(i)!=0){
	
			Ball *b = (Ball*)(balls->get(i));
			if(b->y > 1280 || b->force.getLength()==0){
				balls->set(i,0);	
				emptyBalls->push(i);
				delete b;
			} else {
				b->applyForce(0,gravity);//gravity
				
				//check collisions with lines
				//and bounce accordingly
				
				for(int j=0;j< lines->size() ;j++){
					//am i on one side when i was just on another side a second ago?
					EditLine *thisLine = (EditLine*)(lines->get(j));
					if(thisLine->checkBallCollide(b)){
						//then also reset my memory to give me 1 frame's worth of amnesia.
						b->amnesia();
						b->bounce( thisLine->x1,thisLine->y1, thisLine->x2,thisLine->y2);
						break;//skip the rest of the lines
					}
				}

				b->stepPhysics();
			}
		}
	}
	
	if(displayAlpha>0)displayAlpha--;
	
}


//--------------------------------------------------------------

void testApp::createBall(){

	//load a new ball.
	Ball *b = new Ball(hole, balls->size());
	b->applyForce(0.0001,0);//give it an initial push

	//search for an empty spot in the list
	if(emptyBalls->size()>0){
		balls->set( emptyBalls->unshift(),(long)b);
	} else {//else, you have to make a new one.
		balls->push((long)b);
	}
	
}

//--------------------------------------------------------------

void testApp::deletePickedVertex(){  
	if( closestLineDistance < closestLine_maxPickingDistance){
		//register undoable
		LinkedList *v=new LinkedList();
		v->push((int)(closestLine->x1));
		v->push((int)(closestLine->y1));
		v->push((int)(closestLine->x2));
		v->push((int)(closestLine->y2));
		v->push((long)"delete_line");
		undoables->push((long)v);

		//then one of them is highlighted.
		lines->deleteMatch((long)closestLine);
		delete closestLine;
		closestLine = 0;
		closestLineDistance=100000;//turn off picking!
	}
}

//--------------------------------------------------------------

void testApp::undo(){
	if(undoBusy){
		return;
	}else{
		undoBusy=true;
		if(undoables->size()>0){

			//get the most recent undoable action.
			LinkedList *thisUndoable = (LinkedList*)undoables->pop();
			char *action = (char*)thisUndoable->pop();
			//get its variables and do the action.

			if(0==strcmp(action,"create_line")){
				//kill the line
				EditLine *l = (EditLine*)thisUndoable->pop();
				if(validLine(l)){
					lines->deleteMatch((long)l);
					delete l;
				}

			} else if(0==strcmp(action,"move_line")){
				//move the line back.
				EditLine *l = (EditLine*)thisUndoable->pop();
				if(validLine(l)){
					int y = (int)thisUndoable->pop();
					int x = (int)thisUndoable->pop();
					int which = thisUndoable->pop();
					if(which==0){
						l->set1(x,y);
					} else {
						l->set2(x,y);
					}
				}

			} else if(0==strcmp(action,"delete_line")){

				int y2 = (int)thisUndoable->pop();
				int x2 = (int)thisUndoable->pop();
				int y1 = (int)thisUndoable->pop();
				int x1 = (int)thisUndoable->pop();

				EditLine *l = new EditLine(x1,y1,x2,y2);
				lines->push((long)l);

			} else { 
				printf("Undoable action unknown: %s\n",action);
			}

			delete thisUndoable;
		}
		undoBusy = false;
	}

}

//--------------------------------------------------------------

int  testApp::validLine(EditLine*l){
	int foundOne = 0;
	for(int i=0;i<lines->size();i++){
		if(lines->get(i)==((long)l)){
			foundOne=1;
			break;
		}
	}
	return foundOne;
}


//--------------------------------------------------------------

void testApp::saveToFile(char *filename){

	//initiate the file
	FILE *theFile = fopen(filename,"wb");

	//first list the global properties
	fprintf(theFile,"%f fullscreenMode ", (float)fullScreenMode);
	fprintf(theFile,"%f paused ",         (float)paused);
	fprintf(theFile,"%f ball_drop_rate ", (float)ball_drop_rate);  
	fprintf(theFile,"%f gravity ",        (float)gravity);
	fprintf(theFile,"%f frequencyRange ", (float)frequencyRange);
	fprintf(theFile,"%f friction ",       (float)friction);
	fprintf(theFile,"%f newball_x ",       (float)hole.x);
	fprintf(theFile,"%f newball_y ",       (float)hole.y);

	//list the balls.
	int i;
	for( i=0;i<balls->size();i++){
		Ball *thisBall = (Ball*)balls->get(i);
		if(thisBall==0){
		} else {
			fprintf(theFile,"%f x ",      (float)thisBall->x);
			fprintf(theFile,"%f y ",      (float)thisBall->y);
			fprintf(theFile,"%f oldX ",   (float)thisBall->oldPos.x);
			fprintf(theFile,"%f oldY ",   (float)thisBall->oldPos.y);
			fprintf(theFile,"%f forceX ", (float)thisBall->force.x);
			fprintf(theFile,"%f forceY ", (float)thisBall->force.y);
			fprintf(theFile,"%f jitter ", (float)thisBall->jitter);
			fprintf(theFile,"0.0 ball ", 0);
		}
	}

	//list the lines.
	for(i=0;i<lines->size();i++){
		EditLine *thisLine = (EditLine*)lines->get(i);
		fprintf(theFile,"%f x ",  (float)thisLine->x1);
		fprintf(theFile,"%f y ",  (float)thisLine->y1);
		fprintf(theFile,"%f x2 ", (float)thisLine->x2);
		fprintf(theFile,"%f y2 ", (float)thisLine->y2);

		fprintf(theFile,"0.0 line ", 0);
	}

	//close the file handle
	fclose(theFile);
  
}

//-----------------------------------------------------------------

void testApp::loadFile(char *filename){
	unsigned char *fileString = FileUtils::stringFromFile(filename);
	float newfloat;
	char accum[256];
	int accumlen = 0;
	int state = 1;//zero means waiting for token. 1 means float.
	float waitingFloat;

	//some arg buffers for the functions.
	float x_;
	float y_;
	float oldX_;
	float oldY_;
	float x2_;
	float y2_;
	float forceX_;
	float forceY_;
	float jitter_;

	//recievers of the waiting floats

	if(fileString!=0){

		resetWorld();
		//first erase the current scene.
		for(int i=0;i<strlen((const char*)fileString);i++){
			unsigned char thisChar = fileString[i];
			accum[accumlen]=0;
			//printf("%c %i %f \"%s\" %f\n",thisChar,i,waitingFloat,accum,atof(accum));
			if(thisChar==' '){//do something to the accum.
				accum[accumlen]=0;
				if(state==0){
					if(strcmp(accum,"x")==0){
						x_ = waitingFloat;
					} else if(strcmp(accum,"y")==0){
						y_ = waitingFloat;
					} else if(strcmp(accum,"x2")==0){
						x2_ = waitingFloat;
					} else if(strcmp(accum,"y2")==0){
						y2_ = waitingFloat;
					} else if(strcmp(accum,"oldX")==0){
						oldX_ = waitingFloat;
					} else if(strcmp(accum,"oldY")==0){
						oldY_ = waitingFloat;
					} else if(strcmp(accum,"forceX")==0){
						forceX_ = waitingFloat;
					} else if(strcmp(accum,"forceY")==0){
						forceY_ = waitingFloat;
					} else if(strcmp(accum,"jitter")==0){
						jitter_ = waitingFloat;
					} else if(strcmp(accum,"ball")==0){
						Ball *b = new Ball(V3(x_,y_,0),oldX_,oldY_,forceX_,forceY_,jitter_);
						balls->push((long)b);
					} else if(strcmp(accum,"line")==0){
						EditLine *l = new EditLine(x_,y_,x2_,y2_);	    
						lines->push((long)l);
					} else if(strcmp(accum,"fullscreenMode")==0){
						//do nothing because this is contraversial.
					} else if(strcmp(accum,"paused")==0){
						paused = (int)waitingFloat;
					} else if(strcmp(accum,"newball_x")==0){
						hole.x  = (int)waitingFloat;
						holeLag.x  = (int)waitingFloat;
					} else if(strcmp(accum,"newball_y")==0){
						hole.y  = (int)waitingFloat;
						holeLag.y  = (int)waitingFloat;
					} else if(strcmp(accum,"ball_drop_rate")==0){
						ball_drop_rate = (long)(waitingFloat);
					} else if(strcmp(accum,"gravity")==0){
						gravity = waitingFloat;
					} else if(strcmp(accum,"frequencyRange")==0){
						frequencyRange = waitingFloat;
					} else if(strcmp(accum,"friction")==0){
						friction = waitingFloat;
					} else {
						printf("unrecognized file command: %s\n",accum);
					}
				} else {//else we are waiting for a float
					waitingFloat = (float)atof(accum);
				}
				state = !state;//reverse the state.
				accumlen = 0;//done with it. reset it.
			} else {
				accum[accumlen] = thisChar;
				accumlen++;
			}
		}
	} else {
		printf("Problem opening file \"%s\".\n",filename);
	}

	delete [] fileString;
}

//--------------------------------------------------------------

void testApp::resetWorld(){
  resetLines();
  resetBalls();
  resetVars();
}

//--------------------------------------------------------------

void testApp::resetVars(){
	ball_drop_rate = 3000;
	friction = 0.99997f;
	gravity = 0.01f;
	frequencyRange = 50000;
	hole.copyFrom(400,100,0);
	holeLag.copyFrom(400,100,0);
}

//--------------------------------------------------------------

void testApp::resetBalls(){
	for(int i=0;i<balls->size();i++){
		Ball*thisB = (Ball*)(balls->get(i));
		delete thisB;
	}
	delete balls;
	balls = new LinkedList();
}

//--------------------------------------------------------------

void testApp::resetLines(){
	int i;
	for(i=0;i<lines->size();i++){
		EditLine*thisL = (EditLine*)(lines->get(i));
		delete thisL;
	}
	delete lines;
	lines = new LinkedList();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){ 
	if(key==26){//undo
		if(undoables->size()>0){
			undo();
		}
	}else if(key==6){ //toggle full screen
		ofToggleFullscreen();
	}else if(key==0){ //ctrl+space on the mac
		for(int i=0;i<lines->size();i++){
			EditLine*thisL = (EditLine*)(lines->get(i));
			delete thisL;
		}
		delete lines;
		lines = new LinkedList();		
	} else if (key=='w') { // toggle callcolor
		ballcolor = !ballcolor;
	} else if (key=='t'){
	
		// this used to be the network server enable
		// when network was supported
		/*
			FSOUND_Close();
			if(!server_valid()){
			server_init(6668);
	}
		*/
	} else if (key=='B'||key=='b'){
		hole.copyFrom(mouseX,mouseY,0);
	} else if (key=='T'){
		//server_close();
		//initfmod();
	} else if (key=='s'||key=='S'){
		saveToFile("default.state");
	} else if (key=='o'||key=='O'){
		loadFile("default.state");
	} else if(key=='r'){
		frequencyRange /= 1.09;
		showPanel();
	} else if(key=='R'){
		frequencyRange *= 1.09;
		showPanel();
	} else if(key=='f'){
		friction+=0.0001f;
		showPanel();
	} else if(key=='F'){
		friction-=0.0001f;
		showPanel();
	} else if(key=='g'){
		gravity-=0.001;
		showPanel();
	} else if(key=='G'){
		gravity+=0.001;
		showPanel();
	} else if(key=='e'||key=='E'){
		deletePickedVertex();
	} else if(key=='p'||key=='P'){
		paused = !paused;
	}else if(key=='0'){
		resetVars();
	}else if(key == 45 || key == 95){//-
		ball_drop_rate -=100;
		showPanel();
	}else if(key == 61 || key == 43){//+
		ball_drop_rate += 100;
		showPanel();
	}else if(key==' '){
		if( glutGetModifiers()==GLUT_ACTIVE_CTRL){
	resetLines();
		} else if( glutGetModifiers()==GLUT_ACTIVE_SHIFT ){
	resetBalls();
		}else { // else kill both the lines and the balls.
	resetBalls();
	resetLines();
	  }      
	}else if (key==127 || key==8){
      deletePickedVertex(); // delete last vertex
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){ 
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
 
	if(closestLineDistance < closestLine_maxPickingDistance){
		mousestate_draggingvert=1;

		//taking some notes for the undoable later on.
		if(closestLineVertex==0) closestLine_beginMove.copyFrom( closestLine->x1 , closestLine->y1 , 0);
		else closestLine_beginMove.copyFrom( closestLine->x2 , closestLine->y2 , 0);
		
	} else { 
		clickCount++;  
		if(clickCount%2==0){ //only draw something every 2 clicks.

			//draw with mouse
			if(oldMouse.x!=-1 && oldMouse.y!=-1){ //but only if the old mouse value is useful
				//load a new line
				EditLine *el = new EditLine(oldMouse.x,oldMouse.y,mouseX,mouseY);

				//construct and register an undoable
				LinkedList *v=new LinkedList();
				v->push((long)el);
				v->push((long)"create_line");
				undoables->push((long)v);
				lines->push((long)el);
			}
		}
	}

	oldMouse.copyFrom(mouseX,mouseY,0);
	lastMouseDown.copyFrom(mouseX,mouseY,0);
	
}

//--------------------------------------------------------------

void testApp::mouseReleased(){

	float xd = mouseX - lastMouseDown.x;
	float yd = mouseY - lastMouseDown.y;

	if (  mousestate_draggingvert==1){//then we had been dragging something else.
		
		mousestate_draggingvert=0;
		clickCount = 0;
		//register undoable
		LinkedList *v=new LinkedList();
		v->push(closestLineVertex);
		v->push((int)(closestLine_beginMove.x));
		v->push((int)(closestLine_beginMove.y));
		v->push((long)closestLine);
		v->push((long)"move_line");
		undoables->push((long)v);

	} else {
		if ( sqrt(xd*xd+yd*yd) > 10 ){//10 is the mouse drag movement margin for nondraggers
			mousePressed(mouseX,mouseY,1);//nudge the new line drawing.
		}
	}
}

//--------------------------------------------------------------
/**
	by setting a color channel value to 512, and having it count down
	slowly over time, we get a period of solid opacity
*/
void testApp::showPanel(){
		displayAlpha = 512;
}
//--------------------------------------------------------------
/**
	a line drawing routine that uses different colors for the vertices
*/
void testApp::lineGradient(float x1,float y1,float x2,float y2,float color1,float color2){
	glBegin(GL_LINES);
	ofSetColor(color1,color1,color1);
	glVertex3f(x1,y1,0);
	ofSetColor(color2,color2,color2);
	glVertex3f(x2,y2,0);
	glEnd();
}
//--------------------------------------------------------------

/**
	takes care of mouse interaction with the lines
*/

void testApp::updateClosestPickingLine(){
	
	/*
		find closest picking vertex
		by sorting through them and measuring
		distance from mouse
	*/
	EditLine *closeL=0;
	int closeLV=0;
	float closeDist=100000;//very far to start.
	for(int i=0;i<lines->size();i++){

		EditLine *thisLine = (EditLine*)(lines->get(i));

		//recalculating the closest line for both vertices only if we are not dragging one.
		if ( mousestate_draggingvert==0){
			//v1			
			//measuring distance from mouse
			float xd = thisLine->x1 - mouseX;
			float yd = thisLine->y1 - mouseY;
			float dist = sqrt(xd*xd+yd*yd);
			//have we got a winner?
			if ( dist  < closeDist){
				closeDist = dist;
				closeL = thisLine;
				closeLV = 0;
			}

			//v2
			//measuring distance from mouse
			xd = thisLine->x2 - mouseX;
			yd = thisLine->y2 - mouseY;
			dist = sqrt(xd*xd+yd*yd);
			//have we got a winner?
			if ( dist  < closeDist){
				closeDist = dist;
				closeL = thisLine;
				closeLV = 1;

			}
		}
	}

	/*
		then update interface variables accordingly
	*/

	if ( mousestate_draggingvert==0){//am i free roaming, or am i dragging a vertex?
		//commit local calculations  globally.
		closestLine = closeL;
		closestLineVertex = closeLV;
		closestLineDistance = closeDist;
	} else {
		//set vertex to mouse position.
		if(closestLineVertex==0){//which side of the line?
			closestLine->set1(mouseX,mouseY);
		} else {
			closestLine->set2(mouseX,mouseY);	
		}
		//fix just in case
		if(closestLine->fixDirection()){
			//also adjust the line-siding if it got swapped.
			closestLineVertex=!closestLineVertex;
		}
	}


}
