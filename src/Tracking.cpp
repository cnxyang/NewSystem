#include <iostream>
#include <vector>

#include "Tracking.hpp"
#include "Solver.hpp"

Tracking::Tracking() {
	mpMap = nullptr;
	mNextState = NOT_INITIALISED;
	mORBMatcher = cv::cuda::DescriptorMatcher::createBFMatcher(cv::NORM_HAMMING);
}

bool Tracking::GrabImageRGBD(cv::Mat& imRGB, cv::Mat& imD) {

	mNextFrame = Frame(imRGB, imD);

	bool bOK = Track();
	if(!bOK)
		return false;

	mLastFrame = Frame(mNextFrame);

	return true;
}

bool Tracking::Track() {
	bool bOK;
	switch(mNextState) {
	case NOT_INITIALISED:
		bOK = CreateInitialMap();
		break;

	case OK:
		bOK = TrackLastFrame();
		break;

	case LOST:
		break;
	}

	if(!bOK) {
		mNextState = LOST;
	}
	else {
		mNextState = OK;
	}

	return bOK;
}

bool Tracking::CreateInitialMap() {
	mpMap->SetFirstFrame(mNextFrame);
//	mbNeedNewKF = true;
	mNextState = OK;
//	mNoFrames = 0;
	return true;
}

bool Tracking::TrackLastFrame() {
	mNextFrame.SetPose(mLastFrame);
	bool bOK = TrackFrame();
	if(!bOK)
		return false;
	TrackICP();
	return true;
}

bool Tracking::TrackFrame() {

	std::vector<cv::DMatch> Matches;
	std::vector<std::vector<cv::DMatch>> matches;
	mORBMatcher->knnMatch(mNextFrame.mDescriptors, mLastFrame.mDescriptors, matches, 2);

	for(int i = 0; i < matches.size(); ++i) {
		cv::DMatch& firstMatch = matches[i][0];
		cv::DMatch& secondMatch = matches[i][1];
		if(firstMatch.distance < 0.7 *  secondMatch.distance) {
				Matches.push_back(firstMatch);
		}
	}

	if(Matches.size() < 3)
		return false;

	std::vector<Eigen::Vector3d> p;
	std::vector<Eigen::Vector3d> q;
	for(int i = 0; i < Matches.size(); ++i) {
		int queryId = Matches[i].queryIdx;
		int trainId = Matches[i].trainIdx;
		MapPoint& queryPt = mNextFrame.mMapPoints[queryId];
		MapPoint& trainPt = mLastFrame.mMapPoints[trainId];

		p.push_back(queryPt.pos);
		q.push_back(trainPt.pos);
	}

	vector<bool> outliers;
	Eigen::Matrix4d Td = Eigen::Matrix4d::Identity();
	Solver::SolveAbsoluteOrientation(p, q, outliers, Td);

	Eigen::Matrix4d Tp = mLastFrame.mPose;
	Eigen::Matrix4d Tc = Eigen::Matrix4d::Identity();
	Tc =  Td.inverse() * Tp;

	mNextFrame.SetPose(Tc);

	return true;
}

void Tracking::TrackICP() {

	Eigen::Matrix4d Td;
	Solver::SolveICP(mNextFrame, mLastFrame, Td);
//	ShowResiduals();
}

void Tracking::AddObservation(const Rendering& render) {
	mLastFrame = Frame(mLastFrame, render);
}

void Tracking::SetMap(Map* pMap) {
	mpMap = pMap;
}

void Tracking::ShowResiduals() {

	DeviceArray2D<uchar> warpImg(640, 480);
	DeviceArray2D<uchar> residual(640, 480);
	warpImg.zero();
	residual.zero();
	WarpGrayScaleImage(mNextFrame, mLastFrame, residual);
	ComputeResidualImage(residual, warpImg, mNextFrame);
	cv::Mat cvresidual(480, 640, CV_8UC1);
	warpImg.download((void*)cvresidual.data, cvresidual.step);
	cv::imshow("residual", cvresidual);
}
