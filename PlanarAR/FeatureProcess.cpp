#include "FeatureProcess.h"

void FindGoodMatches(FeatureMap &featureMap, Frame &frame, std::vector<cv::KeyPoint> &keypoints_frame, std::vector<cv::DMatch> &matches, std::vector<cv::Point2f> &featureMapGoodMatches, std::vector<cv::Point2f> &frameGoodMatches)
{
	//This method is to delete the feature points which are multimatch
	
	std::vector<cv::DMatch> goodMatches;
	//	Quick calcul ation of max and min distances between keypoints
	double max_dist = 0; double min_dist = 100;
	for (std::size_t i = 0; i < matches.size(); i++)
	{
		double dist = (double)(matches[i].distance);
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}
	cout << "(max, min) : (" << max_dist << ", " << min_dist << ")\n\n";
	//DebugOpenCVMarkPoint(frame, keypoints_frame, "inputFrame.jpg");
	//	Find the good matches
	min_dist *= 2.0;
	if (min_dist > 0.2)
		min_dist = 0.2;
	for (std::size_t i = 0; i < matches.size(); i++)
	{
		double dist = (double)(matches[i].distance);
		if (dist < min_dist)
			goodMatches.push_back(matches[i]);
	}
	//標記一對多的點為false-> goodMatches
	std::vector<bool> GoodMatchesFlag(goodMatches.size(), true);	//判斷Surf算出來的點
	
	for (std::size_t j = 0; j < goodMatches.size(); j++)
	{
		std::size_t index = j;	//紀錄distance較小的index
		double distance = goodMatches[index].distance;
		if (!GoodMatchesFlag[index])
			continue;
		for (std::size_t k = j + 1; k < goodMatches.size(); k++)
		{
			if (!GoodMatchesFlag[k])
				continue;
			if (goodMatches[index].trainIdx == goodMatches[k].trainIdx)
			{
				if (distance <= goodMatches[k].distance)
					GoodMatchesFlag[k] = false;
				else
				{
					GoodMatchesFlag[index] = false;
					index = k;
					distance = goodMatches[index].distance;
				}
			}
		}
	}
	//DebugOpenCVMatchPoint(featureMap.image, featureMap.keypoints, frame.image, frame.keypoints, goodMatches, "IMG1.JPG");
	//把標記為true的點放入featureMap_goodMatches & frame_goodMatches
	for (std::size_t i = 0; i < goodMatches.size(); i++)
	{
		if (GoodMatchesFlag[i])
		{
			featureMapGoodMatches.push_back(featureMap.keypoints[goodMatches[i].queryIdx].pt);
			frameGoodMatches.push_back(keypoints_frame[goodMatches[i].trainIdx].pt);
		}
	}

	//Release memory
	std::vector<cv::DMatch>().swap(goodMatches);
	std::vector<bool>().swap(GoodMatchesFlag);
}

void FindGoodMatches(KeyFrame &KF1, KeyFrame &KF2, std::vector<cv::DMatch> &matches, std::vector<cv::Point2f> &KF1GoodMatches, std::vector<cv::Point2f> &KF2GoodMatches, std::vector<cv::KeyPoint> keypointsMatches[2], cv::Mat descriptorsMatches[2])
{
	//This method is to triangulate points, and swap good matching keypoints & descriptors
	//For two KeyFrame cases

	//	Quick calcul ation of max and min distances between keypoints
	std::vector<cv::DMatch> goodMatches;
	double max_dist = 0; double min_dist = 100;
	for (std::size_t i = 0; i < matches.size(); i++)
	{
		double dist = (double)(matches[i].distance);
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	//	Find the good matches
	min_dist *= 2.0;
	if (min_dist > 0.2) min_dist = 0.2;
	for (std::size_t i = 0; i < matches.size(); i++)
	{
		double dist = (double)(matches[i].distance);
		if (dist < min_dist)
			goodMatches.push_back(matches[i]);
	}

	//標記一對多的點為false-> goodMatches
	std::vector<bool> GoodMatchesFlag(goodMatches.size(), true);
	for (std::size_t j = 0; j < goodMatches.size(); j++)
	{
		std::size_t index = j;	//紀錄distance較小的index
		double distance = goodMatches[index].distance;
		if (!GoodMatchesFlag[index])
			continue;
		for (std::size_t k = j + 1; k < goodMatches.size(); k++)
		{
			if (!GoodMatchesFlag[k])
				continue;
			if (goodMatches[index].trainIdx == goodMatches[k].trainIdx)
			{
				if (distance <= goodMatches[k].distance)
					GoodMatchesFlag[k] = false;
				else
				{
					GoodMatchesFlag[index] = false;
					index = k;
					distance = goodMatches[index].distance;
				}
			}
		}
	}
	cv::Mat tempdescriptors1(goodMatches.size(), KF1.descriptors.cols, KF1.descriptors.type());
	cv::Mat tempdescriptors2(goodMatches.size(), KF2.descriptors.cols, KF2.descriptors.type());
	//std::vector<cv::KeyPoint> tempkeypoints;
	//std::vector<cv::DMatch> tempMatch;

	for (std::size_t i = 0, j = 0; i < goodMatches.size(); ++i)
	{
		if (GoodMatchesFlag[i])
		{
			KF1GoodMatches.push_back(KF1.keypoints[goodMatches[i].queryIdx].pt);
			KF2GoodMatches.push_back(KF2.keypoints[goodMatches[i].trainIdx].pt);
			keypointsMatches[0].push_back(KF1.keypoints[goodMatches[i].queryIdx]);
			keypointsMatches[1].push_back(KF2.keypoints[goodMatches[i].trainIdx]);
			KF1.descriptors.row(goodMatches[i].queryIdx).copyTo(tempdescriptors1.row(j));
			KF2.descriptors.row(goodMatches[i].trainIdx).copyTo(tempdescriptors2.row(j));

			//tempMatch.push_back(goodMatches[i]);
			//tempkeypoints.push_back(img1.keypoints[goodMatches[i].queryIdx]);
			//tempMatch.push_back(goodMatches[i]);
			//tempMatch[j].queryIdx = j;
			//tempMatch[j].trainIdx = j;
			++j;
		}
	}

	descriptorsMatches[0].create(keypointsMatches[0].size(), KF1.descriptors.cols, KF1.descriptors.type());
	descriptorsMatches[1].create(keypointsMatches[1].size(), KF2.descriptors.cols, KF2.descriptors.type());
	for (std::size_t i = 0; i < keypointsMatches[1].size(); ++i)
	{
		tempdescriptors1.row(i).copyTo(descriptorsMatches[0].row(i));
		tempdescriptors2.row(i).copyTo(descriptorsMatches[1].row(i));
	}
#ifdef SHOWTHEIMAGE
	DebugOpenCVMatchPoint(prevKeyFrame.image, tempkeypoints, latestKeyFrame.image, keypoints_3D, tempMatch, "APoints.jpg");
#endif

	//std::vector<cv::KeyPoint>().swap(tempkeypoints);
	std::vector<cv::DMatch>().swap(matches);
	std::vector<cv::DMatch>().swap(goodMatches);
	std::vector<bool>().swap(GoodMatchesFlag);
}

void FindGoodMatches(KeyFrame &prevKeyFrame, KeyFrame &latestKeyFrame, std::vector<cv::DMatch> &matches)
{
	std::vector<cv::DMatch> goodMatches;
	double max_dist = 0; double min_dist = 100;
	for (std::size_t i = 0; i < matches.size(); i++)
	{
		double dist = (double)(matches[i].distance);
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	//	Find the good matches
	min_dist *= 2.0;
	if (min_dist > 0.2) min_dist = 0.2;
	for (std::size_t i = 0; i < matches.size(); i++)
	{
		double dist = (double)(matches[i].distance);
		if (dist < min_dist)
			goodMatches.push_back(matches[i]);
	}

	//標記一對多的點為false-> goodMatches
	std::vector<bool> GoodMatchesFlag(goodMatches.size(), true);
	for (std::size_t j = 0; j < goodMatches.size(); j++)
	{
		std::size_t index = j;	//紀錄distance較小的index
		double distance = goodMatches[index].distance;
		if (!GoodMatchesFlag[index])
			continue;
		for (std::size_t k = j + 1; k < goodMatches.size(); k++)
		{
			if (!GoodMatchesFlag[k])
				continue;
			if (goodMatches[index].trainIdx == goodMatches[k].trainIdx)
			{
				if (distance <= goodMatches[k].distance)
					GoodMatchesFlag[k] = false;
				else
				{
					GoodMatchesFlag[index] = false;
					index = k;
					distance = goodMatches[index].distance;
				}
			}
		}
	}
	matches.clear();
	for (std::size_t i = 0; i < goodMatches.size(); ++i)
	{
		if (GoodMatchesFlag[i])
			matches.push_back(goodMatches[i]);
	}
}

/*void FindGoodMatches(Frame &prevImage, Frame &newImage, std::vector<cv::DMatch> &matches, std::vector<int> &good3DMatches, std::vector<cv::Point2f> &newImage_goodMatches)
{
	//This method is to find 3D points which are good match

	std::vector<cv::DMatch> goodMatches;

	double max_dist = 0; double min_dist = 100;
	for (std::size_t i = 0; i < matches.size(); ++i)
	{
		double dist = (double)(matches[i].distance);
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}
	//	Find the good matches
 	min_dist *= 2.0;
	if (min_dist > 0.2)
		min_dist = 0.2;
	for (std::size_t i = 0; i < matches.size(); i++)
	{
		double dist = (double)(matches[i].distance);
		if (dist < min_dist)
			goodMatches.push_back(matches[i]);
	}

	//標記一對多的點為false-> goodMatches
	std::vector<bool> GoodMatchesFlag(goodMatches.size(), true);	//判斷Surf算出來的點

	for (std::size_t j = 0; j < goodMatches.size(); ++j)
	{
		std::size_t index = j;	//紀錄distance較小的index
		double distance = goodMatches[index].distance;
		if (!GoodMatchesFlag[index]) continue;
		for (std::size_t k = j + 1; k < goodMatches.size(); ++k)
		{
			if (!GoodMatchesFlag[k]) continue;
			if (goodMatches[index].trainIdx == goodMatches[k].trainIdx)
			{
				if (distance <= goodMatches[k].distance) 
					GoodMatchesFlag[k] = false;
				else
				{
					GoodMatchesFlag[index] = false;
					index = k;
					distance = goodMatches[index].distance;
				}
			}
		}
	}
	std::vector<cv::DMatch> temp;
	for (std::size_t i = 0; i < goodMatches.size(); ++i)
	{
		if (GoodMatchesFlag[i])
		{
			temp.push_back(goodMatches[i]);
			good3DMatches.push_back(goodMatches[i].queryIdx);//Query the features index from the previous frame
			newImage_goodMatches.push_back(newImage.keypoints[goodMatches[i].trainIdx].pt);
		}
	}
	DebugOpenCVMarkPoint(prevImage.image, prevImage.keypoints_3D, "KEYPOINT1.JPG");
	DebugOpenCVMarkPoint(newImage.image, newImage.keypoints, "KEYPOINT2.JPG");
	DebugOpenCVMatchPoint(prevImage.image, prevImage.keypoints_3D, newImage.image, newImage.keypoints, temp, "VisualOdometry.JPG");
	std::vector<cv::DMatch>().swap(goodMatches);
	std::vector<bool>().swap(GoodMatchesFlag);
}*/

void DeleteOverlap(std::vector<cv::Point2f> &featureMapGoodMatches, std::vector<cv::Point2f> &frameGoodMatches)
{
	//	把標記為True的點放入GoodMatchedPoints
	//	刪除OpticalFlow和Surf重複的特徵點

	std::vector<cv::Point2f> GoodMatchedPoints[2];	//[0]->featureMap(marker)中沒有重複的點, [1]->frame中沒有重複的點
	std::vector<bool> NotOverlayPointsFlag(featureMapGoodMatches.size(), true);	//標記keypoints x & y一樣的點為False

	for (std::size_t i = 0; i < featureMapGoodMatches.size(); i++)
	{
		if (!NotOverlayPointsFlag[i])
			continue;
		for (std::size_t j = i + 1; j < featureMapGoodMatches.size(); j++)
		{
			float errX = std::abs(frameGoodMatches[i].x - frameGoodMatches[j].x);
			float errY = std::abs(frameGoodMatches[i].y - frameGoodMatches[j].y);
			if (!NotOverlayPointsFlag[j])
				continue;
			if (errX <= 1.0 && errY <= 1.0)	//誤差值該設定為多少??
				NotOverlayPointsFlag[j] = false;
		}
	}

	//位置接近的特徵點暫時先刪除OpticalFlow算出來的
	for (size_t i = 0; i < NotOverlayPointsFlag.size(); i++)
	{
		if (NotOverlayPointsFlag[i])
		{
			GoodMatchedPoints[0].push_back(featureMapGoodMatches[i]);
			GoodMatchedPoints[1].push_back(frameGoodMatches[i]);
		}
	}
	featureMapGoodMatches.swap(GoodMatchedPoints[0]);
	frameGoodMatches.swap(GoodMatchedPoints[1]);

	//free vector memory
	std::vector<cv::Point2f>().swap(GoodMatchedPoints[0]);
	std::vector<cv::Point2f>().swap(GoodMatchedPoints[1]);
	std::vector<bool>().swap(NotOverlayPointsFlag);
}

void SurfDetection(cv::Mat &image, std::vector<cv::KeyPoint> &keypoints, cv::Mat &descriptors, unsigned int minHessian)
{
	int win = glutGetWindow();
	cv::SurfFeatureDetector detector(minHessian);
	detector.detect(image, keypoints);
	if (keypoints.size() == 0){
		glutSetWindow(win);
		return;
	}
	cv::SurfDescriptorExtractor extractor;
	extractor.compute(image, keypoints, descriptors);

	glutSetWindow(win);
}

void FlannMatching(cv::Mat &descriptors1, cv::Mat &descriptors2, std::vector<cv::DMatch> &matches)
{
	int win = glutGetWindow();
	cv::FlannBasedMatcher matcher;
	if (descriptors1.type() != CV_32F)
		descriptors1.convertTo(descriptors1, CV_32F);
	if (descriptors2.type() != CV_32F)
		descriptors2.convertTo(descriptors2, CV_32F);
	matcher.match(descriptors1, descriptors2, matches);
	glutSetWindow(win);
}

void OpticalFlow(FeatureMap &featureMap, std::vector<KeyFrame> &keyFrames, Frame &currFrame, std::vector<cv::Point2f> &currPts, std::vector<int> &good3DMatchesIdx)
{
	std::vector<cv::Point2f> prevPts;
	std::vector<cv::Point2f> opticalflowPts;
	cv::KeyPoint::convert(keyFrames[keyFrames.size() - 1].keypoints_3D, prevPts);
	std::vector<cv::Point2f> reProjPts;
	cv::KeyPoint::convert(featureMap.reProjPts, reProjPts);
	std::vector<uchar> status;
	std::vector<float> error;
	cv::calcOpticalFlowPyrLK(keyFrames[keyFrames.size() - 1].image, currFrame.image, reProjPts, opticalflowPts, status, error);

	for (int i = 0; i < opticalflowPts.size(); ++i)
	{
		//norm is ...
		if (status[i] == 1 || (cv::norm(opticalflowPts[i] - reProjPts[i]) > 5))
		{
			currPts.push_back(opticalflowPts[i]);
			good3DMatchesIdx.push_back(i);
		}
	}
	std::vector<cv::Point2f>().swap(reProjPts);
	std::vector<cv::Point2f>().swap(prevPts);
	std::vector<cv::Point2f>().swap(opticalflowPts);
	std::vector<uchar>().swap(status);
	std::vector<float>().swap(error);
}

void OpticalFlow(cv::Mat &prevFrame, cv::Mat &currFrame, std::vector<cv::Point2f> &prevFrameGoodMatches, std::vector<cv::Point2f> &prevFeatureMapGoodMatches, std::vector<cv::Point2f> &featureMapGoodMatches, std::vector<cv::Point2f> &frameGoodMatches)
{
	//For PoseEsitmation by homography
	int GoodSize = 0;
	std::vector<cv::Point2f> GoodMatches[2]; //[0]->preFeatureMap, [1]->prevFrame, store the feature points
	std::vector<uchar> status;	//record OpticalPyrLK keypoints which are right tracking
	std::vector<float> error;	//record OpticalPyrLK error
	std::vector<cv::Point2f> OpticalFlow_keypoints;
	
	// OpenCV function to find OpticalFlow points
	cv::calcOpticalFlowPyrLK(prevFrame, currFrame, prevFrameGoodMatches, OpticalFlow_keypoints, status, error);

	for (std::size_t i = 0; i < OpticalFlow_keypoints.size(); ++i)
	{
		//兩張圖error門檻值取大於30都不錯
		//error似乎只是判斷有無值可以用(cv::norm(OpticalFlow_keypoints[i] - prevFeatureMapGoodMatches[i]) < 5))
		//根據這部分code來設定threshold https://github.com/Itseez/opencv/blob/master/samples/cpp/lkdemo.cpp/
		if (!(status[i] == 0 || (cv::norm(OpticalFlow_keypoints[i] - prevFeatureMapGoodMatches[i]) <= 5)))	//目前為經驗法則
		{
			GoodMatches[0].push_back(prevFeatureMapGoodMatches[i]);//避免拍到不是target image時，依舊保留上一個frame的特徵點
			GoodMatches[1].push_back(prevFrameGoodMatches[i]);
			featureMapGoodMatches.push_back(prevFeatureMapGoodMatches[i]);
			frameGoodMatches.push_back(OpticalFlow_keypoints[i]);
			GoodSize++;
		}
	}
	prevFeatureMapGoodMatches.swap(GoodMatches[0]);
	prevFrameGoodMatches.swap(GoodMatches[1]);
	cout << "OpticalFlow Size : " << GoodSize << endl;

	std::vector<cv::Point2f>().swap(GoodMatches[0]);
	std::vector<cv::Point2f>().swap(GoodMatches[1]);
	std::vector<cv::Point2f>().swap(OpticalFlow_keypoints);
	std::vector<uchar>().swap(status);
	std::vector<float>().swap(error);
}

bool FeatureDetectionAndMatching(FeatureMap &featureMap, Frame &currFrame, unsigned char *inputFrame, unsigned char *inputPrevFrame, unsigned int minHessian, std::vector<cv::Point2f> &featureMapGoodMatches, std::vector<cv::Point2f> &currFrameGoodMatches, std::vector<cv::Point2f> &prevFeatureMapInliers, std::vector<cv::Point2f> &prevFrameInliers)
{
	//Camera captures the marker
	cv::Mat prevFrameImg, currFrameImg;
	if (inputPrevFrame != NULL)
		prevFrameImg = cv::Mat(currFrame.image.rows, currFrame.image.cols, CV_8UC3, inputPrevFrame);
	SurfDetection(currFrame.image, currFrame.keypoints, currFrame.descriptors, minHessian);
	if (currFrame.keypoints.size() == 0) return false;
	std::vector<cv::DMatch> matches;
	FlannMatching(featureMap.descriptors, currFrame.descriptors, matches);
	FindGoodMatches(featureMap, currFrame, currFrame.keypoints, matches, featureMapGoodMatches, currFrameGoodMatches);
	//cout << "Surf Good Matches Size : " << currFrameGoodMatches.size() << std::endl;

	bool usingOpticalFlow = false;
	//GoodMatches size is not enough
	if ((int)currFrameGoodMatches.size() < 25)
	{
		if (prevFrameInliers.size() != 0 && prevFeatureMapInliers.size() != 0 && prevFrameImg.data != NULL)
		{
			//Use optical flow to detect feature
			OpticalFlow(prevFrameImg, currFrame.image, prevFrameInliers, prevFeatureMapInliers, featureMapGoodMatches, currFrameGoodMatches);
			usingOpticalFlow = true;
		}
		else
		{
			prevFeatureMapInliers.swap(featureMapGoodMatches);
			prevFrameInliers.swap(currFrameGoodMatches);

			std::vector<cv::DMatch>().swap(matches);
			std::vector<cv::KeyPoint>().swap(currFrame.keypoints);
			std::vector<cv::Point2f>().swap(featureMapGoodMatches);
			std::vector<cv::Point2f>().swap(currFrameGoodMatches);

			return false;
		}
	}

	if (usingOpticalFlow)
	{
		DeleteOverlap(featureMapGoodMatches, currFrameGoodMatches);
		//cout << "Surf + OpticalFlow Good Matches Size : " << currFrameGoodMatches.size() << std::endl;
		if ((int)currFrameGoodMatches.size() < 30)
		{
			std::vector<cv::DMatch>().swap(matches);
			std::vector<cv::KeyPoint>().swap(currFrame.keypoints);
			std::vector<cv::Point2f>().swap(featureMapGoodMatches);
			std::vector<cv::Point2f>().swap(currFrameGoodMatches);
			return false;
		}
	}

	std::vector<cv::DMatch>().swap(matches);
	return true;
}

bool FeatureDetectionAndMatching(std::vector<KeyFrame> &keyFrames, Frame &currFrame, unsigned int minHessian)
{
	SurfDetection(currFrame.image, currFrame.keypoints, currFrame.descriptors, minHessian);
	if (currFrame.keypoints.size() == 0) return false;
	std::vector<cv::DMatch> matches;
}