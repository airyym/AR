#ifndef SFMUTIL_H
#define SFMUTIL_H

#include <vector>
#include "BasicType.h"
#include "KeyFrame.h"
#include "Matrix\MyMatrix.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"


class SFM_Feature
{
public:
	bool isValid; // indicates whether this point is reliable or not
	bool find3d; // indicates whether this point finds its 3d points or not
	cv::Point2f pt; // coordinates of image feature
	int imgIdx;  // index of image on which the point lies
	int ptIdx; // index of corresponding 3D point
	int descriptorIdx; // index of feature descriptor array
	std::vector<int> cores; // index of correspondences in other views
	SFM_Feature() : isValid(false), find3d(false), imgIdx(-1), ptIdx(-1)
	{
	}
};

bool Find3DCoordinates(const std::vector<MyMatrix> &Ps, const std::vector<Point3d> &pts, Point4d &r3DPt);
bool Find3DCoordinates(std::vector<MyMatrix> &Ps, const std::vector<cv::Point2f> &pts, cv::Point3d &r3DPt);
bool Find3DCoordinates(MyMatrix &P1, MyMatrix &P2, const cv::Point2f &pt1, const cv::Point2f &pt2, cv::Point3d &r3DPt);

bool OptimalTriangulation(const MyMatrix &P1, const MyMatrix &P2, const cv::Point2f &pt1, const cv::Point2f &pt2, const MyMatrix &F, cv::Point3d &r3DPt);

void CalculateCameraParameters(const MyMatrix &P, MyMatrix &K, MyMatrix &R, MyMatrix &t);
double EstimateFocalLength(MyMatrix &F, double u0, double v0, double u1, double v1);

void Triangulation(double *cameraPara, std::vector<KeyFrame> &keyFrames);
//Local or Global Optimization function
#endif