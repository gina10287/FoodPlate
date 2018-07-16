#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>

#include "fragment.h"

frag::frag() {
	hasFrag = false;
}

void frag::setInfo(int _r, int _q, int _l, double _score, int _cIndex, int _fIndex, int _fAmount, Mat _warpImg) {
	r = _r;
	q = _q;
	l = _l;
	score = _score;
	cIndex = _cIndex;
	fIndex = _fIndex;
	fAmount = _fAmount;
	warpImg = _warpImg;
}

void frag::setError(double _cError, double _iError, double _iErrorRatio1, double _iErrorRatio2, double _icpError, double _icpVarianceError) {
	cError = _cError;
	iError = _iError;
	iErrorRatio1 = _iErrorRatio1;
	iErrorRatio2 = _iErrorRatio2;
	icpError = _icpError;
	icpVarianceError = _icpVarianceError;
	//sError = _iError + _cError;
}

bool frag::theSame(frag A)
{
	if (q == A.q && r == A.r)
		return true;
	else if (abs(q - r) == abs(A.q - A.r))
		return true;
	else
		return false;
}