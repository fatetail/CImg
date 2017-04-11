
#include"ColorTransfer.h"

ColorTransfer::ColorTransfer(CImg<float> SrcImg, CImg<float> DestImg) {
	src = SrcImg;
	dest = DestImg;
}

void ColorTransfer::Transfer() {
	int width = src._width;
	int height = src._height;

	CImg<float> result(width, height, 1, 3, 0);

	CImg<float> src_lab = src.get_RGBtoLab();
	CImg<float> dest_lab = dest.get_RGBtoLab();

	//Get different channels of Lab image
	CImg<float> src_l = src_lab.get_channel(0);
	CImg<float> src_a = src_lab.get_channel(1);
	CImg<float> src_b = src_lab.get_channel(2);
	CImg<float> dest_l = dest_lab.get_channel(0);
	CImg<float> dest_a = dest_lab.get_channel(1);
	CImg<float> dest_b = dest_lab.get_channel(2);

	//Computer different channels mean and variance
	float src_lmean = src_l.mean();
	float src_amean = src_a.mean();
	float src_bmean = src_b.mean();
	float dest_lmean = dest_l.mean();
	float dest_amean = dest_a.mean();
	float dest_bmean = dest_b.mean();

	float src_lvar = src_l.variance(0);
	float src_avar = src_a.variance(0);
	float src_bvar = src_b.variance(0);
	float dest_lvar = dest_l.variance(0);
	float dest_avar = dest_a.variance(0);
	float dest_bvar = dest_b.variance(0);

	//Use the paper's formula
	cimg_forXY(src_l, i, j) {
		result(i, j, 0) = (sqrtf(dest_lvar) / sqrtf(src_lvar))*(src_l(i, j) - src_lmean) + dest_lmean;
		result(i, j, 1) = (sqrtf(dest_avar) / sqrtf(src_avar))*(src_a(i, j) - src_amean) + dest_amean;
		result(i, j, 2) = (sqrtf(dest_bvar) / sqrtf(dest_bvar))*(src_b(i, j) - src_bmean) + dest_bmean;
	}
	result = result.get_LabtoRGB();
	result.save("result10.bmp");

}