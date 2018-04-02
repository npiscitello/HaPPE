#include <stdio.h>
#include <sys/stat.h>
#include <string>
#include <errno.h>

#include "opencv/cv.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/videoio.hpp"

// define this if you want to build for the pi
//#define CROSS

// Alarm related macros
// how many pieces of PPE each person requires
#define PPE_PER_PERSON 2
// how many consecutive frames the system must be in 
// a triggered state for the alarm to actually go off
#define ALARM_THRESH 4
// values for alarm control function
#define ALARM_TRIGGER 1
#define ALARM_RESET 0

// how many consecutive frames there must be no motion
// to update the background frame
#define BACKGROUND_THRESH 3

#ifndef CROSS
const int MINARGS = 2;
#else
const int MINARGS = 1;
uint16_t frame_number = 1;
#endif

enum retvals {
  RET_OK = 0,
  RET_NEARGS = 1,
  RET_BADFILE = 2
};

cv::Ptr<cv::SimpleBlobDetector> m_detector;
cv::Ptr<cv::SimpleBlobDetector> p_detector;

cv::Mat background_frame;

cv::Mat backsub(cv::Mat frame);
cv::Mat filter_frame(cv::Mat frame);
void control_alarm( uint8_t state );

int main(const int argc, const char* argv[] ) {

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
  uint16_t background_frame_count = 0;

  if( argc < MINARGS + 1 ) {
    printf("Not enough args!\n");
    printf("demo <frames output folder> [infile, if not on Pi]\n");
    printf("Full paths (no '..', '~', etc.) required\n");
    return RET_NEARGS;
  }

  // storage for intermediate images and metadata
  std::string folder_path = std::string(argv[1]);
  mkdir(folder_path.c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "result").c_str(), S_IRWXU | S_IRWXG);
#ifndef CROSS
  mkdir((folder_path + "/" + "m_fgmask").c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "m_blurframe").c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "p_thresh").c_str(), S_IRWXU | S_IRWXG);
  mkdir((folder_path + "/" + "p_blurframe").c_str(), S_IRWXU | S_IRWXG);
  FILE* metadata = fopen((folder_path + "/metadata.csv").c_str(), "w+");
  if( metadata == NULL ) {
    printf("Couldn't make file: %s\n", strerror(errno));
    return RET_BADFILE;
  }
  fprintf(metadata, "frame_number,moving_body_count,ppe_count,trigger,alarm\n");
#endif

  // filter sizes
  cv::Size m_filter_size = cv::Size(75,150);
  cv::Size p_filter_size = cv::Size(25,25);

  // define min and max color threshhold
  cv::Scalar min_color = cv::Scalar(225,100,100);
  cv::Scalar max_color = cv::Scalar(255,255,180);
  // color to copy in for the color threshholded values
  cv::Scalar indication_color = cv::Scalar(0,255,0);
  cv::Mat indication_img;

  // set up motion blob detection object
  cv::SimpleBlobDetector::Params params;
  params.filterByColor = true;
  params.blobColor = 255;
  params.filterByCircularity = false;
  params.filterByInertia = false;
  params.filterByArea = true;
  params.minArea = 25000;
  params.maxArea = 100000;
  params.filterByConvexity = false;
  m_detector = cv::SimpleBlobDetector::create(params);

  // set up PPE blob detection object
  params.filterByColor = true;
  params.blobColor = 255;
  params.filterByCircularity = false;
  params.filterByInertia = false;
  params.filterByArea = false;
  params.minArea = 250;
  params.maxArea = 2500;
  params.filterByConvexity = false;
  p_detector = cv::SimpleBlobDetector::create(params);

  // create the object to read in video frames
#ifndef CROSS
  cv::VideoCapture capture(argv[2]);
#else
  cv::VideoCapture capture(0);
#endif
  if( !capture.isOpened() ) {
    printf("Could not open device for reading\n");
    return RET_BADFILE;
  }

#ifdef CROSS
  capture.set(cv::CAP_PROP_FRAME_WIDTH, 480);
  capture.set(cv::CAP_PROP_FRAME_HEIGHT, 270);
#endif

  // set up first run stuff
  capture.read(background_frame);
  indication_img.create(background_frame.rows, background_frame.cols, background_frame.type());
  indication_img.setTo(indication_color);
  background_frame = filter_frame(background_frame);
  indication_img.create(background_frame.rows, background_frame.cols, background_frame.type());
  indication_img.setTo(indication_color);

  // process!
  while(capture.read(inframe)) {

    /***********************************
     * Human Body Detection
     ***********************************/
#ifdef CROSS
    printf("Starting motion detection...\n");
#endif
    // subtract the background
    m_fgmask = backsub(inframe);
    // filter the image to make coherent blobs
    cv::morphologyEx(m_fgmask, m_blurframe, cv::MORPH_DILATE, cv::getStructuringElement(cv::MORPH_ELLIPSE, m_filter_size));
    // find blobs
    m_detector->detect(m_blurframe, m_keypoints);

    /***********************************
     * PPE Detection
     ***********************************/
#ifdef CROSS
    printf("Starting PPE detection...\n");
#endif
    // threshhold for color
    cv::inRange(inframe, min_color, max_color, p_thresh);
    // filter the image to make coherent blobs
    cv::morphologyEx(p_thresh, p_blurframe, cv::MORPH_DILATE, cv::getStructuringElement(cv::MORPH_ELLIPSE, p_filter_size));
    // find blobs
    p_detector->detect(p_blurframe, p_keypoints);

    /***********************************
     * Alarm Handling
     ***********************************/
    // increment or reset frame count
    if( m_keypoints.size() * PPE_PER_PERSON > p_keypoints.size() ) {
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

    /***********************************
     * Background Management
     ***********************************/
    // increment or reset frame count
    if( m_keypoints.size() == 0 ) {
      background_frame_count++;
    } else {
      background_frame_count = 0;
    }
    // do we need to update the background?
    if( background_frame_count >= BACKGROUND_THRESH ) {
      background_frame = filter_frame(inframe);
    }

    /***********************************
     * Image and Logging Output
     ***********************************/
    std::stringstream ss;
#ifndef CROSS
    // get frame number
    ss << capture.get(cv::CAP_PROP_POS_FRAMES);
#else
    ss << frame_number++;
#endif
    std::string frameNumberString = ss.str();
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
    cv::imwrite(folder_path + "/result/" + frameNumberString + ".png", result);

#ifndef CROSS
    // log extra stuff, output extra images
    // log data
    // frame number, moving body count, ppe count
    fprintf(metadata, "%s,%zu,%zu,%d,%d\n",
        frameNumberString.c_str(),
        m_keypoints.size(),
        p_keypoints.size(),
        m_keypoints.size() * PPE_PER_PERSON > p_keypoints.size(),
        alarm_frame_count >= ALARM_THRESH );
    // draw motion blobs on video
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
    // write the images
    cv::imwrite(folder_path + "/m_fgmask/" + frameNumberString + ".png", m_fgmask);
    cv::imwrite(folder_path + "/m_blurframe/" + frameNumberString + ".png", m_blurframe);
    cv::imwrite(folder_path + "/p_thresh/" + frameNumberString + ".png", p_thresh);
    cv::imwrite(folder_path + "/p_blurframe/" + frameNumberString + ".png", p_blurframe);
#endif

#ifdef CROSS
    printf("Processed a frame!\n");
#endif
  }

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

// will return an empty Mat if it couldn't get a frame
cv::Mat filter_frame(cv::Mat frame) {
  cv::Mat retframe;
  // we don't care about color
  cv::cvtColor(frame, retframe, CV_BGR2GRAY);
  // we need to filter out high freq noise
  cv::GaussianBlur(retframe, retframe, cv::Size(25,25), 0);
  return retframe;
}

cv::Mat backsub(cv::Mat frame) {
  cv::Mat retframe = filter_frame(frame);
  // do the actual comparison
  cv::absdiff(background_frame, retframe, retframe);
  // threshhold
  cv::threshold(retframe, retframe, 75, 255, cv::THRESH_BINARY);
  return retframe;
}
