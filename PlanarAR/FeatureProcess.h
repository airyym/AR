#ifndef FEATUREPROCESS_H
#define FEATUREPROCESS_H

#include <iostream>
#include <windows.h>
#include <GL/gl.h>
#include "glut.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/video/video.hpp"
#include "PoseEstimation.h"
#include "debugfunc.h"
#include "SFMUtil.h"

using std::cout;
using std::endl;
using cv::imshow;
using cv::waitKey;
using cv::imwrite;

/*	Matching method : Flann(OpenCV)	*/
void FlannMatching(cv::Mat &descriptors1, cv::Mat &descriptors2, std::vector<cv::DMatch> &matches);

/* FeatureMap(In database) */
bool FeatureDetection(FeatureMap &featureMap, unsigned int minHessian);

/* Any scenes */
bool FeatureDetection(FrameMetaData &currData, cv::Mat &currFrameMat, unsigned int minHessian);

/*	Homography	*/
bool FeatureMatching(FeatureMap &featureMap, FrameMetaData &currFrame, cv::Mat &currFrameMat, cv::Mat &prevFrameMat, std::vector<cv::Point2f> &featureMapGoodMatches, std::vector<cv::Point2f> &frameGoodMatches, std::vector<cv::Point2f> &prevFeatureMapGoodMatches, std::vector<cv::Point2f> &prevFrameGoodMatches);

/*	PnPRansac	*/
bool FeatureMatching(std::vector<SFM_Feature> &SFM_Features, std::vector<KeyFrame> &keyFrames, FrameMetaData &currData, cv::Mat &currFrameMat, std::vector<int> &neighboringKeyFrameIdx, std::vector< std::vector<cv::DMatch> > &goodMatchesSet);

/*	Triangulation	*/
void FeatureMatching(KeyFrame &query, KeyFrame &train, std::vector<cv::DMatch> &goodMatches);


#endif