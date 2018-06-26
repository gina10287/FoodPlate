#include <iostream>
#include <string>
#include <vector>
#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>

#ifndef food_H
#define food_H

using namespace std;
using namespace cv;

class food
{
public:
	//food(string _name, string _fileName, string _category, int _season, int _period);

	void setInitial(string _name, string _fileName, string _category, int _season, int _period);
	void setHealth(double _cal, double _protein, double _calcium, double _iron, double _vitaminB1, double _vitaminB2, double _vitaminC, double _vitaminE, double _weight, bool _stackable);
	void printInfo();
	void printHealth();

	string name;
	string fileName;
	string category;
	int season;
	int period;
	int orderIdx;

	double cal;
	double protein;
	double calcium;
	double iron;
	double vitaminB1;
	double vitaminB2;
	double vitaminC;
	double vitaminE;
	double weight;
	bool stackable;

	// new info
	int widthShiftLimit;
	int heightShiftLimit;
	Rect rect;
	Mat alphaMat;
	vector<Point> nonAlphaOfFood;

	friend bool operator == (const food &a, const food &b);
	friend bool operator != (const food &a, const food &b);

};

#endif