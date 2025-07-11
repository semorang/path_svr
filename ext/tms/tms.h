#ifndef __TMS_H__
#define __TMS_H__


#define USE_REAL_ROUTE_TSP //���� ��� ��� TSP
#define USE_REAL_ROUTE_TSP_COST // ���� ��� �ڽ�Ʈ ����


typedef enum
{
	TYPE_TSP_VALUE_COST,
	TYPE_TSP_VALUE_DIST,
	TYPE_TSP_VALUE_TIME
}TYPE_TSP_VALUE;


typedef struct _tagWaypoints
{
	int nId;
	double x;
	double y;
	int nLayoverTime; // ���� �ҿ� �ð�
	int nCargoVolume; // ���� ȭ�� ����

	_tagWaypoints() {
		nId = -1;
		x = 0.f;
		y = 0.f;
		nLayoverTime = 0;
		nCargoVolume = 0;
	}
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