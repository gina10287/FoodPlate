/// ¥i¥H¶]ªºª©

#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include <map>
#include <fstream>
#include <dirent.h>
#include <thread>

#include "compare.h"
#include "fragment.h"
#include "descri.h"
#include "food.h"
#include "Library\GridCut\GridGraph_2D_4C.h"
#include "icpSolver.h"

# define PI 3.1415926


using namespace std;
using namespace cv;

bool compareWithLength(map<string, int> input1, map<string, int> input2)
{
	int i = input1["l"];
	int j = input2["l"];
	return(i<j);
}

static vector<double> norVec(Point pre, Point tgt, Point nxt) {
	vector<double> nor1;
	vector<double> nor2;

	nor1.push_back(pre.y - tgt.y);
	nor1.push_back(tgt.x - pre.x);
	nor2.push_back(tgt.y - nxt.y);
	nor2.push_back(nxt.x - tgt.x);

	vector<double> norT;
	norT.push_back(nor1[0] + nor2[0]);
	norT.push_back(nor1[1] + nor2[1]);

	double lengthN = sqrt(norT[0] * norT[0] + norT[1] * norT[1]);
	norT[0] /= lengthN;
	norT[1] /= lengthN;

	return norT;
}

void getCenterPoint(vector<Point>& pointSeq, double& cx, double& cy) {
	cx = 0;
	cy = 0;
	for (int i = 0; i < pointSeq.size(); i++) {
		cx += pointSeq[i].x;
		cy += pointSeq[i].y;
	}
	cx /= pointSeq.size();
	cy /= pointSeq.size();
}

void getCenterPointInt(vector<Point>& pointSeq, int& x, int& y) {
	double cx = 0;
	double cy = 0;
	for (int i = 0; i < pointSeq.size(); i++) {
		cx += pointSeq[i].x;
		cy += pointSeq[i].y;
	}
	cx /= pointSeq.size();
	cy /= pointSeq.size();
	x = cx;
	y = cy;
}

void removeScaleOfWarpMat(Mat &matt) {
	double scale = pow(matt.at<double>(0, 0), 2) + pow(matt.at<double>(0, 1), 2);
	matt.at<double>(0, 0) /= scale;
	matt.at<double>(0, 1) /= scale;
	//matt.at<double>(0, 2) /= scale;
	matt.at<double>(1, 0) /= scale;
	matt.at<double>(1, 1) /= scale;
	//matt.at<double>(1, 2) /= scale;
}

// Bidirectional icp
Mat comp::icp(vector<Point> &contourPoint, vector<Point> &foodPointOri, vector<Point> &foodPoint, vector<int> &contourPair, vector<int> &foodPair, double &dX, double &dY, double &dTheta) {
	// contour points: Q
	// food points: P, P', P''...
	double minDist = -1;
	int minPointIndex;

	Mat d = Mat::zeros(_inputSize, CV_8UC3);
	drawContours(d, vector<vector<Point> >(1, contourPoint), 0, Scalar(0, 0, 255), 1, 8);
	drawContours(d, vector<vector<Point> >(1, foodPoint), 0, Scalar(255, 255, 0), 1, 8);
	circle(d, Point(meanCX, meanCY), 5, Scalar(0, 0, 255), -1);
	circle(d, Point(meanFX, meanFY), 3, Scalar(255, 0, 0), -1);
	vector<double> distVec;

	vector<vector<double> > Contour_normVec;
	for (int j = 0; j < contourPoint.size(); j++) {
		vector<double> normVec = norVec(contourPoint[(j - 1 + contourPoint.size()) % contourPoint.size()], contourPoint[j], contourPoint[(j + 1) % contourPoint.size()]);
		Contour_normVec.push_back(normVec);
	}

	vector<vector<double> > Food_normVec;
	for (int j = 0; j < foodPoint.size(); j++) {
		vector<double> normVec = norVec(foodPoint[(j - 1 + foodPoint.size()) % foodPoint.size()], foodPoint[j], foodPoint[(j + 1) % foodPoint.size()]);
		Food_normVec.push_back(normVec);
	}

	// for every point in contourPoint, find a closest point in foodPoint
	for (int j = 0; j < contourPoint.size(); j++) {
		minDist = -1;
		vector<double> normVec = Contour_normVec[j];

		for (int i = 0; i < foodPoint.size(); i++) {
			double dist = sqrt(pow(contourPoint[j].x - foodPoint[i].x, 2) + pow(contourPoint[j].y - foodPoint[i].y, 2));
			
			/*vector<double> normFC;
			normFC.push_back(foodPoint[i].x - contourPoint[j].x);
			normFC.push_back(foodPoint[i].y - contourPoint[j].y);
			double lengthFC = sqrt(pow(normFC[0], 2) + pow(normFC[1], 2));
			normFC[0] /= lengthFC;
			normFC[1] /= lengthFC;*/
			vector<double> normFC = Food_normVec[i];

			//dist*(1-A dot B)
			double value = dist *(1 - (normVec[0] * normFC[0] + normVec[1] * normFC[1]) / sqrt(2));

			if (minDist == -1) {
				minDist = value;
				minPointIndex = i;
			}
			else if (value < minDist) {
				minDist = value;
				minPointIndex = i;
			}
		}
		contourPair[j] = minPointIndex;
	}

	// for every point in foodPoint, find a closest point in contourPoint
	for (int i = 0; i < foodPoint.size(); i++) {
		minDist = -1;
		vector<double> normVec = Food_normVec[i];
		
		for (int j = 0; j < contourPoint.size(); j++) {
			double dist = sqrt(pow(contourPoint[j].x - foodPoint[i].x, 2) + pow(contourPoint[j].y - foodPoint[i].y, 2));

			/*vector<double> normFC;
			normFC.push_back(contourPoint[j].x - foodPoint[i].x);
			normFC.push_back(contourPoint[j].y - foodPoint[i].y);
			double lengthFC = sqrt(pow(normFC[0], 2) + pow(normFC[1], 2));
			normFC[0] /= lengthFC;
			normFC[1] /= lengthFC;*/
			vector<double> normFC = Contour_normVec[j];

			//dist*(1-A dot B)
			double value = dist *(1 - (normVec[0] * normFC[0] + normVec[1] * normFC[1]) / sqrt(2));

			if (minDist == -1) {
				minDist = value;
				minPointIndex = j;
			}
			else if (value < minDist) {
				minDist = value;
				minPointIndex = j;
			}
		}
		foodPair[i] = minPointIndex;
	}
	
	double sumFXCY = 0;
	double sumFYCX = 0;
	double sumFXCX = 0;
	double sumFYCY = 0;
	for (int i = 0; i < contourPair.size(); i++) {
		line(d, contourPoint[i], foodPoint[contourPair[i]], Scalar(0, 0, 255), 2, 8);
		sumFXCY += (foodPointOri[contourPair[i]].x - meanFX) * (contourPoint[i].y - meanCY);
		sumFYCX += (foodPointOri[contourPair[i]].y - meanFY) * (contourPoint[i].x - meanCX);
		sumFXCX += (foodPointOri[contourPair[i]].x - meanFX) * (contourPoint[i].x - meanCX);
		sumFYCY += (foodPointOri[contourPair[i]].y - meanFY) * (contourPoint[i].y - meanCY);
	}
	for (int i = 0; i < foodPointOri.size(); i++) {
		line(d, contourPoint[foodPair[i]], foodPoint[i], Scalar(255, 255, 0), 1, 8);
		sumFXCY += (foodPointOri[i].x - meanFX) * (contourPoint[foodPair[i]].y - meanCY);
		sumFYCX += (foodPointOri[i].y - meanFY) * (contourPoint[foodPair[i]].x - meanCX);
		sumFXCX += (foodPointOri[i].x - meanFX) * (contourPoint[foodPair[i]].x - meanCX);
		sumFYCY += (foodPointOri[i].y - meanFY) * (contourPoint[foodPair[i]].y - meanCY);
	}

	Mat H = Mat::zeros(Size(2, 2), CV_64FC1);
	H.at<double>(0, 0) = sumFXCX;
	H.at<double>(0, 1) = sumFYCX;
	H.at<double>(1, 0) = sumFXCY;
	H.at<double>(1, 1) = sumFYCY;

	SVD s(H);
	Mat R = s.vt.t()*s.u.t();
	if (R.at<double>(0, 0)>0 && R.at<double>(0, 1)>0) { // cos+ sin+
		dTheta = acos(R.at<double>(0, 0));
	}
	else if (R.at<double>(0, 0)>0 && R.at<double>(0, 1)<0) { // + -
		dTheta = asin(R.at<double>(0, 1));
	}
	else if (R.at<double>(0, 0)<0 && R.at<double>(0, 1)>0) { // - +
		dTheta = acos(R.at<double>(0, 0));
	}
	else { // - -
		dTheta = acos(R.at<double>(0, 0));
		dTheta = 2 * PI - dTheta;
	}

	dX = meanCX - (meanFX*cos(dTheta) - meanFY*sin(dTheta));
	dY = meanCY - (meanFX*sin(dTheta) + meanFY*cos(dTheta));
	Mat newWarpMat = Mat::zeros(Size(3, 2), CV_64F);
	newWarpMat.at<double>(0, 0) = R.at<double>(0, 0);
	newWarpMat.at<double>(0, 1) = R.at<double>(1, 0);
	newWarpMat.at<double>(1, 0) = R.at<double>(0, 1);
	newWarpMat.at<double>(1, 1) = R.at<double>(1, 1);
	
	/*cout << R << endl;
	cout << newWarpMat << endl;
	cout << "dTheta= " << (dTheta / PI) * 180 << endl;
	imwrite("Result0/d_" + to_string(timess) + "_" + to_string(dTheta) + ".png", d);
	system("pause");*/
	return newWarpMat;
}

double lastMinValue = 10;
vector<int> tmp_contourIndex;
vector<int> tmp_foodIndex;
// one Directional icp, do not need to warp to center
Mat comp::icp1(vector<Point> &contourPoint, vector<Point> &foodPointOri, vector<Point> &foodPoint, double &dX, double &dY, double &dTheta) {
	// contour points: Q
	// food points: P, P', P''...
	
	Mat d = Mat::zeros(_inputSize, CV_8UC3);
	for (int i = 0; i < contourPoint.size(); i++)
		circle(d, contourPoint[i], 2, Scalar(0, 0, 255), 1, 8);
	for (int i = 0; i < foodPoint.size(); i++)
		circle(d, foodPoint[i], 2, Scalar(255, 255, 0), 1, 8);
	if (timess > 0) {
		for (int i = 0; i < tmp_contourIndex.size(); i++)
			line(d, foodPoint[tmp_foodIndex[i]], contourPoint[tmp_contourIndex[i]], Scalar(255, 0, 255), 1, 8);
	}
	
	vector<vector<double> > Contour_normVec;
	for (int j = 0; j < contourPoint.size(); j++) {
		vector<double> normVec = norVec(contourPoint[(j - 1 + contourPoint.size()) % contourPoint.size()], contourPoint[j], contourPoint[(j + 1) % contourPoint.size()]);
		Contour_normVec.push_back(normVec);
	}
	vector<vector<double> > Food_normVec;
	for (int j = 0; j < foodPoint.size(); j++) {
		vector<double> normVec = norVec(foodPoint[(j - 1 + foodPoint.size()) % foodPoint.size()], foodPoint[j], foodPoint[(j + 1) % foodPoint.size()]);
		Food_normVec.push_back(normVec);
	}

	double minDist = -1;
	int minPointIndex = 0;

	tmp_contourIndex.clear();
	tmp_foodIndex.clear();
	// for every point in foodPoint, find a closest point in contourPoint
	for (int i = 0; i < foodPoint.size(); i++) {
		minDist = -1;
		vector<double> normVec = Food_normVec[i];

		for (int j = 0; j < contourPoint.size(); j++) {
			double dist = sqrt(pow(contourPoint[j].x - foodPoint[i].x, 2) + pow(contourPoint[j].y - foodPoint[i].y, 2));
			vector<double> normFC = Contour_normVec[j];
			//dist*(1-A dot B)
			double value = dist *(1 - (normVec[0] * normFC[0] + normVec[1] * normFC[1]) / sqrt(2));

			if (minDist == -1) {
				minDist = value;
				minPointIndex = j;
			}
			else if (value < minDist) {
				minDist = value;
				minPointIndex = j;
			}
		}

		if (minDist < lastMinValue) {
			tmp_contourIndex.push_back(minPointIndex);
			tmp_foodIndex.push_back(i);
		}
	}

	double c_meanCX, c_meanCY, c_meanFX, c_meanFY;
	vector<Point> tmp_foodOriVec;
	vector<Point> tmp_contourVec;
	vector<Point> tmp_foodVec;
	for (int i = 0; i < tmp_contourIndex.size(); i++) {
		tmp_foodOriVec.push_back(foodPointOri[tmp_foodIndex[i]]);
		tmp_contourVec.push_back(contourPoint[tmp_contourIndex[i]]);
		tmp_foodVec.push_back(foodPoint[tmp_foodIndex[i]]);
	}


	for (int i = 0; i < tmp_foodOriVec.size(); i++)
		circle(d, tmp_foodOriVec[i], 2, Scalar(255, 255, 255), 3, 8);

	getCenterPoint(tmp_contourVec, c_meanCX, c_meanCY);
	getCenterPoint(tmp_foodOriVec, c_meanFX, c_meanFY);
	Point shiftCenter(c_meanCX - c_meanFX, c_meanCY - c_meanFY);
	c_meanFX = c_meanCX;
	c_meanFY = c_meanCY;
	circle(d, Point(c_meanCX, c_meanCY), 5, Scalar(0, 0, 255), -1);
	circle(d, Point(c_meanFX, c_meanFY), 3, Scalar(255, 0, 0), -1);

	double sumFXCY = 0;
	double sumFYCX = 0; 
	double sumFXCX = 0;
	double sumFYCY = 0;
	for (int i = 0; i < tmp_contourIndex.size(); i++) {
		line(d, tmp_contourVec[i], tmp_foodVec[i], Scalar(255, 255, 0), 1, 8);
		sumFXCY += (tmp_foodOriVec[i].x - c_meanFX) * (tmp_contourVec[i].y - c_meanCY);
		sumFYCX += (tmp_foodOriVec[i].y - c_meanFY) * (tmp_contourVec[i].x - c_meanCX);
		sumFXCX += (tmp_foodOriVec[i].x - c_meanFX) * (tmp_contourVec[i].x - c_meanCX);
		sumFYCY += (tmp_foodOriVec[i].y - c_meanFY) * (tmp_contourVec[i].y - c_meanCY);
	}

	sumFXCY = 4;
	sumFYCX = 0;
	sumFXCX = 8;
	sumFYCY = 0;

	Mat H = Mat::zeros(Size(2, 2), CV_64FC1);
	H.at<double>(0, 0) = sumFXCX;
	H.at<double>(0, 1) = sumFYCX;
	H.at<double>(1, 0) = sumFXCY;
	H.at<double>(1, 1) = sumFYCY;

	

	SVD s(H);
	Mat R = s.vt*s.u.t();
	//Mat R = (s.vt*s.u.t()).t();
	

	// get theta
	if (R.at<double>(0, 0)>0 && R.at<double>(0, 1)>0) { // cos+ sin+
		dTheta = acos(R.at<double>(0, 0));
	}
	else if (R.at<double>(0, 0)>0 && R.at<double>(0, 1)<0) { // + -
		dTheta = asin(R.at<double>(0, 1));
	}
	else if (R.at<double>(0, 0)<0 && R.at<double>(0, 1)>0) { // - +
		dTheta = acos(R.at<double>(0, 0));
	}
	else { // - -
		dTheta = acos(R.at<double>(0, 0));
		dTheta = 2 * PI - dTheta;
	}
	cout << "dTheta= " << (dTheta/PI)*180 << endl;

	Mat newWarpMat = Mat::zeros(Size(3, 2), CV_64F);
	newWarpMat.at<double>(0, 0) = R.at<double>(0, 0);
	newWarpMat.at<double>(0, 1) = R.at<double>(1, 0);
	newWarpMat.at<double>(0, 2) = shiftCenter.x;
	newWarpMat.at<double>(1, 0) = R.at<double>(0, 1);
	newWarpMat.at<double>(1, 1) = R.at<double>(1, 1);
	newWarpMat.at<double>(0, 2) = shiftCenter.y;

	cout << R << endl;
	cout << newWarpMat << endl;
	cout << "dTheta= " << (dTheta / PI) * 180 << endl;

	for (int i = 0; i < tmp_foodOriVec.size(); i++) {
		cout << tmp_foodOriVec[i] << ", ";
		double newX = newWarpMat.at<double>(0, 0)*(double)tmp_foodOriVec[i].x + newWarpMat.at<double>(0, 1)* tmp_foodOriVec[i].y;
		double newY = newWarpMat.at<double>(1, 0)*(double)tmp_foodOriVec[i].x + newWarpMat.at<double>(1, 1)* tmp_foodOriVec[i].y;
		tmp_foodOriVec[i] = Point(newX, newY);
		cout << tmp_foodOriVec[i] << endl;;
	}
	for (int i = 0; i < tmp_foodOriVec.size(); i++)
		circle(d, tmp_foodOriVec[i], 2, Scalar(0, 255, 255), 2, 8);

	getCenterPoint(tmp_foodOriVec, c_meanFX, c_meanFY);

	cout << "FX= " << c_meanFX << ", FY= " << c_meanFY << endl;
	cout << "CX= " << c_meanCX << ", CY= " << c_meanCX << endl;
	dX = c_meanCX - c_meanFX;
	dY = c_meanCY - c_meanFY;
	newWarpMat.at<double>(0, 2) = dX;
	newWarpMat.at<double>(1, 2) = dY;

	cout << "dX= " << dX << ", dY= " << dY << endl;
	imwrite("Result0/d_" + to_string(timess) + "_" + to_string(dTheta) + ".png", d);
	system("pause");
	return newWarpMat;
}

// constructor for group
comp::comp(Size inputSize, bool isReverse) {
	//setInitial();
	_inputSize = inputSize;
	_isReverse = isReverse;
}

// constructor of gridcut
comp::comp(Size inputSize, vector<Point> &pointSeq1, int contourIndex, int foodIndex, Mat dood) {
	_inputSize = inputSize;
	_pointSeq1 = pointSeq1;
	_cIndex = contourIndex;
	_fIndex = foodIndex;

	clock_t s = clock();
	gridCutFood(dood);
	clock_t sf = clock(); //cout << "total compare time: " << sf - s << endl;
}

// constructor of combination
comp::comp(bool stackable, double lLimit, Size inputSize, vector<Mat> &descri1Seq, vector<Mat> &descri2Seq, vector<Point> &pointSeq1, vector<Point> &pointSeq2, int contourIndex, int foodIndex, Mat dood, double scaleSize, vector<Point> &nonAlphaOfFood) {
	_stackable = stackable;
	_lLimit = lLimit;
	_inputSize = inputSize;
	_pointSeq1 = pointSeq1;
	_pointSeq2 = pointSeq2;
	_rDesSize = descri1Seq.size();
	_qDesSize = descri2Seq.size();
	_cIndex = contourIndex;
	_fIndex = foodIndex;
	setInitial();


	//for (int range = _qDesSize)




	Mat ress = Mat::zeros(_inputSize, CV_8UC4);
	for (int p = 0; p < pointSeq1.size(); p++) {
		circle(ress, pointSeq1[p], 2, Scalar(255, 0, 0, 255), 2);
	}
	for (int p = 0; p < pointSeq2.size(); p++) {
		circle(ress, pointSeq2[p], 2, Scalar(0, 255, 0, 255), 2);
	}
	//imwrite("Result0/0002.png", ress);
	ddood = dood.clone();

	clock_t s = clock();
	cout << "!: " << _fIndex << ", _rDesSize= " << _rDesSize << ", _qDesSize= " << _qDesSize << endl;
	if (_rDesSize > 0 && _qDesSize > 0/* && _qDesSize < 2 *_rDesSize*/) {
		clock_t s1 = clock();
		if (_rDesSize >= _qDesSize)	{
			for (int i = 0; i < _rDesSize; i++)
				compareDesN(descri1Seq[i], descri2Seq[0], i, true);
		}
		else {
			for (int i = 0; i < _qDesSize; i++)
				compareDesN(descri1Seq[0], descri2Seq[i], i, false);
		}
		clock_t s2 = clock();
		//cout << "compareDesN time: " << s2 - s1 << endl;

		if (candidateResult.size() > 0) {
			findCombination3(dood, nonAlphaOfFood);
		}
		clock_t s3 = clock();
		//cout << "Combination time: " << s3 - s2 << endl;
	}
	
	//clock_t sf = clock(); cout << "total compare time: " << sf - s << endl;


}

// constructor of oneToOne
comp::comp(double lLimit, Size inputSize, vector<Mat> &descri1Seq, vector<Mat> &descri2Seq, vector<Point> &pointSeq1, vector<Point> &pointSeq2) {
	_lLimit = lLimit;
	_inputSize = inputSize;
	_pointSeq1 = pointSeq1;
	_pointSeq2 = pointSeq2;
	_rDesSize = descri1Seq.size();
	_qDesSize = descri2Seq.size();
	setInitial();

	Mat ress = Mat::zeros(_inputSize, CV_8UC4);
	for (int p = 0; p < pointSeq1.size(); p++) {
		circle(ress, pointSeq1[p], 2, Scalar(255, 0, 0, 255), 2);
	}
	for (int p = 0; p < pointSeq2.size(); p++) {
		circle(ress, pointSeq2[p], 2, Scalar(0, 255, 0, 255), 2);
	}

	clock_t s = clock();
	_warpMatrixOrNot = false;
	cout << "!: " << _fIndex << ", _rDesSize= " << _rDesSize << ", _qDesSize= " << _qDesSize << endl;
	if (_rDesSize > 0 && _qDesSize > 0/* && _qDesSize < 2 *_rDesSize*/) {
		clock_t s1 = clock();
		if (_rDesSize >= _qDesSize)	{
			for (int i = 0; i < _rDesSize; i++)
				compareDesN(descri1Seq[i], descri2Seq[0], i, true);
		}
		else {
			for (int i = 0; i < _qDesSize; i++)
				compareDesN(descri1Seq[0], descri2Seq[i], i, false);
		}
		clock_t s2 = clock();
		cout << "compareDesN time: " << s2 - s1 << endl;

		while (candidateResult.size() > 0) {
			int maxIdx = 0, maxRange = 0;
			for (int i = 0; i < candidateResult.size(); i++) {
				if (candidateResult[i].range > maxRange) {
					maxRange = candidateResult[i].range;
					maxIdx = i;
				}
			}
			_warpMatrix = estimateRigidTransform(candidateResult[maxIdx].matchSeqQ, candidateResult[maxIdx].matchSeqR, false);
			if (_warpMatrix.size() != cv::Size(0, 0)) {
				_warpMatrixOrNot = true;
				removeScaleOfWarpMat(_warpMatrix);
				break;
			}
			else {
				candidateResult.erase(candidateResult.begin() + maxIdx);
			}
		}
	}

	clock_t sf = clock(); cout << "total compare time: " << sf - s << endl;
}

//set initial
void comp::setInitial() {
	_thresholdScore = 1000.0;
	_minScore = _thresholdScore;
	initScore = false;
	_startIndex1 = 0;
	_startIndex2 = 0;
	_range = 0;
	_mapRQ = Mat::zeros(Size(_qDesSize, _rDesSize), CV_32S); //Size(q, r): x is q, y is r
	_mapScore = Mat::zeros(Size(_qDesSize, _rDesSize), CV_64F); //Size(q, r): x is q, y is r
	_mapCandidateIdx = Mat::zeros(Size(_qDesSize, _rDesSize), CV_32S); //Size(q, r): x is q, y is r
	_isGroup = false;
}

Mat alphaBinary2(Mat input) {
	Mat alphaOrNot = Mat::zeros(input.size(), CV_8UC1);
	for (int i = 0; i < input.cols; i++) {
		for (int j = 0; j < input.rows; j++) {
			Vec4b & bgra = input.at<Vec4b>(j, i);
			if (bgra[3] != 0) // not transparency
				alphaOrNot.at<uchar>(j, i) = 255;
			else
				alphaOrNot.at<uchar>(j, i) = 0;
		}
	}
	return alphaOrNot;
}

// add image with transparent background
Mat addTransparent_Comp(Mat &bg, Mat &fg) {
	Mat result;
	bg.copyTo(result);
	for (int y = 0; y < bg.rows; ++y) {
		int fY = y;
		if (fY >= fg.rows) break;
		for (int x = 0; x < bg.cols; ++x) {
			int fX = x;
			if (fX >= fg.cols) break;
			double Fopacity = ((double)fg.data[fY * fg.step + fX * fg.channels() + 3]) / 255; // opacity of the foreground pixel

			for (int c = 0; Fopacity > 0 && c < result.channels(); ++c) { // combine the background and foreground pixel
				unsigned char foregroundPx = fg.data[fY * fg.step + fX * fg.channels() + c];
				unsigned char backgroundPx = bg.data[y * bg.step + x * bg.channels() + c];
				result.data[y*result.step + result.channels()*x + c] = backgroundPx * (1. - Fopacity) + foregroundPx * Fopacity;
			}
		}
	}
	return result;
}

bool comp::stackOrNot(Size c_inputSize, Mat &warpImg, Mat &alphMat, double threshold) {
	// test if it's overlap area is enough
	int overlapCount = 0, foodArea = 0;
	for (int ci = 0; ci < c_inputSize.width; ci++) {
		for (int cj = 0; cj < c_inputSize.height; cj++) {
			if (warpImg.at<Vec4b>(cj, ci)[3] != 0) {
				foodArea++;
				if (alphMat.at<uchar>(cj, ci) == 255) {
					overlapCount++;
					alphMat.at<uchar>(cj, ci) = 0;
				}
			}
		}
	}

	if (((double)overlapCount / (double)foodArea)>threshold) {
		return true;
	}
	else {
		return false;
	}
}

bool compareWithRange(candidateInfo input1, candidateInfo input2) {
	double i = input1.range;
	double j = input2.range;
	return(i>j);
}

bool compareWithstartIdx(candidateInfo input1, candidateInfo input2) {
	double i = input1.startIndex1;
	double j = input2.startIndex1;
	if (i == j) {
		double ii = input1.range;
		double jj = input2.range;
		return(ii < jj);
	}
	return(i>j);
}

Mat comp::testWarpingImg(int starIdx1, int starIdx2, int range, Mat& warpMat) {
	// write img and test
	Mat wwwarpImg = Mat::zeros(_inputSize, CV_8UC4);
	warpAffine(ddood, wwwarpImg, warpMat, _inputSize);
	for (int rr = 0; rr < _pointSeq1.size(); rr++) {
		circle(wwwarpImg, _pointSeq1[rr], 1, Scalar(0, 0, 255, 255), 1);
	}
	for (int rr = 0; rr < _pointSeq2.size(); rr++) {
		double newWX = warpMat.at<double>(0, 0)*_pointSeq2[rr].x + warpMat.at<double>(0, 1)*_pointSeq2[rr].y + warpMat.at<double>(0, 2);
		double newWY = warpMat.at<double>(1, 0)*_pointSeq2[rr].x + warpMat.at<double>(1, 1)*_pointSeq2[rr].y + warpMat.at<double>(1, 2);
		circle(wwwarpImg, Point(newWX, newWY), 1, Scalar(0, 255, 0, 255), 1);
	}

	for (int rr = 0; rr < 1; rr++) {
		circle(wwwarpImg, _pointSeq1[starIdx1 + rr], 3, Scalar(0, 0, 255, 255), 3);
		double newWX = warpMat.at<double>(0, 0)*_pointSeq2[starIdx2 + rr].x + warpMat.at<double>(0, 1)*_pointSeq2[starIdx2 + rr].y + warpMat.at<double>(0, 2);
		double newWY = warpMat.at<double>(1, 0)*_pointSeq2[starIdx2 + rr].x + warpMat.at<double>(1, 1)*_pointSeq2[starIdx2 + rr].y + warpMat.at<double>(1, 2);
		circle(wwwarpImg, Point(newWX, newWY), 3, Scalar(0, 255, 0, 255), 3);
	}
	for (int rr = 1; rr < range; rr++) {
		int idx1 = (starIdx1 + rr) % _pointSeq1.size();
		circle(wwwarpImg, _pointSeq1[idx1], 2, Scalar(0, 0, 255, 255), 2);
		int idx2 = (starIdx2 + rr) % _pointSeq2.size();
		double newWX = warpMat.at<double>(0, 0)*_pointSeq2[idx2].x + warpMat.at<double>(0, 1)*_pointSeq2[idx2].y + warpMat.at<double>(0, 2);
		double newWY = warpMat.at<double>(1, 0)*_pointSeq2[idx2].x + warpMat.at<double>(1, 1)*_pointSeq2[idx2].y + warpMat.at<double>(1, 2);
		circle(wwwarpImg, Point(newWX, newWY), 2, Scalar(0, 255, 0, 255), 2);
	}
	//imwrite("Result0/wwwarpImg.png", wwwarpImg);
	//system("pause");
	return wwwarpImg;
}

// left right top bottom
vector<Point> getValuePart(Mat img) {
	vector<Point> result;
	for (int i = 0; i < img.cols; i++) {
		for (int j = 0; j < img.rows; j++) {
			if (img.at<Vec4b>(j, i)[3] != 0) {
				result.push_back(Point(i,j));
			}
		}
	}
	return result;
}

void warpAffineFct(vector<Point> &pointSeq, Mat &source, Mat &target, Mat &warpMatrix) {
	for (int i = 0; i < pointSeq.size(); i++) {
		double newX = warpMatrix.at<double>(0, 0)*pointSeq[i].x + warpMatrix.at<double>(0, 1)*pointSeq[i].y + warpMatrix.at<double>(0, 2);
		double newY = warpMatrix.at<double>(1, 0)*pointSeq[i].x + warpMatrix.at<double>(1, 1)*pointSeq[i].y + warpMatrix.at<double>(1, 2);
		target.at<Vec4b>(newY, newX) = source.at<Vec4b>(pointSeq[i].y, pointSeq[i].x);
	}
}

void comp::findCombination(Mat &doodImg, vector<Point> &nonAlphaOfFood) {
	clock_t s0 = clock();
	//cout << "-------------- s0" << endl;

	// initial parameters
	int foodAmount = 0;
	Mat originImg = Mat::zeros(_inputSize, CV_8UC4);
	drawContours(originImg, vector<vector<Point>>(1, _pointSeq1), 0, Scalar(255, 255, 255, 255), CV_FILLED);
	Mat originAlphaBin = alphaBinary2(originImg);

	// getValuePoint, alpha!=0
	vector<Point> foodValuePoint = getValuePart(doodImg);

	// sort by startIndex1 and then range
	sort(candidateResult.begin(), candidateResult.end(), compareWithstartIdx);
	//for (int i = candidateResult.size() - 1; i >= 0; i--) 
		//cout << "startIndex=" << candidateResult[i].startIndex1 << ", _range= " << candidateResult[i].range << endl;
	

	// to determine if food is in contour
	//take only the longest range of the same startIndex1
	bool hasFindFirst = false;
	vector<bool> canBeStart(_pointSeq1.size(), true);
	for (int i = candidateResult.size() - 1; i >= 0; i--) {
		if ((!hasFindFirst || candidateResult[i].startIndex1 != candidateResult[i + 1].startIndex1) && canBeStart[candidateResult[i].startIndex1]) {
			//cout << "****** (startIndex)=" << candidateResult[i].startIndex1 << ", _range= " << candidateResult[i].range;
			//cout << ", (startIndex+1)=" << candidateResult[i+1].startIndex1 << ", _range= " << candidateResult[i+1].range << endl;
			candidateResult[i].warpMatrix = estimateRigidTransform(candidateResult[i].matchSeqQ, candidateResult[i].matchSeqR, false);

			if (candidateResult[i].warpMatrix.size() != cv::Size(0, 0)) {
				removeScaleOfWarpMat(candidateResult[i].warpMatrix);
				candidateResult[i].warpImg = Mat::zeros(_inputSize, CV_8UC4);

				//clock_t c0 = clock();
				//warpAffineFct(foodValuePoint, doodImg, candidateResult[i].warpImg, candidateResult[i].warpMatrix);
				//clock_t c1 = clock();
				//cout << "c1-c0= " << c1 - c0;

				warpAffine(doodImg, candidateResult[i].warpImg, candidateResult[i].warpMatrix, _inputSize);
				//clock_t c2 = clock();
				//cout << ", c2-c1= " << c2 - c1;

				Mat originAlphaBinTmp = originAlphaBin.clone();
				//imwrite("Result0/aaa.png", candidateResult[i].warpImg);
				//system("pause");

				for (int cL = 0; cL < candidateResult[i].range / 3; cL++) {
					int cc_Idx = (candidateResult[i].startIndex1 + cL) % canBeStart.size();
					canBeStart[cc_Idx] = false;
				}
				//hasFindFirst = true;
				// if food is in contour
				if (stackOrNot(_inputSize, candidateResult[i].warpImg, originAlphaBinTmp, 0.7))
					hasFindFirst = true;
				else
					candidateResult.erase(candidateResult.begin() + i);
			}
			else
				candidateResult.erase(candidateResult.begin() + i);
		}
		else {
			candidateResult.erase(candidateResult.begin() + i);
		}
	}

	if (candidateResult.size() <= 0)
		return;
	else
		foodAmount = 1;

	// find max range in candidateResult index
	int maxIdx = 0, maxRange = -1;
	for (int i = 0; i < candidateResult.size(); i++) {
		if (candidateResult[i].range > maxRange) {
			maxIdx = i;
			maxRange = candidateResult[i].range;
		}
	}
	for (int i = 0; i < candidateResult.size(); i++) {
		candidateResult[i].fake_startIndex1 = (candidateResult[i].startIndex1 - candidateResult[maxIdx].startIndex1) % _rDesSize;
		candidateResult[i].fake_startIndex2 = (candidateResult[i].fake_startIndex1 + (candidateResult[i].range / 2)) % _rDesSize;
	}
	clock_t s1 = clock();
	//cout << "s1= " << (s1 - s0)  << endl;

	double stackThreshold = 0.4;
	if (!_stackable) { stackThreshold = 0.7; }

	// get current originAlphaBin
	Mat resultStack = candidateResult[maxIdx].warpImg.clone();
	for (int i = 0; i < originAlphaBin.cols; i++) {
		for (int j = 0; j < originAlphaBin.rows; j++) {
			originAlphaBin.at<uchar>(j, i) = 255;
		}
	}
	stackOrNot(_inputSize, candidateResult[maxIdx].warpImg, originAlphaBin, stackThreshold);

	int lastIdx = maxIdx;
	for (int i = 1; i < candidateResult.size(); i++) {
		int currentIdx = (maxIdx + i) % candidateResult.size();
		Mat originAlphaBinTmp = originAlphaBin.clone();

		if ((candidateResult[currentIdx].startIndex1>candidateResult[lastIdx].startIndex2) && stackOrNot(_inputSize, candidateResult[currentIdx].warpImg, originAlphaBinTmp, stackThreshold)) {
		//if (true) {
			foodAmount++;
			originAlphaBin = originAlphaBinTmp.clone();
			resultStack = addTransparent_Comp(resultStack, candidateResult[currentIdx].warpImg);
			//imwrite("Result0/originAlphaBinTmp.png", originAlphaBinTmp);
			testWarpingImg(candidateResult[currentIdx].startIndex1, candidateResult[currentIdx].startIndex2, candidateResult[currentIdx].range, candidateResult[currentIdx].warpMatrix);
			//imwrite("Result0/imGG.png", resultStack);
			//system("pause");
			lastIdx = currentIdx;
		}
		else {
		//imwrite("Result0/originAlphaBinTmp.png", originAlphaBinTmp);
		//system("pause");
		}
	}
	clock_t s2 = clock();
	//cout << "s2= " << (s2 - s1)  << endl;

	int currentIdx = maxIdx;
	_startIndex1 = candidateResult[maxIdx].startIndex1;
	_startIndex2 = candidateResult[maxIdx].startIndex2;
	_range = candidateResult[maxIdx].range;
	int cScore = candidateResult[maxIdx].score;

	// reAdd alpha pixel to originAlphaBin
	for (int i = 0; i < originAlphaBin.cols; i++) {
		for (int j = 0; j < originAlphaBin.rows; j++) {
			Vec4b & bgra = originImg.at<Vec4b>(j, i);
			if (bgra[3] == 0) // not transparency
				originAlphaBin.at<uchar>(j, i) = 0;
		}
	}

	// if foodAmount = 1, then use icp
	double minIcpError = 100;
	if (foodAmount == 1) {
		vector<Point> newPointSeq2;
		for (int sp = 0; sp < _pointSeq2.size(); sp++) {
			Mat *ppM = &candidateResult[maxIdx].warpMatrix;
			double newX = ppM->at<double>(0, 0)*_pointSeq2[sp].x + ppM->at<double>(0, 1)*_pointSeq2[sp].y + ppM->at<double>(0, 2);
			double newY = ppM->at<double>(1, 0)*_pointSeq2[sp].x + ppM->at<double>(1, 1)*_pointSeq2[sp].y + ppM->at<double>(1, 2);
			newPointSeq2.push_back(Point((int)newX, (int)newY));
		}

		Mat lastMat = Mat::zeros(Size(3, 3), CV_64F);
		warpMatToHomogeneous(lastMat, candidateResult[maxIdx].warpMatrix);
		// getCenter
		Point Pcenter(0, 0);
		getCenterPointInt(_pointSeq1, Pcenter.x, Pcenter.y);

		// getIcp
		minIcpError = icpIteration(5, candidateResult[maxIdx].warpMatrix, _pointSeq1, _pointSeq2, newPointSeq2, 1);

		candidateResult[maxIdx].warpImg = Mat::zeros(_inputSize, CV_8UC4);
		warpAffine(doodImg, candidateResult[maxIdx].warpImg, candidateResult[maxIdx].warpMatrix, _inputSize);
		resultStack = candidateResult[maxIdx].warpImg.clone();
	}


	frag fragMax;
	fragMax.setInfo(_startIndex1, _startIndex2, _range, cScore, _cIndex, _fIndex, foodAmount, resultStack);
	fragMax.icpError = minIcpError;
	fragMax.alphaBinMat = originAlphaBin.clone();
	_frag2.push_back(fragMax);

	clock_t s3 = clock();
	//cout << "s3= " << (s3 - s2) << endl;

	//imwrite("Result0/imGG.png", resultStack);
	//system("pause");
}

// use descriptor to do combination
void comp::findCombination2(Mat &doodImg, vector<Point> &nonAlphaOfFood) {
	clock_t s0 = clock();
	//cout << "-------------- s0" << endl;

	// initial parameters
	int foodAmount = 0;
	Mat originImg = Mat::zeros(_inputSize, CV_8UC4);
	drawContours(originImg, vector<vector<Point>>(1, _pointSeq1), 0, Scalar(255, 255, 255, 255), CV_FILLED);
	Mat originAlphaBin = alphaBinary2(originImg);

	// getValuePoint, alpha!=0
	vector<Point> foodValuePoint = getValuePart(doodImg);

	// sort by startIndex1 and then range
	sort(candidateResult.begin(), candidateResult.end(), compareWithstartIdx);
	//for (int i = candidateResult.size() - 1; i >= 0; i--) 
	//cout << "startIndex=" << candidateResult[i].startIndex1 << ", _range= " << candidateResult[i].range << endl;


	// to determine if food is in contour
	//take only the longest range of the same startIndex1
	bool hasFindFirst = false;
	vector<bool> canBeStart(_pointSeq1.size(), true);
	for (int i = candidateResult.size() - 1; i >= 0; i--) {
		if ((!hasFindFirst || candidateResult[i].startIndex1 != candidateResult[i + 1].startIndex1) && canBeStart[candidateResult[i].startIndex1]) {
			//cout << "****** (startIndex)=" << candidateResult[i].startIndex1 << ", _range= " << candidateResult[i].range;
			//cout << ", (startIndex+1)=" << candidateResult[i+1].startIndex1 << ", _range= " << candidateResult[i+1].range << endl;
			candidateResult[i].warpMatrix = estimateRigidTransform(candidateResult[i].matchSeqQ, candidateResult[i].matchSeqR, false);

			if (candidateResult[i].warpMatrix.size() != cv::Size(0, 0)) {
				removeScaleOfWarpMat(candidateResult[i].warpMatrix);
				int overlapCount = 0, foodArea = 0;
				vector<Point> warpSeq;
				for (int p = 0; p < nonAlphaOfFood.size(); p++) {
					double newX = candidateResult[i].warpMatrix.at<double>(0, 0)*nonAlphaOfFood[p].x + candidateResult[i].warpMatrix.at<double>(0, 1)*nonAlphaOfFood[p].y + candidateResult[i].warpMatrix.at<double>(0, 2);
					double newY = candidateResult[i].warpMatrix.at<double>(1, 0)*nonAlphaOfFood[p].x + candidateResult[i].warpMatrix.at<double>(1, 1)*nonAlphaOfFood[p].y + candidateResult[i].warpMatrix.at<double>(1, 2);
					if (newY >= 0 && newY < originAlphaBin.rows && newX >= 0 && newX < originAlphaBin.cols) {
						foodArea++;
						warpSeq.push_back(Point(newX, newY));
						if (originAlphaBin.at<uchar>(newY, newX) == 255)
							overlapCount++;
					}
				}

				for (int cL = 0; cL < candidateResult[i].range / 2; cL++) {
					int cc_Idx = (candidateResult[i].startIndex1 + cL) % canBeStart.size();
					canBeStart[cc_Idx] = false;
				}

				if (((double)overlapCount / (double)foodArea)>0.7) {
					hasFindFirst = true;
					candidateResult[i].warpSeq = warpSeq;
				}
				else
					candidateResult.erase(candidateResult.begin() + i);

				/*clock_t c3 = clock();
				cout << ", c3-c2= " << c3 - c2;
				system("pause");*/

			}
			else
				candidateResult.erase(candidateResult.begin() + i);
		}
		else {
			candidateResult.erase(candidateResult.begin() + i);
		}
	}

	if (candidateResult.size() <= 0)
		return;
	else
		foodAmount = 1;

	// find max range in candidateResult index
	int maxIdx = 0, maxRange = -1;
	for (int i = 0; i < candidateResult.size(); i++) {
		if (candidateResult[i].range > maxRange) {
			maxIdx = i;
			maxRange = candidateResult[i].range;
		}
	}
	for (int i = 0; i < candidateResult.size(); i++) {
		candidateResult[i].fake_startIndex1 = (candidateResult[i].startIndex1 - candidateResult[maxIdx].startIndex1) % _rDesSize;
		candidateResult[i].fake_startIndex2 = (candidateResult[i].fake_startIndex1 + (candidateResult[i].range / 2)) % _rDesSize;
	}
	clock_t s1 = clock();
	//cout << "s1= " << (s1 - s0) << endl;

	double stackThreshold = 0.4;
	if (!_stackable) { stackThreshold = 0.7; }

	// get first resultStack and originAlphaBin
	candidateResult[maxIdx].warpImg = Mat::zeros(_inputSize, CV_8UC4);
	warpAffine(doodImg, candidateResult[maxIdx].warpImg, candidateResult[maxIdx].warpMatrix, _inputSize);
	Mat resultStack = candidateResult[maxIdx].warpImg.clone();
	for (int i = 0; i < originAlphaBin.cols; i++) {
		for (int j = 0; j < originAlphaBin.rows; j++) {
			originAlphaBin.at<uchar>(j, i) = 255;
		}
	}
	//stackOrNot(_inputSize, candidateResult[maxIdx].warpImg, originAlphaBin, stackThreshold);
	for (int p = 0; p < candidateResult[maxIdx].warpSeq.size(); p++) {
		if (originAlphaBin.at<uchar>(candidateResult[maxIdx].warpSeq[p].y, candidateResult[maxIdx].warpSeq[p].x) == 255)
			originAlphaBin.at<uchar>(candidateResult[maxIdx].warpSeq[p].y, candidateResult[maxIdx].warpSeq[p].x) = 0;
	}

	int lastIdx = maxIdx;
	for (int i = 1; i < candidateResult.size(); i++) {
		int currentIdx = (maxIdx + i) % candidateResult.size();
		Mat originAlphaBinTmp = originAlphaBin.clone();

		if ((candidateResult[currentIdx].startIndex1>candidateResult[lastIdx].startIndex2)) {

			int overlapCount = 0;
			for (int p = 0; p < candidateResult[currentIdx].warpSeq.size(); p++) {
				if (originAlphaBinTmp.at<uchar>(candidateResult[currentIdx].warpSeq[p].y, candidateResult[currentIdx].warpSeq[p].x) == 255) {
					overlapCount++;
					originAlphaBinTmp.at<uchar>(candidateResult[currentIdx].warpSeq[p].y, candidateResult[currentIdx].warpSeq[p].x) = 0;
				}
			}

			if (((double)overlapCount / (double)candidateResult[currentIdx].warpSeq.size()) > stackThreshold) {
				//if (stackOrNot(_inputSize, candidateResult[currentIdx].warpImg, originAlphaBinTmp, stackThreshold)) {
				foodAmount++;
				originAlphaBin = originAlphaBinTmp.clone();

				candidateResult[currentIdx].warpImg = Mat::zeros(_inputSize, CV_8UC4);
				warpAffine(doodImg, candidateResult[currentIdx].warpImg, candidateResult[currentIdx].warpMatrix, _inputSize);
				resultStack = addTransparent_Comp(resultStack, candidateResult[currentIdx].warpImg);

				//imwrite("Result0/cresultStack.png", candidateResult[currentIdx].warpImg);
				//imwrite("Result0/originAlphaBinTmp.png", originAlphaBinTmp);
				//testWarpingImg(candidateResult[currentIdx].startIndex1, candidateResult[currentIdx].startIndex2, candidateResult[currentIdx].range, candidateResult[currentIdx].warpMatrix);
				//imwrite("Result0/imGG.png", resultStack);
				//system("pause");
				lastIdx = currentIdx;
			}
		}
		else {
			//imwrite("Result0/originAlphaBinTmp.png", originAlphaBinTmp);
			//system("pause");
		}
	}
	clock_t s2 = clock();
	//cout << "s2= " << (s2 - s1) << endl;

	int currentIdx = maxIdx;
	_startIndex1 = candidateResult[maxIdx].startIndex1;
	_startIndex2 = candidateResult[maxIdx].startIndex2;
	_range = candidateResult[maxIdx].range;
	int cScore = candidateResult[maxIdx].score;

	// reAdd alpha pixel to originAlphaBin
	for (int i = 0; i < originAlphaBin.cols; i++) {
		for (int j = 0; j < originAlphaBin.rows; j++) {
			Vec4b & bgra = originImg.at<Vec4b>(j, i);
			if (bgra[3] == 0) // not transparency
				originAlphaBin.at<uchar>(j, i) = 0;
		}
	}

	// if foodAmount = 1, then use icp
	double minIcpError = 100;
	if (foodAmount == 1) {
		vector<Point> newPointSeq2;
		for (int sp = 0; sp < _pointSeq2.size(); sp++) {
			Mat *ppM = &candidateResult[maxIdx].warpMatrix;
			double newX = ppM->at<double>(0, 0)*_pointSeq2[sp].x + ppM->at<double>(0, 1)*_pointSeq2[sp].y + ppM->at<double>(0, 2);
			double newY = ppM->at<double>(1, 0)*_pointSeq2[sp].x + ppM->at<double>(1, 1)*_pointSeq2[sp].y + ppM->at<double>(1, 2);
			newPointSeq2.push_back(Point((int)newX, (int)newY));
		}

		Mat lastMat = Mat::zeros(Size(3, 3), CV_64F);
		warpMatToHomogeneous(lastMat, candidateResult[maxIdx].warpMatrix);
		// getCenter
		Point Pcenter(0, 0);
		getCenterPointInt(_pointSeq1, Pcenter.x, Pcenter.y);

		// getIcp
		minIcpError = icpIteration(5, candidateResult[maxIdx].warpMatrix, _pointSeq1, _pointSeq2, newPointSeq2, 1);

		candidateResult[maxIdx].warpImg = Mat::zeros(_inputSize, CV_8UC4);
		warpAffine(doodImg, candidateResult[maxIdx].warpImg, candidateResult[maxIdx].warpMatrix, _inputSize);
		resultStack = candidateResult[maxIdx].warpImg.clone();
	}


	frag fragMax;
	fragMax.setInfo(_startIndex1, _startIndex2, _range, cScore, _cIndex, _fIndex, foodAmount, resultStack);
	fragMax.icpError = minIcpError;
	fragMax.alphaBinMat = originAlphaBin.clone();
	_frag2.push_back(fragMax);

	clock_t s3 = clock();
	//cout << "s3= " << (s3 - s2) << endl;

	//imwrite("Result0/imGG.png", resultStack);
	//system("pause");
}

// use descriptor to do combination
void comp::findCombination3(Mat &doodImg, vector<Point> &nonAlphaOfFood) {
	clock_t s0 = clock();
	//cout << "-------------- s0" << endl;

	// initial parameters
	int foodAmount = 0;
	Mat originImg = Mat::zeros(_inputSize, CV_8UC4);
	drawContours(originImg, vector<vector<Point>>(1, _pointSeq1), 0, Scalar(255, 255, 255, 255), CV_FILLED);
	Mat originAlphaBin = alphaBinary2(originImg);

	// getValuePoint, alpha!=0
	vector<Point> foodValuePoint = getValuePart(doodImg);

	// sort by startIndex1 and then range
	sort(candidateResult.begin(), candidateResult.end(), compareWithstartIdx);
	/*for (int i = candidateResult.size() - 1; i >= 0; i--) 
	cout << "startIndex=" << candidateResult[i].startIndex1 << ", _range= " << candidateResult[i].range << endl;
	startIndex = 0, _range = 30
	startIndex = 0, _range = 29
	startIndex = 0, _range = 28
	startIndex = 0, _range = 24
	startIndex = 1, _range = 28
	startIndex = 1, _range = 27
	startIndex = 1, _range = 19
	startIndex = 2, _range = 31
	startIndex = 2, _range = 30
	startIndex = 2, _range = 29*/

	getCenterPoint(_pointSeq2, meanFX, meanFY);
	Point center1(meanCX, meanCY);
	Point center2(meanFX, meanFY);
	// to determine if food is in contour
	//take only the longest range of the same startIndex1

	bool hasFindFirst = false;
	vector<bool> canBeStart(_pointSeq1.size(), true);
	for (int i = candidateResult.size() - 1; i >= 0; i--) {
		if ((!hasFindFirst || candidateResult[i].startIndex1 != candidateResult[i + 1].startIndex1) && canBeStart[candidateResult[i].startIndex1]) {
			//cout << "****** (startIndex)=" << candidateResult[i].startIndex1 << ", _range= " << candidateResult[i].range;
			//cout << ", (startIndex+1)=" << candidateResult[i+1].startIndex1 << ", _range= " << candidateResult[i+1].range << endl;
			candidateResult[i].warpMatrix = estimateRigidTransform(candidateResult[i].matchSeqQ, candidateResult[i].matchSeqR, false);

			if (candidateResult[i].warpMatrix.size() != cv::Size(0, 0)) {
				

				removeScaleOfWarpMat(candidateResult[i].warpMatrix);

				
				double newX = candidateResult[i].warpMatrix.at<double>(0, 0)*center2.x + candidateResult[i].warpMatrix.at<double>(0, 1)*center2.y + candidateResult[i].warpMatrix.at<double>(0, 2);
				double newY = candidateResult[i].warpMatrix.at<double>(1, 0)*center2.x + candidateResult[i].warpMatrix.at<double>(1, 1)*center2.y + candidateResult[i].warpMatrix.at<double>(1, 2);
				if (pointPolygonTest(_pointSeq1, Point(newX, newY), false) > 0) { // if point is in contour
					for (int cL = 0; cL < candidateResult[i].range / 2; cL++) {
						int cc_Idx = (candidateResult[i].startIndex1 + cL) % canBeStart.size();
						canBeStart[cc_Idx] = false;
					}
				}

				int overlapCount = 0, foodArea = 0;
				vector<Point> warpSeq;
				for (int p = 0; p < nonAlphaOfFood.size(); p++) {
					double newX = candidateResult[i].warpMatrix.at<double>(0, 0)*nonAlphaOfFood[p].x + candidateResult[i].warpMatrix.at<double>(0, 1)*nonAlphaOfFood[p].y + candidateResult[i].warpMatrix.at<double>(0, 2);
					double newY = candidateResult[i].warpMatrix.at<double>(1, 0)*nonAlphaOfFood[p].x + candidateResult[i].warpMatrix.at<double>(1, 1)*nonAlphaOfFood[p].y + candidateResult[i].warpMatrix.at<double>(1, 2);
					if (newY >= 0 && newY < originAlphaBin.rows && newX >= 0 && newX < originAlphaBin.cols) {
						foodArea++;
						warpSeq.push_back(Point(newX, newY));
						if (originAlphaBin.at<uchar>(newY, newX) == 255)
							overlapCount++;
					}
				}

				for (int cL = 0; cL < candidateResult[i].range / 2; cL++) {
					int cc_Idx = (candidateResult[i].startIndex1 + cL) % canBeStart.size();
					canBeStart[cc_Idx] = false;
				}

				if (((double)overlapCount / (double)foodArea)>0.7) {
					hasFindFirst = true;
					candidateResult[i].warpSeq = warpSeq;
				}
				else
					candidateResult.erase(candidateResult.begin() + i);

				/*clock_t c3 = clock();
				cout << ", c3-c2= " << c3 - c2;
				system("pause");*/
			}
			else
				candidateResult.erase(candidateResult.begin() + i);
		}
		else {
			candidateResult.erase(candidateResult.begin() + i);
		}
	}

	if (candidateResult.size() <= 0)
		return;
	else
		foodAmount = 1;
	//system("pause");

	// find max range in candidateResult index
	int maxIdx = 0, maxRange = -1;
	for (int i = 0; i < candidateResult.size(); i++) {
		if (candidateResult[i].range > maxRange) {
			maxIdx = i;
			maxRange = candidateResult[i].range;
		}
	}
	for (int i = 0; i < candidateResult.size(); i++) {
		candidateResult[i].fake_startIndex1 = (candidateResult[i].startIndex1 - candidateResult[maxIdx].startIndex1) % _rDesSize;
		candidateResult[i].fake_startIndex2 = (candidateResult[i].fake_startIndex1 + (candidateResult[i].range / 2)) % _rDesSize;
	}
	clock_t s1 = clock();
	//cout << "s1= " << (s1 - s0) << endl;

	double stackThreshold = 0.4;
	if (!_stackable) { stackThreshold = 0.7; }

	// get first resultStack and originAlphaBin
	candidateResult[maxIdx].warpImg = Mat::zeros(_inputSize, CV_8UC4);
	warpAffine(doodImg, candidateResult[maxIdx].warpImg, candidateResult[maxIdx].warpMatrix, _inputSize);
	Mat resultStack = candidateResult[maxIdx].warpImg.clone();
	for (int i = 0; i < originAlphaBin.cols; i++) {
		for (int j = 0; j < originAlphaBin.rows; j++) {
			originAlphaBin.at<uchar>(j, i) = 255;
		}
	}
	//stackOrNot(_inputSize, candidateResult[maxIdx].warpImg, originAlphaBin, stackThreshold);
	for (int p = 0; p < candidateResult[maxIdx].warpSeq.size(); p++) {
		if (originAlphaBin.at<uchar>(candidateResult[maxIdx].warpSeq[p].y, candidateResult[maxIdx].warpSeq[p].x) == 255)
			originAlphaBin.at<uchar>(candidateResult[maxIdx].warpSeq[p].y, candidateResult[maxIdx].warpSeq[p].x) = 0;
	}

	int lastIdx = maxIdx;
	for (int i = 1; i < candidateResult.size(); i++) {
		int currentIdx = (maxIdx + i) % candidateResult.size();
		Mat originAlphaBinTmp = originAlphaBin.clone();

		if ((candidateResult[currentIdx].startIndex1>candidateResult[lastIdx].startIndex2)) {

			int overlapCount = 0;
			for (int p = 0; p < candidateResult[currentIdx].warpSeq.size(); p++) {
				if (originAlphaBinTmp.at<uchar>(candidateResult[currentIdx].warpSeq[p].y, candidateResult[currentIdx].warpSeq[p].x) == 255) {
					overlapCount++;
					originAlphaBinTmp.at<uchar>(candidateResult[currentIdx].warpSeq[p].y, candidateResult[currentIdx].warpSeq[p].x) = 0;
				}
			}

			if (((double)overlapCount / (double)candidateResult[currentIdx].warpSeq.size()) > stackThreshold) {
				//if (stackOrNot(_inputSize, candidateResult[currentIdx].warpImg, originAlphaBinTmp, stackThreshold)) {
				foodAmount++;
				originAlphaBin = originAlphaBinTmp.clone();

				candidateResult[currentIdx].warpImg = Mat::zeros(_inputSize, CV_8UC4);
				warpAffine(doodImg, candidateResult[currentIdx].warpImg, candidateResult[currentIdx].warpMatrix, _inputSize);
				imwrite("Result0/" + to_string(i) + ".png", candidateResult[currentIdx].warpImg);
				resultStack = addTransparent_Comp(resultStack, candidateResult[currentIdx].warpImg);

				//imwrite("Result0/cresultStack.png", candidateResult[currentIdx].warpImg);
				//imwrite("Result0/originAlphaBinTmp.png", originAlphaBinTmp);
				//testWarpingImg(candidateResult[currentIdx].startIndex1, candidateResult[currentIdx].startIndex2, candidateResult[currentIdx].range, candidateResult[currentIdx].warpMatrix);
				//imwrite("Result0/imGG.png", resultStack);
				//system("pause");
				lastIdx = currentIdx;
			}
		}
		else {
			//imwrite("Result0/originAlphaBinTmp.png", originAlphaBinTmp);
			//system("pause");
		}
	}
	clock_t s2 = clock();
	//cout << "s2= " << (s2 - s1) << endl;

	int currentIdx = maxIdx;
	_startIndex1 = candidateResult[maxIdx].startIndex1;
	_startIndex2 = candidateResult[maxIdx].startIndex2;
	_range = candidateResult[maxIdx].range;
	int cScore = candidateResult[maxIdx].score;

	// reAdd alpha pixel to originAlphaBin
	for (int i = 0; i < originAlphaBin.cols; i++) {
		for (int j = 0; j < originAlphaBin.rows; j++) {
			Vec4b & bgra = originImg.at<Vec4b>(j, i);
			if (bgra[3] == 0) // not transparency
				originAlphaBin.at<uchar>(j, i) = 0;
		}
	}

	// if foodAmount = 1, then use icp
	double minIcpError = 100;
	if (foodAmount == 1) {
		vector<Point> newPointSeq2;
		for (int sp = 0; sp < _pointSeq2.size(); sp++) {
			Mat *ppM = &candidateResult[maxIdx].warpMatrix;
			double newX = ppM->at<double>(0, 0)*_pointSeq2[sp].x + ppM->at<double>(0, 1)*_pointSeq2[sp].y + ppM->at<double>(0, 2);
			double newY = ppM->at<double>(1, 0)*_pointSeq2[sp].x + ppM->at<double>(1, 1)*_pointSeq2[sp].y + ppM->at<double>(1, 2);
			newPointSeq2.push_back(Point((int)newX, (int)newY));
		}

		Mat lastMat = Mat::zeros(Size(3, 3), CV_64F);
		warpMatToHomogeneous(lastMat, candidateResult[maxIdx].warpMatrix);
		// getCenter
		Point Pcenter(0, 0);
		getCenterPointInt(_pointSeq1, Pcenter.x, Pcenter.y);

		// getIcp
		minIcpError = icpIteration(5, candidateResult[maxIdx].warpMatrix, _pointSeq1, _pointSeq2, newPointSeq2, 1);

		candidateResult[maxIdx].warpImg = Mat::zeros(_inputSize, CV_8UC4);
		warpAffine(doodImg, candidateResult[maxIdx].warpImg, candidateResult[maxIdx].warpMatrix, _inputSize);
		resultStack = candidateResult[maxIdx].warpImg.clone();
	}


	frag fragMax;
	fragMax.setInfo(_startIndex1, _startIndex2, _range, cScore, _cIndex, _fIndex, foodAmount, resultStack);
	fragMax.icpError = minIcpError;
	fragMax.alphaBinMat = originAlphaBin.clone();
	_frag2.push_back(fragMax);

	clock_t s3 = clock();
	//cout << "s3= " << (s3 - s2) << endl;

	//imwrite("Result0/imGG.png", resultStack);
	//system("pause");
}

Mat imwriteMat(Mat matt) {
	double max = 0;
	for (int i = 0; i < matt.cols; i++) {
		for (int j = 0; j < matt.rows; j++) {
			if (max<matt.at<double>(j, i))
				max = matt.at<double>(j, i);
		}
	}
	double scale = 255 / max;

	for (int i = 0; i < matt.cols; i++) {
		for (int j = 0; j < matt.rows; j++) {
			matt.at<double>(j, i) *= scale;
		}
	}
	return matt;
}

// icp for descriptor
Mat comp::descriptoIcp(vector<Point> &contourPoint, vector<Point> &foodPoint, vector<Point> &foodPointOri) {
	getCenterPoint(contourPoint, meanCX, meanCY);
	getCenterPoint(foodPointOri, meanFX, meanFY);

	double sumFXCY = 0;
	double sumFYCX = 0;
	double sumFXCX = 0;
	double sumFYCY = 0;
	for (int i = 0; i < contourPoint.size(); i++) {
		sumFXCY += (foodPoint[i].x - meanFX) * (contourPoint[i].y - meanCY);
		sumFYCX += (foodPoint[i].y - meanFY) * (contourPoint[i].x - meanCX);
		sumFXCX += (foodPoint[i].x - meanFX) * (contourPoint[i].x - meanCX);
		sumFYCY += (foodPoint[i].y - meanFY) * (contourPoint[i].y - meanCY);
	}

	Mat H = Mat::zeros(Size(2, 2), CV_64FC1);
	H.at<double>(0, 0) = sumFXCX;
	H.at<double>(0, 1) = sumFYCX;
	H.at<double>(1, 0) = sumFXCY;
	H.at<double>(1, 1) = sumFYCY;

	SVD s(H);
	Mat R = s.vt.t()*s.u.t();
	double dTheta;
	if (R.at<double>(0, 0)>0 && R.at<double>(0, 1)>0) { // cos+ sin+
		dTheta = acos(R.at<double>(0, 0));
	}
	else if (R.at<double>(0, 0)>0 && R.at<double>(0, 1)<0) { // + -
		dTheta = asin(R.at<double>(0, 1));
	}
	else if (R.at<double>(0, 0)<0 && R.at<double>(0, 1)>0) { // - +
		dTheta = acos(R.at<double>(0, 0));
	}
	else { // - -
		dTheta = acos(R.at<double>(0, 0));
		dTheta = 2 * PI - dTheta;
	}

	int dX = meanCX - (meanFX*cos(dTheta) - meanFY*sin(dTheta));
	int dY = meanCY - (meanFX*sin(dTheta) + meanFY*cos(dTheta));

	Mat newWarpMat = Mat::zeros(Size(3, 2), CV_64F);
	newWarpMat.at<double>(0, 0) = cos(dTheta);
	newWarpMat.at<double>(0, 1) = -sin(dTheta);
	newWarpMat.at<double>(0, 2) = dX;
	newWarpMat.at<double>(1, 0) = -newWarpMat.at<double>(0, 1);
	newWarpMat.at<double>(1, 1) = newWarpMat.at<double>(0, 0);
	newWarpMat.at<double>(1, 2) = dY;
	return newWarpMat;
}

void comp::compareDesN(Mat input1, Mat input2, int index, bool cLarge) {
	Mat smallerMat;
	Mat sub;
	clock_t c1, c2;
	if (cLarge) {
		smallerMat = input1(Rect(0, 0, input2.cols, input2.rows));
		sub = smallerMat - input2;
		/*Mat input1Mat = imwriteMat(smallerMat);
		imwrite("Result0/input1Mat.png", input1Mat);
		Mat input2Mat = imwriteMat(input2);
		imwrite("Result0/input2Mat.png", input2Mat);*/

		cout << smallerMat << endl;
		system("pause");
		cout << input2 << endl;
		system("pause");
	}
	else {
		smallerMat = input2(Rect(0, 0, input1.cols, input1.rows));
		sub = input1 - smallerMat;
		/*Mat input1Mat = imwriteMat(input1);
		imwrite("Result0/input1Mat.png", input1Mat);
		Mat input2Mat = imwriteMat(smallerMat);
		imwrite("Result0/input2Mat.png", input2Mat);*/
		cout << input1 << endl;
		system("pause");
		cout << smallerMat << endl;
		system("pause");
	}
	cout << smallerMat << endl;
	system("pause");
	Mat integral1; // sum
	Mat integral2; // square sum

	int rLim = 0;
	int lefttopPoint1 = 0;
	int lefttopPoint2 = 0;
	double tmpSum = 0;
	double getScore = 0;
	integral(sub, integral1, integral2);
	cout << "sub.size()= " << sub.size() << "sub.type()= " << sub.type() << endl;
	cout << "integral1.size()= " << integral1.size() << "integral1.type()= " << integral1.type() << endl;
	cout << "integral2.size()= " << integral2.size() << "integral2.type()= " << integral2.type() << endl;

	rLim = _lLimit*(double)integral2.cols; // square size

	for (int i = 0; i < integral2.cols; i++) {

		if (cLarge)	{
			_startIndex1 = (i + index) % input1.cols;
			_startIndex2 = i;
		}
		else {
			_startIndex1 = i;
			_startIndex2 = (i + index) % input2.cols;
		}

		if (_startIndex1 < _mapRQ.rows && _startIndex2 < _mapRQ.cols) {
			for (int r = integral2.cols-i; r > rLim; r--) {
				_range = r;

				//if ((i + r) <= integral2.cols) {
					tmpSum = integral2.at<double>(i, i) + integral2.at<double>(i + r - 1, i + r - 1) - integral2.at<double>(i, i + r - 1) - integral2.at<double>(i + r - 1, i);

					/*Mat descMat = imwriteMat(integral2);
					imwrite("Result0/integral2.png", descMat);*/

					getScore = tmpSum / pow(r, 2);
					if (!initScore && _thresholdScore > getScore){
						_minScore = getScore;
						initScore = true;
					}
					if (/*initScore*/true) {
					//if (initScore && getScore <= 5 * _minScore && getScore >= 0 && _range>_mapRQ.at<int>(_startIndex1, _startIndex2)) {
						vector<Point> matchSeqR = subPointSeq(_pointSeq1, _startIndex1, _range);
						vector<Point> matchSeqQ = subPointSeq(_pointSeq2, _startIndex2, _range);

						Mat warpMat = estimateRigidTransform(matchSeqQ, matchSeqR, false); // (src/query, dst/reference)
						

						if (warpMat.size() != cv::Size(0, 0)) {
							//removeScaleOfWarpMat(warpMat);

							cout << warpMat << endl;
							solver icpSolver;
							warpMat = icpSolver.getTwoSetsTransform(matchSeqQ, matchSeqR);
							cout << warpMat << endl;
							//warpMat = descriptoIcp(matchSeqR, matchSeqQ, _pointSeq2).clone();

							Mat c_warpImg = Mat::zeros(_inputSize, CV_8UC4);
							warpAffine(ddood, c_warpImg, warpMat, _inputSize);
							drawContours(c_warpImg, vector<vector<Point>>(1, _pointSeq1), 0, Scalar(255, 0, 0, 255), 1, 8);
							Mat testWarpM = testWarpingImg(_startIndex1, _startIndex2, _range, warpMat);
							//imwrite("Result0/" + to_string(getScore) + ".png", testWarpM);

							cout << "(" << i << "," << i << ")= " << integral2.at<double>(i, i) << endl;
							cout << "(" << i + r - 1 << "," << i + r - 1 << ")= " << integral2.at<double>(i + r - 1, i + r - 1) << endl;
							cout << "(" << i << "," << i + r - 1 << ")= " << integral2.at<double>(i, i + r - 1) << endl;
							cout << "(" << i + r - 1 << "," << i << ")= " << integral2.at<double>(i + r - 1, i) << endl;
							cout << "_startIndex1= " << _startIndex1 << ", _startIndex2= " << _startIndex2 << ", r = " << r << ", getScore= " << getScore << endl;
							imwrite("Result0/" + to_string(_startIndex1) + "_" + to_string(_startIndex2) + "_" + to_string(_range) + "_" + to_string(getScore) + ".png", testWarpM);
							//system("pause");
						}





						//_minScore = getScore;
						_mapRQ.at<int>(_startIndex1, _startIndex2) = _range;
						_mapScore.at<double>(_startIndex1, _startIndex2) = getScore;
						//cout << "****** startIndex= (" << _startIndex1 << ", " << _startIndex2 << "), _range= " << _range << ", getScore= " << getScore << endl;

							

						int candidIdx = _mapCandidateIdx.at<int>(_startIndex1, _startIndex2);
						if (candidIdx == 0) {
							_mapCandidateIdx.at<int>(_startIndex1, _startIndex2) = candidateResult.size();

							candidateInfo candi;
							candi.startIndex1 = _startIndex1;
							candi.startIndex2 = _startIndex2;
							candi.range = _range;
							candi.score = getScore;
							//candi.warpMatrix = warpMat.clone();
							candi.matchSeqR = matchSeqR;
							candi.matchSeqQ = matchSeqQ;
							candidateResult.push_back(candi);
						}
						else {
							candidateResult[candidIdx - 1].startIndex1 = _startIndex1;
							candidateResult[candidIdx - 1].startIndex2 = _startIndex2;
							candidateResult[candidIdx - 1].range = _range;
							candidateResult[candidIdx - 1].score = getScore;
							//candidateResult[candidIdx - 1].warpMatrix = warpMat.clone();
							candidateResult[candidIdx - 1].matchSeqR = matchSeqR;
							candidateResult[candidIdx - 1].matchSeqQ = matchSeqQ;
						}
						//break;
					}

			}
		}
	}
	//system("pause");

}

double comp::getMinimumDistance(vector<Point> &seqP1, vector<Point> &seqP2) {
	double totalErr = 0;
	double dist, minDist;

	Mat drawing0 = Mat::zeros(_inputSize, CV_8UC3);
	for (int i = 0; i < seqP1.size(); i++)
		circle(drawing0, seqP1[i], 2, Scalar(255, 255, 255), 2);
	for (int i = 0; i < seqP2.size(); i++)
		circle(drawing0, seqP2[i], 2, Scalar(255, 0, 255), 2);

	for (int i = 0; i < seqP2.size(); i++) {
		int minJ = 0;
		for (int j = 0; j < seqP1.size(); j++)	{
			dist = sqrt(pow(seqP1[j].x - seqP2[i].x, 2) + pow(seqP1[j].y - seqP2[i].y, 2));
			if (j == 0)	{
				minDist = dist;
				minJ = j;
			}
			else {
				if (dist < minDist)	{
					minDist = dist;
					minJ = j;
				}
			}

		}
		line(drawing0, seqP1[minJ], seqP2[i], Scalar(255, 255, 0), 2, 8);
		totalErr += minDist;
	}
	totalErr /= seqP2.size(); 
	//imwrite("Result0/" + to_string(_fIndex) + "_" + to_string(totalErr) + "_" + to_string(_inputSize.width) + ".png", drawing0);
	return totalErr;
}

double comp::getMinimumDistance_reverse(int times, int errorMode, vector<Point> &seqP1, vector<Point> &seqP2, Point centerP1) {
	double totalErr = 0;
	double dist = 0, minDist = 0;

	Mat drawing0 = Mat::zeros(_inputSize, CV_8UC3);
	for (int i = 0; i < seqP1.size(); i++)
		circle(drawing0, seqP1[i], 2, Scalar(255, 255, 255), 2);
	for (int i = 0; i < seqP2.size(); i++)
		circle(drawing0, seqP2[i], 2, Scalar(255, 0, 255), 2);
	
	vector<double> distVec;
	/*for (int j = 0; j < seqP2.size(); j++)	{
		double mDist = pointPolygonTest(seqP1, seqP2[j], true);
		totalErr += mDist;
		totalErr2 += abs(mDist);
		distVec.push_back(mDist);
	}*/
	for (int j = 0; j < seqP1.size(); j++)	{
		double mDist = pointPolygonTest(seqP2, seqP1[j], true);
		totalErr += mDist;
		distVec.push_back(mDist);
	}

	double averageErr = totalErr / seqP1.size();
	double vaError = 0;
	for (int i = 0; i < distVec.size(); i++) {
		vaError += pow((distVec[i] - averageErr), 2);
	}
	vaError /= seqP1.size();

	//if (!_isReverse)
	//imwrite("Result0/" + to_string(_fIndex) + "_" + to_string(times) + "_" + to_string(vaError) + ".png", drawing0);
	timess = times;

	if (errorMode == 0)
		return abs(vaError);

	return abs(vaError);
}

void comp::warpMatToHomogeneous(Mat &warpMat_homo, Mat &warpMat) {
	warpMat_homo.at<double>(0, 0) = warpMat.at<double>(0, 0);
	warpMat_homo.at<double>(0, 1) = warpMat.at<double>(0, 1);
	warpMat_homo.at<double>(0, 2) = warpMat.at<double>(0, 2);
	warpMat_homo.at<double>(1, 0) = warpMat.at<double>(1, 0);
	warpMat_homo.at<double>(1, 1) = warpMat.at<double>(1, 1);
	warpMat_homo.at<double>(1, 2) = warpMat.at<double>(1, 2);
	warpMat_homo.at<double>(2, 0) = 0;
	warpMat_homo.at<double>(2, 1) = 0;
	warpMat_homo.at<double>(2, 2) = 1;
}

void comp::homogeneousToWarpMat(Mat &warpMat_homo, Mat &warpMat) {
	warpMat.at<double>(0, 0) = warpMat_homo.at<double>(0, 0);
	warpMat.at<double>(0, 1) = warpMat_homo.at<double>(0, 1);
	warpMat.at<double>(0, 2) = warpMat_homo.at<double>(0, 2);
	warpMat.at<double>(1, 0) = warpMat_homo.at<double>(1, 0);
	warpMat.at<double>(1, 1) = warpMat_homo.at<double>(1, 1);
	warpMat.at<double>(1, 2) = warpMat_homo.at<double>(1, 2);
}

struct minIcpInfo {
	int index;
	double error;
	Mat warpMat;
	vector<Point> newPointSeq;
};

// find the local maximum of RQ map
void comp::localMaxOfRQMap(Mat &doodImg) {
	int iterIcp = 0;
	int maxR = -1;
	int maxQ = -1;
	int maxVal = -1;
	double dX, dY, dTheta;
	double minDist = 0.0;
	double dist = 0.0;
	int minDistIdx = 0;
	Mat dood;
	double minIcpErr;
	int maxIcpTime = 20;
	vector<Point> newPointSeq;

	while (maxVal != 0) {
		maxVal = -1;
		// get local maximum
		for (int i = 0; i < _mapRQ.rows; i++) { //r
			for (int j = 0; j < _mapRQ.cols; j++) { //q
				if (_mapRQ.at<int>(i, j) > maxVal) {
					maxVal = _mapRQ.at<int>(i, j);
					maxR = i;
					maxQ = j;
				}
			}
		}

		if (maxVal > 0 && maxR != -1 && maxQ != -1)	{
			vector<Point> matchSeqR = subPointSeq(_pointSeq1, maxR, maxVal);
			vector<Point> matchSeqQ = subPointSeq(_pointSeq2, maxQ, maxVal);

			Mat warpMat = estimateRigidTransform(matchSeqQ, matchSeqR, false);// src, dst

			if (warpMat.size() != Size(0, 0)) {
				// original point * warpMat
				for (int p = 0; p < _pointSeq2.size(); p++)	{
					double newX = warpMat.at<double>(0, 0)*_pointSeq2[p].x + warpMat.at<double>(0, 1)*_pointSeq2[p].y + warpMat.at<double>(0, 2);
					double newY = warpMat.at<double>(1, 0)*_pointSeq2[p].x + warpMat.at<double>(1, 1)*_pointSeq2[p].y + warpMat.at<double>(1, 2);
					newPointSeq.push_back(Point((int)newX, (int)newY));
				}

				Mat lastMat = Mat::zeros(Size(3, 3), CV_64F);
				warpMatToHomogeneous(lastMat, warpMat);
				double diagonal = sqrt(pow(_inputSize.width, 2) + pow(_inputSize.height, 2));
				double curIcpErr = 2 * icpIteration(6, warpMat, _pointSeq1, _pointSeq2, newPointSeq, 1) / diagonal;

				dood = doodImg.clone();
				int maxCol = max(dood.cols, _inputSize.width);
				int maxRow = max(dood.rows, _inputSize.height);
				Mat newFood = Mat::zeros(Size(maxCol, maxRow), CV_8UC4);
				for (int i = 0; i < dood.cols; i++)
					for (int j = 0; j < dood.rows; j++)
						newFood.at<Vec4b>(j, i) = dood.at<Vec4b>(j, i);
				warpAffine(newFood, newFood, warpMat, newFood.size()); //---------------
				//imwrite("Result0/test.png", newFood);

				newPointSeq.clear();
				for (int p = 0; p < _pointSeq2.size(); p++)	{
					double newX = warpMat.at<double>(0, 0)*_pointSeq2[p].x + warpMat.at<double>(0, 1)*_pointSeq2[p].y + warpMat.at<double>(0, 2);
					double newY = warpMat.at<double>(1, 0)*_pointSeq2[p].x + warpMat.at<double>(1, 1)*_pointSeq2[p].y + warpMat.at<double>(1, 2);
					newPointSeq.push_back(Point((int)newX, (int)newY));
				}

				frag fragMax;
				fragMax.setInfo(maxR, maxQ, maxVal, _mapScore.at<double>(maxR, maxQ), _cIndex, _fIndex, 1, newFood.clone());
				fragMax.icpError = curIcpErr;
				_frag2.push_back(fragMax);
				break;
			}
			else
				_mapRQ.at<int>(maxR, maxQ) = 0;
		}
	}

}

double comp::icpIteration(int maxIcpTime, Mat &finalMat, vector<Point> &pointSeq1, vector<Point> &pointSeq2, vector<Point> &newPointSeq, int errorMode) {
	getCenterPoint(pointSeq1, meanCX, meanCY);
	getCenterPoint(pointSeq2, meanFX, meanFY);

	int iterIcp = 0;
	double dX, dY, dTheta, n_dTheta;

	minIcpInfo minimumIcp;
	minimumIcp.index = 0;
	minimumIcp.error = getMinimumDistance_reverse(iterIcp, errorMode, pointSeq1, newPointSeq, Point(meanCX, meanCY));

	newPointSeq = pointSeq2;
	minimumIcp.warpMat = finalMat.clone();
	minimumIcp.newPointSeq = newPointSeq;

	vector<int> contourPair(pointSeq1.size(), 0);
	vector<int> foodPair(pointSeq2.size(), 0);
	Mat newWarpMat = Mat::zeros(Size(3, 2), CV_64F);

	do {
		iterIcp++;

		newWarpMat = icp(pointSeq1, pointSeq2, newPointSeq, contourPair, foodPair, dX, dY, dTheta);
		if (abs(n_dTheta - dTheta) < 0.01) { iterIcp = maxIcpTime; }
		n_dTheta = dTheta;

		for (int p = 0; p < newPointSeq.size(); p++)	{
			double newX = newWarpMat.at<double>(0, 0)*(double)pointSeq2[p].x + newWarpMat.at<double>(0, 1)*(double)pointSeq2[p].y;// +newWarpMat.at<double>(0, 2);
			double newY = newWarpMat.at<double>(1, 0)*(double)pointSeq2[p].x + newWarpMat.at<double>(1, 1)*(double)pointSeq2[p].y;// +newWarpMat.at<double>(1, 2);
			newPointSeq[p].x = newX;
			newPointSeq[p].y = newY;
		}

		double meanTX, meanTY;
		getCenterPoint(newPointSeq, meanTX, meanTY);
		Point shiftTmp = Point(meanCX - meanTX, meanCY - meanTY);
		for (int p = 0; p < newPointSeq.size(); p++)	{
			newPointSeq[p] += shiftTmp;
		}

		//get icp error
		double curIcpErr = getMinimumDistance_reverse(iterIcp, errorMode, pointSeq1, newPointSeq, Point(meanCX, meanCY));

		if (curIcpErr < minimumIcp.error) {
			//cout << "curIcpErr= " << curIcpErr << ", minimumIcp.error= " << minimumIcp.error << endl;
			minimumIcp.index = iterIcp;
			minimumIcp.error = curIcpErr;
			minimumIcp.newPointSeq = newPointSeq;
			minimumIcp.warpMat = newWarpMat.clone();
		}
	} while (iterIcp < maxIcpTime);

	newPointSeq = minimumIcp.newPointSeq;
	homogeneousToWarpMat(minimumIcp.warpMat, finalMat);
	return minimumIcp.error;
}

// only for outer contour to icp - one direction
double comp::icpIteration1(int maxIcpTime, Mat &finalMat, vector<Point> &pointSeq1, vector<Point> &pointSeq2, vector<Point> &newPointSeq, int errorMode) {
	getCenterPoint(pointSeq1, meanCX, meanCY);
	getCenterPoint(pointSeq2, meanFX, meanFY);

	int iterIcp = 0;
	double dX, dY, dTheta, n_dTheta;

	minIcpInfo minimumIcp;
	minimumIcp.index = 0;
	minimumIcp.error = getMinimumDistance_reverse(iterIcp, errorMode, pointSeq1, newPointSeq, Point(meanCX, meanCY));

	minimumIcp.warpMat = finalMat.clone();
	minimumIcp.newPointSeq = newPointSeq;
	newPointSeq = pointSeq2;

	vector<int> contourPair(pointSeq1.size(), 0);
	vector<int> foodPair(pointSeq2.size(), 0);
	Mat newWarpMat = Mat::zeros(Size(3, 2), CV_64F);
	newWarpMat.at<double>(0, 0) = 1;
	newWarpMat.at<double>(0, 1) = 0;
	newWarpMat.at<double>(0, 2) = 0;
	newWarpMat.at<double>(1, 0) = 0;
	newWarpMat.at<double>(1, 1) = 1;
	newWarpMat.at<double>(0, 2) = 0;
	do {
		iterIcp++;

		newWarpMat = icp1(pointSeq1, pointSeq2, newPointSeq, dX, dY, dTheta).clone();
		if (abs(n_dTheta - dTheta) < 0.01) { iterIcp = maxIcpTime; }
		n_dTheta = dTheta;
		
		cout << newWarpMat.at<double>(0, 0) << " ---------- " << newWarpMat.at<double>(1, 1) << endl;
		if (newWarpMat.at<double>(0, 0)*newWarpMat.at<double>(1, 1) > 0) {
			for (int p = 0; p < newPointSeq.size(); p++)	{
				double newX = newWarpMat.at<double>(0, 0)*(double)pointSeq2[p].x + newWarpMat.at<double>(0, 1)*(double)pointSeq2[p].y + newWarpMat.at<double>(0, 2);
				double newY = newWarpMat.at<double>(1, 0)*(double)pointSeq2[p].x + newWarpMat.at<double>(1, 1)*(double)pointSeq2[p].y + newWarpMat.at<double>(1, 2);
				newPointSeq[p].x = newX;
				newPointSeq[p].y = newY;
			}
			//get icp error
			double curIcpErr = getMinimumDistance_reverse(iterIcp, errorMode, pointSeq1, newPointSeq, Point(meanCX, meanCY));
			minimumIcp.index = iterIcp;
			minimumIcp.error = curIcpErr;
			minimumIcp.newPointSeq = newPointSeq;
			minimumIcp.warpMat = newWarpMat.clone();
		}
		else
			break;
	} while (iterIcp < maxIcpTime);

	newPointSeq = minimumIcp.newPointSeq;
	homogeneousToWarpMat(minimumIcp.warpMat, finalMat);
	return minimumIcp.error;
}

void comp::gridCutFood(Mat &doodImg) {

	typedef GridGraph_2D_4C<short, short, int> Grid;

	Mat food = doodImg.clone();
	int maxCol = max(food.cols, _inputSize.width);
	int maxRow = max(food.rows, _inputSize.height);
	resize(food, food, Size(maxCol, maxRow));
	Mat draw = Mat::zeros(food.size(), CV_8UC4);

	//target contour
	Mat tmp = Mat::zeros(food.size(), CV_8UC4);

	drawContours(draw, vector<vector<Point>>(1, _pointSeq1), 0, Scalar(255, 0, 0, 255), 2, 8);
	drawContours(tmp, vector<vector<Point>>(1, _pointSeq1), 0, Scalar(255, 0, 0, 255), 2, 8);

	//do dilate with first contour
	Mat element = getStructuringElement(MORPH_RECT, Size(25, 25));
	Mat tmp2 = Mat::zeros(tmp.size(), CV_8UC4);
	dilate(tmp, tmp2, element);
	//imwrite("Result/" + to_string(_cIndex) + "_1.png", tmp);
	//imwrite("Result/" + to_string(_cIndex) + "_2.png", tmp2);

	//get contour after dilate
	vector<Mat> sp;

	split(tmp2, sp);

	vector<vector<Point>> tmpC;
	vector<Vec4i> hi;
	findContours(sp[3], tmpC, hi, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

	drawContours(draw, tmpC, 0, Scalar(0, 0, 255, 255), 5, 8);
	//imwrite("Result/" + to_string(_cIndex) + "_3.png", tmp2);
	//system("pause");

	int minX = food.cols;
	int minY = food.rows;
	int maxX = 0;
	int maxY = 0;
	for (int i = 0; i < 1; i++)	{
		for (int j = 0; j < tmpC[i].size(); j++) {
			if (tmpC[i][j].x < minX)
				minX = tmpC[i][j].x;
			if (tmpC[i][j].y < minY)
				minY = tmpC[i][j].y;
			if (tmpC[i][j].x > maxX)
				maxX = tmpC[i][j].x;
			if (tmpC[i][j].y > maxY)
				maxY = tmpC[i][j].y;
		}
	}

	//cout << minX << " " << minY << " " << maxX << " " << maxY << endl;
	//for (int i = minX; i < maxX; i++)
	//	for (int j = minY; j < maxY; j++)
	//		for (int c = 0; c < tmpC.size(); c++)
	//			if (pointPolygonTest(tmpC[c], Point2f(i, j), true) < 0)
	//				draw.at<Vec4b>(j, i) = Vec4b(0, 0, 255, 255);

	imwrite("ssd.png", draw);
	Mat input2(food, Rect(minX, minY, maxX - minX, maxY - minY));
	Mat draw2(draw, Rect(minX, minY, maxX - minX, maxY - minY));

	//build graph
	Grid* grid = new Grid(input2.cols, input2.rows);
	int max = 0;
	for (int i = 0; i < input2.cols; i++) {
		for (int j = 0; j < input2.rows; j++) {
			grid->set_terminal_cap(grid->node_id(i, j), draw2.at<Vec4b>(j, i) == Vec4b(255, 0, 0, 255) ? 1000 : 0, draw2.at<Vec4b>(j, i) == Vec4b(0, 0, 255, 255) ? 1000 : 0);
			Vec4b s1 = input2.at<Vec4b>(j, i);
			if (i < input2.cols - 1)
			{
				Vec4b s2 = input2.at<Vec4b>(j, i + 1);
				const short cap = (abs(s1[0] - s2[0]) + abs(s1[1] - s2[1]) + abs(s1[2] - s2[2])) / 3;
				grid->set_neighbor_cap(grid->node_id(i, j), 1, 0, cap);
				grid->set_neighbor_cap(grid->node_id(i + 1, j), -1, 0, cap);
			}
			if (j < input2.rows - 1)
			{
				Vec4b s2 = input2.at<Vec4b>(j + 1, i);
				const short cap = (abs(s1[0] - s2[0]) + abs(s1[1] - s2[1]) + abs(s1[2] - s2[2])) / 3;
				grid->set_neighbor_cap(grid->node_id(i, j), 0, +1, cap);
				grid->set_neighbor_cap(grid->node_id(i, j + 1), 0, -1, cap);
			}
		}
	}

	//compute max flow and segmentation
	grid->compute_maxflow();
	Mat output2 = Mat::zeros(input2.size(), CV_8UC4);

	for (int i = 0; i < output2.cols; i++) {
		for (int j = 0; j < output2.rows; j++) {
			Vec4b o = input2.at<Vec4b>(j, i);
			Vec4b p = Vec4b(0, 0, 0, 0);
			output2.at<Vec4b>(j, i) = (grid->get_segment(grid->node_id(i, j)) ? p : o);
		}
	}

	vector<Point> tmpCT;
	for (int i = 0; i < tmpC[0].size(); i++) {
		Point t = Point(tmpC[0][i].x - minX, tmpC[0][i].y - minY);
		tmpCT.push_back(t);
	}

	Mat output = Mat::zeros(food.size(), CV_8UC4);
	for (int i = 0; i < output2.cols; i++) {
		for (int j = 0; j < output2.rows; j++) {
			if (pointPolygonTest(tmpCT, Point(i, j), true) > 0)
				output.at<Vec4b>(j + minY, i + minX) = output2.at<Vec4b>(j, i);
		}
	}
	for (int i = 0; i < tmpCT.size(); i++) {
		tmpCT[i] += Point(minX, minY);
	}

	vector<Mat> ssp;
	vector<vector<Point> >tmpCC;
	split(output, ssp);

	findContours(ssp[3], tmpCC, hi, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
	//imwrite("sso.png", output);
	//imwrite("ssp.png", ssp[3]);

	if (tmpCC.size() > 0) {
		int maxS = 0;
		int maxIdx = 0;
		for (int i = 0; i < tmpCC.size(); i++) {
			if (tmpCC[i].size() > maxS)	{
				maxS = tmpCC[i].size();
				maxIdx = i;
			}
		}
		//cout <<tmpCC.size()<<" grid contour"<< maxS << ", " << maxIdx << endl;
		//imwrite("Result/" + to_string(_cIndex) + "_s.png", draw);
		//imwrite("Result/" + to_string(_cIndex) + "_o.png", output);

		descri desTmp(tmpCC[maxIdx], 0, 1);
		
		//set fragment
		frag fragMax;
		fragMax.setInfo(-1, -1, -1, 0.0, _cIndex, _fIndex, 1, output.clone());
		fragMax.icpError = 0;
		fragMax.sError = 0;
		_frag2.push_back(fragMax);
	}
}

bool comp::isGroup() {
	return _isGroup;
}

// return a set of fragment
vector<map<string, int> > comp::fragList() {
	return _frag;
}

// return a list of fragment with fragment.h
vector<frag> comp::fragList2() {
	return _frag2;
}

// return a list of fragment with fragment.h
vector<frag> comp::fragList3() {
	return _frag3;
}

// get sub point sequence
vector<Point> comp::subPointSeq(vector<Point> inputSeq, int startIndex, int matchL) {
	vector<Point> result;

	for (int i = 0; i < matchL; i++) {
		result.push_back(inputSeq[(startIndex + i) % inputSeq.size()]);
	}

	return result;
}

bool comp::isExist(Point idx, vector<Point> &vec) {
	for (int i = 0; i < vec.size(); i++) {
		if (vec[i].x == idx.x) {
			return true;
		}
	}
	return false;
}

void comp::imageOverlap(Size c_inputSize, vector<Point> &samplePSeq1, vector<Point> &samplePSeq2, double &iError, double &ratio1, double &ratio2) {
	clock_t t1, t2;
	t1 = clock();
	/// Calculate the distances to the contour
	Mat drawing0 = Mat::zeros(c_inputSize, CV_8UC3);
	int contourArea = 0, foodArea = 0, overlapArea = 0;
	Mat fill_1 = Mat::zeros(c_inputSize, CV_8UC1);
	Mat fill_2 = Mat::zeros(c_inputSize, CV_8UC1);
	drawContours(fill_1, vector<vector<Point>>(1, samplePSeq1), 0, Scalar(255), CV_FILLED);
	drawContours(fill_2, vector<vector<Point>>(1, samplePSeq2), 0, Scalar(255), CV_FILLED);

	int strideSize = 1;
	for (int i = 0; i < fill_1.cols; i += strideSize) {
		for (int j = 0; j < fill_1.rows; j += strideSize) {
			int same = 0;
			//calculate contour area
			if (fill_1.at<uchar>(j, i) != 0) {
				same++;
				contourArea++;
				circle(drawing0, Point(i, j), 1, Scalar(0, 0, 255), 1);
			}
			//calculate food area
			if (fill_2.at<uchar>(j, i) != 0) {
				same++;
				foodArea++;
				circle(drawing0, Point(i, j), 1, Scalar(0, 255, 0), 1);
			}
			if (same == 2) {
				overlapArea++;
				circle(drawing0, Point(i, j), 1, Scalar(0, 255, 255), 1);
			}
		}
	}

	double ratio_tmp = (double)overlapArea / (double)(contourArea + foodArea - overlapArea);
	double ratio1_tmp = (double)overlapArea / (double)contourArea; // contour 
	double ratio2_tmp = (double)overlapArea / (double)foodArea; // food
	ratio1 = 1 - ratio1_tmp;
	ratio2 = 1 - ratio2_tmp;
	iError = (1 - ratio_tmp);
	//imwrite("Result0/111_" + to_string(ratio1) + "_" + to_string(ratio2) + "_" + to_string(iError) + ".png", drawing0);
	t2 = clock();
	//cout << "iErr: " << t2 - t1 << endl;
}