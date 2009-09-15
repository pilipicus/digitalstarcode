#include "Libdc1394Grabber.h"
#include "Libdc1394GrabberUtils.h"
#include "Libdc1394GrabberFramerateHelper.h"
#include "Libdc1394GrabberVideoFormatHelper.h"


int Libdc1394Grabber::g_cameraIndex = -1;
dc1394_t* Libdc1394Grabber::d = NULL;

Libdc1394Grabber::Libdc1394Grabber()
{

	conversionNeeded = false;
	grabbedFirstImage = false;

	YUV_BYTE_ORDER = DC1394_BYTE_ORDER_UYVY; //DC1394_BYTE_ORDER_UYVY, DC1394_BYTE_ORDER_YUYV
	ISO_SPEED = DC1394_ISO_SPEED_400;

	bayerMethod = DC1394_BAYER_METHOD_BILINEAR;
	bayerPattern = DC1394_COLOR_FILTER_RGGB;

	availableFeatureAmount = 0;

	camera = NULL;
	pixels = NULL;
}

Libdc1394Grabber::~Libdc1394Grabber()
{

	if (camera != NULL )
	{
		fprintf(stderr,"stop transmission\n");
		// Stop data transmission
		if (dc1394_video_set_transmission(camera,DC1394_OFF)!=DC1394_SUCCESS)
		{
			printf("couldn't stop the camera?\n");
		}
		// Close camera
		cleanupCamera();
	}

}


void Libdc1394Grabber::close()
{

	if (camera != NULL )
	{
		// Close camera
		cleanupCamera();
	}
}

bool Libdc1394Grabber::init( int _width, int _height, int _format, int _targetFormat, int _frameRate )
{

    cout << "initialising:"  << endl;
    cout << "width: " << _width << " height: " << _height << endl;
    cout <<" format " << videoFormatToString(_format) << "  targetFormat: " << videoFormatToString(_targetFormat) << endl;

	targetFormat = _targetFormat;

	dc1394framerate_t  desiredFrameRate = Libdc1394GrabberFramerateHelper::numToDcLibFramerate( _frameRate );
	dc1394video_mode_t desiredVideoMode	= Libdc1394GrabberVideoFormatHelper::videoFormatFromParams( _width, _height, _format );

	bool result = initCam( desiredVideoMode, desiredFrameRate );
	if(!result) return false;

    initInternalBuffers();
	startThread(false, true);   // blocking, verbose

	return true;
}

bool Libdc1394Grabber::initCam( dc1394video_mode_t _videoMode, dc1394framerate_t _frameRate )
{

    dc1394error_t err;
	dc1394camera_t **cameras = NULL;
	uint32_t numCameras, i;
	dc1394camera_list_t * list;

    /* Initialise libdc1394 */
    if(!d) {
        d = dc1394_new ();
    }
    if (!d) {
        ofLog(OF_LOG_FATAL_ERROR,"Failed to initialise libdc1394.");
        return false;
    }

    /* Find cameras */
	err=dc1394_camera_enumerate (d, &list);
    DC1394_ERR_RTN(err,"Failed to enumerate cameras");

    /* Verify that we have at least one camera */
    if (list->num == 0) {
		dc1394_log_error("No cameras found!");
		ofLog(OF_LOG_FATAL_ERROR, "No cameras found!");
        return false;
    }

	numCameras = list->num;

/*
	if (err!=DC1394_SUCCESS && err != DC1394_NO_CAMERA)
	{
		fprintf( stderr, "Unable to look for cameras\n\n"  "On Linux, please check \n" "  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' are loaded \n" "  - if you have read/write access to /dev/raw1394\n\n");
		//exit(1);
	}
*/

	ofLog(OF_LOG_NOTICE,"There were %d cameras found attached to your PC\n", numCameras );

    /* Use first camera */
	camera = dc1394_camera_new (d, list->ids[++g_cameraIndex].guid);
    if (!camera) {
        dc1394_log_error("Failed to initialize camera with guid %llx", list->ids[0].guid);
        return false;
    }
    dc1394_camera_free_list (list);
	free(cameras);

	ofLog(OF_LOG_NOTICE, "Using Camera with GUID %u",camera->guid);

	/* Get video modes */
	if (dc1394_video_get_supported_modes(camera,&video_modes) != DC1394_SUCCESS)
	{
	    ofLog(OF_LOG_FATAL_ERROR, "Can't get video modes");
	    cleanupCamera();
	    return false;
    }

	ofLog(OF_LOG_NOTICE, "Listing Modes");
	for(i = 0; i < video_modes.num; i++ )
	{
	    dc1394video_mode_t mode = video_modes.modes[i];
	    Libdc1394GrabberUtils::print_mode_info( camera , mode );
    }

	/* Search the list for our preferred video mode */
	bool foundVideoMode = false;
  	for (i=video_modes.num-1;i>=0;i--)
	{
		if( video_modes.modes[i] == _videoMode )
		{
		    foundVideoMode = true;
            break;
		}
	}

	video_mode = _videoMode;
	cout << "Video Mode = " << video_mode << endl;

	dc1394_get_color_coding_from_video_mode(camera, video_mode, &coding);
	unsigned int source_bpp;
	dc1394_get_color_coding_bit_size(coding,&source_bpp);
	cout << "Source bpp = " << source_bpp  << endl;

	//if ((dc1394_is_video_mode_scalable(video_modes.modes[i]))|| (coding!=DC1394_COLOR_CODING_MONO8))
	//{ fprintf(stderr,"Could not get a valid MONO8 mode\n"); cleanupCamera(); }

	cout << endl << "**** Chosen Color coding ****" << endl;
	Libdc1394GrabberUtils::print_color_coding( coding );
	cout << endl << "*****************************" << endl << endl;

	sourceFormatLibDC = coding;
	sourceFormat = Libdc1394GrabberVideoFormatHelper::libcd1394ColorFormatToVidFormat(  coding );


    cout << endl << "**** get color codings:" << endl;
    dc1394color_codings_t color_modes;
    err = dc1394_format7_get_color_codings(camera, video_mode, &color_modes);

    if(err == DC1394_SUCCESS) {
        for (i=color_modes.num-1;i>=0;i--)
        {
            cout << i << " -> " << color_modes.codings[i] << endl;
        }
    } else {
        cout << "could not get color codings..." << endl;

    }

	cout << "*******" << endl;

	// get framerates
	if (dc1394_video_get_supported_framerates(camera,video_mode,&framerates)!=DC1394_SUCCESS)
	{
		fprintf(stderr,"Can't get framerates\n");
		cleanupCamera();
	}

	// search the list for our preferred framerate
	bool foundFramerate = false;
	for( i = 0; i < framerates.num; i++ )
	{
		if( framerates.framerates[i] == _frameRate )
		{
			framerate = framerates.framerates[i];
			foundFramerate = true;
			break;
		}
	}

	// if we didn't find it, select the highest one.
	if( !foundFramerate ) framerate=framerates.framerates[framerates.num-1];

	/*-----------------------------------------------------------------------
	*  setup capture
	*-----------------------------------------------------------------------*/
	fprintf(stderr,"Setting capture\n");
	dc1394_video_set_iso_speed(camera, DC1394_ISO_SPEED_400);

	dc1394_video_set_mode(camera, video_mode);
	dc1394_video_set_framerate(camera, framerate);

	if (dc1394_capture_setup(camera,4,DC1394_CAPTURE_FLAGS_DEFAULT)!=DC1394_SUCCESS)
	{
		fprintf( stderr,"Unable to setup camera!\n - check that the video mode and framerate are supported by your camera\n");
		cleanupCamera();
		return false;
	}

	cout << "chosen video format: "; Libdc1394GrabberUtils::print_format( video_mode ); cout << endl;
	cout << "chosen frame rate: " << Libdc1394GrabberFramerateHelper::DcLibFramerateToString( framerate ) << endl;

/*
	// report camera's features ----------------------------------
	if (dc1394_get_camera_feature_set(camera,&features) !=DC1394_SUCCESS)  { fprintf( stderr, "unable to get feature set\n"); }
	else  { dc1394_print_feature_set(&features); }
*/


	fprintf(stderr,"start transmission\n");
	/*-----------------------------------------------------------------------
	*  have the camera start sending us data
	*-----------------------------------------------------------------------*/
	if (dc1394_video_set_transmission(camera, DC1394_ON) !=DC1394_SUCCESS)
	{
		fprintf( stderr, "unable to start camera iso transmission\n");
		cleanupCamera();
	}

	fprintf(stderr,"wait transmission\n");
	/*-----------------------------------------------------------------------
	*  Sleep until the camera has a transmission
	*-----------------------------------------------------------------------*/
	dc1394switch_t status = DC1394_OFF;

	i = 0;
	while( status == DC1394_OFF && i++ < 5 )
	{
		usleep(50000);
		if (dc1394_video_get_transmission(camera, &status)!=DC1394_SUCCESS)
		{
			fprintf(stderr, "unable to get transmission status\n");
			cleanupCamera();
		}
	}

	if( i == 5 )
	{
		fprintf(stderr,"Camera doesn't seem to want to turn on!\n");
		cleanupCamera();
	}


    dc1394_get_image_size_from_video_mode(camera, video_mode, &width, &height);
    outputImageWidth = width;
    outputImageHeight = height;

	initFeatures();

	fprintf(stderr,"capture\n");

	return true;
}


void Libdc1394Grabber::initInternalBuffers()
{

	if( targetFormat == VID_FORMAT_GREYSCALE || targetFormat == VID_FORMAT_Y8 ) { bpp = VID_BPP_GREY; }
	else if( targetFormat == VID_FORMAT_RGB || targetFormat == VID_FORMAT_BGR ) { bpp = VID_BPP_RGB; }
	else{ cout << "**** " << endl << "*** Libdc1394Grabber: Unsupported output format! *** " << endl << "***************************************** " << endl; }

	finalImageDataBufferLength = width*height*bpp;
	pixels = new unsigned char[finalImageDataBufferLength];
	for ( int i = 0; i < finalImageDataBufferLength; i++ ) { pixels[i] = 0; }

}

void Libdc1394Grabber::threadedFunction()
{
	while( 1 )
	{
		captureFrame();
		ofSleepMillis(2);
	}
	return;
}

unsigned char* Libdc1394Grabber::getPixels()
{
    return pixels;
}


bool Libdc1394Grabber::grabFrame(unsigned char ** _pixels)
{
    if(bHasNewFrame)
    {
        bHasNewFrame = false;
        memcpy(*_pixels,pixels,width*height*bpp);
        return true;
    }
    else return false;
}

void Libdc1394Grabber::captureFrame()
{

	if( !bHasNewFrame && (camera != NULL ))
	{
		/*-----------------------------------------------------------------------
		*  capture one frame
		*-----------------------------------------------------------------------*/
		if (dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame) != DC1394_SUCCESS)
		{
			fprintf(stderr, "unable to capture a frame\n");
			cleanupCamera();
		}

		if( !grabbedFirstImage )
		{
			setBayerPatternIfNeeded();
			grabbedFirstImage = true;
		}

		processCameraImageData( frame->image );

		dc1394_capture_enqueue(camera, frame);

		bHasNewFrame = true;
	}

}


void Libdc1394Grabber::processCameraImageData( unsigned char* _cameraImageData )
{
    static bool writeonce = true;

	if( sourceFormatLibDC == DC1394_COLOR_CODING_RAW8 || sourceFormatLibDC == DC1394_COLOR_CODING_MONO8 )
	{

		if( targetFormat == VID_FORMAT_GREYSCALE || targetFormat == VID_FORMAT_Y8 )
		{
			pixels = _cameraImageData;
//			if(writeonce) {
//			    writeonce = false;
//                cout << "processCameraImageData() targetFormat  = VID_FORMAT_GREYSCALE || targetFormat == VID_FORMAT_Y8" << endl;
//			}
		}
		else if( targetFormat == VID_FORMAT_RGB )
		{
			dc1394_bayer_decoding_8bit( _cameraImageData, pixels, width, height,  bayerPattern, bayerMethod );
			if(writeonce) {
			    writeonce = false;
                cout << "processCameraImageData() targetFormat  = VID_FORMAT_RGB" << endl;
			}
		}
		else if ( targetFormat == VID_FORMAT_BGR )
		{
			dc1394_bayer_decoding_8bit( _cameraImageData, pixels, width, height,  bayerPattern, bayerMethod ); // we should really be converting this
		}
		else
		{
			cout << "************* Libdc1394Grabber::processCameraImageData Unsupported target format (" << videoFormatToString( targetFormat ) << ") from DC1394_COLOR_CODING_RAW8 or DC1394_COLOR_CODING_MONO8 *************" << endl;
		}

	}
	else if(  sourceFormatLibDC == DC1394_COLOR_CODING_MONO16 || sourceFormatLibDC == DC1394_COLOR_CODING_RAW16 )
	{
		if( targetFormat == VID_FORMAT_RGB )
		{
//			dc1394_bayer_decoding_16bit( _cameraImageData, pixels, width, height,  bayerPattern, bayerMethod );
		}
		else if ( targetFormat == VID_FORMAT_BGR )
		{
//			dc1394_bayer_decoding_16bit( _cameraImageData, pixels, width, height,  bayerPattern, bayerMethod ); // we should really be converting this
		}
		else
		{
			cout << "************* Libdc1394Grabber::processCameraImageData Unsupported target format (" << videoFormatToString( targetFormat ) << ") from DC1394_COLOR_CODING_MONO16 or DC1394_COLOR_CODING_RAW16 *************" << endl;
		}
	}
	else if(  sourceFormatLibDC == DC1394_COLOR_CODING_YUV411 || sourceFormatLibDC == DC1394_COLOR_CODING_YUV422 || sourceFormatLibDC == DC1394_COLOR_CODING_YUV444 )
	{
		if( targetFormat == VID_FORMAT_RGB )
		{
			dc1394_convert_to_RGB8( _cameraImageData, pixels, width, height, YUV_BYTE_ORDER, sourceFormatLibDC, 16);
		}
		else if ( targetFormat == VID_FORMAT_BGR )
		{
			dc1394_convert_to_RGB8( _cameraImageData, pixels, width, height, YUV_BYTE_ORDER, sourceFormatLibDC, 16); // we should really be converting this
		}
		else
		{
			cout << "************* Libdc1394Grabber::processCameraImageData Unsupported target format (" << videoFormatToString( targetFormat ) << ") from DC1394_COLOR_CODING_YUV411, DC1394_COLOR_CODING_YUV422 or DC1394_COLOR_CODING_YUV444 *************" << endl;
		}

	}
	else if(  sourceFormatLibDC == DC1394_COLOR_CODING_RGB8 )
	{
		if( targetFormat == VID_FORMAT_RGB )
		{
		    pixels = _cameraImageData;
		}
		else if ( targetFormat == VID_FORMAT_BGR )
		{
		    pixels = _cameraImageData;
			//memcpy ( pixels, _cameraImageData, finalImageDataBufferLength ); // we should really be converting this
		}
		else
		{
			cout << "************* Libdc1394Grabber::processCameraImageData Unsupported target format (" << videoFormatToString( targetFormat ) << ") from DC1394_COLOR_CODING_RGB8 *************" << endl;
		}
	}
	else
	{

		cout << "************* Libdc1394Grabber::processCameraImageData Unsupported source format! *************" << endl;
	}

}

void Libdc1394Grabber::setBayerPatternIfNeeded()
{
	if( sourceFormatLibDC == DC1394_COLOR_CODING_RAW8 || sourceFormatLibDC == DC1394_COLOR_CODING_MONO8 || sourceFormatLibDC == DC1394_COLOR_CODING_MONO16 || sourceFormatLibDC == DC1394_COLOR_CODING_RAW16 )
	{
		if( targetFormat == VID_FORMAT_RGB || targetFormat == VID_FORMAT_BGR || targetFormat == VID_FORMAT_GREYSCALE )
		{

//		dc1394color_filter_t tmpBayerPattern;
//		if( dc1394_format7_get_color_filter(camera, video_mode, &tmpBayerPattern) != DC1394_SUCCESS )
//		{
//			cout << "Libdc1394Grabber::setBayerPatternIfNeeded(), Failed to get the dc1394_format7_get_color_filter." << endl;
//		}
//		else
//		{
//			cout << "Libdc1394Grabber::setBayerPatternIfNeeded(), We got a pattern, it was: " << tmpBayerPattern << endl;
//		}


			if ( Libdc1394GrabberUtils::getBayerTile( camera, &bayerPattern ) != DC1394_SUCCESS )
			{
				ofLog(OF_LOG_WARNING,"Could not get bayer tile pattern from camera" );
			} else {
			    ofLog(OF_LOG_WARNING,"Grabbed a bayer pattern from the camera");
            }
		}

	}
}


void Libdc1394Grabber::cleanupCamera()
{
	stopThread();
	//while(isThreadRunning()) 1;
	//this sleep seems necessary, at least on OSX, to avoid an occasional hang on exit
	ofSleepMillis(20);

	dc1394switch_t is_iso_on = DC1394_OFF;
	if (dc1394_video_get_transmission(camera, &is_iso_on)!=DC1394_SUCCESS) {
		is_iso_on = DC1394_ON; // try to shut ISO anyway
	}
	if (is_iso_on > DC1394_OFF) {
		if (dc1394_video_set_transmission(camera, DC1394_OFF)!=DC1394_SUCCESS) {
			fprintf(stderr,"Could not stop ISO transmission\n");
		}
	}

	/* cleanup and exit */
	dc1394_capture_stop(camera);
	dc1394_camera_free (camera);
	camera = NULL;

	if(d) {
		dc1394_free (d);
		d = NULL;
	}

}




/*-----------------------------------------------------------------------
 *  Feature Methods
 *-----------------------------------------------------------------------*/
void Libdc1394Grabber::initFeatures()
{

	dc1394featureset_t tmpFeatureSet;

	// get camera features ----------------------------------
	if ( dc1394_feature_get_all(camera,&tmpFeatureSet) !=DC1394_SUCCESS)
	{
        fprintf( stderr, "unable to get feature set\n");
    }
	else
	{
		//dc1394_print_feature_set(&tmpFeatureSet);

		int tmpAvailableFeatures = 0;
		for( int i = 0; i < TOTAL_CAMERA_FEATURES; i++ )
		{
			if( tmpFeatureSet.feature[i].available ) { tmpAvailableFeatures++; }
		}

		availableFeatureAmount = tmpAvailableFeatures;

		featureVals = new ofxVideoGrabberFeature[availableFeatureAmount];

		tmpAvailableFeatures = 0;

        cout << "Name\t" << "\tAbs." << "\tCurr." << "\tCurr2."  << "\tMin." << "\tMax." << endl;
		for( int i = 0; i < TOTAL_CAMERA_FEATURES; i++ )
		{
			if(tmpFeatureSet.feature[i].available)
			{
				featureVals[tmpAvailableFeatures].feature			= Libdc1394GrabberFeatureHelper::libdcFeatureToAVidFeature( tmpFeatureSet.feature[i].id );
				featureVals[tmpAvailableFeatures].name				= cameraFeatureToTitle( featureVals[tmpAvailableFeatures].feature );

				featureVals[tmpAvailableFeatures].isPresent			= tmpFeatureSet.feature[i].available;
				featureVals[tmpAvailableFeatures].isReadable		= tmpFeatureSet.feature[i].readout_capable;

				featureVals[tmpAvailableFeatures].hasAbsoluteMode	= tmpFeatureSet.feature[i].absolute_capable;
				featureVals[tmpAvailableFeatures].absoluteModeActive = tmpFeatureSet.feature[i].abs_control;

                for(unsigned int j = 0; j < tmpFeatureSet.feature[i].modes.num; j++)
                {
                    if(tmpFeatureSet.feature[i].modes.modes[j] == DC1394_FEATURE_MODE_MANUAL)
                    {
                        featureVals[tmpAvailableFeatures].hasManualMode	= true;
                    }
                    else if(tmpFeatureSet.feature[i].modes.modes[j] == DC1394_FEATURE_MODE_AUTO)
                    {
                        featureVals[tmpAvailableFeatures].hasAutoMode = true;
                    }
                    else if(tmpFeatureSet.feature[i].modes.modes[j] == DC1394_FEATURE_MODE_ONE_PUSH_AUTO)
                    {
                        featureVals[tmpAvailableFeatures].hasOnePush = true;
                    }
                }

				featureVals[tmpAvailableFeatures].isOnOffSwitchable	= tmpFeatureSet.feature[i].on_off_capable;
				featureVals[tmpAvailableFeatures].isOn				= tmpFeatureSet.feature[i].is_on;

                if(tmpFeatureSet.feature[i].current_mode == DC1394_FEATURE_MODE_MANUAL)
                {
                    featureVals[tmpAvailableFeatures].hasManualActive	= true;
                }
                else if(tmpFeatureSet.feature[i].current_mode == DC1394_FEATURE_MODE_AUTO)
                {
                    featureVals[tmpAvailableFeatures].hasAutoModeActive	= true;
                }
                else if(tmpFeatureSet.feature[i].current_mode == DC1394_FEATURE_MODE_ONE_PUSH_AUTO)
                {
                    featureVals[tmpAvailableFeatures].hasOnePushActive = true;
                }


				if( featureVals[tmpAvailableFeatures].hasAbsoluteMode )
				{
					dc1394_feature_set_absolute_control( camera, tmpFeatureSet.feature[i].id, DC1394_OFF );

/* Not using Abs values (yet?) */
//					featureVals[tmpAvailableFeatures].currVal = tmpFeatureSet.feature[i].abs_value;
//
//					featureVals[tmpAvailableFeatures].minVal  = tmpFeatureSet.feature[i].abs_min;
//					featureVals[tmpAvailableFeatures].maxVal  = tmpFeatureSet.feature[i].abs_max;

				}
				else
				{
                    if(tmpFeatureSet.feature[i].id == DC1394_FEATURE_WHITE_BALANCE)
                    {
                        featureVals[tmpAvailableFeatures].currVal = (float)tmpFeatureSet.feature[i].BU_value;
                        featureVals[tmpAvailableFeatures].currVal2 = (float)tmpFeatureSet.feature[i].RV_value;
                        featureVals[tmpAvailableFeatures].minVal  = (float)tmpFeatureSet.feature[i].min;
                        featureVals[tmpAvailableFeatures].maxVal  = (float)tmpFeatureSet.feature[i].max;
                    }
                    else {
                        featureVals[tmpAvailableFeatures].currVal = (float)tmpFeatureSet.feature[i].value;
                        featureVals[tmpAvailableFeatures].minVal  = (float)tmpFeatureSet.feature[i].min;
                        featureVals[tmpAvailableFeatures].maxVal  = (float)tmpFeatureSet.feature[i].max;
                    }
				}

				cout << setw(13) << featureVals[tmpAvailableFeatures].name
				<< " :\t" << featureVals[tmpAvailableFeatures].hasAbsoluteMode
				<< "\t" << featureVals[tmpAvailableFeatures].currVal;

				if(tmpFeatureSet.feature[i].id == DC1394_FEATURE_WHITE_BALANCE)
				{
				    cout << "\t" << featureVals[tmpAvailableFeatures].currVal2;
                }
                else
                {
                    cout << "\t";
                }

				cout << "\t" << featureVals[tmpAvailableFeatures].minVal
				<< "\t" << featureVals[tmpAvailableFeatures].maxVal << endl;

				tmpAvailableFeatures++;
			}
		}

	}

}

void Libdc1394Grabber::getAllFeatureValues()
{

	if( availableFeatureAmount == 0 ) return;

	for( int i = 0; i < availableFeatureAmount; i++ )
	{
		if(featureVals[i].isPresent)
		{
			getFeatureValues( &featureVals[i] );
		}
	}

}


void Libdc1394Grabber::getFeatureValues( ofxVideoGrabberFeature* _feature )
{

	dc1394feature_info_t tmpFeatureVals;
	tmpFeatureVals.id = Libdc1394GrabberFeatureHelper::AVidFeatureToLibdcFeature( _feature->feature );

	dc1394_feature_get( camera, &tmpFeatureVals);

/* Not using Abs values (yet?) */
//	if( _feature->hasAbsoluteMode )
//	{
//		_feature->currVal = tmpFeatureVals.abs_value;
//		_feature->minVal = tmpFeatureVals.abs_min;
//		_feature->maxVal = tmpFeatureVals.abs_max;
//	}
//	else
	{
        if(tmpFeatureVals.id == DC1394_FEATURE_WHITE_BALANCE)
        {
            _feature->currVal = (float)tmpFeatureVals.BU_value;
            _feature->currVal2 = (float)tmpFeatureVals.RV_value;
            _feature->minVal = (float)tmpFeatureVals.min;
            _feature->maxVal = (float)tmpFeatureVals.max;
        }
        else {
            _feature->currVal = (float)tmpFeatureVals.value;
            _feature->minVal = (float)tmpFeatureVals.min;
            _feature->maxVal = (float)tmpFeatureVals.max;
        }
	}

	if( _feature->isOnOffSwitchable )
	{
		_feature->isOn = tmpFeatureVals.is_on;
	}

	if( _feature->hasManualMode )
	{
        if(tmpFeatureVals.current_mode == DC1394_FEATURE_MODE_MANUAL)
        {
            _feature->hasManualActive = true;
        }
        else
        {
            _feature->hasManualActive = false;
        }
	}

	if( _feature->hasAutoMode )
	{
        if(tmpFeatureVals.current_mode == DC1394_FEATURE_MODE_AUTO)
        {
            _feature->hasAutoModeActive = true;
        }
        else
        {
            _feature->hasAutoModeActive = false;
        }
	}

    if( _feature->hasOnePush )
	{
        if(tmpFeatureVals.current_mode == DC1394_FEATURE_MODE_ONE_PUSH_AUTO)
        {
            _feature->hasOnePushActive = true;
        }
        else
        {
            _feature->hasOnePushActive = false;
        }
	}

}

void Libdc1394Grabber::setFeatureAbsoluteValue( float _val, int _feature )
{
	dc1394error_t err;

	//cout << " Libdc1394Grabber::setFeatureAbsoluteVal " << _val << ", " << _feature << endl;

	err = dc1394_feature_set_absolute_value( camera, Libdc1394GrabberFeatureHelper::AVidFeatureToLibdcFeature( _feature ), _val );
	if( err != DC1394_SUCCESS )
	{
		cout << "*******************" << endl;
		//cout << dc1394_error_strings[err] << endl;
		cout << "Libdc1394Grabber::setFeatureAbsoluteVal, failed to set feature: " << cameraFeatureToTitle( _feature ) << endl;
		cout << "*******************" << endl;
	}
}

void Libdc1394Grabber::setFeatureValue( float _val1, float _val2, int _feature)
{
	dc1394error_t err = DC1394_FAILURE;

    if(_feature == FEATURE_WHITE_BALANCE) {
        // _val = u_b, _val2 = v_r for whitebalance
        err = dc1394_feature_whitebalance_set_value( camera, (unsigned int) _val1, (unsigned int) _val2 );
    }
	if( err != DC1394_SUCCESS )
	{
		cout << "*******************" << endl;
		cout << "Libdc1394Grabber::setFeatureVal, failed to set feature: " << cameraFeatureToTitle( _feature ) << endl;
		cout << "*******************" << endl;
	}
}

void Libdc1394Grabber::setFeatureValue( float _val, int _feature )
{
	dc1394error_t err;

	//cout << " Libdc1  394Grabber::setFeatureVal " << _val << ", " << _feature << endl;

	err = dc1394_feature_set_value( camera, Libdc1394GrabberFeatureHelper::AVidFeatureToLibdcFeature( _feature ), (unsigned int)_val );


	if( err != DC1394_SUCCESS )
	{
		cout << "*******************" << endl;
		cout << "Libdc1394Grabber::setFeatureVal, failed to set feature: " << cameraFeatureToTitle( _feature ) << endl;
		cout << "*******************" << endl;
	}

}

//void Libdc1394Grabber::getFeatureModes()
//{
//    dc1394error_t err;
//    err = dc1394_feature_has_auto_mode(camera, DC1394_FEATURE_MODE_MANUAL, &value);
//    if( (err == DC1394_SUCCESS) && value )
//    {
//
//    }
//}

void Libdc1394Grabber::setFeatureMode(int _featureMode, int _feature )
{
    dc1394error_t err = DC1394_FAILURE;
    dc1394feature_t tmpFeature = Libdc1394GrabberFeatureHelper::AVidFeatureToLibdcFeature( _feature );
    int index = -1;

	for( int i = 0; i < availableFeatureAmount; i++ )
	{
		if(featureVals[i].feature == _feature)
		{
                index = i;
		}
	}

	if (_featureMode == FEATURE_MODE_MANUAL)
	{
	    if(featureVals[index].isOnOffSwitchable) {
            setFeatureOnOff(true,_feature);
	    }
	    err = dc1394_feature_set_mode( camera, tmpFeature, DC1394_FEATURE_MODE_MANUAL );
//	    if(err == DC1394_SUCCESS) {
//            err = dc1394_feature_set_absolute_control( camera, tmpFeature, DC1394_OFF );
//	    }
	}
	else if(_featureMode == FEATURE_MODE_AUTO)
	{
	    if(featureVals[index].isOnOffSwitchable) {
            setFeatureOnOff(true,_feature);
	    }
	    err = dc1394_feature_set_mode( camera, tmpFeature, DC1394_FEATURE_MODE_AUTO );
//	    if(err == DC1394_SUCCESS) {
//            err = dc1394_feature_set_absolute_control( camera, tmpFeature, DC1394_OFF );
//	    }
	}
	else if(_featureMode == FEATURE_MODE_ONE_PUSH_AUTO)
	{
	    if(featureVals[index].isOnOffSwitchable) {
            setFeatureOnOff(true,_feature);
	    }
	    err = dc1394_feature_set_mode( camera, tmpFeature, DC1394_FEATURE_MODE_ONE_PUSH_AUTO );
//	    if(err == DC1394_SUCCESS) {
//            err = dc1394_feature_set_absolute_control( camera, tmpFeature, DC1394_OFF );
//	    }
	}

    if( err != DC1394_SUCCESS )
	{
		cout << "*******************" << endl;
		cout << "Libdc1394Grabber::setFeatureMode, failed to set mode for feature: " << cameraFeatureToTitle( _feature ) << endl;
		cout << "*******************" << endl;
	}
}

bool Libdc1394Grabber::setFeatureOnOff( bool _val, int _feature )
{
	dc1394error_t err = DC1394_FAILURE;
	dc1394feature_t tmpFeature = Libdc1394GrabberFeatureHelper::AVidFeatureToLibdcFeature( _feature );

    if( _val ) {
        err = dc1394_feature_set_power( camera, tmpFeature, DC1394_ON );
    }
    else {
        err = dc1394_feature_set_power( camera, tmpFeature, DC1394_OFF );
    }
    return true;

	if( err != DC1394_SUCCESS )
	{
		cout << "*******************" << endl;
		cout << "Libdc1394Grabber::setFeatureStateOnOff, failed to set feature: " << cameraFeatureToTitle( _feature ) << endl;
		cout << "*******************" << endl;
	}
	return false;
}



void Libdc1394Grabber::setFeaturesOnePushMode()
{
	setFeatureMode( FEATURE_SHUTTER, FEATURE_MODE_MANUAL );
	dc1394_feature_set_mode( camera, DC1394_FEATURE_SHUTTER,	DC1394_FEATURE_MODE_ONE_PUSH_AUTO );

	//dc1394_feature_set_mode( camera, DC1394_FEATURE_WHITE_BALANCE, DC1394_FEATURE_MODE_ONE_PUSH_AUTO );

	setFeatureMode( FEATURE_GAIN, FEATURE_MODE_MANUAL );
	dc1394_feature_set_mode( camera, DC1394_FEATURE_GAIN,		DC1394_FEATURE_MODE_ONE_PUSH_AUTO );

	//dc1394_feature_set_mode( camera, DC1394_FEATURE_IRIS,		DC1394_FEATURE_MODE_ONE_PUSH_AUTO );

	setFeatureMode( FEATURE_EXPOSURE, FEATURE_MODE_MANUAL );
	dc1394_feature_set_mode( camera, DC1394_FEATURE_EXPOSURE,	DC1394_FEATURE_MODE_ONE_PUSH_AUTO );

	//dc1394_feature_set_mode( camera, DC1394_FEATURE_BRIGHTNESS, DC1394_FEATURE_MODE_ONE_PUSH_AUTO );
}

int Libdc1394Grabber::stringToFeature( string _featureName )
{
	return -1;
}
