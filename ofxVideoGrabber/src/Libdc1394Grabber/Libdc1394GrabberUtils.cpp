#include "Libdc1394GrabberUtils.h"

#include <iostream>
using namespace std;

/*-----------------------------------------------------------------------
 *  Prints the type of format to standard out
 *-----------------------------------------------------------------------*/
void Libdc1394GrabberUtils::print_format( uint32_t format )
{
#define print_case(A) case A: printf(#A ""); break;

  switch( format ) {
    print_case(DC1394_VIDEO_MODE_160x120_YUV444);
    print_case(DC1394_VIDEO_MODE_320x240_YUV422);
    print_case(DC1394_VIDEO_MODE_640x480_YUV411);
    print_case(DC1394_VIDEO_MODE_640x480_YUV422);
    print_case(DC1394_VIDEO_MODE_640x480_RGB8);
    print_case(DC1394_VIDEO_MODE_640x480_MONO8);
    print_case(DC1394_VIDEO_MODE_640x480_MONO16);
    print_case(DC1394_VIDEO_MODE_800x600_YUV422);
    print_case(DC1394_VIDEO_MODE_800x600_RGB8);
    print_case(DC1394_VIDEO_MODE_800x600_MONO8);
    print_case(DC1394_VIDEO_MODE_1024x768_YUV422);
    print_case(DC1394_VIDEO_MODE_1024x768_RGB8);
    print_case(DC1394_VIDEO_MODE_1024x768_MONO8);
    print_case(DC1394_VIDEO_MODE_800x600_MONO16);
    print_case(DC1394_VIDEO_MODE_1024x768_MONO16);
    print_case(DC1394_VIDEO_MODE_1280x960_YUV422);
    print_case(DC1394_VIDEO_MODE_1280x960_RGB8);
    print_case(DC1394_VIDEO_MODE_1280x960_MONO8);
    print_case(DC1394_VIDEO_MODE_1600x1200_YUV422);
    print_case(DC1394_VIDEO_MODE_1600x1200_RGB8);
    print_case(DC1394_VIDEO_MODE_1600x1200_MONO8);
    print_case(DC1394_VIDEO_MODE_1280x960_MONO16);
    print_case(DC1394_VIDEO_MODE_1600x1200_MONO16);
    print_case(DC1394_VIDEO_MODE_EXIF);
    print_case(DC1394_VIDEO_MODE_FORMAT7_0);
    print_case(DC1394_VIDEO_MODE_FORMAT7_1);
    print_case(DC1394_VIDEO_MODE_FORMAT7_2);
    print_case(DC1394_VIDEO_MODE_FORMAT7_3);
    print_case(DC1394_VIDEO_MODE_FORMAT7_4);
    print_case(DC1394_VIDEO_MODE_FORMAT7_5);
    print_case(DC1394_VIDEO_MODE_FORMAT7_6);
    print_case(DC1394_VIDEO_MODE_FORMAT7_7);

  default:
    fprintf(stderr,"Unknown format\n");
  }

}

/*-----------------------------------------------------------------------
 *  Returns the number of pixels in the image based upon the format
 *-----------------------------------------------------------------------*/
uint32_t Libdc1394GrabberUtils::get_num_pixels(dc1394camera_t *camera, uint32_t format ) {
  uint32_t w,h;

  dc1394_get_image_size_from_video_mode(camera, format,&w,&h);

  return w*h;
}

/*-----------------------------------------------------------------------
 *  Prints the type of color encoding
 *-----------------------------------------------------------------------*/
void Libdc1394GrabberUtils::print_color_coding( uint32_t color_id )
{
  switch( color_id ) {
  case DC1394_COLOR_CODING_MONO8:
    printf("MONO8");
    break;
  case DC1394_COLOR_CODING_YUV411:
    printf("YUV411");
    break;
  case DC1394_COLOR_CODING_YUV422:
    printf("YUV422");
    break;
  case DC1394_COLOR_CODING_YUV444:
    printf("YUV444");
    break;
  case DC1394_COLOR_CODING_RGB8:
    printf("RGB8");
    break;
  case DC1394_COLOR_CODING_MONO16:
    printf("MONO16");
    break;
  case DC1394_COLOR_CODING_RGB16:
    printf("RGB16");
    break;
  case DC1394_COLOR_CODING_MONO16S:
    printf("MONO16S");
    break;
  case DC1394_COLOR_CODING_RGB16S:
    printf("RGB16S");
    break;
  case DC1394_COLOR_CODING_RAW8:
    printf("RAW8");
    break;
  case DC1394_COLOR_CODING_RAW16:
    printf("RAW16");
    break;

  default:
    fprintf(stderr,"Unknown color coding = %d\n",color_id);
  }
}


/*-----------------------------------------------------------------------
 *  Prints various information about the mode the camera is in
 *-----------------------------------------------------------------------*/
void Libdc1394GrabberUtils::print_mode_info( dc1394camera_t *camera , dc1394video_mode_t mode )
{
    printf("**********************************************\n");
    printf("Mode: ");
    print_format(mode);
    printf("\n");

    dc1394framerates_t framerates;
    if(dc1394_video_get_supported_framerates(camera,mode,&framerates) != DC1394_SUCCESS) {
        fprintf( stderr, "Can't get frame rates\n");
    }

    printf("Frame Rates:\n");

    for(unsigned int j = 0; j < framerates.num; j++ )
    {
        dc1394framerate_t rate = framerates.framerates[j];
        float f_rate;
        dc1394_framerate_as_float(rate,&f_rate);
        printf("  [%d] rate = %f\n",j,f_rate );
    }

    printf("**********************************************\n");
}


dc1394error_t Libdc1394GrabberUtils::getBayerTile( dc1394camera_t* camera, dc1394color_filter_t* bayerPattern )
{

   uint32_t value;
   dc1394error_t err;

   // query register 0x1040
   // This register is an advanced PGR register called BAYER_TILE_MAPPING
   // For more information check the PGR IEEE-1394 Digital Camera Register Reference
  // err = GetCameraControlRegister( camera, 0x1040, &value );
   err = dc1394_get_register( camera, 0x1040, &value );

   if ( err != DC1394_SUCCESS )
   {
      return err;
   }

   // Ascii R = 52 G = 47 B = 42 Y = 59
   switch( value )
   {
      default:
      case 0x59595959:	// YYYY
	 //*bayerPattern = (dc1394color_filter_t) 0;
	 cout << "using default Bayer" << endl;
	 break;
      case 0x52474742:	// RGGB
	 *bayerPattern = DC1394_COLOR_FILTER_RGGB;
	 cout << "RGGB Bayer" << endl;
	 break;
      case 0x47425247:	// GBRG
	 *bayerPattern = DC1394_COLOR_FILTER_GBRG;
	 cout << "GBRG Bayer" << endl;
	 break;
      case 0x47524247:	// GRBG
	 *bayerPattern = DC1394_COLOR_FILTER_GRBG;
	 cout << "GRBG Bayer" << endl;
	 break;
      case 0x42474752:	// BGGR
	 *bayerPattern = DC1394_COLOR_FILTER_BGGR;
	 cout << "BGGR Bayer" << endl;
	 break;
   }

   return err;
}
