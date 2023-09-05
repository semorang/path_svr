#pragma once

// #define USE_PROJ4_LIB // PROJ4 라이브러리, 컴파일 시에만 사용

#include "../include/MapDef.h"

// 시계방향. // CW
bool isRightSide(IN const SPoint* P1, IN const SPoint* P2, IN const SPoint* P3); //const
// 반시계 방향. // CCW
bool isLeftSide(IN const SPoint* P1, IN const SPoint* P2, IN const SPoint* P3); //const

// 링크간 경사도
double get_road_slope(long x1, long y1, long z1, long x2, long y2, long z2);
double get_road_slope(int32_t dist, int32_t height);

double getRealWorldDistance(IN const double slng, IN const double slat, IN const double elng, IN const double elat);
bool getPointByDistance(IN const double slng, IN const double slat, IN OUT double& elng, IN OUT double& elat, double lDistance);

bool isInBox(IN const double lon, IN const double lat, IN const SBox& inBox);
bool isInPitBox(IN const SBox& fromBox, IN const SBox& inBox);
void getClosestPoint(IN const double x1, IN const double y1, IN const double x2, IN const double y2, IN const double ox, IN const double oy, OUT double *a, OUT double *b, IN OUT double *ir);

SPoint getPolygonCenter(IN const vector<SPoint>& points);
double GetPolygonArea(IN const vector<SPoint>& points);

// for PROJ4
#if defined(USE_PROJ4_LIB)
bool initProj4(IN const char* szType = nullptr);
void releaseProj4();
bool translateUTM52NtoWGS84(IN const double lng, IN const double lat, OUT double& x, OUT double& y);
bool translateUTMKtoWGS84(IN const double lng, IN const double lat, OUT double& x, OUT double& y);
#endif // #if defined(USE_PROJ4_LIB)