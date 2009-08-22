#include "ofxIIDCSettings.h"
#include "ofxVideoGrabberSDK.h"


ofxIIDCSettings::ofxIIDCSettings()
{

}

ofxIIDCSettings::~ofxIIDCSettings()
{

}



void ofxIIDCSettings::setupGUI()
{
    //callback1.SetCallback( dynamic_cast<ofxIIDCSettings*> (this), &ofxIIDCSettings::parameterCallback);
    videoGrabber->getAllFeatureValues();

    panel.setup("ofxIIDCSettings", 0, 0, 480, ofGetHeight());
//	panel.loadFont("OSP_-_NotCourierSans_1.otf", 10);
	panel.loadFont("LiberationMono-Regular.ttf", 10);

    panel.addPanel("Settings", 2, false);
	panel.setWhichPanel("Settings");

    callback.SetCallback( this, &ofxIIDCSettings::parameterCallback);

	panel.setWhichColumn(0);
	int tmp_index = -1;
	int tmp_feature = -1;
	bool whitebalance = false;
	int feature = -1;

    for (int i=0; i < videoGrabber->availableFeatureAmount; ++i)
    {
        if(videoGrabber->featureVals[i].isPresent) {
            feature = videoGrabber->featureVals[i].feature;

            if( feature == FEATURE_WHITE_BALANCE) {
                tmp_index = i;
                tmp_feature = feature;
                whitebalance = true;
            }
            else if(feature == FEATURE_TRIGGER) {
                //not using any features above this
                break;
            }
            else {
                string name = videoGrabber->featureVals[i].name;
                string nameCaps = name;
                transform(nameCaps.begin(), nameCaps.end(), nameCaps.begin(), ::toupper);

                float minVal = videoGrabber->featureVals[i].minVal;
                float maxVal = videoGrabber->featureVals[i].maxVal;
                float currVal = videoGrabber->featureVals[i].currVal;
                panel.addButtonSlider(name,nameCaps, currVal, minVal, maxVal, true,&callback,feature);
            }
        }
    }

    if(whitebalance)
    {
        string name = videoGrabber->featureVals[tmp_index].name;
        string nameCaps = name;
        transform(nameCaps.begin(), nameCaps.end(), nameCaps.begin(), ::toupper);

        float minVal = videoGrabber->featureVals[tmp_index].minVal;
        float maxVal = videoGrabber->featureVals[tmp_index].maxVal;
        float currVal = videoGrabber->featureVals[tmp_index].currVal;
        float currVal2 = videoGrabber->featureVals[tmp_index].currVal2;
        panel.addSlider2D(name,nameCaps, currVal, currVal2, minVal, maxVal, minVal, maxVal, false, &callback,tmp_feature);

    }


    panel.setWhichColumn(1);
    for (int i=0; i < videoGrabber->availableFeatureAmount; ++i)
    {
        if(videoGrabber->featureVals[i].isPresent) {
            feature = videoGrabber->featureVals[i].feature;

            if (feature == FEATURE_TRIGGER)
            {
                //not using any features above this
                break;
            }
            if( feature != FEATURE_WHITE_BALANCE)
            {
                string name = videoGrabber->featureVals[i].name + " mode";
                string nameCaps = name + "_MODE";
                transform(nameCaps.begin(), nameCaps.end(), nameCaps.begin(), ::toupper);

                int defaultval = FEATURE_MODE_MANUAL;

                if(videoGrabber->featureVals[i].hasAutoModeActive) {
                    defaultval = FEATURE_MODE_AUTO;
					videoGrabber->setFeatureMode(FEATURE_MODE_AUTO, feature );
                } else {
					videoGrabber->setFeatureMode(FEATURE_MODE_MANUAL, feature );
				}
                vector <string> modelist;
                modelist.push_back("MANUAL");


                if(videoGrabber->featureVals[i].hasAutoMode)
                {
                    modelist.push_back("AUTO");
                    if(videoGrabber->featureVals[i].hasOnePush )
                    {
                        modelist.push_back("ONE PUSH AUTO");
                    }
                }

                panel.addTextDropDown(name,nameCaps, defaultval, modelist,&callback,feature);
            }
        }
    }

    if(whitebalance)
    {
                feature = FEATURE_WHITE_BALANCE;
                string name = videoGrabber->featureVals[tmp_index].name + " mode";
                string nameCaps = name + "_MODE";
                transform(nameCaps.begin(), nameCaps.end(), nameCaps.begin(), ::toupper);

                int defaultval = FEATURE_MODE_MANUAL;
                if(videoGrabber->featureVals[tmp_index].hasAutoModeActive) {
                    defaultval = FEATURE_MODE_AUTO;
					videoGrabber->setFeatureMode(FEATURE_MODE_AUTO, feature );
                } else {
					videoGrabber->setFeatureMode(FEATURE_MODE_MANUAL, feature );
				}
                vector <string> modelist;
                modelist.push_back("MANUAL");

                if(videoGrabber->featureVals[tmp_index].hasAutoMode)
                {
                    modelist.push_back("AUTO");
                    if(videoGrabber->featureVals[tmp_index].hasOnePush )
                    {
                        modelist.push_back("ONE PUSH AUTO");
                    }
                }

                panel.addTextDropDown(name,nameCaps, defaultval, modelist,&callback,tmp_feature);

    }


}

//--------------------------------------------------------------------
void ofxIIDCSettings::draw()
{
    panel.draw();
}

//--------------------------------------------------------------------
void ofxIIDCSettings::update()
{
    panel.update();
}

void ofxIIDCSettings::parameterCallback(float param1, float param2, int param_mode, int param_id)
{

    if(param_mode != NULL_MODE)
    {
        //cout << "set mode called: "  << param_mode << " param_id = " << param_id << endl;
        videoGrabber->setFeatureMode(param_mode, param_id);
    }
    else {
        if(param_id == FEATURE_WHITE_BALANCE) {
            videoGrabber->setFeatureValue(param1, param2, param_id);
        }
        else {
            //cout << "set feature called: " << param1 << " " << param_id << endl;
            videoGrabber->setFeatureValue(param1, param_id);
        }
    }

}





