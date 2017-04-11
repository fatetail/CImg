#pragma once
#include<iostream>
#include<CImg.h>

using namespace std;
using namespace cimg_library;

class ColorTransfer {
private:
	CImg<float> src;  //source image
	CImg<float> dest; //destination image
	CImg<float> result; //result image
public:
	ColorTransfer(CImg<float> srcImg, CImg<float> destImg);
	void Transfer();
	
};