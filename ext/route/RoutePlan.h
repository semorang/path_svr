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
#define VAL_HEURISTIC_VEHICLE_FACTOR_FOR_TABLE 0.1f//0.5f // 1.0f//0.1f
// #define VAL_HEURISTIC_PEDESTRIAN_FACTOR 0.9f
#define VAL_HEURISTIC_PEDESTRIAN_FACTOR 0.2f
#define VAL_HEURISTIC_FOREST_FACTOR 0.5f
#define VAL_HEURISTIC_FACTOR_MIN_DIST 3000.f // 휴리스틱 적용 최소 거리, 초과일 경우 factor를 증가
#define VAL_DISTANCE_FACTOR_MIN_DIST 5000.f // 출/도착지간 현재 링크의 최소 거리 초과일 경우 factor를 증가
//#define VAL_HEURISTIC_FACTOR 1.9f

#define USE_REQUEST_ROUTEINFO // 요청 옵션을 하나의 구조로 유지 관리, 2025-02-17

#define USE_RDM_COST_RESET // 각 방문지 도착시 다음 방문지로 가기전 확장 코스트를 재설정, 2025-02-28


#define TEST_LEVEL_PROPAGATION // 각 출발지에 대해 특정 상위 레벨을 모두 확장 후, 목적지에서 확장된 상위 레벨까지의 경로를 역탐색 하는 방법, 2025-03-17
#define USE_LINK_HISTORY_TABLE // 현재는 사용하지 않는데, 추가 요청이 계속되고 있어 작업 중, 2025-02-17
// 구현시 용량 이슈 있을 수 있으니 검토 필요, 100개 전국 시 GiB 용량 생성 가능(경로선형정보(x/y double 16byte)

typedef struct _tagCandidateLink {
	KeyID candidateId; // 후보 ID
	KeyID parentLinkId; // 부모 링크 ID
	KeyID linkId; // 링크 ID
	KeyID nodeId; // 노드 ID
	float dist; // 거리
	float time; // 시간
	float cost; // 코스트
	float distTreavel; // 누적 거리
	float timeTreavel; // 누적 시간
	double costTreavel; // 누적 계산 비용
	double costHeuristic; // 방향 가중치 계산 비용
	uint32_t linkDataType; // 데이터 타입, // 0:미정의, 1:숲길, 2:보행자/자전거, 3:자동차
	uint32_t depth : 29; // 탐색 깊이
	uint32_t visited : 1; // 방문 여부
	uint32_t dir : 2; // 탐색 방향, 0:미정의, 1:정방향(S->E), 2:역방향(E->S)
	uint8_t angle; // 이전 링크에서 진입각도
#if defined(USE_VEHICLE_DATA)
	uint8_t speed; // 속도
	uint8_t speed_type : 4; // 속도 타입, 0:미정의, 1:ttl, 2:ks, 3:static
	uint8_t speed_level : 4; // 도로 레벨,// 경로레벨, 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상, 4:일반도로6차선이상, 5:일반도로4차선이상, 6:일반도로2차선이상, 7:일반도로1차선이상, 8:SS도로, 9:GSS도로/단지내도로/통행금지도로/비포장도로
#endif
	bool isEnable;

	_tagCandidateLink* pPrevLink;

	_tagCandidateLink() {
		candidateId.llid = 0;
		parentLinkId.llid = 0;
		linkId.llid = 0;
		nodeId.llid = 0;
		dist = 0.f;
		time = 0.f;
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
		isEnable = true;

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

struct distanceCandidate {
	distanceCandidate(const int32_t _id, const int32_t _dist) : id(_id), dist(_dist) {};
	int32_t id;
	int32_t dist;
};


struct cmpNearestDist {
	bool operator()(const distanceCandidate lhs, const distanceCandidate rhs) {
		return lhs.dist > rhs.dist;
	}
};


struct cmpFarthestDist
{
	bool operator()(const distanceCandidate lhs, const distanceCandidate rhs)
	{
		return lhs.dist < rhs.dist;
	}
};


typedef struct _tagstRouteSubOption {
	union {
		int64_t option;
		struct {
			int32_t course_type; // 0:등산, 1:걷길, 2:자전거, 3:코스 // 2024-05-28 KVX 요청으로 변경, // old 코스 타입 // 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
			int32_t course_id; // 코스 ID
		}mnt;
		struct {
			uint32_t start_id;
			uint32_t end_id;
		}p2p;
		struct {
			char distance_type; // "0:NONE, 1:ROUTE"
			char compare_type; // "0:NONE, 1:COST, 2:DIST, 3:TIME"
			char expand_method; // "0:NONE, 1:LEVEL_PROPAGATION"
			char reserved[5];
		}rdm;
	};

	_tagstRouteSubOption& operator=(const _tagstRouteSubOption& rhs)
	{
		option = rhs.option;
		return *this;
	}
}stRouteSubOption;


typedef struct _tagstRouteTruckOption
{
private:
	TruckOption truck_option;

public:
	const bool isAvoidTruckOption() {
		return truck_option.isEnableTruckOption();
	}

	void setTruckOption(IN const TruckOption* pOption) {
		memcpy(&truck_option, pOption, sizeof(truck_option));
	}

	_tagstRouteTruckOption& operator=(const _tagstRouteTruckOption& rhs)
	{
		memcpy(&truck_option, &rhs, sizeof(truck_option));
		return *this;
	}

	int height() { return truck_option.height; }
	int weight() { return truck_option.weight; }
	int length() { return truck_option.length; }
	int width() { return truck_option.width; }
	int hazardous() { return truck_option.hazardous; }
	TruckOption* option() { return &truck_option;  }
}stRouteTruckOption;


typedef struct _tagRouteLinkInfo {
	int32_t KeyType; // 0:미지정, 1:노드, 2:링크
	int32_t LinkDataType;  // 0:미정의, 1:숲길, 2:보행자/자전거, 3:자동차
	KeyID LinkId; // 링크 ID
	SPoint Coord; // 좌표
	SPoint MatchCoord; // 링크와 직교 접점 좌표
	int32_t Payed; // 유료 링크 <- LinkSubInfo 추가되어서 필요없을 듯 2025-02-25
	int32_t LinkSplitIdx; // 링크와 직교 접점 좌표의 링크 버텍스 idx
	int32_t LinkDistToS; // s로의 거리
	int32_t LinkDistToE; // e로의 거리
	int32_t LinkDir; // 탐색 방향, 0:미정의, 1:정방향(S->E), 2:역방향(E->S)
	int32_t LinkGuideType; // 링크 안내 타입, 0:일반, 1:출발지링크, 2:경유지링크, 3:도착지링크
	vector<SPoint> LinkVtxToS; // 좌표점에서 s버텍스, 종료링크의 경우는 FromS로 이해할것
	vector<SPoint> LinkVtxToE; // 좌표점에서 e버텍스, 종료링크의 경우는 FromE로 이해할것
	stLinkSubInfo LinkSubInfo; // 목적지 매칭 링크 코스트가 높아 주변에서 빙빙돌며 확장만 많이 하는 현상 해소 목적, 2025-02-25
	// 목적 지점 일정 반경(ex 100m)에서는 목적 지점 링크 속성과 동일한 링크는 가중치를 높이지 않는다.
	
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
		LinkSubInfo = 0;

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


typedef struct _tagRequestRouteInfo
{
	string RequestId; // 요청 ID
	uint32_t RequestMode; // 요청 모드, 0:일반(최초탐색, 정차 후 출발), 1:재탐색, 2:..., 100:대안경로
	uint32_t RequestTime; // 요청 시각
	uint32_t RequestTraffic; // 교통 정보, 0:미사용, 1:실시간(REAL), 2:통계(STATIC), 3:실시간-통계(REAL-STATIC)

	uint32_t RouteOption; // 경탐 옵션
	uint32_t AvoidOption; // 회피 옵션
	uint32_t MobilityOption; // 이동체, 0:보행자, 1:자전거, 2:오토바이, 3:자동차, 4:자율주행, 5:트럭, 6:긴급
	uint32_t FreeOption; // 무료 옵션, 0:미사용, 1:무료

	// 경탐 세부옵션, 임시방법이고, 좀더 깔끔한 다른 방식을 고민해 보자, 2023.11.07
	stRouteSubOption RouteSubOption;
	stRouteTruckOption RouteTruckOption;

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

	_tagRequestRouteInfo()
	{
		RequestId = "";
		RequestMode = 0;
		RequestTime = 0;
		RequestTraffic = 0;

		RouteOption = 0;
		AvoidOption = 0;
		MobilityOption = 0;
		FreeOption = 0;

		memset(&RouteSubOption, 0x00, sizeof(RouteSubOption));
		memset(&RouteTruckOption, 0x00, sizeof(RouteTruckOption));

		StartDirIgnore = 0;
		WayDirIgnore = 0;
		EndDirIgnore = 0;
	}

	~_tagRequestRouteInfo()
	{
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

	void SetOption(const _tagRequestRouteInfo* pRhs) // &= operator 사용하는게 낫지 않을라나? 2025-02-25
	{
		if (pRhs != nullptr) {
			RequestId = pRhs->RequestId;
			RequestMode = pRhs->RequestMode;
			RequestTime = pRhs->RequestTime;
			RequestTraffic = pRhs->RequestTraffic;

			RouteOption = pRhs->RouteOption;
			AvoidOption = pRhs->AvoidOption;
			MobilityOption = pRhs->MobilityOption;
			FreeOption = pRhs->FreeOption;

			RouteSubOption = pRhs->RouteSubOption;
			RouteTruckOption = pRhs->RouteTruckOption;

			StartDirIgnore = pRhs->StartDirIgnore;
			WayDirIgnore = pRhs->WayDirIgnore;
			EndDirIgnore = pRhs->EndDirIgnore;
		}
	}
}RequestRouteInfo;


typedef struct _tagRouteInfo {
#ifdef USE_REQUEST_ROUTEINFO
	RequestRouteInfo reqInfo;
#else
	string RequestId; // 요청 ID
	uint32_t RequestMode; // 요청 모드
	uint32_t RequestTime; // 요청 시각
	uint32_t RequestTraffic; // 교통 정보, 0:미사용, 1:실시간(REAL), 2:통계(STATIC), 3:실시간-통계(REAL-STATIC)

	uint32_t RouteOption; // 경탐 옵션
	uint32_t AvoidOption; // 회피 옵션
	uint32_t MobilityOption; // 이동체

	// 경탐 세부옵션, 임시방법이고, 좀더 깔끔한 다른 방식을 고민해 보자, 2023.11.07
	stRouteSubOption RouteSubOpt;

	uint32_t StartDirIgnore; // 출발지 방향성 무시
	uint32_t EndDirIgnore; // 도착지 방향성 무시
#endif // #ifdef USE_REQUEST_ROUTEINFO

	RouteLinkInfo StartLinkInfo;
	RouteLinkInfo EndLinkInfo;

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
#ifdef USE_REQUEST_ROUTEINFO		

#else
		RequestId = "";
		RequestMode = 0;
		RequestTime = 0;
		RequestTraffic = 0;

		RouteOption = 0;
		AvoidOption = 0;
		MobilityOption = 0;

		StartDirIgnore = 0;
		EndDirIgnore = 0;
#endif
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


typedef struct _tagRouteResultLink
{
	KeyID linkId;
	KeyID nodeId;
	uint32_t vertexIdx : 16; // 최대 65353
	uint32_t linkLength : 16; // 최대 65353
	uint32_t linkTime : 16; // 최대 65353
	uint32_t dir : 16; // 최대 65353
}RouteResultLink;

typedef struct _tagRouteResultLinkEx
{
	KeyID link_id;
	KeyID node_id;
	uint64_t link_info;
	float length; // link length
	uint32_t time : 16; // link time // 최대 65535 > 18 h
	uint32_t vtx_off : 12; // vertex 시작 idx // 최대 4096
	uint32_t vtx_cnt : 12; // vertex 갯수 // 최대 4096
	uint32_t rlength : 21; // remain length // 최대 2097151 > 2,000km
	uint32_t rtime : 17; // remain time // 최대 86400 > 31h
	uint32_t angle : 9; // 진출 각도 // 360도 방식
	uint32_t dir : 1; // 링크 방향성, 0:정방향, 1:역방향 // MAP API 결과에 맞추기 위해 변경
	uint32_t guide_type : 4; // 링크 안내 타입, 0:일반, 1:S출발지링크, 2:E도착지링크, 3:V경유지링크, 4:SE출발지-도착지, 5:SV출발지-경유지, 6:VV경유지-경유지, 7, VE경유지-도착지
	uint32_t reserved : 2;
#if defined(USE_VEHICLE_DATA)
	uint8_t speed; // 속도
	uint8_t speed_type : 4; // 속도 타입, 0:미정의, 1:ttl, 2:ks, 3:static
	uint8_t speed_level : 4; // 도로 레벨,// 경로레벨, 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상, 4:일반도로6차선이상, 5:일반도로4차선이상, 6:일반도로2차선이상, 7:일반도로1차선이상, 8:SS도로, 9:GSS도로/단지내도로/통행금지도로/비포장도로
#endif
}RouteResultLinkEx;

typedef struct _tagRouteResultLinkMatchInfo
{
	SPoint Coord; // 좌표
	KeyID LinkId; // 링크 ID
	SPoint MatchCoord; // 링크와 직교 접점 좌표
	int32_t LinkDataType; // 0:미정의, 1:숲길, 2:보행자/자전거, 3:자동차 // 안씀 --> 0:미정의, 1:명칭사전, 2:메쉬, 3:숲길, 4:보행자/자전거, 5:자동차, 6:건물, 7:단지, 8:입구점, 9:교통정보, 10:산바운더리, 11:코스정보
	int32_t LinkSplitIdx; // 링크와 직교 접점 좌표의 링크 버텍스 idx
	int32_t LinkDist; // 직교 접점에서 노드까지의 거리
	vector<SPoint> LinkVtx; // 좌표점에서 s버텍스

	~_tagRouteResultLinkMatchInfo()
	{
		if (!LinkVtx.empty()) {
			LinkVtx.clear();
			vector<SPoint>().swap(LinkVtx);
		}
	}

	void Init()
	{
		memset(&Coord, 0x00, sizeof(Coord));
		LinkId.llid = 0;
		memset(&MatchCoord, 0x00, sizeof(MatchCoord));
		LinkDataType = 0;
		LinkSplitIdx = 0;
		LinkDist = 0;

		if (!LinkVtx.empty()) {
			LinkVtx.clear();
			vector<SPoint>().swap(LinkVtx);
		}
	}

	_tagRouteResultLinkMatchInfo& operator=(const _tagRouteResultLinkMatchInfo& rhs)
	{
		Coord = rhs.Coord; // 좌표
		LinkId = rhs.LinkId; // 링크 ID
		MatchCoord = rhs.MatchCoord; // 링크와 직교 접점 좌표
		LinkDataType = rhs.LinkDataType;
		LinkSplitIdx = rhs.LinkSplitIdx; // 링크와 직교 접점 좌표의 링크 버텍스 idx
		LinkDist = rhs.LinkDist; // 직교 접점에서 노드까지의 거리
		LinkVtx.assign(rhs.LinkVtx.begin(), rhs.LinkVtx.end()); // 좌표점에서 s버텍스

		return *this;
	}
} RouteResultLinkMatchInfo;


typedef struct _tagRouteSummary
{
	uint32_t TotalDist;
	uint32_t TotalTime;
} RouteSummary;


typedef struct _tagRouteResultInfo
{
	uint32_t ResultCode; // 경로 결과 코드, 0:성공, 1~:실패

	RequestRouteInfo reqInfo;
	//string RequestId; // 요청 ID
	//uint32_t RequestMode; // 요청 모드
	//uint32_t RequestTime; // 요청 시각
	//uint32_t RequestTraffic; // 교통 정보, 0:미사용, 1:실시간(REAL), 2:통계(STATIC), 3:실시간-통계(REAL-STATIC)

	//uint32_t RouteOption; // 경로 옵션
	//uint32_t RouteAvoid; // 경로 회피
	//uint32_t RouteMobility; // 이동체

	RouteResultLinkMatchInfo StartResultLink;
	RouteResultLinkMatchInfo EndResultLink;

	double TotalLinkDist; // 경로 전체 거리
	uint32_t TotalLinkCount; // 경로 전체 링크 수
	uint32_t TotalLinkTime; // 경로 전체 소요 시간 (초)

	// 경로선
	SBox RouteBox; // 경로선 영역	
	vector<RouteSummary> RouteSummarys; // 다중 탐색 결과의 개별 경로 요약 정보
	vector<RouteResultLinkEx> LinkInfo; // 링크 정보
	vector<SPoint> LinkVertex; // 경로선

	void Init()
	{
		ResultCode = ROUTE_RESULT_FAILED;

		//RequestId = "";
		//RequestMode = 0;
		//RequestTime = 0;

		//RouteOption = 0;
		//RouteAvoid = 0;
		//RouteMobility = 0;

		StartResultLink.Init();
		EndResultLink.Init();

		TotalLinkDist = 0.f;
		TotalLinkCount = 0;
		TotalLinkTime = 0;

		memset(&RouteBox, 0x00, sizeof(RouteBox));

		if (!RouteSummarys.empty()) {
			RouteSummarys.clear();
			vector<RouteSummary>().swap(RouteSummarys);
		}
		if (!LinkInfo.empty()) {
			LinkInfo.clear();
			vector<RouteResultLinkEx>().swap(LinkInfo);
		}
		if (!LinkVertex.empty()) {
			LinkVertex.clear();
			vector<SPoint>().swap(LinkVertex);
		}
	}// Init()

}RouteResultInfo;


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

	unordered_map<uint64_t, CandidateLink*> mRoutePass;

#ifdef USE_REQUEST_ROUTEINFO
	RequestRouteInfo				reqInfo;
#else
	uint32_t						routeOption; // 경탐 옵션
	uint32_t						avoidOption; // 회피 옵션
	uint32_t						mobilityOption; // 이동체
	uint32_t						timeOption; // 요청시각
	uint32_t						trafficOption; // 교통 정보, 0:미사용, 1:실시간(REAL), 2:통계(STATIC), 3:실시간-통계(REAL-STATIC)
#endif

	size_t							tickStart; // 시작 시각
	size_t							tickFinish; // 종료 시각
#if defined(USE_TMS_API)
	int32_t							cntReplaced; // 경로 변경 횟수
#endif


	priority_queue<CandidateLink*, vector<CandidateLink*>, decltype(CompareCanidate)> pqDijkstra{ CompareCanidate };


	_tagTableBaseInfo() {
#ifdef USE_REQUEST_ROUTEINFO

#else
		routeOption = 0;
		avoidOption = 0;
		mobilityOption = 0;
		timeOption = 0;
		trafficOption = 0;
#endif

		tickStart = 0;
		tickFinish = 0;
#if defined(USE_TMS_API)
		cntReplaced = 0;
#endif
	}

	~_tagTableBaseInfo() {
		release();
	}

	void initPass()
	{
		//for (; !pqDijkstra.empty(); pqDijkstra.pop());
		//or LOG_TRACE(LOG_TEST, "Release Table Base Info, row:%d, link tile:%d, id:%d, queue_size:%d, pass_size:%d", testRowId, routeLinkInfo.LinkId.tile_id, routeLinkInfo.LinkId.nid, pqDijkstra.size(), mRoutePass.size());

		for (; !pqDijkstra.empty(); ) {
			//delete pqDijkstra.top();
			pqDijkstra.pop();
		}
		
		for (unordered_map<uint64_t, CandidateLink*>::iterator it = mRoutePass.begin(); it != mRoutePass.end(); it++) {
			if (it->second) {
				SAFE_DELETE(it->second);
			}
		}
		mRoutePass.clear();
		unordered_map<uint64_t, CandidateLink*>().swap(mRoutePass);
	}

	void release()
	{
		initPass();

		if (!vtCandidateLink.empty()) {
			vtCandidateLink.clear();
			vector<CandidateLink*>().swap(vtCandidateLink);
		}
	}
}TableBaseInfo;


typedef struct _tagRoutePath
{
	uint64_t linkId; // 링크 ID
	uint8_t dir : 2; // 탐색 방향, 0:미정의, 1:정방향(S->E), 2:역방향(E->S)
#if defined(USE_VEHICLE_DATA)
	uint8_t speed; // 속도
	uint8_t speed_type : 4; // 속도 타입, 0:미정의, 1:ttl, 2:ks, 3:static
	uint8_t speed_level : 4; // 도로 레벨,// 경로레벨, 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상, 4:일반도로6차선이상, 5:일반도로4차선이상, 6:일반도로2차선이상, 7:일반도로1차선이상, 8:SS도로, 9:GSS도로/단지내도로/통행금지도로/비포장도로
#endif
}stRoutePath;


typedef struct _tagstPathMatrix
{
	RouteLinkInfo startLinkInfo;
	RouteLinkInfo endLinkInfo;
	vector<stRoutePath> vtRoutePath;
}stPathMatrix;


typedef struct _tagRouteDistMatrix
{
	int32_t typeCreate; // 생성 타입, 0:엔진생성 데이터, 1:서버 파일 데이터, 2:사용자 데이터
	string strUser;
	time_t tmCreate;
	vector<Origins> vtOrigins;
	vector<vector<stDistMatrix>> vtDistMatrix;
	vector<vector<stPathMatrix>> vtPathMatrix;

	_tagRouteDistMatrix() {
		strUser.clear();
		typeCreate = 0;
		tmCreate = 0;
	}
}RouteDistMatrix;


typedef struct _tagRouteDistMatrixLine
{
	string strFileName;
	size_t sizeFile;
	vector<vector<FileIndex>> vtPathFileIndex;

	_tagRouteDistMatrixLine() {
		strFileName.clear();
		sizeFile = 0;
	}
}RouteDistMatrixLine;


typedef struct _tagBestWaypoint
{
	string strUser;
	time_t tmCreate;
	TspOption option;
	vector<stWaypoints> vtWaypoints;
	vector<int32_t> vtBestWays;
	double totalDist;
	int totalTime;
	int totalSpot;

	_tagBestWaypoint()
	{
		strUser.clear();
		tmCreate = 0;

		totalDist = 0.f;
		totalTime = 0;
		totalSpot = 0;
	}
}BestWaypoints;


typedef struct _tagCluster
{
	string strUser;
	time_t tmCreate;
	ClusteringOption option;
	vector<Origins> vtOrigins;
	vector<stDistrict> vtDistrict;
	vector<SPoint> vtPositionLock;

	_tagCluster() 
	{
		strUser.clear();
		tmCreate = 0;
	}
}Cluster;


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

	const int DoRoute(IN const string& reqId, IN const SPoint ptStart, IN const SPoint ptEnd, IN const KeyID sLink, IN const KeyID eLink, IN const uint32_t routeOpt, IN const uint32_t avoidOpt, IN const uint32_t mobilityOpt, IN const stRouteSubOption subOpt, IN const bool ignoreStartDir, IN const bool ignoreEndDir , OUT RouteResultInfo* pRouteResult);
	const int DoRoutes(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults);
	const int DoComplexRoutes(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults);
	const int DoComplexRoutesEx(IN const RequestRouteInfo* pReqInfo/*, IN OUT vector<ComplexPointInfo>& vtCpxRouteInfo*/, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults);
	const int DoCourse(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults);
#if defined(USE_TSP_MODULE)
	const int DoTabulate(IN const RequestRouteInfo* pReqInfo, OUT RouteDistMatrix& RDM);
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
	const int AddNextLinks(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const SBox* pBoxExpandedArea = nullptr);
	const int AddPrevLinks(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo);
	const int AddPrevLinksEx(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo);
	const int AddNextCourse(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const set<uint64_t>* psetCourseLinks);

	const int Propagation(IN TableBaseInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const int dir, IN const SBox& boxExpandArea, IN const vector<SPoint>& vtOrigins, IN const int32_t currIdx, IN const int32_t nextIdx, IN const uint32_t timestamp); // 단순 확장
	const int EdgePropagation(IN const int32_t idx, IN const bool isReverse, IN const vector<stOptimalPointInfo>& vtOptInfo, IN OUT RouteInfo* pRouteInfo, IN const vector<CandidateLink*> vtCandidateInfo/*, OUT vector<ComplexPointInfo>&vtComplexPointInfo*/); // 입구점 확장
	const int LevelPropagation(IN TableBaseInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const int dir, IN const SBox& boxExpandArea, IN const int32_t nLimtLevel, IN OUT int32_t& enableStartLevelIgnore); // 모든 레벨 확장, // enableStartLevelIgnore: 시작 링크가 제한 링크보다 낮은 링크일때 제한 링크까지 허용할 지 여부

	const int MakeRoute(IN const int idx, IN RouteInfo* pRouteInfo, OUT RouteResultInfo* pRouteResult);
	const int MakeCourse(IN const int idx, IN RouteInfo* pRouteInfo, OUT RouteResultInfo* pRouteResult);
#if defined(USE_TSP_MODULE)
	const int MakeTabulate(IN const RequestRouteInfo* pReqInfo, IN const vector<RouteLinkInfo>& LinkInfos, OUT RouteDistMatrix& RDM);
	const int MakeTabulateEx(IN const RequestRouteInfo* pReqInfo, IN const vector<RouteLinkInfo>& LinkInfos, OUT RouteDistMatrix& RDM); // reverse expand route from destination
#endif
	const int MakeRouteResult(IN RouteInfo* pRouteInfo, OUT RouteResultInfo* pRouteResult);
	//const int MakeRouteResultEx(IN const RouteInfo* pRouteInfo/*, IN const vector<ComplexPointInfo>* vtComplexPointInfo*/, IN const int32_t idx, IN const int32_t bestIdx, OUT RouteResultInfo* pRouteResult);
	//const int MakeRouteResultAttatch(IN const RouteInfo* pRouteInfo, IN const CandidateLink* pCurrResult, IN const bool isFromPed, OUT RouteResultInfo* pRouteResult);
	const int MakeRouteResultAttatchEx(IN const RouteInfo* pRouteInfo, IN const bool isReverse, IN const int32_t typeStartKey, IN const int32_t typeEndKey, IN const CandidateLink* pCurrResult, IN const int32_t nGuideType, OUT RouteResultInfo* pRouteResult);

	const double GetTravelTime(IN const RequestRouteInfo* pReqInfo, IN const stLinkInfo* pLink, IN const int spd, IN const int angle, IN const double dist, IN const double time); // 사용자에게 전달될 시간

	const uint8_t GetLinkSpeed(IN const KeyID linkId, IN const uint8_t level, IN const uint32_t opt);
	/*const double GetCost(IN const stLinkInfo* pLink, IN const uint32_t dirTarget, IN const uint32_t opt, IN const uint32_t mobility,IN const double length, IN const uint8_t spd = 255);*/
	//const double GetTravelCost(IN const stLinkInfo* pLink, IN const stLinkInfo* pLinkPrev, IN const int spd, IN const double cost, IN const int angle, IN const uint32_t opt, IN const uint32_t avoid, IN const uint32_t mobility, IN const stLinkSubInfo subinfo);
	const double GetCost(IN const RequestRouteInfo* pReqInfo, IN const stLinkInfo* pLink, IN const uint32_t dirTarget, IN const double length, IN const uint8_t spd = 255);
	const double GetTravelCost(IN const RequestRouteInfo* pReqInfo, IN const stLinkInfo* pLink, IN const stLinkInfo* pLinkPrev, IN const RouteLinkInfo* pLinkInfo, IN const int spd, IN const double cost, IN const int angle);
	const double GetCourseCost(IN const RouteInfo* pRouteInfo, IN const stLinkInfo* pLink, IN const double cost);
	//const uint32_t CheckStartDirectionMaching(IN const stLinkInfo* pLink, IN const RouteLinkInfo* pRoutLinkInfo, IN const int32_t routeOpt, IN const int32_t mobilityOpt, IN const uint32_t timestampOpt, OUT vector<CandidateLink*>& vtCandidateInfo);
	//const uint32_t CheckEndDirectionMaching(IN const stLinkInfo* pLink, IN const RouteLinkInfo* pRoutLinkInfo, IN const int32_t routeOpt, IN const int32_t mobilityOpt, IN const uint32_t timestampOpt, OUT vector<CandidateLink*>& vtCandidateInfo);	const uint32_t CheckStartDirectionMaching(IN const stLinkInfo* pLink, IN const RouteLinkInfo* pRoutLinkInfo, IN const int32_t routeOpt, IN const int32_t mobilityOpt, IN const uint32_t timestampOpt, OUT vector<CandidateLink*>& vtCandidateInfo);
	const uint32_t CheckStartDirectionMaching(IN const RequestRouteInfo* pReqInfo, IN const stLinkInfo* pLink, IN const RouteLinkInfo* pRoutLinkInfo, OUT vector<CandidateLink*>& vtCandidateInfo);
	const uint32_t CheckEndDirectionMaching(IN const RequestRouteInfo* pReqInfo, IN const stLinkInfo* pLink, IN const RouteLinkInfo* pRoutLinkInfo, OUT vector<CandidateLink*>& vtCandidateInfo);
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



