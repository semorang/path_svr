#include "../stdafx.h"

#include "GnOSMCoord.h"
#include <algorithm>
#include <complex>


#if USE_SINGLETON_PATTERN
CGnOSMCoord* CGnOSMCoord::pInstance = NULL;
CGnOSMCoord* CGnOSMCoord::Instance() { if (!pInstance) pInstance = new CGnOSMCoord(); return pInstance; }
void CGnOSMCoord::Free() { if (pInstance) delete pInstance; pInstance = NULL; }
#endif


CGnOSMCoord::CGnOSMCoord()
{
	m_nOSM_ZoomLevel = 2; //18:최대 확대(동네 수준), 0: 지구 수준 축소. m_nZoomLevel의 반대값.
	m_nZoomLevel = OSMMaxZoom - m_nOSM_ZoomLevel;
	m_fptMapCenter.x = m_fptMapCenter.y = 0.;
	m_ScreenCenter.x = m_ScreenCenter.y = 0;
#if USE_ROATE_MAP
	m_fRotateAngle = 0.; // 16.;
#endif
}

CGnOSMCoord::~CGnOSMCoord()
{

}

void CGnOSMCoord::InitScreen(long left, long top, long right, long bottom)
{
	m_ScreenRect.left = left;
	m_ScreenRect.top = top;
	m_ScreenRect.right = right;
	m_ScreenRect.bottom = bottom;
	m_ScreenCenter.x = floor((left + right) / 2 + 0.5);
	m_ScreenCenter.y = floor((top + bottom) / 2 + 0.5);
}

void CGnOSMCoord::InflateRect(RectN* pRect, int w, int h)
{
	pRect->left -= w;
	pRect->top -= h;
	pRect->right += w;
	pRect->bottom += h;
}

// void NormalizeLongitude(double& flongitude)
// {
// 	while (flongitude < -180) flongitude += 360;
// 	while (flongitude > 180) flongitude -= 360;
// }

double CGnOSMCoord::DistanceBetweenPoints(double psX1, double psY1, double psX2, double psY2) //const COSMCtrlPosition& position1, const COSMCtrlPosition& position2, _Out_opt_ double* pdStartBearing, _Out_opt_ double* pdReverseBearing)
{
	//First thing to do is setup the constants for the WGS 84 elipsoid
	const double a = 6378137.0;
	const double b = 6356752.314245;
	const double f = (a - b)/a;
	const double asquared = a * a;
	const double bsquared = b * b;
	const double asquaredminusbsquaredoverbsquared = (asquared - bsquared) / bsquared;

	//Convert all our parameters to radians
	const double phi1 = psY1 / 180 * M_PI;
	const double phi2 = psY2 / 180 * M_PI;
	const double lambda1 = psX1 / 180 * M_PI;
	const double lambda2 = psX2 / 180 * M_PI;

	const double omega = lambda2 - lambda1;
	const double tanphi1 = std::tan(phi1);
	const double tanU1 = (1 - f) * tanphi1;
	const double U1 = std::atan(tanU1);
	const double sinU1 = std::sin(U1);
	const double cosU1 = std::cos(U1);

	const double tanphi2 = std::tan(phi2);
	const double tanU2 = (1 - f) * tanphi2;
	const double U2 = std::atan(tanU2);
	const double sinU2 = std::sin(U2);
	const double cosU2 = std::cos(U2);

	const double sinU1sinU2 = sinU1 * sinU2;
	const double cosU1sinU2 = cosU1 * sinU2;
	const double sinU1cosU2 = sinU1 * cosU2;
	const double cosU1cosU2 = cosU1 * cosU2;

	double lambda = omega; //Equation 13

	bool bConverged = false;
	bool bContinueIteration = true;
	double A = 0;
	double sigma = 0;
	double deltasigma = 0;
	while (bContinueIteration)
	{
		//Remember the previous value
		const double lambda0 = lambda;

		const double sinlambda = std::sin(lambda);
		const double coslambda = std::cos(lambda);

		//Equation 14
		double temp = cosU1sinU2 - sinU1cosU2 * coslambda;
		temp *= temp;
		const double sinsquaredsigma = (cosU2 * sinlambda * cosU2 * sinlambda) + temp;
		const double sinsigma = std::sqrt(sinsquaredsigma);

		const double cossigma = sinU1sinU2 + (cosU1cosU2 * coslambda); //Equation 15

		sigma = std::atan2(sinsigma, cossigma); //Equation 16

		const double sinalpha = (sinsquaredsigma == 0) ? 0 : cosU1cosU2 * sinlambda / sinsigma; //Equation 17

		const double alpha = std::asin(sinalpha);
		const double cosalpha = std::cos(alpha);
		const double cossquaredalpha = cosalpha * cosalpha;

		const double cos2sigmam = (cossquaredalpha == 0) ? 0 : cossigma - 2 * sinU1sinU2 / cossquaredalpha; //Equation 18
		const double cossquared2sigmam = cos2sigmam * cos2sigmam;

		const double usquared = cossquaredalpha * asquaredminusbsquaredoverbsquared;

		A = 1 + (usquared / 16384) * (4096 + usquared * (-768 + usquared * (320 - 175 * usquared))); //Equation 3

		const double B = usquared / 1024 * (256 + usquared * (-128 + usquared * (74 - 47 *usquared))); //Equation 4

		deltasigma = B * sinsigma * (cos2sigmam + (B / 4) * (cossigma * (-1 + 2 * cossquared2sigmam) - B / 6 * cos2sigmam * (-3 + 4 * sinsquaredsigma) * (-3 + 4 * cossquared2sigmam))); //Equation 6

		const double C = f / 16 * cossquaredalpha * (4 + f * (4 - 3 * cossquaredalpha)); //Equation 10

		lambda = omega + (1 - C) * f * sinalpha * (sigma + C * sinsigma * (cos2sigmam + C * cossigma * (-1 + 2 * cossquared2sigmam))); //Equation 11 (modified)

		if (std::abs(lambda) > M_PI)
			bContinueIteration = false;
		else
		{
			bConverged = std::abs(lambda - lambda0) < 0.0000000000001;
			if (bConverged)
				bContinueIteration = false;
		}
	}

	const double s = b * A * (sigma - deltasigma); //Equation 19 (return value will be in meters)


	// 아래 구문은 거리값을 구하기 위해 전달받은 두점의 각도값을 구해 주는 것으로 필요하면 풀어서 쓰면 된다.
	// 	if (!bConverged)
	// 	{
	// 		//The points must be anti-podal, in this case we actually have an infinite number of great circles
	// 		if (phi1 > phi2)
	// 		{
	// 			//The great circle returned will by definition be a southerly bearing from position1 to position2
	// 			if (pdStartBearing != nullptr)
	// 				*pdStartBearing = 180;
	// 			if (pdReverseBearing != nullptr)
	// 				*pdReverseBearing = 0;
	// 		}
	// 		else if (phi1 < phi2)
	// 		{
	// 			//The great circle returned will by definition be a northerly bearing from position1 to position2
	// 			if (pdStartBearing != nullptr)
	// 				*pdStartBearing = 0;
	// 			if (pdReverseBearing != nullptr)
	// 				*pdReverseBearing = 180;
	// 		}
	// 		else if (lambda1 > lambda2)
	// 		{
	// 			//The great circle returned will by definition be a easterly bearing from position1 to position2
	// 			if (pdStartBearing != nullptr)
	// 				*pdStartBearing = 90;
	// 			if (pdReverseBearing != nullptr)
	// 				*pdReverseBearing = 270;
	// 		}
	// 		else
	// 		{
	// 			//The great circle returned will by defintion be a westerly bearing from position1 to position2
	// 			if (pdStartBearing != nullptr)
	// 				*pdStartBearing = 270;
	// 			if (pdReverseBearing != nullptr)
	// 				*pdReverseBearing = 90;
	// 		}
	// 		return s;
	// 	}
	// 
	// 	const double sinlambda = sin(lambda);
	// 	const double coslambda = cos(lambda);
	// 
	// 	if (pdStartBearing != nullptr)
	// 	{
	// 		*pdStartBearing = atan2(cosU2 * sinlambda, cosU1sinU2 - sinU1cosU2 * coslambda) * 180 / M_PI; //Equation 20
	// 
	// 		//Ensure bearing is in the conventional range
	// 		if (*pdStartBearing < 0)
	// 			*pdStartBearing += 360;
	// 		if (*pdStartBearing >= 360)
	// 			*pdStartBearing -= 360;
	// 	}  
	// 
	// 	if (pdReverseBearing != nullptr)
	// 	{  
	// 		*pdReverseBearing = (atan2(cosU1 * sinlambda, -sinU1cosU2 + cosU1sinU2 * coslambda) + M_PI) * 180 / M_PI; //Equation 21
	// 
	// 		if (*pdReverseBearing < 0)
	// 			*pdReverseBearing += 360;
	// 		if (*pdReverseBearing >= 360)
	// 			*pdReverseBearing -= 360;
	// 	}  

	return s;
}

bool CGnOSMCoord::ScreenToWorld(const PointN& _ScrPoint, PointF& _WorldPoint, int nZoomLevel)
{
	return ScreenToWorld(_ScrPoint.x, _ScrPoint.y, _WorldPoint.x, _WorldPoint.y, nZoomLevel);
}

bool CGnOSMCoord::ScreenToWorld(int inX, int inY, double& outX, double& outY, int nZoomLevel) //It was named to ClientToPosition in OSM
{
	if (nZoomLevel < 0) nZoomLevel = m_nOSM_ZoomLevel;
	//What will be the return value from this function (assume the worst)

	//Do the calculations
	double fX = Longitude2TileX(m_fptMapCenter.x, nZoomLevel);
	double fY = Latitude2TileY(m_fptMapCenter.y, nZoomLevel);

	//double fInt = 0;
	//const double fFractionalZoom = 0; //modf(fZoom, &fInt); // Zoom level간 Free Scale 인 경우 사용함. 일단은 고정 scacle 이므로 0 으로 고정함.
	//const double fOSMTileWidth = static_cast<double>(OSMTileWidth + (fFractionalZoom * OSMTileWidth));
	//const double fOSMTileHeight = static_cast<double>(OSMTileHeight + (fFractionalZoom * OSMTileHeight));

#if USE_ROATE_MAP
	// const double fDeltaX = (static_cast<double>(inX) - (m_ScreenRect.right - m_ScreenRect.left)/2.);
	// const double fDeltaY = (static_cast<double>(inY) - (m_ScreenRect.bottom - m_ScreenRect.top)/2.);
	const double fDeltaX = (static_cast<double>(inX) - m_ScreenCenter.x);
	const double fDeltaY = (static_cast<double>(inY) - m_ScreenCenter.y);

	//Rotate the position around the center position
	const double fRadBearing = m_fRotateAngle * 0.017453292519943295769236907684886; 
	const double fCosBearing = cos(fRadBearing);
	const double fSinBearing = sin(fRadBearing);
	const double fRotateX = (fDeltaX*fCosBearing) - (fDeltaY*fSinBearing);
	const double fRotateY = (fDeltaX*fSinBearing) + (fDeltaY*fCosBearing);

	fX += (fRotateX / static_cast<double>(OSMTileWidth));
	fY += (fRotateY / static_cast<double>(OSMTileHeight));
#else 
	fX += (static_cast<double>(inX-(m_ScreenRect.right - m_ScreenRect.left)/2.0)/static_cast<double>(OSMTileWidth));
	fY += (static_cast<double>(inY-(m_ScreenRect.bottom - m_ScreenRect.top)/2.0)/static_cast<double>(OSMTileHeight));
#endif
	const double fMaxTile = g_fastPOWTable[nZoomLevel]; //pow(2.0, (double)nZoomLevel);

	//Wrap the longitude if necessary
	while (fX < 0) fX += fMaxTile;
	while (fX >= fMaxTile) fX -= fMaxTile;

	if ((fX >= 0) && (fX <= fMaxTile) && (fY >= 0) && (fY <= fMaxTile)) {
		outY = TileY2Latitude(fY, nZoomLevel);
		outX = TileX2Longitude(fX, nZoomLevel);
		return true;
	}

	return false;
}

bool CGnOSMCoord::WorldToScreen(const PointF& _WorldPoint, PointN& _ScrPoint, int nZoomLevel)
{
	return WorldToScreen(_WorldPoint.x, _WorldPoint.y, (int&)_ScrPoint.x, (int&)_ScrPoint.y, nZoomLevel);
}

bool CGnOSMCoord::WorldToScreen(const int nPoint, const PointF* _pWorldPoints, PointN* _pScrPoints, int nZoomLevel)
{
	for (int i=0; i<nPoint; ++i) {
		WorldToScreen(_pWorldPoints->x, _pWorldPoints->y, (int&)_pScrPoints->x, (int&)_pScrPoints->y, nZoomLevel);
		++_pScrPoints;
		++_pWorldPoints;
	} //for
	return true;
}

bool CGnOSMCoord::WorldToScreen(IN const double inX, IN const double inY, OUT int& outX, OUT int& outY, IN int nZoomLevel) //It was named to PositionToClient in OSM
{
	if (nZoomLevel < 0) nZoomLevel = m_nOSM_ZoomLevel;
	//What will be the return value from this function (assume the worst)

	//Do the calculations
	const double fXCenter = Longitude2TileX(m_fptMapCenter.x, nZoomLevel);
	const double fYCenter = Latitude2TileY(m_fptMapCenter.y, nZoomLevel);
	const double fX = Longitude2TileX(inX, nZoomLevel);
	const double fY = Latitude2TileY(inY, nZoomLevel);
	const double fMaxTile = g_fastPOWTable[nZoomLevel];// pow(2.0, (double)nZoomLevel);

	if ((fX >= 0) && (fX <= fMaxTile) && (fY >= 0) && (fY <= fMaxTile))
	{
#if USE_ROATE_MAP //화면의 Surface를 직접돌리는 경우 0으로 셋팅해야 함.
		double fInt = 0;
		const double fFractionalZoom = 0; //modf(fZoom, &fInt);
		const double fOSMTileWidth = OSMTileWidth + (fFractionalZoom * OSMTileWidth);
		const double fOSMTileHeight = OSMTileHeight + (fFractionalZoom * OSMTileHeight);
		const double fDeltaX = (fX - fXCenter) * fOSMTileWidth;
		const double fDeltaY = (fY - fYCenter) * fOSMTileHeight;

		//Rotate the position around the center position
		const double fRadBearing = -m_fRotateAngle * 0.017453292519943295769236907684886; 
		const double fCosBearing = cos(fRadBearing);
		const double fSinBearing = sin(fRadBearing);
		const double fRotateX = fDeltaX*fCosBearing - fDeltaY*fSinBearing;
		const double fRotateY = fDeltaX*fSinBearing + fDeltaY*fCosBearing;
		// outX = static_cast<double>((m_ScreenRect.right - m_ScreenRect.left) / 2.0 + m_ScreenRect.left + fRotateX);
		// outY = static_cast<double>((m_ScreenRect.bottom - m_ScreenRect.top) / 2.0 + m_ScreenRect.top + fRotateY);
		outX = static_cast<double>(m_ScreenCenter.x + m_ScreenRect.left + fRotateX);
		outY = static_cast<double>(m_ScreenCenter.y + m_ScreenRect.top + fRotateY);
#else //No use rotate function
		double fInt = 0;
		const double fFractionalZoom = 0; //modf(fZoom, &fInt);
		const double fOSMTileWidth  = static_cast<double>(OSMTileWidth + (fFractionalZoom * OSMTileWidth));
		const double fOSMTileHeight = static_cast<double>(OSMTileHeight + (fFractionalZoom * OSMTileHeight));
		outX = static_cast<int>((m_ScreenRect.right - m_ScreenRect.left) / 2.0 + m_ScreenRect.left + ((fX - fXCenter) * fOSMTileWidth));
		outY = static_cast<int>((m_ScreenRect.bottom - m_ScreenRect.top) / 2.0 + m_ScreenRect.top + ((fY - fYCenter) * fOSMTileHeight));
#endif
		return true;
	}
	return false;
}

void CGnOSMCoord::ZoomIn() //마우스를 몸밖으로 밀어내면 확대, 마우스를 몸쪽으로 당기면 축소
{
	SetZoomLevel( m_nZoomLevel - 1 ); 
}

void CGnOSMCoord::ZoomIn(int curX, int curY) //마우스를 몸밖으로 밀어내면 확대, 마우스를 몸쪽으로 당기면 축소
{
	SetZoomLevel(m_nZoomLevel - 1);
	int scrCenX = m_ScreenCenter.x, scrCenY = m_ScreenCenter.y;
	double wdX, wdY;
	scrCenX += (curX - scrCenX), scrCenY += (curY - scrCenY);			//Zoom In
	ScreenToWorld(scrCenX, scrCenY, wdX, wdY);
	CenterFocus(wdX, wdY);
}

void CGnOSMCoord::ZoomOut() //마우스를 몸밖으로 밀어내면 확대, 마우스를 몸쪽으로 당기면 축소
{
	SetZoomLevel( m_nZoomLevel + 1 ); 
}

void CGnOSMCoord::ZoomOut(int curX, int curY) //마우스를 몸밖으로 밀어내면 확대, 마우스를 몸쪽으로 당기면 축소
{
	SetZoomLevel(m_nZoomLevel + 1);
	int scrCenX = m_ScreenCenter.x, scrCenY = m_ScreenCenter.y;
	double wdX, wdY;
	scrCenX -= ((curX - scrCenX) * 0.5), scrCenY -= ((curY - scrCenY) * 0.5);	//Zoom Out
	ScreenToWorld(scrCenX, scrCenY, wdX, wdY);
	CenterFocus(wdX, wdY);
}

void CGnOSMCoord::PanMap(int FrScrX, int FrScrY, int ToScrX, int ToScrY)
{
#if USE_ROATE_MAP
	//Work out the size of a tile at the current zoom level
	double fInt = 0;
	const double fFractionalZoom = 0; //modf(fZoom, &fInt); // Zoom level간 Free Scale 인 경우 사용함. 일단은 고정 scacle 이므로 0 으로 고정함.
	const double fOSMTileWidth = static_cast<double>(OSMTileWidth + (fFractionalZoom * OSMTileWidth));
	const double fOSMTileHeight = static_cast<double>(OSMTileHeight + (fFractionalZoom * OSMTileHeight));
	//Work out the longitude and latitude of the position where we have dragged to
	const double fDeltaX = (static_cast<double>(ToScrX) - FrScrX) / fOSMTileWidth;
	const double fDeltaY = (static_cast<double>(ToScrY) - FrScrY) / fOSMTileHeight;

	const double fRadBearing = m_fRotateAngle * 0.017453292519943295769236907684886; // m_fRotateAngle 화면 회전..
	const double fCosBearing = cos(fRadBearing);
	const double fSinBearing = sin(fRadBearing);
	const double fX = Longitude2TileX(m_fptMapCenter.x, m_nOSM_ZoomLevel) - (fDeltaX*fCosBearing - fDeltaY*fSinBearing);
	const double fNewLongitude = TileX2Longitude(fX, m_nOSM_ZoomLevel);
	const double fY = Latitude2TileY(m_fptMapCenter.y, m_nOSM_ZoomLevel) - (fDeltaX*fSinBearing + fDeltaY*fCosBearing);
	const double fNewLatitude = TileY2Latitude(fY, m_nOSM_ZoomLevel);
#else
	const double fX = Longitude2TileX(m_fptMapCenter.x, m_nOSM_ZoomLevel) - ((static_cast<double>(ToScrX) - FrScrX)/static_cast<double>(OSMTileWidth));
	const double fNewLongitude = TileX2Longitude(fX, m_nOSM_ZoomLevel);
	const double fY = Latitude2TileY(m_fptMapCenter.y, m_nOSM_ZoomLevel) - ((static_cast<double>(ToScrY) - FrScrY)/static_cast<double>(OSMTileHeight));
	const double fNewLatitude = TileY2Latitude(fY, m_nOSM_ZoomLevel);
#endif

	//Move to the new position
	CenterFocus(fNewLongitude, fNewLatitude);
}

void CGnOSMCoord::SetZoomLevel(int nLevel) { //0:최대 확대(동네 수준), 19: 지구 수준 축소. m_nOSM_ZoomLevel의 반대값.
	//Validate our parameters
	if ((nLevel < OSMMinZoom) || (nLevel > OSMMaxZoom)) return;
	m_nOSM_ZoomLevel = OSMMaxZoom - nLevel;
	_calcMapBoundRect();
}

void CGnOSMCoord::SetOSMZoomLevel(int nOSMLevel) {
	if ((nOSMLevel < OSMMinZoom) || (nOSMLevel > OSMMaxZoom)) return;
	m_nOSM_ZoomLevel = nOSMLevel;
	_calcMapBoundRect();
}

void CGnOSMCoord::CenterFocus(double x, double y) {
	m_fptMapCenter.x = x;
	m_fptMapCenter.y = y;
	_calcMapBoundRect();
}

void CGnOSMCoord::_calcMapBoundRect() {
	double x1, x2, y1, y2;
	ScreenToWorld(m_ScreenRect.left, m_ScreenRect.top, x1, y1);
	ScreenToWorld(m_ScreenRect.right, m_ScreenRect.bottom, x2, y2);
	m_MapRect.left = (x1 <= x2) ? x1 : x2;
	m_MapRect.top = (y1 <= y2) ? y1 : y2;
	m_MapRect.right = (x1 >= x2) ? x1 : x2;
	m_MapRect.bottom = (y1 >= y2) ? y1 : y2;

	m_nZoomLevel = OSMMaxZoom - m_nOSM_ZoomLevel;
}

bool CGnOSMCoord::SetZoomBounds(const PointF& position1, const PointF& position2) {
	return SetZoomBounds(position1.x, position1.y, position2.x, position2.y); 
}

bool CGnOSMCoord::SetZoomBounds(double x1, double y1, double x2, double y2) {
 	double fTopLatitude = -91;
	double fBottomLatitude = 91;
	double fLeftLongitude = 181;
	double fRightLongitude = -181;

	double _x1 = (x1 < x2) ? x1 : x2;
	double _x2 = (x1 > x2) ? x1 : x2;
	double _y1 = (y1 < y2) ? y1 : y2;
	double _y2 = (y1 > y2) ? y1 : y2;

	if (_y1 > fTopLatitude)		fTopLatitude = _y1;
	if (_y1 < fBottomLatitude)	fBottomLatitude = _y1;
	if (_x1 < fLeftLongitude)	fLeftLongitude = _x1;
	if (_x1 > fRightLongitude)	fRightLongitude = _x1;
	if (_y2 > fTopLatitude)		fTopLatitude = _y2;
	if (_y2 < fBottomLatitude)	fBottomLatitude = _y2;
	if (_x2 < fLeftLongitude)	fLeftLongitude = _x2;
	if (_x2 > fRightLongitude)	fRightLongitude = _x2;

	//Next find the center position of the extreme latitudes and longitudes found above
	CenterFocus(((fLeftLongitude + fRightLongitude) / 2.), ((fTopLatitude + fBottomLatitude) / 2.));

	//Starting from the highest zoom level going down, determine which will contain position1 and position2 in the client area

	//Form the extreme bounding positions
	int nFoundZoomLevel = -1;
	for (int i=OSMMaxZoom; (i>=OSMMinZoom) && (nFoundZoomLevel == -1); i--)
	{
		//Determine if the current zoom level will contain the extreme positions
		PointN ptTopLeft;
		PointN ptBottomRight;
		WorldToScreen(fLeftLongitude, fTopLatitude, (int&)ptTopLeft.x, (int&)ptTopLeft.y, i);
		WorldToScreen(fRightLongitude, fBottomLatitude, (int&)ptBottomRight.x, (int&)ptBottomRight.y, i);
		if (POINTINRECT(ptTopLeft.x, ptTopLeft.y, m_ScreenRect.left, m_ScreenRect.top, m_ScreenRect.right, m_ScreenRect.bottom) &&
			POINTINRECT(ptBottomRight.x, ptBottomRight.y, m_ScreenRect.left, m_ScreenRect.top, m_ScreenRect.right, m_ScreenRect.bottom))	
		{
			nFoundZoomLevel = i;
		}
	}

	if (nFoundZoomLevel != -1)
		m_nOSM_ZoomLevel = nFoundZoomLevel;

	_calcMapBoundRect();

	return true;
}

#if USE_ROATE_MAP
void CGnOSMCoord::SetRotate(double fAngle)
{
	while (fAngle < 0) fAngle += 360;
	while (fAngle > 360) fAngle -= 360;
	m_fRotateAngle = fAngle;
}
#endif

void CGnOSMCoord::CalculateForScaleBar(int& nScaleLength, double& fScaleDistance)
{
	//Work out the distance between the center point and either a full tile to the left or right (or half a tile if the zoom level is 0, This is 
	//to ensure we do not end up at the point we started at because at zoom level 0 one tile completely wraps the earth)
	double fX = Longitude2TileX(m_fptMapCenter.x, m_nOSM_ZoomLevel);
	fX += (m_nOSM_ZoomLevel == 0 ? 0.5 : 1);
	const double fLongitude2 = TileX2Longitude(fX, m_nOSM_ZoomLevel);
	double fDistance = DistanceBetweenPoints(m_fptMapCenter.x, m_fptMapCenter.y, fLongitude2, m_fptMapCenter.y) / 1000.; //We want KM not meters!
	if (m_nOSM_ZoomLevel == 0)
		fDistance *= 2;

	//Convert the distance to miles if necessary
	bool bMetric = true;//UseMetric(); -> true: meter, false: mile, 마일단위를 쓰고 싶으면 bMetric를 false로
	if (!bMetric) fDistance *= 0.621371192;

	//Next lets scale down the measure tile width to a most significant digit unit
	double fMSDUnit = pow(10, floor(log10(fDistance)));
	fScaleDistance = fMSDUnit;
	while (fScaleDistance < fDistance)
		fScaleDistance += fMSDUnit;

	// 	double fInt = 0;
	// 	const double fFractionalZoom = 0; //modf(m_fZoom, &fInt);
	// 	const double fOSMTileWidth = static_cast<double>(OSMTileWidth + (fFractionalZoom * OSMTileWidth));
	//	nScaleLength = static_cast<int>((fScaleDistance / fDistance * static_cast<double>(m_nOSM_ZoomLevel == 0 ? fOSMTileWidth/2. : fOSMTileWidth)) + 0.5);

	nScaleLength = static_cast<int>((fScaleDistance / fDistance * static_cast<double>(m_nOSM_ZoomLevel == 0 ? OSMTileWidth/2. : OSMTileWidth)) + 0.5);
}

void CGnOSMCoord::_calcStartTilePositions(double& fClientX, double& fClientY, double& fEndClientX, double& fEndClientY, 
										  int& nStartTileX, int& nStartTileY, double& fOSMTileWidth, double& fOSMTileHeight, 
										  int nOSM_ZoomLevel)
{
	//First thing we need to do is get the X and Y values for the center point of the map
	double fStartX = Longitude2TileX(m_fptMapCenter.x, nOSM_ZoomLevel);
	nStartTileX = static_cast<int>(fStartX);
	double fStartY = Latitude2TileY(m_fptMapCenter.y, nOSM_ZoomLevel);
	nStartTileY = static_cast<int>(fStartY);

	//Work out the size of a tile at the current zoom level
	double fInt = 0;
	double fFractionalZoom = 0; //modf(m_fZoom, &fInt);
	fOSMTileWidth = OSMTileWidth + (fFractionalZoom * OSMTileWidth);
	fOSMTileHeight = OSMTileHeight + (fFractionalZoom * OSMTileHeight);

#if USE_ROATE_MAP
	double fHalfHeight = (m_ScreenRect.bottom - m_ScreenRect.top) / 2.0;
	double fClientRadiusY = fHalfHeight;
	double fHalfWidth = (m_ScreenRect.right - m_ScreenRect.left) / 2.0;
	double fClientRadiusX = fHalfWidth;
	if (m_fRotateAngle != 0.) {
		fClientRadiusX = max(fClientRadiusX, fClientRadiusY);
		fClientRadiusX *= sqrt(2.0);
		fClientRadiusY = fClientRadiusX;
	}
	fEndClientX = m_ScreenRect.left + fHalfWidth + fClientRadiusX;
	fEndClientY = m_ScreenRect.top + fHalfHeight + fClientRadiusY;
#else
	const double fHalfHeight = (m_ScreenRect.bottom - m_ScreenRect.top) / 2.0;
	const double fClientRadiusY = fHalfHeight;
	const double fHalfWidth = (m_ScreenRect.right - m_ScreenRect.left) / 2.0;
	const double fClientRadiusX = fHalfWidth;
	fEndClientX = m_ScreenRect.right;
	fEndClientY = m_ScreenRect.bottom;
#endif //#ifdef USE_ROATE_MAP

	//Next we need to find the X and Y values which occur just before the top left position of the client area
	double fCenterX = m_ScreenRect.left + fHalfWidth;
	double fCenterY = m_ScreenRect.top + fHalfHeight;
	fClientX = fCenterX - (modf(fStartX, &fInt) * fOSMTileWidth);
	while ((fClientX + fClientRadiusX) > fCenterX) {
		--nStartTileX;
		fClientX -= fOSMTileWidth;
	}
	fClientY = fCenterY - (modf(fStartY, &fInt) * fOSMTileHeight);
	while ((fClientY + fClientRadiusY) > fCenterY) {
		--nStartTileY;
		fClientY -= fOSMTileWidth;
	}
}

bool CpOSMTile(stOSMTileInfo& Fr, stOSMTileInfo& Bk) { return (Fr.fDistanceFrScrCenter < Bk.fDistanceFrScrCenter); }
int CGnOSMCoord::CalcCurScreenTiles(vector<stOSMTileInfo>& vt_tile, unsigned int nDrawGridMapSessionID) { return CalcScreenTiles(vt_tile, m_nOSM_ZoomLevel, nDrawGridMapSessionID); }
int CGnOSMCoord::CalcScreenTiles(vector<stOSMTileInfo>& vt_tile, int nOSM_ZoomLevel, unsigned int nDrawGridMapSessionID)
{
	char szTmp[64];
	int nCnt = 0;
	//Calculate the starting position for the tiles
	double fClientX = 0;
	double fClientY = 0;
	double fEndClientX = 0;
	double fEndClientY = 0;
	int nStartTileX = 0;
	int nStartTileY = 0;
	double fOSMTileWidth = 0;
	double fOSMTileHeight = 0;
	_calcStartTilePositions(fClientX, fClientY, fEndClientX, fEndClientY, nStartTileX, nStartTileY, fOSMTileWidth, fOSMTileHeight, nOSM_ZoomLevel);

	const int nMaxTile = static_cast<int>(g_fastPOWTable[nOSM_ZoomLevel]); //  pow(2.0, m_nOSM_ZoomLevel));

	//Only draw the tile if it intersects the clip rect
	double fClientCenterX = (m_ScreenRect.right + m_ScreenRect.left) / 2.f;
	double fClientCenterY = (m_ScreenRect.bottom + m_ScreenRect.top) / 2.f;

	double fY = static_cast<double>(fClientY);
	int nTileY = nStartTileY;

	vt_tile.clear();
	vt_tile.reserve(100);

	while (fY <= fEndClientY) {
		double fX = static_cast<double>(fClientX);
		int nTileX = nStartTileX;
		while (fX <= fEndClientX) {
			//Perform wrapping of invalid x tile values to valid values
			while (nTileX < 0) nTileX += nMaxTile;
			while (nTileX >= nMaxTile) nTileX -= nMaxTile;
			//Form the rect to the tile
			RectN rTile = { static_cast<long>(fX),
							static_cast<long>(fY),
							static_cast<long>(fX) + static_cast<long>(fOSMTileWidth),
							static_cast<long>(fY) + static_cast<long>(fOSMTileHeight) };
			
			//if (RECTONRECT(rTile.left, rTile.top, rTile.right, rTile.bottom, m_ScreenRect.left, m_ScreenRect.top, m_ScreenRect.right, m_ScreenRect.bottom))
			{
				if ((nTileY >= 0) && (nTileY < nMaxTile)) {
					//Calculate the distance of the tile to the center of the client area. Note we use the pixel distance rather than the
					//true elipsoid distance as a speed optimization
					const double fTileXCenter = static_cast<double>(fX) + (fOSMTileWidth / 2) - fClientCenterX;
					const double fTileYCenter = static_cast<double>(fY) + (fOSMTileHeight / 2) - fClientCenterY;
					const double fDistanceFrScrCenter = sqrt((fTileXCenter * fTileXCenter) + (fTileYCenter * fTileYCenter));

					stOSMTileInfo tileInfo = { 0, };
					//아래 내용을 저장 하면 됨.
					tileInfo.nTileX = nTileX;
					tileInfo.nTileY = nTileY;
					tileInfo.fDistanceFrScrCenter = fDistanceFrScrCenter; //화면 중앙점과 각 타일간의 거리. 정렬해서 다운로드 및 Rendering 우선순위에 쓰야 함.
					tileInfo.nOSM_ZoomLevel = nOSM_ZoomLevel;
					tileInfo.rctOnScreen.left = rTile.left; tileInfo.rctOnScreen.top = rTile.top; tileInfo.rctOnScreen.right = rTile.right; tileInfo.rctOnScreen.bottom = rTile.bottom;
					tileInfo.pData = NULL;
					tileInfo.nDataSize = 0;
					tileInfo.nCurShow = 1;
					tileInfo.nChkInDrawBox = 0; // (m_ScreenRect.left <= rTile.left && m_ScreenRect.top <= rTile.top && rTile.right < m_ScreenRect.right && rTile.bottom < m_ScreenRect.bottom);
					tileInfo.tag = nDrawGridMapSessionID;
					vt_tile.push_back(tileInfo);
					nCnt++;
				}
				else {
					//Out of earth! You can draw re the not available cell..
				} //if
			} //if RTonRT
			//Move onto the next column
			fX += static_cast<double>(fOSMTileWidth);
			++nTileX;
		} //while
		//Move onto the next row
		fY += static_cast<double>(fOSMTileHeight);
		++nTileY;
	} //while

	if (vt_tile.size() != nCnt)
		return -1;

	sort(vt_tile.begin(), vt_tile.end(), CpOSMTile);

/*
#if USE_ROATE_MAP
	//회전 변환 대응..
	//x1, y1 : 원 영상의 좌표
	//x0, y0 : 회전 기준점(회전의 중심)
	//x2 = cos(a) * (x1 - x0) - sin(a) * (y1 - y0) + x0
	//y2 = sin(a) * (x1 - x0) + cos(a) * (y1 - y0) + y0
	{
		double x1, y1;
		double x0 = fClientCenterX;
		double y0 = fClientCenterY;
		double a = -m_fRotateAngle * 0.017453292519943295769236907684886; // PI/180
		double sin_a = sin(a);
		double cos_a = cos(a);
		for (vector<stOSMTileInfo>::iterator loc = vt_tile.begin(); loc != vt_tile.end(); ++loc) {
			x1 = loc->rctOnScreen.left; y1 = loc->rctOnScreen.top;
			loc->rctOnScreenB.lt.x = cos_a * (x1 - x0) - sin_a * (y1 - y0) + x0;
			loc->rctOnScreenB.lt.y = sin_a * (x1 - x0) + cos_a * (y1 - y0) + y0;

			x1 = loc->rctOnScreen.right; y1 = loc->rctOnScreen.top;
			loc->rctOnScreenB.rt.x = cos_a * (x1 - x0) - sin_a * (y1 - y0) + x0;
			loc->rctOnScreenB.rt.y = sin_a * (x1 - x0) + cos_a * (y1 - y0) + y0;

			x1 = loc->rctOnScreen.right; y1 = loc->rctOnScreen.bottom;
			loc->rctOnScreenB.rb.x = cos_a * (x1 - x0) - sin_a * (y1 - y0) + x0;
			loc->rctOnScreenB.rb.y = sin_a * (x1 - x0) + cos_a * (y1 - y0) + y0;

			x1 = loc->rctOnScreen.left; y1 = loc->rctOnScreen.bottom;
			loc->rctOnScreenB.lb.x = cos_a * (x1 - x0) - sin_a * (y1 - y0) + x0;
			loc->rctOnScreenB.lb.y = sin_a * (x1 - x0) + cos_a * (y1 - y0) + y0;
		} //for
	}
#else
	for (vector<stOSMTileInfo>::iterator loc = vt_tile.begin(); loc != vt_tile.end(); ++loc) {
		loc->rctOnScreenB.lt.x = loc->rctOnScreen.left; loc->rctOnScreenB.lt.y = loc->rctOnScreen.top;
		loc->rctOnScreenB.rt.x = loc->rctOnScreen.right; loc->rctOnScreenB.rt.y = loc->rctOnScreen.top;
		loc->rctOnScreenB.rb.x = loc->rctOnScreen.right; loc->rctOnScreenB.rb.y = loc->rctOnScreen.bottom;
		loc->rctOnScreenB.lb.x = loc->rctOnScreen.left; loc->rctOnScreenB.lb.y = loc->rctOnScreen.bottom;
	} //for
#endif
*/
	return nCnt;
}

/*
//============================================================================
// Get Label point of Polygon. the result is not a center point of polygon, gijoe.
//============================================================================
#define ORIENTATION(ax,ay,bx,by,cx,cy) ((ax-cx)*(by-cy)-(ay-cy)*(bx-cx))
#define POINTLOCATION(A,B,P) ((((B.x - A.x)* (P.y - A.y) - (B.y - A.y) * (P.x - A.x)) > 0) ? 1 : -1)
#define DISTANCE(A,B,C) (abs((B.x-A.x)*(A.y-C.y)-(B.y-A.y)*(A.x-C.x)))

void simpleHull(PointF *pPoints, long nPoint, long minPoint, long maxPoint, PointF *pHull) { 
	if (nPoint < 3) return;  
	double minX = 2147483647.0; 
	double maxX = -2147483648.0;
	int i = 0, j = 0, k = 0;
	PointF A = pPoints[minPoint]; 
	PointF B = pPoints[maxPoint]; ; 
	long *leftSet = new long[nPoint];
	long *rightSet = new long[nPoint];
	for (i = 0; i < nPoint - 1; i++) { 
		if(i == minPoint || i == maxPoint) continue;
		PointF p = pPoints[i]; 
		if (POINTLOCATION(A,B,p) == -1) { leftSet[j] = i; j++;
		} else { rightSet[k] = i;  k ++; }
	}

	double dist = -2147483648.0; 
	int leftfurthestPoint = -1; 
	int rightfurthestPoint = -1; 

	for(i = 0; i < j; i ++) {
		PointF p = pPoints[leftSet[i]]; 
		double distance1 = DISTANCE(A, B, p); // distance(A, B, p);
		if (distance1 > dist) { dist = distance1; leftfurthestPoint = leftSet[i]; } 
	}

	dist = -2147483648.0; 
	for(i = 0; i < k; i ++) {
		PointF p = pPoints[rightSet[i]]; 
		double distance2 = DISTANCE(A, B, p); //distance(A,B,p); 
		if (distance2 > dist) { dist = distance2; rightfurthestPoint = rightSet[i]; } 
	} 	

	long sort[4]; memset(sort, 0, sizeof(long)* 4) ;
	long sortcount = 4;
	if(rightfurthestPoint != -1) {
		sort[0] = rightfurthestPoint;
		sort[1] = minPoint;
		sort[2] = maxPoint;
		sortcount = 3;
	} else if(leftfurthestPoint != -1)  {
		sort[0] = minPoint;
		sort[1] = leftfurthestPoint;
		sort[2] = maxPoint;
		sortcount = 3;
	} else {
		sort[0] = rightfurthestPoint;
		sort[1] = minPoint;
		sort[2] = leftfurthestPoint;
		sort[3] = maxPoint;
	}
	long tmp = 0;
	for(i = 0 ; i < sortcount; i++) {
		if(sort[0] > sort[i]) { tmp = sort[0]; sort[0] = sort[i]; sort[i] = tmp; }
		if(sort[sortcount - 1] < sort[i]) { tmp = sort[sortcount - 1]; sort[sortcount - 1] = sort[i]; sort[i] = tmp; }
	}
	if(sortcount == 4) {
		if(sort[1] > sort[2]) { tmp = sort[1]; sort[1] = sort[2]; sort[2] = tmp; }
	}
	pHull[0] = pPoints[sort[0]];
	pHull[1] = pPoints[sort[1]];
	pHull[2] = pPoints[sort[2]];
	pHull[3] = pPoints[sort[3]];

	delete []leftSet;
	delete []rightSet;
} 

void GetMBR_EX(PointF *dpPoints, int nCount, RectF *dMbr, int *Or)
{
	long v_xmin, v_ymin, v_xmax, v_ymax;
	double area = 0.0;
	dMbr->left = dMbr->top = 999999999.9;
	dMbr->bottom = dMbr->right = -999999999.9;
	for (int i = 0; i < nCount; i++) {
		if (dpPoints[i].x > dMbr->right) { dMbr->right = dpPoints[i].x; v_xmax = i; }
		if (dpPoints[i].x < dMbr->left) { dMbr->left = dpPoints[i].x; v_xmin = i; }
		if (dpPoints[i].y > dMbr->bottom) { dMbr->bottom = dpPoints[i].y; v_ymax = i; }
		if (dpPoints[i].y < dMbr->top) { dMbr->top = dpPoints[i].y; v_ymin = i; }
	}
	PointF shull[4]; memset(shull, 0, sizeof(PointF) * 4);
	simpleHull(dpPoints, nCount, v_xmin, v_xmax, shull);
	area = ORIENTATION(shull[0].x, shull[0].y, shull[1].x, shull[1].y, shull[2].x, shull[2].y);
	if(area > 0.0) *Or = 1; //CCW
	else *Or = 0; //CW
}

bool PointInPolygon(PointF P, PointF* pPoints, long nPoint) {
	int polySides = nPoint - 1;
	int i, j = polySides -1 ;
	bool oddNodes = false ;
	for (i = 0; i<polySides; i++) {
		if (pPoints[i].y < P.y && pPoints[j].y >= P.y || pPoints[j].y < P.y && pPoints[i].y >= P.y)
			if( pPoints[i].x + (P.y - pPoints[i].y) / (pPoints[j].y - pPoints[i].y) * (pPoints[j].x - pPoints[i].x) < P.x)
				oddNodes = !oddNodes; 
		j = i; 
	}
	return oddNodes; 
} 

PointF getCentroid(PointF *pPoints, long nPoint) {
	double cx = 0, cy = 0;
	int i, j, n = nPoint;
	//double area = getArea(pPoints, nPoint);
	double area = 0.0;
	for (i = 0; i < nPoint - 1; i++)
		area += (pPoints[i].x * pPoints[i + 1].y) - (pPoints[i + 1].x * pPoints[i].y);
	area += (pPoints[nPoint - 1].x * pPoints[0].y) - (pPoints[0].x * pPoints[nPoint - 1].y);
	area = fabs(area / 2.0);

	PointF res;
	double factor = 0;
	for (i = 0; i < n; i++) {
		j = (i + 1) % n;
		factor = (pPoints[i].x * pPoints[j].y - pPoints[j].x * pPoints[i].y);
		cx += (pPoints[i].x + pPoints[j].x) * factor;
		cy += (pPoints[i].y + pPoints[j].y) * factor;
	}
	area *= 6.0f;
	factor = 1 / area;
	cx *= factor;
	cy *= factor;
	res.x = cx;
	res.y = cy;
	return res;
}

void CGnOSMCoord::GetLabelPointOfPolygon(PointF *pPoints, long nPoint, PointF *plbPoint) {
	int i, j, k;
	int o = 0;
	RectF oMbr;
	GetMBR_EX(pPoints, nPoint, &oMbr, &o);
	PointF center;

	plbPoint->x = (oMbr.right - oMbr.left) / 2.0 + oMbr.left;
	plbPoint->y = (oMbr.bottom - oMbr.top) / 2.0 + oMbr.top;
	center = *plbPoint;
	if(PointInPolygon(*plbPoint, pPoints, nPoint) == true) return;
	*plbPoint = getCentroid(pPoints, nPoint);
	if(PointInPolygon(*plbPoint, pPoints, nPoint) == true) return;
	double minDist = 99999999.9;
	double maxArea = -99999999.9;
	PointF bestPt1, bestPt2;
	PointF tempPt;
	for(i = 0; i < nPoint - 1; i ++) {
		if(i == 0) k = nPoint - 2; else k = i - 1;
		j = i + 1;
		double area = ORIENTATION(pPoints[k].x, pPoints[k].y, pPoints[i].x, pPoints[i].y, pPoints[j].x, pPoints[j].y);
		int ior;
		if(area > 0.0) ior = 1; //CCW
		else { ior = 0; //CW
		area *= -1; }
		if(o != ior) continue;
		tempPt.x = (pPoints[k].x + pPoints[i].x + pPoints[j].x) / 3;
		tempPt.y = (pPoints[k].y + pPoints[i].y + pPoints[j].y) / 3;

		double dx = tempPt.x - center.x;
		double dy = tempPt.y - center.y;
		double dist = dx * dx + dy * dy;

		if(area > maxArea) {
			maxArea = area;
			bestPt1 = tempPt;
		}
		if(dist < minDist) {
			minDist = dist;
			bestPt2 = tempPt;
		}
	}
	//오류가 나서 잠시 막아놓음.. 2021/03/31 giJoe
	if(PointInPolygon(bestPt1, pPoints, nPoint) == true) 
		*plbPoint = bestPt1;
	else 
		*plbPoint = tempPt;
}

// *************************** Extra ************************ //
//1. 폴리곤 확대/축소 (독자적으로 쓰이지 않고.. 아래 GetCatmullLine함수의 nZoomDiaMeter에 의해 자동으로 호출됨)
//void GetBorderOfPolygon( const vector<PointD> &p, double thickness, vector<PointD> &q );

const double SMALL = 1.0e-10;
double _signedArea( const vector<PointF> &p )
{
	//========================================================//
	// Assumes:                                               //
	//    N+1 vertices:   p[0], p[1], ... , p[N-1], p[N]      //
	//    Closed polygon: p[0] = p[N]                         //
	// Returns:                                               //
	//    Signed area: +ve if anticlockwise, -ve if clockwise //
	//========================================================//
	double A = 0;
	int N = p.size() - 1;
	for ( int i = 0; i < N; i++ ) A += p[i].x * p[i+1].y - p[i+1].x * p[i].y;
	A *= 0.5;
	return A;
}

vector<PointF> GetBorderOfPolygon( const vector<PointF> &p, double thickness )
{
	//=====================================================//
	// Assumes:                                            //
	//    N+1 vertices:   p[0], p[1], ... , p[N-1], p[N]   //
	//    Closed polygon: p[0] = p[N]                      //
	//    No zero-length sides                             //
	// Returns (by reference, as a parameter):             //
	//    Internal poly:  q[0], q[1], ... , q[N-1], q[N]   //
	//=====================================================//
	vector<PointF> q;
	int N = p.size() - 1;
	q.resize(N+1);              

	double a, b, A, B, d, cross;

	double displacement = - (thickness / 100000.); //thickness;
	if ( _signedArea( p ) < 0 ) displacement = -displacement;     // Detects clockwise order

	// Unit vector (a,b) along last edge
	a = p[N].x - p[N-1].x;
	b = p[N].y - p[N-1].y;
	d = sqrt( a * a + b * b );  
	a /= d;
	b /= d; 

	// Loop round the polygon, dealing with successive intersections of lines
	for ( int i = 0; i < N; i++ ) {
		// Unit vector (A,B) along previous edge
		A = a;
		B = b;
		// Unit vector (a,b) along next edge
		a = p[i+1].x - p[i].x;
		b = p[i+1].y - p[i].y;
		d = sqrt( a * a + b * b );
		a /= d;
		b /= d;
		// New vertex
		cross = A * b - a * B;
		if ( abs( cross ) < SMALL ) {     // Degenerate cases: 0 or 180 degrees at vertex
			q[i].x = p[i].x - displacement * b;
			q[i].y = p[i].y + displacement * a;
		} else {                            // Usual case
			q[i].x = p[i].x + displacement * ( a - A ) / cross;
			q[i].y = p[i].y + displacement * ( b - B ) / cross;
		}
	}
	// Close the inside polygon
	q[N] = q[0];
	return q;
}
//======================================================================
*/