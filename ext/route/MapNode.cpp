#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "MapNode.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

MapNode::MapNode()
{
}

MapNode::~MapNode()
{
	Release();
}

bool MapNode::Initialize(void)
{
	return MapBase::Initialize();
}


void MapNode::Release(void)
{
	if (!mapData.empty())
	{
		for (unordered_map<uint64_t, stNodeInfo*>::iterator it = mapData.begin(); it != mapData.end(); it++)
		{
			delete it->second;
			it->second = nullptr;
		}

		mapData.clear();
		unordered_map<uint64_t, stNodeInfo*>().swap(mapData);
	}

	MapBase::Release();
}


int MapNode::AddData(IN const stNodeInfo * pData)
{
	if (pData != nullptr /*&& pData->node_type != NOT_USE*/) 
	{
		mapData.emplace(pData->node_id.llid, const_cast<stNodeInfo*>(pData));
		return mapData.size();
	}
	else {
		return 0;
	}
}


bool MapNode::DeleteData(IN const KeyID keyId)
{
	unordered_map<uint64_t, stNodeInfo*>::iterator it = mapData.find(keyId.llid);
	if (it != mapData.end()) {
		delete it->second;
		it->second = nullptr;
		mapData.erase(keyId.llid);
		return true;
	}

	return false;
}


stNodeInfo * MapNode::GetNodeById(IN const KeyID keyId)
{
	unordered_map<uint64_t, stNodeInfo*>::iterator it = mapData.find(keyId.llid);
	if (it != mapData.end()) {
		return it->second;
		//return &*mapData.find(keyId.llid)->second;
	}
	
	return nullptr;
}


const uint32_t MapNode::GetCount(void) const
{
	return mapData.size();
}
