#pragma once

#include "MapBase.h"

class MapName : public MapBase
{
public:
	MapName();
	~MapName();

	virtual bool Initialize(void);
	virtual void Release(void);

	virtual const uint32_t GetCount(void) const;

	bool AddData(IN const stNameInfo* pData);
	bool DeleteData(IN const KeyID keyId);
	const char* GetNameData(IN const uint32_t idx) const;

private:
	map<uint32_t, stNameInfo*> mapData;
};

