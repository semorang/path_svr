#pragma once

#include "MapBase.h"

class MapNode : public MapBase
{
public:
	MapNode();
	~MapNode();

	virtual bool Initialize(void);
	virtual void Release(void);

	virtual const uint32_t GetCount(void) const;

	int AddData(IN const stNodeInfo* pData);
	bool DeleteData(IN const KeyID keyId);
	stNodeInfo* GetNodeById(IN const KeyID keyId);

private:
	unordered_map<uint64_t, stNodeInfo*> mapData;

};

