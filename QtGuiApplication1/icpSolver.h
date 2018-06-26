#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

#ifndef icpSolver_H
#define icpSolver_H

class solver
{
public:
	solver();
	solver(bool compareWhole, int icpIterationTimes, Size inputSize, vector<Point> &source_contour, vector<Point> target_contour);
	Mat getTwoSetsTransform(vector<Point> &source, vector<Point> &target);
	double getMinDistanceP(vector<Point> &source, vector<Point> &target);

	Mat getResultMat();
	vector<Point> getResultPointSeq();
	double getIcpVarErr();

private:
	bool _compareWhole;
	Size _inputSize;
	Mat _resultMat;
	vector<Point> _newPointSeq;
	double _varErr;
	double _currentIcpError = 0; // just use for ouput

	// thrshold
	int icpIteration;
	double partialIcpThreshold = 15;
	double thetaBreakThreshold = 0.01;
	double icpErrorBreakThreshold = 10;

	Mat icp_warp(int times, vector<Point> &sourcePointOri, vector<Point> &sourcePoint, vector<Point> &targetPoint);
	Mat icp_warp_part(int times, vector<Point> &sourcePointOri, vector<Point> &sourcePoint, vector<Point> &targetPoint);
	Mat icp_warp_part_withoutNorm(int times, vector<Point> &sourcePointOri, vector<Point> &sourcePoint, vector<Point> &targetPoint);
	double getMinDistanceVarErr(vector<Point> &source, vector<Point> &target);

	Point getCenter(vector<Point>& pointSeq);
	Point2f getNormVector(Point pre, Point tgt, Point nxt);
	double getTheta(Mat &warpMatrixs);
};
#endif
