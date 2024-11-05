#pragma once

#include "MapBase.h"


static const int32_t g_forestMeshId = 100000000; // 단일 메쉬, 30비트 내 값 설정 필요, -> 보행자길 위에 표시하기 위해 1 -> 큰 수로 변경 2024-02-22


class MapMesh : public MapBase
{
public:
	MapMesh();
	~MapMesh();

	virtual bool Initialize(void);
	virtual void Release(void);

	virtual const uint32_t GetCount(void) const;

	void SetBox(IN const SBox* pBox);
	int AddData(IN const stMeshInfo* pData);
	bool DeleteData(IN const uint64_t keyId);

#if defined(USE_FOREST_DATA)
	int InsertFNode(IN const KeyID keyId);
	int InsertFLink(IN const stLinkInfo * pData);
#endif
#if defined(USE_PEDESTRIAN_DATA)
	int InsertWNode(IN const KeyID keyId);
	int InsertWLink(IN const stLinkInfo * pData);
#endif
#if defined(USE_VEHICLE_DATA)
	int InsertVNode(IN const KeyID keyId);
	int InsertVLink(IN const stLinkInfo * pData);
#endif
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
	bool InsertComplex(IN const stPolygonInfo * pData);
	bool InsertBuilding(IN const stPolygonInfo * pData);
#endif

	stMeshInfo* GetMeshById(IN const int32_t id);
	stMeshInfo* GetMeshByPoint(IN const double lng, IN const double lat);
	int GetPitInMesh(IN const double lng, IN const double lat, IN const int nMaxBuff, OUT stMeshInfo** pData);
	int GetPitInData(IN const double lng, IN const double lat, IN const int nMaxBuff, OUT stMeshInfo** pData);
	uint32_t GetPitInRegion(IN const SBox& pRegion, IN const uint32_t cntMaxBuff, OUT stMeshInfo** pMeshInfo);

	stMeshInfo * GetMeshData(IN const uint32_t idx);
	void ExtendDataBox(IN stMeshInfo * pData, IN const double lng, IN const double lat) const;
	void ExtendDataBox(IN stMeshInfo * pData, IN const SPoint& coord) const;
	void CheckNeighborMesh(void);
	void ArrangementMesh(void);

private:
	map<uint64_t, stMeshInfo*> mapData;

	SBox m_rtBox;
};

