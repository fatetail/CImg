#include"Hough.h"
bool sortbydis(RecLine a, RecLine b) {
	return a.dis < b.dis;
}
Hough::Hough(CImg<float> srcImg, double sigma, double gra_threshold, double vot_threshold, double peak_dis) {
	colorsrc = srcImg;
	houghresult = srcImg;
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
	findFourPoints();
	findTransform();
	warping();
}
void Hough::Blur() {
	blurred_img = src.get_blur(sigma);
}

void Hough::Gradient() {
	out_gradient = src;
	out_gradient.fill(0.0f);
	CImg<float> temp;
	CImgList<float> gradient =  blurred_img.get_gradient("xy",2);
	//计算sobel算子

	CImg<float> gradient_x = gradient[0];
	CImg<float> gradient_y = gradient[1];


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
	int xmax = src._width - 1, ymax = src._height - 1;
	for (int i = 0; i < lines.size(); i++) {
		const int x0 = (double)(ymin - lines[i].m) / lines[i].k;
		const int x1 = (double)(ymax - lines[i].m) / lines[i].k;
		const int y0 = xmin*lines[i].k + lines[i].m;
		const int y1 = xmax*lines[i].k + lines[i].m;

		if (abs(lines[i].k) > 1) {
			houghresult.draw_line(x0, ymin, x1, ymax, lines_color);
		}
		else {
			houghresult.draw_line(xmin, y0, xmax, y1, lines_color);
		}
	}



	for (int i = 0; i < intersections.size(); i++) {
		houghresult.draw_circle(intersections[i].x, intersections[i].y, 70, intersections_color);
	}
	houghresult.save("result1.bmp");
	houghresult.display();
}

void Hough::findFourPoints() {
	
	vector<RecLine> Recline;
	// Calculate the four points' positions
	for (int i = 1; i < intersections.size(); i++) {
		double sub_x = intersections[i].x - intersections[0].x;
		double sub_y = intersections[i].y - intersections[0].y;
		double dis = sqrt(sub_x*sub_x + sub_y*sub_y);
		RecLine newrecline(i, dis);
		Recline.push_back(newrecline);
	}
	// The point in the diagoal has the largest distance.
	sort(Recline.begin(), Recline.end(), sortbydis);
	RectPoints.clear();
	RectPoints.push_back(intersections[0]);
	for (int i = 0; i < Recline.size(); i++) {
		RectPoints.push_back(intersections[Recline[i].index]);
	}
}

void Hough::findTransform() {
	CImg<float> tempafour(1240, 1754,1, 3, 0);
	AFour = tempafour;
	// Calculate a,b,c,d,e,f in transform matrix
	Point tempPoint(0, 0, 0);
	destRectPoints.push_back(tempPoint);
	tempPoint.x = tempafour._width - 1;
	destRectPoints.push_back(tempPoint);
	tempPoint.x = 0;
	tempPoint.y = tempafour._height - 1;
	destRectPoints.push_back(tempPoint);
	tempPoint.x = tempafour._width - 1;
	destRectPoints.push_back(tempPoint);

	CImg<float> y1(1, 3, 1, 1, 0);
	CImg<float> y2(1, 3, 1, 1, 0);
	CImg<float> y3(1, 3, 1, 1, 0);
	CImg<float> y4(1, 3, 1, 1, 0);
	CImg<float> coff1(1, 3, 1, 1, 0);
	CImg<float> coff2(1, 3, 1, 1, 0);
	CImg<float> coff3(1, 3, 1, 1, 0);
	CImg<float> coff4(1, 3, 1, 1, 0);
	CImg<float> A1(3, 3, 1, 1, 1);
	CImg<float> A2(3, 3, 1, 1, 1);
	CImg<float> A3(3, 3, 1, 1, 1);
	CImg<float> A4(3, 3, 1, 1, 1);


	for (int i = 0; i < 3; i++) {
		A1(0, i) = destRectPoints[i].x;
		A2(0, i) = destRectPoints[i].x;
		A1(1, i) = destRectPoints[i].y;
		A2(1, i) = destRectPoints[i].y;

		y1(0,i) = RectPoints[i].x;
		y2(0,i) = RectPoints[i].y;
	}
	for (int i = 3; i > 0; i--) {
		A3(0, 3-i) = destRectPoints[i].x;
		A4(0, 3-i) = destRectPoints[i].x;
		A3(1, 3-i) = destRectPoints[i].y;
		A4(1, 3-i) = destRectPoints[i].y;

		y3(0, 3-i) = RectPoints[i].x;
		y4(0, 3-i) = RectPoints[i].y;
	}

	coff1 = y1.solve(A1);
	coff2 = y2.solve(A2);
	coff3 = y3.solve(A3);
	coff4 = y4.solve(A4);
	CImg<float> temptransform1(3, 3, 1, 1, 0);
	for (int i = 0; i < 3; i++) {
		if (i == 0) {
			for (int j = 0; j < 3; j++) {
				temptransform1(j, i) = coff1(0,j);
			}
		}
		else if (i == 1) {
			for (int j = 0; j < 3; j++) {
				temptransform1(j, i) = coff2(0,j);
			}
		}
		else {
			temptransform1(0, 2) = 0;
			temptransform1(1, 2) = 0;
			temptransform1(2, 2) = 1;
		}
	}
	CImg<float> temptransform2(3, 3, 1, 1, 0);
	for (int i = 0; i < 3; i++) {
		if (i == 0) {
			for (int j = 0; j < 3; j++) {
				temptransform2(j, i) = coff3(0, j);
			}
		}
		else if (i == 1) {
			for (int j = 0; j < 3; j++) {
				temptransform2(j, i) = coff4(0, j);
			}
		}
		else {
			temptransform2(0, 2) = 0;
			temptransform2(1, 2) = 0;
			temptransform2(2, 2) = 1;
		}
	}
	transform1 = temptransform1;
	transform2 = temptransform2;

}

void Hough::warping() {
	CImg<float> y(1, 2, 1, 1, 0);
	CImg<float> A(2, 2, 1, 1, 1);
	A(0, 0) = 0;
	A(0, 1) = AFour._width - 1;
	y(0, 0) = AFour._height - 1;
	y(0, 1) = 0;

	CImg<float> coff(1, 2, 1, 1, 0);
	coff= y.solve(A);  // Calculate the diagonal equation
	//cout << endl << endl << "je" << endl;
	//cout << coff(0, 0) << " "<<coff(0,1) << endl;  k = -0.76 b = 1239
	CImg<float> vectorx(1,3,1,1,1);
	for (int i = 0; i < AFour._width; i++) {
		for (int j = 0; j < AFour._height; j++) {
			vectorx(0, 0) = i;
			vectorx(0, 1) = j;
			CImg<float> vectory(1, 3, 1, 1, 1);
			
			double inner_procuct = i*coff(0, 0) - j + coff(0, 1); // Calculate Ax+By+C 
			if (inner_procuct >= 0) {
				vectory = transform1*vectorx;
			}
			else {
				vectory = transform2*vectorx;
			}
			if (int(vectory(0, 0)) < 0) {
				vectory(0, 0) = 0;
			}
			if (int(vectory(0, 0)) >colorsrc._width-1) {
				vectory(0, 0) = colorsrc._width-1;
			}
			if (int(vectory(0, 1)) < 0) {
				vectory(0, 1) = 0;
			}
			if (int(vectory(0, 1)) >colorsrc._height - 1) {
				vectory(0, 1) = colorsrc._height - 1;
			}
			int a = int(vectory(0, 0));
			int b = int(vectory(0, 1));
			AFour(i, j, 0) = colorsrc(int(vectory(0, 0)), int(vectory(0, 1)), 0);
			AFour(i, j, 1) = colorsrc(int(vectory(0, 0)), int(vectory(0, 1)), 1);
			AFour(i, j, 2) = colorsrc(int(vectory(0, 0)), int(vectory(0, 1)), 2);
		}
	}
	AFour.display();
	AFour.save("a41.bmp");
}



