#pragma once

#include "MapDef.h"
#include "DataManager.h"

// #define USE_TSP_MODULE
// #define USE_REAL_ROUTE_TSP //실제 경로 기반 TSP
// #define USE_REAL_ROUTE_TSP_COST // 실제 경로 코스트 기준

#define MAX_WAYPOINT 100
#if defined(USE_PEDESTRIAN_DATA)
#define MAX_SEARCH_DIST 15000 // 직선 최대 15km
#define MAX_SEARCH_RANGE 5
const int searchRange[MAX_SEARCH_RANGE] = {100, 200, 500, 7000, 1000}; // 
#elif defined(USE_TREKKING_DATA)
#define MAX_SEARCH_DIST 30000 // 직선 최대 30km
#define MAX_SEARCH_RANGE 5
const int searchRange[MAX_SEARCH_RANGE] = {100, 500, 1000, 2000, 3000}; // 
#else
#define MAX_SEARCH_DIST 500000 // 직선 최대 500km
#define MAX_SEARCH_RANGE 5
//const int searchRange[MAX_SEARCH_RANGE] = {100, 500, 1000, 2000, 5000}; // 
const array<int32_t, MAX_SEARCH_RANGE> searchRange = { 100, 500, 1000, 2000, 5000 }; // 
#endif



#define VAL_MAX_COST		86400 // 1day sec // 60 * 60 * 24
#define VAL_HEURISTIC_VEHICLE_FACTOR 0.1f
#define VAL_HEURISTIC_VEHICLE_FACTOR_FOR_TABLE 1.0f//0.1f
#define VAL_HEURISTIC_PEDESTRIAN_FACTOR 0.9f
//#define VAL_HEURISTIC_FACTOR 1.9f



typedef struct _tagCandidateLink {
	KeyID candidateId; // 후보 ID
	KeyID parentId; // 부모 링크 ID
	KeyID linkId; // 링크 ID
	KeyID nodeId; // 노드 ID
	float distReal; // 누적 거리
	float timeReal; // 누적 시간
	double costTreavel; // 누적 계산 비용
	double costHeuristic; // 방향 가중치 계산 비용
	unsigned int depth : 29; // 탐색 깊이
	unsigned int visited : 1; // 방문 여부
	unsigned int dir : 2; // 탐색 방향, 0:미정의, 1:정방향(S->E), 2:역방향(E->S)

	_tagCandidateLink() {
		candidateId.llid = 0;
		parentId.llid = 0;
		linkId.llid = 0;
		nodeId.llid = 0;
		distReal = 0.f;
		timeReal = 0.f;
		costTreavel = 0.f;
		costHeuristic = 0.f;
		depth = 0;
		visited = 0;
		dir = 0;
	}

	//bool operator<(const CandidateLink* rhs) const {
	//	if (costHeuristic > rhs->costHeuristic) {
	//		return true;
	//	}
	//	else if (costHeuristic == rhs->costHeuristic) {
	//		return depth > rhs->depth;
	//	}
	//	//if (costReal > rhs.costReal) {
	//	//	return true;
	//	//}
	//	//else if (costReal == rhs.costReal) {
	//	//	return depth > rhs.depth;
	//	//}
	//	else return false;
	//}
}CandidateLink;


static auto CompareCanidate = [](const CandidateLink* lhs, const CandidateLink* rhs) {
	if (lhs->costHeuristic > rhs->costHeuristic) {
		return true;
	}
	else if (lhs->costHeuristic == rhs->costHeuristic) {
		return lhs->depth > rhs->depth;
	}
	else {
		return false;
	}
};

struct nearestDist {
	nearestDist(const int32_t _id, const int32_t _dist) : id(_id), dist(_dist) {};
	int32_t id;
	int32_t dist;
};

struct cmpDist {
	bool operator()(const nearestDist lhs, const nearestDist rhs) {
		return lhs.dist > rhs.dist;
	}
};


typedef struct _tagRouteProbablePath {
	KeyID LinkId; // 링크 ID
	KeyID NodeId; // 노드 ID
	vector<stLinkInfo*> JctLinks; // 정션 링크

	_tagRouteProbablePath() {
		LinkId.llid = 0;
		NodeId.llid = 0;
	}

	~_tagRouteProbablePath() {
		if (!JctLinks.empty()) {
			for (const auto& junction : JctLinks) {
				delete ((stLinkInfo*)junction);
			}
			JctLinks.clear();
			vector<stLinkInfo*>().swap(JctLinks);
		}
	}
}RouteProbablePath;


typedef struct _tagRouteLinkInfo {
	KeyID LinkId; // 링크 ID
	SPoint Coord; // 좌표
	SPoint MatchCoord; // 링크와 직교 접점 좌표
	int32_t LinkSplitIdx; // 링크와 직교 접점 좌표의 링크 버텍스 idx
	int32_t LinkDistToS; // s로의 거리
	int32_t LinkDistToE; // e로의 거리
	int32_t LinkDir; // 탐색 방향, 0:미정의, 1:정방향(S->E), 2:역방향(E->S)
	int32_t LinkGuideType; // 링크 안내 타입, 0:일반, 1:출발지링크, 2:도착지링크, 3:경유지링크
	vector<SPoint> LinkVtxToS; // 좌표점에서 s버텍스
	vector<SPoint> LinkVtxToE; // 좌표점에서 e버텍스

	_tagRouteLinkInfo() {
		LinkId.llid = 0;
		memset(&Coord, 0x00, sizeof(Coord));
		memset(&MatchCoord, 0x00, sizeof(MatchCoord));
		LinkSplitIdx = -1;
		LinkDistToS = 0;
		LinkDistToE = 0;
		LinkDir = 0;
		LinkGuideType = 0;
	}

	~_tagRouteLinkInfo() {
		if (!LinkVtxToS.empty()) {
			LinkVtxToS.clear();
			vector<SPoint>().swap(LinkVtxToS);
		}

		if (!LinkVtxToE.empty()) {
			LinkVtxToE.clear();
			vector<SPoint>().swap(LinkVtxToE);
		}
	}
}RouteLinkInfo;


typedef struct _tagRouteInfo {
	uint32_t RequestMode; // 요청 모드
	uint32_t RequestId; // 요청 ID

	RouteLinkInfo StartLinkInfo;
	RouteLinkInfo EndLinkInfo;

	uint32_t RouteOption; // 경탐 옵션
	uint32_t AvoidOption; // 회피 옵션

	vector<CandidateLink*> vtCandidateResult;
	unordered_map<uint64_t, CandidateLink*> mRoutePass;

	priority_queue<CandidateLink*, vector<CandidateLink*>, decltype(CompareCanidate)> pqDijkstra{ CompareCanidate };

	_tagRouteInfo() {
		RequestMode = 0;
		RequestId = 0;

		RouteOption = 0;
		AvoidOption = 0;
	}

	~_tagRouteInfo() {
		for (; !pqDijkstra.empty(); pqDijkstra.pop());

		if (!vtCandidateResult.empty()) {
			vtCandidateResult.clear();
			vector<CandidateLink*>().swap(vtCandidateResult);
		}

		if (!mRoutePass.empty()) {
			for (unordered_map<uint64_t, CandidateLink*>::iterator it = mRoutePass.begin(); it != mRoutePass.end(); it++) {
				if (it->second) {
					SAFE_DELETE(it->second);
				}
			}
			mRoutePass.clear();
			unordered_map<uint64_t, CandidateLink*>().swap(mRoutePass);
		}
	}
}RouteInfo;


typedef struct _tagRequestRouteInfo {
	uint32_t RequestMode; // 요청 모드
	uint32_t RequestId; // 요청 ID

	uint32_t RouteOption; // 경탐 옵션
	uint32_t AvoidOption; // 회피 옵션

	vector<SPoint> vtPoints; // 요청 좌표
	vector<KeyID> vtIdLinks; // 요청 좌표 링크 id

	_tagRequestRouteInfo() {
		RequestMode = 0;
		RequestId = 0;

		RouteOption = 0;
		AvoidOption = 0;
	}

	~_tagRequestRouteInfo() {
		if (!vtPoints.empty()) {
			vtPoints.clear();
			vector<SPoint>().swap(vtPoints);
		}

		if (!vtIdLinks.empty()) {
			vtIdLinks.clear();
			vector<KeyID>().swap(vtIdLinks);
		}
	}
}RequestRouteInfo;


typedef struct _tagRouteTable {

	uint32_t nOriginIdx;
	uint32_t nBestIdx;
	int32_t nUsable; // 요청 모드 // -1:미사용, 0:미방문, n:방문횟수
	uint32_t nTotalDist;
	uint32_t nTotalTime;
	double dbTotalCost;

	RouteLinkInfo linkInfo;

	_tagRouteTable() {
		nOriginIdx = 0;
		nBestIdx = 0;

		nUsable = -1;
		nTotalDist = 0;
		nTotalTime = 0;
		dbTotalCost = 0;
	}

	~_tagRouteTable() {
	}
}RouteTable;


typedef struct _tagTableBaseInfo {
	RouteLinkInfo					routeLinkInfo;
	vector<CandidateLink>			vtCandidateLink;
	unordered_map<uint64_t, bool>	mRoutePass;

	uint32_t						routeOption; // 경탐 옵션
	uint32_t						avoidOption; // 회피 옵션

	size_t							tickStart; // 시작 시각
	size_t							tickFinish; // 종료 시각

	priority_queue<CandidateLink*, vector<CandidateLink*>, decltype(CompareCanidate)> pqDijkstra{ CompareCanidate };


	_tagTableBaseInfo() {
		routeOption = 0;
		avoidOption = 0;
		tickStart = 0;
		tickFinish = 0;
	}

	~_tagTableBaseInfo() {
		//for (; !pqDijkstra.empty(); pqDijkstra.pop());
		for (; !pqDijkstra.empty(); ) {
			delete pqDijkstra.top();
			pqDijkstra.pop();
		}

		if (!mRoutePass.empty()) {
			mRoutePass.clear();
			unordered_map<uint64_t, bool>().swap(mRoutePass);
		}

		if (!vtCandidateLink.empty()) {
			vtCandidateLink.clear();
			vector<CandidateLink>().swap(vtCandidateLink);
		}
	}
}TableBaseInfo;


typedef struct _tagTspOptions {
	uint32_t algorityhmType;
	uint32_t geneSize;
	uint32_t individualSize;
	uint32_t loopCount;
}TspOptions;


typedef struct _tagRpCost {
	union {
		int32_t base[128];
		struct {
			int32_t cost_lv0; int32_t cost_lv1; int32_t cost_lv2; int32_t cost_lv3; int32_t cost_lv4;
			int32_t cost_lv5; int32_t cost_lv6; int32_t cost_lv7; int32_t cost_lv8; int32_t cost_lv9; 
			int32_t cost_ang0; int32_t cost_ang45;int32_t cost_ang90; int32_t cost_ang135; int32_t cost_ang180;
			int32_t cost_ang225; int32_t cost_ang270; int32_t cost_ang315;
			// 18
		}vehicle;
	};
}RpCost;


class CRoutePlan
{
public:
	CRoutePlan();
	~CRoutePlan();

	void Initialize(void);
	void Release(void);

	void SetDataMgr(IN CDataManager* pDataMgr);
	void SetRouteCost(IN const uint32_t type, IN const RpCost* pCost);

	const int DoRoute(IN const uint32_t reqId, IN const SPoint ptStart, IN const SPoint ptEnd, IN const KeyID sLink, IN const KeyID eLink, IN const uint32_t routeOpt, IN const uint32_t avoidOpt, OUT RouteResultInfo* pRouteResult);
	const int DoRoutes(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults);
#if defined(USE_TSP_MODULE)
	//const int DoTabulate(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults);
	const int DoTabulate(IN const RequestRouteInfo* pReqInfo, OUT RouteTable** ppResultTables);	
#endif

#if defined(USE_SHOW_ROUTE_SATATUS)
	void SetRouteStatusFunc(IN const void* fpHost, IN void(*drawRouting)(const void*, const unordered_map<uint64_t, CandidateLink*>*));
#endif

private:
	const bool SetRouteLinkInfo(IN const SPoint& ptPosition, const IN KeyID keyLink, IN const bool isStart, OUT RouteLinkInfo* pRouteLinkInfo);

	const bool SetVisitedLink(IN RouteInfo* pRouteInfo, IN const KeyID linkId);
	const bool IsVisitedLink(IN RouteInfo* pRouteInfo, IN const KeyID linkId);
	const bool IsAddedLink(IN RouteInfo* pRouteInfo, IN const KeyID linkId);
	const int AddNewLinks(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const int dir);
	const int Propagation(IN TableBaseInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const int dir, IN const SPoint target); // /단순 확장

	const int MakeRoute(IN const int idx, IN RouteInfo* pRouteInfo, OUT RouteResultInfo* pRouteResult);
#if defined(USE_TSP_MODULE)
	const int MakeTabulate(IN const RequestRouteInfo* pReqInfo, IN const vector<RouteLinkInfo>& LinkInfos, OUT RouteTable** ppResultTables);
#endif
	const int MakeRouteResult(IN RouteInfo* pRouteInfo, OUT RouteResultInfo* pRouteResult);

	const double GetCost(IN const stLinkInfo* pLink, IN const uint32_t opt, IN const double length, IN const uint32_t spd); // type, 0:보행자, 1:자전거 
	const double GetTravelCost(IN const stLinkInfo* pLink, IN const stLinkInfo* pLinkPrev, IN const double cost, IN const int angle, IN const uint32_t type, IN const uint32_t opt, IN const uint32_t avoid); // type, 0:보행자, 1:자전거
	const uint32_t CheckStartDirectionMaching(IN const stLinkInfo* pLink, IN const RouteLinkInfo* pRoutLinkInfo, IN const int32_t routeOpt, OUT vector<CandidateLink>& vtCandidateInfo);
	const uint32_t CheckEndDirectionMaching(IN const stLinkInfo* pLink, IN const stNodeInfo* pSNode, IN const stNodeInfo* pENode, IN const RouteLinkInfo* pRoutLinkInfo, IN const int32_t routeOpt, OUT vector<CandidateLink>& vtCandidateInfo);

	CDataManager* m_pDataMgr;
	MapNode* m_pMapNodeMgr;
	MapLink* m_pMapLinkMgr;

	//vector<CandidateLink*> m_vtCandidateResult;
	//int DoRouting(/*Packet*/);

	//RouteInfo m_routeInfo;
	//RouteResultInfo m_routeResult;


	//unordered_map<uint64_t, CandidateLink*> mRoutePass;
	//priority_queue<CandidateLink*> pqDijkstra;
	//priority_queue<CandidateLink*, vector<CandidateLink*>, decltype(CompareCanidate)> pqDijkstra { CompareCanidate };

	// 현재 경로 탐색 상태 Function Call
#if defined(USE_SHOW_ROUTE_SATATUS)
	void* m_pHost;
	void(*m_fpRoutingStatus)(const void* pHost, const unordered_map<uint64_t, CandidateLink*>*);
#endif

	RpCost m_rpCost;
};



