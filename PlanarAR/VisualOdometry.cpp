#include <thread>
#include "VisualOdometry.h"
#include "FeatureProcess.h"
#include "SFMUtil.h"

static vector<cv::Point2f> prevFrameInliers, prevFeatureMapInliers;
static vector<FrameMetaData> frameMetaDatas;
static vector<FeatureMap> featureMaps;
static vector<KeyFrame> keyFrames;
static std::thread Optimization;

/*
void LoadFeatureMap(int argc, char *argv[])
{
	//Loading featureMap
}
*/

bool VO(double *cameraPara, double trans[3][4], FeatureMap &featureMap, cv::Mat &prevFrameMat, cv::Mat &currFrameMat, char &m)
{	
	//cout << "KeyFrame Set : " << keyFrames.size() << endl;
	if (Optimization.joinable())
		Optimization.join();
	
	FrameMetaData currData;
	if (!FeatureDetection(3000, currData, currFrameMat)) return false;

	vector<cv::Point2f> currFrameGoodMatches, featureMapGoodMatches;
	vector<int> neighboringKeyFrameIdx;
	vector< vector<cv::DMatch> > goodMatchesSet;
	
	if (FeatureMatching(featureMap, currData, currFrameMat, prevFrameMat, featureMapGoodMatches, currFrameGoodMatches, prevFeatureMapInliers, prevFrameInliers))
	{
		EstimateCameraTransformation(cameraPara, trans, featureMap, currData, featureMapGoodMatches, currFrameGoodMatches, prevFeatureMapInliers, prevFrameInliers);
		if (keyFrames.size() == 0)
			CreateKeyFrame(cameraPara, currData, currFrameMat, keyFrames);
	}
	else if (FeatureMatching(cameraPara, keyFrames, currData, currFrameMat, neighboringKeyFrameIdx, goodMatchesSet))
	{
		EstimateCameraTransformation(cameraPara, trans, keyFrames, currData, neighboringKeyFrameIdx, goodMatchesSet);
		if (currData.state == 'F')
			return false;
	}
	else return false;
	if (KeyFrameSelection(keyFrames[keyFrames.size() - 1], currData))
	{
		CreateKeyFrame(cameraPara, currData, currFrameMat, keyFrames);
		Triangulation(cameraPara, keyFrames);
		//Optimization = std::thread(Triangulation, cameraPara, ref(keyFrames));
	}
	m = currData.state;
	frameMetaDatas.push_back(currData);

	return true;
}