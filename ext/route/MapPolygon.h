#pragma once

#include "MapBase.h"

class MapPolygon : public MapBase
{
public:
	MapPolygon();
	~MapPolygon();

	virtual bool Initialize(void);
	virtual void Release(void);	
	
	virtual const uint32_t GetCount(void) const;

	bool AddData(IN const stPolygonInfo* pData);
	bool DeleteData(IN const KeyID keyId);
	stPolygonInfo* GetDataById(IN const KeyID keyId);
	
	stPolygonInfo* GetPitInPolygon(IN const KeyID keyId, IN const double lng, IN const double lat, IN const int32_t nMaxDist = 0);

private:
	map<uint64_t, stPolygonInfo*> mapData;

};


bool isPointInPolygon(const double x, const double y, const SPoint *pptPolygon, const int32_t nPolygon);
bool isPointInPolygon(SPoint *ppt, SPoint *pptPolygon, int32_t nPolygon);