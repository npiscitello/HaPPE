// see https://www.pyimagesearch.com/2015/05/25/basic-motion-detection-and-tracking-with-python-and-opencv/

#include <iostream>

#include "opencv/cv.h"
#include "opencv2/videoio.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"

const int MINARGS = 2;

enum retvals {
  RET_OK = 0,
  RET_NEARGS = 1,
  RET_BADFILE = 2
};

cv::Mat first_frame;

cv::Mat backsub(cv::Mat frame);
cv::Mat capture_frame(cv::VideoCapture capture_obj);

int main(const int argc, const char* argv[] ) {

  if( argc < MINARGS + 1 ) {
    std::cerr << "Not enough args!" << std::endl;
    std::cout << "back_sub <infile> <outdir>" << std::endl;
    return RET_NEARGS;
  }

  // storage for frames
  cv::Mat current_frame;
  cv::Mat fgmask;

  // create the object to read in video frames
  cv::VideoCapture capture(argv[1]);
  if( !capture.isOpened() ) {
    std::cerr << "Could not open file for reading: " << argv[1] << std::endl;
    return RET_BADFILE;
  }

  // grab the first frame to serve as the background
  first_frame = capture_frame(capture);
  cv::imwrite(std::string(argv[2]) + "/1.png", first_frame);

  // process!
  while(1) {
    // grab a frame
    current_frame = capture_frame(capture);
    if( current_frame.empty() ) {
      break;
    }
    // update background model
    //pMOG2->apply(frame, fgmask);
    fgmask = backsub(current_frame);
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

// will return an empty Mat if it couldn't get a frame
cv::Mat capture_frame(cv::VideoCapture capture_obj) {
  cv::Mat retframe;
  if( !capture_obj.read(retframe) ) {
    return retframe;
  }
  // we don't care about color
  cv::cvtColor(retframe, retframe, CV_BGR2GRAY);
  // we need to filter out high freq noise
  cv::GaussianBlur(retframe, retframe, cv::Size(25,25), 0);

  return retframe;
}

cv::Mat backsub(cv::Mat frame) {
  cv::Mat retframe;
  // do the actual comparison
  cv::absdiff(first_frame, frame, retframe);
  // threshhold
  cv::threshold(retframe, retframe, 75, 255, cv::THRESH_BINARY);
  // make BLOBs
  cv::morphologyEx(retframe, retframe, cv::MORPH_DILATE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(25,50)));

  return retframe;
}
