#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "icpSolver.h"
#include <Eigen\Dense>
using namespace Eigen;
using namespace std;
using namespace cv;

# define PI 3.1415926

solver::solver() {
}

solver::solver(bool compareWhole, int icpIterationTimes, Size inputSize, vector<Point> &source_contour, vector<Point> target_contour) {
	clock_t s0 = clock();

	_compareWhole = compareWhole;
	icpIteration = icpIterationTimes;
	_inputSize = inputSize;

	double preIcpErr = -1.0;
	double curIcpErr = 0.0;
	double src2tgtErr = 0.0;
	double tgt2srcErr = 0.0;
	double minDist = 0.0;
	double dist = 0.0;

	//get icp error (for scale matrix)
	src2tgtErr = getMinDistanceVarErr(target_contour, source_contour);
	tgt2srcErr = getMinDistanceVarErr(source_contour, target_contour);
	curIcpErr = src2tgtErr + tgt2srcErr;

	vector<Point> shift_source_contour = source_contour;
	// re-shift by center
	Point targetCenter = getCenter(target_contour);
	Point sourceCenter = getCenter(shift_source_contour);

	// Initially, warp sourceCenter to targetCenter
	Point cneterShift(0, 0);
	if (_compareWhole) {
		cneterShift = targetCenter - sourceCenter;
		for (int i = 0; i < shift_source_contour.size(); i++) {
			shift_source_contour[i] += cneterShift;
		}
	}

	int scaleIter = 0;
	double lastTheta = 0;
	bool hasWarp = true;
	Mat resultMat;
	vector<Point> newPointSeq = shift_source_contour;
	do {
		scaleIter++;
		preIcpErr = curIcpErr;

		if (_compareWhole)
			resultMat = icp_warp(scaleIter, shift_source_contour, newPointSeq, target_contour).clone();
		else
			resultMat = icp_warp_part_withoutNorm(scaleIter, shift_source_contour, newPointSeq, target_contour).clone();
		//resultMat = icp_warp_part(scaleIter, shift_source_contour, newPointSeq, target_contour).clone();
		
		//if result is 0, not take warp matrix into points
		if (resultMat.size() == cv::Size(0, 0)) {//cos = 0 & sin = 0
			hasWarp = false;
			break;
		}
		else {
			//draw contour result
			for (int i = 0; i < newPointSeq.size(); i++){
				double newX = resultMat.at<double>(0, 0)*shift_source_contour[i].x + resultMat.at<double>(0, 1)*shift_source_contour[i].y + resultMat.at<double>(0, 2);
				double newY = resultMat.at<double>(1, 0)*shift_source_contour[i].x + resultMat.at<double>(1, 1)*shift_source_contour[i].y + resultMat.at<double>(1, 2);
				newPointSeq[i].x = (int)newX;
				newPointSeq[i].y = (int)newY;
			}

			// get theta to determine break or not
			double dTheta = getTheta(resultMat); //cout << "dTheta= " << dTheta << endl;
			if (abs(lastTheta - dTheta) < thetaBreakThreshold) //thetaBreakThreshold=0.01
				scaleIter = icpIteration;
		}
	} while (abs(curIcpErr - preIcpErr) > icpErrorBreakThreshold && scaleIter < icpIteration);
	// icpErrorBreakThreshold=10, icpIteration=20

	if (!hasWarp) {
		resultMat = Mat::zeros(Size(3, 2), CV_64F);
		resultMat.at<double>(0, 0) = 1;
		resultMat.at<double>(0, 1) = 0;
		resultMat.at<double>(0, 2) = 0;
		resultMat.at<double>(1, 0) = 0;
		resultMat.at<double>(1, 1) = 1;
		resultMat.at<double>(1, 2) = 0;
	}
	else if (_compareWhole) {
		// mat * shift mat
		/*  resultMat [ a, -b, c
		b,  a, d ]
		shift mat [ x
		y ]
		resultMat [ a, -b, ax-by+c
		b,  a, bx+ay+d ]
		*/
		int shiftX = resultMat.at<double>(0, 0)*cneterShift.x + resultMat.at<double>(0, 1)*cneterShift.y + resultMat.at<double>(0, 2);
		int shiftY = resultMat.at<double>(1, 0)*cneterShift.x + resultMat.at<double>(1, 1)*cneterShift.y + resultMat.at<double>(1, 2);
		resultMat.at<double>(0, 2) = shiftX;
		resultMat.at<double>(1, 2) = shiftY;
	}

	src2tgtErr = getMinDistanceVarErr(target_contour, newPointSeq);
	tgt2srcErr = getMinDistanceVarErr(newPointSeq, target_contour);
	curIcpErr = src2tgtErr + tgt2srcErr;
	_currentIcpError = curIcpErr; //cout << "pre: " << preIcpErr << ", current: " << curIcpErr << endl;

	_resultMat = resultMat;
	_newPointSeq = newPointSeq;
	_varErr = curIcpErr / 2;

	clock_t s1 = clock();
	/*cout << "Icp time: " << s1 - s0 << endl;
	system("pause");*/
}

// icp solver for whole every pair between source and target
Mat solver::icp_warp(int times, vector<Point> &sourcePointOri, vector<Point> &sourcePoint, vector<Point> &targetPoint) {
	/*Mat drawing = Mat::zeros(_inputSize, CV_8UC3);
	for (int i = 0; i < sourcePoint.size(); i++)
		circle(drawing, sourcePoint[i], 2, Scalar(0, 0, 255), 2, 8);
	for (int i = 0; i < targetPoint.size(); i++)
		circle(drawing, targetPoint[i], 2, Scalar(255, 255, 0), 2, 8);*/

	// get norm vector of two sequential point
	vector<Point2f> target_normVec;
	for (int j = 0; j < targetPoint.size(); j++) {
		Point2f normVec = getNormVector(targetPoint[(j - 1 + targetPoint.size()) % targetPoint.size()], targetPoint[j], targetPoint[(j + 1) % targetPoint.size()]);
		target_normVec.push_back(normVec);
	}
	vector<Point2f> source_normVec;
	for (int j = 0; j < sourcePoint.size(); j++) {
		Point2f normVec = getNormVector(sourcePoint[(j - 1 + sourcePoint.size()) % sourcePoint.size()], sourcePoint[j], sourcePoint[(j + 1) % sourcePoint.size()]);
		source_normVec.push_back(normVec);
	}
	
	double minDist = 0;
	int minPointIndex = 0;
	vector<Point> sourcePointPair;
	vector<Point> targetPointPair;
	//-----------------------------------
	//find nearest one from source to target
	for (int i = 0; i < sourcePoint.size(); i++) {
		for (int j = 0; j < targetPoint.size(); j++) {
			double dist = sqrt(pow(targetPoint[j].x - sourcePoint[i].x, 2) + pow(targetPoint[j].y - sourcePoint[i].y, 2));
			dist = abs(dist *(1 - (target_normVec[j].x * source_normVec[i].x + target_normVec[j].y * source_normVec[i].y) / sqrt(2)));
			if (j == 0) {
				minDist = dist;
				minPointIndex = j;
			}
			else if (dist < minDist) {
				minDist = dist;
				minPointIndex = j;
			}
		}
		sourcePointPair.push_back(sourcePointOri[i]);
		targetPointPair.push_back(targetPoint[minPointIndex]);

		//line(drawing, sourcePoint[i], targetPoint[minPointIndex], Scalar(255, 0, 255), 1, 8);
	}
	//-----------------------------------
	//find nearest one from target to source
	for (int i = 0; i < targetPoint.size(); i++) {
		for (int j = 0; j < sourcePoint.size(); j++) {
			double dist = sqrt(pow(targetPoint[i].x - sourcePoint[j].x, 2) + pow(targetPoint[i].y - sourcePoint[j].y, 2));
			dist = abs(dist *(1 - (target_normVec[i].x * source_normVec[j].x + target_normVec[i].y * source_normVec[j].y) / sqrt(2)));
			if (j == 0) {
				minDist = dist;
				minPointIndex = j;
			}
			else if (dist < minDist) {
				minDist = dist;
				minPointIndex = j;
			}
		}
		sourcePointPair.push_back(sourcePointOri[minPointIndex]);
		targetPointPair.push_back(targetPoint[i]);

		//line(drawing, sourcePoint[minPointIndex], targetPoint[i], Scalar(255, 255, 0), 1, 8);
	}
	//-----------------------------------
	Mat warpMtx = getTwoSetsTransform(sourcePointPair, targetPointPair);
	/*cout << "warpMtx= " << warpMtx << endl;
	imwrite("Result0/" + to_string(times) + ".png", drawing);
	system("pause");*/
	return warpMtx;
}

// icp solver for only pair between source and target that distance < 10 
Mat solver::icp_warp_part(int times, vector<Point> &sourcePointOri, vector<Point> &sourcePoint, vector<Point> &targetPoint) {
	Mat drawing = Mat::zeros(_inputSize, CV_8UC3);
	for (int i = 0; i < sourcePoint.size(); i++)
		circle(drawing, sourcePoint[i], 2, Scalar(0, 0, 255), 2, 8);
	for (int i = 0; i < targetPoint.size(); i++)
		circle(drawing, targetPoint[i], 2, Scalar(255, 255, 0), 2, 8);

	// get norm vector of two sequential point
	vector<Point2f> target_normVec;
	for (int j = 0; j < targetPoint.size(); j++) {
		Point2f normVec = getNormVector(targetPoint[(j - 1 + targetPoint.size()) % targetPoint.size()], targetPoint[j], targetPoint[(j + 1) % targetPoint.size()]);
		target_normVec.push_back(normVec);
	}
	vector<Point2f> source_normVec;
	for (int j = 0; j < sourcePoint.size(); j++) {
		Point2f normVec = getNormVector(sourcePoint[(j - 1 + sourcePoint.size()) % sourcePoint.size()], sourcePoint[j], sourcePoint[(j + 1) % sourcePoint.size()]);
		source_normVec.push_back(normVec);
	}

	double minDist = 0;
	int minPointIndex = 0;
	vector<Point> sourcePointPair;
	vector<Point> targetPointPair;
	//-----------------------------------
	//find nearest one from source to target
	for (int i = 0; i < sourcePoint.size(); i++) {
		for (int j = 0; j < targetPoint.size(); j++) {
			double dist = sqrt(pow(targetPoint[j].x - sourcePoint[i].x, 2) + pow(targetPoint[j].y - sourcePoint[i].y, 2));
			dist = abs(dist *(1 - (target_normVec[j].x * source_normVec[i].x + target_normVec[j].y * source_normVec[i].y) / sqrt(2)));
			if (j == 0) {
				minDist = dist;
				minPointIndex = j;
			}
			else if (dist < minDist) {
				minDist = dist;
				minPointIndex = j;
			}
		}

		if (minDist < partialIcpThreshold) {
			sourcePointPair.push_back(sourcePointOri[i]);
			targetPointPair.push_back(targetPoint[minPointIndex]);
			line(drawing, sourcePoint[i], targetPoint[minPointIndex], Scalar(255, 0, 255), 1, 8);
		}
	}
	//-----------------------------------
	//find nearest one from target to source
	for (int i = 0; i < targetPoint.size(); i++) {
		for (int j = 0; j < sourcePoint.size(); j++) {
			double dist = sqrt(pow(targetPoint[i].x - sourcePoint[j].x, 2) + pow(targetPoint[i].y - sourcePoint[j].y, 2));
			dist = abs(dist *(1 - (target_normVec[i].x * source_normVec[j].x + target_normVec[i].y * source_normVec[j].y) / sqrt(2)));
			if (j == 0) {
				minDist = dist;
				minPointIndex = j;
			}
			else if (dist < minDist) {
				minDist = dist;
				minPointIndex = j;
			}
		}
		if (minDist < partialIcpThreshold) {
			sourcePointPair.push_back(sourcePointOri[minPointIndex]);
			targetPointPair.push_back(targetPoint[i]);
			line(drawing, sourcePoint[minPointIndex], targetPoint[i], Scalar(255, 255, 0), 1, 8);
		}
	}
	//-----------------------------------
	if (sourcePointPair.size() == 0)
		return Mat();
	
	Mat warpMtx = getTwoSetsTransform(sourcePointPair, targetPointPair);
	//cout << "warpMtx= " << warpMtx << endl;
	//imwrite("Result0/" + to_string(times) + "_" + to_string(_currentIcpError) + ".png", drawing);
	//system("pause");
	return warpMtx;
}

// icp solver for only pair between source and target that distance < 10 
Mat solver::icp_warp_part_withoutNorm(int times, vector<Point> &sourcePointOri, vector<Point> &sourcePoint, vector<Point> &targetPoint) {
	Mat drawing = Mat::zeros(_inputSize, CV_8UC3);
	for (int i = 0; i < sourcePoint.size(); i++)
		circle(drawing, sourcePoint[i], 2, Scalar(0, 0, 255), 2, 8);
	for (int i = 0; i < targetPoint.size(); i++)
		circle(drawing, targetPoint[i], 2, Scalar(255, 255, 0), 2, 8);

	double minDist = 0;
	int minPointIndex = 0;
	vector<Point> sourcePointPair;
	vector<Point> targetPointPair;
	//-----------------------------------
	//find nearest one from source to target
	for (int i = 0; i < sourcePoint.size(); i++) {
		for (int j = 0; j < targetPoint.size(); j++) {
			double dist = sqrt(pow(targetPoint[j].x - sourcePoint[i].x, 2) + pow(targetPoint[j].y - sourcePoint[i].y, 2));
			if (j == 0) {
				minDist = dist;
				minPointIndex = j;
			}
			else if (dist < minDist) {
				minDist = dist;
				minPointIndex = j;
			}
		}
		sourcePointPair.push_back(sourcePointOri[i]);
		targetPointPair.push_back(targetPoint[minPointIndex]);
		line(drawing, sourcePoint[i], targetPoint[minPointIndex], Scalar(255, 0, 255), 1, 8);
	}
	//-----------------------------------
	////find nearest one from target to source
	//for (int i = 0; i < targetPoint.size(); i++) {
	//	for (int j = 0; j < sourcePoint.size(); j++) {
	//		double dist = sqrt(pow(targetPoint[i].x - sourcePoint[j].x, 2) + pow(targetPoint[i].y - sourcePoint[j].y, 2));
	//		if (j == 0) {
	//			minDist = dist;
	//			minPointIndex = j;
	//		}
	//		else if (dist < minDist) {
	//			minDist = dist;
	//			minPointIndex = j;
	//		}
	//	}
	//	if (minDist < partialIcpThreshold) {
	//		sourcePointPair.push_back(sourcePointOri[minPointIndex]);
	//		targetPointPair.push_back(targetPoint[i]);
	//		line(drawing, sourcePoint[minPointIndex], targetPoint[i], Scalar(255, 255, 0), 1, 8);
	//	}
	//}
	//-----------------------------------
	if (sourcePointPair.size() == 0)
		return Mat();

	Mat warpMtx = getTwoSetsTransform(sourcePointPair, targetPointPair);
	/*cout << "warpMtx= " << warpMtx << endl;
	imwrite("Result0/" + to_string(times) + "_" + to_string(_currentIcpError) + ".png", drawing);
	system("pause");*/
	return warpMtx;
}

Mat solver::getTwoSetsTransform(vector<Point> &source, vector<Point> &target) {
	int pSize = source.size();
	
	MatrixXf A1 = MatrixXf::Zero(2 * pSize, 4);
	for (int i = 0; i < pSize; i++) {
		A1(2 * i + 0, 0) = source[i].x;
		A1(2 * i + 0, 1) = -source[i].y;
		A1(2 * i + 0, 2) = 1;
		A1(2 * i + 0, 3) = 0;

		A1(2 * i + 1, 0) = source[i].y;
		A1(2 * i + 1, 1) = source[i].x;
		A1(2 * i + 1, 2) = 0;
		A1(2 * i + 1, 3) = 1;
	}
	//std::cout << "Here is the matrix A:\n" << A1 << std::endl;

	VectorXf b1 = VectorXf::Zero(2 * pSize, 1);
	for (int i = 0; i < pSize; i++) {
		b1(2 * i + 0, 0) = target[i].x;
		b1(2 * i + 1, 0) = target[i].y;
	}
	//std::cout << "Here is the right hand side b:\n" << b1 << std::endl;

	//jacobiSvd 方式:Slow (but fast for small matrices)
	MatrixXf resultM = A1.jacobiSvd(ComputeThinU | ComputeThinV).solve(b1);
	/*std::cout << "The least-squares solution is:\n"
		<< resultM << std::endl;*/

	//colPivHouseholderQr方法:fast
	//MatrixXf resultM = A1.colPivHouseholderQr().solve(b1);
	/*std::cout << "The least-squares solution is:\n"
		<< A1.colPivHouseholderQr().solve(b1) << std::endl;*/

	Mat transform = Mat::zeros(Size(3, 2), CV_64F);
	transform.at<double>(0, 0) = resultM(0, 0);
	transform.at<double>(0, 1) = -resultM(1, 0);
	transform.at<double>(0, 2) = resultM(2, 0);
	transform.at<double>(1, 0) = resultM(1, 0);
	transform.at<double>(1, 1) = resultM(0, 0);
	transform.at<double>(1, 2) = resultM(3, 0);


	// remove scale
	//cout << "--------- " << transform << endl;
	double matScale = sqrt(pow(transform.at<double>(0, 0), 2) + pow(transform.at<double>(0, 1), 2));
	if (_compareWhole) {
		if (matScale > 1.2)
			matScale = 1.2;
		else if (matScale<0.8)
			matScale = 0.8;
	}

	transform.at<double>(0, 0) /= matScale;
	transform.at<double>(0, 1) /= matScale;
	transform.at<double>(1, 0) /= matScale;
	transform.at<double>(1, 1) /= matScale;

	Point c_targetCenter = getCenter(target);
	Point c_sourceCenter = getCenter(source);
	Point new_sourceCenter = c_sourceCenter;
	new_sourceCenter.x = transform.at<double>(0, 0)*c_sourceCenter.x + transform.at<double>(0, 1)*c_sourceCenter.y + transform.at<double>(0, 2);
	new_sourceCenter.y = transform.at<double>(1, 0)*c_sourceCenter.x + transform.at<double>(1, 1)*c_sourceCenter.y + transform.at<double>(1, 2);
	
	transform.at<double>(0, 2) += (c_targetCenter - new_sourceCenter).x;
	transform.at<double>(1, 2) += (c_targetCenter - new_sourceCenter).y;

	return transform;
}

// ------------ basic calculation ------------ 
Point solver::getCenter(vector<Point>& pointSeq) {
	Point center(0, 0);
	for (int i = 0; i < pointSeq.size(); i++) {
		center.x += pointSeq[i].x;
		center.y += pointSeq[i].y;
	}
	center.x /= pointSeq.size();
	center.y /= pointSeq.size();
	return center;
}

double solver::getMinDistanceVarErr(vector<Point> &source, vector<Point> &target) {
	Mat drawing = Mat::zeros(Size(500, 500), CV_8UC4);
	for (int i = 0; i < source.size(); i++)	
		circle(drawing, source[i], 2, Scalar(0, 255, 255, 255), 2, 8);
	for (int i = 0; i < target.size(); i++)	
		circle(drawing, target[i], 2, Scalar(0, 0, 255, 255), 2, 8);
	
	double dist = 0, minDist = 0, averageErr = 0;
	vector<double> distVec;
	//get icp error (for scale matrix)
	for (int i = 0; i < target.size(); i++)	{
		int minY = 0;
		for (int j = 0; j < source.size(); j++)	{
			dist = sqrt(pow(target[i].x - source[j].x, 2) + pow(target[i].y - source[j].y, 2));
			if (j == 0) {
				minY = j;
				minDist = dist;
			}
			else if (dist < minDist) {
				minY = j;
				minDist = dist;
			}
		}
		line(drawing, target[i], source[minY], Scalar(255, 0, 255, 255), 1, 8);

		averageErr += minDist;
		distVec.push_back(minDist);
	}

	averageErr /= (double)target.size();
	double vaError = 0;
	for (int i = 0; i < distVec.size(); i++) {
		vaError += pow((distVec[i] - averageErr), 2);
	}
	vaError /= target.size();
	//imwrite("Result0/drawing" + to_string(vaError) + ".png", drawing);
	return vaError;
}

double solver::getMinDistanceP(vector<Point> &source, vector<Point> &target) {
	// get norm vector of two sequential point
	vector<Point2f> target_normVec;
	for (int j = 0; j < target.size(); j++) {
		Point2f normVec = getNormVector(target[(j - 1 + target.size()) % target.size()], target[j], target[(j + 1) % target.size()]);
		target_normVec.push_back(normVec);
	}
	vector<Point2f> source_normVec;
	for (int j = 0; j < source.size(); j++) {
		Point2f normVec = getNormVector(source[(j - 1 + source.size()) % source.size()], source[j], source[(j + 1) % source.size()]);
		source_normVec.push_back(normVec);
	}

	Point2f averageVecErr = Point2f(0, 0);
	vector<Point2f> distPointVec;
	//get icp error (for scale matrix)
	for (int i = 0; i < target.size(); i++)	{
		Point2f differ = target[i] - source[i];
		//cout << "differ= " << differ << endl;;
		averageVecErr += differ;
		distPointVec.push_back(differ);
	}
	//cout << "averageVecErr= " << averageVecErr;
	averageVecErr.x /= (double)target.size();
	averageVecErr.y /= (double)target.size();
	//cout << ", averageVecErr= " << averageVecErr;

	double varPointError = 0;
	for (int i = 0; i < target.size(); i++) {
		distPointVec[i] -= averageVecErr;
		varPointError += pow(distPointVec[i].x, 2) + pow(distPointVec[i].y, 2);
	}
	//cout << ", varPointError= " << varPointError;
	varPointError /= (double)target.size();
	//system("pause");
	return varPointError;
}

Point2f solver::getNormVector(Point pre, Point tgt, Point nxt) {
	vector<double> nor1;
	vector<double> nor2;

	nor1.push_back(pre.y - tgt.y);
	nor1.push_back(tgt.x - pre.x);
	nor2.push_back(tgt.y - nxt.y);
	nor2.push_back(nxt.x - tgt.x);

	Point2f norT(nor1[0] + nor2[0], nor1[1] + nor2[1]);

	double lengthN = sqrt(norT.x * norT.x + norT.y * norT.y);
	norT.x /= lengthN;
	norT.y /= lengthN;
	return norT;
}

double solver::getTheta(Mat &warpMatrixs) {
	double dTheta = 0;
	if (warpMatrixs.at<double>(0, 0)>0 && warpMatrixs.at<double>(1, 0)>0) { // cos+ sin+
		dTheta = acos(warpMatrixs.at<double>(0, 0));
	}
	else if (warpMatrixs.at<double>(0, 0)>0 && warpMatrixs.at<double>(1, 0)<0) { // + -
		dTheta = asin(warpMatrixs.at<double>(0, 1));
	}
	else if (warpMatrixs.at<double>(0, 0)<0 && warpMatrixs.at<double>(1, 0)>0) { // - +
		dTheta = acos(warpMatrixs.at<double>(0, 0));
	}
	else { // - -
		dTheta = acos(warpMatrixs.at<double>(0, 0));
		dTheta = 2 * PI - dTheta;
	}
	return dTheta;
}

// ------------ return value region ------------ 
Mat solver::getResultMat() {
	return _resultMat;
}

vector<Point> solver::getResultPointSeq() {
	return _newPointSeq;
}

double solver::getIcpVarErr() {
	return _varErr;
}
