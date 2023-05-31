#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "MapLink.h"
#include "../utils/UserLog.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


MapLink::MapLink()
{
}


MapLink::~MapLink()
{
	Release();
}


bool MapLink::Initialize(void)
{
	return MapBase::Initialize();
}


void MapLink::Release(void)
{
	if (!mapData.empty())
	{
		for (map<uint64_t, stLinkInfo*>::iterator it = mapData.begin(); it != mapData.end(); it++)
		{
			delete it->second;
			it->second = nullptr;
		}

		mapData.clear();
		map<uint64_t, stLinkInfo*>().swap(mapData);
	}

	MapBase::Release();
}


const uint32_t MapLink::GetCount(void) const
{
	return mapData.size();
}


int MapLink::AddData(IN const stLinkInfo * pData)
{
	if (pData != nullptr /*&& pData->info != NOT_USE*/) 
	{
		mapData.emplace(pData->link_id.llid, const_cast<stLinkInfo*>(pData));
		return mapData.size();
	}
	else
	{
		return 0;
	}
}


bool MapLink::DeleteData(IN const KeyID keyId)
{
	map<uint64_t, stLinkInfo*>::iterator it = mapData.find(keyId.llid);
	if (it != mapData.end()) {
		delete it->second;
		it->second = nullptr;
		mapData.erase(keyId.llid);
		return true;
	}

	return false;
}


stLinkInfo * MapLink::GetLinkById(IN const KeyID keyId)
{
	map<uint64_t, stLinkInfo*>::iterator it = mapData.find(keyId.llid);
	if (it != mapData.end()) {
		//return &*mapData.find(keyId.llid)->second;
		return it->second;
	}
	
	return nullptr;
}


stLinkInfo * MapLink::GetLinkBySENode(IN const KeyID idx1, IN const KeyID idx2)
{
	for (map<uint64_t, stLinkInfo*>::iterator it = mapData.begin(); it != mapData.end(); it++)
	{
#if defined(USE_MULTIPROCESS)		
		LOG_TRACE(LOG_DEBUG, "Check OpenMP thread id %d", omp_get_thread_num());
#endif

		if (it->second->sub_info == NOT_USE)
		{
			continue;
		}

		if ((it->second->snode_id == idx1 && it->second->enode_id == idx2) ||
			(it->second->snode_id == idx2 && it->second->enode_id == idx1))
		{
			return it->second;
		}
	}

	return nullptr;
}
