#pragma once
#include<CImg.h>
#include<iostream>
#include<vector>
#include<algorithm>
using namespace cimg_library;
using namespace std;

struct Point {
	double x, y;
	int cnt;
	Point(double _x, double _y, int _cnt) :x(_x), y(_y), cnt(_cnt) {}
};

struct Line {
	double k, m;
	Line(double _k, double _m) : k(_k), m(_m) {}
};

struct RecLine {
	int index;
	double dis;
	RecLine(int _index, int _dis):index(_index),dis(_dis){}
};

class Hough {
private:
	CImg<float> src;   // Source Image
	CImg<float> colorsrc; // Three-channel source image
	CImg<float> blurred_img; //Blurred Image
	CImg<float> out_gradient; //gradient Image
	CImg<float> houghspace; // Hough space
	CImg<float> houghresult;// Hough result
	CImg<float> transform1;  // The transform
	CImg<float> transform2;
	CImg<float> AFour;


	vector<Point> peaks; // peaks in Hough space
	vector<Line> lines; // line in Image
	vector<Point> intersections; // intersections of lines
	vector<Point> RectPoints;
	vector<Point> destRectPoints;


	double sigma;
	double gradient_threshold;
	double vote_threshold;
	double peak_dis; // peak_dis
public:
	Hough(CImg<float> srcImg, double sigma, double gra_threshold, double vote_threshold, double peak_dis);
	void HoughProcess();
	void Blur();
	void Gradient();
	void findPeaks();
	void findLines();
	void findFourPoints();
	void findTransform();
	void warping();
	int getRangeX(int theta, int p, int y);
	int getRangeY(int theta, int p, int x);
};