#pragma once

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>
#include <array>
#include <queue>
#include <algorithm>
#include <stdint.h>
#include <stdio.h>

#include "../shp/shpio.h" 
using namespace std;

#if !defined(_WIN32)
#include <string.h>
#include <limits.h>
#include <float.h>
#endif

#define USE_MULTIPROCESS
#if defined(USE_MULTIPROCESS)
#define PROCESS_NUM 8
#include <omp.h>
#else
#define PROCESS_NUM 1
#endif


#define ENGINE_VERSION_MAJOR	1
#define ENGINE_VERSION_MINOR	0
#define ENGINE_VERSION_PATCH	0

#define USE_ROUTE_TABLE_LEVEL	8 // 8 레벨보다 낮아지면 도로 확장이 어려워짐.
//#define __USE_TEMPLETE // 템플릿 사용

// #define USE_OPTIMAL_POINT_API // 최적 지점 API
#define USE_TREKKING_POINT_API // 경로 탐색 API
#if defined(USE_OPTIMAL_POINT_API)
//#define USE_TREKKING_DATA // 숲길 데이터
//#define USE_PEDESTRIAN_DATA // 보행자/자전거 데이터
#define USE_VEHICLE_DATA // 차량 데이터
#define USE_BUILDING_DATA // 건물 데이터
#define USE_COMPLEX_DATA // 단지 데이터
#elif defined(USE_TREKKING_POINT_API)
// #define USE_TREKKING_DATA // 숲길 데이터
// #define USE_PEDESTRIAN_DATA // 보행자/자전거 데이터
#define USE_VEHICLE_DATA // 차량 데이터
#define USE_P2P_DATA // P2P 데이터
// #define TARGET_FOR_KAKAO_VX // 카카오VX제공 
#endif


//#define USING_PED_BYCICLE_TYPE // 자전거 경로 사용(링크 속성중 자전거 속성 사용)

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

#define SAFE_DELETE(x)		{ delete (x); (x) = nullptr; }
#define SAFE_DELETE_ARR(x)	{ delete [] (x); (x) = nullptr; }

#if !defined(_WIN32)
typedef struct tagRECT
{
    uint32_t    left;
    uint32_t    top;
    uint32_t    right;
    uint32_t    bottom;
} RECT;
#endif

enum NETWORKTYPE { LINK, NODE };
enum DIRECTION { BIDIRECTION, FORWARD, REVERSE };

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
			uint64_t type : 2; // 타입, 0:빌딩, 1:단지
		}ent;
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
	vector<KeyID> nodes;
	vector<KeyID> links;
#if defined(USE_PEDESTRIAN_DATA)
	vector<KeyID> wnodes;
	vector<KeyID> wlinks;
#endif
#if defined(USE_VEHICLE_DATA)
	vector<KeyID> vnodes;
	vector<KeyID> vlinks;
#endif
# if defined(USE_OPTIMAL_POINT_API)
	vector<KeyID> buildings;
	vector<KeyID> complexs;
#endif
	unordered_set<uint32_t> neighbors;

#if 1//defined(_DEBUG)
	set<KeyID> setNodeDuplicateCheck;
	set<KeyID> setLinkDuplicateCheck;
#if defined(USE_PEDESTRIAN_DATA)
	set<KeyID> setWNodeDuplicateCheck;
	set<KeyID> setWLinkDuplicateCheck;
#endif
#if defined(USE_VEHICLE_DATA)
	set<KeyID> setVNodeDuplicateCheck;
	set<KeyID> setVLinkDuplicateCheck;
#endif
# if defined(USE_OPTIMAL_POINT_API)
	set<KeyID> setBldDuplicateCheck;
	set<KeyID> setCpxDuplicateCheck;
#endif
#endif

	stMeshInfo() {
		mesh_id.llid = 0;
		memset(&mesh_box, 0x00, sizeof(mesh_box));
		memset(&data_box, 0x00, sizeof(data_box));
	}

	~stMeshInfo() {
		if (!nodes.empty()) { nodes.clear(); vector<KeyID>().swap(nodes); }
		if (!links.empty()) { links.clear(); vector<KeyID>().swap(links); }
#if defined(USE_PEDESTRIAN_DATA)
		if (!wnodes.empty()) { wnodes.clear(); vector<KeyID>().swap(wnodes); }
		if (!wlinks.empty()) { wlinks.clear(); vector<KeyID>().swap(wlinks); }
#endif
#if defined(USE_VEHICLE_DATA)
		if (!vnodes.empty()) { vnodes.clear(); vector<KeyID>().swap(vnodes); }
		if (!vlinks.empty()) { vlinks.clear(); vector<KeyID>().swap(vlinks); }
#endif
# if defined(USE_OPTIMAL_POINT_API)
		if (!buildings.empty()) { buildings.clear(); vector<KeyID>().swap(buildings); }
		if (!complexs.empty()) { complexs.clear(); vector<KeyID>().swap(complexs); }
#endif
		if (!neighbors.empty()) { neighbors.clear(); unordered_set<uint32_t>().swap(neighbors); }

#if 1//defined(_DEBUG)
		if (!setNodeDuplicateCheck.empty()) { setNodeDuplicateCheck.clear(); set<KeyID>().swap(setNodeDuplicateCheck); }
		if (!setLinkDuplicateCheck.empty()) { setLinkDuplicateCheck.clear(); set<KeyID>().swap(setLinkDuplicateCheck); }
#if defined(USE_PEDESTRIAN_DATA)
		if (!setWNodeDuplicateCheck.empty()) { setWNodeDuplicateCheck.clear(); set<KeyID>().swap(setWNodeDuplicateCheck); }
		if (!setWLinkDuplicateCheck.empty()) { setWLinkDuplicateCheck.clear(); set<KeyID>().swap(setWLinkDuplicateCheck); }
#endif
#if defined(USE_VEHICLE_DATA)
		if (!setVNodeDuplicateCheck.empty()) { setVNodeDuplicateCheck.clear(); set<KeyID>().swap(setVNodeDuplicateCheck); }
		if (!setVLinkDuplicateCheck.empty()) { setVLinkDuplicateCheck.clear(); set<KeyID>().swap(setVLinkDuplicateCheck); }
#endif
# if defined(USE_OPTIMAL_POINT_API)
		if (!setBldDuplicateCheck.empty()) { setBldDuplicateCheck.clear(); set<KeyID>().swap(setBldDuplicateCheck); }
		if (!setCpxDuplicateCheck.empty()) { setCpxDuplicateCheck.clear(); set<KeyID>().swap(setCpxDuplicateCheck); }
#endif
#endif
	}
};

typedef struct _tagstNodeBaseInfo{
	uint32_t node_type : 3; // 0:미정의, 1:숲길, 2:보행자, 3:자전거, 4:차량
	uint32_t point_type : 4; // 노드 타입, 1:교차점, 2:단점, 3:더미점, 4:구획변경점, 5:속성변화점, 6:지하철진출입, 7:철도진출입, 8:지하도진출입, 9:지하상가진출입, 10:건물진출입
	uint32_t connnode_count : 4; // 접속 노드 수, MAX:15
	uint32_t shared : 21;
}stNodeBaseInfo;

typedef struct _tagstNodeTrekkingInfo {
	uint32_t dummy_type : 11; // node_base 공유 공간
	uint32_t z_value : 10; // 고도 정보
	uint32_t ped_reserved : 11; // reserved
}stNodeTrekkingInfo;

typedef struct _tagstNodePedestrianInfo {
	uint32_t dummy_type : 11; // node_base 공유 공간
	uint32_t facility_phase : 2; // 시설물 위상, 0:미정의, 1:지하, 2:지상, 3:지상위
	uint32_t gate_phase : 2; // 입출구 위상, 0:미정의, 1:지하, 2:지상, 3:지상위
	uint32_t tre_reserved : 17; // reserved
}stNodePedestrianInfo;

struct stNodeInfo {
	KeyID node_id;
	KeyID edgenode_id; //엣지노드가 있는 경우는 연결노드가 1개일 경우 밖에 없을 것 같아, connnodes와 공유해도 될듯(확인필요)
	SPoint coord;
	KeyID connnodes[8]; // 연결된 링크 ID
	uint16_t conn_attr[8];
	uint32_t name_idx; // 명칭 인덱스
	union {
		uint32_t sub_info;
		stNodeBaseInfo base;
		stNodeTrekkingInfo tre;
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
	uint64_t link_type : 3; // 0:미정의, 1:숲길, 2:보행자, 3:자전거, 4:차량
	uint64_t shared : 29; // 타입별 속성 공유공간
	uint64_t snode_ang : 16; // 360
	uint64_t enode_ang : 16; // 360
}stLinkBaseInfo;

typedef struct _tagstLinkPedestrianInfo {
	uint64_t dummy_type : 3; // link_type 공유 공간
	uint64_t bycicle_type : 2; // 자전거도로 타입, 1:자전거전용, 2:보행자/차량겸용 자전거도로, 3:보행도로
	uint64_t walk_type : 3; //보행자도로 타입, 1:복선도록, 2:차량겸용도로, 3:자전거전용도로, 4:보행전용도로, 5:가상보행도로
	uint64_t facility_type : 4; // 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
	uint64_t gate_type : 4; // 진입로 타입, 0:미정의, 1:경사로, 2:계단, 3:에스컬레이터, 4:계단/에스컬레이터, 5:엘리베이터, 6:단순연결로, 7:횡단보도, 8:무빙워크, 9:징검다리, 10:의사횡단
	uint64_t lane_count : 6; // 차선수, 63
	uint64_t side_walk : 2; // 인도(보도) 여부, 0:미조사, 1:있음, 2:없음
	uint64_t walk_charge : 2; // 보행자도로 유료 여부, 0:무료, 1:유료
	uint64_t bycicle_control : 2; // 자전거도로 규제 코드, 0:양방향, 1:정방향, 2:역방향, 3:통행불가
	uint64_t ped_reserved : 4; // reserved
	uint64_t shared : 32; // link_ang 공유 공간
}stLinkPedestrianInfo;

typedef struct _tagstLinkTrekkingInfo {
	uint64_t dummy_type : 3; // link_type 공유 공간
	uint64_t course_type : 3; // 0:미정의, 1 : 등산로, 2 : 둘레길, 3 : 자전거길, 4 : 종주코스
	uint64_t road_info : 11; // 노면정보 코드, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드
	uint64_t diff : 6; // 난도 0~50, (숫자가 클수록 어려움)
	uint64_t popular : 4; // 인기도 지수, 0-10
	uint64_t tre_reserved : 5; // reserved
	uint64_t shared : 32; // link_ang 공유 공간
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
	uint64_t charge : 1; //유료도로, 0:없음, 1:있음
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
	uint64_t shared : 32; // link_ang 공유 공간
#endif
	// shared 대신 링크별 통행코드를(메모리 사이즈 절약차원) 넣으려했으나 그냥 노드의 통행코드 사용하기로 함(2022-12-12)
	//uint64_t snode_pass : 16; // 0:통행가능(유턴불가), 1:통행가능(유턴가능), 2:통행불가 // 8자리 통행코드를 각 코드 위치별로 2비트씩 16비트에 나눠 담는다.
	//uint64_t enode_pass : 16;
}stLinkVehicleInfo;

typedef struct _tagstLinkVehicleInfoExt {
	uint64_t snode_ang : 16; // 360
	uint64_t enode_ang : 16; // 360
	uint64_t veh_ext_reserved : 32;
}stLinkVehicleInfoExt;

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
		stLinkTrekkingInfo tre;
		stLinkVehicleInfo veh;
	};
#if defined(USE_P2P_DATA)
	union {
		uint64_t sub_ext; // 확장 정보 // 현재는 고유 ID (mesh(6) + snode(6) + enode(6)
		stLinkVehicleInfoExt veh_ext;
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
#if defined(USE_P2P_DATA)
		sub_ext = 0;
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

struct stEntranceInfo {
	uint32_t poly_type : 2; // 폴리곤 타입, 0:미지정, 1:빌딩, 2:단지
	uint32_t ent_code : 4; // 입구점 타입 // 1: 차량 입구점, 2: 택시 승하차 지점(건물), 3: 택시 승하차 지점(건물군), 4: 택배 차량 하차 거점, 5: 보행자 입구점, 	6: 배달 하차점(차량, 오토바이), 7: 배달 하차점(자전거, 도보)
	uint32_t reserved : 26;
	double x;
	double y;
};


struct stTrafficKey{
	union {
		uint64_t llid; // mesh(6) + snode(6) + enode(6) + dir(1)
		struct {
			uint64_t mesh : 20;
			uint64_t snode : 20;
			uint64_t enode : 20;
			uint64_t dir : 4;
		};
	};
};

struct stTrafficInfo {
	stTrafficKey key;
	uint32_t ks_id;
	uint32_t time_stamp; // 최종 업데이트된 시각
	uint8_t speed; // 도로 주행 속도 0~255, 255일 경우는 통제, 주행불가

	unordered_set<uint64_t> setLinks; // ks 링크에 매칭되는 link id -> 나중에는 메모리 관리를 위해 포인터로 관리하자!!
};


#define POLYGON_DATA_ATTR_MAX		5
typedef enum {
	TYPE_POLYGON_DATA_ATTR_PART = 0,// 폴리곤 파트 인덱스
	TYPE_POLYGON_DATA_ATTR_VTX,		// 폴리곤 버텍스
	TYPE_POLYGON_DATA_ATTR_ENT,		// 입구점 정보
	TYPE_POLYGON_DATA_ATTR_LINK,	// 단지내 도로 ID
	TYPE_POLYGON_DATA_ATTR_MESH		// 중첩 메쉬
}TYPE_POLYGON_DATA_ATTR;

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
			uint64_t reserved : 52;
		}cpx;
	};

private:
	std::array<uint32_t, POLYGON_DATA_ATTR_MAX> arrAttrCnt = { 0, };

	uint8_t* pPts; // 폴리곤 파트 인덱스
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

	const uint8_t* getAttributeParts() const {
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
			pPts = new uint8_t[cntData];
			memcpy(pPts, pData, cntData * sizeof(uint8_t));
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
			pPts = new uint8_t[cntData];
			if (fread(pPts, cntData * sizeof(uint8_t), 1, fp)) {
				retRead = cntData * sizeof(uint8_t);
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
			if (fwrite(pPts, arrAttrCnt[typeAttr] * sizeof(uint8_t), 1, fp)) {
				retWrite = arrAttrCnt[typeAttr] * sizeof(uint8_t);
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

// 데이터 파일 구분
typedef enum {
	TYPE_DATA_NAME, // 명칭사전
	TYPE_DATA_MESH, // 메쉬
	TYPE_DATA_TREKKING, // 숲길
	TYPE_DATA_PEDESTRIAN, // 보행자/자전거
	TYPE_DATA_VEHICLE, // 자동차
	TYPE_DATA_BUILDING, // 건물
	TYPE_DATA_COMPLEX, // 단지
	TYPE_DATA_ENTRANCE, // 입구점
	TYPE_DATA_TRAFFIC, // 교통정보
	TYPE_DATA_COUNT,
}TYPE_DATA;


// 숲길 데이터
typedef enum {
	TYPE_TRE_NONE = 0, // 미지정
	TYPE_TRE_HIKING, // 등산로
	TYPE_TRE_TRAIL, // 둘레길
	TYPE_TRE_BIKE, // 자전거길
	TYPE_TRE_CROSS, // 종주길
}TYPE_TREKKING;



// 보행자/자전거 데이터

typedef enum {
	TYPE_WALK_SIDE = 1, // 복선도록
	TYPE_WALK_WITH_CAR, // 차량겸용도로
	TYPE_WALK_WITH_BYC, // 자전거전용도로 -- 우선은 보행 불가 지역으로 이해하자
	TYPE_WALK_ONLY, // 보행전용도로
	TYPE_WALK_THROUGH, //가상보행도로
}TYPE_WALK;

typedef enum {
	TYPE_BYC_ONLY = 1, // 자전거 전용
	TYPE_BYC_WITH_CAR, // 보행자/차량겸용 자전거도로
	TYPE_BYC_WITH_WALK, // 보행도로 -- 우선은 자전거 통과 불가 지역으로 이해하자
}TYPE_BYCICLE;


typedef enum {
	TYPE_NODE_COROSS = 1, // 교차첨
	TYPE_NODE_END, // 단점
	TYPE_NODE_DUMMY, // 더미점
	TYPE_NODE_EDGE, // 구획변교점
	TYPE_NODE_ATTRIBUTE, // 속성변화점
	TYPE_NODE_SUBWAY, // 지하철 진출입
	TYPE_NODE_UNDERPASS, // 지하도 진출입
	TYPE_NODE_UNDERGROUND_MALL, // 지하상가 진출입
	TYPE_NODE_BUILDING, // 건물 진출입
}TYPE_NODE;



typedef enum {
	ROUTE_TYPE_NONE = 0, // 미정의
	ROUTE_TYPE_TREKKING, // 숲길
	ROUTE_TYPE_PEDESTRIAN, // 보행자
	ROUTE_TYPE_BIKE, // 자전거
	ROUTE_TYPE_KICKBOARD, // 킥보드
	ROUTE_TYPE_MOTOCYCLE, // 모터사이클
	ROUTE_TYPE_VEHICLE, // 차량
}ROUTE_TYPE;


typedef enum {
	ROUTE_OPT_SHORTEST = 0, // 최단거리
	ROUTE_OPT_RECOMMENDED, // 추천
	ROUTE_OPT_COMFORTABLE, // 편안한
	ROUTE_OPT_FASTEST, // 최소시간
	ROUTE_OPT_MAINROAD, // 큰길
	ROUTE_OPT_PEDESTRIAN, // 보행자 전용
	ROUTE_OPT_TRAIL, // 둘레길 전용
	ROUTE_OPT_BIKE, // 자전거 전용
	ROUTE_OPT_AUTOMATION, // 자율주행 전용
}ROUTE_OPTION;


typedef enum {
	ROUTE_AVOID_NONE = 0,
	//ROUTE_AVOID_HIKING = 1, // 등산로
	//ROUTE_AVOID_TRAIL = 2, // 둘레길
	//ROUTE_AVOID_BIKE = 4, // 자전거
	//ROUTE_AVOID_CROSS = 8, // 종주길

	ROUTE_AVOID_ALLEY = 1,
	ROUTE_AVOID_PAVE = 2,
	ROUTE_AVOID_STAIRS = 4,
	ROUTE_AVOID_BRIDGE = 8,
	ROUTE_AVOID_ROCK = 16,
	ROUTE_AVOID_RIDGE = 32,
	ROUTE_AVOID_LADDER = 64,
	ROUTE_AVOID_ROPE = 128,
	ROUTE_AVOID_TATTERED = 256,
	ROUTE_AVOID_PALM = 512,
	ROUTE_AVOID_DECK = 1024,
	// 0:미정의, 1 : 등산로, 2 : 둘레길, 3 : 자전거길, 4 : 종주코스
	// 1:오솔길(1), 2:포장길(2), 3:계단(4), 4:교량(8), 5:암릉(16), 6:릿지(32), 7:사다리(64), 8:밧줄(128), 9:너덜길(256), 10:야자수매트(512), 11:데크로드(1024)
}ROUTE_AVOID;

// 링크 서브 타입
typedef enum {
	TYPE_SUBINFO_NONE = 0, // 미지정
	TYPE_SUBINFO_FTYPE, // 시설물 타입
	TYPE_SUBINFO_GTYPE, // 진입로 타입
}TYPE_SUBINFO;

// 건물 종별 코드
typedef enum {
	TYPE_BUILDING_NOT = 0,//미지정
	TYPE_BUILDING_APT, //아파트
	TYPE_BUILDING_SCH, //학교
	TYPE_BUILDING_OFT, //오피스텔
	TYPE_BUILDING_B01, //공공기관
	TYPE_BUILDING_B02, //치안기관
	TYPE_BUILDING_B03, //공원
	TYPE_BUILDING_B04, //교육기관
	TYPE_BUILDING_B05, //언론기관
	TYPE_BUILDING_B06, //금융기관
	TYPE_BUILDING_B07, //문화
	TYPE_BUILDING_B08, //관광
	TYPE_BUILDING_B09, //레져
	TYPE_BUILDING_B10, //음식점
	TYPE_BUILDING_B11, //편의
	TYPE_BUILDING_B12, //복지
	TYPE_BUILDING_B13, //기업
	TYPE_BUILDING_B14, //농공시설
	TYPE_BUILDING_B15, //자동차관련
	TYPE_BUILDING_B16, //교통시설
	TYPE_BUILDING_B17, //지하철시설
	TYPE_BUILDING_B18, //도로시설
	TYPE_BUILDING_B19, //주택관련
	TYPE_BUILDING_ETC, //주택외건물
	TYPE_BUILDING_E2D, //주택외건물2D
	TYPE_BUILDING_IND, //공업단지내건물
	TYPE_BUILDING_STA, //기차역
	TYPE_BUILDING_APC, //주상복합
	TYPE_BUILDING_ROH, //연립주택
	TYPE_BUILDING_APL, //아파트형공장
	TYPE_BUILDING_APS, //아파트 상가
	TYPE_BUILDING_MAX_CNT,
}TYPE_BUILDING;

typedef enum {
	TYPE_BLD_NAME_NOT = 0, // 명칭 사전 인덱스
	TYPE_BLD_NAME_INT, // 정수 '123'
	TYPE_BLD_NAME_ENG, // 영문 'A'
	TYPE_BLD_NAME_DIC, // 명칭 사전
}TYPE_BLD_NAME;

typedef enum {
	TYPE_COMPLEX_NOT = 0, //미지정
	TYPE_COMPLEX_APC,	// 주상복합단지
	TYPE_COMPLEX_APL,	// 아파트형공장단지
	TYPE_COMPLEX_APT,	// 아파트단지
	TYPE_COMPLEX_B01,	// 공공기관단지
	TYPE_COMPLEX_B04,	// 교육기관단지
	TYPE_COMPLEX_B11,	// 편의시설단지
	TYPE_COMPLEX_ETC,	// 기타단지
	TYPE_COMPLEX_OFT,	// 오피스텔단지
	TYPE_COMPLEX_ROH,	// 연립주택단지
	TYPE_COMPLEX_SCH,	// 학교단지
	TYPE_COMPLEX_MAX_CNT,
}TYPE_COMPLEX;

typedef enum {
	TYPE_ENT_NONE = 0, // 미지정
	TYPE_ENT_BUILDING, // 빌딩입구점
	TYPE_ENT_COMPLEX, // 단지입구점
	TYPE_ENT_CPX_ROAD, // 단지내도로
	TYPE_ENT_NEAR_ROAD, // 최근접도로
}TYPE_ENTRANCE;

typedef enum {
	TYPE_LINK_MATCH_NONE = 0, // 미지정
	TYPE_LINK_MATCH_CARSTOP, // 차량 승하자
	TYPE_LINK_MATCH_CARSTOP_EX, // 차량 승하자 + 단지내도로(건물입구점 재확인시, 단지도로에 매칭된 입구점을 확인하기 위해 포함)
	TYPE_LINK_MATCH_CARENTRANCE, // 차량 진출입 전용
	TYPE_LINK_MATCH_FOR_TABLE, // 방문지 테이블용 , 2차선 이상만
	TYPE_LINK_MATCH_FOR_HD, // P2P HD 경로 탐색 용, sd-hd 매칭되는 링크만 선택
}TYPE_LINK_MATCH;

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


typedef enum {
	ROUTE_TARGET_DEFAULT = 0, // default
	ROUTE_TARGET_INAVI, // for inavi
	ROUTE_TARGET_KAKAOVX, // for kakaovx
} ROUTE_TARGET;

typedef enum {
	LINK_GUIDE_TYPE_DEFAULT = 0, // 일반
	LINK_GUIDE_TYPE_DEPARTURE, // 출발지
	LINK_GUIDE_TYPE_DESTINATION, // 도착지
	LINK_GUIDE_TYPE_WAYPOINT, // 경유지
} LINK_GUIDE_TYPE;

typedef enum {
	ROUTE_RESULT_SUCCESS = 0,				//0		성공

	ROUTE_RESULT_FAILED = 1,				//1		탐색 실패	내부 오류에 의한 실패
	ROUTE_RESULT_FAILED_SAME_ROUTE = 2,		//2		스마트 재탐색 적용	기존 경로와 동일
	ROUTE_RESULT_FAILED_WRONG_PARAM = 10,	//10	잘 못된 파라미터, 필수 파라미터 체크
	ROUTE_RESULT_FAILED_SET_MEMORY = 50,	//50	탐색 확장 관련 메모리 할당 오류	탐색 초기화 관련
	ROUTE_RESULT_FAILED_READ_DATA = 51,		//51	탐색 관련 데이터(지도, 옵션 등) 파일 읽기 실패	탐색 초기화 관련
	ROUTE_RESULT_FAILED_SET_START = 70,		//70	출발지가 프로젝션이 안되거나, 잘못된 출발지
	ROUTE_RESULT_FAILED_SET_VIA = 71,		//71	경유지가 프로젝션이 안되거나, 잘못된 경유지
	ROUTE_RESULT_FAILED_SET_END = 72,		//72	목적지가 프로젝션이 안되거나, 잘못된 목적지
	ROUTE_RESULT_FAILED_DIST_OVER = 90,		//90	탐색 가능 거리 초과, 직선거리 15km 이내 허용
	ROUTE_RESULT_FAILED_TIME_OVER = 91,		//91	탐색 시간 초과	10초 이상
	ROUTE_RESULT_FAILED_NODE_OVER = 92,		//92	확장 가능 Node 개수 초과
	ROUTE_RESULT_FAILED_EXPEND = 93,		//93	확장 실패


// 	resultCode	resultMessage	비고	설명
// 0		공통	성공
// 100	Result Not Found	검색 전용	결과 없음
// 101	Argument Error	공통	파라미터 오류
// 102	Internal Server Error	공통	서버 오류
// 103	Different Map Version	검색 전용	지도 버전 다름
// 104	StyleID already exists	검색 전용	이미 동일한 StyleID가 있음
// 201	Searching for Security	검색 전용	POI 보안시설물
// 202	Longitude/Latitude	검색 전용	경위도
// 203	Mobile Phone Number	검색 전용	전화번호 (Mobile)
// 204	Invalid Query	검색 전용	서버 오류
// 205	POI not in given Admin	검색 전용	결과 없음 (지역설정)
// 206	POI not in given Area	검색 전용	결과 없음 (영역설정)
// 207	POI not in given Category	검색 전용	결과 없음 (분류설정)
// 208	Neighbor Search Only	검색 전용	결과 없음 (주변검색만입력)
// 209	Neighbor Search not Found	검색 전용	결과 없음 (주변 + 키워드 검색 결과 없음)
// 300	AppKey Error	공통	AppKey 인증 오류
// 400	Taxi Fare Info not Found	검색 전용	택시 요금 정보 없음
// 401	AreaCode Convert Fail	검색 전용	지역코드 변환 실패
// 501	Unknown Fail	탐색 전용	실패(unknown)
// 502	Apply Smart Re-navigation	탐색 전용	Smart재탐색 적용
// 503	Canceled navigation by user	탐색 전용	사용자에 의한 탐색 취소
// 504	Error due to Checksum	탐색 전용	체크섬 오류
// 517	Memory allocation failure	탐색 전용	메모리 할당 실패
// 518	File open failure	탐색 전용	파일 열기 실패
// 519	File read failure	탐색 전용	파일 읽기 실패
// 520	File write failure	탐색 전용	파일 쓰기 실패
// 521	Socket connection failure	탐색 전용	소켓 연결 실패
// 532	Request parameter is invalid	탐색 전용	요청 파라미터가 유효하지 않음
// 533	The starting point is not selected, or the wrong starting point	탐색 전용	출발지가 선택되지 않았거나, 잘못된 출발지
// 534	Destination is not selected or wrong destination	탐색 전용	목적지가 선택되지 않았거나, 잘못된 목적지
// 535	Wrong stopover	탐색 전용	잘못된 경유지
// 536	Link Projection failure	탐색 전용	Link Projection 실패 (네트워크(도로망)이 없는 경우 등)
// 537	Exceeding the navigational distance (1000km, walking navigation: 20km)	탐색 전용	탐색 가능 거리 초과(1000km, 도보 탐색 : 20km)
// 538	Exceeds the number of expandable nodes	탐색 전용	확장 가능 노드 수 초과
// 539	Expansion failure	탐색 전용	확장 실패
// 540	Expansion failure due to inactivity or traffic control	탐색 전용	특별한 사정이나 교통 통제로 인한 확장 실패
// 541	Expansion failure due to vehicle height/weight restrictions near the starting point	탐색 전용	출발지 근처의 차량 높이/중량 제한으로 확장 실패
// 542	Expansion failed due to a part-time curfew near the departure point	탐색 전용	출발지 근처의 시간제 통행금지로 인해 확장 실패
// 543	The destination is a physical island road, and there are no established ferry routes.	탐색 전용	목적지가 물리적 섬도로이며, 구축된 페리 항로가 없음
// 544	The departure or destination is a logical (transportation) island, and there are more than one destination	탐색 전용	출발 또는 목적지가 논리적(교통) 섬이며, 목적지가 2개 이상인 경우
// 545	No data requested	탐색 전용	요청한 데이터가 없음
}ROUTE_RESULT;

typedef struct _tagRouteResultLink {
	KeyID linkId;
	KeyID nodeId;
	uint32_t vertexIdx : 16; // 최대 65353
	uint32_t linkLength : 16; // 최대 65353
	uint32_t linkTime : 16; // 최대 65353
	uint32_t dir : 16; // 최대 65353
}RouteResultLink;

typedef struct _tagRouteResultLinkEx {
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
	uint32_t dir : 1; // 링크 방향성, 0:정방향, 1:역방향
	uint32_t type : 2; // 링크 안내 타입, 0:일반, 1:출발지링크, 2:도착지링크, 3:경유지링크
	uint32_t reserved : 6;
}RouteResultLinkEx;

typedef struct _tagRouteResultLinkMatchInfo {
	KeyID StartLinkId; // 시작 링크 ID
	SPoint StartMatchCoord; // 링크와 직교 접점 좌표
	int32_t StartLinkSplitIdx; // 링크와 직교 접점 좌표의 링크 버텍스 idx
	int32_t StartLinkDist; // 직교 접점에서 노드까지의 거리
	vector<SPoint> StartLinkVtx; // 시작점에서 노드까지의 버텍스

	SPoint Coord; // 좌표
	KeyID LinkId; // 링크 ID
	SPoint MatchCoord; // 링크와 직교 접점 좌표
	int32_t LinkSplitIdx; // 링크와 직교 접점 좌표의 링크 버텍스 idx
	int32_t LinkDist; // 직교 접점에서 노드까지의 거리
	vector<SPoint> LinkVtx; // 좌표점에서 s버텍스

	~_tagRouteResultLinkMatchInfo() {
		if (!StartLinkVtx.empty()) {
			StartLinkVtx.clear();
			vector<SPoint>().swap(StartLinkVtx);
		}
		
		if (!LinkVtx.empty()) {
			LinkVtx.clear();
			vector<SPoint>().swap(LinkVtx);
		}
	}

	void Init() {
		memset(&Coord, 0x00, sizeof(Coord));
		LinkId.llid = 0;		
		memset(&MatchCoord, 0x00, sizeof(MatchCoord));
		LinkSplitIdx = 0;
		LinkDist = 0;

		if (!LinkVtx.empty()) {
			LinkVtx.clear();
			vector<SPoint>().swap(LinkVtx);
		}
	}

	_tagRouteResultLinkMatchInfo& operator=(const _tagRouteResultLinkMatchInfo& rhs) {
		StartLinkId = rhs.StartLinkId; // 시작 링크 ID
		StartMatchCoord = rhs.StartMatchCoord; // 링크와 직교 접점 좌표
		StartLinkSplitIdx = rhs.StartLinkSplitIdx; // 링크와 직교 접점 좌표의 링크 버텍스 idx
		StartLinkDist = rhs.StartLinkDist; // 직교 접점에서 노드까지의 거리
		StartLinkVtx.assign(rhs.StartLinkVtx.begin(), rhs.StartLinkVtx.end()); // 시작점에서 노드까지의 버텍스

		Coord = rhs.Coord; // 좌표
		LinkId = rhs.LinkId; // 링크 ID
		MatchCoord = rhs.MatchCoord; // 링크와 직교 접점 좌표
		LinkSplitIdx = rhs.LinkSplitIdx; // 링크와 직교 접점 좌표의 링크 버텍스 idx
		LinkDist = rhs.LinkDist; // 직교 접점에서 노드까지의 거리
		LinkVtx.assign(rhs.LinkVtx.begin(), rhs.LinkVtx.end()); // 좌표점에서 s버텍스

		return *this;
	}
} RouteResultLinkMatchInfo;

typedef struct _tagRouteResultInfo {
	uint32_t ResultCode; // 경로 결과 코드, 0:성공, 1~:실패

	uint32_t RequestMode; // 요청 모드
	uint32_t RequestId; // 요청 ID

	uint32_t RouteOption; // 경로 옵션
	uint32_t RouteAvoid; // 경로 회피

	RouteResultLinkMatchInfo StartResultLink;
	RouteResultLinkMatchInfo EndResultLink;

	uint32_t TotalLinkDist; // 경로 전체 거리
	uint32_t TotalLinkCount; // 경로 전체 링크 수
	uint32_t TotalLinkTime; // 경로 전체 소요 시간 (초)

	// 경로선
	SBox RouteBox; // 경로선 영역	
	vector<RouteResultLinkEx> LinkInfo; // 링크 정보
	vector<SPoint> LinkVertex; // 경로선
	
	void Init() {
		ResultCode = ROUTE_RESULT_FAILED;

		RequestMode = 0;
		RequestId = 0;

		RouteOption = 0;
		RouteAvoid = 0;

		StartResultLink.Init();
		EndResultLink.Init();

		TotalLinkDist = 0;
		TotalLinkCount = 0;
		TotalLinkTime = 0;

		memset(&RouteBox, 0x00, sizeof(RouteBox));

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