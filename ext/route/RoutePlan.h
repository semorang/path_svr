#pragma once

#include "../include/MapDef.h"
#include "../include/types.h"
#include "DataManager.h"

#include "../utils/UserLog.h"

#define USE_TSP_MODULE

#define MAX_WAYPOINT 9999
#define MAX_SEARCH_DIST_FOR_BICYCLE 200000 // 직선 최대 200km
#define MAX_SEARCH_DIST_FOR_PEDESTRIAN 30000 // 직선 최대 30km
#define MAX_SEARCH_DIST_FOR_COURSE 100000 // 직선 최대 100km
#define MAX_SEARCH_DIST_FOR_FOREST 50000 // 직선 최대 50km
#define MAX_SEARCH_DIST_FOR_VEHICLE 500000 // 직선 최대 500km
#define MAX_SEARCH_RANGE 5
#if defined(USE_PEDESTRIAN_DATA)
const int searchRange[MAX_SEARCH_RANGE] = { 30, 100, 500, 1000, 5000 }; // 일반
const int hikingRange[MAX_SEARCH_RANGE] = { 100, 300, 500, 1000, 3000 }; // 등산(숲길)
const int trailRange[MAX_SEARCH_RANGE] = { 300, 500, 800, 1000, 3000 }; // 코스(둘레,테마,종주)
const int courseRange[MAX_SEARCH_RANGE] = { 50, 100, 300, 500, 1000 }; // 코스ID
#elif defined(USE_FOREST_DATA)
const int searchRange[MAX_SEARCH_RANGE] = {100, 500, 1000, 2000, 3000}; // 
#else
//const int searchRange[MAX_SEARCH_RANGE] = {100, 500, 1000, 2000, 5000}; // 
const array<int32_t, MAX_SEARCH_RANGE> searchRange = { 100, 500, 1000, 2000, 5000 }; // 
#endif



#define VAL_MAX_COST		86400 // 1day sec // 60 * 60 * 24
#define VAL_HEURISTIC_VEHICLE_FACTOR 0.1f//0.3f//0.1f
#define VAL_HEURISTIC_VEHICLE_FACTOR_FOR_TABLE 0.5f // 1.0f//0.1f
// #define VAL_HEURISTIC_PEDESTRIAN_FACTOR 0.9f
#define VAL_HEURISTIC_PEDESTRIAN_FACTOR 0.2f
#define VAL_HEURISTIC_FOREST_FACTOR 0.5f
#define VAL_HEURISTIC_FACTOR_MIN_DIST 1000.f // 휴리스틱 적용 최소 거리, 초과일 경우 factor를 증가
//#define VAL_HEURISTIC_FACTOR 1.9f



typedef struct _tagCandidateLink {
	KeyID candidateId; // 후보 ID
	KeyID parentLinkId; // 부모 링크 ID
	KeyID linkId; // 링크 ID
	KeyID nodeId; // 노드 ID
	float distReal; // 거리
	float timeReal; // 시간
	float distTreavel; // 누적 거리
	float timeTreavel; // 누적 시간
	double costTreavel; // 누적 계산 비용
	double costHeuristic; // 방향 가중치 계산 비용
	uint32_t linkDataType; // 데이터 타입, // 0:미정의, 1:숲길, 2:보행자/자전거, 3:자동차
	uint32_t depth : 29; // 탐색 깊이
	uint32_t visited : 1; // 방문 여부
	uint32_t dir : 2; // 탐색 방향, 0:미정의, 1:정방향(S->E), 2:역방향(E->S)
#if defined(USE_VEHICLE_DATA)
	uint8_t speed; // 속도
	uint8_t speed_type : 4; // 속도 타입, 0:미정의, 1:ttl, 2:ks, 3:static
	uint8_t speed_level : 4; // 도로 레벨,// 경로레벨, 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상, 4:일반도로6차선이상, 5:일반도로4차선이상, 6:일반도로2차선이상, 7:일반도로1차선이상, 8:SS도로, 9:GSS도로/단지내도로/통행금지도로/비포장도로
#endif

	_tagCandidateLink* pPrevLink;

	_tagCandidateLink() {
		candidateId.llid = 0;
		parentLinkId.llid = 0;
		linkId.llid = 0;
		nodeId.llid = 0;
		distReal = 0.f;
		timeReal = 0.f;
		distTreavel = 0.f;
		timeTreavel = 0.f;
		costTreavel = 0.f;
		costHeuristic = 0.f;
		linkDataType = 0;
		depth = 0;
		visited = 0;
		dir = 0;
#if defined(USE_VEHICLE_DATA)
		speed = 255;
		speed_type = 0;
		speed_level = 9;
#endif
		pPrevLink = nullptr;
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


typedef struct _tagMultimodalPointInfo
{
	double x;
	double y;

	// 차량 매칭 타입, 0:일반도로(미매칭), 1:빌딩입구점, 2:단지입구점, 3:단지내도로//, 4:최근접도로
	// 숲길 매칭 타입, 0:일반도로(미매칭), 1:숲길입구점
	int32_t nType;
	uint64_t id; // 매칭된 오브젝트 ID, 단지/빌딩/도로/산코드
	int32_t nGroupId; // 그룹 ID

	// 경로별 다중 입구점 경로 정보
	vector<double> vtCost;
	vector<double> vtDist;
	vector<double> vtTime;
	vector<stEntryPointInfo> vtEntryPoint;
	vector<CandidateLink*> vtRoutePathInfo;

	_tagMultimodalPointInfo()
	{
		x = 0.f;
		y = 0.f;

		nType = 0;
		id = 0;
		nGroupId = 0;
	}
}MultimodalPointInfo;


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


typedef struct _tagstRouteSubOption {
	union {
		int64_t option;
		struct {
			int32_t course_type; // 0:등산, 1:걷길, 2:자전거, 3:코스 // 2024-05-28 KVX 요청으로 변경, // old 코스 타입 // 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
			int32_t course_id; // 코스 ID
		}mnt;
	};

	_tagstRouteSubOption& operator=(const _tagstRouteSubOption& rhs)
	{
		option = rhs.option;
		return *this;
	}
}stRouteSubOption;


typedef struct _tagRouteLinkInfo {
	int32_t KeyType; // 0:미지정, 1:노드, 2:링크
	int32_t LinkDataType;  // 0:미정의, 1:숲길, 2:보행자/자전거, 3:자동차
	KeyID LinkId; // 링크 ID
	SPoint Coord; // 좌표
	SPoint MatchCoord; // 링크와 직교 접점 좌표
	int32_t Payed; // 유료 링크
	int32_t LinkSplitIdx; // 링크와 직교 접점 좌표의 링크 버텍스 idx
	int32_t LinkDistToS; // s로의 거리
	int32_t LinkDistToE; // e로의 거리
	int32_t LinkDir; // 탐색 방향, 0:미정의, 1:정방향(S->E), 2:역방향(E->S)
	int32_t LinkGuideType; // 링크 안내 타입, 0:일반, 1:출발지링크, 2:경유지링크, 3:도착지링크
	vector<SPoint> LinkVtxToS; // 좌표점에서 s버텍스, 종료링크의 경우는 FromS로 이해할것
	vector<SPoint> LinkVtxToE; // 좌표점에서 e버텍스, 종료링크의 경우는 FromS로 이해할것

	_tagRouteLinkInfo() {
		init();
	}

	~_tagRouteLinkInfo() {
		init();
	}

	void init() {
		KeyType = 0;
		LinkDataType = 0;
		LinkId.llid = 0;
		memset(&Coord, 0x00, sizeof(Coord));
		memset(&MatchCoord, 0x00, sizeof(MatchCoord));
		Payed = 0;
		LinkSplitIdx = -1;
		LinkDistToS = 0;
		LinkDistToE = 0;
		LinkDir = 0;
		LinkGuideType = 0;

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
	uint32_t MobilityOption; // 이동체

	// 경탐 세부옵션, 임시방법이고, 좀더 깔끔한 다른 방식을 고민해 보자, 2023.11.07
	stRouteSubOption RouteSubOpt;

	uint32_t StartDirIgnore; // 출발지 방향성 무시
	uint32_t EndDirIgnore; // 도착지 방향성 무시

	// 입구점 여부 확인 필요 
	MultimodalPointInfo ComplexPointInfo;
	MultimodalPointInfo ComplexPointReverseInfo;

	vector<CandidateLink*> vtCandidateResult;
	unordered_map<uint64_t, CandidateLink*> mRoutePass; // 정방향 탐색
	unordered_map<uint64_t, CandidateLink*> mRouteReversePass; // 역방향 탐색

#if defined(USE_FOREST_DATA)
	unordered_set<uint32_t> CandidateCourse;
#endif

	priority_queue<CandidateLink*, vector<CandidateLink*>, decltype(CompareCanidate)> pqDijkstra{ CompareCanidate };

	_tagRouteInfo() {
		RequestMode = 0;
		RequestId = 0;

		RouteOption = 0;
		AvoidOption = 0;
		MobilityOption = 0;

		StartDirIgnore = 0;
		EndDirIgnore = 0;
	}

	~_tagRouteInfo() {
		release();
	}

	void release() {
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

		if (!mRouteReversePass.empty()) {
			for (unordered_map<uint64_t, CandidateLink*>::iterator it = mRouteReversePass.begin(); it != mRouteReversePass.end(); it++) {
				if (it->second) {
					SAFE_DELETE(it->second);
				}
			}
			mRouteReversePass.clear();
			unordered_map<uint64_t, CandidateLink*>().swap(mRouteReversePass);
		}
	}
}RouteInfo;


typedef struct _tagRequestRouteInfo {
	uint32_t RequestMode; // 요청 모드
	uint32_t RequestId; // 요청 ID

	uint32_t RouteOption; // 경탐 옵션
	uint32_t AvoidOption; // 회피 옵션
	uint32_t MobilityOption; // 이동체, 0:미정의, 1:보행자, 2:자전거, 3:오토바이, 4:자동차

	// 경탐 세부옵션, 임시방법이고, 좀더 깔끔한 다른 방식을 고민해 보자, 2023.11.07
	stRouteSubOption RouteSubOption;

	int32_t StartDirIgnore; // 출발지 방향성 무시
	int32_t WayDirIgnore; // 경유지 방향성 무시
	int32_t EndDirIgnore; // 도착지 방향성 무시

	vector<RouteLinkInfo> vtPointsInfo;

	// vtPointsInfo 로 모아야함..
	vector<SPoint> vtPoints; // 요청 좌표
	vector<int32_t> vtKeyType; //  요청 키 타입, 0:미지정, 1:노드, 2:링크
	vector<int32_t> vtLinkDataType; // 0:미정의, 1:숲길, 2:보행자/자전거, 3:자동차
	vector<KeyID> vtKeyId; // 요청 키 id
#if defined(USE_FOREST_DATA)
	vector<unordered_set<uint32_t>> vtCourse; // 매칭 링크의 코스 정보
#endif

	_tagRequestRouteInfo() {
		RequestMode = 0;
		RequestId = 0;

		RouteOption = 0;
		AvoidOption = 0;
		MobilityOption = 0;

		memset(&RouteSubOption, 0x00, sizeof(RouteSubOption));

		StartDirIgnore = 0;
		WayDirIgnore = 0;
		EndDirIgnore = 0;
	}

	~_tagRequestRouteInfo() {
		if (vtPointsInfo.empty()) {
			vtPointsInfo.clear();
			vector<RouteLinkInfo>().swap(vtPointsInfo);
		}

		if (!vtPoints.empty()) {
			vtPoints.clear();
			vector<SPoint>().swap(vtPoints);
		}

		if (!vtKeyType.empty()) {
			vtKeyType.clear();
			vector<int32_t>().swap(vtKeyType);
		}

		if (!vtKeyId.empty()) {
			vtKeyId.clear();
			vector<KeyID>().swap(vtKeyId);
		}

		if (!vtLinkDataType.empty()) {
			vtLinkDataType.clear();
			vector<int32_t>().swap(vtLinkDataType);
		}
#if defined(USE_FOREST_DATA)
		if (!vtCourse.empty()) {
			vtCourse.clear();
			vector<unordered_set<uint32_t>>().swap(vtCourse);
		}
#endif
	}
}RequestRouteInfo;


typedef struct _tagRouteTable {

	uint32_t nOriginIdx;
	uint32_t nBestIdx;
	int32_t nUsable; // 요청 모드 // -1:미사용, 0:미방문, n:방문횟수
	// uint32_t nTotalDist;
	// uint32_t nTotalTime;
	// double dbTotalCost;

	RouteLinkInfo linkInfo;

	_tagRouteTable() {
		nOriginIdx = 0;
		nBestIdx = 0;
		nUsable = -1;
		// nTotalDist = 0;
		// nTotalTime = 0;
		// dbTotalCost = 0;
	}

	~_tagRouteTable() {
	}
}RouteTable;


typedef struct _tagTableBaseInfo {
	RouteLinkInfo					routeLinkInfo;
	vector<CandidateLink*>			vtCandidateLink;
	unordered_map<uint64_t, bool>	mRoutePass;

	uint32_t						routeOption; // 경탐 옵션
	uint32_t						avoidOption; // 회피 옵션
	uint32_t						mobilityOption; // 이동체

	size_t							tickStart; // 시작 시각
	size_t							tickFinish; // 종료 시각

	priority_queue<CandidateLink*, vector<CandidateLink*>, decltype(CompareCanidate)> pqDijkstra{ CompareCanidate };


	_tagTableBaseInfo() {
		routeOption = 0;
		avoidOption = 0;
		mobilityOption = 0;
		tickStart = 0;
		tickFinish = 0;
	}

	~_tagTableBaseInfo() {
		//for (; !pqDijkstra.empty(); pqDijkstra.pop());
		//or LOG_TRACE(LOG_TEST, "Release Table Base Info, row:%d, link tile:%d, id:%d, queue_size:%d, pass_size:%d", testRowId, routeLinkInfo.LinkId.tile_id, routeLinkInfo.LinkId.nid, pqDijkstra.size(), mRoutePass.size());
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
			vector<CandidateLink*>().swap(vtCandidateLink);
		}
	}
}TableBaseInfo;


class CRoutePlan
{
public:
	CRoutePlan();
	~CRoutePlan();

	void Initialize(void);
	void Release(void);

	void SetDataMgr(IN CDataManager* pDataMgr);

	const DataCost* GetRouteCost(void) const;
	void SetRouteCost(IN const DataCost* pCost);
	void SetRouteCost(IN const uint32_t type, IN const DataCost* pCost, IN const uint32_t cntCost = 0);

	const int Planning(IN const RequestRouteInfo* pReqInfo, OUT RouteResultInfo* pRouteResult);

	const int DoRoute(IN const uint32_t reqId, IN const SPoint ptStart, IN const SPoint ptEnd, IN const KeyID sLink, IN const KeyID eLink, IN const uint32_t routeOpt, IN const uint32_t avoidOpt, IN const uint32_t mobilityOpt, IN const stRouteSubOption subOpt, IN const bool ignoreStartDir, IN const bool ignoreEndDir , OUT RouteResultInfo* pRouteResult);
	const int DoRoutes(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults);
	const int DoComplexRoutes(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults);
	const int DoComplexRoutesEx(IN const RequestRouteInfo* pReqInfo/*, IN OUT vector<ComplexPointInfo>& vtCpxRouteInfo*/, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults);
	const int DoCourse(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults);
#if defined(USE_TSP_MODULE)
	//const int DoTabulate(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults);
	// const int DoTabulate(IN const RequestRouteInfo* pReqInfo, OUT RouteTable** ppResultTables);	
	const int DoTabulate(IN const RequestRouteInfo* pReqInfo, OUT vector<vector<stDistMatrix>>& vtDistMatrix);	
#endif
	// 미확인된 필요 노드 정보 획득 위해, 탐색 가능한 만큼 확장 후 필요한 노드 정보 확인 (입구점 등)
	const int DoEdgeRoute(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>& vtRouteInfos/*, OUT vector<ComplexPointInfo>& vtComplexPointInfo*/);
	// 확인된 모든 노드까지 경로 확장 (입구점 등)
	//const int DoEntryPointRoute(IN RouteInfo* pRouteInfo, IN const int32_t idx, IN const bool isFromPed, IN OUT vector<ComplexPointInfo>& vtComplexPointInfo);
	const int DoEntryPointRoute(IN RouteInfo* pRouteInfo, IN const bool isFromPed, IN const MultimodalPointInfo* pCpxPointInfoTrk, OUT MultimodalPointInfo& cpxPointInfoPed);

	const int GetMostProbableLink(IN const RouteResultInfo* pResult, OUT vector<RouteLinkInfo>& vtMPP);
	const bool GetRouteTravelSpeed(IN OUT RouteResultInfo* pResult);
	
#if defined(USE_SHOW_ROUTE_SATATUS)
	void SetRouteStatusFunc(IN const void* fpHost, IN void(*drawRouting)(const void*, const unordered_map<uint64_t, CandidateLink*>*));
#endif

private:
	const bool SetRouteLinkInfo(IN const SPoint& ptPosition, IN const KeyID keyLink, IN const int32_t linkDataType, IN const int32_t linkDir, IN const bool isStart, IN const bool isDirIgnore, OUT RouteLinkInfo* pRouteLinkInfo);

	const bool SetVisitedLink(IN RouteInfo* pRouteInfo, IN const KeyID linkId, IN const bool isReverse = false);
	const bool IsVisitedLink(IN RouteInfo* pRouteInfo, IN const KeyID linkId, IN const bool isReverse = false);
	const bool IsAddedLink(IN RouteInfo* pRouteInfo, IN const KeyID linkId, IN const bool isReverse = false);
	const int AddNextLinks(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo);
	const int AddPrevLinks(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo);
	const int AddNextCourse(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const set<uint64_t>* psetCourseLinks);

	const int Propagation(IN TableBaseInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const int dir, IN const SPoint target); // 단순 확장
	const int EdgePropagation(IN const int32_t idx, IN const bool isReverse, IN const vector<stOptimalPointInfo>& vtOptInfo, IN OUT RouteInfo* pRouteInfo, IN const vector<CandidateLink*> vtCandidateInfo/*, OUT vector<ComplexPointInfo>&vtComplexPointInfo*/); // 입구점 확장

	const int MakeRoute(IN const int idx, IN RouteInfo* pRouteInfo, OUT RouteResultInfo* pRouteResult);
	const int MakeCourse(IN const int idx, IN RouteInfo* pRouteInfo, OUT RouteResultInfo* pRouteResult);
#if defined(USE_TSP_MODULE)
	// const int MakeTabulate(IN const RequestRouteInfo* pReqInfo, IN const vector<RouteLinkInfo>& LinkInfos, OUT RouteTable** ppResultTables);
	const int MakeTabulate(IN const RequestRouteInfo* pReqInfo, IN const vector<RouteLinkInfo>& LinkInfos, OUT vector<vector<stDistMatrix>>& vtDistMatrix);
#endif
	const int MakeRouteResult(IN RouteInfo* pRouteInfo, OUT RouteResultInfo* pRouteResult);
	//const int MakeRouteResultEx(IN const RouteInfo* pRouteInfo/*, IN const vector<ComplexPointInfo>* vtComplexPointInfo*/, IN const int32_t idx, IN const int32_t bestIdx, OUT RouteResultInfo* pRouteResult);
	//const int MakeRouteResultAttatch(IN const RouteInfo* pRouteInfo, IN const CandidateLink* pCurrResult, IN const bool isFromPed, OUT RouteResultInfo* pRouteResult);
	const int MakeRouteResultAttatchEx(IN const RouteInfo* pRouteInfo, IN const bool isReverse, IN const int32_t typeStartKey, IN const int32_t typeEndKey, IN const CandidateLink* pCurrResult, IN const int32_t nGuideType, OUT RouteResultInfo* pRouteResult);

	const uint8_t GetLinkSpeed(IN const KeyID linkId, IN const uint8_t level);
	const double GetCost(IN const stLinkInfo* pLink, IN const uint32_t dirTarget, IN const uint32_t opt, IN const uint32_t mobility,IN const double length, IN const uint8_t spd = 255);
	const double GetTravelCost(IN const stLinkInfo* pLink, IN const stLinkInfo* pLinkPrev, IN const int spd, IN const double cost, IN const int angle, IN const uint32_t opt, IN const uint32_t avoid, IN const uint32_t mobility, IN const uint32_t payedArea);
	const double GetCourseCost(IN const RouteInfo* pRouteInfo, IN const stLinkInfo* pLink, IN const double cost);
	const uint32_t CheckStartDirectionMaching(IN const stLinkInfo* pLink, IN const RouteLinkInfo* pRoutLinkInfo, IN const int32_t routeOpt, IN const int32_t mobilityOpt, OUT vector<CandidateLink*>& vtCandidateInfo);
	const uint32_t CheckEndDirectionMaching(IN const stLinkInfo* pLink, IN const RouteLinkInfo* pRoutLinkInfo, IN const int32_t routeOpt, IN const int32_t mobilityOpt, OUT vector<CandidateLink*>& vtCandidateInfo);
	const int GetAngle(IN const stLinkInfo* pLink, IN const int dir, IN const bool useTail = true); // dir, 1:정, 2:역, useTail, 종료 링크 사용 여부
	const int GetPathAngle(IN const stLinkInfo* pPrevLink, IN const stLinkInfo* pNextLink);
	const stNodeInfo* GetNextNode(IN const CandidateLink* pCurInfo); // 다음 노드 정보 가져오기
	const stNodeInfo* GetPrevNode(IN const CandidateLink* pCurInfo); // 이전 노드 정보 가져오기



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

	DataCost m_rpCost;
};



