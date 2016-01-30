# ofxVideoGrabber core (with Libdc1394) #

## Version 0.1 ##
  * First available download. No saving or loading of settings yet

## Version 0.11 ##
  * Fixed crash bug when not using texture.
  * Fixed FPS conversion error on OSX

## Version 0.12 ##
  * Fixed bug for certain cameras not debayering properly

## Version 0.3 ##

Basic framework improvements:
  * Loading/Saving from XML
  * setVerbose(), setDeviceID and listDevices() functions
  * Normalized GUI api
  * Fixed small GUI layout bug

Libdc1394 grabber improvements:
  * Thread locking
  * Format 7 support including region of interest (ROI)
  * setDiscardFrames() for better interactivity response
  * 1394b support
  * Cleaned up initialization code
  * Initialise cameras by GUID and unit number (latter is useful for specifying stereo  cams)
  * Mono16 greyscale support
  * Bandwidth and ISO cleanup on start-up
  * Almost all output messages are using the ofLog system