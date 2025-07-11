#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "../utils/GeoTools.h"
#include "MapPolygon.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


MapPolygon::MapPolygon()
{
}


MapPolygon::~MapPolygon()
{
	Release();
}


bool MapPolygon::Initialize(void)
{
	return MapBase::Initialize();
}


void MapPolygon::Release(void)
{
	if (!mapData.empty())
	{
		for (unordered_map<uint64_t, stPolygonInfo*>::iterator it = mapData.begin(); it != mapData.end(); it++)
		{
			delete it->second;
			it->second = nullptr;
		}

		mapData.clear();
		unordered_map<uint64_t, stPolygonInfo*>().swap(mapData);
	}

	MapBase::Release();
}


bool MapPolygon::AddData(IN const stPolygonInfo * pData)
{
	if (pData != nullptr /*&& pData->info != NOT_USE*/) 
	{
		mapData.emplace(pData->poly_id.llid, const_cast<stPolygonInfo*>(pData));
		return true;
	}

	return false;
}


bool MapPolygon::DeleteData(IN const KeyID keyId)
{
	unordered_map<uint64_t, stPolygonInfo*>::iterator it = mapData.find(keyId.llid);
	if (it != mapData.end()) {
		delete it->second;
		it->second = nullptr;
		mapData.erase(keyId.llid);
		return true;
	}

	return false;
}


stPolygonInfo * MapPolygon::GetDataById(IN const KeyID keyId)
{
	unordered_map<uint64_t, stPolygonInfo*>::iterator it = mapData.find(keyId.llid);
	if (it != mapData.end()) {
		return it->second;
	}
	
	return nullptr;
}


const uint32_t MapPolygon::GetCount(void) const
{
	return static_cast<uint32_t>(mapData.size());
}


stPolygonInfo * MapPolygon::GetPitInPolygon(IN const KeyID keyId, IN const double lng, IN const double lat, IN const int32_t nMaxDist)
{
	bool isIn = false;
	stPolygonInfo* pInfo = GetDataById(keyId);
	//if (pInfo != nullptr && !pInfo->vtVtx.empty()) {
	if (pInfo != nullptr && pInfo->getAttributeCount(TYPE_POLYGON_DATA_ATTR_VTX)) {
		if (nMaxDist >= 0) {
			double dwDist = nMaxDist / 100000.;
			SBox boxCheck = { lng - dwDist, lat - dwDist, lng + dwDist, lat + dwDist };
			if (!isInPitBox(boxCheck, pInfo->data_box)) {
				return nullptr;
			}
		}

		SPoint pt = { lng, lat };
		//uint32_t cntVtx = static_cast<uint32_t>(pInfo->vtVtx.size());
		uint32_t cntVtx = pInfo->getAttributeCount(TYPE_POLYGON_DATA_ATTR_VTX);
		uint32_t offPolygon = 0;
		//uint32_t cntPolygon = static_cast<uint32_t>(pInfo->vtVtx.size());
		uint32_t cntPolygon = cntVtx;
		const SPoint* pPolygon = pInfo->getAttributeVertex();
		//for (vector<uint32_t>::const_reverse_iterator itParts = pInfo->vtPts.rbegin(); itParts != pInfo->vtPts.rend(); itParts++) {
		for (int32_t itParts = pInfo->getAttributeCount(TYPE_POLYGON_DATA_ATTR_PART) - 1; itParts >= 0; --itParts) {
			offPolygon = itParts;
			cntPolygon -= offPolygon;
			//isIn = isPointInPolygon(&pt, &pInfo->vtVtx[offPolygon], cntPolygon);
			isIn = isPointInPolygon(&pt, &pPolygon[offPolygon], cntPolygon);
			if (isIn) {
				break;
			}
			cntPolygon = cntVtx - cntPolygon;
		}
	}

	if (isIn) {
		return pInfo;
	}
	else {
		return nullptr;
	}
}