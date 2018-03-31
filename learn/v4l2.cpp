#include <iostream>

#include "opencv/cv.h"
#include "opencv2/videoio.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define RET_OK 0
#define RET_NEARGS 1
#define RET_NOCAM 2

int main(const int argc, const char* argv[] ) {

  const char* grey_im_name = argv[1];
  cv::Mat color_image;

  cv::VideoCapture cap(0);
  if( !cap.isOpened() ) {
    return RET_NOCAM;
  }

  cap.set(cv::CAP_PROP_FRAME_WIDTH, 480);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, 270);

  cap.read(color_image);

  if( argc < 2 || !color_image.data ) {
    std::cout << "Not enough args!" << std::endl;
    std::cout << "greyscale <greyscale image output>" << std::endl;
    return RET_NEARGS;
  }

  cv::Mat grey_image;
  cv::cvtColor(color_image, grey_image, CV_BGR2GRAY);

  cv::imwrite( grey_im_name, grey_image);

  return RET_OK;
}
