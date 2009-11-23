#include "testApp.h"


void testApp::exit()
{
    #ifdef USE_OPENAL
    // need these here for the moment, still experimental
   sound.removeEffect(fx1.getEffectID());
   #ifdef TARGET_LINUX
   sound.removeEffect(fx2.getEffectID());
   #endif
   #endif
}

//--------------------------------------------------------------
void testApp::setup(){

    ofSetFrameRate(30);

    //load mono sound
    sound.loadSound("fx1.wav");
    sound.setLoop(true);
    sound.setPan(0);
    sound.setVolume(0.4);


    // load stereo sound
    drums.loadSound("drumloop.wav");
    drums.setLoop(true);
    drums.setPan(0);
    drums.setVolume(1.0f);

    // load stream
    music.loadSound("bells.ogg",true);
    music.setLoop(true);

    // load mono sound, set multi-play
    gunshot.loadSound("gunshot.wav");
    gunshot.setMultiPlay(true);
    gunshot.setVolume(0.5);

#ifdef USE_OPENAL
    /* warning, experimental! only works on mono samples (sounds), no streams */
    fx1.addEffect(AL_EFFECT_REVERB);
    fx1.setEffectParameter(AL_REVERB_DECAY_TIME, 10.0f);
    fx1.setEffectGain(1.0f);
    sound.assignEffect(fx1.getEffect(),fx1.getEffectID());
#ifdef TARGET_LINUX
    //Echo only works on Linux due to OpenAL Soft.
    fx2.addEffect(AL_EFFECT_ECHO);
    fx2.setEffectGain(1.0f);
    sound.assignEffect(fx2.getEffect(),fx2.getEffectID());
#endif
#endif

    sound.play();
}

//--------------------------------------------------------------
void testApp::update(){

}

//--------------------------------------------------------------
void testApp::draw(){

    ofSetColor(255,255,255);
    ofDrawBitmapString("PRESS KEY: [1] Synth sample loop   [2] Stereo drum loop    [3] Music stream ",20,20);
    ofDrawBitmapString("[spacebar] pause    [l] toggle looping    [s] stop all sounds ",20,40);
    ofDrawBitmapString("Click mouse to trigger multi-play gunshots, move mouse to change pan/pitch ",20,60);
    ofSetColor(0xe84573);
    ofCircle(mouseX,mouseY,30);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    static bool loopToggle = true;
    static bool pauseToggle = false;

    if(key == 32) {
        pauseToggle = !pauseToggle;
        sound.setPaused(pauseToggle);
        drums.setPaused(pauseToggle);
        music.setPaused(pauseToggle);
    }
    if(key == '1')
    {
        sound.play();
        drums.stop();
        music.stop();
    }
    if(key == '2')
    {
        sound.stop();
        drums.play();
        music.stop();
    }
    if(key == '3')
    {
        sound.stop();
        drums.stop();
        music.play();
    }
    if(key == 'l') {
        loopToggle = !loopToggle;
        sound.setLoop(loopToggle);
        drums.setLoop(loopToggle);
        music.setLoop(loopToggle);
    }
    if(key == 's') {
        ofxSoundStopAll();
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    float dx = (float) 2*(x / (float) ofGetWidth());
    sound.setSpeed(dx);
    drums.setPan(dx - 1.0f);
    music.setSpeed(dx);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    gunshot.play();
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

