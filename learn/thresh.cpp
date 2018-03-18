#include <iostream>
#include <sys/stat.h>

#include "opencv/cv.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

#include "opencv2/videoio.hpp"

int main(int argc, char* argv[]) {

  if( argc < 3 ) {
    std::cerr << "Not enough args!" << std::endl;
    std::cout << "thresh <in img> <out img>" << std::endl;
    return 1;
  }

  // temp storage for intermediate images
  std::string folder_path = std::string(argv[2]) + "_frames";
  mkdir(folder_path.c_str(), S_IRWXU | S_IRWXG);

  // create the object to read in video frames
  cv::VideoCapture capture(argv[1]);
  if( !capture.isOpened() ) {
    std::cerr << "Could not open file for reading: " << argv[1] << std::endl;
    return 1;
  }

  cv::Mat inframe;
  cv::Mat thresh;
  while( capture.read(inframe) ) {
    cv::inRange(inframe, cv::Scalar(50, 0, 150), cv::Scalar(100, 50, 255), thresh);

    // get frame number
    std::stringstream ss;
    ss << capture.get(cv::CAP_PROP_POS_FRAMES);
    std::string frameNumberString = ss.str();
    // write the images
    cv::imwrite(folder_path + "/" + frameNumberString + ".png", thresh);
  }

  cv::imwrite(argv[2], thresh);

  return 0;
}
