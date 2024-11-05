#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "MapName.h"
#include "../utils/UserLog.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


MapName::MapName()
{

}


MapName::~MapName()
{
	Release();
}


bool MapName::Initialize(void)
{
	return MapBase::Initialize();
}


void MapName::Release(void)
{
	if (!mapData.empty())
	{
		for (map<uint32_t, stNameInfo*>::iterator it = mapData.begin(); it != mapData.end(); it++)
		{
			delete it->second;
			it->second = nullptr;
		}

		mapData.clear();
		map<uint32_t, stNameInfo*>().swap(mapData);
	}

	MapBase::Release();
}


bool MapName::AddData(IN const stNameInfo * pData)
{
	if (pData != nullptr) {
		mapData.emplace(pData->name_id, const_cast<stNameInfo*>(pData));
		return true;
	}
	
	return false;
}


bool MapName::DeleteData(IN const KeyID keyId)
{
	map<uint32_t, stNameInfo*>::iterator it = mapData.find(static_cast<int32_t>(keyId.nid));
	if (it != mapData.end()) {
		delete it->second;
		it->second = nullptr;
		mapData.erase(keyId.nid);
		return true;
	}

	return false;
}


const uint32_t MapName::GetCount(void) const
{
	return mapData.size();
}


const char* MapName::GetNameData(IN const uint32_t id) const
{
	map<uint32_t, stNameInfo*>::const_iterator it = mapData.find(id);
	if (it != mapData.end()) {
		return it->second->name.c_str();
	}

	return nullptr;
}
