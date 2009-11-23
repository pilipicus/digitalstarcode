#pragma once

#include "ofxSoundPlayer.h"

extern "C" {
#include "fmod.h"
#include "fmod_errors.h"
}



class ofxSoundPlayerFMOD : public ofxSoundPlayerEngine
{

public:

	ofxSoundPlayerFMOD() {};
	virtual ~ofxSoundPlayerFMOD() {};

// ----------------------------------------------------------------------------
// Player functions from ofSoundPlayerEngine:
// ----------------------------------------------------------------------------
	bool    loadSound(string fileName, bool stream = false);
	void    unloadSound();
	void    play();
	void    stop();

	void    setVolume(float vol);
	void    setPan(float vol);
	void    setSpeed(float spd);
	void    setPaused(bool bP);
	void    setLoop(bool bLp);
	void    setMultiPlay(bool bMp);
	void    setPosition(float pct); // 0 = start, 1 = end;

	float   getPosition();
	bool    getIsPlaying();
	float   getSpeed();
	float   getPan();
	bool    getLoop();


// ----------------------------------------------------------------------------
// FMOD implementation specific:
// ----------------------------------------------------------------------------

    void assignEffect(unsigned int uiEffectSlot, unsigned int uiSlotID); // not yet implemented
    void removeEffect(unsigned int slotID);  // not yet implemented

	FMOD_RESULT result;
	FMOD_CHANNEL * channel;
	FMOD_SOUND * sound;
};

