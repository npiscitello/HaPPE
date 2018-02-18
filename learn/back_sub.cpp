#include <iostream>

#include "opencv/cv.h"
#include "opencv2/videoio.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/video/background_segm.hpp"

const int MINARGS = 2;

enum retvals {
  RET_OK = 0,
  RET_NEARGS = 1,
  RET_BADFILE = 2
};

cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2;

int main(const int argc, const char* argv[] ) {

  if( argc < MINARGS + 1 ) {
    std::cerr << "Not enough args!" << std::endl;
    std::cout << "back_sub <infile> <outdir>" << std::endl;
    return RET_NEARGS;
  }

  // storage for frames
  cv::Mat frame;
  cv::Mat fgmask;

  // create actual background subtractor object - history, threshhold, shadow detect
  pMOG2 = cv::createBackgroundSubtractorMOG2();

  // create the object to read in video frames
  cv::VideoCapture capture(argv[1]);
  if( !capture.isOpened() ) {
    std::cerr << "Could not open file for reading: " << argv[1] << std::endl;
    return RET_BADFILE;
  }

  // process!
  while( capture.read(frame) ) {
    // update background model
    pMOG2->apply(frame, fgmask);
    // get frame number
    std::stringstream ss;
    ss << capture.get(cv::CAP_PROP_POS_FRAMES);
    std::string frameNumberString = ss.str();
    // write the images
    cv::imwrite(std::string(argv[2]) + "/" + frameNumberString + ".png", fgmask);
  }

  // clean up when we're done
  capture.release();

  return RET_OK;
}
