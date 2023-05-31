#pragma once

#include "../route/MapDef.h"

#if defined(__USE_TEMPLETE)
#include "MapBase.h"
#endif


#define FILE_VERSION_MAJOR	1
#define FILE_VERSION_MINOR	0
#define FILE_VERSION_PATCH	0

#if 1//defined(_DEBUG)
//#define _USE_TEST_MESH
static const array<int, 25 > g_arrTestMesh = { 
	185308, 185405, 185406, 185313, 185410, 185411, 185318, 185415, 185416, 186303, 186400, 186401, 186308, 186405, 186406, // 서울
	186410, 186411, 186415, 186416, // 하남
	186306, 186307 , 186308, 186311, 186312, 186313, // 성남
	};
#endif

#define NULL_VALUE	uint32_t(-1)

struct stMesh
{
	uint32_t MeshID;
	SBox meshBox;
};

// 파일 타입
static char g_szTypeName[TYPE_DATA_COUNT][4] = {
	{ "DIC" }, //TYPE_DATA_NAME, // 명칭사전
	{ "MES" }, //TYPE_DATA_MESH, // 메쉬
	{ "TRK" }, //TYPE_DATA_TREKKING, // 숲길
	{ "PED" }, //TYPE_DATA_PEDESTRIAN, // 보행자/자전거
	{ "CAR" }, //TYPE_DATA_CAR, // 자동차
	{ "BLD" }, //TYPE_DATA_BUILDING, // 건물
	{ "CPX" }, //TYPE_DATA_COMPLEX, // 단지
	{ "ENT" }, //TYPE_DATA_ENTRANCE, // 입구점
	{ "RTT" }, //TYPE_DATA_TRAFFIC, // 교통정보
};

// 파일 명
static char g_szTypeTitle[TYPE_DATA_COUNT][16] = {
	{ "name" }, //TYPE_DATA_NAME, // 명칭사전
	{ "mesh" }, //TYPE_DATA_MESH, // 메쉬
	{ "trekking" }, //TYPE_DATA_TREKKING, // 숲길
	{ "pedestrian" }, //TYPE_DATA_PEDESTRIAN, // 보행자/자전거
	{ "vehicle" }, //TYPE_DATA_CAR, // 자동차
	{ "building" }, //TYPE_DATA_BUILDING, // 건물
	{ "complex" }, //TYPE_DATA_COMPLEX, // 단지
	{ "entrance" }, //TYPE_DATA_ENTRANCE, // 입구점
	{ "traffic" }, //TYPE_DATA_TRAFFIC, // 교통정보
};

// 파일 확장자
static char g_szTypeExec[TYPE_DATA_COUNT][4] = {
	{ "gmd" }, //TYPE_DATA_NAME, // 명칭사전
	{ "gmt" }, //TYPE_DATA_MESH, // 메쉬
	{ "gml" }, //TYPE_DATA_TREKKING, // 숲길
	{ "gml" }, //TYPE_DATA_PEDESTRIAN, // 보행자/자전거
	{ "gml" }, //TYPE_DATA_CAR, // 자동차
	{ "gmp" }, //TYPE_DATA_BUILDING, // 건물
	{ "gmp" }, //TYPE_DATA_COMPLEX, // 단지
	{ "gmp" }, //TYPE_DATA_ENTRANCE, // 입구점
	{ "gmr" }, //TYPE_DATA_TRAFFIC, // 교통정보
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
		struct {
			uint32_t cntNode;	// 노드 갯수
			uint32_t cntLink;	// 링크 갯수
		} link;
		struct {
			uint32_t cntPolygon; // 폴리곤 갯수
			uint32_t reserved; // 예약
		} area;
		struct {
			uint32_t cntComplex;	// 단지 갯수
			uint32_t cntBuilding;	// 빌딩 갯수
		} ent;
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
#if defined(USE_P2P_DATA)
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

char* strsep(char** stringp, const char* delim);
char* trim(char *line);

void boxMerge(IN OUT SBox& lhs, IN const SBox& rhs);
size_t linkMerge(IN OUT vector<SPoint>& lhs, IN const vector<SPoint>& rhs);
size_t linkMerge(IN OUT vector<SPoint>& lhs, IN const SPoint * pData, IN const uint32_t cntData);
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


	uint32_t m_nFileType; // TYPE_DATA

	char m_szDataPath[MAX_PATH];
	
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

