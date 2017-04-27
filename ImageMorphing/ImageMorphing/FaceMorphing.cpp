#include "FaceMorphing.h"

/*Constructor of 2D Vector*/
Vector2::Vector2(Point _start, Point _end) {
	start = _start;
	end = _end;

	x = end.x - start.x;
	y = end.y - start.y;
}

/*Calculate the dot product of two vectors*/
float Vector2::Dot(Vector2 another) {
	return x*another.x + y*another.y;
}




FaceMorphing::FaceMorphing(CImg<float> _src, CImg<float> _dst, vector<Point> srcpoints, vector<Point> dstpoints,vector<vector<int>>_index, int framecnt) {
	src = _src;
	dst = _dst;
	src_points = srcpoints;
	dst_points = dstpoints;
	index = _index;
	this->framecnt = framecnt;

	//Get source image and destination image triangles by indexs;
	for (int i = 0; i < _index.size(); i++) {
		triangle srctri(src_points[_index[i][0]], src_points[_index[i][1]], src_points[_index[i][2]]);
		triangle dsttri(dst_points[_index[i][0]], dst_points[_index[i][1]], dst_points[_index[i][2]]);
		src_triangle_list.push_back(srctri);
		dst_triangle_list.push_back(dsttri);
	}


	//Get middle image feature points in every frame(except the first frame and last frame)
	for (int i = 0; i < framecnt-1; i++) {
		vector<Point> tempvector;
		for (int j = 0; j < src_points.size(); j++) {
			double midx = double(src_points[j].x) + (double(i+1)) / double(framecnt)*double(dst_points[j].x - src_points[j].x);
			double midy = double(src_points[j].y) + (double(i+1)) / double(framecnt)*double(dst_points[j].y - src_points[j].y);
			Point midpoint(midx, midy);
			tempvector.push_back(midpoint);
		}
		mid_points.push_back(tempvector);
	}

	
	//Get middle image triangle in every frame(except the first frame and last frame)
	for (int i = 0; i < framecnt-1; i++) {
		vector<triangle> tempmid;
		for (int j = 0; j < _index.size(); j++) {
			triangle tri(mid_points[i][_index[j][0]], mid_points[i][_index[j][1]], mid_points[i][_index[j][2]]);
			tempmid.push_back(tri);
		}
		mid_triangle_list.push_back(tempmid);
	}

}

/*Get a 2D transform matrix for a transfrom*/
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


/*Judge whether a point is in a triangle using centroid method*/
bool FaceMorphing::is_intriangle(Point P, triangle tri) {
	Vector2 v0(tri.p1, tri.p3);
	Vector2 v1(tri.p1, tri.p2);
	Vector2 v2(tri.p1, P);
	/*The bug I made
	int dot00 = v0.Dot(v0);
	int dot01 = v0.Dot(v1);
	int dot02 = v0.Dot(v2);
	int dot11 = v1.Dot(v1);
	int dot12 = v1.Dot(v2);
	*/
	float dot00 = v0.Dot(v0);
	float dot01 = v0.Dot(v1);
	float dot02 = v0.Dot(v2);
	float dot11 = v1.Dot(v1);
	float dot12 = v1.Dot(v2);
	
	float inverse_deno = float(1) / (float)(dot00*dot11 - dot01*dot01);
	float u = float(dot11*dot02 - dot01*dot12)*inverse_deno;
	if (u < 0 || u > 1) return false;
	float v = float(dot00*dot12 - dot01*dot02)*inverse_deno;
	if (v < 0 || v > 1) return false;

	return u + v <= 1;
}

//Facemorphing for every frame
void FaceMorphing::morphing() {
	int size = mid_triangle_list[0].size();  //The num of middle image triangles for every frame

	int width = dst._width;
	int height = dst._height;
	result.push_back(src);  // first frame

	for (int k = 0; k < framecnt-1; k++) {
		CImg<float> temp(width, height, 1, 3, 1);
		cimg_forXY(temp, i, j) {
			CImg<float> x(1, 3, 1, 1, 1);
			CImg<float> y1(1, 3, 1, 1, 1);
			CImg<float> y2(1, 3, 1, 1, 1);
				for (int cnt = 0; cnt < size; cnt++) {
					Point p(i, j);
					if (is_intriangle(p, mid_triangle_list[k][cnt])) {
						
						CImg<float>transform_src = find_transform(mid_triangle_list[k][cnt], src_triangle_list[cnt]);   // The transform from middle image to source image
						x(0, 0) = i;
						x(0, 1) = j;
						y1 = transform_src * x;
						int dstx1 = int(y1(0, 0));
						int dsty1 = int(y1(0, 1));
						

						CImg<float>transform_dst = find_transform(mid_triangle_list[k][cnt],dst_triangle_list[cnt]);
						y2 = transform_dst*x;
						int dstx2 = int(y2(0, 0));
						int dsty2 = int(y2(0, 1));

						//The combination of source color and destination color
						double a = double(k+1) / double(framecnt);
						temp(i, j, 0) = (1 - a)*src(dstx1, dsty1, 0) + a*dst(dstx2, dsty2, 0);
						temp(i, j, 1) = (1 - a)*src(dstx1, dsty1, 1) + a*dst(dstx2, dsty2, 1);
						temp(i, j, 2) = (1 - a)*src(dstx1, dsty1, 2) + a*dst(dstx2, dsty2, 2);
						break;
					}
				}
		}
		//temp.display();
		result.push_back(temp);  // The middle frame
	}
	result.push_back(dst);  // The last frame
	for (int i = 0; i < result.size(); i++) {
		string s = to_string(i+3);
		s += ".bmp";
		result[i].save_bmp(s.c_str());  // Save every frame
	}
}

/*The run function*/
void FaceMorphing::run() {
	morphing();
}