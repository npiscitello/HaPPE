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

// how many pieces of PPE each person requires
#define PPE_PER_PERSON 2

// how many consecutive frames the system must be in 
// an alarm state for the alarm to actually go off
#define ALARM_THRESH 8

// values for alarm control function
#define ALARM_TRIGGER 1
#define ALARM_RESET 0

const int MINARGS = 0;

enum retvals {
  RET_OK = 0,
  RET_NEARGS = 1,
  RET_BADFILE = 2
};

cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2;
cv::Ptr<cv::SimpleBlobDetector> m_detector;
cv::Ptr<cv::SimpleBlobDetector> p_detector;

void control_alarm( uint8_t state );

int main(const int argc, const char* argv[] ) {

  if( argc < MINARGS + 1 ) {
    printf("Not enough args!\n");
    printf("demo <infile> <frames output folder>\n");
    printf("Full paths (no '..', '~', etc.) required\n");
    return RET_NEARGS;
  }

  // state storage for frames and blob data
  cv::Mat inframe;
  cv::Mat m_fgmask;
  cv::Mat m_blurframe;
  cv::Mat p_thresh;
  cv::Mat p_blurframe;
  cv::Mat result;
  std::vector<cv::KeyPoint> m_keypoints;
  std::vector<cv::KeyPoint> p_keypoints;
  uint16_t alarm_frame_count = 0;

  /*
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
  fprintf(metadata, "frame_number,moving_body_count,ppe_count,alarm\n");
  */

  // create actual background subtractor object - history, threshhold, shadow detect
  pMOG2 = cv::createBackgroundSubtractorMOG2(20, 2000, false);

  // filter size
  cv::Size m_filter_size = cv::Size(50,75);
  cv::Size p_filter_size = cv::Size(25,25);

  // set up motion blob detection object
  cv::SimpleBlobDetector::Params params;
  params.filterByColor = true;
  params.blobColor = 255;
  params.filterByCircularity = false;
  params.filterByInertia = false;
  params.filterByArea = true;
  params.minArea = 10000;
  params.maxArea = 50000;
  params.filterByConvexity = false;
  m_detector = cv::SimpleBlobDetector::create(params);

  // set up threshhold blob detection object
  params.filterByColor = true;
  params.blobColor = 255;
  params.filterByCircularity = false;
  params.filterByInertia = false;
  params.filterByArea = true;
  params.minArea = 250;
  params.maxArea = 1500;
  params.filterByConvexity = false;
  p_detector = cv::SimpleBlobDetector::create(params);

  // define min and max color threshhold
  cv::Scalar min_color = cv::Scalar(200,125,125);
  cv::Scalar max_color = cv::Scalar(255,200,200);
  // color to copy in for the color threshholded values
  cv::Scalar indication_color = cv::Scalar(0,255,0);
  cv::Mat indication_img;

  // create the object to read in video frames
  //cv::VideoCapture capture(argv[1]);
  cv::VideoCapture capture(0);
  if( !capture.isOpened() ) {
    printf("Could not open device for reading\n");
    return RET_BADFILE;
  }

  capture.set(cv::CAP_PROP_FRAME_WIDTH, 480);
  capture.set(cv::CAP_PROP_FRAME_HEIGHT, 270);

  // set up first run stuff
  capture.read(inframe);
  indication_img.create(inframe.rows, inframe.cols, inframe.type());
  indication_img.setTo(indication_color);

  printf("Set up capturing!\n");

  // process!
  do {
    result = inframe;
    /*
    // get frame number
    std::stringstream ss;
    ss << capture.get(cv::CAP_PROP_POS_FRAMES);
    std::string frameNumberString = ss.str();
    */

    /***********************************
     * Human Body Detection
     ***********************************/
    printf("Starting motion detection...\n");
    // update background model
    pMOG2->apply(inframe, m_fgmask);
    // filter the image to make coherent blobs
    cv::morphologyEx(m_fgmask, m_blurframe, cv::MORPH_DILATE, cv::getStructuringElement(cv::MORPH_ELLIPSE, m_filter_size));
    // find blobs
    m_detector->detect(m_blurframe, m_keypoints);
    // draw blobs on video
    cv::drawKeypoints(  m_fgmask, 
                        m_keypoints, 
                        m_fgmask, 
                        cv::Scalar(0,0,255), 
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::drawKeypoints(  m_blurframe, 
                        m_keypoints, 
                        m_blurframe, 
                        cv::Scalar(0,0,255), 
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    /***********************************
     * PPE Detection
     ***********************************/
    printf("Starting PPE detection...\n");
    // threshhold for color
    cv::inRange(inframe, min_color, max_color, p_thresh);
    // filter the image to make coherent blobs
    cv::morphologyEx(p_thresh, p_blurframe, cv::MORPH_DILATE, cv::getStructuringElement(cv::MORPH_ELLIPSE, p_filter_size));
    // find blobs
    p_detector->detect(p_blurframe, p_keypoints);

    // prepare result frame
    cv::drawKeypoints(  inframe, 
                        m_keypoints, 
                        result, 
                        cv::Scalar(0,0,255), 
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::drawKeypoints(  result, 
                        p_keypoints, 
                        result, 
                        cv::Scalar(0,255,0), 
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    /*
    // write the images
    cv::imwrite(folder_path + "/m_fgmask/" + frameNumberString + ".png", m_fgmask);
    cv::imwrite(folder_path + "/m_blurframe/" + frameNumberString + ".png", m_blurframe);
    cv::imwrite(folder_path + "/p_thresh/" + frameNumberString + ".png", p_thresh);
    cv::imwrite(folder_path + "/p_blurframe/" + frameNumberString + ".png", p_blurframe);
    cv::imwrite(folder_path + "/result/" + frameNumberString + ".png", result);
    */

    // increment or reset frame count
    if( m_keypoints.size() * PPE_PER_PERSON >= p_keypoints.size() ) {
      alarm_frame_count++;
    } else {
      alarm_frame_count = 0;
    }

    // do we need to trigger the alarm?
    if( alarm_frame_count >= ALARM_THRESH ) {
      control_alarm( ALARM_TRIGGER );
    } else {
      control_alarm( ALARM_RESET );
    }

    /*
    // log data
    // frame number, moving body count, ppe count
    fprintf(metadata, "%s,%zu,%zu,%d\n",
        frameNumberString.c_str(),
        m_keypoints.size(),
        p_keypoints.size(),
        m_keypoints.size() * PPE_PER_PERSON > p_keypoints.size());
    */

    printf("Processed a frame!\n");
  } while( capture.read(inframe) );

  // clean up when we're done
  capture.release();
  //fclose(metadata);

  return RET_OK;
}

// trigger or reset alarm
void control_alarm( uint8_t state ) {
  switch( state ) {
    case ALARM_RESET:
      break;
    case ALARM_TRIGGER:
      break;
    default:
      break;
  }
  return;
}

