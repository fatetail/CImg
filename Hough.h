#pragma once
#include<CImg.h>
#include<iostream>
#include<vector>
using namespace cimg_library;
using namespace std;

struct Point {
	int x, y, cnt;
	Point(int _x, int _y, int _cnt) :x(_x) , y(_y) , cnt(_cnt) {}
};

struct Line {
	double k, m;
	Line(double _k, double _m): k(_k),m(_m){}
};
class Hough {
private:
	CImg<float> src;   // Source Image
	CImg<float> blurred_img; //Blurred Image
	CImg<float> out_gradient; //gradient Image
	CImg<float> houghspace; // Hough space
	CImg<float> result;
	vector<Point> peaks; // peaks in Hough space
	vector<Line> lines; // line in Image
	vector<Point> intersections; // intersections of lines
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
	int getRangeX(int theta, int p, int y);
	int getRangeY(int theta, int p, int x);
};