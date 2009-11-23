#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxSoundPlayer.h"

// need this for the effects only
#include "Effect.h"

using namespace ofxOpenALPlayer;

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

        ofxSoundPlayer sound;
        ofxSoundPlayer gunshot;
        ofxSoundPlayer drums;
        ofxSoundPlayer music;
        #ifdef USE_OPENAL
        Effect fx1;
        Effect fx2;
        #endif

};

#endif
