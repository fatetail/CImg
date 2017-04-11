#include"Hough.h"
#include<string>
using namespace std;
using namespace cimg_library;

int main() {
	string infile = "input1.bmp";
	CImg<float> src(infile.c_str());
	Hough h(src, 10.5f, 30, 1000, 60);
	h.HoughProcess();
	return 0;


}