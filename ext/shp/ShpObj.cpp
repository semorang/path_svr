#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <assert.h>
#include "shpio.h"

// #ifdef _DEBUG
// #define new DEBUG_NEW
// #undef THIS_FILE
// static char THIS_FILE[] = __FILE__;
// #endif

#define  MATH_TOL0		1.0e-8			    // is zero tol

#if !defined(_WIN32)
int my_stricmp (const char *s1, const char *s2)
{
    while (*s1 != 0 && *s2 != 0)
    {
        if (*s1 != *s2 && ::toupper (*s1) != ::toupper(*s2)) {
            return -1;
        }
        s1++;
        s2++;
    }
    return (*s1 == 0 && *s2 == 0) ? 0 : -1;
}
#endif

template <class T>
inline double I_Distance2( const T &p1, const T &p2 )
{
	double dx = p1.x-p2.x;
	double dy = p1.y-p2.y;
	return dx*dx+dy*dy;
}

template <class T>
inline int I_iGetLeftLow( int n, const T *vtx_list )
{
	n--;
	int m=0;
	T leftbottom_pos = vtx_list[0];

	for(int iCnt=1; iCnt<n; iCnt++)
	{
		if( leftbottom_pos.x < vtx_list[iCnt].x ) {
			m = iCnt;
			leftbottom_pos = vtx_list[iCnt];
		}else if( leftbottom_pos.x == vtx_list[iCnt].x && leftbottom_pos.y < vtx_list[iCnt].y ) {
			m = iCnt;
			leftbottom_pos = vtx_list[iCnt];
		}
	}

	return m;
}

template <class T>
inline int I_IsCCW( int n, const T *vtx_list )
{
	T a, b, c; //Just renaming

	n--;
	int m =  I_iGetLeftLow ( n, vtx_list );
	b = vtx_list[m];

	int nLoopCnt = 0;
	int m1 = (m + (n-1)) % n; 
	while( I_Distance2( b, vtx_list[m1] ) < MATH_TOL0 && m!=m1 ) {
		m1 = (m1-1 + (n-1)) % n; 
		nLoopCnt++;
		if( nLoopCnt==n ) {
			m1 = (m + (n-1)) % n; 
			break;
		}
	}
	a = vtx_list[m1];

	nLoopCnt = 0;
	m1 = (m+1)%n;
	while( I_Distance2( b, vtx_list[m1] ) < MATH_TOL0 && m!=m1 ) {
		m1 = (m1+1 + (n)) % n; 
		nLoopCnt++;
		if( nLoopCnt==n ) {
			m1 = (m+1)%n;
			break;
		}
	}
	c = vtx_list[ m1%n ];

	double area2 =  a.x * b.y - a.y * b.x + 
		a.y * c.x - a.x * c.y +
		b.x * c.y - c.x * b.y;

	if ( area2 > 0 )	    return 1;
	else if ( area2 < 0 )	return -1;

	return 0;
}

template <class T>
inline void ReversePoints( int numPoints, T *points )
{
	int loop_cnt = numPoints/2;
	for( int i=0; i<loop_cnt; i++ ) {
		T pt;
		pt						= points[i];
		points[i]				= points[numPoints-i-1];
		points[numPoints-i-1]	= pt;
	}
}

// 처음점과 끝점을 비교하여 같으면 0을 다르면 1을 리턴한다.
static int iSameFEP(int iVtxCnt, SPoint *pPoints ) // (0)same, (1)not same
{
	if( I_Distance2( pPoints[0], pPoints[iVtxCnt-1] ) < MATH_TOL0 ) 
		return 0;
	else
		return 1;
}

inline void SetEnvelope( SBox *pBox, int nCount, const SPoint *pPoints )
{
	pBox->Xmin = pBox->Xmax = pPoints->x;
	pBox->Ymin = pBox->Ymax = pPoints->y;
	pPoints++;
	for( int i=1; i<nCount; i++ ) {
		if( pBox->Xmin > pPoints->x ) pBox->Xmin = pPoints->x;
		if( pBox->Xmax < pPoints->x ) pBox->Xmax = pPoints->x;
		if( pBox->Ymin > pPoints->y ) pBox->Ymin = pPoints->y;
		if( pBox->Ymax < pPoints->y ) pBox->Ymax = pPoints->y;
		pPoints++;
	}
}

int	SHP_GetGeometrySize( const SHPGeometry *pObj )
{
	//	assert( pObj );
	if( pObj==NULL )
		return 0;

	int o_size = 0;
	switch( pObj->obj.ShapeType ) {
		case shpNullShape   : 
			return sizeof(SHPNullObj);
		case shpPoint       : 
			return sizeof(SHPPoint);
		case shpPolyLine    : 
			return sizeof(SHPPolyLine) + pObj->polyline.NumParts*sizeof(long)+pObj->polyline.NumPoints*sizeof(SPoint);
		case shpPolygon     :
			return sizeof( SHPPolygon ) + pObj->polygon.NumParts*sizeof(long)+pObj->polygon.NumPoints*sizeof(SPoint);
		case shpMultiPoint  :
			return sizeof(SHPMultiPoint) + pObj->mpoint.NumPoints*sizeof(SPoint);
		case shpPointZ      :
			return sizeof(SHPPoint) + sizeof(double);
		case shpPolyLineZ   :
			o_size = sizeof(SHPPolyLine) + pObj->polyline.NumParts*sizeof(long)+pObj->polyline.NumPoints*sizeof(SPoint);
			return o_size + pObj->polyline.NumPoints*sizeof(double);
		case shpPolygonZ    :
			o_size = sizeof(SHPPolygon) + pObj->polygon.NumParts*sizeof(long)+pObj->polygon.NumPoints*sizeof(SPoint);
			return o_size + pObj->polygon.NumPoints*sizeof(double);
		case shpMultiPointZ :
			o_size = sizeof(SHPMultiPoint) + pObj->mpoint.NumPoints*sizeof(SPoint);
			return o_size + pObj->mpoint.NumPoints*sizeof(double);
		case shpPointM      :
			return sizeof(SHPPoint) + sizeof(double)*2;
		case shpPolyLineM   :
			o_size = sizeof(SHPPolyLine) + pObj->polyline.NumParts*sizeof(long)+pObj->polyline.NumPoints*sizeof(SPoint);
			return o_size + pObj->polyline.NumPoints*sizeof(double)*2;
		case shpPolygonM    :
			o_size = sizeof(SHPPolygon) + pObj->polygon.NumParts*sizeof(long)+pObj->polygon.NumPoints*sizeof(SPoint);
			return o_size + pObj->polygon.NumPoints*sizeof(double)*2;
		case shpMultiPointM :
			o_size = sizeof(SHPMultiPoint) + pObj->mpoint.NumPoints*sizeof(SPoint);
			return o_size + pObj->mpoint.NumPoints*sizeof(double)*2;
		case shpMultiPatch  :
			return sizeof(SHPMultiPatch) + pObj->mpatch.NumParts*sizeof(long)+pObj->mpatch.NumPoints*sizeof(SPoint);

	}
	return 0;
}

SPoint *SHP_GetPartPoints( const SHPGeometry *pObj, int nPart )
{
	int pointsOff = sizeof(SHPPolyLine) + pObj->polyline.NumParts * sizeof(long) ;
	SPoint *points = (SPoint *) ( ((char*)pObj) + pointsOff );
	return points + pObj->polyline.Parts[ nPart ];
}

bool SHP_IsOuterRing( const SHPGeometry *pObj, int nPart )
{
	int     numPoints;
	SPoint *points;

	numPoints = SHP_GetNumPartPoints( pObj, nPart );
	points    = SHP_GetPartPoints( pObj, nPart  );
	if( I_IsCCW( numPoints, points ) < 0 )
		return true;
	return false;
}

int SHP_GetNumPartPoints( const SHPGeometry *pObj, int nPart )
{
	if( pObj==NULL )
		return 0;
	switch( pObj->obj.ShapeType%10 ) {
		case shpNullShape   : 
			return 0;
		case shpPoint       : 
			return 1;
		case shpPolyLine    : 
			if( nPart==(pObj->polyline.NumParts-1) ) 
				return pObj->polyline.NumPoints - pObj->polyline.Parts[ nPart ];
			else
				return pObj->polyline.Parts[ nPart+1 ] - pObj->polyline.Parts[ nPart ];
			break;
		case shpPolygon     :
			if( nPart==(pObj->polygon.NumParts-1) ) 
				return pObj->polygon.NumPoints - pObj->polygon.Parts[ nPart ];
			else
				return pObj->polygon.Parts[ nPart+1 ] - pObj->polygon.Parts[ nPart ];
			break;
		case shpMultiPoint  :
			return 1;
	}

	return 0;
}


int SHP_CalcGeometrySizePoint( void )
{
	return sizeof(SHPPoint);
}

int SHP_CreatePoint2( SHPGeometry *pObj, const SPoint &point )
{
	pObj->point.ShapeType = shpPoint;
	pObj->point.point     = point;

	return sizeof(SHPPoint);
}

SHPGeometry	*SHP_CreatePoint( const SPoint &point )
{
	SHPGeometry	*pObj = (SHPGeometry *) new char [ sizeof(SHPPoint) ];
	SHP_CreatePoint2( pObj, point );

	return pObj;
}

int SHP_CalcGeometrySizeMultiPoint( int numPoint, const SPoint *points )
{
	return sizeof(SHPMultiPoint)+sizeof(SPoint)*numPoint;
}

int SHP_CreateMultiPoint2( SHPGeometry *pObj, int numPoint, const SPoint *points )
{
	int o_size = SHP_CalcGeometrySizeMultiPoint( numPoint, points );

	pObj->obj.ShapeType    = shpMultiPoint;
	pObj->mpoint.NumPoints = numPoint;
	memcpy( pObj->mpoint.Points, points, sizeof(SPoint)*numPoint );

	SetEnvelope( &(pObj->mpoint.Box), numPoint, pObj->mpoint.Points );

	return o_size;
}

SHPGeometry	*SHP_CreateMultiPoint( int numPoint, const SPoint *points )
{
	SHPGeometry	*pObj      = (SHPGeometry *) new char [sizeof(SHPMultiPoint)+sizeof(SPoint)*numPoint];

	pObj->obj.ShapeType    = shpMultiPoint;
	pObj->mpoint.NumPoints = numPoint;
	memcpy( pObj->mpoint.Points, points, sizeof(SPoint)*numPoint );

	SetEnvelope( &(pObj->mpoint.Box), numPoint, pObj->mpoint.Points );

	return pObj;
}

SHPGeometry	*SHP_CreatePolyline( const SPoint *lpPoints, const int *lpVtxCounts, int nCount )
{
	if( lpPoints==NULL || lpVtxCounts==NULL || nCount<=0 )
		return NULL;

	int nTotalPoint = 0;
	int i;
	for( i=0; i<nCount; i++ )
		nTotalPoint += lpVtxCounts[i];

	int o_size = sizeof(SHPPolyLine) + nCount*sizeof(long)+nTotalPoint*sizeof(SPoint);

	SHPGeometry	*pObj = (SHPGeometry*) new char [ o_size ];
	pObj->polyline.ShapeType = shpPolyLine;
	pObj->polyline.NumParts  = nCount;
	pObj->polyline.NumPoints = nTotalPoint;
	int off = 0;
	for( i=0; i<pObj->polyline.NumParts; i++ ) {
		pObj->polyline.Parts[ i ] = off;
		off += lpVtxCounts[i];
	}
	SPoint *points = (SPoint *) ( ((char*)pObj) + sizeof(SHPPolyLine) + nCount*sizeof(long) );
	memcpy( points, lpPoints, sizeof(SPoint)*nTotalPoint );

	SetEnvelope( &(pObj->polyline.Box), nTotalPoint, lpPoints );

	return pObj;
}

SHPGeometry	*SHP_CreatePolygon( const SPoint *lpPoints, const int *lpVtxCounts, int nCount )
{
	if( lpPoints==NULL || lpVtxCounts==NULL || nCount<=0 )
		return NULL;

	int nTotalPoint = 0;
	int i = 0;
	SPoint *src_pnts = (SPoint *)lpPoints;

	for( i=0; i<nCount; i++ ) {
		int numPoints = abs(lpVtxCounts[i]);
		int iSem = iSameFEP( numPoints, src_pnts );
		nTotalPoint += numPoints + iSem;
		src_pnts    += numPoints;
	}
	int o_size = sizeof(SHPPolygon) + nCount*sizeof(long)+nTotalPoint*sizeof(SPoint);

	SHPGeometry	*pObj = (SHPGeometry*) new char [ o_size ];
	pObj->polygon.ShapeType = shpPolygon;
	pObj->polygon.NumParts  = nCount;
	pObj->polygon.NumPoints = nTotalPoint;
	SPoint *dst_pnts = (SPoint *) &( pObj->polygon.Parts[ nCount ] );
	src_pnts    = (SPoint *)lpPoints;
	int off = 0;
	for( i=0; i<pObj->polygon.NumParts; i++ ) {
		pObj->polygon.Parts[ i ] = off;
		int numPoints = abs(lpVtxCounts[i]);
		if( numPoints < 3 ) {
			delete pObj;
			return NULL;
		}

		memcpy( dst_pnts, src_pnts, sizeof(SPoint)*numPoints );
		int iSem = iSameFEP( numPoints, src_pnts );
		if( iSem ) 
			dst_pnts[ numPoints ] = src_pnts[0];

		int isCCW = I_IsCCW( numPoints, dst_pnts );
		if( isCCW * lpVtxCounts[i] > 0 ) 
			ReversePoints( numPoints+iSem, dst_pnts );

		off        += numPoints + iSem;
		dst_pnts   += numPoints + iSem;
		src_pnts   += numPoints;

	}

	dst_pnts = (SPoint *) &( pObj->polygon.Parts[ nCount ] );
	SetEnvelope( &(pObj->polygon.Box), nTotalPoint, dst_pnts );

	return pObj;
}

void SHP_DestroyGeometry( SHPGeometry *pObj )
{
	delete [] ((char*)pObj);
}

