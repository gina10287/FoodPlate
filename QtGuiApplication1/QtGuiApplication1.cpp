#include "QtGuiApplication1.h"
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
#include <omp.h>

#include <QDropEvent>
#include <QUrl>
#include <QFile>
#include <QTextStream>
#include <QMimeData>

#include <QApplication>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDirModel>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QStyleOptionViewItem>
#include <QtWidgets/QSlider>
#include <QSignalMapper>
#include <QPixmap>
#include <QPainter>
#include <QToolButton>
#include <QTextCodec>
#include <QToolTip>
#include <QDrag>
#include <QDebug>

#include "tree.hh"
#include "descri.h"
#include "compare.h"
#include "fragment.h"
#include "recipe.h"
#include "food.h"
#include "svgParse.h"
#include "icpSolver.h"

# define PI 3.1415926

using namespace std;
using namespace cv;

////////////////////////////////////////////////////////////////////////
struct cfMap {
	map<int, fragList> Element;
};

struct cfragMap {
	map<int, frag> Element;
};

struct foodCate {
	map<string, vector<food> > Element;
};

struct foodAmount{
	map<string, vector<int> > Element;
};

struct cateMap {
	map<string, foodCate> Element;
};

vector<frag> foodSuggestedList;

vector<leafNode> leafNodeVec;
leafNode current_LeafNode;
leafNode *current_ptr;

int sumT = 0;
vector<pair<string, int> >imgInfo;
/* --------------------------------Global Parameters-------------------------------- */
Mat dishImg = imread("Icon/dish.png", -1);

/* folder name */
string recipeDir("recipeBinC/");
vector<string> recipeFileName;
string preDataDir("preData_500/");
string foodPreDir("bin500C/");
string categoryImgDir("CategoryImg/");
string inputImageDir("inputImg0/");
string inputSVGDir("svgImg/new/");

/* Global Parameters of input image */
Size inputSize(500, 500);
Size inputSizeOri(500, 500);
Size inputSizeSmall(300, 300);

int selectContourIdx = -1;
int selectFoodIdx = -1;
food selectFood;

bool selectImageOrNot = false;
bool stopOrNot = false;

/* Global Parameters of food image */
// contour information of food image
vector<vector<Mat> > desOfFood, desOfFoodReverse;
vector<vector<Point> > samplepointsOfFood, samplepointsOfFoodReverse;
vector<vector<Point> > PointsOfFoodOri, PointsOfFoodReverseOri;
// food information of all
vector<food> totalFood;
//vector<Point> totalFood;
map<string, Mat> totalFoodImg;
map<string, map<string, int> > foodRelate;
// food category by (1)all (2)recipe
cateMap cateFood;
map<string, vector<food> > cateRecipe;
vector<string> categoryStr;

/* Selecet Food Parameters */
vector<int> currentFoodTableOrder{ 0, 1, 2, 3 }; // delete, foodMat, amount, slider
vector<QString> recipeSelectedIndex;
foodCate foodSelectedIndex;
foodAmount foodSelectedIndex_Amount;

/* Compare Parameters */
int applyMode = 0;
int nutrient_ratio = 0;
int topLabelSize = 300;
int listWidth = 100;
string recommendFood = "N";
int foodInitialAmount = 10;
int resultIdxAmount_min = 5;
int resultIdxAmount_max = 30;
int leafNodeAmount = 100;
int contourSmallAmount = 5;
double globalScaleSize = 1;

/* Color Parameters */
QColor select_Color(255, 253, 234, 200);
QString select_Color_Str("255, 253, 234, 200");
QColor selectDisable_Color(255, 253, 234, 0);
QString selectDisable_Color_Str("255, 253, 234, 0");

QString white_Color_Str("255, 255, 255, 100");
QString blue_Color_Str("127, 230, 255, 220");
QString darkBlue_Color_Str("127, 230, 255, 255");

/* Nutrient Parameters */
QTextCodec *codec = QTextCodec::codecForName("Big5-ETen");
vector<QString> nutrient_Str{ "Cal", "Pr", "Ca", "Fe", "B1", "B2", "C", "E" };
//vector<QString> nutrient_Str{ codec->toUnicode("卡路里"), codec->toUnicode("蛋白質"), codec->toUnicode("鈣"), codec->toUnicode("鐵"), codec->toUnicode("維他命B1"), codec->toUnicode("維他命B2"), codec->toUnicode("維他命C"), codec->toUnicode("維他命E") };
vector<int> nutrientValue(nutrient_Str.size(), 0);
vector<double> nutrientDay{ 2100, 50, 700, 15, 1.1, 1.2, 50, 10 }; // maxCal maxProtein maxCalcium maxIron maxVitaminB1 maxVitaminB2 maxVitaminC maxVitaminE
QString nutrient_transparent = "200";
int nutrient_transparentI = 255;
vector<QString> nutrient_Color_Str{
	"(208, 16, 76, " + nutrient_transparent + ")",
	"(204, 84, 58, " + nutrient_transparent + ")",
	"(88, 178, 220, " + nutrient_transparent + ")",
	"(148, 122, 109, " + nutrient_transparent + ")",
	"(255, 177, 27, " + nutrient_transparent + ")",
	"(233, 233, 43, " + nutrient_transparent + ")",
	"(134, 193, 102, " + nutrient_transparent + ")",
	"(119, 66, 141, " + nutrient_transparent + ")" };
vector<QColor> nutrient_Color{
	QColor(208, 16, 76, nutrient_transparentI),
	QColor(204, 84, 58, nutrient_transparentI),
	QColor(88, 178, 220, nutrient_transparentI),
	QColor(148, 122, 109, nutrient_transparentI),
	QColor(255, 177, 27, nutrient_transparentI),
	QColor(233, 233, 43, nutrient_transparentI),
	QColor(134, 193, 102, nutrient_transparentI),
	QColor(119, 66, 141, nutrient_transparentI) };

/* Main Function */
void prePocessOfUserDraw_A(leafNode &cNode, double scaleSize);
void prePocessOfUserDraw_B(leafNode &cNode);

/* Build Tree Function */
vector<string> split_str(string s, char ch);
void appendAllFrag(int state, tree<string>& tr, string c_str, int nextIndex, vector<fragList>& sortedFragList);
void stackWithAddingOrder(leafNode &cNode);
double getHealthError(vector<int> nutrient);
vector<int> getStrNutrient(vector<fragList>& sortedFragList, recipe& recipeTmp);
vector<food> getFoodVecFromStr(string name);

/* Food Combination Function */

/* Pre-Processing */
void preProcess(string s_file, vector<vector<Point> >& samplePoint, string p_file);
vector<Mat> vecmatread(const string& filename); //store food descriptor
vector<Point> vecPointRead(const string& filename); //store food samplePoint
void getAllRecipe(string recipeDir, vector<string> &files, vector<recipe>& RecipeAll);
void readFoodPreData(vector<food>& totalFood);
void readBinaryImg(vector<Mat>& totalImg, string fileName);
void getOverlapRatio(food &totalFood, vector<Point> &pointSeq);
Vec4b getFoodColor(Mat &foodImg);

/* Image Processing */
Mat doFlipMat(Mat &tmpWarpImg, Rect &rect);
Point getCenterPoint(vector<Point> &pointSeq);
vector<Point> getAllContourPoint(Mat &image);
double tmpCompareWithoutDes(bool compareWhole, int iterationTimes, Size &inputSize, vector<Point> seqP1, vector<Point> seqP2, Mat &finalMat, vector<Point> &newSampleP, int errorMode);
void grouping(leafNode &cNode, vector<vector<Point> > &samplepointsOfDrawReverse);
void getBorder(Mat &image, int& top, int& left, int& bottom, int& right);
void reversePoint(Point center, vector<Point>& contourPoint); //reverse contour with center
Mat addTransparent(Mat &bg, Mat &fg); //add image with transparent background
vector<Point> subPointSeq(vector<Point> inputSeq, int startIndex, int range); //get subPointSeq
Mat cannyThreeCh(Mat input, bool mode); //three channels Canny
int getdir(string dir, vector<string> &files); //get all files in the dir
Mat alphaBinary(Mat input); //segment image with alpha value
Mat scaleIconImg(Mat foodImg); // scale img icon
Mat scaleSuggestResultImg(Mat foodImg, vector<Point> &pointVec, Vec4b &color); // scale img icon

/* Error Function */
double colorError(Vec4b contourColor, Vec4b foodColor ); //color errorrefError

/* Comparison Value Function */
bool compareContourSize(vector<Point> contour1, vector<Point> contour2);
bool compareWithiError(frag input1, frag input2);
bool compareWithiError_cError(frag input1, frag input2);
bool compareWithsError(frag input1, frag input2);
bool compareWithError_Suggest(frag input1, frag input2);
void setAllError(frag &inputFrag, leafNode &cNode, int contourIdx);

/* Comparison Contour Function */
void setFragToGroup(leafNode &cNode, int contourIdx1, fragList &fragList1, int contourIdx2, fragList &fragList2);
int finGroupIdx(vector<vector<int> > &cGroupIdx, int idx);
void doCompare(bool userMove, fragList& pairSeq, leafNode &cNode, int contourIdx, food &currentFood);

/* test function */
void test_contourMat(leafNode &cNode);
void test_icp();

double getMinDistance(vector<Point> seqTarget, vector<Point> seqSource);

Mat RotateMat(Mat &img, float rad);
Mat overlayImage(Mat &bg, Mat &fg, Point &p);
vector<Point> RotatePoint(vector<Point> &pointSeq, float rad, Size imgSize);
vector<Point> RotatePoint2(Mat &img, vector<Point> smapleP, float rad);
Rect getBoundaryRectAndAlpha(Mat img, Rect &rect, vector<Point> &nonAlpha);

/* --------------------------------UI Function-------------------------------- */
void removeFileExtension(vector<string>& fileName) {
	for (int i = 0; i < fileName.size(); i++) {
		vector<string> strs = split_str(fileName[i], '.');
		fileName[i] = strs[0];
	}
}

QString getToolTipQString(food& currentFood) {
	QString nutrient_Str_Color = " bgcolor=\"#c4fcff\"";
	QString nutrient_Value_Color = " bgcolor=\"#fafce3\"";

	QString html =
		"<h3> <strong>" + codec->toUnicode(currentFood.name.c_str()) + "</strong> </h3>"
		"<table  style=\" margin-top:3px; margin-bottom:0px; margin-left:0px; margin-right:0px; \" cellspacing=\"0\" cellpadding=\"0\"  border=\"1\" font face=\"DFKai - sb\">"
		"<tr> <td" + nutrient_Str_Color + ">" + nutrient_Str[0] + "</td>  <td" + nutrient_Value_Color + ">" + QString::number(100 * currentFood.cal / nutrientDay[0]) + "%</td>  </tr>"
		"<tr> <td" + nutrient_Str_Color + ">" + nutrient_Str[1] + "</td>  <td" + nutrient_Value_Color + ">" + QString::number(100 * currentFood.protein / nutrientDay[1]) + "%</td>  </tr>"
		"<tr> <td" + nutrient_Str_Color + ">" + nutrient_Str[2] + "</td>  <td" + nutrient_Value_Color + ">" + QString::number(100 * currentFood.calcium / nutrientDay[2]) + "%</td>  </tr>"
		"<tr> <td" + nutrient_Str_Color + ">" + nutrient_Str[3] + "</td>  <td" + nutrient_Value_Color + ">" + QString::number(100 * currentFood.iron / nutrientDay[3]) + "%</td>  </tr>"
		"<tr> <td" + nutrient_Str_Color + ">" + nutrient_Str[4] + "</td>  <td" + nutrient_Value_Color + ">" + QString::number(100 * currentFood.vitaminB1 / nutrientDay[4]) + "%</td>  </tr>"
		"<tr> <td" + nutrient_Str_Color + ">" + nutrient_Str[5] + "</td>  <td" + nutrient_Value_Color + ">" + QString::number(100 * currentFood.vitaminB2 / nutrientDay[5]) + "%</td>  </tr>"
		"<tr> <td" + nutrient_Str_Color + ">" + nutrient_Str[6] + "</td>  <td" + nutrient_Value_Color + ">" + QString::number(100 * currentFood.vitaminC / nutrientDay[6]) + "%</td>  </tr>"
		"<tr> <td" + nutrient_Str_Color + ">" + nutrient_Str[7] + "</td>  <td" + nutrient_Value_Color + ">" + QString::number(100 * currentFood.vitaminE / nutrientDay[7]) + "%</td>  </tr>"
		"</table>";
	return html;
}

void initTab(QTabWidget *tab) {
	//tab->setStyleSheet(QString("QTabBar::tab { height: 45px; width: %1px; } ").arg(s_width / tab->count()) + "QTabWidget{ background-color: transparent; font-size: " + QString::number(18) + "pt; font: bold large 'Times New Roman';}"
	tab->setStyleSheet("QTabBar::tab { height: 45px; } "
		//"QTabWidget{ background-color: transparent; font-size: " + QString::number(18) + "pt; font: bold large 'Times New Roman';}"
		"QTabWidget{ background-color: transparent; font-size: " + QString::number(18) + "pt;}"
		"QTabBar::tab:selected { background-color: rgba(" + blue_Color_Str + "); }"
		"QTabBar::tab-bar { border: none; left: 5px; }"
		//"QTabWidget::pane{ border-top: 2px solid; background-color: rgba(" + white_Color_Str + ");}");
		"QTabWidget::pane{ border-top: 1px solid; background-color: rgba(" + white_Color_Str + ");}");
}

void initTree(QTreeWidget *tree) {
	tree->header()->close();
	tree->setIconSize(QSize(70, 70));
	tree->setExpandsOnDoubleClick(false);
	tree->setMouseTracking(true);
	tree->setStyleSheet("QTreeWidget { border: none; font-size: " + QString::number(16) + "pt; font: bold large 'Times New Roman'; background-color: rgba(" + selectDisable_Color_Str + ");  show-decoration-selected: 1;}"
		// https://www.ics.com/files/qtdocs/stylesheet-examples.html
		//"QTreeView::branch:open:has-children{ background-color:rgba(" + white_Color_Str + "); border-image: url(indicator_open.png) 0; }"
		"QTreeView::branch:open:has-children{ border-image: url(Icon/indicator_open.png) 0; }"
		"QTreeView::branch:closed:has-children{ border-image: url(Icon/indicator_close.png) 0; }"
		);
}

void initLabel(QLabel *label) {
	label->setAlignment(Qt::AlignCenter);
	label->setStyleSheet("background-color: rgba(" + selectDisable_Color_Str + "); QLabelView { border: none; }");
	label->setScaledContents(true);
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void initTable(QTableWidget *table) {
	table->setShowGrid(false);
	table->horizontalHeader()->hide();
	table->verticalHeader()->hide();
	table->resizeRowsToContents();
}

void initNutrientTable(QTableWidget *table) {
	table->setStyleSheet("QTableWidget {border: none; background-color: rgba(" + white_Color_Str + ");}"
		"QTableWidget::item {padding: 5px;}"
		"QHeaderView::section {background-color: transparent;}"
		"QHeaderView {background-color: transparent;}"
		"QTableCornerButton::section {background-color: transparent;} ");
	table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	table->horizontalHeader()->setStretchLastSection(true);
	table->setRowCount(nutrient_Str.size());
	table->resizeRowsToContents();
	table->setColumnCount(3);
	//table->setColumnWidth(0, 50);
	//table->setColumnWidth(1, 50);

	for (int i = 0; i < nutrient_Str.size(); i++) {
		QTableWidgetItem *qItemName = new QTableWidgetItem(nutrient_Str[i]);
		qItemName->setTextAlignment(Qt::AlignCenter);
		table->setItem(i, 0, qItemName);
		//table->item(i, 0)->setTextColor(nutrient_Color[i]);

		QFont fnt;
		fnt.setPointSize(12);
		table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(to_string(nutrientValue[i]) + "%")));
		table->item(i, 1)->setFont(fnt);
	}
	table->resizeColumnsToContents();
}

void QtGuiApplication1::setLabelD(Mat &img) {
	QPixmap pix = QPixmap::fromImage(QImage((uchar*)img.data, img.cols, img.rows, QImage::Format_ARGB32));
	QPixmap resPix = pix.scaled(topLabelSize, topLabelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_show->setPixmap(resPix);
}

// has userDraw as background
void QtGuiApplication1::setLabel_show(bool hasResultStack, Mat userDraw, Mat img, Point centerOfimg) {
	Size dishSize = dishImg.size();
	Size newSize(dishSize.width - 80, dishSize.height - 80);
	cv::resize(img, img, newSize);
	cv::resize(userDraw, userDraw, newSize);

	//  do same as overlayImage, just set transparent to 1/10
	//Mat finalMat = overlayImage(dishImg, userDraw, Point((dishSize.width - newSize.width) / 2, (dishSize.height - newSize.height) / 2));
	Mat finalMat = dishImg.clone();
	/*Point startC((dishSize.width - newSize.width) / 2, (dishSize.height - newSize.height) / 2);
	for (int i = 0; i < userDraw.cols; i++) {
		for (int j = 0; j < userDraw.rows; j++) {
			if (userDraw.at<Vec4b>(j, i)[3] != 0) {
				finalMat.at<Vec4b>(j + startC.y, i + startC.x) = userDraw.at<Vec4b>(j, i);
				finalMat.at<Vec4b>(j + startC.y, i + startC.x)[3] /= 10;
			}
		}
	}*/

	if (hasResultStack)
		finalMat = overlayImage(finalMat, img, Point((dishSize.width - newSize.width) / 2, (dishSize.height - newSize.height) / 2));

	QPixmap pix = QPixmap::fromImage(QImage((uchar*)finalMat.data, finalMat.cols, finalMat.rows, QImage::Format_ARGB32));
	QPixmap resPix = pix.scaled(topLabelSize, topLabelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_show->setPixmap(resPix);
}

void QtGuiApplication1::setLabel_input(Mat &img) {
	QPixmap pix = QPixmap::fromImage(QImage((uchar*)img.data, img.cols, img.rows, QImage::Format_ARGB32));
	QPixmap resPix = pix.scaled(topLabelSize, topLabelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_input->setPixmap(resPix);
}

// calculate contourMat
void label2_feature(leafNode &cNode) {
	int longest = cNode.userDraw.cols;
	if (cNode.userDraw.cols < cNode.userDraw.rows) { longest = cNode.userDraw.rows; }

	double ratioW = (double)topLabelSize / (double)cNode.inputSize.width;
	double ratioH = (double)topLabelSize / (double)cNode.inputSize.height;
	vector<vector<Point> > disjointContourTmp = cNode.samplepointsOfDrawOri;
	for (int i = 0; i < disjointContourTmp.size(); i++)	{
		for (int p = 0; p < disjointContourTmp[i].size(); p++) {
			disjointContourTmp[i][p].x = (double)disjointContourTmp[i][p].x * ratioW;
			disjointContourTmp[i][p].y = (double)disjointContourTmp[i][p].y * ratioH;
		}
	}

	Mat contour_dist(Size(topLabelSize, topLabelSize), CV_32FC1);
	Mat thisContourM = Mat::zeros(Size(topLabelSize, topLabelSize), CV_32S);
	for (int i = 0; i < topLabelSize; i += 1) {
		for (int j = 0; j < topLabelSize; j += 1) {
			int tmpI = i*longest / topLabelSize;
			int tmpJ = j*longest / topLabelSize;

			if (cNode.userDraw.at<Vec4b>(tmpJ, tmpI)[3] != 0) {
				bool hasValue = false;
				for (int k = disjointContourTmp.size() - 1; k >= 0; k--) {
					contour_dist.at<float>(j, i) = pointPolygonTest(disjointContourTmp[k], Point2f(i, j), true);
					//calculate contour area
					if (contour_dist.at<float>(j, i) > 0) {
						hasValue = true;
						thisContourM.at<int>(j, i) = k;
						break;
					}
				}
				if (!hasValue)
					thisContourM.at<int>(j, i) = -1;
			}
			else
				thisContourM.at<int>(j, i) = -1;
		}
	}
	cNode.contourMat = thisContourM;
}

/* --------------------------------Slot Function-------------------------------- */
/*recommend food*/
string getRecommendFood()
{
	string maxKey = "N";
	int maxValue = 0;
	map<string, vector<food> >::iterator iterF;
	for (iterF = foodSelectedIndex.Element.begin(); iterF != foodSelectedIndex.Element.end(); iterF++)
	{
		map<string, int>::iterator iter;
		vector<string>::iterator iterS;
		for (iter = foodRelate[iterF->first].begin(); iter != foodRelate[iterF->first].end(); iter++)
		{
			map<string, vector<food>>::iterator iterFindFood;
			iterFindFood = foodSelectedIndex.Element.find(iter->first);
			//iterS = find(fN.begin(), fN.end(), iter->first);
			if (iter->second > maxValue && iterFindFood == foodSelectedIndex.Element.end())
			{
				maxKey = iter->first;
				maxValue = iter->second;

				break;
			}
		}
	}
	return maxKey;
}

string getChinese(QString str) {
	QTextCodec *codec = QTextCodec::codecForName("Big5");
	QByteArray encodedString = codec->fromUnicode(str);
	return encodedString.data();
}

food getfoodCateFood(foodCate &foodC, int index) {
	map<string, vector<food>>::iterator iterSelect = foodC.Element.begin();
	for (int it = 0; it < index; it++)
		iterSelect++;
	food tmpF = iterSelect->second[0];
	return tmpF;
}

void QtGuiApplication1::updateSuggestTable(QTableWidget *table, vector<Point> &pointVec, Vec4b &color, vector<frag> &foodSuggestVec) {
	sort(foodSuggestVec.begin(), foodSuggestVec.end(), compareWithError_Suggest);

	table->clear();
	table->setRowCount(foodSuggestVec.size());
	table->setStyleSheet("QTableWidget {border: none; background-color: rgba(" + white_Color_Str + ");}"
		"QTableWidget::item {background-color: rgba(" + selectDisable_Color_Str + "); padding: 5px;}"
		"QHeaderView::section {background-color: transparent;}"
		"QHeaderView {background-color: transparent;}"
		"QTableCornerButton::section {background-color: transparent;} ");

	for (int i = 0; i < foodSuggestVec.size(); i++) {
		//food foodTmp = foodSuggestVec[i].sFood;
		food foodTmp = totalFood[foodSuggestVec[i].fIndex];

		Mat modifyFoodImage = scaleIconImg(totalFoodImg[foodTmp.fileName]);
		QPixmap pixelImage = QPixmap::fromImage(QImage((uchar*)modifyFoodImage.data, modifyFoodImage.cols, modifyFoodImage.rows, QImage::Format_ARGB32));
		QPixmap resPix = pixelImage.scaled(75, 75, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QTableWidgetItem *pCell = new QTableWidgetItem();
		pCell->setData(Qt::DecorationRole, resPix);
		table->setItem(i, 0, pCell);
		QString html = getToolTipQString(foodTmp);
		pCell->setToolTip(html);

		//QTableWidgetItem *qItem = new QTableWidgetItem(codec->toUnicode(foodTmp.name.c_str()));
		QTableWidgetItem *qItem = new QTableWidgetItem("_" + QString::number(foodSuggestVec[i].sError));// +"_" + QString::number(foodSuggestVec[i].candidate.iErrorRatio1) + "_" + QString::number(samplepointsOfFood[foodTmp.orderIdx].size()));

		qItem->setTextAlignment(Qt::AlignCenter);
		table->setItem(i, 1, qItem);

		Mat modifyFoodImage2 = scaleSuggestResultImg(foodSuggestVec[i].warpImg, pointVec, color);

		QPixmap pixelImage2 = QPixmap::fromImage(QImage((uchar*)modifyFoodImage2.data, modifyFoodImage2.cols, modifyFoodImage2.rows, QImage::Format_ARGB32));
		QPixmap resPix2 = pixelImage2.scaled(75, 75, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QTableWidgetItem *pCell2 = new QTableWidgetItem();
		pCell2->setData(Qt::DecorationRole, resPix2);
		table->setItem(i, 2, pCell2);
	}
}

void QtGuiApplication1::updateShowTable(QTableWidget *table) {
	// recommend Food
	int recommendOrNot = 0;
	recommendFood = getRecommendFood();

	//cout << "!recF: "<< recommendFood << endl;
	if (recommendFood != "N")
		recommendOrNot = 1;

	//table->clearContents();
	table->clear();
	
	/*ui.tableWidget->setStyleSheet("QTableWidget {border: none; background-color: rgba(" + white_Color_Str + ");}"
		"QTableWidget::item {background-color: rgba(" + select_Color_Str + "); padding: 5px;}"
		"QHeaderView::section {background-color: transparent;}"
		"QHeaderView {background-color: transparent;}"
		"QTableCornerButton::section {background-color: transparent;} ");
	ui.tableWidget->setColumnCount(4);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->setColumnWidth(0, 35);
	ui.tableWidget->setColumnWidth(1, 80);
	ui.tableWidget->setColumnWidth(2, 80);
	ui.tableWidget->verticalHeader()->setDefaultSectionSize(80);
	connect(ui.tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(removeFoodItem(QTableWidgetItem*)));
	*/

	table->setRowCount(foodSelectedIndex.Element.size() + recommendOrNot);
	table->setStyleSheet("QTableWidget {border: none; background-color: rgba(" + white_Color_Str + ");}"
		"QTableWidget::item {background-color: rgba(" + selectDisable_Color_Str + "); padding: 5px;}"
		"QHeaderView::section {background-color: transparent;}"
		"QHeaderView {background-color: transparent;}"
		"QTableCornerButton::section {background-color: transparent;} ");

	/*add stuff inside the table view*/
	map<string, vector<food>>::iterator iterSelect;
	map<string, vector<int>>::iterator iterSelectAmount = foodSelectedIndex_Amount.Element.begin();
	int count = 0;
	for (iterSelect = foodSelectedIndex.Element.begin(); iterSelect != foodSelectedIndex.Element.end(); iterSelect++) {
		table->setSpan(count, currentFoodTableOrder[2], 1, 1); //sets the 1st row 1st column cell to span over all the columns

		food foodTmp = iterSelect->second[0];
		int foodAmountTmp = iterSelectAmount->second[0];

		QPixmap pixelImage_delete = QPixmap::fromImage(QImage("Icon/delete.png"));
		QPixmap resPix_delete = pixelImage_delete.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		//QPixmap pixelImage_delete("Icon/delete.png");
		QTableWidgetItem *pCell_delete = new QTableWidgetItem();
		pCell_delete->setData(Qt::DecorationRole, resPix_delete);
		table->setItem(count, currentFoodTableOrder[0], pCell_delete);
	
		Mat modifyFoodImage = scaleIconImg(totalFoodImg[foodTmp.fileName]);
		QPixmap pixelImage = QPixmap::fromImage(QImage((uchar*)modifyFoodImage.data, modifyFoodImage.cols, modifyFoodImage.rows, QImage::Format_ARGB32));
		QPixmap resPix = pixelImage.scaled(75, 75, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QTableWidgetItem *pCell = new QTableWidgetItem();
		pCell->setData(Qt::DecorationRole, resPix);
		table->setItem(count, currentFoodTableOrder[1], pCell);
		QString html = getToolTipQString(foodTmp);
		pCell->setToolTip(html);

		/*
		QTableWidgetItem *qItem = new QTableWidgetItem(codec->toUnicode(foodTmp.name.c_str()));
		qItem->setTextAlignment(Qt::AlignCenter);
		table->setItem(count, currentFoodTableOrder[2], qItem);
		*/
		///*
		QSlider *sliderTmp = new QSlider(Qt::Horizontal, table);
		sliderTmp->setMinimum(1);
		sliderTmp->setMaximum(20);
		sliderTmp->setValue(foodAmountTmp);
		sliderTmp->setStyleSheet(
			"QSlider::groove:horizontal { border: 1px solid #999999; height: 8px; background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #B1B1B1, stop:1 #c4c4c4); }"
			"QSlider::handle:horizontal{ width: 5px; background: #69c2ef; border: 2px solid #004370; margin: -8px 0; border-radius: 15px;}"
			"QSlider::sub-page:horizontal{	background: #69c2ef; }"
			);
		table->setCellWidget(count, currentFoodTableOrder[3], sliderTmp);
		
		QTableWidgetItem *qItem = new QTableWidgetItem(QString::number(sliderTmp->value()));
		qItem->setTextAlignment(Qt::AlignCenter);
		table->setItem(count, currentFoodTableOrder[2], qItem);
		QSignalMapper* signalMapper = new QSignalMapper(this);
		connect(sliderTmp, SIGNAL(valueChanged(int)), signalMapper, SLOT(map()));
		signalMapper->setMapping(sliderTmp, count);
		connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(changeAmount_Food(int)));
		//*/
		count++;
		iterSelectAmount++;
	}
	if (recommendOrNot == 1) {
		/* add recommend */
		food foodTmp = getFoodVecFromStr(recommendFood)[0];
		int foodAmountTmp = foodInitialAmount;

		QPixmap pixelImage_delete = QPixmap::fromImage(QImage("Icon/add.png"));
		QPixmap resPix_delete = pixelImage_delete.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		//QPixmap pixelImage_delete(""Icon/delete.png");
		QTableWidgetItem *pCell_delete = new QTableWidgetItem();
		pCell_delete->setData(Qt::DecorationRole, resPix_delete);
		table->setItem(count, currentFoodTableOrder[0], pCell_delete);

		Mat modifyFoodImage = scaleIconImg(totalFoodImg[foodTmp.fileName]);
		QPixmap pixelImage = QPixmap::fromImage(QImage((uchar*)modifyFoodImage.data, modifyFoodImage.cols, modifyFoodImage.rows, QImage::Format_ARGB32));
		QPixmap resPix = pixelImage.scaled(75, 75, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QTableWidgetItem *pCell = new QTableWidgetItem();
		pCell->setData(Qt::DecorationRole, resPix);
		table->setItem(count, currentFoodTableOrder[1], pCell);
		QString html = getToolTipQString(foodTmp);
		pCell->setToolTip(html);

		table->setSpan(count, currentFoodTableOrder[2], 1, 2); //sets the 1st row 1st column cell to span over all the columns
		/*
		QTableWidgetItem *qItem = new QTableWidgetItem(codec->toUnicode(recommendFood.c_str()));
		qItem->setTextAlignment(Qt::AlignCenter);
		table->setItem(count, currentFoodTableOrder[2], qItem);
		*/
		///*
		QTableWidgetItem *qItem = new QTableWidgetItem(codec->toUnicode(recommendFood.c_str()));
		qItem->setTextAlignment(Qt::AlignCenter);
		table->setItem(count, currentFoodTableOrder[2], qItem);
		//*/
	}
}

/* Recipe TreeItem Clicked Function */
void setColor_Recipe(QTreeWidget* tree1, QTreeWidget* tree2, food& tmpF, QBrush color) {
	for (int i = 0; i < tree1->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = tree1->topLevelItem(i);
		for (int j = 0; j < item->childCount(); j++) {
			food tmpF2 = cateRecipe[getChinese(item->text(0))][j];
			if (tmpF2 == tmpF) {
				item->child(j)->setBackground(0, color);
			}
		}
	}
	for (int i = 0; i < tree2->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = tree2->topLevelItem(i);
		for (int j = 0; j < item->childCount(); j++) {
			map<string, vector<food> >::iterator iterFood;
			iterFood = cateFood.Element[getChinese(item->text(0))].Element.begin();
			advance(iterFood, j);
			food tmpF2 = iterFood->second[0];
			if (tmpF2 == tmpF) {
				item->child(j)->setBackground(0, color);
			}
		}
	}
}

void QtGuiApplication1::QTreeWidgetClicked_Recipe(QTreeWidgetItem* item, int column) {
	if (item->parent()) {
		int currentIdx = item->parent()->indexOfChild(item);
		food tmpF = cateRecipe[getChinese(item->parent()->text(0))][currentIdx];
		
		map<string, vector<food>>::iterator iterFindFood;
		map<string, vector<int>>::iterator iterFindFoodAmount;
		iterFindFood = foodSelectedIndex.Element.find(tmpF.name);
		iterFindFoodAmount = foodSelectedIndex_Amount.Element.find(tmpF.name);

		if (iterFindFood != foodSelectedIndex.Element.end()) { // already in food Select Index
			item->setBackground(0, QBrush(selectDisable_Color));
			setColor_Recipe(ui.treeWidget, ui.treeWidget_2, tmpF, QBrush(selectDisable_Color));

			foodSelectedIndex.Element.erase(iterFindFood);
			foodSelectedIndex_Amount.Element.erase(iterFindFoodAmount);
		}
		else { // not exist, need add vector of food
			item->setBackground(0, QBrush(select_Color));
			setColor_Recipe(ui.treeWidget, ui.treeWidget_2, tmpF, QBrush(select_Color));

			vector<food> tmpFoodVec = getFoodVecFromStr(tmpF.name);
			foodSelectedIndex.Element[tmpF.name] = tmpFoodVec;
			vector<int> tmpAmount(tmpFoodVec.size(), foodInitialAmount);
			foodSelectedIndex_Amount.Element[tmpF.name] = tmpAmount;
		}
	}
		
	else {
		bool hasRecipe = false;
		for (int i = 0; i < recipeSelectedIndex.size(); i++) {
			if (item->text(0) == recipeSelectedIndex[i]) {
				hasRecipe = true;
				recipeSelectedIndex.erase(recipeSelectedIndex.begin() + i);
				break;
			}
		}
		// if user cancel to select the whole recipe
		if (hasRecipe) {
			item->setBackground(0, QBrush(selectDisable_Color));
			//item->setExpanded(false);

			for (int i = 0; i < item->childCount(); i++) {
				food tmpF = cateRecipe[getChinese(item->text(0))][i];

				map<string, vector<food>>::iterator iterFindFood;
				map<string, vector<int>>::iterator iterFindFoodAmount;
				iterFindFood = foodSelectedIndex.Element.find(tmpF.name);
				iterFindFoodAmount = foodSelectedIndex_Amount.Element.find(tmpF.name);

				if (iterFindFood != foodSelectedIndex.Element.end()) { // already in food Select Index
					item->child(i)->setBackground(0, QBrush(selectDisable_Color));
					setColor_Recipe(ui.treeWidget, ui.treeWidget_2, tmpF, QBrush(selectDisable_Color));
					foodSelectedIndex.Element.erase(iterFindFood);
					foodSelectedIndex_Amount.Element.erase(iterFindFoodAmount);
				}
			}
		}
		// if user select the whole recipe
		else {
			//item->setBackground(0, QBrush(select_Color));
			item->setExpanded(true);
			recipeSelectedIndex.push_back(item->text(0));

			for (int i = 0; i < item->childCount(); i++) {
				food tmpF = cateRecipe[getChinese(item->text(0))][i];

				map<string, vector<food>>::iterator iterFindFood;
				iterFindFood = foodSelectedIndex.Element.find(tmpF.name);
				if (iterFindFood == foodSelectedIndex.Element.end()) { // already in food Select Index
					//item->child(i)->setBackground(0, QBrush(select_Color));
					setColor_Recipe(ui.treeWidget, ui.treeWidget_2, tmpF, QBrush(select_Color));

					vector<food> tmpFoodVec = getFoodVecFromStr(tmpF.name);
					foodSelectedIndex.Element[tmpF.name] = tmpFoodVec;
					vector<int> tmpAmount(tmpFoodVec.size(), foodInitialAmount);
					foodSelectedIndex_Amount.Element[tmpF.name] = tmpAmount;
				}
			}
		}
		
	}
	updateShowTable(ui.tableWidget_selectedFood);
}

/* Food TreeItem Clicked Function */
void setColor_Food(QTreeWidget* tree1, food& tmpF, QBrush color) {
	for (int i = 0; i < tree1->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = tree1->topLevelItem(i);
		for (int j = 0; j < item->childCount(); j++) {
			food tmpF2 = cateRecipe[getChinese(item->text(0))][j];
			if (tmpF2 == tmpF) {
				item->child(j)->setBackground(0, color);
			}
		}
	}
}

void QtGuiApplication1::QTreeWidgetClicked_Food(QTreeWidgetItem* item, int column) {
	if (item->parent()) {
		int currentIdx = item->parent()->indexOfChild(item);
		map<string, vector<food> >::iterator iterFood;
		iterFood = cateFood.Element[getChinese(item->parent()->text(0))].Element.begin();
		advance(iterFood, currentIdx);
		vector<food> tmpFoodVec = iterFood->second;
		string newFoodName = iterFood->first;
		
		map<string, vector<food>>::iterator iterFindFood;
		map<string, vector<int>>::iterator iterFindFoodAmount;
		iterFindFood = foodSelectedIndex.Element.find(newFoodName);
		iterFindFoodAmount = foodSelectedIndex_Amount.Element.find(newFoodName);

		if (iterFindFood != foodSelectedIndex.Element.end()) { // already in food Select Index
			item->setBackground(0, QBrush(selectDisable_Color));
			setColor_Food(ui.treeWidget, iterFindFood->second[0], QBrush(selectDisable_Color));

			foodSelectedIndex.Element.erase(iterFindFood);
			foodSelectedIndex_Amount.Element.erase(iterFindFoodAmount);
		}
		else { // not exist, need add vector of food
			food tmpF = iterFood->second[0];
			item->setBackground(0, QBrush(select_Color));
			setColor_Food(ui.treeWidget, tmpF, QBrush(select_Color));

			foodSelectedIndex.Element[newFoodName] = iterFood->second;
			vector<int> tmpAmount(iterFood->second.size(), foodInitialAmount);
			foodSelectedIndex_Amount.Element[newFoodName] = tmpAmount;
		}
	}
	else {
		item->setExpanded(!item->isExpanded());
	}
	updateShowTable(ui.tableWidget_selectedFood);
}

void  QtGuiApplication1::removeAllFoodItem() {
	map<string, vector<food>>::iterator iterSelect;
	for (iterSelect = foodSelectedIndex.Element.begin(); iterSelect != foodSelectedIndex.Element.end(); iterSelect++) {
		food tmpF = getFoodVecFromStr(iterSelect->first)[0];
		setColor_Recipe(ui.treeWidget, ui.treeWidget_2, tmpF, QBrush(selectDisable_Color));
	}
	foodSelectedIndex.Element.clear();
	foodSelectedIndex_Amount.Element.clear();
	updateShowTable(ui.tableWidget_selectedFood);
}

void QtGuiApplication1::removeLeafNode(int column) {
	imwrite("Result0/leafNode.png", current_ptr->resultStack);
	/*cout << "column00= " << column << endl;
	cout << "column01= " << current_ptr->leafNodeVecIdx << endl;
	if (column == current_ptr->leafNodeVecIdx) {
		cout << "column10= " << column << endl;
		cout << "column11= " << current_ptr->leafNodeVecIdx << endl;
		current_ptr->isStoredAsNode = false;
		current_ptr->preprocessingDone = false;
		current_ptr->preprocessingDone_B = false;

		Mat tmpImg = Mat::zeros(Size(topLabelSize, topLabelSize), CV_8UC4);
		setLabelD(dishImg);
		setLabel_input(tmpImg);
	}

	leafNodeVec.erase(leafNodeVec.begin() + column);
	for (int i = column; i < leafNodeVec.size(); i++) {
		leafNodeVec[i].leafNodeVecIdx -= 1;
	}
	updateResultTable(1);*/
}

/* Remove new food item into ui.tableWidget */
void QtGuiApplication1::removeFoodItem(QTableWidgetItem* item) {
	selectFoodIdx = -1;
	if (item->column() == currentFoodTableOrder[0]) {
		if (item->row() < foodSelectedIndex.Element.size()) { //delete food
			map<string, vector<food> >::iterator iterFood = foodSelectedIndex.Element.begin();
			map<string, vector<int> >::iterator iterFoodAmount = foodSelectedIndex_Amount.Element.begin();
			advance(iterFood, item->row());
			advance(iterFoodAmount, item->row());
			food tmpF = iterFood->second[0];

			setColor_Recipe(ui.treeWidget, ui.treeWidget_2, tmpF, QBrush(selectDisable_Color));
			foodSelectedIndex.Element.erase(iterFood);
			foodSelectedIndex_Amount.Element.erase(iterFoodAmount);
			updateShowTable(ui.tableWidget_selectedFood);
		}
		else { //add food
			vector<food> tmpFoodVec = getFoodVecFromStr(recommendFood);
			foodSelectedIndex.Element[recommendFood] = tmpFoodVec;
			vector<int> tmpAmount(tmpFoodVec.size(), foodInitialAmount);
			foodSelectedIndex_Amount.Element[recommendFood] = tmpAmount;

			setColor_Recipe(ui.treeWidget, ui.treeWidget_2, tmpFoodVec[0], QBrush(select_Color));
			updateShowTable(ui.tableWidget_selectedFood);
		}
	}
	// if select food to map that contour
	else if (item->column() == currentFoodTableOrder[1]) {
		selectFoodIdx = item->row();
		selectFood = getfoodCateFood(foodSelectedIndex, selectFoodIdx);
	}

}

void QtGuiApplication1::suggestFoodItem(QTableWidgetItem* item) {
	if (item->column() == 2) {
		int cRow = item->row();
		if (cRow < foodSuggestedList.size()) {
			int c_contourIdx = foodSuggestedList[cRow].cIndex;
			//cout << "contour= " << c_contourIdx << endl;
			fragList frag;
			frag.Element.push_back(foodSuggestedList[cRow]);
			current_ptr->sortedFragList[c_contourIdx] = frag;
			//cout << "000000000000000" << current_ptr->addingOrder.size() << endl;
			stackWithAddingOrder(*current_ptr);

			if (!current_ptr->isStoredAsNode) {
				current_ptr->isStoredAsNode = true;
				current_ptr->leafNodeVecIdx = leafNodeVec.size();
				leafNodeVec.push_back(*current_ptr);
				current_ptr = &leafNodeVec[leafNodeVec.size() - 1];
			}
			updateResultTable(0);
		}
	}
}

/* Change food amount in ui.tableWidget_selectedFood */
void QtGuiApplication1::changeAmount_Food(int currentRow) {
	map<string, vector<int> >::iterator iterFoodAmount = foodSelectedIndex_Amount.Element.begin();
	advance(iterFoodAmount, currentRow);

	QSignalMapper* signalMapper = qobject_cast<QSignalMapper*>(sender());
	QSlider *sliderTmp = qobject_cast<QSlider*>(signalMapper->mapping(currentRow));
	QTableWidgetItem *qItem = new QTableWidgetItem(QString::number(sliderTmp->value()));
	qItem->setTextAlignment(Qt::AlignCenter);
	ui.tableWidget_selectedFood->item(currentRow, currentFoodTableOrder[2])->setText(QString::number(sliderTmp->value()));

	vector<int>::iterator fN;
	for (fN = iterFoodAmount->second.begin(); fN != iterFoodAmount->second.end(); fN++)
		*fN = sliderTmp->value();
}

void QtGuiApplication1::preprocessBinit(leafNode &cNode) {
	prePocessOfUserDraw_B(cNode);
	if (cNode.scaleRatio == 0.5)
		scaleBtn0_Clicked();
	else if (cNode.scaleRatio == 1)
		scaleBtn1_Clicked();
	else if (cNode.scaleRatio == 2)
		scaleBtn2_Clicked();
	
	qApp->processEvents();
}

/* Change the big image on table when click anither index */
void QtGuiApplication1::ResultIndexClicked(QTableWidgetItem* item) {
	int currentColumn = item->column();

	current_LeafNode = leafNodeVec[currentColumn];
	current_ptr = &leafNodeVec[currentColumn];

	setLabel_show(true, current_ptr->userDraw, current_ptr->resultStack, current_ptr->globalCenter);
	setLabel_input(current_ptr->userDraw);

	foodSuggestedList.clear();
	updateSuggestTable(ui.tableWidget_3, vector<Point>(), Vec4b(), foodSuggestedList);
}

/* --------------------------------Nutrient Function-------------------------------- */
void QtGuiApplication1::initAmount_Nutrient(int num) {
	//for (int i = 0; i < nutrientValue.size(); i++) {
		//nutrientValue[i] = leafNodeVec[0].nutrientVec[i];
	//}

	ui.tableWidget_2->clear();
	initTable(ui.tableWidget_2);
	initNutrientTable(ui.tableWidget_2);
	for (int i = 0; i < nutrient_Str.size(); i++) {
		QSlider *sliderTmp = new QSlider(Qt::Horizontal, ui.tableWidget_2);
		sliderTmp->setMinimum(0);
		sliderTmp->setMaximum(150);
		sliderTmp->setValue(nutrientValue[i]);
		sliderTmp->setStyleSheet(
			"QSlider::groove:horizontal { border: 1px solid #999999; height: 15px; background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #B1B1B1, stop:1 #c4c4c4); }"
			"QSlider::handle:horizontal { width: 0px; background: rgba" + nutrient_Color_Str[i] + "; border: 5px solid rgba" + nutrient_Color_Str[i] + "; margin: -5px 0; border-radius: 8px;}"
			"QSlider::handle:vertical { height: 0px; background: rgba" + nutrient_Color_Str[i] + "; border: 5px solid rgba" + nutrient_Color_Str[i] + "; margin: -8px 0; border-radius: 10px;}"
			"QSlider::sub-page:horizontal { background: rgba" + nutrient_Color_Str[i] + "; }"
			);
		ui.tableWidget_2->setCellWidget(i, 2, sliderTmp);
		QSignalMapper* signalMapper = new QSignalMapper(this);
		connect(sliderTmp, SIGNAL(valueChanged(int)), signalMapper, SLOT(map()));
		signalMapper->setMapping(sliderTmp, i);
		connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(changeAmount_Nutrient(int)));
	}
}

/* Change nutrient value in ui.tableWidget_2 */
void QtGuiApplication1::changeAmount_Nutrient(int currentRow) {
	QSignalMapper* signalMapper = qobject_cast<QSignalMapper*>(sender());
	QSlider *sliderTmp = qobject_cast<QSlider*>(signalMapper->mapping(currentRow));
	ui.tableWidget_2->item(currentRow, 1)->setText(QString::fromStdString(to_string(nutrientValue[currentRow]) + "%"));
	nutrientValue[currentRow] = sliderTmp->value();

	if (leafNodeVec.size() > 0) {
		// sort with totalError(originError+hErr)
		for (int i = 0; i < leafNodeVec.size(); i++) {
			double hErr = getHealthError(leafNodeVec[i].nutrientVec);
			leafNodeVec[i].totalError = (leafNodeVec[i].originError*(100 - nutrient_ratio) + hErr*nutrient_ratio)/100;
		}
		updateResultTable(1);
	}
}

/* change ratio */
void QtGuiApplication1::sliderRatioChange(int value) {
	nutrient_ratio = value;
	if (leafNodeVec.size() > 0) {
		// sort with totalError(originError+hErr)
		for (int i = 0; i < leafNodeVec.size(); i++) {
			double hErr = getHealthError(leafNodeVec[i].nutrientVec);
			leafNodeVec[i].totalError = (leafNodeVec[i].originError*(100 - nutrient_ratio) + hErr*nutrient_ratio) / 100;
		}
		updateResultTable(1);
	}
}

/* --------------------------------Button Click Function-------------------------------- */
/* Add new file Function */
void QtGuiApplication1::initClicked() {
	selectContourIdx = -1;
	selectFoodIdx = -1;

	setLabelD(dishImg);
	/*ui.label_show->clear();
	QPixmap tmpp(topLabelSize, topLabelSize);
	tmpp.fill(Qt::transparent);
	ui.label_show->setPixmap(tmpp);*/

	//leafNodeVec.clear();
	ui.progressBar->setValue(1);
	stopOrNot = false;
}

/* Apply Button Function */
void QtGuiApplication1::applyClicked() {
	applyMode = 0;
	if (selectImageOrNot) {
		initClicked();

		if (foodSelectedIndex.Element.size() > 0) {
			recipe rec;
			map<string, vector<food>>::iterator iterSelect;
			map<string, vector<int>>::iterator iterSelectAmount = foodSelectedIndex_Amount.Element.begin();
			for (iterSelect = foodSelectedIndex.Element.begin(); iterSelect != foodSelectedIndex.Element.end(); iterSelect++) {
				rec.addFood(iterSelect->first, iterSelectAmount->second[0], 0, iterSelect->second[0].category);
				iterSelectAmount++;
			}
			rec.print();
			compareWithRecipe(current_LeafNode, rec, 0, 100);
		}
		ui.progressBar->setValue(100);
	}
	cout << "------------------------------------------------- finish" << endl;
}

/* Run All Reipe Button Function */
void QtGuiApplication1::runAllReipeClicked() {
	clock_t t = clock();
	applyMode = 1;
	if (selectImageOrNot) {
		initClicked();

		for (int i = 0; i < recipeFileName.size(); i++) {
			if (stopOrNot) {
				stopOrNot = false;
				break;
			}

			recipe rec;
			string recipeName = recipeFileName[i];
			for (int j = 0; j < cateRecipe[recipeName].size(); j++) {
				rec.addFood(cateRecipe[recipeName][j].name, foodInitialAmount, 0, cateRecipe[recipeName][j].category);
			}
			compareWithRecipe(current_LeafNode, rec, 100 / recipeFileName.size()*(i), 100 / recipeFileName.size()*(i + 1));
			if (i == recipeFileName.size() - 1)
				ui.progressBar->setValue(100);
			else
				ui.progressBar->setValue(100 / recipeFileName.size()*(i + 1));
		}
	}

	ui.progressBar->setValue(100);
	clock_t t1 = clock();
	cout << "total time: " << t1 - t << endl;
	cout << "aver time: " << sumT / cateRecipe.size() << endl;
	cout << "------------------------------------------------- finish" << endl;
}

string ProjectDir = "C:/Users/ec131b/Documents/Visual Studio 2013/Projects/QtGuiApplication2/QtGuiApplication1/";
/* Run All Image Button Function */
void QtGuiApplication1::runAllImageClicked() {
	applyMode = 2;
	clock_t t = clock();
	int imageAmount = 0;

	if (foodSelectedIndex.Element.size() > 0) {
		initClicked();

		recipe rec;
		map<string, vector<food>>::iterator iterSelect;
		map<string, vector<int>>::iterator iterSelectAmount = foodSelectedIndex_Amount.Element.begin();
		for (iterSelect = foodSelectedIndex.Element.begin(); iterSelect != foodSelectedIndex.Element.end(); iterSelect++) {
			rec.addFood(iterSelect->first, iterSelectAmount->second[0], 0, iterSelect->second[0].category);
			iterSelectAmount++;
		}
		rec.print();

		vector<string> inputImageFileName;
		getdir(inputSVGDir, inputImageFileName);

		string Deputy = ".svg";
		for (int i = inputImageFileName.size()-1; i >= 0; i--) {
			std::size_t found = inputImageFileName[i].find(Deputy);
			if (found == std::string::npos)
				inputImageFileName.erase(inputImageFileName.begin() + i);
		}
		imageAmount = inputImageFileName.size();

		for (int i = 0; i < inputImageFileName.size(); i++) {
			bool isStop = false;
			clock_t s1 = clock();
			if (stopOrNot) {
				stopOrNot = false;
				isStop = true;
				break;
			}

			openInputImage(ProjectDir+inputSVGDir + inputImageFileName[i]);
			compareWithRecipe(current_LeafNode, rec, 100 / inputImageFileName.size()*(i), 100 / inputImageFileName.size()*(i + 1));

			clock_t s2 = clock();
			pair<string, int> pp(inputImageFileName[i], s2 - s1);
			imgInfo.push_back(pp);
			sumT += s2 - s1;
			if (i == inputImageFileName.size() - 1)
				ui.progressBar->setValue(100);
			else
				ui.progressBar->setValue(100 / inputImageFileName.size()*(i + 1));

			if (!isStop && !stopOrNot) {
				setLabel_input(current_LeafNode.userDraw);
				cout << "samplepointsOfDrawSize= " << current_LeafNode.samplepointsOfDraw.size() << endl;
			}
			qApp->processEvents();
		}
	}
	ui.progressBar->setValue(100);
	clock_t t1 = clock();
	cout << "aver time: " << sumT / imageAmount << endl;
	cout << "total time: " << t1 - t << endl;
	for (auto it = imgInfo.begin(); it != imgInfo.end(); it++)
		cout << it->first << " " << it->second << endl;

	cout << "------------------------------------------------- finish" << endl;
}

// set path, inputSize, userDraw
void QtGuiApplication1::openInputImage(string filePath) {
	selectImageOrNot = true;
	current_LeafNode.path = filePath;
	current_LeafNode.inputSize = inputSize;
	current_LeafNode.isStoredAsNode = false;
	current_LeafNode.preprocessingDone = false;
	current_LeafNode.preprocessingDone_B = false;

	current_LeafNode.sortedFragList.clear();
	current_LeafNode.resultStack = cv::Mat(inputSize, CV_8UC4);
	current_LeafNode.nutrientVec.clear();

	// open image, and set as current_LeafNode.userDraw
	svgParse svgFile(filePath, inputSize);
	current_LeafNode.scaleRatio = 1;
	current_LeafNode.inputSize = inputSize;
	current_LeafNode.userDraw = svgFile._userDraw.clone();

	current_LeafNode.boundaryOfContour = svgFile._boundaryOfContour;
	current_LeafNode.centerOfContour = svgFile._centerOfContour;
	current_LeafNode.colorOfContour = svgFile._colorOfContour;
	current_LeafNode.contourPoint = svgFile._contourPoint;

	current_LeafNode.contourAmount = svgFile._contourAmount;
	current_LeafNode.globalCenter = svgFile._globalCenter;

	// prePocessing of UserDraw and show in ui.label_input
	prePocessOfUserDraw_A(current_LeafNode, 1);
	setLabel_input(current_LeafNode.userDraw);
	// classify each pixel to which contour
	label2_feature(current_LeafNode);
	//test_contourMat(current_LeafNode);
	
	// setScale
	scaleBtn_show();
	setLabel_show(false, current_LeafNode.userDraw, current_LeafNode.userDraw, current_LeafNode.globalCenter);

	current_ptr = &current_LeafNode;
	if (current_ptr->preprocessingDone_B)
		cout << "preprocessingDone_B" << endl;
	else
		cout << "preprocessingDone_B not" << endl;
	//for (int i = 0; i < current_LeafNode.contourPoint.size(); i++)
		//compareWithAllFood(current_LeafNode, i);

}

void QtGuiApplication1::openImageClicked() {
	QFileDialog *fileDialog = new QFileDialog(this);
	fileDialog->setWindowTitle(tr("Open Image"));
	fileDialog->setDirectory(".");

	if (fileDialog->exec() == QDialog::Accepted) {
		QString path = fileDialog->selectedFiles()[0];
		string tmpStr = split_str(path.toStdString(), '/').back();
		vector<string> tmpStrVec2 = split_str(tmpStr, '.');

		if (tmpStrVec2[1] == "svg") {
			openInputImage(path.toStdString());
		}
	}
	else {
		ui.label_4->setText("You didn't select correct image file format!");
	}
}

void QtGuiApplication1::stopCompare() {
	stopOrNot = true;
}

void QtGuiApplication1::scaleBtn_show() {
	globalScaleSize = 1;
	ui.scaleBtn_0->show();
	ui.scaleBtn_1->show();
	ui.scaleBtn_2->show();
}

void QtGuiApplication1::scaleBtn0_Clicked() {
	if (selectImageOrNot) {
		globalScaleSize = 0.5;
		ui.scaleBtn_0->show();
		ui.scaleBtn_1->hide();
		ui.scaleBtn_2->hide();
	}
}

void QtGuiApplication1::scaleBtn1_Clicked() {
	if (selectImageOrNot) {
		globalScaleSize = 1;
		ui.scaleBtn_0->hide();
		ui.scaleBtn_1->show();
		ui.scaleBtn_2->hide();
	}
}

void QtGuiApplication1::scaleBtn2_Clicked() {
	if (selectImageOrNot) {
		globalScaleSize = 2;
		ui.scaleBtn_0->hide();
		ui.scaleBtn_1->hide();
		ui.scaleBtn_2->show();
	}
}

// normalize
Point2f normalizeVec(Point2f p) {
	Point2f resultP;
	double lengthN = sqrt(pow(p.x, 2) + pow(p.y, 2));
	resultP.x = (double)p.x / lengthN * 10;
	resultP.y = (double)p.y / lengthN * 10;
	return resultP;
}

struct TF {
	double rotation;
	double displasement;
	Point2f direcVec;
	Point2f linearDist;
	Point2f center;
	int groupflag;
	Mat testMM;
	vector<double> testVec;
}; 

struct GroupCenter {
	double rotation;
	Point2f direcVec;
	Point2f linearDist;
	Point2f center;
	vector<int> group;
	vector<double> testVec;
};

bool hasVisitAll(vector<TF> &TFSpace, int &id) {
	for (int i = 0; i < TFSpace.size(); i++) {
		if (TFSpace[i].groupflag < 0) {
			id = i;
			return false;
		}
	}
	id = -1;
	return true;
}

int newpoint(vector<bool> vecbool) {
	int id = 0;
	for (int i = 0; i<vecbool.size(); i++) {
		if (!vecbool[i]) {
			id = i;
			break;
		}
	}
	return id;
}

float caldistance(vector<float> center, vector<float> pt) {
	float distance = 0.0;
	for (int i = 0; i<center.size(); i++)
		distance += pow(center[i] - pt[i], 2);
	distance = sqrt(distance);
	return distance;
}

void polar(double x, double y, double& r, double& theta) {
	r = sqrt((pow(x, 2)) + (pow(y, 2)));
	theta = atan(y / x);
	theta = (theta * 180) / 3.141592654;
}

double getTFDist(TF &tf1, TF &tf2) {
	double tmpListDist = pow(tf1.linearDist.x - tf2.linearDist.x, 2) + pow(tf1.linearDist.y - tf2.linearDist.y, 2);
	double tmpListDist2 = pow(tf1.direcVec.x - tf2.direcVec.x, 2) + pow(tf1.direcVec.y - tf2.direcVec.y, 2);
	double dist = sqrt(tmpListDist2 + tmpListDist);
	return dist;
}

void getGroupOfSymmmetry(vector<Point> &c_samPoint, Point2f &centerOfContour, Mat &testM,  double diameter) {
	// find curvature
	/*Point posOld, posOlder;
	Point f1stDerivative, f2ndDerivative;
	vector<float> vecCurvature(c_samPoint.size());
	for (int j = 0; j < vecCurvature.size(); j++) {
	Point pos = c_samPoint[j];
	if (j == 0){ posOld = posOlder = pos; }
	f1stDerivative.x = pos.x - posOld.x;
	f1stDerivative.y = pos.y - posOld.y;
	f2ndDerivative.x = -pos.x + 2.0f * posOld.x - posOlder.x;
	f2ndDerivative.y = -pos.y + 2.0f * posOld.y - posOlder.y;
	float curvature2D = 0.0f;
	if (std::abs(f2ndDerivative.x) > 10e-4 && std::abs(f2ndDerivative.y) > 10e-4) {
	curvature2D = sqrt(std::abs(
	pow(f2ndDerivative.y*f1stDerivative.x - f2ndDerivative.x*f1stDerivative.y, 2.0f) /
	pow(f2ndDerivative.x + f2ndDerivative.y, 3.0)));
	}
	vecCurvature[j] = curvature2D;
	posOlder = posOld;
	posOld = pos;
	}*/

	// find normal
	vector<Point2f> normVec;
	for (int j = 0; j < c_samPoint.size(); j++) {
		Point pre = c_samPoint[(j - 1 + c_samPoint.size()) % c_samPoint.size()];
		Point tgt = c_samPoint[j];
		Point nxt = c_samPoint[(j + 1) % c_samPoint.size()];
		vector<double> nor1;
		vector<double> nor2;
		nor1.push_back(pre.y - tgt.y);
		nor1.push_back(tgt.x - pre.x);
		nor2.push_back(tgt.y - nxt.y);
		nor2.push_back(nxt.x - tgt.x);
		Point c_normal(nor1[0] + nor2[0], nor1[1] + nor2[1]);
		Point2f c_normal_nmliz = normalizeVec(c_normal);
		normVec.push_back(c_normal_nmliz);
		line(testM, c_samPoint[j], (c_samPoint[j] + Point(c_normal_nmliz)), Scalar(0, 0, 255, 255), 1, 8);
		double lengthN2 = sqrt(pow(c_normal_nmliz.x, 2) + pow(c_normal_nmliz.y, 2));
		//cout << lengthN2 << ", ";
	}

	vector<TF> TFSpace;
	for (int j0 = 1; j0 < c_samPoint.size(); j0++) {
		Point p0 = c_samPoint[j0];
		Point np0 = normVec[j0];
		for (int j1 = j0 + 1; j1 < c_samPoint.size(); j1++) {
			Point p1 = c_samPoint[j1];
			Point np1 = normVec[j1];

			Point2f subVec = p0 - p1; // substraction vector = (p0-p1) 
			Point2f centerP((p0.x + p1.x) / 2, (p0.y + p1.y) / 2); // vertical vector = (-1)/subVec
			Point2f centerVec(subVec.y, -subVec.x); // vertical vector = (-1)/subVec
			centerVec = normalizeVec(centerVec);
			Point2f addNormVec = np0 + np1; // substraction norm vector = (p0-p1)
			addNormVec = normalizeVec(addNormVec);

			double InnerPdt = (centerVec.x*addNormVec.x + centerVec.y*addNormVec.y);
			double costheta = InnerPdt / (sqrt(pow(centerVec.x, 2) + pow(centerVec.y, 2))*sqrt(pow(addNormVec.x, 2) + pow(addNormVec.y, 2)));
			if (costheta > 0.98) {
				Mat testMM = testM.clone();
				line(testMM, p0, p1, Scalar(255, 255, 0, 255), 2, 8);
				line(testMM, centerP, centerP + centerVec * 100, Scalar(255, 0, 255, 255), 1, 8);
				line(testMM, centerP, centerP - centerVec * 100, Scalar(255, 0, 255, 255), 1, 8);
				line(testMM, centerP, centerP + addNormVec * 10, Scalar(0, 255, 255, 255), 1, 8);

				TF tfTmp;
				// set rotation value - angle with pi
				double aa = (np0.x*np1.x + np0.y*np1.y);
				double bb = sqrt((pow(np0.x, 2) + pow(np0.y, 2))*(pow(np1.x, 2) + pow(np1.y, 2)));
				double c_theta = acos(aa / bb);

				// set translate
				double displace = sqrt(pow(subVec.x, 2) + pow(subVec.y, 2));

				// set linearDist from origin inputSize/2, inputSize/2
				//circle(testM, centerOfContour, 1, Scalar(255, 255, 255, 255), 2);
				double Fa = subVec.y, Fb = subVec.x;
				double Fc = 0;
				double Fd = Fb*centerP.x - Fa*centerP.y;
				double Rx = (Fa*Fc + Fb*Fd) / (pow(Fa, 2) + pow(Fb, 2));
				double Ry = (Fb*Fc - Fa*Fd) / (pow(Fa, 2) + pow(Fb, 2));
				/*double Fa = subVec.y, Fb = -subVec.x, Fe = centerP.x*Fa + centerP.y*Fb;
				double Fc = subVec.x, Fd = subVec.y, Ff = centerP.x*Fc + centerP.y*Fd;
				double Rx = 0, Ry = 0;
				double determinant = Fa*Fd - Fb*Fc;
				if (determinant != 0) {
				Rx = (Fe*Fd - Fb*Ff) / determinant;
				Ry = (Fa*Ff - Fe*Fc) / determinant;
				}
				else {
				printf("Cramer equations system: determinant is zero\n"
				"there are either no solutions or many solutions exist.\n");
				}*/

				tfTmp.center = centerP;
				tfTmp.rotation = c_theta;
				tfTmp.direcVec = centerVec;
				tfTmp.displasement = sqrt((pow(centerP.x, 2))*(pow(centerP.y, 2))) / diameter;
				tfTmp.linearDist = Point2f(Rx, Ry); // (x, y)
				vector<double> FFVec = { Fb, -Fa, Fd };
				tfTmp.testVec = FFVec;
				//tfTmp.linearDist -= centerOfContour;
				tfTmp.groupflag = -1;
				tfTmp.testMM = testMM.clone();
				TFSpace.push_back(tfTmp);
				//imwrite("Result/" + to_string(j1) + "_" + to_string(costheta) + "_" + to_string(tfTmp.displasement) + ".png", testMM);
			}
		}
	}
	//imshow(to_string(i) + ".png", testM);

	// count wach pair
	//vector<vector<float> > TFDist(TFSpace.size(), vector<float>(TFSpace.size(), 0)); // have visit or not
	//for (int m = 0; m < TFSpace.size(); m++) {
	//	TFDist[m][m] = 0;
	//	for (int n = m + 1; n < TFSpace.size(); n++) {
	//		TFDist[m][n] = abs(TFSpace[m].rotation - TFSpace[n].rotation) + abs(TFSpace[m].displasement - TFSpace[n].displasement);
	//		TFDist[n][m] = TFDist[m][n];
	//		//cout << "m= " << m << ", n=" << n << ", = " << TFDist[n][m] << endl;
	//	}
	//}

	// mean shift
	int id = 0;
	vector<GroupCenter> groupNcenter;
	while (!hasVisitAll(TFSpace, id)) {
		if (id < 0) break;
		// set new groupflag
		int c_groupflag = groupNcenter.size();
		TFSpace[id].groupflag = c_groupflag;
		// set TF to presenet current center point info
		TF center_TF = TFSpace[id];
		// push element if should in this group
		vector<int> c_Group;
		c_Group.push_back(id);

		double radius = 10;
		double stopthresh = 10;
		while (true) {
			// if (distance between id-n) < radius, then they should in the same group
			for (int n = 0; n < TFSpace.size(); n++) {
				if (TFSpace[n].groupflag < 0) {
					///////double dist = sqrt(pow(center_TF.center.x - TFSpace[n].center.x, 2) + pow(center_TF.center.y - TFSpace[n].center.y, 2));
					if (getTFDist(center_TF, TFSpace[n]) < radius) {
						TFSpace[n].groupflag = c_groupflag;
						c_Group.push_back(n);
					}
				}
			}

			// reset center point every while
			TF lastCenter = center_TF;
			center_TF.rotation = 0;
			center_TF.center = Point2f(0, 0);
			center_TF.direcVec = Point2f(0, 0);
			center_TF.linearDist = Point2f(0, 0);
			for (int k = 0; k < c_Group.size(); k++) { // 二維資訊要保留 不能單純的distance
				center_TF.rotation += TFSpace[c_Group[k]].rotation;
				center_TF.center += TFSpace[c_Group[k]].center;
				center_TF.direcVec += TFSpace[c_Group[k]].direcVec;
				center_TF.linearDist += TFSpace[c_Group[k]].linearDist;
				///////center_TF.center.x += TFSpace[c_Group[k]].center.x;
				///////center_TF.center.y += TFSpace[c_Group[k]].center.y;
			}
			center_TF.rotation /= c_Group.size();
			center_TF.center.x /= c_Group.size();
			center_TF.center.y /= c_Group.size();
			center_TF.direcVec.x /= c_Group.size();
			center_TF.direcVec.y /= c_Group.size();
			center_TF.linearDist.x /= c_Group.size();
			center_TF.linearDist.y /= c_Group.size();
			///////center_TF.center.x /= c_Group.size();
			///////center_TF.center.y /= c_Group.size();

			// if new (lastCenter-newCenter) < stopthresh, break while -- stop update the center position 
			if (stopthresh > getTFDist(lastCenter, center_TF)) {
				///////if (stopthresh > sqrt(pow(lastCenter.x - center_TF.center.x, 2) + pow(lastCenter.y - center_TF.center.y, 2))) {
				break;
			}
		}

		int merge = -1;
		for (int k = 0; k < groupNcenter.size(); k++) {
			double distt = pow(center_TF.linearDist.x - groupNcenter[k].linearDist.x, 2) + pow(center_TF.linearDist.y - groupNcenter[k].linearDist.y, 2);
			double distt2 = pow(center_TF.direcVec.x - groupNcenter[k].direcVec.x, 2) + pow(center_TF.direcVec.y - groupNcenter[k].direcVec.y, 2);
			if (radius / 2 > sqrt(distt2 + distt)) {
				merge = k;
				break;
			}
		}
		if (merge >= 0) { //合併
			for (int k = 0; k < c_Group.size(); k++)
				TFSpace[c_Group[k]].groupflag = merge;
			groupNcenter[merge].group.assign(c_Group.begin(), c_Group.end());

			double g_rotation = 0; 
			Point2f g_center = Point2f(0, 0);
			Point2f g_direcVec = Point2f(0, 0);
			Point2f g_linearDist = Point2f(0, 0);
			//////////Point2f g_center = Point2f(0, 0);
			vector<int> *g_group = &groupNcenter[merge].group;
			for (int k = 0; k < g_group->size(); k++) { // 二維資訊要保留 不能單純的distance
				g_rotation += TFSpace[g_group->at(k)].rotation;
				g_center += TFSpace[g_group->at(k)].center;
				g_direcVec += TFSpace[g_group->at(k)].direcVec;
				g_linearDist += TFSpace[g_group->at(k)].linearDist;
				//////////g_center.y += TFSpace[g_group->at(k)].center.y;
				//////////g_center.y += TFSpace[g_group->at(k)].center.y;
			}
			g_rotation /= g_group->size();
			g_center.x /= g_group->size();
			g_center.y /= g_group->size();
			g_direcVec.x /= g_group->size();
			g_direcVec.y /= g_group->size();
			g_linearDist.x /= g_group->size();
			g_linearDist.y /= g_group->size();
			//////////g_center.x /= g_group->size();
			//////////g_center.y /= g_group->size();
			//////////groupNcenter[merge].center = g_center;
		}
		else { //不合併
			GroupCenter tmpGC;
			tmpGC.group = c_Group;
			tmpGC.rotation = center_TF.rotation;
			tmpGC.center = center_TF.center;
			tmpGC.direcVec = center_TF.direcVec;
			tmpGC.linearDist = center_TF.linearDist;
			groupNcenter.push_back(tmpGC);
		}
	}

	int maxI = 0;
	int thresholdSize = (c_samPoint.size()*(c_samPoint.size() - 1) / 2) / groupNcenter.size();
	cout << "c_samPoint.size()= " << c_samPoint.size() << ",  groupNcenter.size()= " << groupNcenter.size() << endl;
	for (int m = 0; m < groupNcenter.size(); m++) {
		if (groupNcenter[m].group.size()>groupNcenter[maxI].group.size())
			maxI = m;
		cout << "group[" << m << "]= " << groupNcenter[m].group.size() << endl;;
		//cout << groupNcenter[m].center << endl;

		if (groupNcenter[m].group.size() > thresholdSize) {
			Mat testMMMM = testM.clone();
			line(testMMMM, groupNcenter[m].center, groupNcenter[m].center + groupNcenter[m].direcVec * 100, Scalar(255, 0, 255, 255), 2, 8);
			line(testMMMM, groupNcenter[m].center, groupNcenter[m].center - groupNcenter[m].direcVec * 100, Scalar(255, 0, 255, 255), 2, 8);
			imwrite("Result/" + to_string(m) + "_" + to_string(groupNcenter[m].group.size()) + ".png", testMMMM);
		}
	}
	
	int m = maxI;

	/*for (int n = 0; n < groupNcenter[m].group.size(); n++) {
		int aa = groupNcenter[m].rotation;
		int bb = groupNcenter[m].linearDist.x;
		int cc = groupNcenter[m].linearDist.y;
		imwrite("Result/" + to_string(m) + "_" + to_string(n) + "_" + to_string(aa) + "_" + to_string(bb) + "_" + to_string(cc) + ".png", TFSpace[groupNcenter[m].group[n]].testMM);
	}*/


	

}

void testSymmetry() {
	svgParse svgFile("svgImg/testSym2.svg", Size(500, 500));
	leafNode testNode;
	testNode.scaleRatio = 1;
	testNode.inputSize = inputSize;
	testNode.userDraw = svgFile._userDraw.clone();
	testNode.contourPoint = svgFile._contourPoint;
	testNode.colorOfContour = svgFile._colorOfContour;
	testNode.centerOfContour = svgFile._centerOfContour;
	testNode.boundaryOfContour = svgFile._boundaryOfContour;
	testNode.contourAmount = svgFile._contourAmount;
	prePocessOfUserDraw_A(testNode, 1);
	
	for (int i = 0; i < 1; i++) {
		Mat testM = cv::Mat(Size(500, 500), CV_8UC4);
		drawContours(testM, testNode.samplepointsOfDrawOri, i, Scalar(255, 255, 255, 255), 1, 8);
		circle(testM, testNode.samplepointsOfDrawOri[i][0], 2, Scalar(0, 0, 255, 255), 2);
		circle(testM, testNode.samplepointsOfDrawOri[i][5], 1, Scalar(0, 255, 0, 255), 2);

		vector<Point> c_samPoint = testNode.samplepointsOfDrawOri[i];
		Point2f centerOfContour(testNode.centerOfContour[i]);
		double diameter = (testNode.boundaryOfContour[i].width + testNode.boundaryOfContour[i].height) / 2;

		int  timess = 0;
		while (timess++ < 3){}

		getGroupOfSymmmetry(c_samPoint, centerOfContour, testM, diameter);


		
		
	}
}

/* --------------------------------UI Main Function-------------------------------- */
QtGuiApplication1::QtGuiApplication1(QWidget *parent):QMainWindow(parent) {
	std::ios_base::sync_with_stdio(false);
	ui.setupUi(this);
	
	this->showFullScreen();
	std::ios_base::sync_with_stdio(false);

	ui.horizontalLayout_3->setStretch(0, 4);
	ui.horizontalLayout_3->setStretch(1, 6);

	ui.horizontalLayout_2->setStretch(0, 4);
	ui.horizontalLayout_2->setStretch(1, 4);

	ui.horizontalLayout_4->setStretch(0, 6);
	ui.horizontalLayout_4->setStretch(1, 4);

	ui.horizontalLayout_5->setStretch(0, 3);
	ui.horizontalLayout_5->setStretch(1, 3);
	ui.horizontalLayout_5->setStretch(2, 2);
	ui.horizontalLayout_5->setStretch(3, 2);

	ui.verticalLayout_3->setStretch(0, 5);
	ui.verticalLayout_3->setStretch(1, 0.5);
	ui.verticalLayout_3->setStretch(2, 0.5);
	ui.verticalLayout_3->setStretch(3, 0.5);
	ui.verticalLayout_3->setStretch(4, 3);
	ui.verticalLayout_3->setStretch(5, 1);

	ui.verticalLayout->setStretch(0, 4);
	ui.verticalLayout->setStretch(1, 1);
	ui.verticalLayout->setStretch(2, 3);
	ui.verticalLayout->setStretch(3, 1);
	
	ui.horizontalLayout_7->setStretch(0, 4);
	ui.horizontalLayout_7->setStretch(1, 2);
	ui.horizontalLayout_7->setStretch(2, 2);
	ui.horizontalLayout_7->setStretch(3, 2);

	//testSymmetry();
	//test_icp();

	/* -----------------PreProcessing----------------- */
	// pre-process for contour descriptor and sample points
	preProcess("foodDesSize.bin", samplepointsOfFood, "foodSample.bin");
	//preProcess("foodDesSize.bin", samplepointsOfFood, "foodSample.bin", desOfFood, "foodDesSeq.bin", "foodDesSeq2.bin", "foodDesSeq3.bin", "foodDesSeq4.bin");
	//preProcess("foodDesSize_reverse.bin", samplepointsOfFoodReverse, "foodSample_reverse.bin", desOfFoodReverse, "foodDesSeq_reverse.bin", "foodDesSeq_reverse2.bin", "foodDesSeq_reverse3.bin", "foodDesSeq_reverse4.bin");
	readFoodPreData(totalFood);
	for (int i = 0; i < totalFood.size(); i++)  {
		vector<Point> food_pointSeq = samplepointsOfFood[totalFood[i].orderIdx];
		getOverlapRatio(totalFood[i], food_pointSeq);
	}

	// get whole contour
	for (int i = 0; i < samplepointsOfFood.size(); i++) {
		Mat FoodAlpha = Mat::zeros(inputSize, CV_8UC1);
		drawContours(FoodAlpha, samplepointsOfFood, i, Scalar(255), CV_FILLED);
		PointsOfFoodOri.push_back(getAllContourPoint(FoodAlpha));

		Point center = getCenterPoint(PointsOfFoodOri[i]);
		vector<Point> reverseP = PointsOfFoodOri[i];
		reversePoint(center, reverseP);
		PointsOfFoodReverseOri.push_back(reverseP);
	}

	vector<Mat> tmpFoodImg;
	readBinaryImg(tmpFoodImg, foodPreDir + "preFoodImgData1.bin");
	readBinaryImg(tmpFoodImg, foodPreDir + "preFoodImgData2.bin");
	readBinaryImg(tmpFoodImg, foodPreDir + "preFoodImgData3.bin");
	readBinaryImg(tmpFoodImg, foodPreDir + "preFoodImgData4.bin");
	readBinaryImg(tmpFoodImg, foodPreDir + "preFoodImgData5.bin");

	/*for (int i = 0; i < sampleporecipeBinintsOfFood.size(); i++) {
		Mat tmpD = Mat::zeros(Size(500, 500), CV_8UC3);
		drawContours(tmpD, samplepointsOfFood, i, Scalar(255, 255, 255), 1, 8);
		imwrite("preFood/food_" + to_string(i) + "_1.png", tmpD);
	}

	for (int i = 0; i < tmpFoodImg.size(); i++)
		imwrite("preFood/food_" + to_string(i) + "_0.png", tmpFoodImg[i]);*/

	for (int i = 0; i < totalFood.size(); i++) {
		totalFoodImg[totalFood[i].fileName] = tmpFoodImg[i];
		totalFood[i].color = getFoodColor(tmpFoodImg[i]);
		//imwrite("Result/" + to_string(i) + ".png", tmpFoodImg[i]);
	}
	//cout << "image PreData" << endl;


	QPalette palette;
	palette.setBrush(QPalette::Background, QColor(234, 252, 255, 200));
	this->setPalette(palette);

	/* get recipe by dir */
	vector<recipe> RecipeFiles;
	getdir(recipeDir, recipeFileName);

	getAllRecipe(recipeDir, recipeFileName, RecipeFiles);
	removeFileExtension(recipeFileName); //remove ".bin"

	/*food relate*/
	for (int i = 0; i < RecipeFiles.size(); i++) {
		for (int v1 = 0; v1 < RecipeFiles[i].foodName().size() - 1; v1++) {
			for (int v2 = v1 + 1; v2 < RecipeFiles[i].foodName().size(); v2++) {
				foodRelate[RecipeFiles[i].foodName()[v1]][RecipeFiles[i].foodName()[v2]]++;
				foodRelate[RecipeFiles[i].foodName()[v2]][RecipeFiles[i].foodName()[v1]]++;
			}
		}
	}
	/* -----------------UI----------------- */
	ui.progressBar->setAlignment(Qt::AlignHCenter);

	/* recipe treeWidget - left */
	initTree(ui.treeWidget);
	QList<QTreeWidgetItem *> rootList;
	for (int i = 0; i < RecipeFiles.size(); i++) {
		QTreeWidgetItem *root = new QTreeWidgetItem(ui.treeWidget, QStringList() << codec->toUnicode(recipeFileName[i].c_str()));
		//root->setIcon(0, QIcon(QString::fromStdString("inputImg/bear.png"))); // set icon
		root->setSizeHint(0, QSize(50, 80));
		root->setExpanded(true);

		for (int j = 0; j < RecipeFiles[i].size; j++) {
			string recFoodName = RecipeFiles[i].foodName()[j];
			vector<food> tmpFoodVec = getFoodVecFromStr(recFoodName);
			if (tmpFoodVec.size() > 0) {
				food tmpF = tmpFoodVec[0];

				cateRecipe[recipeFileName[i]].push_back(tmpF);
				QTreeWidgetItem *leaf = new QTreeWidgetItem(root, QStringList() << codec->toUnicode(tmpF.name.c_str()));
				Mat modifyFoodImage = scaleIconImg(totalFoodImg[tmpF.fileName]);
				QPixmap pixelImage = QPixmap::fromImage(QImage((uchar*)modifyFoodImage.data, modifyFoodImage.cols, modifyFoodImage.rows, QImage::Format_ARGB32));
				QPixmap resPix = pixelImage.scaled(80, 80);
				leaf->setIcon(0, QIcon(resPix)); // set icon

				QString html = getToolTipQString(tmpF);
				leaf->setToolTip(0, html);
			}
		}
		rootList << root;
	}
	ui.treeWidget->insertTopLevelItems(0, rootList);  //設定此容器內容為QTreeWidget的根節點
	connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(QTreeWidgetClicked_Recipe(QTreeWidgetItem*, int)));

	/* food treeWidget - left, get food by dir */
	initTree(ui.treeWidget_2);
	QList<QTreeWidgetItem *> rootList2;
	// get category name automatically
	for (int i = 0; i < totalFood.size(); i++) {
		cateFood.Element[totalFood[i].category].Element[totalFood[i].name].push_back(totalFood[i]);
		bool categoryIsExit = false;
		for (int j = 0; j < categoryStr.size(); j++) {
			if (!totalFood[i].category.compare(categoryStr[j])) {
				categoryIsExit = true;
				break;
			}
		}
		if (!categoryIsExit)
			categoryStr.push_back(totalFood[i].category);
	}

	map<string, vector<food> >::iterator iterFood;
	for (int i = 0; i < categoryStr.size(); i++) {
		QTreeWidgetItem *root = new QTreeWidgetItem(ui.treeWidget_2, QStringList() << codec->toUnicode(categoryStr[i].c_str()));
		root->setIcon(0, QIcon(QString::fromStdString(categoryImgDir + categoryStr[i].c_str() + ".png"))); // set icon
		root->setSizeHint(0, QSize(50, 80));
		root->setExpanded(true);

		for (iterFood = cateFood.Element[categoryStr[i]].Element.begin(); iterFood != cateFood.Element[categoryStr[i]].Element.end(); iterFood++) {
			QTreeWidgetItem *leaf = new QTreeWidgetItem(root, QStringList() << codec->toUnicode(iterFood->second[0].name.c_str()));
			Mat modifyFoodImage = scaleIconImg(totalFoodImg[iterFood->second[0].fileName]);
			QPixmap pixelImage = QPixmap::fromImage(QImage((uchar*)modifyFoodImage.data, modifyFoodImage.cols, modifyFoodImage.rows, QImage::Format_ARGB32));
			QPixmap resPix = pixelImage.scaled(80, 80);
			leaf->setIcon(0, QIcon(resPix)); // set icon

			QString html = getToolTipQString(iterFood->second[0]);
			leaf->setToolTip(0, html);
		}
		rootList2 << root;
	}
	ui.treeWidget_2->insertTopLevelItems(0, rootList2);  //設定此容器內容為QTreeWidget的根節點
	connect(ui.treeWidget_2, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(QTreeWidgetClicked_Food(QTreeWidgetItem*, int)));

	/* init origin and result label */
	initLabel(ui.label_show);
	initLabel(ui.label_input);
	if (ui.label_input->width() > ui.label_input->height())	{ topLabelSize = ui.label_input->width(); }
	else { topLabelSize = ui.label_input->height(); }
	QPixmap tmpp(topLabelSize, topLabelSize);
	tmpp.fill(Qt::transparent);
	ui.label_show->setPixmap(tmpp);
	ui.label_input->setPixmap(tmpp);
	cv::resize(dishImg, dishImg, inputSize);
	setLabelD(dishImg);

	/* init the show table */
	initTab(ui.tabWidget);
	initTab(ui.tabWidget_2);
	initTab(ui.tabWidget_3);

	///*
	QPushButton* addButton = new QPushButton(ui.tabWidget_2);
	addButton->setText("+");
	addButton->setMaximumWidth(25);
	addButton->setMaximumHeight(25);
	addButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt; font: large 'Times New Roman'; background-color: rgba(255, 255, 255, 220); } QPushButton:hover { background-color: rgba(255, 250, 188); }" );
	ui.tabWidget_2->tabBar()->setTabButton(0, QTabBar::LeftSide, addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(openImageClicked()));

	/* init table style */
	initTable(ui.tableWidget_selectedFood);
	initTable(ui.tableWidget_6);
	initTable(ui.tableWidget_2);
	initTable(ui.tableWidget_3);

	///* click current food table event */
	ui.tableWidget_selectedFood->setStyleSheet("QTableWidget {border: none; background-color: rgba(" + white_Color_Str + ");}"
		"QTableWidget::item {background-color: rgba(" + select_Color_Str + "); padding: 5px;}"
		"QHeaderView::section {background-color: transparent;}"
		"QHeaderView {background-color: transparent;}"
		"QTableCornerButton::section {background-color: transparent;} ");
	ui.tableWidget_selectedFood->setColumnCount(4);
	ui.tableWidget_selectedFood->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget_selectedFood->setColumnWidth(0, 35);
	ui.tableWidget_selectedFood->setColumnWidth(1, 80);
	ui.tableWidget_selectedFood->setColumnWidth(2, 80);
	ui.tableWidget_selectedFood->verticalHeader()->setDefaultSectionSize(80);
	connect(ui.tableWidget_selectedFood, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(removeFoodItem(QTableWidgetItem*)));

	///* init result table and click index event */
	ui.tableWidget_6->setStyleSheet( "QTableWidget {border: none; background-color: rgba(" + white_Color_Str + ");}"
		"QHeaderView::section {background-color: transparent;}"
		"QHeaderView {background-color: transparent;}"
		"QTableCornerButton::section {background-color: transparent;} ");
	ui.tableWidget_6->setRowCount(2);
	ui.tableWidget_6->setRowHeight(1, 30);
	ui.tableWidget_6->verticalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

	///* init nutrient table */
	initNutrientTable(ui.tableWidget_2);
	for (int i = 0; i < nutrient_Str.size(); i++) {
		QSlider *sliderTmp = new QSlider(Qt::Horizontal, ui.tableWidget_2);
		sliderTmp->setMinimum(0);
		sliderTmp->setMaximum(150);
		sliderTmp->setValue(nutrientValue[i]);
		sliderTmp->setStyleSheet(
			"QSlider::groove:horizontal { border: 1px solid #999999; height: 15px; background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #B1B1B1, stop:1 #c4c4c4); }"
			"QSlider::handle:horizontal { width: 0px; background: rgba" + nutrient_Color_Str[i] + "; border: 5px solid rgba" + nutrient_Color_Str[i] + "; margin: -5px 0; border-radius: 8px;}"
			"QSlider::handle:vertical { height: 0px; background: rgba" + nutrient_Color_Str[i] + "; border: 5px solid rgba" + nutrient_Color_Str[i] + "; margin: -8px 0; border-radius: 10px;}"
			"QSlider::sub-page:horizontal { background: rgba" + nutrient_Color_Str[i] + "; }"
			);
		ui.tableWidget_2->setCellWidget(i, 2, sliderTmp);
		QSignalMapper* signalMapper = new QSignalMapper(this);
		connect(sliderTmp, SIGNAL(valueChanged(int)), signalMapper, SLOT(map()));
		signalMapper->setMapping(sliderTmp, i);
		connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(changeAmount_Nutrient(int)));
	}

	/* init suggest table */
	ui.tableWidget_3->setStyleSheet("QTableWidget {border: none; background-color: rgba(" + white_Color_Str + ");}"
		"QTableWidget::item {background-color: rgba(" + select_Color_Str + "); padding: 5px;}"
		"QHeaderView::section {background-color: transparent;}"
		"QHeaderView {background-color: transparent;}"
		"QTableCornerButton::section {background-color: transparent;} ");
	ui.tableWidget_3->setColumnCount(3);
	ui.tableWidget_3->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget_3->setColumnWidth(0, 90);
	ui.tableWidget_3->verticalHeader()->setDefaultSectionSize(90);
	connect(ui.tableWidget_3, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(suggestFoodItem(QTableWidgetItem*)));

	/* set slider */
	QSlider *ratio_slider = new QSlider(Qt::Horizontal, this);;
	ratio_slider->setSingleStep(1);
	ratio_slider->setMinimum(0);
	ratio_slider->setMaximum(100);
	ratio_slider->setValue(nutrient_ratio);
	ratio_slider->setStyleSheet(
		"QSlider { border: 10px;  }"
		"QSlider::groove:horizontal { border: 1px solid #999999; height: 15px; margin: 3px; background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgba(177, 177, 177, 5), stop:1 rgba(91, 89, 89, 5)); }"
		"QSlider::handle:horizontal{ width: 6px; background: rgba(105, 194, 239, 250); border: 2px solid rgba(0, 67, 112, 150); margin: -4px -4px;}"
		"QSlider::sub-page:horizontal{ background: rgba(105, 194, 239, 180); }"
		);
	connect(ratio_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderRatioChange(int)));
	QFont fnt;
	fnt.setPointSize(15);
	fnt.setFamily("Times New Roman");
	//fnt.setFamily("DFKai-sb");
	QLabel *label1 = new QLabel(codec->toUnicode(" "), this);
	QLabel *label2 = new QLabel(codec->toUnicode("Nutrient "), this);
	//QLabel *label2 = new QLabel(codec->toUnicode("營養 "), this);
	label1->setFont(fnt);
	label2->setFont(fnt);
	ui.gridLayout_5->addWidget(label1, 0, 0, 1, 1, Qt::AlignLeft);
	ui.gridLayout_5->addWidget(label2, 0, 2, 1, 1, Qt::AlignRight);
	ui.gridLayout_5->addWidget(ratio_slider, 0, 0, 1, 3);

	/* set button
	ui.ApplyButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt; font: large 'Times New Roman'; background-color: rgba(170, 240, 255); } QPushButton:hover { background-color: rgba(80, 220, 255); }");
	ui.ExitButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt; font: large 'Times New Roman'; background-color: rgba( 255, 170, 170); } QPushButton:hover { background-color: rgba(255, 119, 119); }");
	ui.RunRecipeButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt; font: large 'Times New Roman'; background-color: rgba(170, 240, 255); } QPushButton:hover { background-color: rgba(80, 220, 255); }");
	ui.RunImageButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt; font: large 'Times New Roman'; background-color: rgba(170, 240, 255); } QPushButton:hover { background-color: rgba(80, 220, 255); }");
	ui.stopButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt; font: large 'Times New Roman'; background-color: rgba(255, 223, 170); } QPushButton:hover { background-color: rgba(252, 206, 116); }");
	ui.RemoveAllButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt;" );
	*/

	ui.ApplyButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt; background-color: rgba(170, 240, 255); } QPushButton:hover { background-color: rgba(80, 220, 255); }");
	ui.ExitButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt; background-color: rgba( 255, 170, 170); } QPushButton:hover { background-color: rgba(255, 119, 119); }");
	ui.RunRecipeButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt; background-color: rgba(170, 240, 255); } QPushButton:hover { background-color: rgba(80, 220, 255); }");
	ui.RunImageButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt; background-color: rgba(170, 240, 255); } QPushButton:hover { background-color: rgba(80, 220, 255); }");
	ui.stopButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt; background-color: rgba(255, 223, 170); } QPushButton:hover { background-color: rgba(252, 206, 116); }");
	/*
	ui.RemoveAllButton->setStyleSheet("QPushButton { font-size: " + QString::number(20) + "pt;" );
	ui.RemoveAllButton->setIcon(QIcon("Icon/delete.png"));
	ui.RemoveAllButton->setIconSize(QSize(40, 40));
	ui.RemoveAllButton->setMaximumWidth(45);
	*/

	QPushButton *RemoveAllButton = new QPushButton();
	RemoveAllButton->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
	RemoveAllButton->setIcon(QIcon("Icon/delete.png"));
	RemoveAllButton->setIconSize(QSize(25, 25));
	ui.tabWidget_3->tabBar()->setTabButton(0, QTabBar::LeftSide, RemoveAllButton);

	connect(ui.ApplyButton, SIGNAL(clicked()), this, SLOT(applyClicked()));
	connect(ui.ExitButton, SIGNAL(clicked()), QApplication::instance(), SLOT(quit()));
	connect(ui.RunRecipeButton, SIGNAL(clicked()), this, SLOT(runAllReipeClicked()));
	connect(ui.RunImageButton, SIGNAL(clicked()), this, SLOT(runAllImageClicked()));
	connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(stopCompare()));
	connect(RemoveAllButton, SIGNAL(clicked()), this, SLOT(removeAllFoodItem()));


	connect(ui.scaleBtn_0, SIGNAL(clicked()), this, SLOT(scaleBtn0_Clicked()));
	connect(ui.scaleBtn_1, SIGNAL(clicked()), this, SLOT(scaleBtn1_Clicked()));
	connect(ui.scaleBtn_2, SIGNAL(clicked()), this, SLOT(scaleBtn2_Clicked()));



	// image test
	ui.label_input->setAcceptDrops(true);
	ui.tableWidget_selectedFood->dragEnabled();
	//ui.tableWidget->installEventFilter(this);
	//ui.tableWidget->viewport()->installEventFilter(this);
	qApp->installEventFilter(this);



	/*Mat userImage0 = imread("img.png", -1);
	vector<vector<Point> > userDrawContours0;
	vector<Vec4i> hierarchy0;
	Mat cannyColor0 = cannyThreeCh(userImage0, true);
	findContours(cannyColor0.clone(), userDrawContours0, hierarchy0, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0));

	Mat imgTesting = Mat::zeros(userImage0.size(), CV_8UC4);
	for (int i = 0; i < userDrawContours0.size(); i++) {
			drawContours(imgTesting, vector<vector<Point>>(1, userDrawContours0[i]), 0, Scalar(255, 255, 255, 255), 1, 8);
	}
	imshow("R.png", imgTesting);*/

}

bool mouseIsOnImage = false;
bool mouseDraged = false;

void QtGuiApplication1::mouseDragOnImage(leafNode &cNode, int state, int cValue) {
	mouseIsOnImage = false;
	if (cValue >= 0) {
		Size c_inputSize = cNode.inputSize;

		if (state == 1)
			mouseIsOnImage = true;

		int longest = c_inputSize.width;
		if (c_inputSize.width < c_inputSize.height) { longest = c_inputSize.height; }

		// draw border to show which contour is dragged
		Mat ImgTmp = Mat::zeros(Size(topLabelSize, topLabelSize), CV_8UC4);;
		cv::resize(cNode.userDraw, ImgTmp, Size(topLabelSize, topLabelSize));
		vector<Point> tmpSampleP = cNode.samplepointsOfDrawOri[cValue];
		double ratioW = (double)topLabelSize / (double)cNode.inputSize.width;
		double ratioH = (double)topLabelSize / (double)cNode.inputSize.height;
		for (int p = 0; p < tmpSampleP.size(); p++) {
			tmpSampleP[p].x = (double)tmpSampleP[p].x * ratioW;
			tmpSampleP[p].y = (double)tmpSampleP[p].y * ratioH;
		}
		for (int j = 0; j < tmpSampleP.size(); j++)
			circle(ImgTmp, tmpSampleP[j], 2, Scalar(0, 0, 0, 255), 2);
		setLabel_input(ImgTmp);
	}
	else {
		setLabel_input(cNode.userDraw);
	}
}

bool QtGuiApplication1::eventFilter(QObject *obj, QEvent *event) {
	QString str;
	QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
	QPoint pt = mouseEvent->pos();
	QDragMoveEvent *dragEvent = static_cast<QDragMoveEvent*>(event);
	QPoint pt_drag = dragEvent->pos();

	if (selectImageOrNot) {
		// ???????????????????????
		if (event->type() == QEvent::DragEnter) {
			/*
			QDrag *drag = new QDrag(this);
			drag->setPixmap(QPixmap("add.png"));
			QCursor cursor(Qt::OpenHandCursor);
			drag->setDragCursor(cursor.pixmap(), Qt::MoveAction);
			cursor.pixmap();
			Qt::DropAction dropAction = drag->exec();
			*/
		}

		// if drag a food item in tableWidget
		if (event->type() == QEvent::DragMove && mouseDraged) {
			QPoint p_topLeft = ui.label_input->mapTo(ui.centralWidget, ui.label_input->rect().topLeft());
			QPoint p_bottomRight = ui.label_input->mapTo(ui.centralWidget, ui.label_input->rect().bottomRight());
			QPoint c_position = pt_drag - p_topLeft;
			if (c_position.x() < ui.label_input->width() && c_position.y() < ui.label_input->height() && current_ptr->preprocessingDone) {
				int cValue = current_ptr->contourMat.at<int>(c_position.y(), c_position.x());
				ui.label_4->setText(QString::number(c_position.y()) + "_" + QString::number(c_position.x()));
				selectContourIdx = cValue;
				mouseDragOnImage(*current_ptr, 1, cValue);
			}
		}

		// if click on tableWidget, determine if user click on food
		if (obj == ui.tableWidget_selectedFood->viewport() && event->type() == QEvent::MouseButtonPress) {
			QModelIndex mIdx = ui.tableWidget_selectedFood->indexAt(pt);
			int c_column = mIdx.column();
			int c_row = mIdx.row();

			if (c_column == currentFoodTableOrder[1] && c_row < foodSelectedIndex.Element.size()) {
				//cout << "mouseDraged = true" << endl;
				mouseDraged = true;
				selectFoodIdx = c_row;
				selectFood = getfoodCateFood(foodSelectedIndex, selectFoodIdx);
			}
		}

		// if click on tabWidget_3, determine if user click on food
		if (obj == ui.tableWidget_3->viewport() && event->type() == QEvent::MouseButtonPress) {
			QModelIndex mIdx = ui.tableWidget_3->indexAt(pt);
			int c_column = mIdx.column();
			int c_row = mIdx.row();

			if (c_column == 0) {
				mouseDraged = true;
				selectFoodIdx = c_row;
				selectFood = totalFood[foodSuggestedList[selectFoodIdx].fIndex];
			}
		}

		// if mouse release, determine if user drag on catoon image or not
		if (event->type() == QEvent::MouseButtonRelease) {
			mouseDraged = false;
			if (mouseIsOnImage) {
				mouseIsOnImage = false;

				if (selectFoodIdx >= 0 && selectContourIdx >= 0 && current_ptr->preprocessingDone) {
					ContourIndexClicked(*current_ptr);
				}
				else
					ui.label_4->setText("Please select a food first");
				//selectContourIdx = cValue;
			}
		}

		// if mouse move on contour, then show contour
		if (event->type() == QEvent::MouseMove && obj == ui.label_input && current_ptr->preprocessingDone) {
			//test_contourMat(*current_ptr);
			int cValue = current_ptr->contourMat.at<int>(pt.y(), pt.x());
			ui.label_4->setText(QString::number(pt.y()) + "_" + QString::number(pt.x()));
			mouseDragOnImage(*current_ptr, 0, cValue);
		}

		// if mouse press on contour, then suggest food
		if (event->type() == QEvent::MouseButtonPress && obj == ui.label_input && current_ptr->preprocessingDone) {
			ui.tableWidget_3->clear();
			ui.tabWidget_3->setCurrentIndex(1);
			qApp->processEvents();

			int cValue = current_ptr->contourMat.at<int>(pt.y(), pt.x());
			if (cValue >= 0) {
				compareWithAllFood(*current_ptr, cValue);
			}
		}
	}

	return false;
}

/* --------------------------------Main Function-------------------------------- */
Mat doFlipMat(Mat &tmpWarpImg, Rect &rect) {
	Mat flipWarpImg = Mat::zeros(tmpWarpImg.size(), CV_8UC4);
	////get center x
	//int minX = tmpWarpImg.cols - 1, maxX = 0;
	//for (int fc = 0; fc < tmpWarpImg.cols; fc++)
	//	for (int fr = 0; fr < tmpWarpImg.rows; fr++)
	//		if (tmpWarpImg.at<Vec4b>(fr, fc)[3] != 0) {
	//			if (fc < minX)	minX = fc;
	//			if (fc > maxX)	maxX = fc;
	//		}

	int minX = rect.x, maxX = rect.x + rect.width;
	//do flip image
	for (int wc = 0; wc < tmpWarpImg.cols; wc++) {
		for (int wr = 0; wr < tmpWarpImg.rows; wr++) {
			int wValue = (minX + maxX) - wc;
			if (tmpWarpImg.at<Vec4b>(wr, wc)[3] != 0 && wValue >= 0 && wValue < tmpWarpImg.cols) {
				flipWarpImg.at<Vec4b>(wr, wValue) = tmpWarpImg.at<Vec4b>(wr, wc);
			}
		}
	}
	return flipWarpImg;
}

vector<Point> getWholeContourByPoint(Size _inputSize, vector<Point> vecP) {
	Mat tmp = Mat::zeros(_inputSize, CV_8UC1);
	drawContours(tmp, vector<vector<Point>>(1, vecP), 0, Scalar(255), CV_FILLED);

	vector<vector<Point>> SeqContoursTmp2;
	findContours(tmp, SeqContoursTmp2, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	// find max contour
	int maxIdx = 0, maxNum = 0;
	for (int j = 0; j < SeqContoursTmp2.size(); j++) {
		if (SeqContoursTmp2[j].size() > maxNum) {
			maxIdx = j;
			maxNum = SeqContoursTmp2[j].size();
		}
	}
	return SeqContoursTmp2[maxIdx];
}

vector<Point> getWholeContour(Mat &image) {
	Mat tmp = Mat::zeros(image.size(), CV_8UC1);
	for (int fc = 0; fc < image.cols; fc++)
		for (int fr = 0; fr < image.rows; fr++)
			if (image.at<Vec4b>(fr, fc)[3] != 0)
				tmp.at<uchar>(fr, fc) = 255;
			
	vector<vector<Point>> SeqContoursTmp;
	findContours(tmp, SeqContoursTmp, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	for (int i = 0; i < SeqContoursTmp.size(); i++)
		drawContours(tmp, SeqContoursTmp, i, Scalar(255), CV_FILLED);
	
	Mat dst = tmp.clone();
	dilate(tmp, dst, Mat());
	erode(dst, dst, Mat());

	vector<vector<Point>> SeqContoursTmp2;
	findContours(dst, SeqContoursTmp2, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	// find max contour
	int maxIdx = -1, maxNum = 0;
	for (int j = 0; j < SeqContoursTmp2.size(); j++) {
		if (SeqContoursTmp2[j].size() > maxNum) {
			maxIdx = j;
			maxNum = SeqContoursTmp2[j].size();
		}
	}
	if (maxIdx >= 0)
		return SeqContoursTmp2[maxIdx];
	else
		return vector<Point>();
}

vector<Point>  getContourBiggerThanSize(Mat &image, int foodContourArea, Point &center) {
	vector<vector<Point>> SeqContoursTmp2;
	findContours(image, SeqContoursTmp2, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);

	vector<Point> result;
	// find max contour
	int maxIdx = 0, maxNum = 0;
	double minDist = 500;
	int minIdx = 0;
	for (int j = 0; j < SeqContoursTmp2.size(); j++) {
		double Dist = pointPolygonTest(SeqContoursTmp2[j], center, true);
		if (Dist >= 0) {
			return SeqContoursTmp2[j];
		}
		if (abs(Dist) < minDist) {
			minDist = abs(Dist);
			minIdx = j;
			result = SeqContoursTmp2[j];
		}
		/*if (contourArea(SeqContoursTmp2[j]) > foodContourArea) {
			result.push_back(SeqContoursTmp2[j]);
		}*/
	}
	return result;
}

// for channel 1
vector<Point> getAllContourPoint(Mat &image) {
	vector<Point> Result;
	vector<vector<Point>> SeqContoursTmp2;
	findContours(image, SeqContoursTmp2, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	int maxNum = 0;
	for (int j = 0; j < SeqContoursTmp2.size(); j++) {
		if (SeqContoursTmp2[j].size() > maxNum) {
			Result.assign(SeqContoursTmp2[j].begin(), SeqContoursTmp2[j].end());
			maxNum = SeqContoursTmp2[j].size();
		}
	}

	if (maxNum == 0)  {
		vector<Point> tmp;
		return tmp;
	}
	return Result;
}

vector<vector<Point> > reScaleFood2(Mat doo, Size &cSize, vector<Point> smapleP, double scaleSize, vector<float> &rotateAngle, vector<Mat> &rotateMat) {
	Mat canv = Mat::zeros(cSize, CV_8UC1);
	drawContours(canv, vector<vector<Point>>(1, smapleP), 0, Scalar(255), CV_FILLED);
	// resize only if it scale
	if (scaleSize != 1) {
		cSize.width /= scaleSize;
		cSize.height /= scaleSize;
		cv::resize(canv, canv, cSize);
	}
	cv::resize(doo, doo, cSize);

	vector<vector<Point> > orientationP;
	for (int i = 0; i < rotateAngle.size(); i++) {
		//Mat canv_c = canv.clone();
		Mat rot_mat = getRotationMatrix2D(Point(cSize.width / 2, cSize.height / 2), rotateAngle[i], 1);
		Mat dst;
		warpAffine(canv, dst, rot_mat, Size(cSize.width, cSize.height));

		// find whole contour
		vector<vector<Point>> SeqContoursTmp;
		findContours(dst, SeqContoursTmp, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);

		// find max contour, get _contourPoint
		svgParse tmpParse;
		int maxIdx = tmpParse.getMaxContour(SeqContoursTmp);

		descri descriUser(SeqContoursTmp[maxIdx], 0, 1);
		orientationP.push_back(descriUser.sampleResult());
		if (i == 0) {
			smapleP.clear();
			smapleP = descriUser.sampleResult();
		}

		Mat doo22 = doo.clone();
		Mat doo33;
		warpAffine(doo22, doo33, rot_mat, Size(cSize.width, cSize.height));
		rotateMat.push_back(doo33.clone());
		//drawContours(doo33, vector<vector<Point>>(1, descriUser.sampleResult()), 0, Scalar(255, 0, 255, 255), 2, 8);
		//imwrite("Result0/123/" + to_string(rotateAngle[i]) + "_" + to_string(i) + ".png", doo33);
	}
	//system("pause");
	return orientationP;
}

// set colorOfContour, samplePointOri, groupIdx, groupWarpMatOfContour// centerOfContour, topLeft, bottomRight, centerOfContour, 
void prePocessOfUserDraw_A(leafNode &cNode, double scaleSize) {
	vector<Vec4b> colorOfContour(cNode.colorOfContour.begin(), cNode.colorOfContour.end());
	cNode.samplepointsOfDrawOri.clear();
	cNode.colorOfContour.clear();
	for (int i = 0; i < cNode.contourPoint.size(); i++) {
		vector<Point> userDrawContours(cNode.contourPoint[i].begin(), cNode.contourPoint[i].end());
		descri descriUser(userDrawContours, 0, 1);
		cNode.samplepointsOfDrawOri.push_back(descriUser.sampleResult());
		cNode.colorOfContour.push_back(colorOfContour[i]);
	}
	cNode.contourAmount = cNode.samplepointsOfDrawOri.size();

	// get reverse desOfDraw and samplepointsOfDraw
	vector<vector<Point> > samplepointsOfDrawReverse;
	for (int i = 0; i < cNode.samplepointsOfDrawOri.size(); i++) {
		vector<Point> tmpContour = cNode.samplepointsOfDrawOri[i];
		reversePoint(cNode.centerOfContour[i], tmpContour);
		samplepointsOfDrawReverse.push_back(tmpContour);
	}

	// grouping
	cNode.groupIdx.clear();
	cNode.group.resize(cNode.contourAmount);
	for (int i = 0; i < cNode.contourAmount; i++){
		cNode.group[i].warpMatOfContour = Mat::zeros(Size(2, 3), CV_64F);
		cNode.group[i].warpMatOrNot = false;
		cNode.group[i].flipOrNot = true;
	}

	grouping(cNode, samplepointsOfDrawReverse);

	cNode.preprocessingDone = true;
	cNode.sortedFragList.clear();
	cNode.sortedFragList.resize(cNode.contourAmount);
	cNode.foodSuggestedVec.clear();
	cNode.foodSuggestedVec.resize(cNode.contourAmount);
}

// samplePoint, descri
void prePocessOfUserDraw_B(leafNode &cNode) {
	cout << "-------------------------@@@@@@@@@@@@@@@@@---------------------------- prePocessOfUserDraw_B" << endl;
	cNode.scaleRatio = globalScaleSize;
	cNode.samplepointsOfDraw.clear();
	cNode.stackOfContour.clear();

	for (int i = 0; i < cNode.contourAmount; i++) {
		vector<Point> userDrawContours(cNode.contourPoint[i].begin(), cNode.contourPoint[i].end());
		descri descriUser(userDrawContours, 1, cNode.scaleRatio);
		cNode.samplepointsOfDraw.push_back(descriUser.sampleResult());
	}

	cNode.stackOfContour.resize(cNode.samplepointsOfDraw.size());
	for (int i = 0; i < cNode.samplepointsOfDraw.size(); i++) {
		cNode.stackOfContour[i] = true;
		Mat imgI = Mat::zeros(cNode.inputSize, CV_8UC1);
		drawContours(imgI, vector<vector<Point> >(1, cNode.samplepointsOfDraw[i]), 0, Scalar(255), CV_FILLED);
		Mat imgJ = Mat::zeros(cNode.inputSize, CV_8UC1);

		for (int j = i + 1; j < cNode.samplepointsOfDraw.size(); j++) {
			if (i != j) {
				drawContours(imgJ, vector<vector<Point> >(1, cNode.samplepointsOfDraw[j]), 0, Scalar(255), CV_FILLED);
			}
		}

		int overlap = 0;
		for (int cols = 0; cols < cNode.inputSize.width; cols++) {
			for (int rows = 0; rows < cNode.inputSize.height; rows++) {
				if (imgI.at<uchar>(rows, cols) > 0 && imgJ.at<uchar>(rows, cols) > 0) {
					overlap++;
				}
			}
		}

		if (overlap>0.1*contourArea(cNode.samplepointsOfDraw[i]))
			cNode.stackOfContour[i] = false;
	}

	/*for (int i = 0; i < cNode.samplepointsOfDraw.size(); i++) {
		Mat imgI = Mat::zeros(cNode.inputSize, CV_8UC1);
		drawContours(imgI, vector<vector<Point> >(1, cNode.samplepointsOfDraw[i]), 0, Scalar(255), CV_FILLED);
		imwrite("Result0/" + to_string(i) + "_" + to_string(cNode.stackOfContour[i]) + ".png", imgI);
	}*/

	//// reset group
	//for (int i = 0; i < cNode.groupWarpMatOrNot.size(); i++) {
	//	if (cNode.groupWarpMatOrNot[i]) {
	//		cNode.groupWarpMatOfContour[i].at<double>(0, 0) *= c_scaleSize;
	//		cNode.groupWarpMatOfContour[i].at<double>(0, 1) *= c_scaleSize;
	//		cNode.groupWarpMatOfContour[i].at<double>(0, 2) *= c_scaleSize;
	//		cNode.groupWarpMatOfContour[i].at<double>(1, 0) *= c_scaleSize;
	//		cNode.groupWarpMatOfContour[i].at<double>(1, 1) *= c_scaleSize;
	//		cNode.groupWarpMatOfContour[i].at<double>(1, 2) *= c_scaleSize;
	//	}
	//}

	//// reset boundaryOfContour, centerOfContour
	//for (int i = 0; i < cNode.boundaryOfContour.size(); i++) {
	//	cNode.boundaryOfContour[i].x *= c_scaleSize;
	//	cNode.boundaryOfContour[i].y *= c_scaleSize;
	//	cNode.boundaryOfContour[i].width *= c_scaleSize;
	//	cNode.boundaryOfContour[i].height *= c_scaleSize;
	//	cNode.centerOfContour[i].x *= c_scaleSize;
	//	cNode.centerOfContour[i].y *= c_scaleSize;
	//}
	
	cNode.preprocessingDone_B = true;
	cNode.sortedFragList.resize(cNode.samplepointsOfDraw.size());
}

void getStackVec(vector<int> &stackOrder, vector<fragList> &candidate, topo &graph) {
	stackOrder.clear();
	vector<int> notExistVec;
	for (int i = 0; i < candidate.size(); i++) {
		bool isExist = false;
		for (int j = 0; j < graph.sortResult.size(); j++) {
			if (i == graph.sortResult[j]) {
				isExist = true;
				break;
			}
		}
		if (!isExist && candidate[i].Element.size()>0) {
			notExistVec.push_back(i);
		}
	}
	//*/
	int notExistIdx = 0;
	for (int i = 0; i < graph.sortResult.size(); i++) {
		while (notExistIdx<notExistVec.size() && graph.sortResult[i]>notExistVec[notExistIdx]) {
			stackOrder.push_back(notExistVec[notExistIdx]);
			notExistIdx++;
		}
		if (candidate[graph.sortResult[i]].Element.size()>0) {
			stackOrder.push_back(graph.sortResult[i]);
		}
	}
}

void getStackPointVec(vector<Point> &resultVec, vector<vector<Point> > &seqVec) {
	resultVec.clear();
	resultVec.insert(resultVec.end(), seqVec[seqVec.size() - 1].begin(), seqVec[seqVec.size() - 1].end());

	for (int cIdx = seqVec.size() - 2; cIdx >= 0; cIdx--) {
		for (int pp = 0; pp < seqVec[cIdx].size(); pp++) {
			Point currentPoint(seqVec[cIdx][pp]);
			bool isInArea = false;
			for (int cIdx2 = seqVec.size() - 1; cIdx2 > cIdx; cIdx2--) {
				float cDist = pointPolygonTest(seqVec[cIdx2], currentPoint, true);
				if (cDist > 0) {
					isInArea = true;
				}
			}
			if (!isInArea) {
				resultVec.push_back(currentPoint);
			}
		}
	}
}

void QtGuiApplication1::ContourIndexClicked(leafNode &cNode) {
	clock_t start = clock();
	ui.label_4->setText("");

	if (!cNode.preprocessingDone_B) {
		preprocessBinit(cNode);
	}
	if (!cNode.isStoredAsNode) {
		cNode.sortedFragList.resize(cNode.samplepointsOfDraw.size());
		for (int i = 0; i < cNode.sortedFragList.size(); i++)
			cNode.sortedFragList[i].Element.clear();
	}

	int contourIdx = selectContourIdx;
	int sGroupIdx_I = finGroupIdx(cNode.groupIdx, contourIdx);
	int contourIdx_first = cNode.groupIdx[sGroupIdx_I][0];
	Size c_inputSize = cNode.inputSize;
	food tmpFood = selectFood;
	recipe tmpRec;
	tmpRec.addFood(tmpFood.name, 10, 0, tmpFood.category);

	fragList tmpPair;
	doCompare(true, tmpPair, cNode, contourIdx_first, tmpFood);

	if (tmpPair.Element.size() > 0) {
		// if selectContourIdx is the first in its group
		if (contourIdx_first == contourIdx) {
			cNode.sortedFragList[contourIdx_first] = tmpPair;
		}
		else {
			cNode.sortedFragList[contourIdx] = tmpPair;
			setFragToGroup(cNode, contourIdx_first, cNode.sortedFragList[contourIdx_first], contourIdx, cNode.sortedFragList[contourIdx]);
		}
		stackWithAddingOrder(cNode);

		if (!cNode.isStoredAsNode) {
			cNode.isStoredAsNode = true;
			cNode.leafNodeVecIdx = leafNodeVec.size();
			leafNodeVec.push_back(cNode);
			current_ptr = &leafNodeVec[leafNodeVec.size() - 1];
		}
		updateResultTable(0);
		ui.label_4->setText("Find a candidate");
	}
	else
		ui.label_4->setText("Cannot find a candidate");

	clock_t end = clock();
	cout << "Total time = " << (end - start) / 1000 << " seconds" << endl;
}

void QtGuiApplication1::compareWithAllFood(leafNode &cNode, int cValue) {
	if (cValue >= 0) {
		clock_t start = clock();
		ui.progressBar->setValue(0);

		if (!cNode.preprocessingDone_B) {
			preprocessBinit(cNode);
		}
		int contourIdx = cValue;

		// if current contourIdx has no foodSuggestedVec
		if (cNode.foodSuggestedVec[contourIdx].size() > 0) {
			foodSuggestedList = cNode.foodSuggestedVec[contourIdx];
		}
		else {
			vector<frag> c_foodSuggestedIdx;
			int sGroupIdx_I = finGroupIdx(cNode.groupIdx, contourIdx);
			int contourIdx_first = cNode.groupIdx[sGroupIdx_I][0];

			int currentC = 0;
			int totalFoodA = 0;
			for (int i = 0; i < categoryStr.size(); i++)
				totalFoodA += cateFood.Element[categoryStr[i]].Element.size();
			map<string, vector<food> >::iterator iterFood;
			for (int i = 0; i < categoryStr.size(); i++) {
				for (iterFood = cateFood.Element[categoryStr[i]].Element.begin(); iterFood != cateFood.Element[categoryStr[i]].Element.end(); iterFood++, currentC++) {
					ui.progressBar->setValue((100 / totalFoodA) * currentC);
					food tmpF = iterFood->second[0];

					fragList tmpFrag;
					doCompare(false, tmpFrag, cNode, contourIdx_first, tmpF);

					for (int fragS = 0; fragS < tmpFrag.Element.size(); fragS++) {
						if ((tmpFrag.Element[fragS].fAmount > 1 && tmpFrag.Element[fragS].iError < 0.3) || (tmpFrag.Element[fragS].fAmount == 1 && tmpFrag.Element[fragS].icpVarianceError < 100 && tmpFrag.Element[fragS].iError < 0.5)) {
							c_foodSuggestedIdx.push_back(tmpFrag.Element[fragS]);
						}
					}
				}
			}

			cNode.foodSuggestedVec[contourIdx_first] = c_foodSuggestedIdx;
			for (int j = 1; j < cNode.groupIdx[sGroupIdx_I].size(); j++) {
				int contourIdx2 = cNode.groupIdx[sGroupIdx_I][j];

				for (int k = 0; k < c_foodSuggestedIdx.size(); k++) {
					frag tmpTable = c_foodSuggestedIdx[k];
					tmpTable.cIndex = contourIdx2;

					// do warp image
					Mat tmpWarpImg = Mat::zeros(cNode.inputSize, CV_8UC4);
					tmpWarpImg = RotateMat(tmpTable.warpImg, cNode.group[contourIdx2].angle);

					warpAffine(tmpWarpImg, tmpWarpImg, cNode.group[contourIdx2].warpMatOfContour, cNode.inputSize);
					if (cNode.group[contourIdx2].flipOrNot)
						tmpTable.warpImg = doFlipMat(tmpWarpImg, cNode.boundaryOfContour[contourIdx2]).clone();
					else
						tmpTable.warpImg = tmpWarpImg.clone();
					//imwrite("Result0/" + to_string(k) + ".png", tmpTable.candidate.warpImg);

					cNode.foodSuggestedVec[contourIdx2].push_back(tmpTable);
				}
			}
		}
		// store result to contour ans its group
		foodSuggestedList = cNode.foodSuggestedVec[contourIdx];

		ui.progressBar->setValue(100);
		updateSuggestTable(ui.tableWidget_3, cNode.contourPoint[contourIdx], cNode.colorOfContour[contourIdx], foodSuggestedList);
		qApp->processEvents();

		clock_t end = clock();
		cout << "CompareWithAllFood Time = " << (end - start) / 1000 << " seconds" << endl;
	}
}

void QtGuiApplication1::compareWithRecipe(leafNode &cNode, recipe& rec2, int progressValue0, int progressValue1) {
	clock_t start = clock();
	if (!cNode.preprocessingDone_B) {
		preprocessBinit(cNode);
	}

	// compare contour with food
	vector<fragList> sortedFragList(cNode.samplepointsOfDraw.size());
	cNode.isStoredAsNode = false;

	// compare contour according to group
	for (int i = 0; i < cNode.groupIdx.size(); i++) {
		// set progressing bar
		if (applyMode == 0) { ui.progressBar->setValue(100 / cNode.groupIdx.size() * (i + 1)); }
		else { ui.progressBar->setValue(progressValue0 + ((progressValue1 - progressValue0) / cNode.groupIdx.size() * (i + 1))); }

		int contourIdx1 = cNode.groupIdx[i][0];
		vector<fragList> pairSeq(rec2.size);
		//#pragma omp parallel for 
		for (int j = 0; j < rec2.size; j++)	{
			food tmpFood;
			tmpFood.setInitial(rec2.foodName()[j], rec2.foodName()[j], rec2.foodCategory()[j], 0, 0);
			doCompare(false, pairSeq[j], cNode, contourIdx1, tmpFood);
		}
		// every food push the best to foodCandidate
		for (int j = 0; j < rec2.size; j++)
			for (int k = 0; k < pairSeq[j].Element.size(); k++)
				if (pairSeq[j].Element.size()>0)
					sortedFragList[contourIdx1].Element.push_back(pairSeq[j].Element[0]);
		sort(sortedFragList[contourIdx1].Element.begin(), sortedFragList[contourIdx1].Element.end(), compareWithiError_cError);
		
		// decide the adding order
		if (sortedFragList[contourIdx1].Element.size()>0) {
			for (int j = 0; j < sortedFragList[contourIdx1].Element.size(); j++) {
				imwrite("Result0/AAA_" + to_string(sortedFragList[contourIdx1].Element[j].cIndex) + "_" + to_string(sortedFragList[contourIdx1].Element[j].iError) + "_" + to_string(sortedFragList[contourIdx1].Element[j].cError) + ".png", sortedFragList[contourIdx1].Element[j].warpImg);
			}
			// for other contour "contourIdx2" that are in the same group(contourIdx)
			for (int g = 1; g < cNode.groupIdx[i].size(); g++) {
				int contourIdx2 = cNode.groupIdx[i][g];
				sortedFragList[contourIdx2] = sortedFragList[contourIdx1];
				setFragToGroup(cNode, contourIdx1, sortedFragList[contourIdx1], contourIdx2, sortedFragList[contourIdx2]);
			}
		}
	}
	clock_t end = clock();
	cout << "compare Time: " << end - start << ", ";

	// Greedy build Start ----------------------------------------
	cNode.sortedFragList = sortedFragList;
	stackWithAddingOrder(cNode);

	cNode.isStoredAsNode = true;
	cNode.leafNodeVecIdx = leafNodeVec.size();
	leafNodeVec.push_back(cNode);

	current_ptr = &leafNodeVec[leafNodeVec.size() - 1];
	updateResultTable(0);
	qApp->processEvents();
}

/* --------------------------------Build Tree Function-------------------------------- */
vector<string> split_str(string s, char ch) {
	vector<string> tokens;
	istringstream ss(s);
	string token;

	while (std::getline(ss, token, ch)) {
		tokens.push_back(token);
	}
	return tokens;
}

double getMinDistance(vector<Point> seqTarget, vector<Point> seqSource) {
	double totalDist = 0;
	for (int cIdx = 0; cIdx < seqTarget.size(); cIdx++) {
		Point p0(seqTarget[cIdx]);
		double minDist = 0;
		int minCIdx2 = 0;

		for (int cIdx2 = 0; cIdx2 < seqSource.size(); cIdx2++) {
			Point p1(seqSource[cIdx2]);
			double dist = sqrt(pow(p0.x - p1.x, 2) + pow(p0.y - p1.y, 2));

			if (cIdx2 == 0)
				minDist = dist;
			else if (dist < minDist) {
				minDist = dist;
				minCIdx2 = cIdx2;
			}
		}
		totalDist += minDist;
	}
	totalDist /= seqTarget.size();
	return totalDist;
}

void appendAllFrag(int state, tree<string>& tr, string c_str, int nextIndex, vector<fragList>& sortedFragList) {

	tree<string>::iterator loc = find(tr.begin(), tr.end(), c_str);
	if (state != 0) {
		for (int i = 0; i < sortedFragList[nextIndex].Element.size(); i++) {
			tr.append_child(loc, c_str + "_" + to_string(nextIndex) + "*" + to_string(i));
		}
	}
	else {
		for (int i = 0; i < sortedFragList[nextIndex].Element.size(); i++) {
			tr.append_child(loc, to_string(nextIndex) + "*" + to_string(i));
		}
	}
}

void stackWithAddingOrder(leafNode &cNode) {
	Mat resultStack = Mat::zeros(inputSizeOri, CV_8UC4);
	for (int i = 0; i < cNode.sortedFragList.size(); i++) {
		if (cNode.sortedFragList[i].Element.size()>0)
			resultStack = addTransparent(resultStack, cNode.sortedFragList[i].Element[0].warpImg);
	}
	cNode.resultStack = resultStack.clone();
};

double getHealthError(vector<int> nutrient) {
	double tmpHErr = 0;
	for (int i = 0; i < nutrient_Str.size(); i++)
		tmpHErr += abs((nutrient[i]/nutrientDay[i]-nutrientValue[i]));
	return tmpHErr / nutrient_Str.size();
}

vector<int> getStrNutrient(vector<fragList>& c_sortedFragList, recipe& recipeTmp) {
	vector<int> tmp_Nutrient(nutrient_Str.size(), 0);

	for (int i = 0; i < c_sortedFragList.size(); i++) {
		if (c_sortedFragList[i].Element.size()>0) {
			int candidateIndex = 0;
			for (int j = 0; j < recipeTmp.foodName().size(); j++) {
				food tmpF = getFoodVecFromStr(recipeTmp.foodName()[j])[0];
				int recipeNum = c_sortedFragList[i].Element[candidateIndex].recipe_num[j];
				if (recipeNum > 0) {
					tmp_Nutrient[0] += tmpF.cal*recipeNum;
					tmp_Nutrient[1] += tmpF.protein*recipeNum;
					tmp_Nutrient[2] += tmpF.calcium*recipeNum;
					tmp_Nutrient[3] += tmpF.iron*recipeNum;
					tmp_Nutrient[4] += tmpF.vitaminB1*recipeNum;
					tmp_Nutrient[5] += tmpF.vitaminB2*recipeNum;
					tmp_Nutrient[6] += tmpF.vitaminC*recipeNum;
					tmp_Nutrient[7] += tmpF.vitaminE*recipeNum;
				}
			}
		}
	}
	return tmp_Nutrient;
}

vector<food> getFoodVecFromStr(string name) {
	vector<food> tmpFoodVec;
	for (int i = 0; i < totalFood.size(); i++)
		if (totalFood[i].name.compare(name) == 0)
			tmpFoodVec.push_back(totalFood[i]);
	
	return tmpFoodVec;
}

void QtGuiApplication1::updateResultTable(int mode) {
	// resultIdxAmount_min resultIdxAmount_max
	ui.tableWidget_6->clearContents();
	ui.tableWidget_6->setRowCount(2);

	if (mode == 0) {
		//line(currentTopImage, Point(10, 10), Point(10, 60), Scalar(255, 255, 255, 255), 1, 8);
		setLabel_show(true, current_ptr->userDraw, current_ptr->resultStack, current_ptr->globalCenter);
	}

	int imgSize = ui.tableWidget_6->width() / resultIdxAmount_min;
	if (ui.tableWidget_6->width() / resultIdxAmount_min < ui.tableWidget_6->height())
		imgSize = ui.tableWidget_6->height();

	if (leafNodeVec.size() > leafNodeAmount) { leafNodeVec.pop_back(); }

	int smallSize = resultIdxAmount_max;
	if (leafNodeVec.size() < resultIdxAmount_max) { smallSize = leafNodeVec.size(); }
	ui.tableWidget_6->setColumnCount(smallSize);

	ui.tableWidget_6->horizontalHeader()->setDefaultSectionSize(ui.tableWidget_6->height());

	for (int i = 0; i < smallSize; i++) {
		Mat currentImage = leafNodeVec[i].resultStack.clone();
		QImage qImg((uchar*)currentImage.data, currentImage.cols, currentImage.rows, QImage::Format_ARGB32);
		QImage smallImg = (&qImg)->scaled(imgSize, imgSize, Qt::KeepAspectRatio);

		QTableWidgetItem *pCell = new QTableWidgetItem;
		pCell->setData(Qt::DecorationRole, QPixmap::fromImage(smallImg));
		ui.tableWidget_6->removeCellWidget(0, i);
		ui.tableWidget_6->setItem(0, i, pCell);

		//string _name, string _fileName, string _category, int _season, int _period
		string tmpName = "Result " + to_string(i + 1);
		food foodTmp;
		foodTmp.setInitial(tmpName, "", "", 0, 0);
		//foodTmp.setHealth(leafNodeVec[i].nutrientVec[0], leafNodeVec[i].nutrientVec[1], leafNodeVec[i].nutrientVec[2], leafNodeVec[i].nutrientVec[3], leafNodeVec[i].nutrientVec[4], leafNodeVec[i].nutrientVec[5], leafNodeVec[i].nutrientVec[6], leafNodeVec[i].nutrientVec[7], 1);
		QString html = getToolTipQString(foodTmp);
		pCell->setToolTip(html);

		QWidget* pWidget = new QWidget();
		QPushButton* btn_del = new QPushButton();
		btn_del->setText("Delete");
		btn_del->setMinimumWidth(imgSize - 50);
		btn_del->setStyleSheet("QPushButton { font-size: " + QString::number(15) + "pt; background-color: rgba( 255, 170, 170); } QPushButton:hover { background-color: rgba(255, 119, 119); }");

		QSignalMapper* signalMapper = new QSignalMapper(this);
		connect(btn_del, SIGNAL(clicked()), signalMapper, SLOT(map()));
		signalMapper->setMapping(btn_del, i);
		connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(removeLeafNode(int)));

		QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
		pLayout->addWidget(btn_del);
		pLayout->setAlignment(Qt::AlignCenter);
		pLayout->setContentsMargins(0, 0, 0, 0);
		pWidget->setLayout(pLayout);

		ui.tableWidget_6->setCellWidget(1, i, pWidget);
	}
	
	connect(ui.tableWidget_6, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(ResultIndexClicked(QTableWidgetItem*)));
}

/* --------------------------------Food Combination Function-------------------------------- */
vector<vector<Point>> getNewContourOri(vector<Point> &contourPoint, Mat warpImg) {
	Mat contour_dist(warpImg.size(), CV_32FC1);
	Mat foodAlphaBin = alphaBinary(warpImg);
	Mat new_drawClone = Mat::zeros(warpImg.size(), CV_8UC1);

	for (int i = 0; i < warpImg.cols; i += 1) {
		for (int j = 0; j < warpImg.rows; j += 1) { //height
			contour_dist.at<float>(j, i) = pointPolygonTest(contourPoint, Point2f(i, j), true);
			if (contour_dist.at<float>(j, i) > 0 && !(foodAlphaBin.at<int>(j, i))) {
				new_drawClone.at<uchar>(j, i) = 255;
			}
		}
	}
	//morphological opening (removes small objects from the foreground)
	erode(new_drawClone, new_drawClone, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(new_drawClone, new_drawClone, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	//morphological closing (removes small holes from the foreground)
	dilate(new_drawClone, new_drawClone, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(new_drawClone, new_drawClone, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	vector<vector<Point> > userDrawContours;
	vector<Vec4i> hierarchy;
	findContours(new_drawClone.clone(), userDrawContours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0));
	sort(userDrawContours.begin(), userDrawContours.end(), compareContourSize);

	if (userDrawContours.size() > 0) {
		vector<int>hull;
		convexHull(userDrawContours[0], hull, true);
		if (hull[0] > hull[1])
			reverse(userDrawContours[0].begin(), userDrawContours[0].end());
		Mat drawing = Mat::zeros(warpImg.size(), CV_8UC3);
		for (int u = 0; u < userDrawContours.size(); u++) {
			drawContours(drawing, userDrawContours, u, Scalar(255, 255, 255), 1, 8);
		}
		return userDrawContours;
	}
	else {
		vector<vector<Point>> buff;
		return buff;
	}
}

vector<vector<Point>> getNewContour(Size c_inputSize, Mat &alphaImg) {
	//morphological opening (removes small objects from the foreground)
	erode(alphaImg, alphaImg, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(alphaImg, alphaImg, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	//morphological closing (removes small holes from the foreground)
	dilate(alphaImg, alphaImg, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(alphaImg, alphaImg, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	vector<vector<Point> > userDrawContours;
	vector<Vec4i> hierarchy;
	findContours(alphaImg.clone(), userDrawContours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0));
	sort(userDrawContours.begin(), userDrawContours.end(), compareContourSize);

	if (userDrawContours.size() > 0) {
		vector<int>hull;
		convexHull(userDrawContours[0], hull, true);
		if (hull[0] > hull[1])
			reverse(userDrawContours[0].begin(), userDrawContours[0].end());
		Mat drawing = Mat::zeros(c_inputSize, CV_8UC3);
		for (int u = 0; u < userDrawContours.size(); u++) {
			drawContours(drawing, userDrawContours, u, Scalar(255, 255, 255), 1, 8);
		}
		return userDrawContours;
	}
	else {
		vector<vector<Point>> buff;
		return buff;
	}
}

/* --------------------------------Pre-Processing-------------------------------- */
void preProcess(string s_file, vector<vector<Point> >& samplePoint, string p_file) {
	int idx = 0;

	vector<int> getSize;
	ifstream fin(preDataDir + s_file, ios::in | ios::binary);
	int num;
	fin.read((char *)&num, sizeof(num));
	
	int testM = 0;
	for (int i = 0; i<num; ++i) {
		int buff;
		fin.read((char *)&buff, sizeof(buff));
		getSize.push_back(buff);
		testM += buff;
	}

	vector<Point> get_sampleResult = vecPointRead(preDataDir + p_file);
	vector<Point> buff_sampleResult;
	cout << "get_sampleResult.size()= " << get_sampleResult.size() << endl;

	int count = 0;
	for (int i = 0; i < get_sampleResult.size(); i++)	{
		buff_sampleResult.push_back(get_sampleResult[i]);
		count++;
		if (count == getSize[idx]) {
			idx++;
			count = 0;
			samplePoint.push_back(buff_sampleResult);
			buff_sampleResult.clear();
		}
	}
}

vector<Mat> vecmatread(const string& filename)
{
	vector<Mat> matrices;
	ifstream fs(filename, fstream::binary);

	// Get length of file
	fs.seekg(0, fs.end);
	int length = fs.tellg();
	cout << filename<<": "<<length << endl;
	fs.seekg(0, fs.beg);
	
	while (fs.tellg() < length)
	{
		//cout << "!!!" << endl;
		// Header
		int rows, cols, type, channels;
		fs.read((char*)&rows, sizeof(int));         // rows
		fs.read((char*)&cols, sizeof(int));         // cols
		fs.read((char*)&type, sizeof(int));         // type
		fs.read((char*)&channels, sizeof(int));     // channels

		// Data
		Mat mat(rows, cols, type);
		fs.read((char*)mat.data, CV_ELEM_SIZE(type) * rows * cols);

		matrices.push_back(mat);
	}

	return matrices;
}

vector<Point> vecPointRead(const string& filename)
{
	Point buff;
	vector<Point> matrices;
	ifstream fs(filename, fstream::binary);

	int num;
	fs.read((char *)&num, sizeof(num));

	for (int i = 0; i<num; ++i) {
		int val_x, val_y;
		fs.read((char *)&val_x, sizeof(val_x));
		fs.read((char *)&val_y, sizeof(val_y));
		buff.x = val_x;
		buff.y = val_y;
		matrices.push_back(buff);
	}
	return matrices;
}

void getAllRecipe(string recipeDir, vector<string>& files, vector<recipe>& RecipeFiles) {
	/*add stuff inside the table view*/
	for (int i = 0; i < files.size(); i++) {
		recipe rec;
		rec.readRecipeFile(recipeDir + files[i]);
		rec.print();
		RecipeFiles.push_back(rec);
	}
}

void readFoodPreData(vector<food> &totalFoodVec) {
	ifstream fileN(foodPreDir + "name.bin", ios::binary);
	ifstream fileF(foodPreDir + "fileName.bin", ios::binary);
	ifstream fileC(foodPreDir + "cate.bin", ios::binary);
	ifstream fileS(foodPreDir + "season.bin", ios::binary);
	ifstream fileP(foodPreDir + "period.bin", ios::binary);
	ifstream fileH(foodPreDir + "health.bin", ios::binary);

	int length;

	/*fileN*/
	vector<string> foodN;
	fileN.seekg(0, fileN.end);
	length = fileN.tellg();
	fileN.seekg(0, fileN.beg);
	char* tempN = new char[length + 1];
	if (fileN.good())
		fileN.read(tempN, length);
	else
		cout << "fileN error" << endl;

	tempN[length] = '\0';

	stringstream  dataN(tempN);
	string lineN;

	while (getline(dataN, lineN, '*'))
		foodN.push_back(lineN);


	/*fileF*/
	vector<string> foodF;
	fileF.seekg(0, fileF.end);
	length = fileF.tellg();
	fileF.seekg(0, fileF.beg);
	char* tempF = new char[length + 1];
	if (fileF.good())
		fileF.read(tempF, length);
	else
		cout << "fileF error" << endl;

	tempF[length] = '\0';

	stringstream  dataF(tempF);
	string lineF;

	while (getline(dataF, lineF, '*'))
		foodF.push_back(lineF);

	/*fileC*/
	vector<string> foodC;
	fileC.seekg(0, fileC.end);
	length = fileC.tellg();
	fileC.seekg(0, fileC.beg);
	char* tempC = new char[length + 1];
	if (fileC.good())
		fileC.read(tempC, length);
	else
		cout << "fileC error" << endl;

	tempC[length] = '\0';

	stringstream  dataC(tempC);
	string lineC;

	while (getline(dataC, lineC, '*'))
		foodC.push_back(lineC);

	/*fileS*/
	fileS.seekg(0, fileS.end);
	length = fileS.tellg();
	fileS.seekg(0, fileS.beg);

	vector<int> foodS(length / sizeof(int));
	fileS.read((char*)foodS.data(), foodS.size()*sizeof(int));

	/*fileP*/
	fileP.seekg(0, fileP.end);
	length = fileP.tellg();
	fileP.seekg(0, fileP.beg);

	vector<int> foodP(length / sizeof(int));
	fileP.read((char*)foodP.data(), foodP.size()*sizeof(int));
	
	/*fileH*/
	fileH.seekg(0, fileH.end);
	length = fileH.tellg();
	fileH.seekg(0, fileH.beg);
	vector<double> foodH(length / sizeof(double));
	fileH.read((char*)foodH.data(), foodH.size()*sizeof(double));

	fileH.close();
	cout << "-----------------------------" << endl;
	/*push food*/
	for (int i = 0; i < foodN.size(); i++)
	{
		food food;
		food.setInitial(foodN[i], foodF[i], foodC[i], foodS[i], foodP[i]);
		int fSize = 10;

		bool stackable = true;
		if (foodH[i * fSize + 9] == 0) { stackable = false; }
		food.setHealth(foodH[i*fSize], foodH[i * fSize + 1], foodH[i *fSize + 2], foodH[i * fSize + 3], foodH[i *fSize + 4], foodH[i * fSize + 5], foodH[i * fSize + 6], foodH[i * fSize + 7], foodH[i * fSize + 8], stackable);
		food.orderIdx = i;
		//food.printInfo();
		//food.printHealth();
		//cout << food.name << endl;
		totalFoodVec.push_back(food);
	}
}

void readBinaryImg(vector<Mat> &totalImg, string fileName)
{
	ifstream fileI(fileName, ios::binary);
	fileI.seekg(0, fileI.end);
	int length = fileI.tellg();
	fileI.seekg(0, fileI.beg);
	int i = 0;
	while (fileI.tellg() < length)
	{
		int rows, cols, type;
		fileI.read((char*)&rows, sizeof(int));
		fileI.read((char*)&cols, sizeof(int));
		fileI.read((char*)&type, sizeof(int));

		Mat nMat = Mat::zeros(rows, cols, type);
		for (int j = 0; j < nMat.rows*nMat.cols; j++)
		{
			Vec4b data;
			fileI.read((char*)&data, sizeof(data));
			nMat.at<Vec4b>(j) = data;
		}
		//cout << "i: " << i << " ";
		//i++;
		totalImg.emplace_back(nMat);
	}
	fileI.close();
}

// set widthShiftLimit, heightShiftLimit
void getOverlapRatio(food &cFood, vector<Point> &pointSeq) {
	// food info
	bool stackable = cFood.stackable;

	int foodContourArea = contourArea(pointSeq);
	Mat foodAlpha = Mat::zeros(inputSize, CV_8UC1);
	drawContours(foodAlpha, vector<vector<Point>>(1, pointSeq), 0, Scalar(255), CV_FILLED);

	// find boundary of contour anf food, // left right top bottom // find non alpha point of food
	vector<Point> nonAlphaOfFood;
	Rect foodRect = getBoundaryRectAndAlpha(foodAlpha, Rect(0, 0, foodAlpha.cols, foodAlpha.rows), nonAlphaOfFood);
	Point lastFP = Point(foodRect.x, foodRect.y);

	int combineFAmount = 0;
	int tolerantOverlapArea = 0.3*foodContourArea;

	Mat testFoodWidthOverlap = Mat::zeros(Size(foodAlpha.cols * 2, foodAlpha.rows), CV_8UC1);
	foodAlpha.copyTo(testFoodWidthOverlap(cv::Rect(0, 0, foodAlpha.cols, foodAlpha.rows)));

	// find width overlap limit
	int widthShiftLimit = foodAlpha.cols;
	for (int w = 0; w <foodAlpha.cols; w += 3) {
		int overlapCount = 0;
		//Mat testWidthShift = testFoodWidthOverlap.clone();
		for (int i = 0; i < nonAlphaOfFood.size(); i++) {
			Point cPoint = Point(nonAlphaOfFood[i].x + w, nonAlphaOfFood[i].y);
			if (cPoint.x >= 0 && cPoint.x < foodAlpha.cols && foodAlpha.at<uchar>(cPoint.y, cPoint.x) > 0)
				overlapCount++;
			//else
				//circle(testWidthShift, cPoint, 1, 125, 2);
		}
		//imwrite("Result0/testWidthShift" + to_string(w) + ".png", testWidthShift);
		if (overlapCount < tolerantOverlapArea) {
			widthShiftLimit = w;
			break;
		}
	}
	//system("pause");

	Mat testFoodHeightOverlap = Mat::zeros(Size(foodAlpha.cols, foodAlpha.rows * 2), CV_8UC1);
	foodAlpha.copyTo(testFoodHeightOverlap(cv::Rect(0, 0, foodAlpha.cols, foodAlpha.rows)));
	// find height overlap limit
	int heightShiftLimit = foodAlpha.rows;
	for (int h = 0; h <foodAlpha.rows; h += 3) {
		int overlapCount = 0;
		for (int i = 0; i < nonAlphaOfFood.size(); i++) {
			Point cPoint = Point(nonAlphaOfFood[i].x, nonAlphaOfFood[i].y + h);
			if (cPoint.y >= 0 && cPoint.y < foodAlpha.rows && foodAlpha.at<uchar>(cPoint.y, cPoint.x) > 0) 
				overlapCount++;
		}
		if (overlapCount < tolerantOverlapArea) {
			heightShiftLimit = h;
			break;
		}
	}

	cFood.rect = foodRect;
	cFood.alphaMat = foodAlpha.clone();
	cFood.widthShiftLimit = widthShiftLimit;
	cFood.heightShiftLimit = heightShiftLimit;
	
} 

// set widthShiftLimit, heightShiftLimit
Vec4b getFoodColor(Mat &foodImg) {
	double blue = 0, green = 0, red = 0;
	int count = 0;
	for (int i = 0; i <foodImg.cols; i++) {
		for (int j = 0; j < foodImg.rows; j++) {
			if (foodImg.at<Vec4b>(j, i)[3]>0) {
				blue += foodImg.at<Vec4b>(j, i)[0];
				green += foodImg.at<Vec4b>(j, i)[1];
				red += foodImg.at<Vec4b>(j, i)[2];
				count++;
			}
		}
	}
	blue /= (double)count;
	green /= (double)count;
	red /= (double)count;
	Vec4b c_color(blue, green, red, 255);
	return c_color;
}

/* --------------------------------Image Processing-------------------------------- */
Mat shiftMat(Mat &img, int shiftX, int shiftY) {
	Mat warpImg = Mat::zeros(img.size(), img.type());
	Mat warpingMatrix = Mat::zeros(Size(3, 2), CV_64F);
	warpingMatrix.at<double>(0, 0) = 1;
	warpingMatrix.at<double>(0, 1) = 0;
	warpingMatrix.at<double>(0, 2) = shiftX;
	warpingMatrix.at<double>(1, 0) = 0;
	warpingMatrix.at<double>(1, 1) = 1;
	warpingMatrix.at<double>(1, 2) = shiftY;
	warpAffine(img, warpImg, warpingMatrix, img.size());
	return warpImg;
}

void MatACutMatB(Rect &boundary, Mat &MatA, Mat &MatB) {
	for (int i = boundary.x; i < boundary.x + boundary.width; i++) {
		for (int j = boundary.y; j < boundary.y + boundary.height; j++) {
			if (MatB.at<Vec4b>(j, i)[3]>0) {
				MatA.at<uchar>(j, i) = 0;
			}
		}
	}
	//imwrite("Result0/MatAA.png", MatA);
}

Point2f warpPoint(Point2f prePoint, Mat &warpMat) {
	double newWX = warpMat.at<double>(0, 0)*prePoint.x + warpMat.at<double>(0, 1)*prePoint.y + warpMat.at<double>(0, 2);
	double newWY = warpMat.at<double>(1, 0)*prePoint.x + warpMat.at<double>(1, 1)*prePoint.y + warpMat.at<double>(1, 2);
	return Point2f(newWX, newWY);
}

Point2f warpNorm(Point2f &norm, Mat &warpMat) {
	double newWX = warpMat.at<double>(0, 0)*norm.x + warpMat.at<double>(0, 1)*norm.y;
	double newWY = warpMat.at<double>(1, 0)*norm.x + warpMat.at<double>(1, 1)*norm.y;
	double scale = sqrt(pow(newWX, 2) + pow(newWY, 2));
	return Point2f(newWX / scale, newWY / scale);
}

vector<Point> getNormPoint(Point p, Point2f norm) {
	Point2f c_normal_nmliz = normalizeVec(norm);
	return vector<Point>{p, p + Point(c_normal_nmliz)};
}

void drawPoint(Size imgSize, vector<Point> &vec, string str) {
	Mat tmp = Mat::zeros(imgSize, CV_8UC4);
	for (int i = 0; i < vec.size(); i++) {
		circle(tmp, vec[i], 1, Scalar(255, 255, 255, 255), 2);
	}
	imwrite(str, tmp);
}

void draw2Point(Size imgSize, vector<Point> &vec, vector<Point> &vec2, string str) {
	Mat tmp = Mat::zeros(imgSize, CV_8UC4);
	for (int i = 0; i < vec.size(); i++) {
		circle(tmp, vec[i], 1, Scalar(255, 255, 255, 255), 2);
	}
	for (int i = 0; i < vec2.size(); i++) {
		circle(tmp, vec2[i], 1, Scalar(255, 0, 255, 255), 2);
	}
	imwrite(str, tmp);
}

void draw3Point(Size imgSize, vector<Point> &vec, vector<Point> &vec2, vector<Point> &vec3, string str) {
	Mat tmp = Mat::zeros(imgSize, CV_8UC4);
	for (int i = 0; i < vec.size(); i++) {
		circle(tmp, vec[i], 1, Scalar(255, 255, 255, 255), 2);
	}
	for (int i = 0; i < vec2.size(); i++) {
		circle(tmp, vec2[i], 1, Scalar(255, 0, 255, 255), 2);
	}
	for (int i = 0; i < vec3.size(); i++) {
		circle(tmp, vec3[i], 1, Scalar(255, 255, 0, 255), 2);
	}
	imwrite(str, tmp);
}

double tmpCompareWithoutDes(bool compareWhole, int iterationTimes, Size &inputSize, vector<Point> seqP1, vector<Point> seqP2, Mat &finalMat, vector<Point> &newSampleP, int errorMode) {
	solver icpSolver(compareWhole, iterationTimes, inputSize, seqP2, seqP1);
	newSampleP = icpSolver.getResultPointSeq();
	finalMat = icpSolver.getResultMat().clone();
	return icpSolver.getIcpVarErr();
}

// paste BGRA to BGRA
Mat overlayImage(Mat &bg, Mat &fg, Point &p) {
	Mat result = bg.clone();
	for (int i = 0; i < fg.cols; i++) {
		for (int j = 0; j < fg.rows; j++) {
			if (fg.at<Vec4b>(j, i)[3] != 0) {
				result.at<Vec4b>(j + p.y, i + p.x) = fg.at<Vec4b>(j, i);
			}
		}
	}
	return result;
}

Mat RotateMat(Mat &img, float rad) {
	Point2f src_center(img.cols / 2.0F, img.rows / 2.0F);
	Mat rot_mat = getRotationMatrix2D(src_center, rad, 1.0);
	Mat dst;
	warpAffine(img, dst, rot_mat, img.size());
	return dst;
}

vector<Point> RotatePoint(vector<Point> &pointSeq, float rad, Size imgSize) {
	Point centerP(imgSize.width / 2, imgSize.height / 2);
	vector<Point> new_pointSeq;
	for (int i = 0; i < pointSeq.size(); i++) {
		Point trans_pt(pointSeq[i] - centerP);
		float x = std::cos(rad) * trans_pt.x - std::sin(rad) * trans_pt.y;
		float y = std::sin(rad) * trans_pt.x + std::cos(rad) * trans_pt.y;
		Point new_pt(Point(x, y) + centerP);
		new_pointSeq.push_back(new_pt);
	}

	/*Mat canv4 = Mat::zeros(imgSize, CV_8UC4);
	drawContours(canv4, vector<vector<Point>>(1, new_pointSeq), 0, Scalar(255, 0, 255, 255), 1, 8);
	imwrite("canv_" + to_string(rad) + ".png", canv4);*/
	return new_pointSeq;
}

vector<Point> RotatePoint2(Mat &img, vector<Point> smapleP, float rad) {
	vector<Point> resultVec;
	Mat rot_mat = getRotationMatrix2D(Point(img.cols / 2, img.rows / 2), rad, 1.0);
	for (int i = 0; i < smapleP.size(); i++) {
		Point2f p(smapleP[i].x, smapleP[i].y);
		Mat coordinates = (Mat_<double>(3, 1) << p.x, p.y, 1);
		Mat result = rot_mat * coordinates;

		Point p1_back;
		p1_back.x = (int)result.at<double>(0, 0);
		p1_back.y = (int)result.at<double>(1, 0);
		resultVec.push_back(p1_back);
	}
	return resultVec;
}

void grouping(leafNode &cNode, vector<vector<Point> > &samplepointsOfDrawReverse) {
	int smapleSize = cNode.contourAmount;
	vector<vector<bool> > tmp_group(smapleSize, vector<bool>(smapleSize, false));
	for (int i = 0; i < smapleSize; i++) {
		//int iSize = cNode.samplepointsOfDrawOri[i].size();
		int iSize = contourArea(cNode.samplepointsOfDrawOri[i]);
		for (int j = (i + 1); j < smapleSize; j++) {
			bool breakOrNot = false;
			//int jSize = cNode.samplepointsOfDrawOri[j].size();
			int jSize = contourArea(cNode.samplepointsOfDrawOri[j]);
			for (int k = 0; k < smapleSize; k++) {
				if (tmp_group[j][k]) {
					breakOrNot = true;
					break;
				}
			}
			int smallDis = iSize/10;
			if ( /*i==16 && j==30 &&*/ (!breakOrNot) && cNode.colorOfContour[i] == cNode.colorOfContour[j] && (iSize + smallDis) > jSize && (iSize - smallDis) < jSize) {
				Mat finalMat = Mat::zeros(Size(3, 2), CV_64F);
				double thresholdIcpError = 15;
				double minIcpError = thresholdIcpError;
				vector<Point> newSampleP = cNode.samplepointsOfDrawOri[i];
				int angleIdx = 0;

				Mat drawing00 = Mat::zeros(cNode.inputSize, CV_8UC4);
				vector<float> rotateAngle = { 0, 180 };
				//vector<float> rotateAngle = { 0, 90, 180, 270 };
				for (int ori = 0; ori < rotateAngle.size(); ori++) {
					// food different orientation info
					int theta = rotateAngle[ori];
					vector<Point> sampleP_TMP = RotatePoint2(cNode.userDraw, cNode.samplepointsOfDrawOri[i], theta);
					Mat finalMat0 = Mat::zeros(Size(3, 2), CV_64F);
					vector<Point> newSampleP0 = sampleP_TMP;
					double minIcpError0 = tmpCompareWithoutDes(true, 20, cNode.inputSize, samplepointsOfDrawReverse[j], sampleP_TMP, finalMat0, newSampleP0, 0);
					if (minIcpError0 < minIcpError) {
						angleIdx = theta;
						minIcpError = minIcpError0;
						newSampleP = newSampleP0;
						finalMat = finalMat0.clone();
						cNode.group[j].flipOrNot = true;
						//draw3Point(cNode.inputSize, samplepointsOfDrawReverse[j], sampleP_TMP, newSampleP0, "Result0/" + to_string(i) + "_" + to_string(j) + "_" + to_string(theta) + "_0.png");
					}
					//system("pause");
					Mat finalMat1 = Mat::zeros(Size(3, 2), CV_64F);
					vector<Point> newSampleP1 = sampleP_TMP;
					double minIcpError1 = tmpCompareWithoutDes(true, 20, cNode.inputSize, cNode.samplepointsOfDrawOri[j], sampleP_TMP, finalMat1, newSampleP1, 0);
					if (minIcpError1 < minIcpError) {
						angleIdx = theta;
						minIcpError = minIcpError1;
						newSampleP = newSampleP1;
						finalMat = finalMat1.clone();
						cNode.group[j].flipOrNot = false;
						//draw3Point(cNode.inputSize, cNode.samplepointsOfDrawOri[j], sampleP_TMP, newSampleP1, "Result0/" + to_string(i) + "_" + to_string(j) + "_" + to_string(theta) + "_1.png");
					}
					//system("pause");
				}
				
				if (minIcpError < thresholdIcpError) {
					tmp_group[i][j] = true;
					tmp_group[j][i] = true;
					cNode.group[j].warpMatOrNot = true;
					cNode.group[j].angle = angleIdx;
					cNode.group[j].warpMatOfContour = finalMat.clone();

					Mat drawing0 = Mat::zeros(cNode.inputSize, CV_8UC4);
					drawContours(drawing0, cNode.samplepointsOfDrawOri, i, Scalar(255, 255, 255, 255), 1, 8);

					// draw point that was warpped
					Mat tmpWarpImgT = Mat::zeros(cNode.inputSize, CV_8UC4);
					Mat tmpWarpImg = Mat::zeros(cNode.inputSize, CV_8UC4);
					drawContours(tmpWarpImgT, cNode.samplepointsOfDrawOri, i, Scalar(255, 255, 255, 255), 1, 8);
					tmpWarpImg = RotateMat(tmpWarpImgT, angleIdx);
					//imwrite("Result0/tmpWarpImg0.png", tmpWarpImg);
					warpAffine(tmpWarpImg, tmpWarpImg, finalMat, cNode.inputSize);
					if (cNode.group[j].flipOrNot) 
						tmpWarpImg = doFlipMat(tmpWarpImg, cNode.boundaryOfContour[j]);
					//imwrite("Result0/tmpWarpImg1.png", tmpWarpImg);
					for (int a = 0; a < tmpWarpImg.cols; a++) {
						for (int b = 0; b < tmpWarpImg.rows; b++) {
							if (tmpWarpImg.at<Vec4b>(j, i)[3] != 0)
								drawing0.at<Vec4b>(j, i) = Vec4b(0, 0, 255, 255);
						}
					}

					if (cNode.group[j].flipOrNot)
						drawContours(drawing0, samplepointsOfDrawReverse, j, Scalar(255, 255, 255, 255), 1, 8);
					else
						drawContours(drawing0, cNode.samplepointsOfDrawOri, j, Scalar(255, 255, 255, 255), 1, 8);

					vector<vector<Point> > testSample;
					testSample.push_back(newSampleP);
					drawContours(drawing0, testSample, 0, Scalar(255, 0, 255, 255), 1, 8);

					imwrite("Result0/group/" + to_string(i) + "_" + to_string(j) + "_" + to_string(minIcpError) + ".png", drawing0);
				}


			}
		}
	}

	vector<int> gIdx(smapleSize, -1);
	vector<int> tmpIdx;
	int gcount = 0;
	for (int i = 0; i < smapleSize; i++) {
		if (gIdx[i] == -1) {
			gIdx[i] = gcount;
			tmpIdx.push_back(i);

			for (int j = (i + 1); j <smapleSize; j++) {
				if (tmp_group[i][j]) {
					gIdx[j] = gcount;
					tmpIdx.push_back(j);
				}
			}
			cNode.groupIdx.push_back(tmpIdx);
			gcount++;
		}
		tmpIdx.clear();
	}

	for (int i = 0; i < cNode.groupIdx.size(); i++) {
		cout << "groupIdx" << i << "= ";
		for (int g = 0; g < cNode.groupIdx[i].size(); g++)
			cout << cNode.groupIdx[i][g] << ", ";
		cout << endl;
	}
}

// get the top, left, bottom, right
void getBorder(Mat &image, int& top, int& left, int& width, int& height) {
	int bottom, right;

	bool findIt = false; // find left
	for (int j = 0; j < image.cols; j++) {
		for (int i = 0; i < image.rows; i++) {
			if (image.at<Vec4b>(i, j)[3] != 0) {
				findIt = true;
				left = j;
				break;
			}
		}
		if (findIt)
			break;
	}
	findIt = false; // find right
	for (int j = image.cols - 1; j >= 0; j--) {
		for (int i = 0; i < image.rows; i++) {
			if (image.at<Vec4b>(i, j)[3] != 0) {
				findIt = true;
				right = j;
				break;
			}
		}
		if (findIt)
			break;
	}
	findIt = false; // find top
	for (int i = 0; i < image.rows; i++) {
		for (int j = left; j <= right; j++) {
			if (image.at<Vec4b>(i, j)[3] != 0) {
				findIt = true;
				top = i;
				break;
			}
		}
		if (findIt)
			break;
	}
	findIt = false; // find bottom
	for (int i = image.rows - 1; i >= 0; i--) {
		for (int j = left; j <= right; j++) {
			if (image.at<Vec4b>(i, j)[3] != 0)
			{
				findIt = true;
				bottom = i;
				break;
			}
		}
		if (findIt)
			break;
	}

	width = right - left + 1;
	height = bottom - top + 1;
}

// reverse vVector<Point> with center
void reversePoint(Point center, vector<Point>& contourPoint) {
	int dist;
	for (int i = 0; i < contourPoint.size(); i++) {
		dist = contourPoint[i].x - center.x;
		contourPoint[i].x -= 2 * dist;
	}
	reverse(contourPoint.begin(), contourPoint.end());
}

Point getCenterPoint(vector<Point> &pointSeq) {
	int tX = 0, tY = 0;
	for (int i = 0; i < pointSeq.size(); i++) {
		tX += pointSeq[i].x;
		tY += pointSeq[i].y;
	}
	tX /= pointSeq.size();
	tY /= pointSeq.size();
	return Point(tX, tY);
}

Point getCenterPointFromImg(Mat &img) {
	vector<Point> nonAlphaP;
	for (int i = 0; i < img.cols; i++) {
		for (int j = 0; j < img.rows; j++) {
			if (img.at<Vec4b>(j, i)[3] != 0)
				nonAlphaP.push_back(Point(i,j));
		}
	}
	return getCenterPoint(nonAlphaP);
}

// add image with transparent background
Mat addTransparent(Mat &bg, Mat &fg)
{
	Mat result;
	bg.copyTo(result);
	for (int y = 0; y < bg.rows; ++y)
	{
		int fY = y;
		if (fY >= fg.rows) break;
		for (int x = 0; x < bg.cols; ++x)
		{
			int fX = x;
			if (fX >= fg.cols) break;
			double Fopacity = ((double)fg.data[fY * fg.step + fX * fg.channels() + 3]) / 255; // opacity of the foreground pixel

			for (int c = 0; Fopacity > 0 && c < result.channels(); ++c) // combine the background and foreground pixel
			{
				unsigned char foregroundPx = fg.data[fY * fg.step + fX * fg.channels() + c];
				unsigned char backgroundPx = bg.data[y * bg.step + x * bg.channels() + c];
				result.data[y*result.step + result.channels()*x + c] = backgroundPx * (1. - Fopacity) + foregroundPx * Fopacity;

			}
		}
	}
	return result;
}

// get subPointSeq
vector<Point> subPointSeq(vector<Point> inputSeq, int startIndex, int range)
{
	vector<Point> result;

	for (int i = 0; i < range; i++)
	{
		result.push_back(inputSeq[(startIndex + i) % inputSeq.size()]);
	}
	return result;
}

// canny edge detection for each channel
Mat cannyThreeCh(Mat input, bool mode) {
	vector<Mat> channels;

	split(input, channels);

	Mat B = channels[0];
	Mat G = channels[1];
	Mat R = channels[2];
	Mat A = channels[3];

	for (int i = 0; i < input.rows; i++) {
		for (int j = 0; j < input.cols; j++) {
			if (A.at<uchar>(i, j) == 0) {
				B.at<uchar>(i, j) = 0;
				G.at<uchar>(i, j) = 0;
				R.at<uchar>(i, j) = 0;
			}
		}
	}

	/*imwrite("_R.png", R);
	imwrite("_G.png", G);
	imwrite("_B.png", B);
	imwrite("_A.png", A);*/

	int lowTh;
	int highTh;

	if (mode) {
		lowTh = 50;
		highTh = 150;
	}
	else {
		lowTh = 250;
		highTh = 750;
	}

	Mat cannyB, cannyG, cannyR, cannyA;

	Canny(B, cannyB, lowTh, highTh, 3);
	Canny(G, cannyG, lowTh, highTh, 3);
	Canny(R, cannyR, lowTh, highTh, 3);
	Canny(A, cannyA, lowTh, highTh, 3);
	Mat cannyColor;

	/*imwrite("_RC.png", cannyR);
	imwrite("_GC.png", cannyG);
	imwrite("_BC.png", cannyB);
	imwrite("_AC.png", cannyA);*/

	bitwise_or(cannyB, cannyG, cannyColor);
	bitwise_or(cannyColor, cannyR, cannyColor);
	bitwise_or(cannyColor, cannyA, cannyColor);
	//imwrite("_C.png", cannyColor);
	return cannyColor;
}

// get all files in the dir
int getdir(string dir, vector<string> &files)
{
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(dir.c_str())) == NULL)
	{
		return errno;
	}
	while ((dirp = readdir(dp)) != NULL)
	{
		files.push_back(dirp->d_name);
	}
	closedir(dp);

	files.erase(files.begin(), files.begin() + 2);
	return 0;
}

//segment image with alpha value
Mat alphaBinary(Mat input)
{
	Mat alphaOrNot = Mat::zeros(input.size(), CV_32S);
	for (int i = 0; i < input.cols; i++)
	{
		for (int j = 0; j < input.rows; j++)
		{
			Vec4b & bgra = input.at<Vec4b>(j, i);
			if (bgra[3] != 0) // not transparency
				alphaOrNot.at<int>(j, i) = 1;
			else
				alphaOrNot.at<int>(j, i) = 0;

		}
	}
	return alphaOrNot;
}

//scale img icon
Mat scaleIconImg(Mat foodImg) {
	int tmp_top, tmp_left, tmp_width, tmp_height;
	getBorder(foodImg, tmp_top, tmp_left, tmp_width, tmp_height);
	Mat cutMat = foodImg(Rect(tmp_left, tmp_top, tmp_width, tmp_height));

	double w_scale = (double)foodImg.cols / (double)tmp_width;
	double h_scale = (double)foodImg.rows / (double)tmp_height;
	double small_scale;
	if (w_scale < h_scale)
		small_scale = w_scale;
	else
		small_scale = h_scale;
	small_scale *= 0.7;

	Size newS = Size(cutMat.cols*small_scale, cutMat.rows*small_scale);
	resize(cutMat, cutMat, newS);

	Size iconSize = Size(foodImg.cols * 0.8, foodImg.rows * 0.8);
	Mat finalMat = Mat::zeros(iconSize, CV_8UC4);
	cutMat.copyTo(finalMat(cv::Rect((finalMat.cols - cutMat.cols) / 2, (finalMat.rows - cutMat.rows) / 2, cutMat.cols, cutMat.rows)));

	return finalMat;
}

//scale img icon
Mat scaleSuggestResultImg(Mat foodImg, vector<Point> &pointVec, Vec4b &color) {
	/*for (int i = 0; i < pointVec.size(); i++) {
		Point cPoint = pointVec[i];
		if (foodImg.at<Vec4b>(cPoint.y, cPoint.x)[3] == 0) {
			foodImg.at<Vec4b>(cPoint.y, cPoint.x) = color;
		}
	}*/
	
	//Mat newIconImg = cv::Mat(foodImg.size(), CV_8UC4);
	////drawContours(tmpFrag.Element[k].warpImg, vector<vector<Point>>(1, *pointSeq1), 0, Scalar(255, 0, 255, 255), 2, 8);
	//drawContours(newIconImg, vector<vector<Point> >(1, pointVec), 0, Scalar(color), CV_FILLED);
	////foodImg.copyTo(newIconImg.rowRange(0, foodImg.rows).colRange(0, foodImg.cols));
	//foodImg = newIconImg.clone();
	
	int tmp_top, tmp_left, tmp_width, tmp_height;
	getBorder(foodImg, tmp_top, tmp_left, tmp_width, tmp_height);
	Mat cutMat = Mat::zeros(Size(tmp_width, tmp_height), CV_8UC4);
	cutMat = foodImg(Rect(tmp_left, tmp_top, tmp_width, tmp_height));

	double w_scale = (double)foodImg.cols / (double)tmp_width;
	double h_scale = (double)foodImg.rows / (double)tmp_height;
	double small_scale;
	if (w_scale < h_scale)
		small_scale = w_scale;
	else
		small_scale = h_scale;
	small_scale *= 0.5;

	Size newS = Size(cutMat.cols*small_scale, cutMat.rows*small_scale);
	resize(cutMat, cutMat, newS);

	Size iconSize = Size(foodImg.cols * 0.8, foodImg.rows * 0.8);
	Mat finalMat = Mat::zeros(iconSize, CV_8UC4);
	cutMat.copyTo(finalMat(cv::Rect((finalMat.cols - cutMat.cols) / 2, (finalMat.rows - cutMat.rows) / 2, cutMat.cols, cutMat.rows)));

	return finalMat;
}

/* --------------------------------Error Function-------------------------------- */
Vec3f RGBToHSV(Vec4b RGB_color) {
	int R = RGB_color[2], G = RGB_color[1], B = RGB_color[0];
	int Cmax = max(max(R, G), B);
	int Cmin = min(min(R, G), B);
	int delta = Cmax - Cmin;

	// set H value
	double H = 0;
	if (delta != 0) {
		if (Cmax==R) {
			if (G >= B) {
				H = 60 * ((double)(G - B) / (double)delta);
			}
			else {
				H = (60 * ((double)(G - B) / (double)delta)) + 360;
			}
		}
		else if (Cmax == G) {
			H = (60 * ((double)(B - R) / (double)delta)) + 120;
		}
		else { // Cmax == B
			H = (60 * ((double)(R - G) / (double)delta)) + 240;
		}
	}

	// set S value
	double S = 0;
	if (Cmax != 0) {
		S = 1 - ((double)Cmin / (double)Cmax);
	}

	// set V value
	double V = Cmax;

	return Vec3f(H, S, V);
}

double colorError(Vec4b contourColor, Vec4b foodColor) {
	// HSV color
	double h0 = RGBToHSV(contourColor)[0];
	double h1 = RGBToHSV(foodColor)[0];
	double hueDistance = min(abs(h1 - h0), 360 - abs(h1 - h0));
	hueDistance /= 180;

	double s0 = RGBToHSV(contourColor)[1];
	double s1 = RGBToHSV(foodColor)[1];
	double saturateDistance = abs(s1 - s0);

	double v0 = RGBToHSV(contourColor)[2];
	double v1 = RGBToHSV(foodColor)[2];
	double valueDistance = abs(v1 - v0);
	valueDistance /= 255;

	double cError = sqrt(hueDistance*hueDistance + saturateDistance*saturateDistance + valueDistance*valueDistance);

	/*Mat img = Mat::zeros(Size(500, 500), CV_8UC4);
	rectangle(img, Point(0, 0), Point(250, 500), Scalar(contourColor), -1);
	rectangle(img, Point(250, 0), Point(500, 500), Scalar(foodColor), -1);
	imwrite("Result0/color/" + to_string(cError) + "_" + to_string(hueDistance) + "_" + to_string(saturateDistance) + "_" + to_string(valueDistance) + ".png", img);*/

	return cError;
	//return cError;

	//// RGB color distance
	//double resultScore = sqrt(pow(contourColor[0] - foodColor[0], 2) + pow(contourColor[1] - foodColor[1], 2) + pow(contourColor[2] - foodColor[2], 2));
	//double denominator = pow(256, 2);
	//denominator = 3 * denominator;
	//denominator = sqrt(denominator);
	//return (resultScore / denominator);
}

// resultStack - 3channel, foodAlpha - 1channel
void imageOverlap(Mat &resultStack, Mat &foodAlpha, double &_iError, double &_ratio1, double &_ratio2) {
	//Mat drawing0 = Mat::zeros(resultStack.size(), CV_8UC3);
	//drawing0 = addTransparent(drawing0, resultStack);
	//drawing0 = addTransparent(drawing0, foodAlpha);

	int contourArea = 0, foodArea = 0, overlapArea = 0;
	for (int i = 0; i < resultStack.cols; i += 2) {
		for (int j = 0; j < resultStack.rows; j += 2) {
			if (resultStack.at<Vec4b>(j, i)[3] > 0) {
				contourArea++;
				if (foodAlpha.at<Vec4b>(j, i)[3] > 0) {
					foodArea++;
					overlapArea++;
					//circle(drawing0, Point(i, j), 1, Scalar(0, 255, 255), 1);
				}
				else {
					//circle(drawing0, Point(i, j), 1, Scalar(0, 0, 255), 1);
				}
			}
			else {
				if (foodAlpha.at<Vec4b>(j, i)[3] > 0) {
					foodArea++;
					//circle(drawing0, Point(i, j), 1, Scalar(0, 255, 0), 1);
				}
			}
		}
	}
	if (overlapArea==0) {
		_ratio1 = 1;
		_ratio2 = 1;
		_iError = 1;
	}
	else {
		double ratio_tmp = (double)overlapArea / (double)(contourArea + foodArea - overlapArea);
		double ratio1_tmp = (double)overlapArea / (double)contourArea; // contour 
		double ratio2_tmp = (double)overlapArea / (double)foodArea; // food
		_ratio1 = (1-ratio1_tmp);
		_ratio2 = (1-ratio2_tmp);
		_iError = (1-ratio_tmp);
	}
	//imwrite("Result0/" + to_string(_ratio1) + "_" + to_string(_ratio2) + "_" + to_string(_iError) + ".png", drawing0);
	//system("pause");
}

// resultStack - 3channel, foodAlpha - 1channel
bool imageOverlapOutandFront(Mat &contourAlpha, Mat &resultStack, Mat &foodStack) {
	//Mat drawing0 = Mat::zeros(resultStack.size(), CV_8UC3);
	//Mat drawing1 = Mat::zeros(resultStack.size(), CV_8UC3);

	int contourArea = 0, stackArea = 0, foodArea = 0;
	int contourArea_overlap = 0, stackArea_overlap = 0;
	for (int i = 0; i < foodStack.cols; i += 1) {
		for (int j = 0; j < foodStack.rows; j += 1) {
			// determine the outer contour
			if (foodStack.at<Vec4b>(j, i)[3] > 0) { // if is in foodStack
				foodArea++;
				//circle(drawing0, Point(i, j), 1, Scalar(0, 255, 0), 1);
				//circle(drawing1, Point(i, j), 1, Scalar(0, 255, 0), 1);
				if (contourAlpha.at<uchar>(j, i)>0) { // if is in contourAlpha
					contourArea++;
					contourArea_overlap++;
					//circle(drawing0, Point(i, j), 1, Scalar(0, 255, 255), 1);
				}
				if (resultStack.at<Vec4b>(j, i)[3] > 0) {// if is in frontAlpha
					stackArea++;
					stackArea_overlap++;
					//circle(drawing1, Point(i, j), 1, Scalar(0, 255, 255), 1);
				}
			}
			else {
				if (contourAlpha.at<uchar>(j, i) > 0) { // if is in contourAlpha
					contourArea++;
					//circle(drawing0, Point(i, j), 1, Scalar(0, 0, 255), 1);
				}
				if (resultStack.at<Vec4b>(j, i)[3] > 0) { // if is in frontAlpha
					stackArea++;
					//circle(drawing1, Point(i, j), 1, Scalar(0, 0, 255), 1);
				}
			}
		}
	}
	

	// overlap info of contourAlpha
	double out_ratio1 = 0, out_ratio2 = 0, out_iError = 0;
	double ratio_tmp = (double)contourArea_overlap / (double)(contourArea + foodArea - contourArea_overlap);
	double ratio1_tmp = (double)contourArea_overlap / (double)contourArea; // contour 
	double ratio2_tmp = (double)contourArea_overlap / (double)foodArea; // food
	out_ratio1 = 1 - ratio1_tmp;
	out_ratio2 = 1 - ratio2_tmp;
	out_iError = (1 - ratio_tmp);

	// overlap info of resultStack
	double stack_ratio1 = 0, stack_ratio2 = 0, stack_iError = 0;
	ratio_tmp = (double)stackArea_overlap / (double)(stackArea + foodArea - contourArea_overlap);
	ratio1_tmp = (double)stackArea_overlap / (double)stackArea; // contour 
	ratio2_tmp = (double)stackArea_overlap / (double)foodArea; // food
	stack_ratio1 = 1 - ratio1_tmp;
	stack_ratio2 = 1 - ratio2_tmp;
	stack_iError = (1 - ratio_tmp);

	//imwrite("Result0/drawing0" + to_string(out_ratio2) + ".png", drawing0);
	//imwrite("Result0/drawing1" + to_string(stack_ratio2) + ".png", drawing1);
	//system("pause");

	if (out_ratio2 < 0.3 && stack_ratio2 > 0.65)
		return true;
	return false;
}

void imageOverlapIndivi(vector<Point> &samplePSeq1, Mat &resultStack, double &_iError, double &_ratio1, double &_ratio2) {
	Mat foodAlphaBin = alphaBinary(resultStack);

	Mat fill_1 = Mat::zeros(resultStack.size(), CV_8UC1);
	drawContours(fill_1, vector<vector<Point>>(1, samplePSeq1), 0, Scalar(255), CV_FILLED);

	int contourArea = 0, foodArea = 0, overlapArea = 0;
	Mat drawing0 = Mat::zeros(resultStack.size(), CV_8UC3);
	for (int i = 0; i < resultStack.cols; i += 1) {
		for (int j = 0; j < resultStack.rows; j += 1) {
			int same = 0;
			// if is in contour
			if (fill_1.at<uchar>(j, i) != 0) {
				contourArea++;
				same++;
				circle(drawing0, Point(i, j), 1, Scalar(0, 0, 255), 1);
			}
			// if is in resultStack
			if ((foodAlphaBin.at<int>(j, i))) {
				foodArea++;
				same++;
				circle(drawing0, Point(i, j), 1, Scalar(0, 255, 0), 1);
			}
			if (same==2) {
				overlapArea++;
				circle(drawing0, Point(i, j), 1, Scalar(0, 255, 255), 1);
			}
			
		}
	}

	double ratio_tmp = (double)overlapArea / (double)(contourArea + foodArea - overlapArea);
	double ratio1_tmp = (double)overlapArea / (double)contourArea; // contour 
	double ratio2_tmp = (double)overlapArea / (double)foodArea; // food
	_ratio1 = 1 - ratio1_tmp;
	_ratio2 = 1 - ratio2_tmp;
	_iError = (1 - ratio_tmp);
	//imwrite("Result0/" + to_string(_ratio1) + "_" + to_string(_ratio2) + "_" + to_string(_iError) + ".png", drawing0);
	//system("pause");
}

double icpError(Mat &resultStack, vector<Point> &contour, int fIndex) {
	Size cSize = resultStack.size();
	vector<Point> foodContour = getWholeContour(resultStack);

	double totalErr = 0, totalErr2 = 0;
	double dist = 0, minDist = 0;

	Mat drawing0 = resultStack.clone();
	for (int i = 0; i < contour.size(); i++)
		circle(drawing0, contour[i], 2, Scalar(255, 255, 255, 255), 2);
	for (int i = 0; i < foodContour.size(); i++)
		circle(drawing0, foodContour[i], 2, Scalar(255, 0, 255, 255), 2);

	vector<double> distVec;
	for (int j = 0; j < contour.size(); j++)	{
		double mDist = pointPolygonTest(foodContour, contour[j], true);
		totalErr += mDist;
		totalErr2 += abs(mDist);
		distVec.push_back(mDist);
	}

	double averageErr = totalErr / contour.size();
	return abs(averageErr);

	totalErr2 /= contour.size();

	double vaError = 0;
	for (int i = 0; i < distVec.size(); i++) {
		vaError += pow((distVec[i] - averageErr), 2);
	}
	vaError /= contour.size();
	
	//imwrite("Result0/" + to_string(vaError) + ".png", drawing0);
	//system("pasue");
	return abs(vaError);
}

void setAllError(frag &inputFrag, leafNode &cNode, int contourIdx) {

	// set color error, already normalize
	double _cError = colorError(cNode.colorOfContour[contourIdx], totalFood[inputFrag.fIndex].color);

	vector<Point> sampleP1 = cNode.samplepointsOfDraw[contourIdx];
	// set iError - overlap ratio
	double _iError = 0;
	double _iErrorRatio1 = 0;
	double _iErrorRatio2 = 0;
	imageOverlapIndivi(sampleP1, inputFrag.warpImg, _iError, _iErrorRatio1, _iErrorRatio2);

	// set icp error
	double _icpError, _icpVarianceError = 999;
	_icpError = icpError(inputFrag.warpImg, sampleP1, inputFrag.fIndex);
	if (inputFrag.fAmount == 1)
		_icpVarianceError = inputFrag.icpVarianceError;

	inputFrag.setError(_cError, _iError, _iErrorRatio1, _iErrorRatio2, _icpError, _icpVarianceError);
	inputFrag.sError = _cError + _iError + _icpError;
}

/* --------------------------------Comparison Value Function-------------------------------- */
bool compareContourSize(vector<Point> contour1, vector<Point> contour2) {
	size_t i = contour1.size();
	size_t j = contour2.size();
	return (i > j);
}

bool compareWithiError(frag input1, frag input2) {
	double i = input1.iError;
	double j = input2.iError;
	return(i<j);
}

bool compareWithiError_cError(frag input1, frag input2) {
	double i = input1.iError*input1.cError;
	double j = input2.iError*input2.cError;
	return(i<j);
}

bool compareWithsError(frag input1, frag input2) {
	double i = input1.sError;
	double j = input2.sError;
	return(i<j);
}

bool compareWithError_Suggest(frag input1, frag input2) {
	if (input1.fAmount == 1) {
		if (input2.fAmount == 1)
			return (input1.icpVarianceError < input2.icpVarianceError);
		else
			return true;
	}
	else if (input2.fAmount == 1) {
		return false;
	}

	return(input1.iError<input2.iError);
}

/* --------------------------------Comparison Contour Function-------------------------------- */
int finGroupIdx(vector<vector<int> > &cGroupIdx, int idx) {
	int sGroupIdx_I = -1, sGroupIdx_J = -1;
	// first find selectContourIdx in which group, whicj index
	for (int i = 0; i < cGroupIdx.size(); i++) {
		for (int j = 0; j < cGroupIdx[i].size(); j++) {
			if (idx == cGroupIdx[i][j]) {
				sGroupIdx_I = i;
				sGroupIdx_J = j;
				break;
			}
		}
		if (sGroupIdx_I >= 0)
			break;
	}
	return sGroupIdx_I;
}

// set fragList of contourIdx1 to contourIdx2
void setFragToGroup(leafNode &cNode, int contourIdx1, fragList &fragList1, int contourIdx2, fragList &fragList2) {
	// for other contour "contourIdx2" that are in the same group(contourIdx)
	for (int j = 0; j < fragList2.Element.size(); j++) {
		fragList2.Element[j].cIndex = contourIdx2;
		fragList2.Element[j].warpImgGroupOri = fragList2.Element[j].warpImg.clone();
		// do warp image
		Mat tmpWarpImg = Mat::zeros(cNode.inputSize, CV_8UC4);
		tmpWarpImg = RotateMat(fragList2.Element[j].warpImgGroupOri, cNode.group[contourIdx2].angle);
		warpAffine(tmpWarpImg, tmpWarpImg, cNode.group[contourIdx2].warpMatOfContour, cNode.inputSize);
		if (cNode.group[contourIdx2].flipOrNot)
			fragList2.Element[j].warpImg = doFlipMat(tmpWarpImg, cNode.boundaryOfContour[contourIdx2]).clone();
		else
			fragList2.Element[j].warpImg = tmpWarpImg.clone();
		//imwrite("Result0/"+to_string(fragList.Element[j].cIndex) + to_string(fragList.Element[j].sError) + ".png", fragList.Element[j].warpImg);
	}
}

// left right top bottom
Rect getBoundaryRectAndAlpha(Mat img, Rect &rect, vector<Point> &nonAlpha) {
	// count center, and border value
	int top = img.rows, bottom = 0, left = img.cols, right = 0;
	for (int i = rect.x; i < (rect.x + rect.width); i++) {
		for (int j = rect.y; j < (rect.y + rect.height); j++) {
			if (img.at<uchar>(j, i) != 0) {
				nonAlpha.push_back(Point(i, j));
				if (i < left)
					left = i;
				if (i > right)
					right = i;
				if (j < top)
					top = j;
				if (j > bottom)
					bottom = j;
			}
		}
	}
	Rect newRect = Rect(left, top, right - left, bottom - top);
	return newRect;
}

// left right top bottom
Rect getBoundaryRectByPoint(vector<Point> &vec, Size imgSize) {
	// count center, and border value
	int top = imgSize.height, bottom = 0, left = imgSize.width, right = 0;
	for (int i = 0; i < vec.size(); i++) {
		if (vec[i].x < left)
			left = vec[i].x;
		if (vec[i].x > right)
			right = vec[i].x;
		if (vec[i].y < top)
			top = vec[i].y;
		if (vec[i].y > bottom)
			bottom = vec[i].y;
	}
	Rect newRect = Rect(left, top, right - left, bottom - top);
	return newRect;
}

// left right top bottom
Rect getWidthBoundaryRect(Mat img, Rect &rect) {
	int left = img.cols;
	for (int j = rect.y; j < (rect.y + rect.height); j++) {
		for (int i = rect.x; i < (rect.x + rect.width); i++) {
			if (img.at<uchar>(j, i) != 0) {
				if (i < left)
					left = i;
			}
		}
	}
	int right = 0;
	for (int j = rect.y; j < (rect.y + rect.height); j++) {
		for (int i = (rect.x + rect.width) - 1; i >= rect.x; i--) {
			if (img.at<uchar>(j, i) != 0) {
				if (i > right)
					right = i;
			}
		}
	}
	Rect newRect = Rect(left, rect.y, right - left, rect.height);
	return newRect;
}

double pointDist(Point A, Point B) {
	double dist = sqrt(pow(A.x - B.x, 2) + pow(A.y - B.y, 2));
	return dist;
}

/* ------------------------------------------------------------------------------------------- */
Point2f norVec(Point pre, Point tgt, Point nxt) {
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

	return Point2f(norT[0], norT[1]);
}

void twoSeqPointToWarp(vector<Point> &seq1, vector<Point> &seq2, int idx1, int idx2, int range, Mat &warpMat, double &icpError) {
	// init seqP1 for seq1, from (idx1) ~ (idx1+range)
	// init seqP2 for seq2, from (idx2) ~ (idx2+range)
	vector<Point> seqP1;
	vector<Point> seqP2;
	for (int rr = 0; rr < range; rr++) {
		seqP1.push_back(seq1[(idx1 + rr) % seq1.size()]);
		seqP2.push_back(seq2[(idx2 + rr) % seq2.size()]);
	}
	// get warp matrix and warp pointSeq
	solver icpSolver;
	warpMat = icpSolver.getTwoSetsTransform(seqP2, seqP1);

	// warp_seqP2 = warpMat * seqP2
	vector<Point> warp_seqP2;
	for (int rr = 0; rr < range; rr++)
		warp_seqP2.push_back(warpPoint(seqP2[rr], warpMat));
	// calculate icp error and weight error
	icpError = icpSolver.getMinDistanceP(warp_seqP2, seqP1);
	//double weightError = ((double)(maxRange - range + 1) / (double)maxRange)*icpError;
}

double getThetaValue(Mat &warpMatrix) {
	double dTheta = 0;
	double matScale = abs(sqrt(pow(warpMatrix.at<double>(0, 0), 2) + pow(warpMatrix.at<double>(0, 1), 2)));

	if (warpMatrix.at<double>(0, 0)>0 && warpMatrix.at<double>(1, 0)>0) { // cos+ sin+
		dTheta = acos(min(1.0, warpMatrix.at<double>(0, 0) / matScale));
	}
	else if (warpMatrix.at<double>(0, 0)>0 && warpMatrix.at<double>(1, 0)<0) { // + -
		dTheta = asin(max(-1.0, warpMatrix.at<double>(1, 0) / matScale));
	}
	else if (warpMatrix.at<double>(0, 0)<0 && warpMatrix.at<double>(1, 0)>0) { // - +
		dTheta = acos(max(-1.0, warpMatrix.at<double>(0, 0) / matScale));
	}
	else { // - -
		dTheta = acos(max(-1.0, warpMatrix.at<double>(0, 0) / matScale));
		dTheta = 2*PI - dTheta;
	}
	return dTheta;
}

struct warpMatrixInfo {
	Mat warpMatrix;
	double theta;
	int groupflag;
};

struct warpGroupCenter {
	double theta;
	vector<int> group;
};

bool hasVisitAll_WarpMatrixInfo(vector<warpMatrixInfo> &warpMatrixInfoVec, int &id) {
	for (int i = 0; i < warpMatrixInfoVec.size(); i++) {
		if (warpMatrixInfoVec[i].groupflag < 0) {
			id = i;
			return false;
		}
	}
	id = -1;
	return true;
}

Mat meanShiftOfWarpMatrix(vector<warpMatrixInfo> &warpMatrixInfoVec, Mat &dood) {
	// mean shift
	int id = 0;
	vector<warpGroupCenter> groupNcenter;
	while (!hasVisitAll_WarpMatrixInfo(warpMatrixInfoVec, id)) {
		if (id < 0) 
			break;
		// set new groupflag
		int c_groupflag = groupNcenter.size();
		// set warpMatrixInfo to presenet current center point info
		warpMatrixInfo center_warp = warpMatrixInfoVec[id];
		warpMatrixInfoVec[id].groupflag = c_groupflag;
		// push element if should in this group
		vector<int> c_Group;
		c_Group.push_back(id);

		double thetaDifferThreshold = 0.3;
		double stopthresh = 0.3;
		while (true) {
			// if (distance between id-n) < radius, then they should in the same group
			for (int n = 0; n < warpMatrixInfoVec.size(); n++) {
				if (warpMatrixInfoVec[n].groupflag < 0) {
					//cout << "center_warp.theta= " << center_warp.theta << ",  warpMatrixInfoVec[" << n << "].theta= " << warpMatrixInfoVec[n].theta << endl;
					if (abs(center_warp.theta - warpMatrixInfoVec[n].theta) < thetaDifferThreshold) {
						warpMatrixInfoVec[n].groupflag = c_groupflag;
						c_Group.push_back(n);
					}
				}
			}

			// reset center point every while
			warpMatrixInfo lastCenter = center_warp;
			center_warp.theta = 0;
			for (int k = 0; k < c_Group.size(); k++) {
				center_warp.theta += warpMatrixInfoVec[c_Group[k]].theta;
			}
			center_warp.theta /= c_Group.size();

			//cout << "lastCenter.theta= " << lastCenter.theta << ", center_warp.theta= " << center_warp.theta << endl;
			// if new (lastCenter-newCenter) < stopthresh, break while -- stop update the center position 
			if (stopthresh > abs(lastCenter.theta - center_warp.theta)) {
				break;
			}
		}

		int merge = -1;
		for (int k = 0; k < groupNcenter.size(); k++) {
			if ((thetaDifferThreshold / 2) > abs(center_warp.theta - groupNcenter[k].theta)) {
				merge = k;
				break;
			}
		}
		if (merge >= 0) { //合併
			for (int k = 0; k < c_Group.size(); k++)
				warpMatrixInfoVec[c_Group[k]].groupflag = merge;
			groupNcenter[merge].group.assign(c_Group.begin(), c_Group.end());

			double g_theta = 0;
			vector<int> *g_group = &groupNcenter[merge].group;
			for (int k = 0; k < g_group->size(); k++) { // 二維資訊要保留 不能單純的distance
				g_theta += warpMatrixInfoVec[g_group->at(k)].theta;
			}
			g_theta /= g_group->size();
		}
		else { //不合併
			warpGroupCenter tmpGC;
			tmpGC.group = c_Group;
			tmpGC.theta = center_warp.theta;
			groupNcenter.push_back(tmpGC);
		}
	}

	int maxI = (groupNcenter.size() > 0) ? 0 : -1;
	//cout << "groupNcenter.size()= " << groupNcenter.size() << ", maxI= " << maxI<< endl;
	for (int m = 0; m < groupNcenter.size(); m++) {
		if (groupNcenter[m].group.size()>groupNcenter[maxI].group.size())
			maxI = m;
		/*cout << "group[" << m << "]= " << groupNcenter[m].group.size() << ", theta= " << groupNcenter[m].theta << endl;
		for (int s = 0; s < groupNcenter[m].group.size(); s++) 
			cout << groupNcenter[m].group[s] << ", ";*/
		/*for (int g = 0; g < groupNcenter[m].group.size(); g++) {
			Mat warpFood = Mat::zeros(inputSize, CV_8UC4);
			warpAffine(dood, warpFood, warpMatrixInfoVec[groupNcenter[m].group[g]].warpMatrix, inputSize);
			imwrite("Result0/" + to_string(m) + "_" + to_string(g) + "_" + to_string(warpMatrixInfoVec[groupNcenter[m].group[g]].theta) + ".png", warpFood);
		}*/
	}

	if (maxI >= 0) {
		if (groupNcenter[maxI].group.size()==1)
			return Mat();

		// get median warpMatrix
		double median_Theta = 0;
		for (int s = 0; s < groupNcenter[maxI].group.size(); s++) {
			median_Theta += warpMatrixInfoVec[groupNcenter[maxI].group[s]].theta;
		}
		median_Theta /= (double)groupNcenter[maxI].group.size();

		Mat mediaWarpMat = warpMatrixInfoVec[groupNcenter[maxI].group[0]].warpMatrix;
		mediaWarpMat.at<double>(0, 0) = cos(median_Theta);
		mediaWarpMat.at<double>(1, 0) = sin(median_Theta);
		mediaWarpMat.at<double>(0, 1) = -mediaWarpMat.at<double>(1, 0);
		mediaWarpMat.at<double>(1, 1) = mediaWarpMat.at<double>(0, 0);

		int medianIdx = groupNcenter[maxI].group[min(groupNcenter[maxI].group.size() - 1, groupNcenter[maxI].group.size() / 2)];
		return mediaWarpMat;
	}
	else
		return Mat();
}

// icp algorithm, stack only outer contour
int doCombination(vector<Point> pointSeq1, vector<Point> pointSeq2, Mat &dood, Mat &resultStack, Mat &contourAlpha, Mat &InnerWarpMat) {
	cout << "------------------------------------------------ doCombination6 " << endl;

	// get norm of pointSeq1 vec
	vector<Point2f> normOfPointSeq1;
	for (int idx1 = 0; idx1 < pointSeq1.size(); idx1++) {
		normOfPointSeq1.push_back(norVec(pointSeq1[(idx1 - 1 + pointSeq1.size()) % pointSeq1.size()], pointSeq1[idx1], pointSeq1[(idx1 + 1) % pointSeq1.size()]));
	}
	//// get dist value from front point of pointSeq1
	//vector<double> distValueOfPointSeq1;
	//for (int idx1 = 1; idx1 < pointSeq1.size(); idx1++) {
	//	distValueOfPointSeq1.push_back(pointDist(pointSeq1[idx1 - 1], pointSeq1[idx1]));
	//	cout << pointSeq1[idx1 - 1] << ", " << pointSeq1[idx1] << endl;
	//}

	// get norm of pointSeq2 vec
	vector<Point2f> normOfPointSeq2;
	for (int idx2 = 0; idx2 < pointSeq2.size(); idx2++) {
		normOfPointSeq2.push_back(norVec(pointSeq2[(idx2 - 1 + pointSeq2.size()) % pointSeq2.size()], pointSeq2[idx2], pointSeq2[(idx2 + 1) % pointSeq2.size()]));
	}

	int combineFAmount = 0;
	int idx1 = 0;
	int endOfIdx1 = pointSeq1.size();
	bool firstTime = true;

	vector<warpMatrixInfo> warpMatrixInfoVec;
	while (idx1 < endOfIdx1) {
		double min_icpError = 1500;
		int min_idx1 = idx1;
		int min_idx2 = 0;
		int min_range = 0;
		Mat min_warpMatrix;

		for (int idx2 = 0; idx2 < pointSeq2.size(); idx2 += 2) {
			int range = max(3, min(pointSeq1.size(), pointSeq2.size())*0.5);
			Mat warpMat;
			double icpError;
			twoSeqPointToWarp(pointSeq1, pointSeq2, idx1, idx2, range, warpMat, icpError);
			// if current icp value is bigger that before, then break
			if (icpError < min_icpError) {
				// push valut to vec to deterMinimum
				min_icpError = icpError;
				min_idx1 = idx1;
				min_idx2 = idx2;
				min_range = range;
				min_warpMatrix = warpMat;
			}
		}

		if (min_range > 0) { // find a candidate
			// if stack outside, then break
			Mat warpFood = Mat::zeros(inputSize, CV_8UC4);
			warpAffine(dood, warpFood, min_warpMatrix, inputSize);

			// if overlap range of the last element  
			if (imageOverlapOutandFront(contourAlpha, resultStack, warpFood)) {
				combineFAmount++;
				vector<Point> new_seqP1;
				vector<Point> add_seqP1; // use only for testing
				vector<Point> new_seqP2;
				vector<Point> new_warp_seqP2;

				// test and write image
				//Mat testImg = warpFood.clone();
				/* ---------------------------- start to decide step size ---------------------------- */
				for (int rr = 0; rr < min_range / 2; rr++) {
					new_seqP1.push_back(pointSeq1[(min_idx1 + rr) % pointSeq1.size()]);
					new_seqP2.push_back(pointSeq2[(min_idx2 + rr) % pointSeq2.size()]);
					new_warp_seqP2.push_back(warpPoint(pointSeq2[(min_idx2 + rr) % pointSeq2.size()], min_warpMatrix));
				}
				for (int rr = min_range / 2; rr < min(pointSeq1.size(), pointSeq2.size()); rr++) {
					// info of pointSeq1
					int c_Idx1 = (min_idx1 + rr) % pointSeq1.size();
					Point p1 = pointSeq1[c_Idx1];
					Point2f normVec1 = normOfPointSeq1[c_Idx1];
					// info of pointSeq2
					int c_Idx2 = (min_idx2 + rr) % pointSeq2.size();
					Point p2 = warpPoint(pointSeq2[c_Idx2], min_warpMatrix);
					Point2f normVec2 = warpNorm(normOfPointSeq2[c_Idx2], min_warpMatrix);
					// calculate cosTheta between normVec1 and normVec2
					double cosTheta = ((normVec1.x * normVec2.x + normVec1.y * normVec2.y) / sqrt(pow(normVec1.x, 2) + pow(normVec1.y, 2) + pow(normVec2.x, 2) + pow(normVec2.y, 2)));
					double currentDist = (pointDist(p2, p1)*(1 - cosTheta)) / cosTheta;
					// draw testing
					//circle(testImg, p1, 2, Scalar(0, 0, 255, 255), 2);
					//circle(testImg, p2, 2, Scalar(0, 255, 0, 255), 2);
					vector<Point> normPoint1 = getNormPoint(p1, normVec1);
					vector<Point> normPoint2 = getNormPoint(p2, normVec2);
					//line(testImg, normPoint1[0], normPoint1[1], Scalar(0, 0, 255, 255), 1, 8);
					//line(testImg, normPoint2[0], normPoint2[1], Scalar(0, 255, 0, 255), 1, 8);
					if (currentDist >= 0 && currentDist < 100) {
						new_seqP1.push_back(p1);
						new_seqP2.push_back(pointSeq2[c_Idx2]);
						new_warp_seqP2.push_back(p2);
						if (rr >= min_range) {
							add_seqP1.push_back(p1);
							//circle(testImg, p1, 1, Scalar(255, 0, 255, 255), 1);
						}
					}
					else
						break;
					//imwrite("Result0/R_" + to_string(c_Idx1) + "_" + to_string(c_Idx2) + "_" + to_string(currentDist) + ".png", testImg);
				}

				if (firstTime) { // if thr first food, determine thr front 
					for (int rr = 0; rr < min(pointSeq1.size(), pointSeq2.size()); rr++) {
						// info of pointSeq1
						int c_Idx1 = (min_idx1 - rr + pointSeq1.size()) % pointSeq1.size();
						Point p1 = pointSeq1[c_Idx1];
						Point2f normVec1 = normOfPointSeq1[c_Idx1];
						// info of pointSeq2
						int c_Idx2 = (min_idx2 - rr + pointSeq2.size()) % pointSeq2.size();
						Point p2 = warpPoint(pointSeq2[c_Idx2], min_warpMatrix);
						Point2f normVec2 = warpNorm(normOfPointSeq2[c_Idx2], min_warpMatrix);
						// calculate cosTheta between normVec1 and normVec2
						double cosTheta = ((normVec1.x * normVec2.x + normVec1.y * normVec2.y) / sqrt(pow(normVec1.x, 2) + pow(normVec1.y, 2) + pow(normVec2.x, 2) + pow(normVec2.y, 2)));
						double currentDist = (pointDist(p2, p1)*(1 - cosTheta)) / cosTheta;
						if (currentDist >= 0 && currentDist < 100) {
							endOfIdx1--;
							new_seqP1.push_back(p1);
							new_warp_seqP2.push_back(p2);
						}
						else
							break;
					}
				}

				/* ---------------------------- end to decide step size ---------------------------- */
				// partial icp to re shift
				vector<Point> newSampleP;
				tmpCompareWithoutDes(false, 5, resultStack.size(), new_seqP1, new_warp_seqP2, Mat(), newSampleP, 1);
				if (firstTime) {
					firstTime = false;
					for (int popCount = 0; popCount < (pointSeq1.size() - endOfIdx1); popCount++)
						newSampleP.pop_back();
				}
				// get warp matrix and warp pointSeq
				solver icpSolver;
				min_warpMatrix = icpSolver.getTwoSetsTransform(new_seqP2, newSampleP).clone();

				// warp to get final image
				Mat newFood = Mat::zeros(inputSize, CV_8UC4);
				warpAffine(dood, newFood, min_warpMatrix, inputSize);
				resultStack = addTransparent(resultStack, newFood);
				
				warpMatrixInfo warpInfo;
				warpInfo.warpMatrix = min_warpMatrix.clone();
				warpInfo.theta = getThetaValue(min_warpMatrix);
				warpInfo.groupflag = -1;
				warpMatrixInfoVec.push_back(warpInfo);

				//cout << "last_startIdx1= " << last_startIdx1 << endl;
				//cout << "last_startIdx1= " << last_startIdx1 << endl;
				//cout << "range= " << (new_seqP1.size() - (min_range / 2)) << endl;
				// walkStep
				int walkStep = (new_seqP1.size() - (min_range / 2));
				idx1 += max(1, walkStep);
				//walkStep += (min_range / 2);
				//system("pause");
				/* ---------------------------- end ---------------------------- */
				//// test and write image
				//Mat wwwarpImg = Mat::zeros(inputSize, CV_8UC4);
				//warpAffine(dood, wwwarpImg, min_warpMatrix, inputSize);
				//// for the first element
				//circle(wwwarpImg, new_seqP1[0], 3, Scalar(0, 0, 255, 255), 1);
				//circle(wwwarpImg, new_seqP1[1], 2, Scalar(0, 0, 255, 255), 1);
				//circle(wwwarpImg, new_warp_seqP2[0], 3, Scalar(0, 255, 0, 255), 1);
				//circle(wwwarpImg, new_warp_seqP2[1], 2, Scalar(0, 255, 0, 255), 1);
				//for (int rr = 2; rr < walkStep; rr++) {
				//	circle(wwwarpImg, new_seqP1[rr], 2, Scalar(0, 0, 255, 255), 2);
				//	circle(wwwarpImg, new_warp_seqP2[rr], 2, Scalar(0, 255, 0, 255), 2);
				//}
				////draw norm between normVec1 and normVec2
				//for (int range = 0; range < walkStep; range++) {
				//	int normIdx1 = (min_idx1 + range) % pointSeq1.size();
				//	int normIdx2 = (min_idx2 + range) % pointSeq2.size();
				//	Point2f normVec1 = normOfPointSeq1[normIdx1];
				//	Point2f normVec2 = warpNorm(normOfPointSeq2[normIdx2], min_warpMatrix);
				//	circle(wwwarpImg, new_seqP1[range], 3, Scalar(0, 0, 255, 255), 1);
				//	circle(wwwarpImg, new_warp_seqP2[range], 3, Scalar(0, 255, 0, 255), 1);
				//	vector<Point> normPoint1 = getNormPoint(new_seqP1[range], normVec1);
				//	vector<Point> normPoint2 = getNormPoint(new_warp_seqP2[range], normVec2);
				//	line(wwwarpImg, normPoint1[0], normPoint1[1], Scalar(0, 0, 255, 255), 1, 8);
				//	line(wwwarpImg, normPoint2[0], normPoint2[1], Scalar(0, 255, 0, 255), 1, 8);
				//}
				//// use for test add point correct or not
				//for (int rr = 0; rr < add_seqP1.size(); rr++) {
				//	circle(wwwarpImg, add_seqP1[rr], 1, Scalar(255, 0, 255, 255), 1);
				//}
				//for (int range = 0; range < add_seqP1.size(); range++) {
				//	int normIdx1 = (min_idx1 + min_range + range) % pointSeq1.size();
				//	Point2f normVec1 = normOfPointSeq1[normIdx1];
				//	circle(wwwarpImg, add_seqP1[range], 3, Scalar(255, 0, 255, 255), 1);
				//	vector<Point> normPoint1 = getNormPoint(add_seqP1[range], normVec1);
				//	line(wwwarpImg, normPoint1[0], normPoint1[1], Scalar(255, 0, 255, 255), 1, 8);
				//}
				//imwrite("Result0/R_" + to_string(min_idx1) + "_" + to_string(min_idx2) + "_" + to_string(min_range) + ".png", wwwarpImg);
				//system("pause");
			}
			else 
				idx1 += 1;
		} 
		else 
			idx1 += 1;
	}
	//vector<Point> new_seqP1;
	//vector<Point> new_seqP2;
	//vector<Point> new_warp_seqP2;
	//// partial icp to re shift
	//int range = min(endOfIdx1 - last_startIdx1, pointSeq2.size());
	//for (int rr = 0; rr < range; rr++) {
	//	new_seqP1.push_back(pointSeq1[(last_startIdx1 + rr) % pointSeq1.size()]);
	//	new_seqP2.push_back(pointSeq2[(last_startIdx2 + rr) % pointSeq2.size()]);
	//	new_warp_seqP2.push_back(warpPoint(pointSeq2[(last_startIdx2 + rr) % pointSeq2.size()], lastWarpMat));
	//}
	//vector<Point> newSampleP;
	//tmpCompareWithoutDes(false, 5, lastResultStack.size(), new_seqP1, new_warp_seqP2, Mat(), newSampleP, 1);
	//// get warp matrix and warp pointSeq
	//solver icpSolver;
	//lastWarpMat = icpSolver.getTwoSetsTransform(new_seqP2, newSampleP).clone();

	//imwrite("Result0/resultStack.png", resultStack);
	InnerWarpMat = meanShiftOfWarpMatrix(warpMatrixInfoVec, dood).clone();

	return combineFAmount;
}

// stack only inner contour, then warp to center
int doCombinationInner(Rect contourRect, Mat &c_Alpha, Mat &contourAlpha, Mat &dood, Mat &resultStack, food &cFood, Point contourCenter) {
	Mat currentStack = Mat::zeros(resultStack.size(), resultStack.type());
	//imwrite("Result0/c_Alpha.png", c_Alpha);

	// food info 
	int widthShiftLimit = cFood.widthShiftLimit;
	int heightShiftLimit = cFood.heightShiftLimit;
	Point lastFP = Point(cFood.rect.x, cFood.rect.y);

	int combineFAmount = 0;
	// set height interval amount
	int H_amount = (contourRect.height - cFood.rect.height) / heightShiftLimit + 1;;
	if (H_amount <= 0) { H_amount = 1; }
	// adjust H_interval
	double H_interval = 0;
	if (H_amount > 1)
		H_interval = (contourRect.height - cFood.rect.height) / (H_amount - 1);
	// adjust remainder size
	int startH = contourRect.height - min(contourRect.height, ((H_amount - 1)*H_interval + cFood.rect.height));
	int acc_H_interval = 0;

	contourRect.y++;
	for (int h = 0; h < H_amount; h++) {
		int c_H_interval = H_interval;
		if (h < startH)
			c_H_interval++;

		Rect c_contourRect = getWidthBoundaryRect(c_Alpha, Rect(0, (contourRect.y + acc_H_interval), c_Alpha.cols, cFood.rect.height));
		// set width interval
		int W_amount = (c_contourRect.width - cFood.rect.width) / widthShiftLimit + 1;
		if (W_amount <= 0) { W_amount = 1; }
		// adjust W_interval
		double W_interval = 0;
		if (W_amount > 1)
			W_interval = (c_contourRect.width - cFood.rect.width) / (W_amount - 1);
		// adjust remainder size
		int startW = c_contourRect.width - min(c_contourRect.width, (W_amount - 1)*W_interval + cFood.rect.width);
		int acc_W_interval = 0;

		c_contourRect.x++;
		for (int w = 0; w < W_amount; w++) {
			int c_W_interval = W_interval;
			if (w < startW)
				c_W_interval++;

			Rect cc_contourRect = getWidthBoundaryRect(c_Alpha, Rect(c_contourRect.x + acc_W_interval, (contourRect.y + acc_H_interval), cFood.rect.width, cFood.rect.height));

			Mat warpFoodAlpha = Mat::zeros(c_Alpha.size(), c_Alpha.type());
			Mat resultTmp = currentStack.clone();
			Point currentShift = Point(c_contourRect.x + acc_W_interval, c_contourRect.y) - lastFP;

			int lapCount = 0;
			Mat resultFoodMat = shiftMat(dood, currentShift.x, currentShift.y).clone();

			resultTmp = addTransparent(resultTmp, resultFoodMat);

			if ((h > 0 && h < H_amount && w>0 && w < W_amount) || imageOverlapOutandFront(contourAlpha, resultStack, resultFoodMat)) {
				combineFAmount++;
				currentStack = resultTmp;
			}
			acc_W_interval += c_W_interval;

			//imwrite("Result0/resultStack.png", resultStack);
			//imwrite("Result0/contourAlpha.png", contourAlpha);
		}
		acc_H_interval += c_H_interval;
	}

	if (combineFAmount > 0) {
		// move resiltStackStacl to contour center
		Point foodStackCenter = getCenterPointFromImg(currentStack);
		Point shiftCenter = contourCenter - foodStackCenter;
		Mat warpCurrentStack = shiftMat(currentStack, shiftCenter.x, shiftCenter.y);
		resultStack = addTransparent(resultStack, warpCurrentStack);
	}
	return combineFAmount;
}

// compare contour with descriptor and sample point
void doCompare(bool userMove, fragList& pairSeq, leafNode &cNode, int contourIdx, food &currentFood) {
	string foodName = currentFood.name;
	vector<food> tmpFVec = getFoodVecFromStr(foodName);
	vector<vector<Point> > *sampleP_food = &samplepointsOfFood;
	vector<vector<Point> > *pointsOfFood = &PointsOfFoodOri;

	// contour info
	int contourSize = cNode.samplepointsOfDraw[contourIdx].size();
	int contourCArea = contourArea(cNode.samplepointsOfDraw[contourIdx]);
	vector<Point> *pointSeq1 = &cNode.samplepointsOfDraw[contourIdx];
	Rect contourBoundary = cNode.boundaryOfContour[contourIdx];
	Point pointCenter = cNode.centerOfContour[contourIdx];
	Size imgSize = cNode.inputSize;

	fragList tmpFragList;
	for (int f = 0; f < tmpFVec.size(); f++) {
		// food info 
		int foodImgIdx = tmpFVec[f].orderIdx;
		int foodSize = sampleP_food->at(foodImgIdx).size();
		int foodContourArea = contourArea(sampleP_food->at(foodImgIdx));
		int heightShiftLimit = tmpFVec[f].heightShiftLimit;
		vector<Point> pointSeq2 = sampleP_food->at(foodImgIdx);
		vector<Point> pointSeq2Ori = pointsOfFood->at(foodImgIdx);
		Mat dood = totalFoodImg[totalFood[foodImgIdx].fileName].clone();
		Point foodCenter = getCenterPoint(pointSeq2);
		cv::resize(dood, dood, cNode.inputSize);
		//// test contour if their samplepoint is in the same direction
		//Mat testContour0 = Mat::zeros(cNode.inputSize, CV_8UC4);
		//drawContours(testContour0, vector<vector<Point>>(1, *pointSeq1), 0, Scalar(255, 0, 255, 255), 1, 8);
		//circle(testContour0, pointSeq1->at(0), 3, Scalar(255, 0, 255, 255), 2);
		//circle(testContour0, pointSeq1->at(3), 2, Scalar(255, 0, 255, 255), 2);
		//imwrite("testContour0_" + to_string(contourIdx) + ".png", testContour0);
		//Mat testContour = Mat::zeros(cNode.inputSize, CV_8UC4);
		//drawContours(testContour, vector<vector<Point>>(1, pointSeq2), 0, Scalar(255, 255, 0, 255), 1, 8);
		//circle(testContour, pointSeq2[0], 3, Scalar(255, 255, 0, 255), 2);
		//circle(testContour, pointSeq2[3], 2, Scalar(255, 255, 0, 255), 2);
		//imwrite("testContour_" + to_string(foodImgIdx) + ".png", testContour);

		if (/*foodImgIdx == 26 && */true && (userMove || cNode.stackOfContour[contourIdx] || totalFood[tmpFVec[f].orderIdx].stackable)) {
			if (foodImgIdx == 76) {
				cout << "doGridCut ---------------------------- food= " << foodImgIdx << ", " << tmpFVec[f].name << endl;
				comp compDes(imgSize, *pointSeq1, contourIdx, foodImgIdx, dood);
				frag tmpFrag = compDes.fragList2()[0];
				tmpFrag.fAmount = 1;
				tmpFragList.Element.push_back(tmpFrag);
			}
			else if (foodImgIdx == 103 || foodImgIdx == 109 || foodImgIdx == 3) {
				cout << "doGridCut ---------------------------- food= " << foodImgIdx << ", " << tmpFVec[f].name << endl;
				vector<Point> pointSeq1FullTmp;
				Mat1b mask(imgSize, uchar(0)); // Black initialized mask, same size as 'frame'
				ellipse(mask, pointCenter, Size(contourBoundary.width / 2, contourBoundary.height / 2), 0, 0, 360, Scalar(255), CV_FILLED, 1); // Draw white circle on mask
				findNonZero(mask, pointSeq1FullTmp); // Find non zero points on mask, and put them in 'allpoints'
				descri descriUser(pointSeq1FullTmp, 0, 1);
				comp compDes(imgSize, descriUser.sampleResult(), contourIdx, foodImgIdx, dood);

				// use icp if it is rotated
				// food different orientation info
				Mat finalMat = Mat::zeros(Size(3, 2), CV_64F);
				vector<Point> newSampleP = descriUser.sampleResult();
				double minIcpError = tmpCompareWithoutDes(true, 20, cNode.inputSize, *pointSeq1, descriUser.sampleResult(), finalMat, newSampleP, 1);

				Mat newFood = Mat::zeros(imgSize, CV_8UC4);
				warpAffine(compDes.fragList2()[0].warpImg, newFood, finalMat, newFood.size());
				compDes.fragList2()[0].warpImg = newFood.clone();

				frag tmpFrag = compDes.fragList2()[0];
				tmpFrag.fAmount = 1;
				tmpFragList.Element.push_back(tmpFrag);
			}
			else {
				// food size is similar as contour, use icp
				int combineFAmount = 0;
				if (contourCArea > foodContourArea*1.7) { //// food is far smaller than contour, use combination
					cout << "combination -------------------- food= " << foodImgIdx << ", " << tmpFVec[f].name << endl;
					Mat contourAlpha = Mat::zeros(cNode.inputSize, CV_8UC1);
					drawContours(contourAlpha, vector<vector<Point>>(1, *pointSeq1), 0, Scalar(255), CV_FILLED);
					Mat contourAlphaOri = contourAlpha.clone();
					Mat resultStack = Mat::zeros(cNode.inputSize, CV_8UC4);
					Mat InnerWarpMat;
					combineFAmount += doCombination(*pointSeq1, pointSeq2, dood, resultStack, contourAlpha, InnerWarpMat);

					// find the main warpMatrix of outer food combination
					Mat warpDood;
					vector<Point> newWarpFood;
					if (InnerWarpMat.size() != Size(0, 0)) {
						warpDood = Mat::zeros(dood.size(), dood.type());
						warpAffine(dood, warpDood, InnerWarpMat, dood.size());
						newWarpFood = getWholeContour(warpDood);
					}
					else {
						warpDood = dood.clone();
						newWarpFood = pointSeq2;
					}

					if (newWarpFood.size() > 0) {
						// set food overlap info
						food cFood;
						getOverlapRatio(cFood, newWarpFood);
						MatACutMatB(contourBoundary, contourAlpha, resultStack);
						vector<Point> newSeqContour = getContourBiggerThanSize(contourAlpha, 0.5*foodContourArea, pointCenter);

						// stack front top left to bottom right
						if (newSeqContour.size() > 0) {
							Rect c_Rect = getBoundaryRectByPoint(newSeqContour, cNode.inputSize);
							Mat c_Alpha = Mat::zeros(cNode.inputSize, CV_8UC1);
							drawContours(c_Alpha, vector<vector<Point>>(1, newSeqContour), 0, Scalar(255), CV_FILLED);
							combineFAmount += doCombinationInner(c_Rect, c_Alpha, contourAlphaOri, warpDood, resultStack, cFood, pointCenter);
						}
					}

					if (combineFAmount > 1) {
						// shift to center
						//Point centerOfResult = getCenterPointFromImg(resultStack);
						//Mat trans_mat = (Mat_<double>(2, 3) << 1, 0, pointCenter.x - centerOfResult.x, 0, 1, pointCenter.y - centerOfResult.y);
						//warpAffine(resultStack, resultStack, trans_mat, resultStack.size());
						frag tmpFrag;
						tmpFrag.setInfo(0, 0, 0, 0, contourIdx, foodImgIdx, combineFAmount, resultStack.clone());
						tmpFragList.Element.push_back(tmpFrag);
						//imwrite("Result0/" + to_string(foodImgIdx) + "_" + to_string(combineFAmount) + ".png", resultStack);
					}
				}

				if (combineFAmount == 0 && contourCArea * 3 > foodContourArea) {
					cout << "icp ---------------------------- food= " << foodImgIdx << ", " << tmpFVec[f].name << endl;
					//icp
					vector<float> rotateAngle = { 0, 90, 180, 270 };
					vector<Mat> rotateMat;
					vector<vector<Point> > sampleP_orientation = reScaleFood2(dood, imgSize, pointSeq2, 1, rotateAngle, rotateMat); // change pointSeq2 position with scale
						
					frag fragMin;
					fragMin.setInfo(0, 0, 0, 0, contourIdx, foodImgIdx, 1, Mat());
					double minError = 99999;
					for (int ori = 0; ori < sampleP_orientation.size(); ori++) {
						// food different orientation info
						Mat finalMat = Mat::zeros(Size(3, 2), CV_64F);
						vector<Point> newSampleP = sampleP_orientation[ori];
						double minIcpError = tmpCompareWithoutDes(true, 20, imgSize, *pointSeq1, sampleP_orientation[ori], finalMat, newSampleP, 1);
						Mat newFood = Mat::zeros(imgSize, CV_8UC4);
						warpAffine(rotateMat[ori], newFood, finalMat, imgSize);

						if (ori == 0 || minIcpError < minError) {
							minError = minIcpError;
							fragMin.warpImg = newFood.clone();
							fragMin.icpVarianceError = minIcpError;
							//imwrite("Result0/" + to_string(foodImgIdx) + "_" + to_string(minIcpError) + ".png", newFood);
						}
					}
					tmpFragList.Element.push_back(fragMin);
				}
			}

				
		}

	}

	// set error to all candidate
	for (int k = 0; k < tmpFragList.Element.size(); k++) {
		double errorBoundary = 0.6;
		setAllError(tmpFragList.Element[k], cNode, contourIdx);
		tmpFragList.Element[k].scaleRatio = cNode.scaleRatio;

		//imwrite("Result0/" + to_string(foodImgIdx) + "_" + to_string(tmpFragList.Element[k].fAmount) + "_" + to_string(tmpFragList.Element[k].icpError) + "_" + to_string(tmpFragList.Element[k].iError) + ".png", tmpFragList.Element[k].warpImg);
	}
	if (tmpFragList.Element.size()>0) {
		sort(tmpFragList.Element.begin(), tmpFragList.Element.end(), compareWithiError);
		pairSeq.Element.push_back(tmpFragList.Element[0]);
	}
	sort(pairSeq.Element.begin(), pairSeq.Element.end(), compareWithiError);

} 

/* --------------------------------End-------------------------------- */
void test_contourMat(leafNode &cNode) {
	RNG rng(12345);
	vector<Scalar> colorVec;
	for (int i = 0; i < cNode.samplepointsOfDrawOri.size(); i++) {
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		colorVec.push_back(color);
	}

	Mat imageTmp = Mat::zeros(cNode.contourMat.size(), CV_8UC3);
	for (int i = 0; i < cNode.contourMat.cols; i++) {
		for (int j = 0; j < cNode.contourMat.rows; j++) {
			int colorIdx = cNode.contourMat.at<int>(i, j);
			if (colorIdx > 0)
				circle(imageTmp, Point(j, i), 1, colorVec[colorIdx], CV_FILLED, 1); // Draw white circle on mask
			else
				circle(imageTmp, Point(j, i), 1, Scalar(0, 0, 0), CV_FILLED, 1); // Draw white circle on mask
		}
	}
	imshow("contourMat.png", imageTmp);
}

void test_icp() {
	Mat userImage0 = imread("t1.png", -1);
	Mat userImage1 = imread("t0.png", -1);
	vector<vector<Point> > userDrawContours0;
	vector<vector<Point> > userDrawContours1;
	vector<Vec4i> hierarchy0;
	vector<Vec4i> hierarchy1;
	Mat cannyColor0 = cannyThreeCh(userImage0, true);
	Mat cannyColor1 = cannyThreeCh(userImage1, true);
	findContours(cannyColor0.clone(), userDrawContours0, hierarchy0, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0));
	findContours(cannyColor1.clone(), userDrawContours1, hierarchy1, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0));

	vector<Point> sourceP;
	vector<Point> targetP;

	Mat imggg = Mat::zeros(userImage0.size(), CV_8UC4);
	for (int i = userDrawContours0.size() - 1; i >= 0; i--) {
		double cArea = contourArea(userDrawContours0[i], false) / userDrawContours0[i].size();
		if (cArea > 2 && hierarchy0[i][3] != -1) {
			descri descriUser(userDrawContours0[i], 0, 1);
			sourceP = descriUser.sampleResult();
		}
	}
	for (int i = userDrawContours1.size() - 1; i >= 0; i--) {
		double cArea = contourArea(userDrawContours1[i], false) / userDrawContours1[i].size();
		if (cArea > 2 && hierarchy1[i][3] != -1) {
			descri descriUser(userDrawContours1[i], 0, 1);
			targetP = descriUser.sampleResult();
		}
	}

	drawContours(imggg, vector<vector<Point>>(1, sourceP), 0, Scalar(0, 0, 255, 255), 2, 8);
	imshow("sourceP.png", imggg);
	drawContours(imggg, vector<vector<Point>>(1, targetP), 0, Scalar(0, 255, 0, 255), 2, 8);
	imshow("targetP.png", imggg);

	Mat finalMat = Mat::zeros(Size(3, 2), CV_64F);
	vector<Point> newSampleP1 = sourceP;
	leafNode cNode;
	cNode.inputSize = userImage0.size();
	double minIcpError1 = tmpCompareWithoutDes(true, 20, cNode.inputSize, targetP, sourceP, finalMat, newSampleP1, 0);
}