#pragma once
#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <errno.h>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <windows.h>

#include <QtWidgets/QMainWindow>
#include "ui_QtGuiApplication1.h"
#include "fragment.h"
#include "recipe.h"
#include "tree.hh"
#include "food.h"
#include "tpSort.h"


using namespace std;
using namespace cv;


#ifndef QtGuiApplication1_H
#define QtGuiApplication1_H

struct fragList
{
	vector<frag> Element;
};

struct independentSide {
	int startPoint;
	int length;
	vector<Point> samplePoint;
	vector<Mat> descriSeq;
};

struct foodSuggestedTable {
	int contourIdx;
	food sFood;
	frag candidate;
};

struct groupInfo {
	int angle;
	bool warpMatOrNot;
	bool flipOrNot;
	Mat warpMatOfContour;
};

struct leafNode {
	// userDraw information
	string path;
	Mat userDraw;
	Mat contourMat;

	double scaleRatio;
	Size inputSize;
	int contourAmount;
	Point globalCenter;

	// contour information of input image
	vector<Rect> boundaryOfContour; // get from svgImage
	vector<Point> centerOfContour; // get from svgImage
	vector<Vec4b> colorOfContour; // get from svgImage
	vector<vector<Point> > contourPoint; // get from svgImage
	vector<vector<Point> > samplepointsOfDrawOri; // get from svgImage
	vector<vector<Point> > samplepointsOfDraw; // get from svgImage
	vector<vector<Mat> > desOfDraw;
	vector<vector<foodSuggestedTable> > foodSuggestedVec;

	// grouping of contour
	vector<vector<int> > groupIdx;
	vector<groupInfo> group;
	//vector<Mat> groupWarpMatOfContour;
	//vector<bool> groupWarpMatOrNot;
	//vector<bool> groupFlipOrNot;

	bool preprocessingDone;
	bool preprocessingDone_B;
	bool isStoredAsNode;
	int leafNodeVecIdx;

	// comparision result
	vector<fragList> sortedFragList;
	Mat resultStack;
	double originError;
	double totalError;
	vector<int> nutrientVec;
};

class QtGuiApplication1 : public QMainWindow
{
    Q_OBJECT
		
public:
	Ui::QtGuiApplication1Class ui;
	QtGuiApplication1(QWidget *parent = Q_NULLPTR);
	
private:
	void initClicked();
	void openInputImage(string filePath);

	void updateSuggestTable(QTableWidget *table, vector<Point> &pointVec, Vec4b &color, vector<foodSuggestedTable> &foodSuggestVec);
	void updateShowTable(QTableWidget *table);
	void updateResultTable(int mode);

	void updateNutrientSlider(std::vector<int> tmpNutrient);

	void preprocessBinit(leafNode &cNode);

	void initAmount_Nutrient(int num);
	void ContourIndexClicked(leafNode &cNode);
	void mouseDragOnImage(leafNode &cNode, int state, int cValue);

	void compareWithRecipeOri(leafNode &cNode, recipe& rec2, int progressValue0, int progressValue1);
	void compareWithAllFood(leafNode &cNode, int cValue);

	void scaleBtn_show();

	private slots:
	void setLabelD(Mat &img);
	void setLabel(bool hasResultStack, Mat userDraw, Mat img, Point centerOfimg);
	void setLabel_input(Mat &img);
	bool eventFilter(QObject *obj, QEvent *);
	void QTreeWidgetClicked_Recipe(QTreeWidgetItem* item, int column);
	void QTreeWidgetClicked_Food(QTreeWidgetItem* item, int column);
	void ResultIndexClicked(QTableWidgetItem* item);
	void removeFoodItem(QTableWidgetItem* item);
	void suggestFoodItem(QTableWidgetItem* item);
	void changeAmount_Food(int currentRow);
	void changeAmount_Nutrient(int currentRow);
	void sliderRatioChange(int value);

	// button function
	void applyClicked();
	void runAllReipeClicked();
	void runAllImageClicked();
	void openImageClicked();
	void stopCompare();
	void removeAllFoodItem();
	void removeLeafNode(int column);

	void scaleBtn0_Clicked();
	void scaleBtn1_Clicked();
	void scaleBtn2_Clicked();

};
#endif
