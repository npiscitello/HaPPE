#include <iostream>

#include "opencv/cv.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

int main(int argc, char* argv[]) {

  cv::Mat orig = cv::imread( argv[1], cv::IMREAD_COLOR );

  if( argc < 3 ) {
    std::cerr << "Not enough args!" << std::endl;
    std::cout << "thresh <in img> <out img>" << std::endl;
    return 1;
  }

  cv::Mat thresh;
  cv::inRange(orig, cv::Scalar(0, 0, 175), cv::Scalar(75, 75, 255), thresh);

  cv::imwrite(argv[2], thresh);

  return 0;
}
