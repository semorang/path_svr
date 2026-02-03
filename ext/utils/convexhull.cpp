#include "convexhull.h"
#include "CatmullRom.h"
#include <math.h>

using namespace std;

int SPointCompare(const void* Q1, const void* Q2)
{
	SPoint*p1 = (SPoint*)Q1;
	SPoint*p2 = (SPoint*)Q2;

	if (p1->x == p2->x)
		return p1->y > p2->y ? 1 : p1->y < p2->y ? -1 : 0;
	return p1->x > p2->x ? 1 : p1->x < p2->x ? -1 : 0;
}

double ConvexHull_Cross(SPoint *O, SPoint *A, SPoint *B)
{
	return (A->x - O->x) * (B->y - O->y) - (A->y - O->y) * (B->x - O->x);
}

SPoint calculatePoint(IN const SPoint center, IN const double distance, IN const double angle)
{
	SPoint result;
	double radAngle = angle * PI_PRECISION / 180.0;
	result.y = center.y + distance * cos(radAngle);
	result.x = center.x + distance * sin(radAngle) / cos(center.y * PI_PRECISION / 180.0);
	
	return result;
}


void calculateTriangle(IN const SPoint p1, IN const double distance, OUT vector<SPoint>& vtPoints)
{
	array<double, 3> angles = { 0.0, 120.0, 240.0 }; // 각도 설정

	for (const auto& angle : angles) {
		vtPoints.push_back(calculatePoint(p1, distance, angle));
	}

	// make closed polygon
	vtPoints.push_back(vtPoints.front());
}

void calculateRectangle(SPoint p1, SPoint p2, double distance, OUT vector<SPoint>& vtPoints)
{
	// 직선의 방향 벡터 계산
	double dx = p2.x - p1.x;
	double dy = p2.y - p1.y;
	double length = sqrt(dx*dx + dy*dy);

	// 직선의 단위 방향 벡터 계산
	double ux = dx / length;
	double uy = dy / length;

	// 직선에 수직인 방향 벡터 계산 (90도 회전)
	double perp_x = -uy;
	double perp_y = ux;

	// 사각형 네 점 계산
	vtPoints.push_back({ p1.x + perp_x * distance, p1.y + perp_y * distance });
	vtPoints.push_back({ p1.x - perp_x * distance, p1.y - perp_y * distance });
	vtPoints.push_back({ p2.x - perp_x * distance, p2.y - perp_y * distance });
	vtPoints.push_back({ p2.x + perp_x * distance, p2.y + perp_y * distance });

	// make closed polygon
	vtPoints.push_back(vtPoints[0]);
}

void ConvexHull(IN vector<SPoint> &r, OUT vector<SPoint> &q)
{
	if (r.size() > 2)
	{
		vector<SPoint> p(r.size());
		p.assign(r.begin(), r.end());

		qsort(&p[0], p.size(), sizeof(SPoint), SPointCompare);

		// 동일 데이터 제거
		vector<SPoint> vtTmp;
		SPoint prevPt{};
		for (const auto & ptNow : p) {
			if ((ptNow.x != prevPt.x) || (ptNow.y != prevPt.y)) {
				vtTmp.emplace_back(ptNow);
			}
			prevPt = ptNow;
		}

		if (p.size() != vtTmp.size()) {
			p.clear();
			p.assign(vtTmp.begin(), vtTmp.end());
		}

		if (p.size() <= 1) {
			return;
		}

		int n = p.size(), k = 0;
		// std::vector<SPoint> h(2 * n);
		//q.reserve(2 * n);
		q.resize(2 * n);

		// Build lower hull
		for (int i = 0; i < n; ++i) {
			while (k >= 2 && ConvexHull_Cross(&q[k - 2], &q[k - 1], &p[i]) <= 0) k--;
			q[k++] = p[i];
		}

		// Build upper hull
		for (int i = n - 2, t = k + 1; i >= 0; i--) {
			while (k >= t && ConvexHull_Cross(&q[k - 2], &q[k - 1], &p[i]) <= 0) k--;
			q[k++] = p[i];
		}

		if (k > 1) {
			//H = Arrays.copyOfRange(H, 0, k - 1); // remove non-hull vertices after k; remove k - 1 which is a duplicate
			std::vector<SPoint> NewH(k - 1);
			NewH.assign(q.begin(), q.begin() + (k - 1));
			q = NewH;
		}
		// return h.size();
	} else if ((r.size() == 2) && (r[0] != r[1])) {
		q.reserve(4); // make rectangle;
		
		// 직선의 방향 벡터 계산
		double dx = r[1].x - r[0].x;
		double dy = r[1].y - r[0].y;
		double length = sqrt(dx*dx + dy*dy);
		if (length > 0.00500) {
			length = 0.00500; // 500m 반경
		}
		calculateRectangle(r[0], r[1], length / 4, q); // 500m 반경
	} else if (r.size() <= 2) {
		q.reserve(3); // make triangle;
		calculateTriangle(r[0], 500 / 100000.0, q); // 500m 반경
	}
	// else if (p.size() <= 1)
	// 	return p.size();

	// return 0;
}


double signedArea(const vector<SPoint> &p)
{
	double A = 0;
	//========================================================//
	// Assumes:                                               //
	//    N+1 vertices:   p[0], p[1], ... , p[N-1], p[N]      //
	//    Closed polygon: p[0] = p[N]                         //
	// Returns:                                               //
	//    Signed area: +ve if anticlockwise, -ve if clockwise //
	//========================================================//
	int N = p.size() - 1;
	for (int i = 0; i < N; i++) A += p[i].x * p[i + 1].y - p[i + 1].x * p[i].y;
	A *= 0.5;
	return A;
}


const double SMALL = 1.0e-10;

void GetCatmullLine(IN const vector<SPoint> &p, IN const double nZoomDiaMeter, OUT vector<SPoint> &q)
{
	int i = 0;
	SPoint pt;
	// vector<SPoint> vtPtDZoomResult;
	CatmullRom curve;

	//if (nZoomDiaMeter != 0)
	//	GetBorderOfPolygon(vtPtD, nZoomDiaMeter, vtPtDZoomResult);
	//else
		// vtPtDZoomResult = p;

	curve.begin(50); // generate 100 interpolate points between the last 4 way points
	for (i = 0; i < p.size(); ++i) {
		curve.add_way_point(CVector2D(p[i].x, p[i].y));
	}
	curve.end(true);

	// vector<SPoint> vtPtD_Result; 
	q.reserve(curve.node_count());
	for (i = 0; i < curve.node_count(); ++i) {
		pt.x = curve.node(i).x, pt.y = curve.node(i).y;
		q.push_back(pt);
	}
	// return q;
}

// *************************** Extra ************************ //
//1. 폴리곤 확대/축소 (독자적으로 쓰이지 않고.. 아래 GetCatmullLine함수의 nZoomDiaMeter에 의해 자동으로 호출됨)
//void GetBorderOfPolygon( const vector<SPoint> &p, double thickness, vector<SPoint> &q );

void GetBorderOfPolygon(IN const vector<SPoint> &p, IN const double thickness, OUT vector<SPoint> &q)
{
	//=====================================================//
	// Assumes:                                            //
	//    N+1 vertices:   p[0], p[1], ... , p[N-1], p[N]   //
	//    Closed polygon: p[0] = p[N]                      //
	//    No zero-length sides                             //
	// Returns (by reference, as a parameter):             //
	//    Internal poly:  q[0], q[1], ... , q[N-1], q[N]   //
	//=====================================================//
	int N = p.size() - 1;
	q.resize(N + 1);

	double a, b, A, B, d, cross;

	double displacement = thickness;
	if (signedArea(p) < 0) displacement = -displacement;     // Detects clockwise order

															 // Unit vector (a,b) along last edge
	a = p[N].x - p[N - 1].x;
	b = p[N].y - p[N - 1].y;
	d = sqrt(a * a + b * b);
	a /= d;
	b /= d;

	// Loop round the polygon, dealing with successive intersections of lines
	for (int i = 0; i < N; i++)
	{
		// Unit vector (A,B) along previous edge
		A = a;
		B = b;
		// Unit vector (a,b) along next edge
		a = p[i + 1].x - p[i].x;
		b = p[i + 1].y - p[i].y;
		d = sqrt(a * a + b * b);
		a /= d;
		b /= d;
		// New vertex
		cross = A * b - a * B;
		if (abs(cross) < SMALL)      // Degenerate cases: 0 or 180 degrees at vertex
		{
			q[i].x = p[i].x - displacement * b;
			q[i].y = p[i].y + displacement * a;
		}
		else                             // Usual case
		{
			q[i].x = p[i].x + displacement * (a - A) / cross;
			q[i].y = p[i].y + displacement * (b - B) / cross;
		}
	}

	// Close the inside polygon
	q[N] = q[0];
}

//======================================================================




double calc_line_length2D(SPoint& first, SPoint& second)
{
	double dDistance;
	double dSinG, dCosL, dCosF, dSinL, dSinF, dCosG;
	double dS, dMu, dR;

	double dRadius = 6378.13700;
	double dFlatness = 1 / 298.257;

	double dL = (first.x - second.x) / 2;
	double dF = (first.y + second.y) / 2;
	double dG = (first.y - second.y) / 2;

	dL = PI_PRECISION / 180.0 * dL;
	dF = PI_PRECISION / 180.0 * dF;
	dG = PI_PRECISION / 180.0 * dG;

	dSinG = sin(dG);
	dCosL = cos(dL);
	dCosF = cos(dF);
	dSinL = sin(dL);
	dSinF = sin(dF);
	dCosG = cos(dG);

	dS = dSinG * dSinG * dCosL * dCosL + dCosF * dCosF * dSinL * dSinL;

	dMu = asin(sqrt(dS));
	dR = sqrt(dS * (1 - dS));

	if (dS == 1.0 || dS == 0.0)
		return 0;

	dDistance = dRadius * (2 * dMu + dFlatness * ((3 * dR - dMu) / (1 - dS) * dSinF * dSinF * dCosG * dCosG -
		(3 * dR + dMu) / dS * dSinG * dSinG * dCosF * dCosF));

	dDistance *= 1000.f;

	return dDistance;
}


double calc_lineN_length2D(const std::vector<SPoint>& vtPolyline) 
{ //except Z
	double dLength = 0.;
	for (int i = 1; i < vtPolyline.size(); ++i) {
		double dTemp = calc_line_length2D((SPoint&)vtPolyline[i - 1], (SPoint&)vtPolyline[i]);
		//printf("%d> (%f, %f -> %f, %f) ==> %f\n", i, vtPolyline[i - 1].x(), vtPolyline[i - 1].y(), vtPolyline[i].x(), vtPolyline[i].y(), dLength);
		dLength += dTemp;
	} //for
	return dLength;
}

//line에서 dist만큼 진입한 *위치를 구함. i.e> [start]---------*--------------
std::tuple<int, SPoint> calc_online_point_by_dist(const std::vector<SPoint>& vtPolyline, double dDistM) 
{
	std::vector<SPoint>::const_iterator loc = vtPolyline.begin();
	SPoint PointResult = *loc;

	if (dDistM <= 0.0f) return std::make_tuple(-1, PointResult);

	double len_cur = 0.0;
	double dRemain, len_sum = 0.0;
	double dx, x1, x0 = loc->x;
	double dy, y1, y0 = loc->y;
	//double dz, z1, z0 = loc->z();
	int i = 0;

	for (++loc; loc != vtPolyline.end(); ++loc, ++i) {
		x1 = loc->x; y1 = loc->y;// z1 = loc->z();
		dx = x1 - x0;
		dy = y1 - y0;
		//dz = z1 - z0;
		len_cur = sqrt(dx * dx + dy * dy);// +dz * dz);
		if (dDistM <= (len_sum + len_cur)) {
			dRemain = len_sum + len_cur - (dDistM);
			PointResult.x = x1 - (dx * dRemain) / len_cur;
			PointResult.y = y1 - (dy * dRemain) / len_cur;
			//PointResult[2] = z1 - (dz * dRemain) / len_cur;
			break;
		}
		len_sum += len_cur;
		x0 = x1;
		y0 = y1;
		//z0 = z1;
	} //for
	int nOffsetResult = i;
	//double RemainDistResult = len_cur;
	//if (PointResult[0] == vtPolyline[0][0] && PointResult[1] == vtPolyline[0][1])
	//	PointResult = vtPolyline[vtPolyline.size() - 1];

	std::tuple<int, SPoint> tupRes = std::make_tuple(nOffsetResult, PointResult);

	return tupRes;
}

void make_Line_has_same_gap(const std::vector<SPoint>& vt_3dpnt4Line, int nMeterGap, std::vector<SPoint>& vtResultPolyline) 
{ //nMeterGap 으로 구성된 Polyline을 재 생성함..
	int nVert = vt_3dpnt4Line.size();
	if (nVert < 3) return;
	vtResultPolyline.clear();
	vtResultPolyline.reserve(nVert);

	double toGetDist = 0;
	double pline_dist = calc_lineN_length2D(vt_3dpnt4Line);
	for (double i = 0; i < pline_dist; i += nMeterGap) {
		toGetDist = i;
		std::tuple<int, SPoint> tupRst = calc_online_point_by_dist(vt_3dpnt4Line, toGetDist * 0.00001);
		int nOff = std::get<0>(tupRst);
		SPoint OffPoint = std::get<1>(tupRst);
		vtResultPolyline.push_back(OffPoint);    // Points coordinates.
	} //for
	vtResultPolyline.push_back(vt_3dpnt4Line[nVert - 1]);    // Points coordinates.
}

void GetSlicedLine(IN const vector<SPoint> &p, IN const int nMeterGap, vector<SPoint> &vtResultPolyline)
{
	return make_Line_has_same_gap(p, nMeterGap, vtResultPolyline);
}