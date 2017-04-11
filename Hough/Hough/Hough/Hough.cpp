#include"Hough.h"

Hough::Hough(CImg<float> srcImg, double sigma, double gra_threshold, double vot_threshold, double peak_dis) {
	result = srcImg;
	src = srcImg.get_RGBtoGray();
	this->sigma = sigma;
	this->gradient_threshold = gra_threshold;
	this->vote_threshold = vot_threshold;
	this->peak_dis = peak_dis;
}
void Hough::HoughProcess() {
	Blur();
	Gradient();
	findPeaks();
	findLines();
}
void Hough::Blur() {
	blurred_img = src.get_blur(sigma); // Gauss smooth
}

void Hough::Gradient() {
	out_gradient = src;
	out_gradient.fill(0.0f);
	//计算sobel算子
	CImg<float> sobelx(3, 3, 1, 1, 0);
	CImg<float> sobely(3, 3, 1, 1, 0);
	sobelx(0, 0) = -1, sobely(0, 0) = 1;
	sobelx(0, 1) = 0, sobely(0, 1) = 2;
	sobelx(0, 2) = 1, sobely(0, 2) = 1;
	sobelx(1, 0) = -2, sobely(1, 0) = 0;
	sobelx(1, 1) = 0, sobely(1, 1) = 0;
	sobelx(1, 2) = 2, sobely(1, 2) = 0;
	sobelx(2, 0) = -1, sobely(2, 0) = -1;
	sobelx(2, 1) = 0, sobely(2, 1) = -2;
	sobelx(2, 2) = 1, sobely(2, 2) = -1;

	CImg<float> gradient_x = blurred_img;
	CImg<float> gradient_y = blurred_img;

	gradient_x = gradient_x.get_convolve(sobelx); //calculate the gradient for x and y
	gradient_y = gradient_y.get_convolve(sobely);

	int maxp = (int)sqrt(out_gradient._width*out_gradient._width + out_gradient._height*out_gradient._height);
	CImg<float> hough_space(360, maxp, 1, 1, 0); // initial hough space


	cimg_forXY(out_gradient, i, j) {
		double grad = sqrt(gradient_x(i, j)*gradient_x(i, j) + gradient_y(i, j)*gradient_y(i, j));
		if (grad > gradient_threshold) {
			out_gradient(i, j) = grad;
			cimg_forX(hough_space, angle) {
				double theta = ((double)angle*cimg::PI) / 180;
				int p = (int)(i*cos(theta) + j*sin(theta));
				if (p >= 0 && p < maxp) {
					hough_space(angle, p)++;
				}
			}
		}
	}
	houghspace = hough_space;
}

int Hough::getRangeX(int theta, int p, int y) {
	double angel = (double)theta*cimg::PI / 180;
	return ((double)p / (cos(angel))) - double(y)*(tan(angel));
}

int Hough::getRangeY(int theta, int p, int x) {
	double angel = (double)theta*cimg::PI / 180;
	return ((double)p / (sin(angel))) - double(x)*(1 / tan(angel));
}

void Hough::findPeaks() {
	peaks.clear(); // clear the vector
	cimg_forXY(houghspace, theta, p) {
		if (houghspace(theta, p) > vote_threshold) {
			bool flag = true;
			const int xmin = 0;
			const int xmax = src._width - 1;
			const int y0 = getRangeY(theta, p, xmin);
			const int y1 = getRangeY(theta, p, xmax);

			const int ymin = 0;
			const int ymax = src._height - 1;
			const int x0 = getRangeX(theta, p, ymin);
			const int x1 = getRangeX(theta, p, ymax);

			if (x0 >= xmin && x0 <= xmax || x1 >= xmin && x1 <= xmax ||
				y0 >= ymin && y0 <= ymax || y1 >= ymin && y1 <= ymax) {
				for (int i = 0; i < peaks.size(); i++) {
					if (sqrt((peaks[i].x - theta)*(peaks[i].x - theta) + (peaks[i].y - p)*(peaks[i].y - p)) < peak_dis) {
						flag = false;
						if (peaks[i].cnt < houghspace(theta, p)) {
							Point temp(theta, p, houghspace(theta, p));
							peaks[i] = temp;
						}
					}

				}
				if (flag) {
					Point temp(theta, p, houghspace(theta, p));
					peaks.push_back(temp);
				}
			}
		}
	}
}

void Hough::findLines() {
	lines.clear();

	for (int i = 0; i < peaks.size(); i++) {
		double theta = double(peaks[i].x)*cimg::PI / 180;
		double k = -cos(theta) / sin(theta);
		double m = double(peaks[i].y) / sin(theta);
		Line templine(k, m);
		lines.push_back(templine);
		cout << "直线方程为：y = " << k << " x + " << m << endl;
	}
	cout << endl << endl;
	intersections.clear();

	for (int i = 0; i < lines.size(); i++) {
		for (int j = i + 1; j < lines.size(); j++) {
			double k0 = lines[i].k;
			double k1 = lines[j].k;
			double m0 = lines[i].m;
			double m1 = lines[j].m;

			double x = (m1 - m0) / (k0 - k1);
			double y = (k0*m1 - k1*m0) / (k0 - k1);

			if (x >= 0 && x < src._width && y >= 0 && y < src._height) {
				Point tempPoint(x, y, 0);
				intersections.push_back(tempPoint);
				cout << "直线的交点为： x = " << x << ", y = " << y << endl;
			}
		}
	}
	cout << endl << endl;

	const double lines_color[] = { 255,0,0 };
	const double intersections_color[] = { 0,0,255 };

	int xmin = 0, ymin = 0;
	int xmax = src._width-1, ymax = src._height-1;
	for (int i = 0; i < lines.size(); i++) {
		const int x0 = (double)(ymin - lines[i].m) / lines[i].k;
		const int x1 = (double)(ymax - lines[i].m) / lines[i].k;
		const int y0 = xmin*lines[i].k + lines[i].m;
		const int y1 = xmax*lines[i].k + lines[i].m;

		if (abs(lines[i].k) > 1) {
			result.draw_line(x0, ymin, x1, ymax, lines_color);
		}
		else {
			result.draw_line(xmin, y0, xmax, y1, lines_color);
		}
	}



	for (int i = 0; i < intersections.size(); i++) {
		result.draw_circle(intersections[i].x, intersections[i].y, 70, intersections_color);
	}
	result.save("result3.bmp");
	result.display();
}





