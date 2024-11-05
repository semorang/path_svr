#pragma once

#include "MapBase.h"

struct stExtendMeshInfo
{
	unordered_map<uint64_t, stExtendInfo*> mapNode;
	unordered_map<uint64_t, stExtendInfo*> mapLink;
};


class MapExtend : public MapBase
{
public:
	MapExtend();
	~MapExtend();

	virtual bool Initialize(void);
	virtual void Release(void);

	virtual const uint32_t GetCount(void) const;

	const unordered_map<uint32_t, stExtendMeshInfo>* GetMapData(void); // 가능하면 쓰지 말자
	const stExtendMeshInfo* GetMapMeshData(IN const uint32_t tile_id);

	bool AddData(IN const stExtendInfo* pData, IN const int32_t nKeyType); // nDataType 0:None, 1:node, 2:link
	bool AddData(IN const KeyID keyId, IN const int8_t type, IN const double value, IN const int32_t nKeyType); // nDataType 0:None, 1:node, 2:link

	stExtendInfo* GetExtendById(IN const KeyID keyId, IN const int32_t nKeyType); // nDataType 0:None, 1:node, 2:link
	double GetExtendDataById(IN const KeyID keyId, IN const int8_t type, IN const int32_t nKeyType); // nDataType 0:None, 1:node, 2:link

private:
	unordered_map<uint32_t, stExtendMeshInfo> m_mapData;
};

