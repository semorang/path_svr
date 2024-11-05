#ifndef __TMS_H__
#define __TMS_H__


#define USE_REAL_ROUTE_TSP //���� ��� ��� TSP
#define USE_REAL_ROUTE_TSP_COST // ���� ��� �ڽ�Ʈ ����


typedef enum
{
	TYPE_TSP_VALUE_COST,
	TYPE_TSP_VALUE_DIST,
	TYPE_TSP_VALUE_TIME
}TYPE_TSP_VALUEEEE;


typedef struct _tagWaypoints
{
	int nId;
	double x;
	double y;
}stWaypoints;


typedef struct _tagstDistMatrix{
	int		nTotalDist;
	int		nTotalTime;
	double	dbTotalCost;

	_tagstDistMatrix() {
		nTotalDist = 0;
		nTotalTime = 0;
		dbTotalCost = 0.f;
	}
}stDistMatrix;


#endif // __TMS_H__