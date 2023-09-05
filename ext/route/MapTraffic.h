#pragma once

#include "MapBase.h"


#define SPEED_NOT_AVALABLE	0xFF // 255

struct stTrafficMesh {
	uint32_t mesh;
	unordered_map<uint64_t, uint64_t> mapMatch;
};

class MapTraffic : public MapBase
{
public:
	MapTraffic();
	~MapTraffic();

	virtual bool Initialize(void);
	virtual void Release(void);

	virtual const uint32_t GetCount(void) const;

	int AddData(IN stTrafficInfo* pData); // add traffic
	int AddData(IN const stTrafficKey& key, IN const uint32_t ksId); // add traffic link match
	int AddData(IN const stLinkInfo* pData); // add data to matching table 
	bool DeleteData(IN const uint64_t keyId);

	const uint32_t GetSpeed(IN const KeyID link);
	const stTrafficInfo* GetTrafficInfo(IN const uint32_t ksId);
	const unordered_map<uint32_t, stTrafficInfo*>* GetTrafficMapData(void);
	
private:
	unordered_map<uint32_t, stTrafficInfo*> mapTraffic;
	unordered_map<uint32_t, stTrafficMesh*> mapData;
};

