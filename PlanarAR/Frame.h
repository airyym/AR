#ifndef FRAME_H
#define FRAME_H
#define PI 3.141592653589793
#include <vector>
#include <cmath>
#include <ctime>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2\stitching\stitcher.hpp"
#include "Matrix\MyMatrix.h"
#include "BasicType.h"

//extern std::vector<MyMatrix> projMatrixSet;

class Frame
{
public:
	void release()
	{
		image.release();
		std::vector<cv::KeyPoint>().swap(keypoints);
		descriptors.release();
	}
	Frame& operator= (Frame &frame)
	{
		frame.image.copyTo(image);
		keypoints = frame.keypoints;
		frame.descriptors.copyTo(descriptors);
		return *this;
	}
	cv::Mat image;
	std::vector<cv::KeyPoint> keypoints;	//Record all keypoints in the image
	cv::Mat descriptors;	//Record all descriptors in the image
	clock_t timeStamp;
	bool state;
	MyMatrix R;
	Vector3d t;
};

class KeyFrame
{
public:
	cv::Mat image;
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;
	MyMatrix projMatrix;
	MyMatrix R;
	Vector3d t;

	std::vector<cv::KeyPoint> keypoints_3D;
	cv::Mat descriptors_3D;
	unsigned long index;
};

bool KeyFrameSelection(unsigned long index, MyMatrix &R, Vector3d t, std::vector<KeyFrame> &keyFrames);
#endif