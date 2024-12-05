#pragma once

#include "DataManager.h"
#include "RoutePlan.h"

#if defined(USE_TSP_MODULE)
#include "../tsp/environment.h"
#include "../tms/prim.h"
#include "../tms/tsp_ga.h"
#include "../tms/TMSManager.h"
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
	void SetRouteOption(IN const vector<uint32_t>& routes, IN const vector<uint32_t>& avoids, IN const uint32_t mobility = 0);
	void AddRouteOption(IN const uint32_t option, IN const uint32_t avoid = 0, IN const uint32_t mobility = 0);
	void SetRouteSubOption(IN const uint64_t sub);
	void SetRouteDirOption(IN const uint32_t departuretDir, IN const uint32_t waypointDir, IN const uint32_t destinationDir);
	void SetRouteCost(IN const uint32_t type, IN const DataCost* pCost, IN const uint32_t cntCost = 0);

	const int GetWayPointCount(void) const;

	KeyID GetPositionLink(IN const double lng, IN const double lat, IN const int32_t matchType = TYPE_LINK_MATCH_NONE, IN const int32_t dataType = TYPE_DATA_NONE);
	const SPoint* GetDeparture(IN const bool useMatch = false);
	const SPoint* GetWaypoint(IN const uint32_t idx, IN const bool useMatch = false);
	const SPoint* GetDestination(IN const bool useMatch = false);

	int Route(const int opt = 0/*packet*/);
	int GetWeightMatrix(IN const char* szRequest, OUT vector<vector<stDistMatrix>>& vtWeightMatrix);
	int GetBestway(IN const char* szRequest, OUT vector<stWaypoints>& vtWaypoints, OUT vector<uint32_t>& vtBestWaypoints, OUT double& dist, OUT int32_t& time);
	int GetCluster(IN const char* szRequest, OUT vector<stDistrict>& vtCluster, OUT vector<SPoint>& vtPositionLock);
	int GetBoundary(IN const vector<SPoint>& vtPois, OUT vector<SPoint>& vtBoundary, OUT SPoint& center);

	int GetCluster_for_geoyoung(IN const int32_t cntCluster, OUT vector<stDistrict>& vtCluster);

	const uint32_t GetRouteResultsCount(void) const;
	const RouteResultInfo* GetRouteResult(void) const;
	const RouteResultInfo* GetMultiRouteResult(IN const uint32_t idx) const;
	const vector<RouteResultInfo>* GetMultiRouteResults(void) const;

	const int GetCandidateRoute(void);

#if defined(USE_SHOW_ROUTE_SATATUS)
	void SetRouteStatusFunc(IN const void *pHost, IN void (*fpDrawRouting)(const void *, const unordered_map<uint64_t, CandidateLink*>*));
#endif
	// void GetClosestPoint(SPoint spt, SPoint ept, SPoint upt, SPoint& result);

	//일단 모든 함수 몰아서
	//각 클래스별로 분류하는건 금방할 수 있을듯.......

	// for TSP
#if defined(USE_TSP_MODULE)
	int GetBestWaypointResult(IN const TspOptions* pOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, OUT vector<uint32_t>& vtBestWaypoints, OUT double& dist, OUT int32_t& time);
#endif


private:
	int SingleRoute();
	int MultiRoute(const int opt = 0/*packet*/);

	int DoRouting(/*Packet*/);
	int DoMultiRouting(IN const int32_t routeCount/*, IN const int32_t routeOptions[], IN const int32_t routeAvoids[]*/);
	int DoComplexRouting(/*Packet*/);
	int DoMultiComplexRouting(IN const int32_t routeCount/*, IN const int32_t routeOptions[], IN const int32_t routeAvoids[]*/);
	// int setNode(/*FLAG, Packet*/);
	int DoCourse(/*Packet*/);
	int DoTabulate(IN const vector<SPoint>vtOrigins, OUT RequestRouteInfo& reqInfo, OUT vector<vector<stDistMatrix>>& vtDistMatrix);

	KeyID SetPosition(IN const double lng, IN const double lat, IN const int matchType, OUT RouteLinkInfo& pointLinkInfo);

	SBox m_rtRouteBox;

	//void GetClosestPoint(double x1, double y1, double x2, double y2, double ox, double oy, double *a, double *b, int *ir);
	KeyID Projection(IN const double lng, IN const double lat);

	// int SearchOneDirection();

	RouteLinkInfo m_linkDeparture;
	RouteLinkInfo m_linkDestination;
	vector<RouteLinkInfo>m_linkWaypoints;
	
	vector<uint32_t> m_vtRouteOpt;
	vector<uint32_t> m_vtAvoidOpt;
	uint32_t m_nMobilityOpt;

	// 경탐 세부옵션, 임시방법이고, 좀더 깔끔한 다른 방식을 고민해 보자, 2023.11.07
	stRouteSubOption m_routeSubOpt;

	int32_t m_nDepartureDirIgnore; // 출발지 방향성 무시
	int32_t m_nWaypointDirIgnore; // 경유지 방향성 무시
	int32_t m_nDestinationDirIgnore; // 도착지 방향성 무시

	vector<RouteInfo> m_vtRouteInfo;
	vector<RouteResultInfo> m_vtRouteResult;


	CDataManager* m_pDataMgr;
	CRoutePlan* m_pRoutePlan;
	CTMSManager* m_pTmsMgr;
};
