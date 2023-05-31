#pragma once

#include "../route/MapDef.h"
#include "../cvt/FileManager.h"


#include "MapName.h"
#include "MapMesh.h"
#include "MapNode.h"
#include "MapLink.h"
#include "MapPolygon.h"
#include "MapTraffic.h"

#if defined(__USE_TEMPLETE)
#include "MapBase.h"
#endif

//#define USE_DATA_CACHE // 파일 일부만 읽어 메모리에 캐싱하자
#define MAX_MESH_COUNT  10000 // 현재 원도의 최대 메쉬 카운트
#define MAX_CASH_COUNT	300 // 우선 테스트로 n개만 해서 상태 확인 후 확장


#pragma pack (push, 1)

// 입구점 정보
struct stEntryPointInfo {
	int32_t nAttribute; // 속성, 1: 차량 입구점, 2: 택시 승하차 지점(건물), 3: 택시 승하차 지점(건물군), 4: 택배 차량 하차 거점, 5: 보행자 입구점, 	6: 배달 하차점(차량, 오토바이), 7: 배달 하차점(자전거, 도보)
	double dwDist; // 요청 지점으로부터 거리(m)
	double x;
	double y;
};

// 최적 지점 결과 정보
struct stOptimalPointInfo {
	int32_t nType;	// 매칭 타입, 0:일반도로(미매칭), 1:빌딩입구점, 2:단지입구점, 3:단지내도로//, 4:최근접도로
	double x;
	double y;
	string name;
	vector<stEntryPointInfo> vtEntryPoint;
	vector<SPoint> vtPolygon;
};

// 입구점 요청 타입
struct stReqEntryType {
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

#pragma pack (pop)


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

protected:
	char m_szDataPath[MAX_PATH];
	
	std::unordered_map<uint32_t, FileIndex> m_mapFileIndex;

	MapName* m_pMapName;
	MapMesh* m_pMapMesh;
	MapNode* m_pMapNode;
	MapLink* m_pMapLink;
	MapNode* m_pMapWNode;
	MapLink* m_pMapWLink;
	MapNode* m_pMapVNode;
	MapLink* m_pMapVLink;
	MapPolygon* m_pMapComplex;
	MapPolygon* m_pMapBuilding;
	MapTraffic* m_pMapTraffic;


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
	void SetFileMgr(IN CFileManager* pFileMgr);

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

	bool AddNodeData(IN const stNodeInfo * pData);
	bool AddLinkData(IN const stLinkInfo * pData);
	bool AddWNodeData(IN const stNodeInfo * pData);
	bool AddWLinkData(IN const stLinkInfo * pData);
	bool AddVNodeData(IN const stNodeInfo * pData);
	bool AddVLinkData(IN const stLinkInfo * pData);
	bool AddComplexData(IN const stPolygonInfo * pData);
	bool AddBuildingData(IN const stPolygonInfo * pData);
	bool AddEntranceData(IN const KeyID keyId, IN const stEntranceInfo* pData);
	bool AddNameData(IN const stNameInfo * pData);
	bool AddTrafficData(IN stTrafficInfo* pData);
	bool AddTrafficMatchData(IN const stTrafficKey& key, IN const uint64_t ksId);
	bool AddTrafficLinkData(IN const stLinkInfo* pData);

	stMeshInfo * GetMeshDataById(IN const uint32_t id, IN const bool force = true);
	stNodeInfo * GetNodeDataById(IN const KeyID keyId, IN const bool force = true);
	stLinkInfo * GetLinkDataById(IN const KeyID keyId, IN const bool force = true);
	stNodeInfo * GetWNodeDataById(IN const KeyID keyId, IN const bool force = true);
	stLinkInfo * GetWLinkDataById(IN const KeyID keyId, IN const bool force = true);
	stNodeInfo * GetVNodeDataById(IN const KeyID keyId, IN const bool force = true);
	stLinkInfo * GetVLinkDataById(IN const KeyID keyId, IN const bool force = true);
	stPolygonInfo* GetBuildingDataById(IN const KeyID keyId, IN const bool force = true);
	stPolygonInfo* GetComplexDataById(IN const KeyID keyId, IN const bool force = true);
	const char* GetNameDataByIdx(IN const uint32_t idx, IN const bool force = true);
	const unordered_map<uint32_t, stTrafficInfo*>* GetTrafficMapData(void);
	const stTrafficInfo* GetTrafficInfo(IN const uint32_t ksId);

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
	stLinkInfo * GetLinkDataByPointAround(IN const double lng, IN const double lat, IN const int32_t nMaxDist, OUT double& retLng, OUT double& retLat, OUT double& retDist, IN const int32_t nMatchType = TYPE_LINK_MATCH_CARSTOP, OUT int32_t* pMatchVtxIdx = nullptr);
	int32_t GetLinkVertexDataByPoint(IN const double lng, IN const double lat, IN const int32_t nMaxDist, IN const KeyID linkId, OUT double& retLng, OUT double& retLat, OUT double& retDist);
	stPolygonInfo* GetPolygonDataByPoint(IN OUT double& lng, IN OUT double& lat, IN const int32_t nType = 0); // 0:알아서, 1:빌딩만, 2:단지만
	
	bool GetNearRoadByPoint(IN const double lng, IN const double lat, IN const int32_t maxDist, IN const int32_t matchType, OUT stEntryPointInfo& entInfo);
	int32_t GetOptimalPointDataByPoint(IN const double lng, IN const double lat, OUT stOptimalPointInfo* pOptInfo, IN const int32_t nEntType = 0, IN const int32_t nRetCount = 0, IN const int32_t nMatchType = TYPE_LINK_MATCH_CARSTOP);
	// nPolyType, 폴리곤 타입 // 0:알아서, 1:빌딩만, 2:단지만																																																		  
	// nEntType, 입구점 타입 // 0:알아서, 1: 차량 입구점, 2: 택시 승하차 지점(건물), 3: 택시 승하차 지점(건물군), 4: 택배 차량 하차 거점, 5: 보행자 입구점, 	6: 배달 하차점(차량, 오토바이), 7: 배달 하차점(자전거, 도보)
	///////////////////////////////////////////////////////////////////////////
	
	const char* GetErrorMessage(void);
};

