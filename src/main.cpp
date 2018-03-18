#include <iostream>
#include <sys/stat.h>
#include <string>

#include "opencv/cv.h"

#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"

#include "opencv2/videoio.hpp"
#include "opencv2/video/background_segm.hpp"

const int MINARGS = 2;

enum retvals {
  RET_OK = 0,
  RET_NEARGS = 1,
  RET_BADFILE = 2
};

cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2;
cv::Ptr<cv::SimpleBlobDetector> detector;

int main(const int argc, const char* argv[] ) {

  if( argc < MINARGS + 1 ) {
    std::cerr << "Not enough args!" << std::endl;
    std::cout << "demo <infile> <outfile>" << std::endl;
    return RET_NEARGS;
  }

  // storage for frames and blob data
  cv::Mat inframe;
  cv::Mat fgmask;
  cv::Mat blurframe;
  cv::Mat thresh;
  cv::Mat result;
  std::vector<cv::KeyPoint> keypoints;

  // temp storage for intermediate images
  std::string folder_path = std::string(argv[2]) + "_frames";
  mkdir(folder_path.c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "fgmask").c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "blurframe").c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "thresh").c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "result").c_str(), S_IRWXU | S_IRWXG);

  // create actual background subtractor object - history, threshhold, shadow detect
  pMOG2 = cv::createBackgroundSubtractorMOG2(10000, 1500, false);
  // set up blob detection object
  cv::SimpleBlobDetector::Params params;
  params.filterByColor = true;
  params.blobColor = 255;
  params.filterByCircularity = false;
  params.filterByInertia = false;
  params.filterByArea = true;
  params.minArea = 3000;
  params.maxArea = 50000;
  params.filterByConvexity = false;
  detector = cv::SimpleBlobDetector::create(params);
  // define min and max color threshhold
  cv::Scalar min_color = cv::Scalar(50,0,150);
  cv::Scalar max_color = cv::Scalar(100,50,255);
  // color to copy in for the color threshholded values
  cv::Scalar indication_color = cv::Scalar(0,255,0);
  cv::Mat indication_img;

  // create the object to read in video frames
  cv::VideoCapture capture(argv[1]);
  if( !capture.isOpened() ) {
    std::cerr << "Could not open file for reading: " << argv[1] << std::endl;
    return RET_BADFILE;
  }

  // set up first run stuff
  capture.read(inframe);
  indication_img.create(inframe.rows, inframe.cols, inframe.type());
  indication_img.setTo(indication_color);

  // process!
  do {
    result = inframe;
    // get frame number
    std::stringstream ss;
    ss << capture.get(cv::CAP_PROP_POS_FRAMES);
    std::string frameNumberString = ss.str();

    // threshhold for color
    cv::inRange(inframe, min_color, max_color, thresh);

    // update background model
    pMOG2->apply(inframe, fgmask);
    // filter the image to make coherent blobs
    cv::morphologyEx(fgmask, blurframe, cv::MORPH_DILATE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(20,20)));
    // find blobs
    detector->detect(blurframe, keypoints);
    // draw blobs on video
    cv::drawKeypoints(  fgmask, 
                        keypoints, 
                        fgmask, 
                        cv::Scalar(0,0,255), 
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::drawKeypoints(  blurframe, 
                        keypoints, 
                        blurframe, 
                        cv::Scalar(0,0,255), 
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    // prepare result frame
    cv::drawKeypoints(  inframe, 
                        keypoints, 
                        result, 
                        cv::Scalar(0,0,255), 
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    indication_img.copyTo(result, thresh);

    // write the images
    cv::imwrite(folder_path + "/fgmask/" + frameNumberString + ".png", fgmask);
    cv::imwrite(folder_path + "/blurframe/" + frameNumberString + ".png", blurframe);
    cv::imwrite(folder_path + "/thresh/" + frameNumberString + ".png", thresh);
    cv::imwrite(folder_path + "/result/" + frameNumberString + ".png", result);
  } while( capture.read(inframe) );

  // clean up when we're done
  capture.release();

  return RET_OK;
}
