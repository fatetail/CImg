#include"Hough.h"
#include<string>
using namespace std;
using namespace cimg_library;

int main() {
	string infile = "input15.bmp";
	CImg<float> src(infile.c_str());
	Hough h(src, 5.5f, 30, 1000, 200);
	h.HoughProcess();
	return 0;


}