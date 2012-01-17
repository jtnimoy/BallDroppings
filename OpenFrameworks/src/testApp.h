#ifndef _TEST_APP
#define _TEST_APP

/**

BallDroppings
by Josh Nimoy, 2003

*/

#include "ofMain.h"
#include "EditLine.h"
#include "Ball.h"
#include "LinkedList.h"
#include "FileUtils.h"

using namespace std;

class testApp : public ofSimpleApp{
	
	public:
	
		//openframeworks subclassed methods
		
		void setup();
		void update();
		void draw();		
		void step();		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased();
		
		//custom methods
		
		void createBall(); ///< 	create a new ball at the emmitter's position 
		void deletePickedVertex(); ///< delete the vertex currently under the mouse along with the line attached to it
		void undo(); ///< undo the last user interface move
		int validLine(EditLine*l); ///< checks whether a line is still part of the system
		void saveToFile(char *filename); ///< saves balldroppings world/game state to a file
		void loadFile(char *filename); ///< load a balldroppings state and parse it into the system		

		//resetting methods
		
		void resetWorld(); ///< 	reset the entire app
		void resetVars();///< reset global state variables
		void resetBalls(); ///< clear only the balls
		void resetLines(); ///< clear only the lines
		void showPanel();//sets the info display panel to visible
		float oldMillis; ///< remembering what time it was
		long clickCount; ///< number of times the user has pressed the mouse button
		long ball_drop_rate; ///< how fast or slow to release balls
		bool paused; ///< freezing the app like pausing the game
		float gravity; ///< strength of gravity pulling towards bottom of screen
		float ballcolor; ///< a gray tone for the color of the ball (0 is black, 1 is white)
		static float friction; ///< friction multiplier to keep the physics realistic
		static float frequencyRange; ///< pitch range for ball song
		bool fullScreenMode; ///< whether or not the app is fullscreen

		LinkedList *balls; ///< the list of balls
		LinkedList *lines; ///< the list of lines
		LinkedList *emptyBalls; ///< registry for offstage balls waiting to be recycled

		//undo stuff
		
		LinkedList *undoables; ///< list of undoable actions
		int undoBusy; ///< true if the undo process is still busy undo
		
		V3 oldMouse; ///< mouse position one frame ago
		V3 lastMouseDown; ///< where the mouse was last time the button was pressed
		
		//interface for working with the line closest to the mouse
		EditLine *closestLine;
		int closestLineVertex;
		float closestLine_maxPickingDistance;
		float closestLineDistance;
		V3 closestLine_beginMove;
		int mousestate_draggingvert;

		//positioning for the ball emitter circle
		V3 hole;
		V3 holeLag;
		
		ofTrueTypeFont font; ///< typeface for the info display
		float displayAlpha;///< opacity of the info display
		void lineGradient(float x1,float y1,float x2,float y2,float color1,float color2);//2 color line drawing
		void updateClosestPickingLine();///< recalculates the line closest to the mouse
};

#endif

