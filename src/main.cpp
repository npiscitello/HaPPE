#include <stdio.h>
#include <sys/stat.h>
#include <string>
#include <errno.h>

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
    printf("Not enough args!\n");
    printf("demo <infile> <frames output folder>\n");
    printf("Full paths (no '..', '~', etc.) required\n");
    return RET_NEARGS;
  }

  // storage for frames and blob data
  cv::Mat inframe;
  cv::Mat m_fgmask;
  cv::Mat m_blurframe;
  cv::Mat p_thresh;
  cv::Mat p_blurframe;
  cv::Mat result;
  std::vector<cv::KeyPoint> keypoints;

  // storage for intermediate images and metadata
  std::string folder_path = std::string(argv[2]);
  mkdir(folder_path.c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "m_fgmask").c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "m_blurframe").c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "p_thresh").c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "p_blurframe").c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "result").c_str(), S_IRWXU | S_IRWXG);
  FILE* metadata = fopen((folder_path + "/metadata.csv").c_str(), "w+");
  if( metadata == NULL ) {
    printf("Couldn't make file: %s\n", strerror(errno));
    return RET_BADFILE;
  }
  fprintf(metadata, "frame number,moving body count,ppe count\n");

  // create actual background subtractor object - history, threshhold, shadow detect
  pMOG2 = cv::createBackgroundSubtractorMOG2(20, 2000, false);

  // filter size
  cv::Size m_filter_size = cv::Size(50,75);
  cv::Size p_filter_size = cv::Size(25,25);

  // set up blob detection object
  cv::SimpleBlobDetector::Params params;
  params.filterByColor = true;
  params.blobColor = 255;
  params.filterByCircularity = false;
  params.filterByInertia = false;
  params.filterByArea = true;
  params.minArea = 5000;
  params.maxArea = 50000;
  params.filterByConvexity = false;
  detector = cv::SimpleBlobDetector::create(params);

  // define min and max color threshhold
  //cv::Scalar min_color = cv::Scalar(50,0,150);
  //cv::Scalar max_color = cv::Scalar(150,100,255);
  cv::Scalar min_color = cv::Scalar(56,37,161);
  cv::Scalar max_color = cv::Scalar(100,75,255);
  // color to copy in for the color threshholded values
  cv::Scalar indication_color = cv::Scalar(0,255,0);
  cv::Mat indication_img;

  // create the object to read in video frames
  cv::VideoCapture capture(argv[1]);
  if( !capture.isOpened() ) {
    printf("Could not open file for reading: %s", argv[1]);
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

    /***********************************
     * Human Body Detection
     ***********************************/
    // update background model
    pMOG2->apply(inframe, m_fgmask);
    // filter the image to make coherent blobs
    cv::morphologyEx(m_fgmask, m_blurframe, cv::MORPH_DILATE, cv::getStructuringElement(cv::MORPH_ELLIPSE, m_filter_size));
    // find blobs
    detector->detect(m_blurframe, keypoints);
    // draw blobs on video
    cv::drawKeypoints(  m_fgmask, 
                        keypoints, 
                        m_fgmask, 
                        cv::Scalar(0,0,255), 
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::drawKeypoints(  m_blurframe, 
                        keypoints, 
                        m_blurframe, 
                        cv::Scalar(0,0,255), 
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    /***********************************
     * PPE Detection
     ***********************************/
    // threshhold for color
    cv::inRange(inframe, min_color, max_color, p_thresh);
    // filter the image to make coherent blobs
    cv::morphologyEx(p_thresh, p_blurframe, cv::MORPH_DILATE, cv::getStructuringElement(cv::MORPH_ELLIPSE, p_filter_size));

    // prepare result frame
    cv::drawKeypoints(  inframe, 
                        keypoints, 
                        result, 
                        cv::Scalar(0,0,255), 
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    indication_img.copyTo(result, p_blurframe);

    // write the images
    cv::imwrite(folder_path + "/m_fgmask/" + frameNumberString + ".png", m_fgmask);
    cv::imwrite(folder_path + "/m_blurframe/" + frameNumberString + ".png", m_blurframe);
    cv::imwrite(folder_path + "/p_thresh/" + frameNumberString + ".png", p_thresh);
    cv::imwrite(folder_path + "/p_blurframe/" + frameNumberString + ".png", p_blurframe);
    cv::imwrite(folder_path + "/result/" + frameNumberString + ".png", result);

    // frame number, moving body count, ppe count
    fprintf(metadata, "%s,%zu,%d\n",
        frameNumberString.c_str(),
        keypoints.size(),
        0);
  } while( capture.read(inframe) );

  // clean up when we're done
  capture.release();
  fclose(metadata);

  return RET_OK;
}
