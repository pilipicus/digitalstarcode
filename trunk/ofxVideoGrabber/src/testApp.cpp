#include "testApp.h"





//--------------------------------------------------------------
void testApp::setup(){

	camWidth 		= 320;	// try to grab at this size.
	camHeight 		= 240;
	appWidth        = ofGetWidth();
	appHeight       = ofGetHeight();

	//vidGrabber.setVerbose(true);
	bool result = vidGrabber.initGrabber( camWidth, camHeight, VID_FORMAT_YUV411, VID_FORMAT_RGB, 30 );
	//bool result = vidGrabber.initGrabber( camWidth, camHeight, VID_FORMAT_GREYSCALE, VID_FORMAT_GREYSCALE, 30, new Libdc1394Grabber);
	//bool result = vidGrabber.initGrabber( camWidth, camHeight, VID_FORMAT_RGB, VID_FORMAT_RGB, 15 );
    if(result) {
//        vidGrabber.setFeatureMode(FEATURE_BRIGHTNESS,FEATURE_MODE_MANUAL);
//        vidGrabber.setFeatureVal(269,FEATURE_BRIGHTNESS);
//
//        vidGrabber.setFeatureMode(FEATURE_EXPOSURE,FEATURE_MODE_MANUAL);
//        vidGrabber.setFeatureVal(511,FEATURE_EXPOSURE);
//
//        vidGrabber.setFeatureMode(FEATURE_SHARPNESS,FEATURE_MODE_MANUAL);
//        vidGrabber.setFeatureVal(49,FEATURE_SHARPNESS);
//
//        vidGrabber.setFeatureMode(FEATURE_WHITE_BALANCE,FEATURE_MODE_MANUAL);
//        vidGrabber.setFeatureVal(91,75,FEATURE_WHITE_BALANCE);
//
//        vidGrabber.setFeatureMode(FEATURE_SATURATION,FEATURE_MODE_MANUAL);
//        vidGrabber.setFeatureVal(113,FEATURE_SATURATION);
//
//        vidGrabber.setFeatureMode(FEATURE_GAMMA,FEATURE_MODE_MANUAL);
//        vidGrabber.setFeatureVal(1,FEATURE_GAMMA);
//
//        vidGrabber.setFeatureMode(FEATURE_SHUTTER,FEATURE_MODE_MANUAL);
//        vidGrabber.setFeatureVal(6,FEATURE_SHUTTER);
//
//        vidGrabber.setFeatureMode(FEATURE_GAIN,FEATURE_MODE_MANUAL);
//        vidGrabber.setFeatureVal(239,FEATURE_GAIN);
    }

}


//--------------------------------------------------------------
void testApp::update(){

	ofBackground(100,100,100);

	vidGrabber.update();
	//vidGrabber2.grabFrame();
	//vidGrabber3.grabFrame();

	if (vidGrabber.isFrameNew()){

        /* Framerate display */
        mytimeNow = ofGetElapsedTimef();
		if( (mytimeNow-mytimeThen) > 0.05f || myframes == 0 ) {
			myfps = (double) myframes / (mytimeNow-mytimeThen);
			mytimeThen = mytimeNow;
			myframes = 0;
			myframeRate = 0.9f * myframeRate + 0.1f * myfps;
			sprintf(buf2,"Capture framerate : %f",myframeRate);
		}
		myframes++;
	}

    sprintf(buf,"App framerate : %f",ofGetFrameRate());

    //vidGrabber.doneWithCurrentFrame();
    //vidGrabber2.doneWithCurrentFrame();
    //vidGrabber3.doneWithCurrentFrame();
}

//--------------------------------------------------------------
void testApp::draw(){
	ofSetColor(0xffffff);
	vidGrabber.draw(appWidth - camWidth,0);
	//vidGrabber2.draw(camWidth,0);
	//vidGrabber3.draw(0,camHeight);


    /* Framerate display */
	ofDrawBitmapString(buf,10,appHeight - 40);
	ofDrawBitmapString(buf2,10,appHeight - 20);
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){

	// in fullscreen mode, on a pc at least, the
	// first time video settings the come up
	// they come up *under* the fullscreen window
	// use alt-tab to navigate to the settings
	// window. we are working on a fix for this...

	if (key == 's' || key == 'S'){
		vidGrabber.videoSettings();
	}

	if (key == 27) {
        vidGrabber.close();
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

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}
