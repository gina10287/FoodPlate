#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>

#ifndef fragment_H
#define fragment_H

using namespace std;
using namespace cv;

class frag {
public:
	frag();

	bool hasFrag;
	int r; // reference start
	int q; // query start
	int l; // match length
	int fIndex; //file index
	int cIndex; //contour index
	int rIndex; // recipe index
	int fAmount;

	double score; //fragment score
	vector<int> recipe_num;
	Mat warpImg;
	Mat warpImgGroupOri;
	double cError; //color error
	double sError; //sum of three error
	double iError; // intersection error
	double iErrorRatio1; // intersection error
	double iErrorRatio2; // intersection error
	double icpError; // intersection error
	double icpVarianceError; // intersection error

	double scaleRatio;
	
	void setInfo(int _r, int _q, int _l, double _score, int _cIndex, int _fIndex, int _fAmount, Mat _warpImg);
	void setError(double _cError, double _iError, double _iErrorRatio1, double _iErrorRatio2, double icpError, double icpVarianceError);
	bool theSame(frag A);
private:


};

#endif