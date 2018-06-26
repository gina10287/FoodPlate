#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <map>
#include "fragment.h"
#include "food.h"

# define PI 3.1415926

using namespace std;
using namespace cv;

#ifndef compare_H
#define compare_H

struct candidateInfo {
	bool hasValue;
	int startIndex1;
	int startIndex2;
	int fake_startIndex1;
	int fake_startIndex2;
	int range;
	double score;
	Mat warpMatrix;
	Mat warpImg;
	vector<Point> matchSeqR;
	vector<Point> matchSeqQ;
	vector<Point> warpSeq;
};

struct candiList {
	bool hasValue;
	int startIndex1;
	int startIndex2;
	int fake_startIndex1;
	int fake_startIndex2;
	int range;
	double score;
	Mat warpMatrix;
	Mat warpImg;
	vector<Point> matchSeqR;
	vector<Point> matchSeqQ;
	candiList *next;
};

class comp
{
public:
	comp(Size inputSize, bool isReverse);
	comp(Size inputSize, vector<Point> &pointSeq1, int contourIndex, int foodIndex, Mat dood);
	comp(bool stackable, double lLimit, Size inputSize, vector<Mat> &descri1Seq, vector<Mat> &des2Seq, vector<Point> &pointSeq1, vector<Point> &pointSeq2, int contourIndex, int foodIndex, Mat dood, double scaleSize, vector<Point> &nonAlphaOfFood);
	comp(double lLimit, Size inputSize, vector<Mat> &descri1Seq, vector<Mat> &descri2Seq, vector<Point> &pointSeq1, vector<Point> &pointSeq2);
	
	double comp::icpIteration(int maxIcpTime, Mat &finalMat, vector<Point> &matchSeqR, vector<Point> &matchSeqQ, vector<Point> &newPointSeq, int errorMode);
	double comp::icpIteration1(int maxIcpTime, Mat &finalMat, vector<Point> &matchSeqR, vector<Point> &matchSeqQ, vector<Point> &newPointSeq, int errorMode);
	void warpMatToHomogeneous(Mat &warpMat_homo, Mat &warpMat);
	void homogeneousToWarpMat(Mat &warpMat_homo, Mat &warpMat);

	vector<map<string, int> > fragList();
	vector<frag> fragList2();
	vector<frag> fragList3();
	bool isGroup();


	bool _warpMatrixOrNot;
	Mat _warpMatrix;

private:
	void setInitial();
	void findCombination(Mat &doodImg, vector<Point> &nonAlphaOfFood);
	void findCombination2(Mat &doodImg, vector<Point> &nonAlphaOfFood);
	void findCombination3(Mat &doodImg, vector<Point> &nonAlphaOfFood);
	void setScoreThreshold();
	void compareDesNOri(Mat input1, Mat input2, int index, bool cLarge);
	void compareDesN(Mat input1, Mat input2, int index, bool cLarge);

	Mat icp(vector<Point> &contourPoint, vector<Point> &foodPointOri, vector<Point> &foodPoint, vector<int> &contourPair, vector<int> &foodPair, double &dX, double &dY, double &dTheta);
	Mat icp1(vector<Point> &contourPoint, vector<Point> &foodPointOri, vector<Point> &foodPoint, double &dX, double &dY, double &dTheta);
	Mat descriptoIcp(vector<Point> &contourPoint, vector<Point> &foodPoint, vector<Point> &foodPointOri);

	double getMinimumDistance(vector<Point> &seqP1, vector<Point> &seqP2);
	double getMinimumDistance_reverse(int times, int errorMode, vector<Point> &seqP1, vector<Point> &seqP2, Point centerP1);
	void localMaxOfRQMap(Mat &doodImg);

	void gridCutFood(Mat &doodImg);
	void cutFood();
	vector<Point> subPointSeq(vector<Point> inputSeq, int startIndex, int matchL);
	bool isExist(Point idx, vector<Point> &vec);

	bool stackOrNot(Size c_inputSize, Mat &warpImg, Mat &alphMat, double threshold);
	void imageOverlap(Size c_inputSize, vector<Point> &samplePSeq1, vector<Point> &samplePSeq2, double &_iError, double &_ratio1, double &_ratio2);

	Mat testWarpingImg(int starIdx1, int starIdx2, int range, Mat& warpMat);

	string _foodDir;
	double _lLimit;
	int _startIndex1;
	int _startIndex2;
	int _range;
	int _fIndex;
	int _cIndex;
	bool _isReverse;
	bool _isGroup;
	bool _stackable;
	double _ratio1;
	double _ratio2;
	double _iError;
	int _rDesSize;
	int _qDesSize;

	bool initScore;
	double _thresholdScore;
	double _minScore;
	Size _inputSize;

	double meanCX;
	double meanCY;
	double meanFX;
	double meanFY;

	int timess;

	Mat _mapRQ;
	Mat _mapScore;
	vector<map<string, int> > _frag;
	vector<frag> _frag2;
	vector<frag> _frag3;
	vector<map<string, int> > _totalFrag;
	vector< map<string, int> > _clearResult;
	vector<Point> _pointSeq1;
	vector<Point> _pointSeq2;

	int testTimes;
	Mat ddood;
	Mat _mapCandidateIdx;
	vector<candidateInfo> candidateResult;
};
#endif
