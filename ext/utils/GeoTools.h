#pragma once

#include "../include/MapDef.h"

#if defined(USE_P2P_DATA)
// #define USE_PROJ4_LIB // PROJ4 라이브러리, 컴파일 시에만 사용
#endif

// 시계방향. // CW
bool isRightSide(IN const SPoint* P1, IN const SPoint* P2, IN const SPoint* P3); //const
// 반시계 방향. // CCW
bool isLeftSide(IN const SPoint* P1, IN const SPoint* P2, IN const SPoint* P3); //const

// 링크간 경사도
double get_road_slope(long x1, long y1, long z1, long x2, long y2, long z2);
double get_road_slope(int32_t dist, int32_t height);

double getRealWorldDistance(IN const double slng, IN const double slat, IN const double elng, IN const double elat);
bool getPointByDistance(IN const double slng, IN const double slat, IN OUT double& elng, IN OUT double& elat, const double lDistance);
bool getPointByDistanceFromCenter(IN const double slng, IN const double slat, IN const double& elng, IN const double& elat, IN const double lDistance, IN const bool isRight, OUT double& x, OUT double& y);

void extendDataBox(IN OUT SBox& box, IN const double lng, IN const double lat);
void extendDataBox(IN OUT SBox& box, IN const SPoint* pCoords, IN const int nCoordCount);
void extendDataBox(IN OUT SBox& box, IN const SBox& data);

bool isInBox(IN const double lon, IN const double lat, IN const SBox& baseBox, IN const double inMargin = 0);
bool isInPitBox(IN const SBox& inBox, IN const SBox& baseBox);
bool isOnPitBox(IN const SBox& onBox, IN const SBox& baseBox);
//void getClosestPoint(IN const double x1, IN const double y1, IN const double x2, IN const double y2, IN const double ox, IN const double oy, OUT double *a, OUT double *b, IN OUT double *ir);
void getClosestPoint(IN const double lng1, IN const double lat1, IN const double lng2, IN const double lat2, IN const double lng3, IN const double lat3, OUT double *lng, OUT double *lat, IN OUT double *ir);

//////////////////////////////////////////////////////////////////////////////////////////////////////
// POLYGON
bool isPointInPolygon(const double x, const double y, const SPoint *pptPolygon, const int32_t nPolygon);
bool isPointInPolygon(const SPoint *ppt, const SPoint *pptPolygon, const int32_t nPolygon);
SPoint getPolygonCenter(IN const vector<SPoint>& points);
double getPolygonArea(IN const vector<SPoint>& points);



// for PROJ4
#if defined(USE_PROJ4_LIB)
bool initProj4(IN const char* szType = nullptr);
void releaseProj4();
bool translateUTM52NtoWGS84(IN const double lng, IN const double lat, OUT double& x, OUT double& y);
bool translateUTMKtoWGS84(IN const double lng, IN const double lat, OUT double& x, OUT double& y);
#endif // #if defined(USE_PROJ4_LIB)