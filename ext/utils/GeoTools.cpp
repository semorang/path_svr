#include "GeoTools.h"

#include <math.h>

#include "../utils/UserLog.h"
#include "../utils/Strings.h"

#if defined(USE_PROJ4_LIB)
#include "../proj4/include/proj.h"
//#pragma comment(lib, "../proj4/lib/proj.lib")
#endif

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

// 시계방향. // CW
bool isRightSide(IN const SPoint* P1, IN const SPoint* P2, IN const SPoint* P3) //const
{
	// Is the order of (P1->P2->P3) Clock Wise?
	// . P1->P2벡터에 CW방향으로 수직인 벡터 NP1P2를 구한다.
	// NP1P2는 P1->P2벡터(P2.x-P1.x, P2.y-P1.y,0)와 Z축(0,0,1)을 외적하면 구할 수 있다.
	// NP1P2.x = (P2.y-P1.y)*1 - (0)*0 = P2.y-P1.y
	// NP1P2.y = (0)*0 - (P2.x-P1.x)*1 = P1.x-P2.x
	// . P1->P3 벡터 P1P3를 구한다.
	// P1P3.x = P3.x-P1.x
	// P1P3.y = P3.y-P1.y
	// . NP1P2벡터와 P1P3벡터를 내적하여 그 값이 0이상이면 CW 아니면 CCW이다.
	// NP1P2 (dot) P1P3 = (P2.y-P1.y)(P3.x-P1.x) + (P1.x-P2.x)(P3.y-P1.y)
	// = P2.y*P3.x - P2.y*P1.x - P1.y*P3.x + P1.y*P1.x + P1.x*P3.y - P1.x*P1.y - P2.x*P3.y + P2.x*P1.y
	// = P2.y*P3.x - P2.y*P1.x - P1.y*P3.x    (소거)   + P1.x*P3.y    (소거)   - P2.x*P3.y + P2.x*P1.y
	// = P2.y*P3.x + P1.x*P3.y + P2.x*P1.y - P2.y*P1.x - P1.y*P3.x - P2.x*P3.y
	// = P1.y*(P2.x-P3.x) + P2.y*(P3.x-P1.x) + P3.y*(P1.x-P2.x)

	//return (P2.x - P1.x)*(P3.y - P1.y) - (P3.x - P1.x)*(P2.y - P1.y) <= 0;

	if (P1 && P2 && P3) {
		return P1->y*(P2->x - P3->x) + P2->y*(P3->x - P1->x) + P3->y*(P1->x - P2->x) >= 0;
	}

	return false;
}

// 반시계 방향. // CCW
bool isLeftSide(IN const SPoint* P1, IN const SPoint* P2, IN const SPoint* P3) //const
{
	if (P1 && P2 && P3) {
		return P1->y*(P2->x - P3->x) + P2->y*(P3->x - P1->x) + P3->y*(P1->x - P2->x) < 0;
	}

	return false;
}


double get_road_slope(long x1, long y1, long z1, long x2, long y2, long z2)
{
	double road_slope = 0.f;
	// calc real distance in Meter`
	long double iDistance = sqrtl((x2 - x1) + (y2 - y1));
	long iHeight = (z2 - z1);

	return road_slope = atan(iHeight / static_cast<double>(iDistance)) * 100;
}


double get_road_slope(int32_t dist, int32_t height)
{
	static double rad2deg = 57.2957951; // 180 / pi
	return asin(height / static_cast<double>(dist)) * rad2deg;
}


double getRealWorldDistance(IN const double slng, IN const double slat, IN const double elng, IN const double elat)
{
	double dDistance;
	double dSinG, dCosL, dCosF, dSinL, dSinF, dCosG;
	double dS, dMu, dR;

	double dRadius = 6378.13700;
	double dFlatness = 1 / 298.257;

	double dL = (slng - elng) / 2;
	double dF = (slat + elat) / 2;
	double dG = (slat - elat) / 2;

	dL = 3.141592 / 180.0 * dL;
	dF = 3.141592 / 180.0 * dF;
	dG = 3.141592 / 180.0 * dG;

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


bool getPointByDistance(IN const double slng, IN const double slat, IN OUT double& elng, IN OUT double& elat, const double lDistance)
{
	if ((slng == elng) && (slat == elat)) {
		return false;
	}

	const double	totalDistance = getRealWorldDistance(slng, slat, elng, elat);
	if (totalDistance == 0) {
		return false;
	}

	const double	distanceRatio = (float)lDistance / totalDistance;

	const double	dx = (slng - elng) * distanceRatio;
	const double	dy = (slat - elat) * distanceRatio;

	elng = slng - dx;
	elat = slat - dy;

	return true;
}


// 두점 사이의 중심에서 수직으로 일정 거리 떨어진 지점의 좌표 계산
bool getPointByDistanceFromCenter(IN const double slng, IN const double slat, IN const double& elng, IN const double& elat, IN const double lDistance, IN const bool isRight, OUT double& x, OUT double& y)
{
	// 중간점 계산
	double xm = (slng + elng) / 2.0;
	double ym = (slat + elat) / 2.0;

	// 두 점 사이의 거리 계산
	double dx, dy;
	if (isRight) {
		dx = slng - elng;
		dy = slat - elat;
	} else {
		dx = elng - slng;
		dy = elat - slat;
	}

	double length = std::sqrt(dx * dx + dy * dy);

	if (length == 0.f) {
		return false;
	}

	// 수직 방향으로 이동한 좌표 계산
	x = xm - lDistance * dy / length;
	y = ym + lDistance * dx / length;

	return true;
}


void extendDataBox(IN OUT SBox& box, IN const double lng, IN const double lat)
{
	if (lng < box.Xmin || box.Xmin == 0) { box.Xmin = lng; }
	if (box.Xmax < lng || box.Xmax == 0) { box.Xmax = lng; }
	if (lat < box.Ymin || box.Ymin == 0) { box.Ymin = lat; }
	if (box.Ymax < lat || box.Ymax == 0) { box.Ymax = lat; }
}


void extendDataBox(IN OUT SBox& box, IN const SPoint* pCoords, IN const int nCoordCount)
{
	if (pCoords != nullptr && nCoordCount > 0) {
		for (int ii = 0; ii < nCoordCount; ii++) {
			extendDataBox(box, pCoords[ii].x, pCoords[ii].y);
		}
	}
}


void extendDataBox(IN OUT SBox& box, IN const SBox& data)
{
	extendDataBox(box, data.Xmin, data.Ymin);
	extendDataBox(box, data.Xmin, data.Ymax);
	extendDataBox(box, data.Xmax, data.Ymax);
	extendDataBox(box, data.Xmax, data.Ymin);
}

bool isInBox(IN const double lon, IN const double lat, IN const SBox& baseBox, IN const double inMargin)
{
	if (lon < baseBox.Xmin - inMargin) return false;
	if (lon > baseBox.Xmax + inMargin) return false;
	if (lat < baseBox.Ymin - inMargin) return false;
	if (lat > baseBox.Ymax + inMargin) return false;

	return true;
}

// 겹치는지 여부만 판단하는 함수
bool isBoxOverlap(const SBox& inBox, const SBox& baseBox)
{
	return !(inBox.Xmax < baseBox.Xmin || 
		inBox.Xmin > baseBox.Xmax || 
		inBox.Ymax < baseBox.Ymin || 
		inBox.Ymin > baseBox.Ymax);
}

// 포함 여부 판단 (전체 포함 여부)
bool isBoxInside(const SBox& inner, const SBox& outer, double margin = 0.0)
{
	return (inner.Xmin >= outer.Xmin - margin &&
		inner.Xmax <= outer.Xmax + margin &&
		inner.Ymin >= outer.Ymin - margin &&
		inner.Ymax <= outer.Ymax + margin);
}


bool isInPitBox(IN const SBox& inBox, IN const SBox& baseBox)
{
	if (isInBox(inBox.Xmin, inBox.Ymin, baseBox)) return true; // LT
	if (isInBox(inBox.Xmax, inBox.Ymin, baseBox)) return true; // RT
	if (isInBox(inBox.Xmax, inBox.Ymax, baseBox)) return true; // RB
	if (isInBox(inBox.Xmin, inBox.Ymax, baseBox)) return true; // LB

	if ((inBox.Xmin <= baseBox.Xmin && baseBox.Xmin <= inBox.Xmax &&
		baseBox.Ymin <= inBox.Ymin && inBox.Ymax <= baseBox.Ymax) // L
		|| (inBox.Xmin <= baseBox.Xmax && baseBox.Xmax <= inBox.Xmax &&
			baseBox.Ymin <= inBox.Ymin && inBox.Ymax <= baseBox.Ymax) // R
		|| (inBox.Ymin <= baseBox.Ymin && baseBox.Ymin <= inBox.Ymax &&
			baseBox.Xmin <= inBox.Xmin && inBox.Xmax <= baseBox.Xmax) // T
		|| (inBox.Ymin <= baseBox.Ymax && baseBox.Ymax <= inBox.Ymax &&
			baseBox.Xmin <= inBox.Xmin && inBox.Xmax <= baseBox.Xmax)) { // B
		return true;
	}

	// 화면이 메쉬에 포함
	if (isInBox(baseBox.Xmin, baseBox.Ymin, inBox)) return true; // LT
	if (isInBox(baseBox.Xmax, baseBox.Ymin, inBox)) return true; // RT
	if (isInBox(baseBox.Xmax, baseBox.Ymax, inBox)) return true; // RB
	if (isInBox(baseBox.Xmin, baseBox.Ymax, inBox)) return true; // LB

	return false;
}


bool isOnPitBox(IN const SBox& onBox, IN const SBox& baseBox)
{
	if (isInBox(onBox.Xmin, onBox.Ymin, baseBox)) return true; // LT
	if (isInBox(onBox.Xmax, onBox.Ymin, baseBox)) return true; // RT
	if (isInBox(onBox.Xmax, onBox.Ymax, baseBox)) return true; // RB
	if (isInBox(onBox.Xmin, onBox.Ymax, baseBox)) return true; // LB

	if ((onBox.Xmin <= baseBox.Xmin && baseBox.Xmin <= onBox.Xmax &&
		baseBox.Ymin <= onBox.Ymin && onBox.Ymax <= baseBox.Ymax) // L
		|| (onBox.Xmin <= baseBox.Xmax && baseBox.Xmax <= onBox.Xmax &&
			baseBox.Ymin <= onBox.Ymin && onBox.Ymax <= baseBox.Ymax) // R
		|| (onBox.Ymin <= baseBox.Ymin && baseBox.Ymin <= onBox.Ymax &&
			baseBox.Xmin <= onBox.Xmin && onBox.Xmax <= baseBox.Xmax) // T
		|| (onBox.Ymin <= baseBox.Ymax && baseBox.Ymax <= onBox.Ymax &&
			baseBox.Xmin <= onBox.Xmin && onBox.Xmax <= baseBox.Xmax)) { // B
		return true;
	}

	// 화면이 메쉬에 포함
	if (isInBox(baseBox.Xmin, baseBox.Ymin, onBox)) return true; // LT
	if (isInBox(baseBox.Xmax, baseBox.Ymin, onBox)) return true; // RT
	if (isInBox(baseBox.Xmax, baseBox.Ymax, onBox)) return true; // RB
	if (isInBox(baseBox.Xmin, baseBox.Ymax, onBox)) return true; // LB

	return false;
}


// WGS84 → Web Mercator (EPSG:3857)
double lngToX(double lng)
{
	return lng * 20037508.34 / 180.0;
}

double latToY(double lat)
{
	double rad = lat * M_PI / 180.0;
	return log(tan(M_PI / 4.0 + rad / 2.0)) * 20037508.34 / M_PI;
}

// Web Mercator → WGS84
double xTolng(double x)
{
	return x * 180.0 / 20037508.34;
}

double yToLat(double y)
{
	double rad = y * M_PI / 20037508.34;
	return atan(sinh(rad)) * 180.0 / M_PI;
}

//x1, y1  : first point 
//x2, y2  : Second point
//ox, oy  : Selected Point 
//*a, *b  : Projection Point
//*ir     : Link Ratio
//void CTrekkingRouteDlg::GetClosestPoint(int x1, int y1, int x2, int y2, int ox, int oy, int *a, int *b, int *ir)
void getClosestPoint(IN const double lng1, IN const double lat1, IN const double lng2, IN const double lat2, IN const double lng3, IN const double lat3, OUT double *lng, OUT double *lat, IN OUT double *ir)
{
#if 1
	double x1 = lngToX(lng1);
	double y1 = latToY(lat1);
	double x2 = lngToX(lng2);
	double y2 = latToY(lat2);
	double x3 = lngToX(lng3);
	double y3 = latToY(lat3);

	double dx = x2 - x1;
	double dy = y2 - y1;
	double l2 = (dx * dx) + (dy * dy);

	if (l2 == 0.0) {
		// x1, y1 == x2, y2 : 선분이 아닌 점일 경우
		*lng = lng1;
		*lat = lat1;
		*ir = 0.0;
		return;
	}

	double t = ((x3 - x1) * dx + (y3 - y1) * dy) / l2;

	if (t < 0.0) {
		*lng = lng1;
		*lat = lat1;
	} else if (t > 1.0) {
		*lng = lng2;
		*lat = lat2;
	} else {
		*lng = xTolng(x1 + t * dx);
		*lat = yToLat(y1 + t * dy);
	}

	*ir = t; // 비율 (0~1), 혹은 0보다 작거나 1보다 큼
#else
	double dx = lng2 - lng1;
	double dy = lat2 - lat1;
	double l2 = (dx * dx) + (dy * dy);
	double precision = IR_PRECISION;

	double dbIr = (((lng3 - lng1) * dx) + ((lat3 - lat1) * dy)) / l2 * precision;

	if (dbIr < 0) {
		*lng = lng1;
		*lat = lat1;
		//*ir = 0;
		*ir = dbIr;
	}
	else if (dbIr > precision) {
		*lng = lng2;
		*lat = lat2;
		//*ir = precision;
		*ir = dbIr;
	}
	else {
		*lng = lng1 + ((dbIr * dx) / precision);
		*lat = lat1 + ((dbIr * dy) / precision);
		*ir = dbIr;
	}
#endif
}


#if defined(USE_PROJ4_LIB)
PJ_CONTEXT* proj_ctxt = nullptr;
PJ* proj_pj = nullptr;

// Source CRS : UTM52N
// Source CRS : UTMK(EPSG:5179)
// Target CRS : WGS84(EPSG:4326)
// 교차 변환은 인자 Text만 변경하면 됨
bool initProj4(IN const char* szType)
{
	char strcoordType[64] = { 0, };
	if (szType != nullptr && strlen(szType) > 0) {
		strcpy(strcoordType, szType);
		strupper(strcoordType);
	}

	proj_ctxt = proj_context_create();
	if (proj_ctxt != nullptr) {
		if (strcmp(strcoordType, "UTMK") == 0) { // UTMK(EPSG:5179)
			proj_pj = proj_create_crs_to_crs(proj_ctxt, "+proj=tmerc +lat_0=38 +lon_0=127.5 +k=0.9996 +x_0=1000000 +y_0=2000000 +ellps=GRS80 +units=m +no_defs", "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs", nullptr);
			//proj_pj = proj_create_crs_to_crs(proj_ctxt, "+proj=tmerc +lat_0=38 +lon_0=127 +k=1 +x_0=200000 +y_0=600000 +ellps=GRS80 +units=m +no_defs", "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs", nullptr);
		}
		else { // if (strcmp(strupr(szType), "UTM52N") == 0) {
			proj_pj = proj_create_crs_to_crs(proj_ctxt, "+proj=utm +zone=52 +ellps=WGS84 +datum=WGS84 +units=m +no_defs", "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs", nullptr);
		} 
		//P = proj_create_crs_to_crs(ctxt, "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs", "+proj=utm +zone=52 +ellps=WGS84 +datum=WGS84 +units=m +no_defs", nullptr);
	}

	if (proj_ctxt == nullptr || proj_pj == nullptr) {
		LOG_TRACE(LOG_WARNING, "Failed, PROJ4 create failed.");
		return false;
	}

	return true;
}

void releaseProj4()
{
	if (proj_pj != nullptr) {
		proj_destroy(proj_pj);
		proj_pj = nullptr;
	}
	if (proj_ctxt != nullptr) {
		proj_context_destroy(proj_ctxt);
		proj_ctxt = nullptr;
	}
}

bool translateUTM52NtoWGS84(IN const double lng, IN const double lat, OUT double& x, OUT double& y)
{
	if (proj_pj == nullptr) {
		LOG_TRACE(LOG_WARNING, "PROJ4 library not defined");
		return false;
	}

	PJ_COORD c;
	c.v[0] = lng;
	c.v[1] = lat;

	PJ_COORD c_ori = c;
	auto res = proj_trans(proj_pj, PJ_FWD, c);

	//printf("%f, %f\n", res.v[0], res.v[1]);

	x = res.v[0];
	y = res.v[1];

	return true;
}

bool translateUTMKtoWGS84(IN const double lng, IN const double lat, OUT double& x, OUT double& y)
{
	if (proj_pj == nullptr) {
		LOG_TRACE(LOG_WARNING, "PROJ4 library not defined");
		return false;
	}

	PJ_COORD c;
	c.v[0] = lng;
	c.v[1] = lat;

	PJ_COORD c_ori = c;
	auto res = proj_trans(proj_pj, PJ_FWD, c);

	//printf("%f, %f\n", res.v[0], res.v[1]);

	x = res.v[0];
	y = res.v[1];

	return true;
}
#endif // #if defined(USE_PROJ4_LIB)


//////////////////////////////////////////////////////////////////////////////////////////////////////
// POLYGON
bool isPointInPolygon(const double x, const double y, const SPoint *pptPolygon, const int32_t nPolygon)
{
	int32_t i, j;
	bool c = false;

	for (i = 0, j = nPolygon - 1; i < nPolygon; j = i++) {
		if (((((float)pptPolygon[i].y <= (float)y) && ((float)y < (float)pptPolygon[j].y)) ||
			(((float)pptPolygon[j].y <= (float)y) && ((float)y < (float)pptPolygon[i].y))) &&
			((float)x < ((float)pptPolygon[j].x - (float)pptPolygon[i].x) * ((float)y - (float)pptPolygon[i].y) / ((float)pptPolygon[j].y - (float)pptPolygon[i].y) + (float)pptPolygon[i].x))
			c = !c;
	}

	return c;
}


bool isPointInPolygon(const SPoint *ppt, const SPoint *pptPolygon, const int32_t nPolygon)
{
	bool ret = false;

	if (ppt && pptPolygon && nPolygon) {
		ret = isPointInPolygon(ppt->x, ppt->y, pptPolygon, nPolygon);
	}

	return ret;
}


// 폴리곤의 중심점을 구하는 함수
SPoint getPolygonCenter(IN const vector<SPoint>& points)
{
	SPoint center = { 0.0, 0.0 };

	for (int i = 0; i < points.size(); i++) {
		center.x += points[i].x;
		center.y += points[i].y;
	}
	center.x /= points.size();
	center.y /= points.size();

	return center;
}

// 폴리곤 면적을 구하는 함수
double getPolygonArea(IN const vector<SPoint>& points) {
	double ret = 0;
	int i, j;
	int cnt = points.size();
	i = cnt - 1;

	if (cnt <= 2) {
		ret = 0;
	} else {
		for (j = 0; j < cnt; ++j) {
			ret += points[i].x * points[j].y - points[j].x * points[i].y;
			i = j;
		}
		ret = ret < 0 ? -ret : ret;
		ret /= 2;
	}

	return ret * 100000;
}
