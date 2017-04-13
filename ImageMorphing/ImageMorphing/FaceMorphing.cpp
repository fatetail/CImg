#include "FaceMorphing.h"


Vector2::Vector2(Point _start, Point _end) {
	start = _start;
	end = _end;

	x = end.x - start.x;
	y = end.y - start.y;
}
int Vector2::Dot(Vector2 another) {
	return x*another.x + y*another.y;
}

FaceMorphing::FaceMorphing(CImg<float> _src, CImg<float> _dst, vector<Point> srcpoints, vector<Point> dstpoints) {
	src = _src;
	dst = _dst;
	src_points = srcpoints;
	dst_points = dstpoints;
}

void FaceMorphing::set_src_triangle(vector<triangle> triangle_arr) {
	src_triangle_list = triangle_arr;
}

void FaceMorphing::set_dst_triangle(vector<triangle> triangle_arr) {
	dst_triangle_list = triangle_arr;
}

void FaceMorphing::find_transform() {
	int size = src_triangle_list.size();
	CImg<float> y1(1, 3, 1, 1, 0);
	CImg<float> y2(1, 3, 1, 1, 0);
	CImg<float> coff1(1, 3, 1, 1, 0);
	CImg<float> coff2(1, 3, 1, 1, 0);
	CImg<float> A1(3, 3, 1, 1, 1);
	CImg<float> A2(3, 3, 1, 1, 1);

	CImg<float> temptransform(3, 3, 1, 1, 0);
	for (int i = 0; i < size; i++) {
		Point srcp1 = src_triangle_list[i].p1;
		Point srcp2 = src_triangle_list[i].p2;
		Point srcp3 = src_triangle_list[i].p3;
		Point dstp1 = dst_triangle_list[i].p1;
		Point dstp2 = dst_triangle_list[i].p2;
		Point dstp3 = dst_triangle_list[i].p3;

		y1(0, 0) = dstp1.x;
		y2(0, 0) = dstp1.y;
		y1(0, 1) = dstp2.x;
		y2(0, 1) = dstp2.y;
		y1(0, 2) = dstp3.x;
		y2(0, 2) = dstp3.y;

		A1(0, 0) = srcp1.x;
		A1(1, 0) = srcp1.y;
		A1(0, 1) = srcp2.x;
		A1(1, 1) = srcp2.y;
		A1(0, 2) = srcp3.x;
		A1(1, 2) = srcp3.y;

		A2 = A1;

		coff1 = y1.solve(A1);
		coff2 = y2.solve(A2);

		for (int j = 0; j < 3; j++) {
			temptransform(j,0) = coff1(0, j);
			temptransform(j, 1) = coff2(0, j);
		}
		temptransform(2, 2) = 1;
		transform.push_back(temptransform);
	}

}

bool FaceMorphing::is_intriangle(Point P, triangle tri) {
	Vector2 v0(tri.p1, tri.p3);
	Vector2 v1(tri.p1, tri.p2);
	Vector2 v2(tri.p1, P);

	int dot00 = v0.Dot(v0);
	int dot01 = v0.Dot(v1);
	int dot02 = v0.Dot(v2);
	int dot11 = v1.Dot(v1);
	int dot12 = v1.Dot(v2);

	float inverse_deno = 1 / (float)(dot00*dot11 - dot01*dot01);
	float u = (dot11*dot02 - dot01*dot12)*inverse_deno;
	if (u < 0 || u > 1) return false;
	float v = (dot00*dot12 - dot01*dot02)*inverse_deno;
	if (v < 0 || v > 1) return false;
	return u + v <= 1;
}
void FaceMorphing::morphing() {
	int size = src_triangle_list.size();
	bool flag = true;
	for (int k = 0; k < ; k++) {
		
	}
	for (int k = 0; k < 5; k++) {
		CImg<float> temp = src;
		cimg_forXY(src, i, j) {
			CImg<float> x(1, 3, 1, 1, 1);
			CImg<float> y(1, 3, 1, 1, 1);
				for (int cnt = 0; cnt < size; cnt++) {
					Point p(i, j);
					if (is_intriangle(p, src_triangle_list[cnt])) {
						x(0, 0) = i;
						x(0, 1) = j;
						y = transform[cnt] * x;
						int dstx = int(y(0, 0));
						int dsty = int(y(0, 1));
						temp(i, j) = src(i, j) + (k*(dst(dstx, dsty) - src(i, j)) / 3);
						flag = false;
						break;
					}
				}
				if (!flag) {
					temp(i, j) = dst(i, j);
					flag = true;
			}
		}
		temp.display();
	}
}

void FaceMorphing::run(vector<triangle> src_triangle, vector<triangle> dst_triangle) {
	set_src_triangle(src_triangle);
	set_dst_triangle(dst_triangle);
	find_transform();
	morphing();
}