#include <iostream>

#include "opencv/cv.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define RET_OK 0
#define RET_NEARGS 1

int main(const int argc, const char* argv[] ) {

  const char* color_im_name = argv[1];
  const char* grey_im_name = argv[2];
  cv::Mat color_image = cv::imread(color_im_name, cv::IMREAD_COLOR);

  if( argc < 3 || !color_image.data ) {
    std::cout << "Not enough args!" << std::endl;
    std::cout << "greyscale <color image input> <greyscale image output>" << std::endl;
    return RET_NEARGS;
  }

  cv::Mat grey_image;
  cv::cvtColor(color_image, grey_image, CV_BGR2GRAY);

  cv::imwrite( grey_im_name, grey_image);

  return RET_OK;
}
