#pragma once

#include "../include/MapDef.h"
#include "../cvt/FileManager.h"

#include "MapName.h"
#include "MapMesh.h"
#include "MapNode.h"
#include "MapLink.h"
#include "MapExtend.h"
#include "MapPolygon.h"
#include "MapTraffic.h"
#include "MapCourse.h"

#if defined(__USE_TEMPLETE)
#include "MapBase.h"
#endif

//#define USE_DATA_CACHE // 파일 일부만 읽어 메모리에 캐싱하자
#define MAX_MESH_COUNT  10000 // 현재 원도의 최대 메쉬 카운트
#define MAX_CASH_COUNT	300 // 우선 테스트로 n개만 해서 상태 확인 후 확장

#define USE_INAVI_STATIC_DATA

#pragma pack (push, 1)

// 입구점 정보
struct stEntryPointInfo {
	int32_t nAttribute; // 속성, 1: 차량 입구점, 2: 택시 승하차 지점(건물), 3: 택시 승하차 지점(건물군), 4: 택배 차량 하차 거점, 5: 보행자 입구점, 	6: 배달 하차점(차량, 오토바이), 7: 배달 하차점(자전거, 도보)
	double dwDist; // 요청 지점으로부터 거리(m)
	double x;
	double y;
	int32_t nAngle; // 최적지점API는 링크 각도 0~360
	uint64_t nID_1; // 숲길 경탐 시, 입구점 매칭 숲길 노드 ID 정보
	uint64_t nID_2; // 숲길 경탐 시, 입구점 매칭 보행자 노드 ID 정보
};

// 최적 지점 결과 정보
struct stOptimalPointInfo {
	double x;
	double y;

	// 차량 매칭 타입, 0:일반도로(미매칭), 1:빌딩입구점, 2:단지입구점, 3:단지내도로//, 4:최근접도로
	// 숲길 매칭 타입, 0:일반도로(미매칭), 1:숲길입구점
	int32_t nType;	
	uint64_t id; // 매칭된 오브젝트 ID, 단지/빌딩/도로/산코드
	string name;
	vector<stEntryPointInfo> vtEntryPoint;
	vector<SPoint> vtPolygon;
};

// 최적지점 요청
struct stReqOptimal {
	double x;
	double y;
	bool isExpand;
	union {
		int32_t typeAll;
		struct {
			uint8_t type1st;
			uint8_t type2nd;
			uint8_t type3rd;
			uint8_t type4th;
		};
	};
};

struct FileCacheData {
	uint32_t nTileId;
	int32_t nMatchKey;
	int32_t cntMatchingPoint;

	bool operator<(const FileCacheData* rhs) const {
		if (cntMatchingPoint > rhs->cntMatchingPoint) {
			return true;
		}
		return false;
	}
};

struct stLinkMatchTable {
	uint64_t link_id;
	int32_t link_idx;
};

// 나중에 Cost Manager 따로 두고 관리하는 것도 고려해볼만함.
typedef struct _tagDataCost {
	union {
		double base[128];
		struct {
			double cost_lv0; double cost_lv1; double cost_lv2; double cost_lv3; double cost_lv4;
			double cost_lv5; double cost_lv6; double cost_lv7; double cost_lv8; double cost_lv9;
			double cost_ang0; double cost_ang45; double cost_ang90; double cost_ang135; double cost_ang180;
			double cost_ang225; double cost_ang270; double cost_ang315;
			// 18
		}vehicle;
		struct
		{
			// 차선가중치(걷기/자전거): 최단거리, 추천, 편한, 최소, 큰길
			double cost_lane_walk0; double cost_lane_walk1; double cost_lane_walk2; double cost_lane_walk3; double cost_lane_walk4;
			double cost_lane_bike0; double cost_lane_bike1; double cost_lane_bike2; double cost_lane_bike3; double cost_lane_bike4;

			// 횡단보도(걷기/자전거): 최단거리, 추천, 편한, 최소, 큰길
			double cost_cross_walk0; double cost_cross_walk1; double cost_cross_walk2; double cost_cross_walk3; double cost_cross_walk4;
			double cost_cross_bike0; double cost_cross_bike1; double cost_cross_bike2; double cost_cross_bike3; double cost_cross_bike4;

			// 회전(걷기/자전거): 최단거리, 추천, 편한, 최소, 큰길
			double cost_angle_walk0; double cost_angle_walk1; double cost_angle_walk2; double cost_angle_walk3; double cost_angle_walk4;
			double cost_angle_bike0; double cost_angle_bike1; double cost_angle_bike2; double cost_angle_bike3; double cost_angle_bike4;

			// 자전거도로(전용/겸용/보행): 최단거리, 추천, 편한, 최소, 큰길
			double cost_bike_bike0; double cost_bike_bike1; double cost_bike_bike2; double cost_bike_bike3; double cost_bike_bike4;
			double cost_bike_with0; double cost_bike_with1; double cost_bike_with2; double cost_bike_with3; double cost_bike_with4;
			double cost_bike_walk0; double cost_bike_walk1; double cost_bike_walk2; double cost_bike_walk3; double cost_bike_walk4;

			// 숲길(기본): 최단거리, 추천, 편한, 최소, 큰길
			double cost_forest_base0; double cost_forest_base1; double cost_forest_base2; double cost_forest_base3; double cost_forest_base4;

			// 숲길(인기도): 최단거리, 추천, 편한, 최소, 큰길
			double cost_forest_popular0; double cost_forest_popular1; double cost_forest_popular2; double cost_forest_popular3; double cost_forest_popular4;

			// 숲길(코스): 최단거리, 추천, 편한, 최소, 큰길
			double cost_forest_course0; double cost_forest_course1; double cost_forest_course2; double cost_forest_course3; double cost_forest_course4;

			// 숲길(경사도): 최단거리, 추천, 편한, 최소, 큰길
			double cost_forest_slop0; double cost_forest_slop1; double cost_forest_slop2; double cost_forest_slop3; double cost_forest_slop4;

			// cnt : 60
		}pedestrian;
		struct {
			double cost_lv0; double cost_lv1; double cost_lv2; double cost_lv3; double cost_lv4;
			double cost_lv5; double cost_lv6; double cost_lv7; double cost_lv8; double cost_lv9;
			double cost_mes; double cost_bld; double cost_cpx; double cost_ent; double cost_rod;
			// 15
		}optimal;
	};
}DataCost;

#pragma pack (pop)


class ReadTTLSpd;


class CDataManager
{
public:
	CDataManager();
	~CDataManager();

protected:
	// 전체 지도 영역
	SBox m_rtBox;


private:
	CFileManager* m_pFileMgr;

	std::vector<INDEXED> m_vtLinkIdxTable;
	std::vector<INDEXED> m_vtNodeIdxTable;
	std::map<int, SBox> m_vtMeshRaw;


	string m_strMsg;

	// cache
	uint32_t m_cntMaxCache;
	std::map<uint32_t, FileCacheData> m_mapCache;

	// for optimal cost
	DataCost m_dataCost;

protected:
	char m_szDataPath[FILENAME_MAX];
	
	std::unordered_map<uint32_t, FileIndex> m_mapFileIndex;

	MapName* m_pMapName;
	MapMesh* m_pMapMesh;
	MapNode* m_pMapNode;
	MapLink* m_pMapLink;
	MapExtend* m_pMapFExtend;
	MapNode* m_pMapWNode;
	MapLink* m_pMapWLink;
	MapExtend* m_pMapWExtend;
	MapNode* m_pMapVNode;
	MapLink* m_pMapVLink;
	MapExtend* m_pMapVExtend;
	MapPolygon* m_pMapComplex;
	MapPolygon* m_pMapBuilding;
	MapTraffic* m_pMapTraffic;
	MapCourse* m_pMapCourse;
#if defined(USE_INAVI_STATIC_DATA)
	ReadTTLSpd* m_pStaticMgr;
#endif

#if defined(__USE_TEMPLETE)
	MapBase<stComplexInfo> m_MapBase;
#endif

private:
	// cache
	int RemoveCacheItem(IN const int cntItem = -1);
	int CalculateCache(unordered_map<uint32_t, FileIndex>& mapNeedMatch);

public:
	bool Initialize(void);
	void Release(void);

	bool LoadStaticData(IN const char* pszFilePath);
	void SetFileMgr(IN CFileManager* pFileMgr);

	void SetDataPath(IN const char* pszFilePath);
	const char* GetDataPath(void) const;


	// set value
	void SetDataCost(IN const uint32_t type, IN const DataCost* pCost);

	// cache
	bool AddIndexData(IN const FileIndex& pData);
	uint32_t SetCacheCount(IN const uint32_t cntCache);

	void SetMeshBox(IN const SBox* pBox);
	void SetNeighborMesh(void);
	void ArrangementMesh(void);
	bool AddMeshData(IN const stMeshInfo * pData);
	//bool AddMeshDataByNode(IN const stMeshInfo * pInfo, IN const stNodeInfo * pData);
	//bool AddMeshDataByLink(IN const stMeshInfo * pInfo, IN const stLinkInfo * pData);
	//bool AddMeshDataByBuilding(IN const stMeshInfo * pInfo, IN const stPolygonInfo * pData);
	//bool AddMeshDataByComplex(IN const stMeshInfo * pInfo, IN const stPolygonInfo * pData);
	stMeshInfo * GetMeshData(IN const uint32_t idx);

	bool AddNodeExData(IN const stExtendInfo * pData, IN const int32_t dataType = TYPE_NODE_DATA_NONE);
	bool AddNodeExData(IN const KeyID keyId, IN const int8_t type, IN const double value, IN const int32_t dataType = TYPE_NODE_DATA_NONE);
	bool AddLinkExData(IN const stExtendInfo * pData, IN const int32_t dataType = TYPE_LINK_DATA_NONE);
	bool AddLinkExData(IN const KeyID keyId, IN const int8_t type, IN const double value, IN const int32_t dataType = TYPE_LINK_DATA_NONE);

	bool AddFNodeData(IN const stNodeInfo * pData);
	bool AddFLinkData(IN const stLinkInfo * pData);
	bool AddWNodeData(IN const stNodeInfo * pData);
	bool AddWLinkData(IN const stLinkInfo * pData);
	bool AddVNodeData(IN const stNodeInfo * pData);
	bool AddVLinkData(IN const stLinkInfo * pData);
	bool AddComplexData(IN const stPolygonInfo * pData);
	bool AddBuildingData(IN const stPolygonInfo * pData);
	bool AddEntranceData(IN const KeyID keyId, IN const stEntranceInfo* pData);
	bool AddNameData(IN const stNameInfo * pData);

	// traffic
	uint8_t GetTrafficSpeed(IN const KeyID link, IN const uint8_t dir, IN OUT uint8_t& type);
	uint64_t GetTrafficId(IN const KeyID link, IN const uint8_t dir, IN const uint8_t type) const;

	// ks
	bool AddTrafficKSData(IN const uint32_t ks_id, IN const uint32_t tile_nid, IN const uint32_t link_nid, IN const uint8_t dir);
	bool UpdateTrafficKSData(IN const uint32_t ksId, IN const uint8_t speed, uint32_t timestamp);
	const unordered_map<uint32_t, stTrafficInfoKS*>* GetTrafficKSMapData(void) const;

	// ttl
	bool AddTrafficTTLData(IN const uint32_t ttl_nid, IN const uint32_t tile_nid, IN const uint32_t link_nid, IN const uint8_t dir);
	bool UpdateTrafficTTLData(IN const uint64_t ttlId, IN const uint8_t speed, uint32_t timestamp);
	const unordered_map<uint32_t, stTrafficMesh*>* GetTrafficMeshData(void);

	// static
	uint8_t GetTrafficStaticSpeed(IN const KeyID link, IN const uint8_t dir, IN const uint32_t timestamp = 0);

	bool AddCourseDataByLink(IN const uint64_t linkId, IN const uint32_t courseId);
	bool AddLinkDataByCourse(IN const uint32_t courseId, IN const uint64_t linkId);

	stNodeInfo * GetNodeDataById(IN const KeyID keyId, IN const int32_t type, IN const bool force = true);
	stLinkInfo * GetLinkDataById(IN const KeyID keyId, IN const int32_t type, IN const bool force = true);

	stMeshInfo * GetMeshDataById(IN const uint32_t id, IN const bool force = true);
	stNodeInfo * GetFNodeDataById(IN const KeyID keyId, IN const bool force = true);
	stLinkInfo * GetFLinkDataById(IN const KeyID keyId, IN const bool force = true);
	stNodeInfo * GetWNodeDataById(IN const KeyID keyId, IN const bool force = true);
	stLinkInfo * GetWLinkDataById(IN const KeyID keyId, IN const bool force = true);
	stNodeInfo * GetVNodeDataById(IN const KeyID keyId, IN const bool force = true);
	stLinkInfo * GetVLinkDataById(IN const KeyID keyId, IN const bool force = true);
	stPolygonInfo* GetBuildingDataById(IN const KeyID keyId, IN const bool force = true);
	stPolygonInfo* GetComplexDataById(IN const KeyID keyId, IN const bool force = true);
	const char* GetNameDataByIdx(IN const uint32_t idx, IN const bool force = true);

	MapExtend* GetMapExtend(void) const;
	stExtendInfo * GetExtendInfoById(IN const KeyID keyId, IN const int32_t keyType);
	double GetExtendDataById(IN const KeyID keyId, IN const int8_t type, IN const int32_t keyType);

	set<uint32_t>* GetCourseByLink(IN const uint64_t linkId);
	set<uint64_t>* GetLinkByCourse(IN const uint32_t courseId);


	const int32_t GetVLinkDataCount(void);
	const int32_t GetComplexDataCount(void);
	const int32_t GetBuildingDataCount(void);

	// mesh
	uint32_t GetMeshCount(void);
	const SBox* GetMeshRegion(IN const int32_t idx = -1) const;
	stMeshInfo * GetMeshDataByPoint(IN const double lng, IN const double lat);
	uint32_t GetMeshDataByPoint(IN const double lng, IN const double lat, IN const uint32_t cntMaxBuff, OUT stMeshInfo** pMeshInfo);
	uint32_t GetMeshDataByRegion(IN const SBox& pRegion);
	uint32_t GetMeshDataByRegion(IN const SBox& pRegion, IN const uint32_t cntMaxBuff, OUT stMeshInfo** pMeshInfo);

	bool GetNewData(IN const uint32_t idTile);
	bool GetNewData(IN const SBox& rtWorld);
	bool GetNewData(IN const double& lng, IN const double& lat);
	bool DeleteData(IN const uint64_t id);
	//// node
	//uint32_t GetNodeCount(void);

	//// link
	//uint32_t GetLinkCount(void);
	// 분기점 노드의 경로선을 제외한 나머지 링크 얻기
	const size_t GetJunctionData(IN const KeyID beforeLinkId, IN const KeyID nextLinkId, IN const uint32_t cntMaxBuff, OUT stLinkInfo** pLinkInfo);

	void SetVersion(IN const uint32_t nDataType, IN const uint32_t nMajor, IN const uint32_t nMinor, IN const uint32_t nPatch, IN const uint32_t nBuild);
	const char* GetVersionString(IN const uint32_t nDataType);

	// nMatchType : 차량 출/도착지 매칭 옵션(고속도로/터널/지하차도 등은 매칭 안되도록), 이미 매칭되어 들어오는 좌표일 경우에는 사용 안함으로
	stLinkInfo * GetLinkDataByPointAround(IN const double lng, IN const double lat, IN const int32_t nMaxDist, OUT double& retLng, OUT double& retLat, OUT double& retDist, IN const int32_t nMatchType = TYPE_LINK_MATCH_CARSTOP, IN const int32_t nLinkDataType = TYPE_LINK_DATA_NONE, OUT int32_t* pMatchVtxIdx = nullptr);
	int32_t GetLinkVertexDataByPoint(IN const double lng, IN const double lat, IN const int32_t nMaxDist, IN const KeyID linkId, OUT double& retLng, OUT double& retLat, OUT double& retDist);
	stPolygonInfo* GetPolygonDataByPoint(IN const double lng, IN const double lat, IN const int32_t nType = 0, IN const bool useNeighborMesh = true); // nType 0:입구점있는것만, 1:모두, 2:빌딩만, 3:단지만, useNeighborMesh : 이웃메쉬 확장 검색	
	stLinkInfo * GetNearRoadByPoint(IN const double lng, IN const double lat, IN const int32_t maxDist, IN const int32_t nMatchType, IN const int32_t nLinkDataType, OUT stEntryPointInfo& entInfo);
	int32_t GetOptimalPointDataByPoint(IN const double lng, IN const double lat, OUT stOptimalPointInfo* pOptInfo, IN const int32_t nEntType = 0, IN const int32_t nRetCount = 0, IN const int32_t nMatchType = TYPE_LINK_MATCH_CARSTOP, IN const int32_t nLinkDataType = TYPE_LINK_DATA_NONE, IN const int32_t nOption = 0);
	// nPolyType, 폴리곤 타입 // 0:알아서, 1:빌딩만, 2:단지만	
	stLinkInfo * GetNearLinkDataByCourseId(IN const int32_t courseId, IN const double lng, IN const double lat, IN const int32_t nMaxDist, OUT double& retLng, OUT double& retLat, OUT double& retDist);

	// nEntType, 입구점 타입 // 0:알아서, 1: 차량 입구점, 2: 택시 승하차 지점(건물), 3: 택시 승하차 지점(건물군), 4: 택배 차량 하차 거점, 5: 보행자 입구점, 	6: 배달 하차점(차량, 오토바이), 7: 배달 하차점(자전거, 도보)
	// nOption, 0: 없음, 1: 주변 가까운 도로 무조건 추가
	///////////////////////////////////////////////////////////////////////////
	
	//const char* GetErrorMessage(void);
};

