#pragma once

#include <math.h>
#include <vector>
#include "GnType.h"

using namespace std;


#define USE_ROATE_MAP 1 //1:Use rotate function, 0:No rotate

//Don't touch below values.
//const int OSMMaxZoom = 19;
const int OSMMaxZoom = 20;
const int OSMMinZoom = 0;
const int OSMTileWidth = 256;
const int OSMTileHeight = 256;
const int OSMTileBMPMemSize = 262144;
const int g_fastPOWTable[22] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536,131072,262144,524288,1048576, 2097152}; // [20] 1048576, [21] 2097152
//Max Level : 19, Max Tile Count of 19 Level : x(524287), y(524287)
//As a result Full Tile Count of level 19 is => x * y = 524287 * 524287

#if 0 //static methods
//[Reference]
// <Lon./lat. to tile numbers>
// xtile = (2 ^ zoom) * ((lon_deg + 180) / 360)
// ytile = (2 ^ zoom) * (1 - (log(tan(lat_rad) + sec(lat_rad)) / π)) / 2

// <Tile numbers to lon./lat.>
// lon_deg = xtile / (2 ^ zoom) * 360.0 - 180.0
// lat_rad = arctan(sinh(π * (1 - 2 * ytile / (2 ^ zoom))))
// lat_deg = lat_rad * 180.0 / π

static double Longitude2TileX(double Longitude, int nZoom) { 
	return (Longitude + 180.0) / 360.0 * g_fastPOWTable[nZoom]; } //pow(2.0, nZoom);

static double Latitude2TileY(double Latitude, int nZoom) { 
	return (1.0 - std::log(std::tan(Latitude * _PI_DIV_180) + 1.0 / std::cos(Latitude * _PI_DIV_180)) / M_PI) / 2.0 * g_fastPOWTable[nZoom]; } //pow(2.0, nZoom); 

static double TileX2Longitude(double X, int nZoom) {
	return X / g_fastPOWTable[nZoom] * 360.0 - 180; } //pow(2.0, nZoom); 

static double TileY2Latitude(double Y, int nZoom) {
	//const double n = M_PI - _DBL_PI * Y / g_fastPOWTable[nZoom]; 
	//return _180_DIV_PI * std::atan(0.5 * (std::exp(n) - std::exp(-n)));
	return std::atan(std::sinh(M_PI * (1. - 2. * Y / g_fastPOWTable[nZoom]))) * _180_DIV_PI;
}
#else //for more speed
// #define Longitude2TileX(Longitude, nZoom) (double)((Longitude+180.0)/360.0*g_fastPOWTable[nZoom])
// #define Latitude2TileY(Latitude, nZoom) (double)((1.0-std::log(std::tan(Latitude*_PI_DIV_180)+1.0/std::cos(Latitude*_PI_DIV_180))/M_PI)/2.0*g_fastPOWTable[nZoom])
// #define TileX2Longitude(X, nZoom) (double)(X/g_fastPOWTable[nZoom]*360.0-180.0)
// #define TileY2Latitude(Y, nZoom) (double)(std::atan(std::sinh(M_PI*(1.-2.*Y/g_fastPOWTable[nZoom])))*_180_DIV_PI)

#define Longitude2TileX(Longitude, nZoom) (double)(((Longitude)+180.0)/360.0*g_fastPOWTable[nZoom])
#define Latitude2TileY(Latitude, nZoom) (double)((1.0-log(tan((Latitude)*_PI_DIV_180)+1.0/cos((Latitude)*_PI_DIV_180))/M_PI)/2.0*g_fastPOWTable[nZoom])
#define TileX2Longitude(X, nZoom) (double)((double)(X)/g_fastPOWTable[nZoom]*360.0-180.0)
#define TileY2Latitude(Y, nZoom) (double)(atan(sinh(M_PI*(1.-2.*(double)(Y)/g_fastPOWTable[nZoom])))*_180_DIV_PI)

#endif

#define POINTINRECT(x, y, xmin, ymin, xmax, ymax) \
	((x >= xmin && y >= ymin && x <= xmax && y <= ymax) ? true : false)

// #define RECTINRECT(xmin, ymin, xmax, ymax, mapxmin, mapymin, mapxmax, mapymax) \
// 	((xmin >= mapxmin && ymin >= mapymin && xmax <= mapxmax && ymax <= mapymax) ? TRUE : FALSE)

#define RECTONRECT(xmin, ymin, xmax, ymax, mapxmin, mapymin, mapxmax, mapymax) \
	((xmax) >= (mapxmin) && (xmin) <= (mapxmax) && (ymax) >= (mapymin) &&  (ymin) <= (mapymax))
//(!(((xmin) > (mapxmax)) || ((ymin) > (mapymax)) || ((xmax) < (mapxmin)) || ((ymax) < (mapymin))))

typedef struct {
	union {
		long long n64TileID;
		struct {
			long long nTileX : 28; //100?? ???X
			long long nTileY : 28; //100?? ???Y
			long long nOSM_ZoomLevel : 8;
		};
	};
	double fDistanceFrScrCenter; //화면 중앙점과 각 타일간의 거리. 정렬해서 다운로드 및 Rendering 우선순위에 쓰야 함.
	RectN rctOnScreen;
	BoundF rctOnScreenB;
	short nCurShow;
	short nChkInDrawBox;
	int nDataSize;
	unsigned char* pData;
	unsigned int tag; //For real time data version control
} stOSMTileInfo;

/* 이미지 다운로드 경로.. //https://tile.openstreetmap.org/Z/X/Y.png */

#define USE_SINGLETON_PATTERN 1

class CGnOSMCoord {
public:
#if USE_SINGLETON_PATTERN
	static CGnOSMCoord* Instance();
	static void Free();
	static CGnOSMCoord* pInstance;
#endif
	CGnOSMCoord();
	virtual ~CGnOSMCoord();
private:
	void _calcMapBoundRect();

	//for OSM Image
	void _calcStartTilePositions(double& fClientX, double& fClientY, double& fEndClientX, double& fEndClientY, 
								 int& nStartX, int& nStartY, double& fOSMTileWidth, double& fOSMTileHeight,
								 int nOSM_ZoomLevel);
public:
	//==============================================================================
	// Map Pan Method
	//==============================================================================
	void CenterFocus(double x, double y);
	void PanMap(int FrScrX, int FrScrY, int ToScrX, int ToScrY);
	//==============================================================================
	// Map Zoom Method
	//==============================================================================
	bool ScreenToWorld(int inX, int inY, double& outX, double& outY, int nZoomLevel=-1);
	bool ScreenToWorld(const PointN& _ScrPoint, PointF& _WorldPoint, int nZoomLevel=-1);

	bool WorldToScreen(IN const double inX, IN const double inY, OUT int& outX, OUT int& outY, IN int nZoomLevel=-1);
	bool WorldToScreen(const PointF& _WorldPoint, PointN& _ScrPoint, int nZoomLevel=-1);
	bool WorldToScreen(const int nPoint, const PointF* _pWorldPoints, PointN* _pScrPoints, int nZoomLevel=-1);

	//==============================================================================
	// Map 영역계산용
	//==============================================================================
	void InitScreen(long left, long top, long right, long bottom);
	bool SetZoomBounds(const PointF& position1, const PointF& position2);
	bool SetZoomBounds(double x1, double y1, double x2, double y2);

	int  GetZoomLevel() { return (OSMMaxZoom - m_nOSM_ZoomLevel); };
	void SetZoomLevel(int nLevel);
	int  GetOSMZoomLevel() { return m_nOSM_ZoomLevel; };
	void SetOSMZoomLevel(int nOSMLevel);

	void ZoomIn();
	void ZoomIn(int scX, int scY);
	void ZoomOut();
	void ZoomOut(int scX, int scY);
	void InflateRect(RectN* pRect, int w, int h);

	//for Utilize..
	double DistanceBetweenPoints(double psX1, double psY1, double psX2, double psY2); //const COSMCtrlPosition& position1, const COSMCtrlPosition& position2, _Out_opt_ double* pdStartBearing, _Out_opt_ double* pdReverseBearing)
	void CalculateForScaleBar(int& nScaleLength, double& fScaleDistance);
	//void GetLabelPointOfPolygon(PointF *pPoints, long nPoint, PointF *plbPoint);

	//for OSM Image
	int CalcScreenTiles(vector<stOSMTileInfo>& vt_tile, int nOSM_ZoomLevel, unsigned int nDrawGridMapSessionID=0);
	int CalcCurScreenTiles(vector<stOSMTileInfo>& vt_tile, unsigned int nDrawGridMapSessionID=0);
public:
	int m_nOSM_ZoomLevel; //18:최대 확대(동네 수준), 0: 지구 수준 축소. m_nZoomLevel의 반대값. 내부 연산에서만 사용함.
	int m_nZoomLevel; //0:최대 확대(동네 수준), 18: 지구 수준 축소. m_nOSM_ZoomLevel의 반대값.
	RectN m_ScreenRect;
	RectF m_MapRect;
	PointF m_fptMapCenter;
	PointN m_ScreenCenter;
#if USE_ROATE_MAP
	
	double m_fRotateAngle;
	void SetRotate(double fAngle);
#endif
};