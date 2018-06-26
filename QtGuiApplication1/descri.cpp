#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

#include "descri.h"

# define PI 3.1415926

using namespace std;
using namespace cv;

// constructor
descri::descri(string &imgPath)
{
	Mat input = imread(imgPath, -1);
	imgToDes(input);
	//desToGrayImg(input);
}

descri::descri(vector<Point> contour, int mode, double scale) {
	vector<int>hull;
	convexHull(contour, hull, true);
	if (hull[0] > hull[1])
		reverse(contour.begin(), contour.end());

	//get sample points: points in vector
	getSamplePointsByScale(contour, scale);

	if (mode == 1) {
		//get descriptor
		descriptor(_sampleResult);
		//get seq descriptor
		getSeqDescriptor(_sampleResult);
		//return grayDescri;
	}
}

descri::descri(vector<Point> sampleContour) {
	_sampleResult = sampleContour;

	vector<int>hull;
	convexHull(_sampleResult, hull, true);
	if (hull[0] > hull[1])
		reverse(_sampleResult.begin(), _sampleResult.end());

	//get descriptor
	descriptor(_sampleResult);
	//get seq descriptor
	getSeqDescriptor(_sampleResult);
	//return grayDescri;
}

// inputImg to des
void descri::imgToDes(Mat input)
{
	//cout <<"not resort"<<endl;
	// resize input image
	Mat inputScaleTwo;
	//resize(input, inputScaleTwo, Size(input.cols*2, input.rows*2) );

	//to binary image with alpha value
	Mat alpha = alphaBinary(input);

	//color to Gray
	Mat inputGray;
	cvtColor(alpha, inputGray, CV_RGB2GRAY);

	//edge detection
	Mat inputCanny;
	Canny(inputGray, inputCanny, 50, 150, 3);

	// find contour
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(inputCanny, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

	vector<int>hull;
	int cindex = maxContour(contours);
	vector<Point> tmpp;
	tmpp.assign(contours[cindex].begin(), contours[cindex].end());

	convexHull(tmpp, hull, true);
	if (hull[0] > hull[1])
		reverse(tmpp.begin(), tmpp.end());

	Mat drawing = Mat::zeros(inputCanny.size(), CV_8UC1);

	//get sample points: points in vector
	getSamplePoints(tmpp);

	//get descriptor
	descriptor(_sampleResult);

	getSeqDescriptor(_sampleResult);
}

// alpha to binary
Mat descri::alphaBinary(Mat input)
{
	Mat alphaOrNot = Mat::zeros(input.size(), CV_8UC3);
	for (int i = 0; i < input.cols; i++)
	{
		for (int j = 0; j < input.rows; j++)
		{
			Vec4b & bgra = input.at<Vec4b>(j, i);
			Vec3b color = alphaOrNot.at<Vec3b>(j, i);
			if (bgra[3] != 0) // not transparency
			{
				color[0] = 255;
				color[1] = 255;
				color[2] = 255;
			}
			else
			{
				color[0] = 0;
				color[1] = 0;
				color[2] = 0;
			}
			alphaOrNot.at<Vec3b>(j, i) = color;
		}
	}
	return alphaOrNot;
}

//return index of max contour
int descri::maxContour(vector<vector<Point>> contours)
{
	int maxSize = -1;
	int maxIndex = -1;
	for (int i = 0; i < contours.size(); i++)
	{

		if (int(contours[i].size()) > maxSize)
		{
			//cout << "index: "<<i<<", size: "<<contours[i].size() << endl;
			maxSize = int(contours[i].size());
			maxIndex = i;
		}
	}
	return maxIndex;
}

// return contour length
double descri::contourLength(vector<Point> singleContour)
{
	double totalDist = 0;
	Point start;
	Point finish;
	start = singleContour[0];

	for (int i = 1; i < singleContour.size(); i++)
	{
		finish = singleContour[i];
		cout << sqrt(pow(finish.x - start.x, 2) + pow(finish.y - start.y, 2)) << endl;
		totalDist += sqrt(pow(finish.x - start.x, 2) + pow(finish.y - start.y, 2));
		//cout << "x: "<<singleContour[i].x<<endl;
		//cout << "y: "<<singleContour[i].y<<endl;
		start = finish;
	}

	return totalDist;
}

void descri::getSamplePointsByScale(vector<Point> singleContour, double scale)
{
	int pointCount = (double)singleContour.size() / 10 * scale;
	int totalPoints = (int)singleContour.size();

	vector<int> pointIndex;
	//vector<Point> samplePoints;
	int count = (int)singleContour.size();
	int tmp = 0;

	//float step = totalPoints/pointCount;

	//cout <<"step: " <<step<<endl;

	for (int i = 0; i < pointCount; i++)
	{
		tmp = 0 + ((double)i*totalPoints / pointCount);
		//cout << tmp << " ";
		pointIndex.push_back(tmp);
	}
	//cout << endl;
	//sort(pointIndex.begin(),pointIndex.end());

	_sampleResult.clear();
	for (int i = 0; i < pointIndex.size(); i++)
	{
		_sampleResult.push_back(singleContour[pointIndex[i]]);
	}
}

// return vector of sample point
void descri::getSamplePoints(vector<Point> singleContour)
{
	int pointCount = singleContour.size() / 10;
	int totalPoints = (int)singleContour.size();

	vector<int> pointIndex;
	//vector<Point> samplePoints;
	int count = (int)singleContour.size();
	int tmp = 0;

	//float step = totalPoints/pointCount;

	//cout <<"step: " <<step<<endl;

	for (int i = 0; i < pointCount; i++)
	{
		tmp = 0 + ((double)i*totalPoints / pointCount);
		//cout << tmp << " ";
		pointIndex.push_back(tmp);
	}
	//cout << endl;
	//sort(pointIndex.begin(),pointIndex.end());

	for (int i = 0; i < pointIndex.size(); i++)	{
		_sampleResult.push_back(singleContour[pointIndex[i]]);
	}
}

// get descriptor
void descri::descriptor(vector<Point> samplePoints)
{
	Point pi;
	Point pj;
	Point pjMinusDelta;

	int delta = 3;
	double tmp = 0;
	int pointsNum = (int)samplePoints.size();
	Mat shapeDes = Mat::zeros(pointsNum, pointsNum, CV_64FC1);

	for (int i = 0; i < shapeDes.rows; i++) {
		for (int j = 0; j < shapeDes.cols; j++)	{
			pi = samplePoints[i];
			pj = samplePoints[j];


			if (abs(int((i)-(j))) < delta) {
				shapeDes.at<double>(i, i) = 0;
			}
			else {
				if (i > j)
					pjMinusDelta = samplePoints[j + delta];
				else
					pjMinusDelta = samplePoints[j - delta];

				tmp = angle(pi, pj, pjMinusDelta);
				shapeDes.at<double>(i, j) = tmp;
			}
		}
	}
	_resultDescri = shapeDes;
	//return shapeDes;
}

// get seq descriptor with n start point
void descri::getSeqDescriptor(vector<Point> samplePoints)
{
	Point pi;
	Point pj;
	Point pjMinusDelta;

	int delta = 3;
	double tmp = 0;
	int pointsNum = (int)samplePoints.size();

	//start n descriptor
	for (int n = 0; n < pointsNum; n++)
	{
		Mat shapeDes = Mat::zeros(pointsNum, pointsNum, CV_64FC1);
		for (int i = 0; i < shapeDes.rows; i++)
		{
			for (int j = 0; j < shapeDes.cols; j++)
			{
				pi = samplePoints[(i + n) % pointsNum];
				pj = samplePoints[(j + n) % pointsNum];

				if (abs(int((i)-(j))) < delta)
				{
					shapeDes.at<double>(i, j) = 0;
				}
				else
				{
					if (i > j)
						pjMinusDelta = samplePoints[j + delta];
					else
						pjMinusDelta = samplePoints[j - delta];

					tmp = angle(pi, pj, pjMinusDelta);
					shapeDes.at<double>(i, j) = tmp;
				}
			}
		}
		_seqDescri.push_back(shapeDes);

	}

}

// single descriptor
Mat descri::resultDescri()
{
	return _resultDescri;
}

//sequence descriptor
vector<Mat> descri::seqDescri()
{
	//Mat tmpp = _seqDescri[0];
	/*for(int i = 0 ; i < _seqDescri.size() ; i++)
	imwrite(to_string(i)+"_.jpg", _seqDescri[i]);*/

	return _seqDescri;
}

//sample point result
vector<Point> descri::sampleResult()
{
	return _sampleResult;
}

// get angle
double descri::angle(Point i, Point j, Point jMinusDelta)
{
	//A = i, B = j, C = jMinusDelta
	double cosAngle = 0; //corner i_j_jMinusDelta
	double angle = 0;
	double distA = 0; //BC
	double distB = 0; //AC
	double distC = 0; //AB

	distA = sqrt(pow(j.x - jMinusDelta.x, 2) + pow(j.y - jMinusDelta.y, 2));
	distB = sqrt(pow(i.x - jMinusDelta.x, 2) + pow(i.y - jMinusDelta.y, 2));
	distC = sqrt(pow(i.x - j.x, 2) + pow(i.y - j.y, 2));

	if (distA == 0 || distC == 0)
		angle = 0;
	else
	{
		cosAngle = (pow(distA, 2) + pow(distC, 2) - pow(distB, 2)) / (2 * distC*distA);
		if (cosAngle>1)
			angle = 0;
		else if (cosAngle<-1)
			angle = 180;
		else
			angle = acos(cosAngle) * 180.0 / PI;
	}

	return angle;
}