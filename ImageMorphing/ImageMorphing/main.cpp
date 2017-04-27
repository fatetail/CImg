#include"FaceMorphing.h"
#include<fstream>
#include<sstream>
#include<string>
int main() {
	CImg<float> src, dst;
	src.load_bmp("1.bmp");
	dst.load_bmp("2.bmp");

	vector<Point> src_points;
	vector<Point> dst_points;
	vector<triangle> src_tri;
	vector<triangle> dst_tri;

	/*Read source points coordinates*/
	ifstream infile_src_points;
	infile_src_points.open("srcpoints.txt");
	if (!infile_src_points.is_open()) {
		cout << "error read srcpoints.txt" << endl;
		return 0;
	}
	string s;
	int x, y;
	while (getline(infile_src_points, s)) {
		stringstream ss(s);
		ss >> x >> y;
		Point p(x, y);
		src_points.push_back(p);
	}

	/*Read destination points coordinates*/
	ifstream infile_dst_points;
	infile_dst_points.open("dstpoints.txt");
	if (!infile_dst_points.is_open()) {
		cout << "error read dstpoints.txt" << endl;
		return 0;
	}
	string s2;
	int x2, y2;
	while (getline(infile_dst_points, s2)) {
		stringstream ss(s2);
		ss >> x2 >> y2;
		Point p(x2, y2);
		dst_points.push_back(p);
	}

	/*Read the indexs of triangles points*/
	ifstream infile_tri;
	infile_tri.open("triangle.txt");
	if (!infile_tri.is_open()) {
		cout << "error read triangle.txt" << endl;
		return 0;
	}

	vector<vector<int>> index;
	string s3;
	int index1, index2, index3;
	while (getline(infile_tri, s3)) {
		vector<int> tempindex;
		stringstream ss(s3);
		ss >> index1 >> index2 >> index3;
		tempindex.push_back(index1 - 1);
		tempindex.push_back(index2 - 1);
		tempindex.push_back(index3 - 1);
		index.push_back(tempindex);
	}


	FaceMorphing fm(src,dst, src_points, dst_points, index, 12);
	fm.run();
	return 0;

}