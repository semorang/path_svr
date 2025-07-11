#ifndef __TMS_H__
#define __TMS_H__


#define USE_REAL_ROUTE_TSP //실제 경로 기반 TSP
#define USE_REAL_ROUTE_TSP_COST // 실제 경로 코스트 기준


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
	int nLayoverTime; // 지점 소요 시간
	int nCargoVolume; // 지점 화물 수량

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