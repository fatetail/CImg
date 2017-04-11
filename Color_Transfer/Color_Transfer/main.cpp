#include "ColorTransfer.h"

int main() {
	CImg<float> src, dest;
	src.load_bmp("20.bmp");
	dest.load_bmp("19.bmp");
	ColorTransfer ct(src, dest);
	ct.Transfer();
	return 0;
}