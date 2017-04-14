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

FaceMorphing::FaceMorphing(CImg<float> _src, CImg<float> _dst, vector<Point> srcpoints, vector<Point> dstpoints,vector<vector<int>>_index, int framecnt) {
	src = _src;
	dst = _dst;
	src_points = srcpoints;
	dst_points = dstpoints;

	for (int i = 0; i < _index.size(); i++) {
		triangle srctri(src_points[_index[i][0]], src_points[_index[i][1]], src_points[_index[i][2]]);
		triangle dsttri(dst_points[_index[i][0]], dst_points[_index[i][1]], dst_points[_index[i][2]]);
		src_triangle_list.push_back(srctri);
		dst_triangle_list.push_back(dsttri);
	}


	this->framecnt = framecnt;
	for (int i = 0; i < framecnt; i++) {
		vector<Point> tempvector;
		for (int j = 0; j < src_points.size(); j++) {
			double midx = double(src_points[j].x) + (double(i)) / double(framecnt - 1)*double(dst_points[j].x - src_points[j].x);
			double midy = double(src_points[j].y) + (double(i)) / double(framecnt - 1)*double(dst_points[j].y - src_points[j].y);
			Point midpoint(midx, midy);
			tempvector.push_back(midpoint);
		}
		mid_points.push_back(tempvector);
	}
	//cout << mid_points[1].size() << "  " << mid_points.size() << endl;;
	

	for (int i = 0; i < framecnt; i++) {
		vector<triangle> tempmid;
		for (int j = 0; j < _index.size(); j++) {
			triangle tri(mid_points[i][_index[j][0]], mid_points[i][_index[j][1]], mid_points[i][_index[j][2]]);
			tempmid.push_back(tri);
		}
		mid_triangle_list.push_back(tempmid);
	}
	/*
	cout << mid_triangle_list.size() << endl << endl;
	cout << mid_triangle_list[1].size() << endl << endl;
	for (int i = 0; i < framecnt; i++) {
		for (int j = 0; j < mid_triangle_list[i].size(); j++) {
			cout << mid_triangle_list[i][j].p1.x << " " << mid_triangle_list[i][j].p1.y << endl;
			 
		}
	}
	*/
}

void FaceMorphing::set_src_triangle(vector<triangle> triangle_arr) {
	src_triangle_list = triangle_arr;
}

void FaceMorphing::set_dst_triangle(vector<triangle> triangle_arr) {
	dst_triangle_list = triangle_arr;
}

CImg<float> FaceMorphing::find_transform(triangle before, triangle after) {
	CImg<float> y1(1, 3, 1, 1, 0);
	CImg<float> y2(1, 3, 1, 1, 0);
	CImg<float> coff1(1, 3, 1, 1, 0);
	CImg<float> coff2(1, 3, 1, 1, 0);
	CImg<float> A1(3, 3, 1, 1, 1);
	CImg<float> A2(3, 3, 1, 1, 1);

	CImg<float> temptransform(3, 3, 1, 1, 0);

	Point srcp1 = before.p1;
	Point srcp2 = before.p2;
	Point srcp3 = before.p3;
	Point dstp1 = after.p1;
	Point dstp2 = after.p2;
	Point dstp3 = after.p3;

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
	coff2 = y2.solve(A1);

	for (int j = 0; j < 3; j++) {
		temptransform(j, 0) = coff1(0, j);
		temptransform(j, 1) = coff2(0, j);
	}
	temptransform(2, 2) = 1;
	return temptransform;

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

	float inverse_deno = float(1) / (float)(dot00*dot11 - dot01*dot01);
	float u = float(dot11*dot02 - dot01*dot12)*inverse_deno;
	if (u < 0 || u > 1) return false;
	float v = float(dot00*dot12 - dot01*dot02)*inverse_deno;
	if (v < 0 || v > 1) return false;
	return u + v <= 1;
}
void FaceMorphing::morphing() {
	int size = mid_triangle_list[1].size();

	int width = dst._width;
	int height = dst._height;
	for (int k = 0; k < framecnt; k++) {
		CImg<float> temp(width, height, 1, 3, 1);
		cimg_forXY(temp, i, j) {
			CImg<float> x(1, 3, 1, 1, 1);
			CImg<float> y1(1, 3, 1, 1, 1);
			CImg<float> y2(1, 3, 1, 1, 1);
				for (int cnt = 0; cnt < size; cnt++) {
					Point p(i, j);
					if (is_intriangle(p, mid_triangle_list[k][cnt])) {
						CImg<float>transform_src = find_transform(mid_triangle_list[k][cnt], src_triangle_list[cnt]);
						x(0, 0) = i;
						x(0, 1) = j;
						y1 = transform_src * x;
						int dstx1 = int(y1(0, 0));
						int dsty1 = int(y1(0, 1));
						
						int tempnum = cnt;


						CImg<float>transform_dst = find_transform(mid_triangle_list[k][cnt],dst_triangle_list[cnt]);
						y2 = transform_dst*x;
						int dstx2 = int(y2(0, 0));
						int dsty2 = int(y2(0, 1));

						if (dstx1 < 0) dstx1 = 0;
						if (dstx1 > src._width - 1) dstx1 = src._width - 1;
						if (dsty1 < 0) dsty1 = 0;
						if (dsty1 > src._height - 1) dsty1 = src._height - 1;

						if (dstx2 < 0) dstx2 = 0;
						if (dstx2 > dst._width - 1) dstx2 = dst._width - 1;
						if (dsty2 < 0) dsty2 = 0;
						if (dsty2 > dst._height - 1) dsty2 = dst._height - 1;

						double a = double(k) / double(framecnt - 1);
						temp(i, j,0) = (1-a)*src(dstx1, dsty1,0) + a*dst(dstx2,dsty2,0);
						temp(i, j, 1) = (1 - a)*src(dstx1, dsty1, 1) + a*dst(dstx2, dsty2, 1);
						temp(i, j, 2) = (1 - a)*src(dstx1, dsty1, 2) + a*dst(dstx2, dsty2, 2);
						break;
					}
				}
		}
		temp.display();
	}
}

void FaceMorphing::run() {
	//find_transform();
	morphing();
}