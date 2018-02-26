#include <iostream>

#include "opencv/cv.h"
#include "opencv2/videoio.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"

const int MINARGS = 2;

enum retvals {
  RET_OK = 0,
  RET_NEARGS = 1,
  RET_BADFILE = 2
};

cv::Ptr<cv::SimpleBlobDetector> detector;

int main(const int argc, const char* argv[] ) {

  if( argc < MINARGS + 1 ) {
    std::cerr << "Not enough args!" << std::endl;
    std::cout << "back_sub <infile> <outdir>" << std::endl;
    return RET_NEARGS;
  }

  // storage for frames and blob data
  cv::Mat inframe;
  cv::Mat blurframe;
  cv::Mat blobframe;
  std::vector<cv::KeyPoint> keypoints;

  // set up blob detection object
  cv::SimpleBlobDetector::Params params;
  params.filterByColor = true;
  params.blobColor = 255;
  params.filterByCircularity = false;
  params.filterByInertia = false;
  params.filterByArea = true;
  params.minArea = 2000;
  params.maxArea = 50000;
  params.filterByConvexity = false;
  detector = cv::SimpleBlobDetector::create(params);

  // read in video frames
  cv::VideoCapture capture(argv[1]);
  if( !capture.isOpened() ) {
    std::cerr << "Could not open file for reading: " << argv[1] << std::endl;
    return RET_BADFILE;
  }


  // process!
  while( capture.read(inframe) ) {
    // filter the image to make coherent blobs
    //cv::GaussianBlur(inframe, blurframe, cv::Size(9,9), 3);
    cv::morphologyEx(inframe, blurframe, cv::MORPH_DILATE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(20,20)));
    detector->detect(blurframe, keypoints);
    blobframe = inframe;
    cv::drawKeypoints(  blurframe, 
                        keypoints, 
                        blobframe, 
                        cv::Scalar(0,0,255), 
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    // get frame number
    std::stringstream ss;
    ss << capture.get(cv::CAP_PROP_POS_FRAMES);
    std::string frameNumberString = ss.str();

    // save frame
    cv::imwrite(std::string(argv[2]) + "/" + frameNumberString + ".png", blobframe);
  }

  capture.release();

  return RET_OK;
}
