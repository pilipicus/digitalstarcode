#ifndef __OFX_VIDEO_GRABBER__SDK_H
#define __OFX_VIDEO_GRABBER__SDK_H

#include "ofxVideoGrabberFeature.h"

class ofxVideoGrabberSDK
{
	public:

        ofxVideoGrabberSDK() {bHasNewFrame = false;};
		virtual ~ofxVideoGrabberSDK() {cout << "ofxVideoGrabberSDK destructor" << endl;};
		
		virtual bool init( int _width = 320, int _height = 240, int _format = VID_FORMAT_RGB, int _targetFormat = VID_FORMAT_RGB , int _frameRate = 15 ) = 0;
		virtual void close() = 0;
		virtual unsigned char* getPixels() = 0;
		virtual bool grabFrame(unsigned char ** _pixels) = 0;

        /* camera settings */
		virtual void setFeaturesOnePushMode() = 0;
		virtual void getAllFeatureValues() = 0;
		virtual void getFeatureValues( ofxVideoGrabberFeature* _feature ) = 0;
		virtual void setFeatureMode( int _val, int _feature ) = 0;
		virtual bool setFeatureOnOff( bool _val, int _feature ) = 0;
		virtual void setFeatureAbsoluteValue( float _val, int _feature ) = 0;
		virtual void setFeatureValue( float _val, int _feature ) = 0;
        virtual void setFeatureValue( float _val1, float _val2, int _feature) = 0;

		unsigned int width;
		unsigned int height;
        unsigned int bpp;

        int availableFeatureAmount;
		ofxVideoGrabberFeature* featureVals;

    protected:
        bool bHasNewFrame;
		unsigned char* pixels;
		int sourceFormat;
		int targetFormat;

};

#endif
