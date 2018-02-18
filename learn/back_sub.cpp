#include <iostream>

#include "opencv/cv.h"
#include "opencv2/videoio.hpp"
#include "opencv2/video.hpp"

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
    std::cout << "back_sub <infile> <outfile>" << std::endl;
    return RET_NEARGS;
  }

  // storage for frames
  cv::Mat frame;
  cv::Mat fgmask;

  // create actual background subtractor object - history, threshhold, shadow detect
  pMOG2 = cv::createBackgroundSubtractorMOG2(500, 16, false);

  // create the object to read in/write out video frames
  cv::VideoCapture capture(argv[1]);
  if( !capture.isOpened() ) {
    std::cerr << "Could not open file for reading: " << argv[1] << std::endl;
    return RET_BADFILE;
  }

  // grab a frame to know what we're dealing with and create the video writer
  capture.read(frame);
  cv::VideoWriter writer(argv[2], cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 10, frame.size());
  if( !writer.isOpened() ) {
    std::cerr << "Could not open file for writing: " << argv[2] << std::endl;
    return RET_BADFILE;
  }

  // process!
  do {
    // update background model
    pMOG2->apply(frame, fgmask);

    // annotate with frame number
    std::stringstream ss;
    rectangle(fgmask, cv::Point(10, 2), cv::Point(50,20), cv::Scalar(255,255,255), -1);
    ss << capture.get(cv::CAP_PROP_POS_FRAMES);
    std::string frameNumberString = ss.str();
    putText(fgmask, frameNumberString.c_str(), cv::Point(15, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));

    // write it to a new video
    writer.write(fgmask);
  } while( capture.read(frame) );

  // clean up when we're done
  capture.release();
  writer.release();

  return RET_OK;
}
