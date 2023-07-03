#pragma once

#include "DataManager.h"
#include "RoutePlan.h"

#if defined(USE_TSP_MODULE)
#if !defined(USE_REAL_ROUTE_TSP)
#include "../tsp/environment.h"
#endif
#include "../mst/prim.h"
#include "../mst/tsp_ga.h"
#endif

class CRouteManager
{
public:
	CRouteManager();
	~CRouteManager();

public:
	bool Initialize(void);
	void Release(void);

	void SetDataMgr(CDataManager* pDataMgr);

	void InitPoints(void);

	KeyID SetDeparture(IN const double lng, IN const double lat, IN const int matchType  = TYPE_LINK_MATCH_NONE);
	KeyID SetWaypoint(IN const double lng, IN const double lat, IN const int matchType = TYPE_LINK_MATCH_NONE);
	KeyID SetDestination(IN const double lng, IN const double lat, IN const int matchType = TYPE_LINK_MATCH_NONE);
	void SetRouteOption(IN const uint32_t route, IN const uint32_t avoid);
	void SetRouteCost(IN const uint32_t type, IN const RpCost* pCost);

	int GetWayPointCount(void);

	KeyID GetPositionLink(IN const double lng, IN const double lat, IN const int matchType = TYPE_LINK_MATCH_NONE);
	SPoint* GetDeparture(void);
	SPoint* GetWaypoint(IN const uint32_t idx);
	SPoint* GetDestination(void);

	int Route(/*packet*/);
	int Table(TspOptions* pOpt, IN RouteTable** ppResultTables, OUT vector<uint32_t>& vtBestWaypoints);
	int GetTable(OUT RouteTable** ppResultTables);

	const RouteResultInfo* GetRouteResult(void) const;

	const uint32_t GetRouteResultsCount(void) const;
	const RouteResultInfo* GetRouteResults(IN const uint32_t idx) const;


	// 분기점 노드의 경로선을 제외한 나머지 링크 얻기 // like a mpp
	// length = 본경로의 최대 길이, 0: 전체, ...
	// expansion = 본 경로 외 확장할 레벨, 0: 현재 경로만, 1: 현재 경로에서 1단계만 가지치기 ...
	// branchLength = 가지치기한 경로의 최대 길이
	const size_t GetRouteProbablePath(OUT vector<RouteProbablePath*>& vtJctInfo, IN const double length = 0, IN const int32_t expansion = 1, IN const double branchLength = 50);


#if defined(USE_SHOW_ROUTE_SATATUS)
	void SetRouteStatusFunc(IN const void *pHost, IN void (*fpDrawRouting)(const void *, const unordered_map<uint64_t, CandidateLink*>*));
#endif
	// void GetClosestPoint(SPoint spt, SPoint ept, SPoint upt, SPoint& result);

	//일단 모든 함수 몰아서
	//각 클래스별로 분류하는건 금방할 수 있을듯.......

	// for TSP
#if defined(USE_TSP_MODULE)
	bool GetBestWaypointResult(IN TspOptions* pOpt, IN const RouteTable** ppResultTables, OUT vector<uint32_t>& vtBestWaypoints);
#endif


private:
	int SingleRoute();

	int DoRouting(/*Packet*/);
	// int setNode(/*FLAG, Packet*/);
	int DoTabulate(TspOptions* pOpt, IN RouteTable** ppResultTables, OUT vector<uint32_t>& vtBestWaypoints);

	SBox m_rtRouteBox;

	//void GetClosestPoint(double x1, double y1, double x2, double y2, double ox, double oy, double *a, double *b, int *ir);
	KeyID Projection(IN const double lng, IN const double lat);

	// int SearchOneDirection();

	SPoint m_ptDeparture;
	SPoint m_ptNewDeparture;
	vector<SPoint> m_ptWaypoints;
	vector<SPoint> m_ptNewWaypoints;
	SPoint m_ptDestination;
	SPoint m_ptNewDestination;
	
	KeyID keyDeparture;
	KeyID keyDestination;
	vector<KeyID> keyWaypoints;


	uint32_t m_nRouteOpt;
	uint32_t m_nAvoidOpt;

	RouteResultInfo m_routeResult;

	vector<RouteInfo> m_vtRouteInfo;
	vector<RouteResultInfo> m_vtRouteResult;


	CDataManager* m_pDataMgr;
	CRoutePlan* m_pRoutePlan;


};
