#pragma once
#include<iostream>
#include<CImg.h>
#include<vector>
#include<string>
using namespace std;
using namespace cimg_library;

struct Point {
	int x;
	int y;
	Point() {}
	Point(int _x, int _y):x(_x),y(_y) {}
};
struct triangle {
	Point p1, p2, p3;
	triangle(Point _p1, Point _p2, Point _p3):p1(_p1),p2(_p2),p3(_p3) {}
};


class Vector2 {
private:
	Point start;
	Point end;
	int x;
	int y;
public:
	Vector2(Point _start, Point _end);
	float Dot(Vector2 another);
};

class FaceMorphing {
private:
	vector<Point> src_points;
	vector<Point> dst_points;
	vector<vector<Point>> mid_points;  // Store every frame's middle image points

	vector<vector<int>> index;
	vector<triangle> src_triangle_list;
	vector<triangle> dst_triangle_list;
	vector<vector<triangle>> mid_triangle_list; // Store every frame's middle image triangles

	CImg<float> src;
	CImg<float> dst;
	CImgList<float> result;   // The result imagelist

	int framecnt;   // The count of frame
public:
	FaceMorphing(CImg<float> _src, CImg<float> _dst, vector<Point> srcpoints, vector<Point> dstpoints, vector<vector<int>> index, int framecnt);
	CImg<float> find_transform(triangle before, triangle after);
	bool is_intriangle(Point P, triangle tri);
	void morphing();
	void run();
};