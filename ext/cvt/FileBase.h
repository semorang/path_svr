#pragma once

#include "../include/MapDef.h"

#if defined(__USE_TEMPLETE)
#include "MapBase.h"
#endif


#if defined(USE_OPTIMAL_POINT_API)
// 1.0.1 -> stEntranceInfo 구조체 필드 64bit 증가
// 1.0.2 -> link/node type 변경, // 0:미정의, 1:숲길, 2:보행자, 3:자전거, 4:차량 -> 0:미정의, 1:숲길, 2:보행자/자전거, 3:차량
// 1.0.2 -> poly type 변경, // 0:빌딩, 1 : 단지, 2 : 산바운더리 -> 0:미정, 1:빌딩, 2:단지, 3:산바운더리
// 1.0.3 -> stEntranceInfo에 angle 추가
// 1.0.4 -> 폴리곤 parts offset 데이터 사이즈 변경 uint8_t --> uint16_t
// 1.0.5 -> 파일 명칭 및 확장자 변경
// 1.0.6 -> add entry restriction, 진출입 제한 속성 추가
#	define FILE_VERSION_MAJOR	1
#	define FILE_VERSION_MINOR	0
#	define FILE_VERSION_PATCH	6
#elif defined(USE_ROUTING_POINT_API)
#	if defined(USE_FOREST_DATA)
// 1.0.3 -> 숲길 네트워크에 그룹ID 추가, 각 지점간 동일 산(동일 네트워크 연결) 여부 확인 용
// 1.0.4 -> 정/역 확장 매니저 따로 관리하도록 기능 추가
// 1.0.5 -> 다중 메쉬 데이터 적용 (검색위해 단일 메쉬 -> 다중 메쉬 사용)
// 1.0.6 -> 폴리곤 parts offset 데이터 사이즈 변경 uint8_t --> uint16_t
// 1.0.7 -> 인기도 등급 적용, uint32_t:12 --> uint32_t:4
// 1.0.8 -> 파일 명칭 및 확장자 변경
// 1.0.9 -> body size 0 데이터 저장 하지 않도록 함
#	define FILE_VERSION_MAJOR	1
#	define FILE_VERSION_MINOR	0
#	define FILE_VERSION_PATCH	9
#	elif defined(USE_PEDESTRIAN_DATA)
// 1.0.1 -> 폴리곤 parts offset 데이터 사이즈 변경 uint8_t --> uint16_t
// 1.0.2 -> 노드 명칭 정보 유무 추가
// 1.0.3 -> 파일 명칭 및 확장자 변경
// 1.0.4 -> body size 0 데이터 저장 하지 않도록 함
#	define FILE_VERSION_MAJOR	1
#	define FILE_VERSION_MINOR	0
#	define FILE_VERSION_PATCH	4
#	elif defined(USE_VEHICLE_DATA)
// 0.0.1 -> 폴리곤 parts offset 데이터 사이즈 변경 uint8_t --> uint16_t
// 0.0.2 -> TTL_ID를 포함한 차량 네트워크 V1.0.2 20240401
// 0.0.3 -> 파일 명칭 및 확장자 변경
// 0.0.4 -> 교통속도 데이터를 링크에 속성추가하여 바로 적용 : match with engine 0.0.10
// 0.0.5 -> body size 0 데이터 저장 하지 않도록 함
#	define FILE_VERSION_MAJOR	0
#	define FILE_VERSION_MINOR	0
#	define FILE_VERSION_PATCH	5
#	endif
#else
#	define FILE_VERSION_MAJOR	0
#	define FILE_VERSION_MINOR	0
#	define FILE_VERSION_PATCH	1
#endif

#define NULL_VALUE	uint32_t(-1)

#if defined(_DEBUG)
static const uint32_t g_cntLogPrint = 100000;
#else
static const uint32_t g_cntLogPrint = 1000000;
#endif

struct stMesh
{
	uint32_t MeshID;
	SBox meshBox;
};

// 파일 명
static char g_szTypeTitle[TYPE_DATA_COUNT][16] = {
	{ "nope" }, //TYPE_DATA_NONE, // 미지정
	{ "name" }, //TYPE_DATA_NAME, // 명칭
	{ "mesh" }, //TYPE_DATA_MESH, // 메쉬
	{ "forest" }, //TYPE_DATA_TREKKING, // 숲길
	{ "pedestrian" }, //TYPE_DATA_PEDESTRIAN, // 보행자/자전거
	{ "vehicle" }, //TYPE_DATA_CAR, // 자동차
	{ "building" }, //TYPE_DATA_BUILDING, // 건물
	{ "complex" }, //TYPE_DATA_COMPLEX, // 단지
	{ "entrance" }, //TYPE_DATA_ENTRANCE, // 입구점
	{ "traffic" }, //TYPE_DATA_TRAFFIC, // 교통정보
	{ "mountain" }, //TYPE_DATA_MOUNTAIN, // 산바운더리
	{ "course" }, //TYPE_DATA_COURSE, // 코스정보
	{ "extend" }, //TYPE_DATA_EXTEND, // 확장정보
};

// 파일 타입
static char g_szTypeName[TYPE_EXEC_COUNT][4] = {
	{ "NOP" }, // TYPE_EXEC_NONE = 0, // 미정의
	{ "STR" }, // TYPE_EXEC_NAME, // 명칭
	{ "MSH" }, // TYPE_EXEC_MESH, // 메쉬
	{ "LNK" }, // TYPE_EXEC_LINK, // 링크
	{ "NOD" }, // TYPE_EXEC_NODE, // 노드
	{ "NET" }, // TYPE_EXEC_NETWORK, // 네트워크
	{ "POL" }, // TYPE_EXEC_POLYGON, // 폴리곤
	{ "ENT" }, // TYPE_EXEC_ENTRANCE, // 입구점
	{ "TRF" }, // TYPE_EXEC_TRAFFIC, // 교통정보
	{ "COS" }, // TYPE_EXEC_COURSE, // 코스정보
	{ "IDX" }, // TYPE_EXEC_INDEX, // 인덱스정보
	{ "EXT" }, // TYPE_EXEC_EXTEND, // 확장정보
};

// 파일 확장자
static char g_szTypeExec[TYPE_EXEC_COUNT][4] = {
	{ "nop" }, // TYPE_EXEC_NONE = 0, // 미정의
	{ "str" }, // TYPE_EXEC_NAME, // 명칭
	{ "msh" }, // TYPE_EXEC_MESH, // 메쉬
	{ "lnk" }, // TYPE_EXEC_LINK, // 링크
	{ "nod" }, // TYPE_EXEC_NODE, // 노드
	{ "net" }, // TYPE_EXEC_NETWORK, // 네트워크
	{ "pol" }, // TYPE_EXEC_POLYGON, // 폴리곤
	{ "ent" }, // TYPE_EXEC_ENTRANCE, // 입구점
	{ "trf" }, // TYPE_EXEC_TRAFFIC, // 교통정보
	{ "cos" }, // TYPE_EXEC_COURSE, // 코스정보
	{ "idx" }, // TYPE_EXEC_INDEX, // 인덱스정보
	{ "ext" }, // TYPE_EXEC_EXTEND, // 확장정보
};
#pragma pack (push, 1)

// 기본 정보
typedef struct _tagFileBase {
	char szType[4];		// 데이터 타입
	uint32_t version[4]; // version
	uint32_t szHeader;	// 헤더파일 사이즈
}FileBase;

// 데이터 정보 헤더
typedef struct _tagFileHeader {
	SBox	rtMap;		// 맵 rect
	uint32_t cntIndex;	// 타일 인덱스 갯수
	uint32_t offIndex;	// 인덱스 시작 offset
	uint32_t offBody;	// 데이터 시작 offset
}FileHeader;

// 인덱스 정보
typedef struct _tagFileIndex {
	uint32_t idxTile;	// 타일 idx
	uint32_t idTile;	// 타일 id
	uint32_t idNeighborTile[8]; // 이웃 타일 id
	SBox	rtTile;		// 타일 rect
	SBox	rtData;		// 데이터 rect
	uint32_t szBody;	// 데이터 사이즈
	uint32_t offBody;	// 데이터 offset
}FileIndex;

// 데이터 정보
typedef struct _tagFileBody {
	uint32_t idTile;	// 타일 id
	uint32_t szData;	// 데이터 blob 사이즈 - node, link, vertex
	union {
		uint64_t cntData;
		struct {
			uint32_t cntNode;	// 노드 갯수
			uint32_t cntLink;	// 링크 갯수
		} net;
		struct {
			uint32_t cntPolygon; // 폴리곤 갯수
			uint32_t reserved; // 예약
		} area;
		struct {
			uint32_t cntComplex;	// 단지 갯수
			uint32_t cntBuilding;	// 빌딩 갯수
		} ent;
		struct
		{
			uint32_t cntTraffic;	// 교통정보 갯수
			uint32_t cntMatching;	// 매칭 갯수
		} traffic;
	};
}FileBody;

typedef struct _tagFileNode {
	KeyID node_id;
	KeyID edgenode_id;
	SPoint coord;
	KeyID connnodes[8];
	uint16_t conn_attr[8];
	uint32_t sub_info;
}FileNode;

typedef struct _tagFileLink {
	KeyID link_id;
	KeyID snode_id;
	KeyID enode_id;
	double length;
	uint32_t name_idx;
	uint32_t cntVertex;
	uint64_t sub_info;
#if defined(USE_P2P_DATA) || defined(USE_MOUNTAIN_DATA)
	uint64_t sub_ext; // HD 매칭 ID
#endif
}FileLink;

typedef struct _tagFilePolygon {
	KeyID polygon_id; // 폴리곤 id
	SBox rtBox;	// 폴리곤 영역
	uint32_t parts : 16;
	uint32_t points : 16;
	uint32_t links : 16; // 폴리곤에 매칭되는 링크수
	uint32_t meshes : 16; // 중첩 메쉬수
	uint64_t sub_info;
}FilePolygon;

typedef struct _tagFileEntrance {
	KeyID polygon_id; // 폴리곤 id
	uint32_t cntEntrance;
}FileEntrance;

typedef struct _tagNameDicIndex {
	uint32_t idxName;	// 명칭 idx
	uint32_t szBody;	// 데이터 사이즈
	uint32_t offBody;	// 데이터 offset
}NameDicIndex;

#pragma pack (pop)


void boxMerge(IN OUT SBox& lhs, IN const SBox& rhs);
#if defined(USE_P2P_DATA)
int linkMerge(IN OUT vector<SPoint>& lhs, IN const vector<SPoint>& rhs, IN const bool isAllowDuplicates = true);
int linkMerge(IN OUT vector<SPoint>& lhs, IN const SPoint * pData, IN const uint32_t cntData, IN const bool isAllowDuplicates = true);
#else
int linkMerge(IN OUT vector<SPoint>& lhs, IN const vector<SPoint>& rhs, IN const bool isAllowDuplicates = false);
int linkMerge(IN OUT vector<SPoint>& lhs, IN const SPoint * pData, IN const uint32_t cntData, IN const bool isAllowDuplicates = false);
#endif
int32_t linkProjection(IN stLinkInfo* pData, IN const double& lng, IN const double& lat, IN const int32_t nMaxDist, OUT double& retLng, OUT double& retLat, OUT double& retDist, IN OUT double& retIr);

class CDataManager;
class CFileName;

class CFileBase
{
public:
	CFileBase();
	virtual ~CFileBase();

private:

protected:
	// 전체 지도 영역
	uint32_t m_nLinkIdx;
	uint32_t m_nNodeIdx;

	map<uint32_t, stMeshInfo*> m_mapMesh;
	unordered_map<uint64_t, uint32_t> m_mapNodeIndex; // 실제 노드 ID에 대응하는 노드 IDX 관리
	unordered_map<uint64_t, stNodeInfo*> m_mapNode;
	unordered_map<uint64_t, stLinkInfo*> m_mapLink;

	string m_strErrMsg;

	// 전체 지도 영역
	SBox m_rtBox;


	uint32_t m_nDataType; // TYPE_DATA
	uint32_t m_nFileType; // TYPE_DATA

	char m_szDataPath[MAX_PATH];

	char m_szSrcPath[MAX_PATH];
	char m_szWorkPath[MAX_PATH];
	char m_szDstPath[MAX_PATH];
	
	FileHeader m_fileHeader;
	std::vector<FileIndex> m_vtIndex;
	
	CDataManager* m_pDataMgr;
	CFileName* m_pNameMgr;

#if defined(__USE_TEMPLETE)
	MapBase<stComplexInfo> m_MapBase;
#endif
private:

protected:
	void SetMeshBox(IN const SBox* pBox);
	void SetNeighborMesh(void);

	stMeshInfo * GetMeshData(IN const uint32_t idx);
	stMeshInfo * GetMeshDataById(IN const uint32_t id);

	bool AddMeshData(IN const stMeshInfo * pData);
	//bool AddMeshDataByNode(IN const stMeshInfo * pInfo, IN const stNodeInfo * pData);
	//bool AddMeshDataByLink(IN const stMeshInfo * pInfo, IN const stLinkInfo * pData);
	//bool AddMeshDataByPolygon(IN const stMeshInfo * pInfo, IN const stPolygonInfo * pData);

	bool AddNameData(IN const stNameInfo * pData);
	bool AddNodeData(IN const stNodeInfo * pData);
	bool AddLinkData(IN const stLinkInfo * pData);
	bool AddPolygonData(IN const stPolygonInfo * pData);
	bool AddEntranceData(IN const KeyID keyId, IN const stEntranceInfo * pData);

	//stLinkInfo * GetLinkDataBySENode(IN const KeyID idx1, IN const KeyID idx2);
	//stLinkIDX GetConnectedLinkByLink(stLinkIDX _Link);
	//stLinkIDX* GetLinkInMesh(uint32_t meshID, uint32_t sNode, uint32_t eNode);

	bool MergeEdgePoint(IN const int nLinkStartEnd, IN stLinkInfo* pLink, IN unordered_map<uint64_t, stLinkInfo*>* pMapLink, IN unordered_map<uint64_t, stNodeInfo*>* pMapNode);
	void MergeLink(IN stLinkInfo* pLink, IN unordered_map<uint64_t, stLinkInfo*>* pMapLink, IN unordered_map<uint64_t, stNodeInfo*>* pMapNode);
	//bool FindStartEnd_INDEX(int netType, uint32_t meshID, int &sIndex, int &eIndex);
	//stLinkIDX CalculateShortestDistance(std::vector<stLinkIDX> &vGetConnectedLinks);

	// Dictionary
	uint32_t AddNameDic(IN char* pData);

	const char* GetErrorMsg();

	bool CheckDataInMesh(IN const double x, IN const double y);

public:
	virtual bool Initialize();
	virtual void Release();

	//bool GetData(IN const uint32_t idTile);
	//bool GetData(IN const SBox& rtWorld);
	//bool GetData(IN const double& lng, IN const double& lat);

	const uint32_t GetDataType(void) const;
	char* GetDataTypeName(void) const;

	void SetDataManager(IN CDataManager* pDataMgr);
	void SetNameManager(IN CFileName* pNameMgr);
	void SetMeshRegion(IN const SBox* pRegion);

	// mesh
	uint32_t GetMeshCount(void);
	const SBox* GetMeshRegion(IN const int32_t idx = -1) const;

	virtual bool ParseData(IN const char* fname);
	virtual bool GenServiceData();
	virtual void AddDataFeild(IN const int idx, IN const int type, IN const char* colData);
	virtual void AddDataRecord();

	///////////////////////////////////////////////////////////////////////////
	//virtual bool OpenFile(IN const vector<string>* pvtFilePath);
	virtual bool SetPath(IN const char* szSrcPath, IN const char* szWorkPath, IN const char* szDstPath);
	virtual bool OpenFile(IN const char* szFilePath);
	virtual bool SaveData(IN const char* szFilePath);

	virtual size_t WriteBase(FILE* fp);
	virtual size_t WriteHeader(FILE* fp);
	virtual size_t WriteIndex(FILE* fp);
	virtual size_t WriteBody(FILE* fp, IN const uint32_t fileOff);

	///////////////////////////////////////////////////////////////////////////
	virtual bool LoadData(IN const char* szFilePath);
	virtual bool LoadDataByIdx(IN const uint32_t idx);

	virtual size_t ReadBase(FILE* fp);
	virtual size_t ReadHeader(FILE* fp);
	virtual size_t ReadIndex(FILE* fp);
	virtual size_t ReadBody(FILE* fp);
};

