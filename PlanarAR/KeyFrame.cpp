#include <iostream>
#include <cmath>
#include <sstream>
#include <string>
#include "KeyFrame.h"
#include "SFMUtil.h"
#include "debugfunc.h"
using std::cout;
using std::endl;

unsigned int keyFrameIdx = 1;


void CreateProjMatrix(double *cameraPara, const MyMatrix &R, const Vector3d &t, MyMatrix &projMatrix)
{
	MyMatrix K(3, 3);
	for (int i = 0; i < 9; ++i)
		K.m_lpdEntries[i] = cameraPara[i];
	projMatrix.CreateMatrix(3, 4);
	projMatrix.m_lpdEntries[0] =  R.m_lpdEntries[0];
	projMatrix.m_lpdEntries[1] =  R.m_lpdEntries[1];
	projMatrix.m_lpdEntries[2] =  R.m_lpdEntries[2];
	projMatrix.m_lpdEntries[3] =  t.x;
	projMatrix.m_lpdEntries[4] =  R.m_lpdEntries[3];
	projMatrix.m_lpdEntries[5] =  R.m_lpdEntries[4];
	projMatrix.m_lpdEntries[6] =  R.m_lpdEntries[5];
	projMatrix.m_lpdEntries[7] =  t.y;
	projMatrix.m_lpdEntries[8] =  R.m_lpdEntries[6];
	projMatrix.m_lpdEntries[9] =  R.m_lpdEntries[7];
	projMatrix.m_lpdEntries[10] = R.m_lpdEntries[8];
	projMatrix.m_lpdEntries[11] = t.z;

	projMatrix = K * projMatrix;
}

void CreateKeyFrame(double *cameraPara, FrameMetaData &currData, cv::Mat &currFrameImg, std::vector<KeyFrame> &keyFrames)
{
	KeyFrame keyFrame;
	currFrameImg.copyTo(keyFrame.image);
	currData.keypoints.swap(keyFrame.keypoints);
	currData.descriptors.copyTo(keyFrame.descriptors);
	keyFrame.R.CreateMatrix(3, 3);
	keyFrame.R = currData.R;
	keyFrame.t = currData.t;
	CreateProjMatrix(cameraPara, keyFrame.R, keyFrame.t, keyFrame.projMatrix);
	keyFrames.push_back(keyFrame);
	std::stringstream ss;
	std::string fileName;
	ss << "KeyFrame" << keyFrameIdx++ << ".jpg";
	fileName = ss.str();
	SavingKeyFrame(fileName, keyFrame.image);
}

double calcDistance(Vector3d &t1, Vector3d &t2)
{
	double t1Length = sqrt(pow(t1.x, 2.0) + pow(t1.y, 2.0) + pow(t1.z, 2.0));
	double t2Length = sqrt(pow(t2.x, 2.0) + pow(t2.y, 2.0) + pow(t2.z, 2.0));
	//Cosine值？!
	double Cosine = (t1.x*t2.x + t1.y*t2.y + t1.z*t2.z) / (t1Length*t2Length);
	double theta = acos(Cosine);
	if (theta > 90)
		return 0.00;
	cout << "Theta : " << theta;
	//畫出一個三角形就知道了
	double distance = sqrt(pow(t1Length, 2.0) + pow(t2Length, 2.0) - 2 * t1Length*t2Length*Cosine);
	return distance;
}

double calcAngle(MyMatrix &R1, MyMatrix &R2)
{
	//相機方向
	Vector3d R1Col2, R2Col2;
	R1Col2.x = R1.m_lpdEntries[2] * (-1);
	R1Col2.y = R1.m_lpdEntries[5] * (-1);
	R1Col2.z = R1.m_lpdEntries[8] * (-1);
	R2Col2.x = R2.m_lpdEntries[2] * (-1);
	R2Col2.y = R2.m_lpdEntries[5] * (-1);
	R2Col2.z = R2.m_lpdEntries[8] * (-1);
	
	double R1Col2Length = sqrt(pow(R1Col2.x, 2.0) + pow(R1Col2.y, 2.0) + pow(R1Col2.z, 2.0));
	double R2Col2Length = sqrt(pow(R2Col2.x, 2.0) + pow(R2Col2.y, 2.0) + pow(R2Col2.z, 2.0));
	double Cosine = (R1Col2.x*R2Col2.x + R1Col2.y*R2Col2.y + R1Col2.z*R2Col2.z) / (R1Col2Length*R2Col2Length);
	double theta = acos(Cosine);

	return theta;
}

bool isNegihboringKeyFrame(Vector3d &t1, Vector3d &t2, Vector3d &r3dPtVec)
{
	//用兩張frame的原點算出原點的3D點座標
	//計算t跟原點跟3D點座標的向量之夾角
	
	double r3dPtVecLength = sqrt(pow(r3dPtVec.x, 2.0) + pow(r3dPtVec.y, 2.0) + pow(r3dPtVec.z, 2.0));
	double t1Length = sqrt(pow(t1.x, 2.0) + pow(t1.y, 2.0) + pow(t1.z, 2.0));
	double t2Length = sqrt(pow(t2.x, 2.0) + pow(t2.y, 2.0) + pow(t2.z, 2.0));
	double Cosine1 = (t1.x*r3dPtVec.x + t1.y*r3dPtVec.y + t1.z*r3dPtVec.z) / (r3dPtVecLength*t1Length);
	double Cosine2 = (t2.x*r3dPtVec.x + t2.y*r3dPtVec.y + t2.z*r3dPtVec.z) / (r3dPtVecLength*t2Length);
	double theta1 = acos(Cosine1);
	double theta2 = acos(Cosine2);
	
	//求出來的theta都很小 0.5~1.5
	//cout << theta1 << " " << theta2 << endl;
	//cout << abs(theta1 - theta2) << endl;
	if (theta1 > 90.0 || theta2 > 90.0)
		return false;
	if (abs(theta1 - theta2) >= 30.0)
		return false;
	return true;
}

void FindNeighboringKeyFrames(std::vector<KeyFrame> &keyFrames, FrameMetaData &currData, std::vector<int> &neighboringKeyFrameIdx)
{
	/*	Use the last keyframe to find the neighboring keyframes	*/
	int keyFramesSize = (int)keyFrames.size() - 1;
	cv::Point2f originPt(400.0f, 300.0f);
	int index = 0;
	for (std::vector<KeyFrame>::iterator KF = keyFrames.begin(); index < keyFramesSize; ++index)
	{
		cv::Point3d r3dPt;
		if (Find3DCoordinates(KF->projMatrix, keyFrames.back().projMatrix, originPt, originPt, r3dPt))
		{
			Vector3d r3dPtVec;
			r3dPtVec.x = r3dPt.x; r3dPtVec.y = r3dPt.y; r3dPtVec.z = r3dPt.z;
			if (isNegihboringKeyFrame(KF->t, currData.t, r3dPtVec))
				neighboringKeyFrameIdx.push_back(index);
		}
	}
	/*	Push the last keyframe	*/
	neighboringKeyFrameIdx.push_back(keyFramesSize);
}

bool KeyFrameSelection(KeyFrame &keyFramesBack, FrameMetaData &currData, vector <Measurement> &measurementData)
{
	double distance = calcDistance(keyFramesBack.t, currData.t);
	if(distance < 150.0 || isnan(distance))
		return false;
	double angle = calcAngle(keyFramesBack.R, currData.R);
	if (angle < 0.15 || isnan(angle))
		return false;

	Measurement measurement;
	measurement.distance = distance;
	measurement.angle = angle;
	measurementData.push_back(measurement);
	return true;
}