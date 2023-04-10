#pragma once

#include "MapBase.h"

class MapLink : public MapBase
{
public:
	MapLink();
	~MapLink();

	virtual bool Initialize(void);
	virtual void Release(void);

	virtual const uint32_t GetCount(void) const;

	int AddData(IN const stLinkInfo* pData);
	bool DeleteData(IN const KeyID keyId);

	stLinkInfo* GetLinkById(IN const KeyID keyId);
	stLinkInfo* GetLinkBySENode(IN const KeyID idx1, IN const KeyID idx2);
	
private:
	map<uint64_t, stLinkInfo*> mapData;
};

