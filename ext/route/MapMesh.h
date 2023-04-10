#pragma once

#include "MapBase.h"

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

	int InsertNode(IN const KeyID keyId);
	int InsertLink(IN const stLinkInfo * pData);
#if defined(USE_PEDESTRIAN_DATA)
	int InsertWNode(IN const KeyID keyId);
	int InsertWLink(IN const stLinkInfo * pData);
#endif
#if defined(USE_VEHICLE_DATA)
	int InsertVNode(IN const KeyID keyId);
	int InsertVLink(IN const stLinkInfo * pData);
#endif
# if defined(USE_OPTIMAL_POINT_API)
	bool InsertComplex(IN const stPolygonInfo * pData);
	bool InsertBuilding(IN const stPolygonInfo * pData);
#endif

	stMeshInfo* GetMeshById(IN const int32_t id);
	stMeshInfo* GetMeshByPoint(IN const double lng, IN const double lat);
	int GetPitInMesh(IN const double lng, IN const double lat, IN const int nMaxBuff, OUT stMeshInfo** pData);
	int GetPitInData(IN const double lng, IN const double lat, IN const int nMaxBuff, OUT stMeshInfo** pData);
	
	stMeshInfo * GetMeshData(IN const uint32_t idx);
	void ExtendDataBox(IN stMeshInfo * pData, IN const double lng, IN const double lat) const;
	void ExtendDataBox(IN stMeshInfo * pData, IN const SPoint& coord) const;
	void CheckNeighborMesh(void);
	void ArrangementMesh(void);

private:
	map<uint64_t, stMeshInfo*> mapData;

	SBox m_rtBox;
};

