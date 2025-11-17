#pragma once

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>
#include <array>
#include <queue>
#include <algorithm>
#include <memory>
#include <stdio.h>
#include <stdexcept>
#include <time.h>
#include <cstring>

#include "types.h"
#include "../shp/shpio.h" 
#include "../tms/tms.h"

using namespace std;


#if !defined(_WIN32)
#include <string.h>
#include <limits.h>
#include <float.h>
#endif

#if !defined(_DEBUG)
#define USE_MULTIPROCESS
#endif
#if defined(USE_MULTIPROCESS)
#define PROCESS_NUM 8
#include <omp.h>
#else
#define PROCESS_NUM 8
#endif


#define USE_ROUTE_TABLE_LEVEL	7 // 7(1차선이상일반도로) 레벨보다 낮아지면 도로 확장이 어려워짐. 수도권 50 POI 수행 시간 : LV6/교통 == 약 13s/177s, LV7/교통 = 약 33s/244s
//#define __USE_TEMPLETE // 템플릿 사용

//#define TARGET_FOR_KAKAO_VX // 카카오VX제공

//#define USE_OPTIMAL_POINT_API // 최적 지점 API
#define USE_ROUTING_POINT_API // 경로 탐색 API
//#define USE_TMS_API // 특수목적 경로탐색 
//#define USE_PM_API // 보행/자전거/킥보드 전용 경로탐색

#if defined(TARGET_FOR_KAKAO_VX)
#	define USE_FOREST_DATA // 카카오숲길
#	define USE_MOUNTAIN_DATA // 산바운더리 데이터
#	define USE_PEDESTRIAN_DATA // 보행자/자전거 데이터
#elif defined(USE_OPTIMAL_POINT_API)
//#define USE_PEDESTRIAN_DATA // 보행자/자전거 데이터
#define USE_VEHICLE_DATA // 차량 데이터
#define USE_BUILDING_DATA // 건물 데이터
#define USE_COMPLEX_DATA // 단지 데이터
#elif defined(USE_ROUTING_POINT_API)
#	if defined(USE_TMS_API)
#	define USE_VEHICLE_DATA // 차량 데이터
#	elif defined(USE_PM_API)
#	define USE_PEDESTRIAN_DATA // 보행자/자전거 데이터
#	else
#	define USE_VEHICLE_DATA // 차량 데이터
#	define USE_P2P_DATA // P2P 데이터
//#	define USE_SAMSUNG_HEAVY // 삼성 중공업 데이터
#	endif
#endif


//#define USING_PED_BICYCLE_TYPE // 자전거 경로 사용(링크 속성중 자전거 속성 사용)

//#define USE_SHOW_ROUTE_SATATUS // 경로 탐색 진행 상태 모니터링

#define USE_ROUTE_USING_SERVER // 경로 서버 사용

#define IN
#define OUT

#ifndef NOT_USE
#	define NOT_USE 0xFFFFFFFF
#endif

#ifndef MAX_PATH
#	define MAX_PATH 260
#endif

#ifndef BYTE
#	define BYTE unsigned char
#endif

#define IR_PRECISION 1000.f
#define PI_PRECISION 3.14159265358979323846

#define SAFE_FREE(x)		{ if (x != nullptr) free(x); (x) = nullptr; }
#define SAFE_DELETE(x)		{ if (x != nullptr) delete (x); (x) = nullptr; }
#define SAFE_DELETE_ARR(x)	{ if (x != nullptr) delete [] (x); (x) = nullptr; }

#if !defined(_WIN32)
typedef struct tagRECT
{
    uint32_t    left;
    uint32_t    top;
    uint32_t    right;
    uint32_t    bottom;
} RECT;
#endif


#if defined(USE_P2P_DATA)
// 0.0.4 add ttl_id & change data file names
// 0.0.5 add to coordinate for inavi air navigation
// 0.0.6 add candidate route
// 0.0.6 merge to request route options to struct data <-- 미리 추가함 2025-02-25
// 0.0.6 change u-turn status behave action
// 0.0.6 Turn-left allows a minimum distance applied as 50m to the intersection from start point
// 0.0.6 add candidate request option
// 0.0.7 add to avoid continuous short turns
#define ENGINE_VERSION_MAJOR	0
#define ENGINE_VERSION_MINOR	0
#define ENGINE_VERSION_PATCH	7
#define ENGINE_VERSION_BUILD	0
#elif defined(USE_OPTIMAL_POINT_API)
// 1.0.6 fix road param setting, and bycicle -> bicicle
// 1.0.7 add optimal point angle attribute when data compile step, with data v1.0.3
// 1.0.8 file execute name change
// 1.0.9 change avoid road pass code type 3 -> 4
// 1.0.10 fix optimal polygon check compare with building and complex 
// 1.0.11 change max multi core max -> max - 1 
// 1.0.12 fix cjson print free after using result
// 1.0.13 add to multi optimal position function
// 1.0.14 change the minimum limit distance value when there are other near links from the matched link
// 1.0.15 fix entry type case switching code and empty building case.
#define ENGINE_VERSION_MAJOR	1
#define ENGINE_VERSION_MINOR	0
#define ENGINE_VERSION_PATCH	15
#define ENGINE_VERSION_BUILD	0
#elif defined(USE_ROUTING_POINT_API)
#	if defined(USE_FOREST_DATA)
// 0.0.5 add multi modal route (forest entrance + pedestrian)
// 0.0.6 add pedestrian bike option default avoid set
// 0.0.7 use default avoid set hiking
// 0.0.8 add multirouting function for kvx perfomance
// 0.0.9 fix link info vertex offset calculating
// 0.1.0 add course routing function
// 0.1.1 fix GetNearLinkDataByCourseId function - access nullptr
// 0.1.2 bicycle route cost tuning
// 0.1.3 using multi mesh for link manage & upgrade routing routine
// 0.1.4 course id match routing apply only using course option
// 0.1.5 course id option change route option recommended to shortest, and fix slop value when link dir nagative, population grade applyed
// 1.0.0 add io name info, and extend search bound forest/wal 50km, course 100km, bicycle 200km
// 1.0.1 fix to pedestrian route failed process
// 1.0.2 merge to request route options to struct data <-- 미리 추가함 2025-02-25
#define ENGINE_VERSION_MAJOR	1
#define ENGINE_VERSION_MINOR	0
#define ENGINE_VERSION_PATCH	1
#define ENGINE_VERSION_BUILD	0
#	elif defined(USE_PEDESTRIAN_DATA)
// 0.0.6 add charging link attribute
// 0.0.7 fix waypoint guide-type value 3->2
// 0.0.8 use ttl_id
// 0.0.9 add cost data table and upgrade using kvx version
// 0.0.10 check destination link type with current expanded link type and merge to request route options to struct data
// 0.0.11 fix cost file loading code and change some cost for pedestrian
// 0.0.12 add guide_type for start-via and start-end case
// 0.0.13 add bicycle type link match for avoid link
// 0.0.14 allow bicycle only stairs attribute when use bicycle mobility
// 0.0.15 cross walk cost decrease when using bicycle
// 0.0.16 change bike cost to increase path similarity with kakao/naver
#define ENGINE_VERSION_MAJOR	0
#define ENGINE_VERSION_MINOR	0
#define ENGINE_VERSION_PATCH	16
#define ENGINE_VERSION_BUILD	0
#	else // defined(USE_VEHICLE_DATA)
#		if defined(USE_TMS_API)
// 0.0.4 add junction option, fix pedestrian cost value
// 0.0.6 add charging link attribute
// 0.0.7 fix waypoint guide-type value 3->2
// 0.0.8 use ttl_id
// 0.0.9 add link isolate check and avoid matching (for tms)
// 0.0.10 update traffic data in link using subdata
// 0.0.11 update auto deviation value for tms
// 0.0.12 fix tsp ext algorithm for infinite loop
// 0.0.13 upgrade rdm made speed using min-max position boundary
// 0.0.14 upgrade log utils - add key and using multi log files
// 0.0.15 upgrade log utils, change log check by thread
// 0.0.16 merge to request route options to struct data <-- 미리 추가함 2025-02-25
// 0.1.0 propagation method change, 첫번쨰 지점 도착시 우선순위 큐의 휴리스틱을 확장 코스트로 변경하여 코스 외곡을 방지 & 먼거리 지점을 우선 타겟팅하여 휴리스특 적용
// 0.1.1 한계값의 다중 타입 값을 중복처리 하도록 적용
// 0.1.2 거리 분할 + 체류 시간 포함 적용
// 0.1.3 출/도착지 정보의 분할 입력 가능하도록 적용, add destinations field in request json
// 0.1.4 add to bestvrp api for or-tools
#define ENGINE_VERSION_MAJOR	0
#define ENGINE_VERSION_MINOR	1
#define ENGINE_VERSION_PATCH	4
#define ENGINE_VERSION_BUILD	0
#		else // #		if defined(USE_TMS_API)
// 0.0.6 add charging link attribute
// 0.0.7 fix waypoint guide-type value 3->2
// 0.0.8 use ttl_id
// 0.0.9 add link isolate check and avoid matching (for tms)
// 0.0.10 update traffic data in link using subdata
// 0.0.11 update auto deviation value for tms
// 0.0.12 fix tsp ext algorithm for infinite loop
// 0.0.13 upgrade rdm made speed using min-max position boundary
// 0.0.14 upgrade log utils - add key and using multi log files
// 0.0.15 upgrade log utils, change log check by thread
// 0.0.16 merge to request route options to struct data <-- 미리 추가함 2025-02-25
#define ENGINE_VERSION_MAJOR	0
#define ENGINE_VERSION_MINOR	0
#define ENGINE_VERSION_PATCH	15
#define ENGINE_VERSION_BUILD	0
#		endif // #		if defined(USE_TMS_API)
#	endif // defined(USE_VEHICLE_DATA)
#else // #elif defined(USE_ROUTING_POINT_API)
#define ENGINE_VERSION_MAJOR	1
#define ENGINE_VERSION_MINOR	0
#define ENGINE_VERSION_PATCH	0
#define ENGINE_VERSION_BUILD	0
#endif

#define USE_CJSON

#define GLOBAL_MESH_ID		0 // 전체 데이터 영역 관리 메시 ID, z-order 와 관련있음, 0 일경우 좌표 등을 매칭 시 우선 타겟팅 됨

enum NETWORKTYPE { LINK, NODE };
enum DIRECTION { BIDIRECTION, FORWARD, DIR_POSITIVE = FORWARD, REVERSE, DIR_NAGATIVE = REVERSE };

#ifdef SPoint
static const size_t SIZE_SPOINT = sizeof(SPoint);
#else
static const size_t SIZE_SPOINT = 16;
#endif

struct KeyID {
	union {
		uint64_t llid;
		struct {
			uint64_t nid : 32;
			uint64_t tile_id : 30;
			uint64_t dir : 2; // 방향성, 0:양방향, 1:정방향, 2:역방향, 3:통행불가
		};
		struct {
			uint64_t parents_id : 32;
			uint64_t current_id : 32;
		};
		struct {
			uint64_t nid : 32;
			uint64_t tile_id : 30;
			uint64_t type : 2; // 타입, 0:미정, 1:빌딩, 2:단지, 3:산바운더리
		}poly;
		struct {
			uint64_t nid : 32;
			uint64_t tile_id : 30;
			uint64_t type : 2; // 타입, 0:미정, 1:빌딩, 2:단지, 3:산바운더리
		}ent;
		struct
		{
			uint64_t dir : 2; // 0:없음, 1:정방향, 2:역방향
			uint64_t nid : 62;
		}trf; // traffic ks
	};

	int operator== (const KeyID rhs) const { return llid == rhs.llid; }
	int operator!= (const KeyID rhs) const { return llid != rhs.llid; }
	int operator< (const KeyID rhs) const { return llid < rhs.llid; }
};


struct stNameInfo {
	uint32_t name_id;
	//uint32_t name_size;
	//char* name;
	string name;
};


struct stMeshInfo {
	KeyID mesh_id;
	SBox mesh_box;
	SBox data_box;
#if defined(USE_FOREST_DATA)
#ifdef TEST_SPATIALINDEX
	void* flinkStorage; // IStorageManager
	void* flinkTree; // IStorageManager
#else
	vector<KeyID> nodes;
	vector<KeyID> links;
#endif
#endif
#if defined(USE_PEDESTRIAN_DATA)
#ifdef TEST_SPATIALINDEX
	void* wlinkStorage; // IStorageManager
	void* wlinkTree; // IStorageManager
#else
	vector<KeyID> wnodes;
	vector<KeyID> wlinks;
#endif
#endif
#if defined(USE_VEHICLE_DATA)
#ifdef TEST_SPATIALINDEX
	void* vlinkStorage; // IStorageManager
	void* vlinkTree; // IStorageManager
#else
	vector<KeyID> vnodes;
	vector<KeyID> vlinks;
#endif
#endif
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_FOREST_DATA)
#ifdef TEST_SPATIALINDEX
	void* buildingStorage; // IStorageManager
	void* buildingTree; // IStorageManager
	void* complexStorage; // IStorageManager
	void* complexTree; // IStorageManager
#else
	vector<KeyID> buildings;
	vector<KeyID> complexs;
#endif
#endif
	unordered_set<uint32_t> neighbors;

#if defined(USE_FOREST_DATA)
	set<KeyID> setFNodeDuplicateCheck;
	set<KeyID> setFLinkDuplicateCheck;
#endif
#if defined(USE_PEDESTRIAN_DATA)
	set<KeyID> setWNodeDuplicateCheck;
	set<KeyID> setWLinkDuplicateCheck;
#endif
#if defined(USE_VEHICLE_DATA)
	set<KeyID> setVNodeDuplicateCheck;
	set<KeyID> setVLinkDuplicateCheck;
#endif
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
	set<KeyID> setBldDuplicateCheck;
	set<KeyID> setCpxDuplicateCheck;
#endif

	stMeshInfo() {
		mesh_id.llid = 0;
		memset(&mesh_box, 0x00, sizeof(mesh_box));
		memset(&data_box, 0x00, sizeof(data_box));

#ifdef TEST_SPATIALINDEX
#if defined(USE_FOREST_DATA)
		flinkStorage = nullptr;
		flinkTree = nullptr;
#endif
#if defined(USE_PEDESTRIAN_DATA)
		wlinkStorage = nullptr;
		wlinkTree = nullptr;
#endif
#if defined(USE_VEHICLE_DATA)
		vlinkStorage = nullptr;
		vlinkTree = nullptr;
#endif
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_FOREST_DATA)
		buildingStorage = nullptr;
		buildingTree = nullptr;
		complexStorage = nullptr;
		complexTree = nullptr;
#endif
#endif // #ifdef TEST_SPATIALINDEX
	}

	~stMeshInfo() {
#if defined(USE_FOREST_DATA)
#ifdef TEST_SPATIALINDEX
		if (flinkStorage) { SAFE_DELETE(flinkStorage); }
		if (flinkTree) { SAFE_DELETE(flinkTree); }
#else
		if (!nodes.empty()) { nodes.clear(); vector<KeyID>().swap(nodes); }
		if (!links.empty()) { links.clear(); vector<KeyID>().swap(links); }
#endif
#endif
#if defined(USE_PEDESTRIAN_DATA)
#ifdef TEST_SPATIALINDEX
		if (wlinkStorage) { SAFE_DELETE(wlinkStorage); }
		if (wlinkTree) { SAFE_DELETE(wlinkTree); }
#else
		if (!wnodes.empty()) { wnodes.clear(); vector<KeyID>().swap(wnodes); }
		if (!wlinks.empty()) { wlinks.clear(); vector<KeyID>().swap(wlinks); }
#endif
#endif
#if defined(USE_VEHICLE_DATA)
#ifdef TEST_SPATIALINDEX
		if (vlinkStorage) { SAFE_DELETE(vlinkStorage); }
		if (vlinkTree) { SAFE_DELETE(vlinkTree); }
#else
		if (!vnodes.empty()) { vnodes.clear(); vector<KeyID>().swap(vnodes); }
		if (!vlinks.empty()) { vlinks.clear(); vector<KeyID>().swap(vlinks); }
#endif
#endif
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
#ifdef TEST_SPATIALINDEX
		if (buildingStorage) { SAFE_DELETE(buildingStorage); }
		if (buildingTree) { SAFE_DELETE(buildingTree); }
		if (complexStorage) { SAFE_DELETE(complexStorage); }
		if (complexTree) { SAFE_DELETE(complexTree); }
#else
		if (!buildings.empty()) { buildings.clear(); vector<KeyID>().swap(buildings); }
		if (!complexs.empty()) { complexs.clear(); vector<KeyID>().swap(complexs); }
#endif
#endif
		if (!neighbors.empty()) { neighbors.clear(); unordered_set<uint32_t>().swap(neighbors); }

#if defined(USE_FOREST_DATA)
		if (!setFNodeDuplicateCheck.empty()) { setFNodeDuplicateCheck.clear(); set<KeyID>().swap(setFNodeDuplicateCheck); }
		if (!setFLinkDuplicateCheck.empty()) { setFLinkDuplicateCheck.clear(); set<KeyID>().swap(setFLinkDuplicateCheck); }
#endif
#if defined(USE_PEDESTRIAN_DATA)
		if (!setWNodeDuplicateCheck.empty()) { setWNodeDuplicateCheck.clear(); set<KeyID>().swap(setWNodeDuplicateCheck); }
		if (!setWLinkDuplicateCheck.empty()) { setWLinkDuplicateCheck.clear(); set<KeyID>().swap(setWLinkDuplicateCheck); }
#endif
#if defined(USE_VEHICLE_DATA)
		if (!setVNodeDuplicateCheck.empty()) { setVNodeDuplicateCheck.clear(); set<KeyID>().swap(setVNodeDuplicateCheck); }
		if (!setVLinkDuplicateCheck.empty()) { setVLinkDuplicateCheck.clear(); set<KeyID>().swap(setVLinkDuplicateCheck); }
#endif
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
		if (!setBldDuplicateCheck.empty()) { setBldDuplicateCheck.clear(); set<KeyID>().swap(setBldDuplicateCheck); }
		if (!setCpxDuplicateCheck.empty()) { setCpxDuplicateCheck.clear(); set<KeyID>().swap(setCpxDuplicateCheck); }
#endif
	}
};

typedef struct _tagstNodeBaseInfo{
	uint32_t node_type : 3; // 0:미정의, 1:숲길, 2:보행자/자전거, 3:차량
	uint32_t point_type : 4; // 노드 타입, 1:교차점, 2:단점, 3:더미점, 4:구획변경점, 5:속성변화점, 6:지하철진출입, 7:철도진출입, 8:지하도진출입, 9:지하상가진출입, 10:건물진출입
	uint32_t connnode_count : 4; // 접속 노드 수, MAX:15
	uint32_t shared : 21;
}stNodeBaseInfo;

typedef struct _tagstNodeTrekkingInfo {
	uint32_t dummy_type : 11; // node_base 공유 공간
	uint32_t entrance : 1; // 숲길 입구점 보유 여부, 0:미정의, 1:보유(보유시, nodeInfo의 edgenode_id에 보행자길 노드ID 공유)
	uint32_t z_value : 12; // 고도 정보 ~(4095)
	uint32_t ent_info : 1; // 입구점 명칭, 0:없음, 1:있음
	uint32_t mnt_info : 1; // 봉우리 명칭, 0:없음, 1:있음
	uint32_t trk_reserved : 6; // reserved
}stNodeTrekkingInfo;

typedef struct _tagstNodePedestrianInfo {
	uint32_t dummy_type : 11; // node_base 공유 공간
	uint32_t fct_phase : 2; // 시설물 위상, 0:미정의, 1:지하, 2:지상, 3:지상위
	uint32_t gate_phase : 2; // 입출구 위상, 0:미정의, 1:지하, 2:지상, 3:지상위
	uint32_t name_info : 1; // 노드 명칭, 0:없음, 1:있음, 실제 표지판을 기준으로 교차점 노드에 입력
	uint32_t fct_info : 1; // 시설물 상세 명칭, 0:없음, 1:있음
	uint32_t gate_info : 1; // 입출구 명칭, 0:없음, 1:있음
	uint32_t ped_reserved : 14; // reserved
}stNodePedestrianInfo;

struct stNodeInfo {
	KeyID node_id;
	KeyID edgenode_id; //엣지노드가 있는 경우는 연결노드가 1개일 경우 밖에 없을 것 같아, connnodes와 공유해도 될듯(확인필요), 숲길에서는 보행자 입구점 노드ID를 공유
	SPoint coord;
	union {
		KeyID connnodes[8]; // 연결된 링크 ID (uint64_t)
		uint32_t connnodes_ex[16]; // 기본 8개 이상의 연결 링크를 가지는 경우 -->kakao-vx
	};
	uint16_t conn_attr[8];
	uint32_t name_idx; // 명칭 인덱스
	union {
		uint32_t sub_info;
		stNodeBaseInfo base;
		stNodeTrekkingInfo trk;
		stNodePedestrianInfo ped;
	};

	stNodeInfo() {
		node_id.llid = 0;
		edgenode_id.llid = 0;
		memset(&coord, 0x00, sizeof(coord));
		memset(&connnodes, 0x00, sizeof(connnodes));
		sub_info = 0;
	}
};

typedef struct _tagstLinkBaseInfo {
	uint64_t link_type : 3; // 0:미정의, 1:숲길, 2:보행자/자전거, 3:차량
	uint64_t shared : 43; // 타입별 속성 공유공간
	uint64_t snode_ang : 9; // 360
	uint64_t enode_ang : 9; // 360
}stLinkBaseInfo;

typedef struct _tagstLinkPedestrianInfo {
	uint64_t dummy_type : 3; // link_type 공유 공간
	uint64_t bicycle_type : 2; // 자전거도로 타입, 1:자전거전용, 2:보행자/차량겸용 자전거도로, 3:보행도로
	uint64_t walk_type : 3; //보행자도로 타입, 1:복선도로, 2:차량겸용도로, 3:자전거전용도로, 4:보행전용도로, 5:가상보행도로
	uint64_t facility_type : 4; // 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
	uint64_t gate_type : 4; // 진입로 타입, 0:미정의, 1:경사로, 2:계단, 3:에스컬레이터, 4:계단/에스컬레이터, 5:엘리베이터, 6:단순연결로, 7:횡단보도, 8:무빙워크, 9:징검다리, 10:의사횡단
	uint64_t lane_count : 6; // 차선수, 63
	uint64_t side_walk : 2; // 인도(보도) 여부, 0:미조사, 1:있음, 2:없음
	uint64_t walk_charge : 2; // 보행자도로 유료 여부, 0:무료, 1:유료
	uint64_t bicycle_control : 2; // 자전거도로 규제 코드, 0:양방향, 1:정방향, 2:역방향, 3:통행불가
	uint64_t ped_reserved : 18; // reserved
	uint64_t shared : 18; // link_ang 공유 공간
}stLinkPedestrianInfo;

typedef struct _tagstLinkTrekkingInfo {
	//uint64_t dummy_type : 3; // link_type 공유 공간
	//uint64_t course_type : 3; // 0:미정의, 1 : 등산로, 2 : 둘레길, 3 : 자전거길, 4 : 종주코스
	//uint64_t road_info : 11; // 노면정보 코드, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드
	//uint64_t diff : 6; // 난도 0~50, (숫자가 클수록 어려움)
	//uint64_t tre_reserved : 5; // reserved
	//uint64_t shared : 32; // link_ang 공유 공간
	
	uint64_t dummy_type : 3; // link_type 공유 공간
	uint64_t course_type : 3; // 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
	uint64_t road_info : 12; // 노면정보, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드, 12:철구조물
	uint64_t dir_cd : 2; // 방면방향정보, 0:미정의, 1:정, 2:역
	uint64_t diff : 3; // 법정탐방로 1비트를 위해 4->3비트 변경 (diff는 아직 값이 없기에), 2024-02-27 <== 난도 0~15, (숫자가 클수록 어려움)
	uint64_t slop : 8; // 경사도 +/- 100
	uint64_t fw_tm : 10;// 0~1023 정방향 이동 시간 (초)
	uint64_t bw_tm : 10;// 0~1023 역방향 이동 시간 (초)
	uint64_t pop_grade : 4; // 인기도 등급, 0-10 낮을수록 인기도 높음, 인기도: 0-4095
	uint64_t legal : 1; // 법정탐방로 여부, 0:비법정, 1:법정
	uint64_t trk_reserved : 8; // reserved
}stLinkTrekkingInfo;

typedef struct _tagstLinkVehicleInfo {
	uint64_t dummy_type : 3; // link_type 공유 공간
	uint64_t road_type : 4; // 도로종별, 1:고속도록, 2:도시고속도로, 3:일반국도, 4:페리항로, 5:지방도, 6:일반도로, 7:소로, 8:골목길, 9:시장길
	uint64_t lane_cnt : 4; // 차선수, 15
	uint64_t link_type : 4; // 링크종별, 0:입구점, 1:본선비분리, 2:본선분리, 3:연결로, 4:교차로, 5:램프, 6:로터리, 7:휴계소(SA), 8:유턴
	uint64_t link_dtype : 2; // 링크세부종별(dk3), 0:미정의, 1:고가도로,지하차도 옆길, 2:비포장도로, 3:단지내도로
	uint64_t level : 4; // 경로레벨, 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상, 4:일반도로6차선이상, 5:일반도로4차선이상, 6:일반도로2차선이상, 7:일반도로1차선이상, 8:SS도로, 9:GSS도로/단지내도로/통행금지도로/비포장도로 <-- SS(교행이 어려운도로), GSS(도로폭이 매우 좁아 경유로 사용되지 말아야하고, 목적지로만 사용하는 도로)
	uint64_t pass_code : 4;// 규제코드, 1:통행가능, 2:통행불가, 3:공사구간, 4:공사계획구간, 5:일방통행_정, 6:일방통행_역 // 5,6은 control 코드 값임.
	uint64_t car_only : 1; // 자동차전용도로, 0:없음, 1:있음
	uint64_t charge : 1; // 유료도로, 0:없음, 1:있음
	uint64_t tunnel : 1; // 터널, 0:없음, 1:있음
	uint64_t under_pass : 1; // 지하차도, 0:없음, 1:있음
	uint64_t safe_zone : 3; // 어린이보호구역, 노인보호구역, 0:없음, 1:어린이보호구역, 2:노인보호구역, 3:마을주민보호구역, 4:장애인보호구역
	// 32 bit
#if defined(USE_P2P_DATA)
	uint64_t speed_f : 8; // 정방향 제한속도 0~255
	uint64_t speed_b : 8; // 역방향 제한속도 0~255
	uint64_t rtt_f : 1; // 정방향 실시간 교통정보, 0:미사용, 1:사용
	uint64_t rtt_b : 1; // 역방향 실시간 교통정보, 0:미사용, 1:사용
	uint64_t weight : 6; // 중량 제한 T(톤) 0~63
	uint64_t height : 3; // 높이 제한 M(미터) 0~7
	uint64_t bridge : 1; // 교량, 0:없음, 1:있음
	uint64_t over_pass : 1; // 고가도로, 0:없음, 1:있음
	uint64_t hd_flag : 2; // HD 링크 매핑, 0:없음, 1:전체, 2:부분
	uint64_t veh_reserved : 1; // reserved 
	//p2p에서는 angle 공유하지 말자
#else
	//uint32_t RoadNameIDX; // 도로명 인덱스
	uint64_t weight : 1; // 중량 제한 여부,, 0:없음, 1:있음 - T(톤) 단위 
	uint64_t height : 1; // 높이 제한 여부, , 0:없음, 1:있음 - M(미터) 단위 
	uint64_t bridge : 1; // 교량 유무, 0:없음, 1:있음
	uint64_t over_pass : 1; // 고가도로 유무, 0:없음, 1:있음
	//uint32_t BusOnly : 1; // 버스전용차로 유무, 0:없음, 1~19
	//uint32_t ExpressMode; // 고속모드 유무, 0:없음, 1:있음
	//uint32_t ExpressModeSub; // 고속모드부가 유무, 0:없음, 1:있음
	//uint32_t LaneInfo; // 차선정보 유무, 0:없음, 1:있음
	//uint32_t DirInfo; // 방면정보 유무, 0:없음, 1:있음
	//uint32_t IndicatorLane; // 차로유도차선정보 유무, 0:없음, 1:있음
	//uint32_t IndicatorDir; // 차로유도방면정보 유무, 0:없음, 1:있음
	uint64_t toll : 1; // 톨게이트 유무, 0:없음, 1:있음
	//uint32_t JunctionInfo; // 확대도 유무, 0:없음, 1:있음
	//uint32_t Tpeg; // TPEG 유무(YTN기준), 0:없음, 1:있음
	uint64_t restriction : 3; // 진입제한 유무, 0:없음, 1:사람, 2:이륜, 3:화물, 4:택시, 5:입주민 외
	uint64_t water_safe_zone : 1; // 상수원보호구역

	uint64_t veh_reserved : 5; // 
	uint64_t shared : 18; // link_ang 공유 공간
#endif
	// shared 대신 링크별 통행코드를(메모리 사이즈 절약차원) 넣으려했으나 그냥 노드의 통행코드 사용하기로 함(2022-12-12)
	//uint64_t snode_pass : 16; // 0:통행가능(유턴불가), 1:통행가능(유턴가능), 2:통행불가 // 8자리 통행코드를 각 코드 위치별로 2비트씩 16비트에 나눠 담는다.
	//uint64_t enode_pass : 16;
}stLinkVehicleInfo;

typedef struct _tagstLinkVehicleInfoExt {
	uint64_t snode_ang : 16; // 360
	uint64_t enode_ang : 16; // 360
	uint64_t spd_type_p : 2; // 트래픽 타입
	uint64_t spd_type_n : 2; // 트래픽 타입
	uint64_t spd_p : 8; // 정방향속도 // RDM 생성시 트래픽맵에서 읽어오면 너무 느려서 직접 속도 설정
	uint64_t spd_n : 8; // 역방향속도 // RDM 생성시 트래픽맵에서 읽어오면 너무 느려서 직접 속도 설정
	uint64_t veh_ext_reserved : 14;
}stLinkVehicleInfoExt;

typedef struct _tagstLinkTrekkingInfoExt {
	uint64_t course_type : 3; // 코스타입, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
	uint64_t course_cd : 20; // 코스 CD
	uint64_t course_cnt : 5; // 코스 갯수(기본+중용), MAX 18
	uint64_t group_id : 12; // 그룹 ID, 동일 숲길 링크 ID
	uint64_t trk_ext_reserved : 24; // 0xFFFFFF = 16777215, 우선 메쉬로 쓰자
}stLinkTrekkingInfoExt;

typedef struct _tagstCourseInfo {
	union {
		uint32_t course_value;
		struct {
			uint32_t course_type : 3; // 코스타입, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
			uint32_t course_cd : 20; // 코스 CD
			uint32_t course_reserved : 9;
		};
	};
}stCourseInfo;


typedef struct _tagLinkSubInfo
{
	union
	{
		uint64_t sub_info;
		stLinkBaseInfo base;
		stLinkPedestrianInfo ped; // 보행로
		stLinkTrekkingInfo trk; // 숲길
		stLinkVehicleInfo veh; // 차로
	};

	_tagLinkSubInfo() {
		sub_info = 0;
	}

	void operator =(const uint64_t& rhs) { sub_info = rhs; }
	bool operator <=(const uint64_t& rhs) { return sub_info <= rhs; }
	bool operator >=(const uint64_t& rhs) { return sub_info >= rhs; }
	bool operator ==(const uint64_t& rhs) { return sub_info == rhs; }
	bool operator !=(const uint64_t& rhs) { return sub_info != rhs; }
}stLinkSubInfo;


struct stLinkInfo {
	KeyID link_id;
	KeyID snode_id;
	KeyID enode_id;
	double length;
	uint32_t name_idx;
	union {
		uint64_t sub_info;
		stLinkBaseInfo base;
		stLinkPedestrianInfo ped;
		stLinkTrekkingInfo trk;
		stLinkVehicleInfo veh;
	};
#if defined(USE_P2P_DATA) || defined(USE_MOUNTAIN_DATA) || defined(USE_TMS_API)
	union {
		uint64_t sub_ext; // 확장 정보 // 현재는 고유 ID (mesh(6) + snode(6) + enode(6)
		stLinkTrekkingInfoExt trk_ext; // 트래킹 확장
		stLinkVehicleInfoExt veh_ext; // 차량 확장
	};
#endif
	//std::vector<SPoint> vtPts;
	uint32_t cntVtx; // 폴리곤 갯수
	SPoint* pVtx; // 폴리곤 버텍스

	stLinkInfo() {
		link_id.llid = 0;
		snode_id.llid = 0;
		enode_id.llid = 0;
		length = 0;
		sub_info = 0;
#if defined(USE_P2P_DATA) || defined(USE_MOUNTAIN_DATA) || defined(USE_TMS_API)
		sub_ext = 0;
#	if defined(USE_TMS_API)
		veh_ext.spd_n = veh_ext.spd_p = 0xFF; // 
#	endif
#endif
		cntVtx = 0;
		pVtx = nullptr;
	}

	~stLinkInfo() {
		SAFE_DELETE_ARR(pVtx);
	}

	const uint32_t getVertexCount(void) const {
		return cntVtx;
	}

	const SPoint* getVertex(void) const {
		return pVtx;
	}

	const SPoint* getVertex(IN const uint32_t idxData) const {
		if (cntVtx <= 0 || cntVtx <= idxData) {
			return nullptr;
		}

		return &pVtx[idxData];
	}

	const double getVertexX(IN const uint32_t idxData) const {
		if (cntVtx <= 0 || cntVtx <= idxData) {
			return 0.f;
		}

		return pVtx[idxData].x;
	}

	const double getVertexY(IN const uint32_t idxData) const {
		if (cntVtx <= 0 || cntVtx <= idxData) {
			return 0.f;
		}

		return pVtx[idxData].y;
	}

	void setVertex(IN const SPoint* pData = nullptr, IN const uint32_t cntData = 0) {
		cntVtx = cntData;
		SAFE_DELETE_ARR(pVtx);

		if (pData != nullptr && cntData > 0) {
			pVtx = new SPoint[cntData];
			memcpy(pVtx, pData, cntData * SIZE_SPOINT);
		}
	}
	
	size_t readVertex(IN FILE* fp, IN const uint32_t cntData = 0) {
		size_t retRead = 0;

		if (fp == nullptr || cntData <= 0) {
			return retRead;
		}

		cntVtx = cntData;
		SAFE_DELETE_ARR(pVtx);
		pVtx = new SPoint[cntData];
		if (fread(pVtx, cntData * SIZE_SPOINT, 1, fp)) {
			retRead = cntData * SIZE_SPOINT;
		}

		return retRead;
	}

	size_t writeVertex(IN FILE* fp) {
		size_t retWrite = 0;

		if (fp == nullptr || pVtx == nullptr || cntVtx <= 0) {
			return retWrite;
		}

		if (fwrite(pVtx, cntVtx * SIZE_SPOINT, 1, fp)) {
			retWrite = cntVtx * SIZE_SPOINT;
		}

		return retWrite;
	}

	void reverseVertex(IN const uint32_t idx = 0, IN uint32_t cnt = 0)
	{
		if (pVtx == nullptr || cntVtx <= 0 || cnt == 1 || cntVtx <= idx || cntVtx <= cnt) {
			return;
		}

		if (idx == 0 && cnt == 0) {
			cnt = cntVtx;
		}

		SPoint ptTmp;
		int next = idx + cnt - 1;
		for (int ii = 0; ii < cnt / 2; ii++, --next) {
			memcpy(&ptTmp, &pVtx[next], SIZE_SPOINT);
			memcpy(&pVtx[next], &pVtx[idx + ii], SIZE_SPOINT);
			memcpy(&pVtx[idx + ii], &ptTmp, SIZE_SPOINT);
		}
	}

	uint32_t mergeVertex(IN const stLinkInfo* pLink)
	{
		if (pLink == nullptr || pLink->getVertex() == nullptr || pLink->getVertexCount() <= 0) {
			return cntVtx;
		}

		const SPoint* pData = pLink->getVertex();
		const uint32_t rhsCnt = pLink->getVertexCount() - 1; // lhs 의 처음 or 마지막과 동일해야 함
		const uint32_t lhsCnt = cntVtx;
		const uint32_t totCnt = lhsCnt + rhsCnt;

		if (lhsCnt <= 0) // new
		{
			SAFE_DELETE_ARR(pVtx);
			pVtx = new SPoint[rhsCnt + 1];
			memcpy(pVtx, pData, (rhsCnt + 1) * SIZE_SPOINT);
		}
		// head with head // <--- ---> to <--- <---
		else if (pVtx[0].x == pData[0].x && pVtx[0].y == pData[0].y)
		{
			SPoint* pNewVtx = new SPoint[totCnt];

			memcpy(pNewVtx, pData + 1, rhsCnt * SIZE_SPOINT);
			memcpy(pNewVtx + rhsCnt, pVtx, lhsCnt * SIZE_SPOINT);
			//reverse(pNewVtx + lhsCnt, pNewVtx + totCnt);

			SAFE_DELETE_ARR(pVtx);
			pVtx = pNewVtx;
			cntVtx = totCnt;

			reverseVertex(0, rhsCnt);
		}
		// head with tail // <--- <--- to <--- <---
		else if (pVtx[0].x == pData[rhsCnt].x && pVtx[0].y == pData[rhsCnt].y)
		{
			SPoint* pNewVtx = new SPoint[totCnt];

			memcpy(pNewVtx, pData, rhsCnt * SIZE_SPOINT);
			memcpy(pNewVtx + rhsCnt, pVtx, lhsCnt * SIZE_SPOINT);

			SAFE_DELETE_ARR(pVtx);
			pVtx = pNewVtx;
			cntVtx = totCnt;
		}
		// tail with head // ---> ---> to ---> --->
		else if (pVtx[lhsCnt - 1].x == pData[0].x && pVtx[lhsCnt - 1].y == pData[0].y)
		{
			SPoint* pNewVtx = new SPoint[totCnt];

			memcpy(pNewVtx, pVtx, lhsCnt * SIZE_SPOINT);
			memcpy(pNewVtx + lhsCnt, pData + 1, rhsCnt* SIZE_SPOINT);

			SAFE_DELETE_ARR(pVtx);
			pVtx = pNewVtx;
			cntVtx = totCnt;
		}
		// tail with tail // ---> <--- to ---> --->
		else if (pVtx[lhsCnt - 1].x == pData[rhsCnt].x && pVtx[lhsCnt - 1].y == pData[rhsCnt].y)
		{
			SPoint* pNewVtx = new SPoint[totCnt];

			memcpy(pNewVtx, pVtx, lhsCnt * SIZE_SPOINT);
			//reverse(pNewVtx, pNewVtx + rhsCnt);
			memcpy(pNewVtx + lhsCnt, pData, rhsCnt * SIZE_SPOINT);

			SAFE_DELETE_ARR(pVtx);
			pVtx = pNewVtx;
			cntVtx = totCnt;

			reverseVertex(lhsCnt , rhsCnt);
		}
		else
		{
			SPoint* pNewVtx = new SPoint[totCnt];

			memcpy(pNewVtx, pVtx, lhsCnt * SIZE_SPOINT);
			memcpy(pNewVtx + lhsCnt, pData, rhsCnt * SIZE_SPOINT);

			SAFE_DELETE_ARR(pVtx);
			pVtx = pNewVtx;
			cntVtx = totCnt;
		}

		return cntVtx;
	}
};


struct stExtendInfo
{
	KeyID keyId;
	uint8_t cntData; // max 255
	vector<uint8_t> vtType; // 확장 타입
	vector<double> vtValue; // 확장 데이터

	size_t read(IN FILE* fp)
	{
		size_t retSize = 0;

		if (fp == nullptr) {
			return retSize;
		}

		// key_id
		if (fread(&keyId.llid, sizeof(keyId.llid), 1, fp) != 1) {
			return 0;			
		}
		retSize += sizeof(keyId.llid);

		// data count
		if (fread(&cntData, sizeof(cntData), 1, fp) != 1) {
			return 0;
		}
		retSize += sizeof(cntData);

		// data value
		if (cntData > 0) {
			vtType.resize(cntData);
			vtValue.resize(cntData);

			// data type
			if (fread(&vtType.front(), sizeof(uint8_t) * cntData, 1, fp) != 1) {
				return 0;
			}
			retSize += sizeof(uint8_t) * cntData;

			// data value
			if (fread(&vtValue.front(), sizeof(double) * cntData, 1, fp) != 1) {
				return 0;
			}
			retSize += sizeof(double) * cntData;
		}
		
		return retSize;
	} // size_t read(IN FILE* fp)

	size_t write(IN FILE* fp)
	{
		size_t retSize = 0;

		if (fp == nullptr) {
			return retSize;
		}

		// key_id
		if (fwrite(&keyId.llid, sizeof(keyId.llid), 1, fp) != 1) {
			return 0;
		}
		retSize += sizeof(keyId.llid);

		cntData = vtType.size();

		// cnt value
		if (fwrite(&cntData, sizeof(cntData), 1, fp) != 1) {
			return 0;
		}
		retSize += sizeof(cntData);

		// data type
		if (fwrite(&vtType.front(), sizeof(int8_t) * cntData, 1, fp) != 1) {
			return 0;
		}
		retSize += sizeof(int8_t) * cntData;

		// data value
		if (fwrite(&vtValue.front(), sizeof(double) * cntData, 1, fp) != 1) {
			return 0;
		}
		retSize += sizeof(double) * cntData;

		return retSize;
	} // size_t write(IN FILE* fp)
};


struct stEntranceInfo {
	union {
		struct {
			uint32_t poly_type : 2; //  폴리곤 타입, 0:미지정(TYPE_ENT_NONE), 1:빌딩(TYPE_POLYGON_BUILDING), 2:단지(TYPE_POLYGON_COMPLEX), 3:산바운더리(TYPE_POLYGON_MOUNTAIN)
			uint32_t ent_code : 4; // 입구점 타입 // 1:차량 입구점, 2:택시 승하차 지점(건물), 3:택시 승하차 지점(건물군), 4:택배 차량 하차 거점, 5:보행자 입구점, 	6:배달 하차점(차량, 오토바이), 7:배달 하차점(자전거, 도보), 8:숲길 입구점
			uint32_t angle : 9; // 최근접 링크의 진출 각도
			uint32_t reserved : 17;
			double x;
			double y;
			uint64_t reserved1; 
			uint64_t reserved2;
		};
		struct {
			uint32_t poly_type : 2; // 폴리곤 타입, 0:미지정(TYPE_ENT_NONE), 1:빌딩(TYPE_POLYGON_BUILDING), 2:단지(TYPE_POLYGON_COMPLEX), 3:산바운더리(TYPE_POLYGON_MOUNTAIN)
			uint32_t ent_code : 4; // 입구점 타입 // 1:차량 입구점, 2:택시 승하차 지점(건물), 3:택시 승하차 지점(건물군), 4:택배 차량 하차 거점, 5:보행자 입구점, 	6:배달 하차점(차량, 오토바이), 7:배달 하차점(자전거, 도보), 8:숲길 입구점
			uint32_t ent_id : 26; // 입구점 ID // max(67108863)
			double x;
			double y;
			uint64_t fnode_id; // 숲길 노드 ID
			uint64_t wnode_id; // 보행자 노드 ID
			//uint32_t name_idx; // 입구점 명칭 IDX
			//uint32_t mnt_code; // 산코드
		}mnt;
	};
};


struct stTrafficInfoKS 
{
	uint32_t ks_id;
	uint32_t timestamp; // 최종 업데이트된 시각
	uint8_t speed; // 도로 주행 속도 0~255, 255일 경우는 통제, 주행불가

	// link_id = mesh + link + dir
	unordered_set<uint64_t> setLinks; // ks 링크에 매칭되는 link_id -> 나중에는 메모리 관리를 위해 포인터로 관리하자!!
	
	stTrafficInfoKS() {
		ks_id = 0;
		timestamp = 0;
		speed = 255;
	}
};


struct stTrafficInfoTTL
{
	uint32_t timestamp; // 최종 업데이트된 시각
	uint8_t speed; // 도로 주행 속도 0~255, 255일 경우는 통제, 주행불가
	uint32_t ttl_nid; // ttl id
	uint32_t link_nid; // ttl 링크에 매칭되는 link id
	uint8_t ttl_dir : 4; // ttl 방향 (정/역)
	uint8_t link_dir : 4; // ttl 링크에 매칭되는 link dir (양방향/정/역)

	stTrafficInfoTTL() {
		timestamp = 0;
		speed = 0xFF;
		ttl_nid = 0;
		link_nid = 0;
		ttl_dir = 0;
		link_dir = 0;
	}
};


struct stPolygonInfo {
public:
	KeyID poly_id;
	SBox data_box;
	union {
		uint64_t sub_info;
		struct {
			uint64_t code : 5; // 건물 종별 코드, ~30
			uint64_t height : 10; // 건물 높이, ~1000(m)
			uint64_t apt_type : 3; // 건물 동 번호 타입 0~7, 0:인덱스, 1:정수('101'동), 2:영어('A'동), 3:한글('가'동)
			uint64_t apt_val : 14; // 건물 동 번호 인덱스 ~16383
			//uint64_t name : 32; // 명칭 사전 인덱스
			uint64_t sd_sgg_code : 8; // 시도/시군구 코드 MAX 65535
			uint64_t reserved : 24;
		}bld;
		struct {
			uint64_t code : 4; // 건물 종별 코드, ~9
			uint64_t sd_sgg_code : 8; // 시도/시군구 코드 MAX 65535
			uint64_t name_idx : 32; // 명칭 인덱스
			uint64_t reserved : 20;
		}cpx;
	};

private:
	std::array<uint32_t, POLYGON_DATA_ATTR_MAX> arrAttrCnt = { 0, };

	uint16_t* pPts; // 폴리곤 파트 인덱스
	SPoint* pVtx; // 폴리곤 버텍스
	stEntranceInfo* pEnt; // 입구점 정보
	KeyID* pLnk; // 단지내 도로 ID
	uint32_t* pMsh; // 중첩 메쉬



public:
	//std::vector<uint32_t> vtPts; // 폴리곤 파트 인덱스
	//std::vector<SPoint> vtVtx; // 폴리곤 버텍스
	//std::vector<stEntranceInfo> vtEnt; // 입구점 정보
	//std::vector<KeyID> vtLink; // 단지내 도로 ID
	//std::vector<uint32_t> vtJoinedMesh; // 중첩 메쉬

	stPolygonInfo() {
		poly_id.llid = 0;
		memset(&data_box, 0x00, sizeof(data_box));
		sub_info = 0;

		pPts = nullptr;
		pVtx = nullptr;
		pEnt = nullptr;
		pLnk = nullptr;
		pMsh = nullptr;

		arrAttrCnt.fill(0);
	}

	~stPolygonInfo() {
		SAFE_DELETE_ARR(pPts);
		SAFE_DELETE_ARR(pVtx);
		SAFE_DELETE_ARR(pEnt);
		SAFE_DELETE_ARR(pLnk);
		SAFE_DELETE_ARR(pMsh);
	}

	const uint32_t getAttributeCount(IN const TYPE_POLYGON_DATA_ATTR typeAttr) const {
		int ret = 0;

		if (typeAttr < POLYGON_DATA_ATTR_MAX) {
			ret = arrAttrCnt[typeAttr];
		}

		return ret;
	}

	const uint16_t* getAttributeParts() const {
		return pPts;
	}

	const SPoint* getAttributeVertex() const {
		return pVtx;
	}

	const stEntranceInfo* getAttributeEntrance() const {
		return pEnt;
	}

	const KeyID* getAttributeLink() const {
		return pLnk;
	}

	const uint32_t* getAttributeMesh() const {
		return pMsh;
	}

	void setAttribute(IN const uint32_t typeAttr, IN const void* pData, IN const uint32_t cntData = 0) {
		if (typeAttr >= POLYGON_DATA_ATTR_MAX || pData == nullptr || cntData <= 0) {
			return;
		}

		// 초기화
		arrAttrCnt[typeAttr] = cntData;

		switch (typeAttr) {
		case TYPE_POLYGON_DATA_ATTR_PART:
		{
			SAFE_DELETE_ARR(pPts);
			pPts = new uint16_t[cntData];
			memcpy(pPts, pData, cntData * sizeof(uint16_t));
		}
		break;

		case TYPE_POLYGON_DATA_ATTR_VTX:
		{
			SAFE_DELETE_ARR(pVtx);
			pVtx = new SPoint[cntData];
			memcpy(pVtx, pData, cntData * SIZE_SPOINT);
		}
		break;

		case TYPE_POLYGON_DATA_ATTR_ENT:
		{
			SAFE_DELETE_ARR(pEnt);
			pEnt = new stEntranceInfo[cntData];
			memcpy(pEnt, pData, cntData * sizeof(stEntranceInfo));
		}
		break;

		case TYPE_POLYGON_DATA_ATTR_LINK:
		{
			SAFE_DELETE_ARR(pLnk);
			pLnk = new KeyID[cntData];
			memcpy(pLnk, pData, cntData * sizeof(KeyID));
		}
		break;

		case TYPE_POLYGON_DATA_ATTR_MESH:
		{
			SAFE_DELETE_ARR(pMsh);
			pMsh = new uint32_t[cntData];
			memcpy(pMsh, pData, cntData * sizeof(uint32_t));
		}
		break;


		default: {

		}
		break;
		} // switch		
	}

	size_t readAttribute(IN const uint32_t typeAttr, IN FILE* fp, IN const uint32_t cntData = 0) {
		size_t retRead = 0;

		if (typeAttr >= POLYGON_DATA_ATTR_MAX || fp == nullptr || cntData <= 0) {
			return retRead;
		}

		// 초기화
		arrAttrCnt[typeAttr] = cntData;


		switch (typeAttr) {
		case TYPE_POLYGON_DATA_ATTR_PART:
		{
			SAFE_DELETE_ARR(pPts);
			pPts = new uint16_t[cntData];
			if (fread(pPts, cntData * sizeof(uint16_t), 1, fp)) {
				retRead = cntData * sizeof(uint16_t);
			}
		}
		break;

		case TYPE_POLYGON_DATA_ATTR_VTX:
		{
			SAFE_DELETE_ARR(pVtx);
			pVtx = new SPoint[cntData];
			if (fread(pVtx, cntData * SIZE_SPOINT, 1, fp)) {
				retRead = cntData * SIZE_SPOINT;
			}
		}
		break;

		case TYPE_POLYGON_DATA_ATTR_ENT:
		{
			SAFE_DELETE_ARR(pEnt);
			pEnt = new stEntranceInfo[cntData];
			if (fread(pEnt, cntData * sizeof(stEntranceInfo), 1, fp)) {
				retRead = cntData * sizeof(stEntranceInfo);
			}
		}
		break;

		case TYPE_POLYGON_DATA_ATTR_LINK:
		{
			SAFE_DELETE_ARR(pLnk);
			pLnk = new KeyID[cntData];
			if (fread(pLnk, cntData * sizeof(KeyID), 1, fp)) {
				retRead = cntData * sizeof(KeyID);
			}
		}
		break;

		case TYPE_POLYGON_DATA_ATTR_MESH:
		{
			SAFE_DELETE_ARR(pMsh);
			pMsh = new uint32_t[cntData];
			if (fread(pMsh, cntData * sizeof(uint32_t), 1, fp)) {
				retRead = cntData * sizeof(uint32_t);
			}
		}
		break;

		default: {
		}
		break;
		} // switch		

		return retRead;
	}

	size_t writeAttribute(IN const uint32_t typeAttr, IN FILE* fp) {
		size_t retWrite = 0;

		if (typeAttr >= POLYGON_DATA_ATTR_MAX || fp == nullptr || arrAttrCnt[typeAttr] <= 0) {
			return retWrite;
		}

		switch (typeAttr) {
		case TYPE_POLYGON_DATA_ATTR_PART:
		{
			if (fwrite(pPts, arrAttrCnt[typeAttr] * sizeof(uint16_t), 1, fp)) {
				retWrite = arrAttrCnt[typeAttr] * sizeof(uint16_t);
			}
		}
		break;

		case TYPE_POLYGON_DATA_ATTR_VTX:
		{
			if (fwrite(pVtx, arrAttrCnt[typeAttr] * SIZE_SPOINT, 1, fp)) {
				retWrite = arrAttrCnt[typeAttr] * SIZE_SPOINT;
			}
		}
		break;

		case TYPE_POLYGON_DATA_ATTR_ENT:
		{
			if (fwrite(pEnt, arrAttrCnt[typeAttr] * sizeof(stEntranceInfo), 1, fp)) {
				retWrite = arrAttrCnt[typeAttr] * sizeof(stEntranceInfo);
			}
		}
		break;

		case TYPE_POLYGON_DATA_ATTR_LINK:
		{
			if (fwrite(pLnk, arrAttrCnt[typeAttr] * sizeof(KeyID), 1, fp)) {
				retWrite = arrAttrCnt[typeAttr] * sizeof(KeyID);
			}
		}
		break;

		case TYPE_POLYGON_DATA_ATTR_MESH:
		{
			if (fwrite(pMsh, arrAttrCnt[typeAttr] * sizeof(uint32_t), 1, fp)) {
				retWrite = arrAttrCnt[typeAttr] * sizeof(uint32_t);
			}
		}
		break;

		default: {
		}
		break;
		} // switch		

		return retWrite;
	}
};

//
//struct stBuildingInfo {
//	KeyID bld_id;
//	SBox data_box;
//	union {
//		uint32_t sub_info;
//		struct {
//			uint32_t type : 5; // 건물 종별 코드, ~30
//			uint32_t height : 10; // 건물 높이, ~1000(m)
//			uint32_t name_type : 3; // 건물 동 번호 타입 0~7, 0:인덱스, 1:정수('101'동), 2:영어('A'동), 3:한글('가'동)
//			uint32_t name_val : 14; // 건물 동 번호 인덱스 ~16383
//		};
//	};
//	std::vector<uint32_t> vtPts; // 폴리곤 파트 인덱스
//	std::vector<SPoint> vtVtx; // 폴리곤 버텍스
//
//	stBuildingInfo() {
//		bld_id.llid = 0;
//		memset(&data_box, 0x00, sizeof(data_box));
//		sub_info = 0;
//	}
//};
//
//struct stComplexInfo {
//	KeyID cpx_id;
//	SBox data_box;
//	union {
//		uint32_t sub_info;
//		struct {
//			uint32_t type : 4; // 건물 종별 코드, ~9
//			uint32_t reserved : 28;
//		};
//	};
//
//	std::vector<uint32_t> vtPts; // 폴리곤 파트 인덱스
//	std::vector<SPoint> vtVtx; // 폴리곤 버텍스
//
//	stComplexInfo() {
//		cpx_id.llid = 0;
//		memset(&data_box, 0x00, sizeof(data_box));
//		sub_info = 0;
//	}
//};



struct INDEXED {
	uint32_t meshID;
	uint32_t sIDX;
	uint32_t eIDX;
};


// DBF 파일에 선형 속성을 위한 클래스
struct _VEG {
	char Type[4];
	char Ratio[4];
};

struct _SC {
	char Type[2];
	char Code[6];
};
struct _ConnectNodeInfo {
	char connectNodeID[7];
	uint32_t connectAngle;
};

struct IDBase {
	union {
		uint64_t ID : 32;
		uint64_t MeshID : 32;
	};
	uint64_t IDBASE;
};

//-----------------------------------------------------------
// Raw Data Part
//-----------------------------------------------------------
struct _MeshRaw
{
	uint32_t MeshID;
	SBox meshBox;
};

struct _LinkRaw
{
	//IDBase LinkBase;
	uint32_t MeshID;
	uint32_t LinkID;
	uint32_t LinkType;
	char szName[256];
	char sNode_id[7];
	char eNode_id[7];
	char CoursType[2];
	char CoursCode[6];
	std::vector<SPoint> vtPoints;
	std::vector<_SC> vSC;
	char Direction[2];
	char DirectionInfo[51];
	std::vector<int> vRoadCode;
	uint32_t ConnectedRoad;
	double LinkLen;
	uint32_t Difficult;
	uint32_t FW_Time;
	uint32_t BW_Time;
	uint32_t Popular;
	char SummerAutumn_Rest[9];
	char SpringWinter_Rest[9];
	std::vector<_VEG> vVEG;
	char Mountain_NM[13];
};

struct _NodeRaw
{
	//IDBase Nodebase;
	uint32_t MeshID;
	uint32_t NodeID;
	uint32_t NodeTypeCode;
	//char szName[256];
	char AdjEdgeMesh[7];
	char AdjEdgeNode[7];
	uint32_t ConnectNum;
	SPoint nodePoint;
	std::vector<_ConnectNodeInfo> _vConnectNode;
};
struct stLinkVtx {
	std::vector<SPoint> vtPts;
};
//-----------------------------------------------------------
// Service Data Part (Link)   Routing 하기 위한 정보
//-----------------------------------------------------------
struct stLinkIDX {

	uint32_t MeshID;
	uint32_t LinkID;
	uint32_t mesh_sNodeID;
	uint32_t sNodeID;
	uint32_t mesh_eNodeID;
	uint32_t eNodeID;
	stLinkVtx	ptLinkVtx;
	int Direction;
	double LinkLength;
	//IDBase linkBase;
	//IDBase sNodeBase;
	//IDBase eNdoeBase;	
	//int nRoadLevel;			
	//int nRoadType;		

	//uint32_t nContOffSet;
	//std::vector<stLinkIDX>vConnectLink; //uint32_t conLinks[12];	
};
//-----------------------------------------------------------
// Service Data Part (Node)   Routing 하기 위한 정보
//-----------------------------------------------------------
struct stNodeIDX {
	//IDBase nodeBase;
	uint32_t MeshID;
	uint32_t NodeID;
	SPoint point;
	uint32_t adjNode_Mesh;
	uint32_t adjNode_ID;
	std::vector<_ConnectNodeInfo> _vConnectNode;
	//uint32_t NodeType;
};


//-----------------------------------------------------------
// 각 데이터 (Link, Node)의 속성 부가정보
//-----------------------------------------------------------

struct stNodeContent {
	char szName[256];
};

struct stLinkContent {
	char szName[256];
	int SummerAutumn_Rest;
	int SpringWinter_Rest;
	int Popular;
	char Mountain_NM[12];
	std::vector<_VEG> vVEG;
};


typedef struct _tagstDistrict
{
	int32_t id; // 클러스터 ID
	double dist; // 예상 거리
	int32_t time; // 예상 시간
	int32_t cargo; // 화물 수량
	uint32_t etd; // 출발 예상 시각
	uint32_t eta; // 도착 예상 시각
	SPoint center; // 무게 중심

	vector<int32_t> vtPois;
	vector<SPoint> vtCoord;
	vector<SPoint> vtBorder;
	vector<int32_t> vtTimes;
	vector<int32_t> vtLayoverTimes;
	vector<int32_t> vtCargoVolume;
	vector<int32_t> vtDistances;

	_tagstDistrict()
	{
		id = 0;
		dist = 0.f;
		time = 0.f;
		cargo = 0;
		etd = 0;
		eta = 0;
	}

	void init()
	{
		id = 0;
		dist = 0.f;
		time = 0.f;
		cargo = 0;
		etd = 0;
		eta = 0;
		center = { 0, 0 };

		vtPois.clear();
		vtCoord.clear();
		vtBorder.clear();
		vtTimes.clear();
		vtLayoverTimes.clear();
		vtCargoVolume.clear();
		vtDistances.clear();
	}
}stDistrict;


struct Origins
{
	SPoint position; // 좌표(WGS84)
	int32_t layoverTime = 0; // 소요시간(초)
	int32_t cargoVolume = 0; // 화물수량

	// 각 지점별 추가 할당 리소스 정보
	int32_t weight = 0; // 추가무게(kg)
	int32_t count = 0; // 추가개수
	int32_t size = 0; // 추가사이즈(cm, 가로+세로+높이)

	//_tagOrigins()
	//{
	//	time = 0;
	//	weight = 0;
	//	count = 0;
	//	size = 0;
	//}
};


struct TruckOption
{
	int32_t height = 0; // 트럭 높이(cm), 제한
	int32_t weight = 0; // 트럭 중량(kg), 제한
	int32_t length = 0; // 트럭 길이(cm), 제한
	int32_t width = 0; // 트럭 너비(cm), 제한
	int32_t hazardous = 0; // 위험 수하물, 0://일발, 1:위험물, 상수원보호구역진입 제한

	bool isEnableTruckOption() const {
		return (height || weight || length || width || hazardous) ? true : false;
	}
};

typedef struct _tagBaseOption
{
	//string userId; // 대입할때 주소복사 되지 않도록 주의할것
	//string uri; // 데이터 경로, 기본적으로 timestamp 값을 준다.
	char userId[128];
	int32_t option; // 탐색 옵션,
	int32_t avoid; // 회피 옵션,
	int32_t traffic; // 교통 정보, 0:미사용, 1:실시간(REAL), 2:통계(STATIC), 3:실시간-통계(REAL-STATIC)
	int32_t free; // 무료 적용, 0:미사용, 1:무료
	int32_t timestamp;
	int32_t fileCache;
	int32_t binary;
	int32_t mobility;
	int32_t distance_type; // RDM 생성 타입, 0:미사용(직선거리) 1:경로
	int32_t compare_type; // RDM 비교 타입, 0:미사용(기본) 1:코스트, 2:거리, 3:시간
	int32_t expand_method; // RDM 확장 방식, 0:기본, 1:레벨 단계화	

	TruckOption truck; // 트럭옵션

	_tagBaseOption()
	{
		//userId = "";
		//uri = "";
		memset(userId, 0x00, sizeof(userId));
		option = 0;
		avoid = 0;
		timestamp = 0;
		traffic = 0;
		free = 0;
		fileCache = 0;
		binary = 0;
		mobility = TYPE_MOBILITY_VEHICLE;
		distance_type = 1; // "0:NONE, 1:ROUTE",
		compare_type = 0; // "0:NONE, 1:COST, 2:DIST, 3:TIME",
		expand_method = 1; // "0:NONE, 1:LEVEL_PROPAGATION", // 2025-08-20 기본 레벨확장방식 사용
	}

	_tagBaseOption& operator=(const _tagBaseOption& rhs)
	{
		strcpy(this->userId, rhs.userId);
		//this->userId = rhs.userId;
		//this->uri = rhs.uri;
		this->option = rhs.option;
		this->avoid = rhs.avoid;
		this->traffic = rhs.traffic;
		this->timestamp = rhs.timestamp;
		this->free = rhs.free;
		this->fileCache = rhs.fileCache;
		this->binary = rhs.binary;
		this->mobility = rhs.mobility;

		this->distance_type = rhs.distance_type;
		this->compare_type = rhs.compare_type;
		this->expand_method = rhs.expand_method;

		memcpy(&this->truck, &rhs.truck, sizeof(this->truck));

		return *this;
	}
}BaseOption;


typedef struct _tagTspOption
{
	BaseOption baseOption;

	int32_t algorithm;
	int32_t geneSize;
	int32_t individualSize;
	int32_t loopCount;
	int32_t seed; // 랜덤 seed
	int32_t compareType;
	int32_t endpointType; // 지점 고정, 0:미사용, 1:출발지 고정, 2:도착지 고정, 3:출발지-도착지 고정, 4:출발지 회귀

	_tagTspOption()
	{
		algorithm = TYPE_TSP_ALGORITHM_TSP_GA;
		geneSize = 0;
		individualSize = 100;
		loopCount = 1000;
		seed = 10000;// 1000 + 49 * 2;
		compareType = TYPE_TSP_VALUE_DIST;
		endpointType = TYPE_TSP_ENDPOINT_NONE;
	}

	_tagTspOption& operator=(const _tagTspOption& rhs)
	{
		this->baseOption = rhs.baseOption;

		this->algorithm = rhs.algorithm;
		this->geneSize = rhs.geneSize;
		this->individualSize = rhs.individualSize;
		this->loopCount = rhs.loopCount;
		this->seed = rhs.seed;
		this->compareType = rhs.compareType;
		this->endpointType = rhs.endpointType;

		return *this;
	}
}TspOption;


typedef struct _tagClusteringOption
{
	TspOption tspOption;
	
	int32_t algorithm;
	int32_t seed; // 랜덤 seed
	int32_t compareType;

	int32_t divisionType; // 분배 타입, 0:갯수균등, 1:거리균등, 2:시간균등
	int32_t limitCluster; // 최대 배차(클러스터링) 차량 수
	int32_t limitValue; // 차량당 최대 배송지수, 최대 거리(미터), 최대 시간(초)-분으로 입력받아 초로 변환
	int32_t limitDeviation; // 차량당 최대 운행 정보 편차
	int32_t max_spot; // 차량당 최대 운송 가능 개수
	int32_t max_distance; // 차량당 최대 운행 가능 거리
	int32_t max_time; // 차량당 최대 운행 가능 시간
	int32_t max_cargo; // 차량당 최대 적재 가능 화물
	int32_t reservation; // 예약 시각
	int32_t reservationType; // 예약 타입, 0:미사용, 1:출발시간, 2:도착시간
	int32_t endpointType; // 지점 고정, 0:미사용, 1:출발지 고정, 2:도착지 고정, 3:출발지-도착지 고정, 4:출발지 회귀
	int32_t additionalType; // 추가 배당 타입, "0:미사용, 1:갯수, 2:무게(g), 3:사이즈(cm)",
	int32_t additionalLimit; // 추가 배당 최대 한도

	_tagClusteringOption()
	{
		algorithm = TYPE_TSP_ALGORITHM_TSP;
		seed = 10006;// 1000 + 49 * 2;
		compareType = TYPE_TSP_VALUE_DIST;

		divisionType = 0;
		limitCluster = 0;
		limitValue = 0;
		limitDeviation = 0;
		max_spot = 0;
		max_distance = 0;
		max_time = 0;
		max_cargo = 0;
		reservation = 0;
		reservationType = 0;
		endpointType = TYPE_TSP_ENDPOINT_NONE;
		additionalType = 0;
		additionalLimit = 0;
	}

	_tagClusteringOption& operator=(const _tagClusteringOption& rhs)
	{
		this->tspOption = rhs.tspOption;

		this->algorithm = rhs.algorithm;
		this->seed = rhs.seed;
		this->compareType = rhs.compareType;

		this->divisionType = rhs.divisionType;
		this->limitCluster = rhs.limitCluster;
		this->limitValue = rhs.limitValue;
		this->limitDeviation = rhs.limitDeviation;
		this->max_spot = rhs.max_spot;
		this->max_distance = rhs.max_distance;
		this->max_time = rhs.max_time;
		this->max_cargo = rhs.max_cargo;
		this->reservation = rhs.reservation;
		this->reservationType = rhs.reservationType;
		this->endpointType = rhs.endpointType;
		this->additionalType = rhs.additionalType;
		this->additionalLimit = rhs.additionalLimit;

		return *this;
	}
}ClusteringOption;
