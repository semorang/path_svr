#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "MapTraffic.h"
#include "../utils/UserLog.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


MapTraffic::MapTraffic()
{
}


MapTraffic::~MapTraffic()
{
	Release();
}


bool MapTraffic::Initialize(void)
{
	return MapBase::Initialize();
}


void MapTraffic::Release(void)
{
	if (!mapData.empty())
	{
		// clear traffic
		for (unordered_map<uint32_t, stTrafficInfo*>::iterator it = mapTraffic.begin(); it != mapTraffic.end(); it++)
		{
			SAFE_DELETE(it->second);
		}
		
		mapTraffic.clear();
		unordered_map<uint32_t, stTrafficInfo*>().swap(mapTraffic);


		// clear match
		for (unordered_map<uint32_t, stTrafficMesh*>::iterator it = mapData.begin(); it != mapData.end(); it++)
		{
			it->second->mapMatch.clear();
			unordered_map<uint64_t, uint64_t>().swap(it->second->mapMatch);

			SAFE_DELETE(it->second);
		}

		mapData.clear();
		unordered_map<uint32_t, stTrafficMesh*>().swap(mapData);
	}

	MapBase::Release();
}


const uint32_t MapTraffic::GetCount(void) const
{
	return mapData.size();
}


int MapTraffic::AddData(IN stTrafficInfo* pData) // add traffic
{
	if (pData != nullptr /*&& pData->info != NOT_USE*/)
	{
		mapTraffic.emplace(pData->ks_id, pData);
	}
	
	return 0;
}


int MapTraffic::AddData(IN const stTrafficKey& key, IN const uint32_t ksId)
{
	stTrafficInfo* pTraffic = nullptr;

	// traffic key의 map 확인 
	unordered_map<uint32_t, stTrafficInfo*>::const_iterator it = mapTraffic.find(ksId);
	if (it != mapTraffic.end()) {
		pTraffic = it->second;
	}
	else {
		// create
		pTraffic = new stTrafficInfo();
		pTraffic->key.llid = key.llid;
		pTraffic->ks_id = ksId;

		mapTraffic.emplace(ksId, pTraffic);
	}


	// 메쉬 확인
	stTrafficMesh* pMesh = nullptr;
	
	unordered_map<uint32_t, stTrafficMesh*>::const_iterator match = mapData.find(key.mesh);
	if (match == mapData.end()) {
		// 링크가 우선 작업되고, 이후에 매칭용 정보를 등록하기 때문에 트래픽 등록 시 여기로 오면 안됨. 

		//LOG_TRACE(LOG_WARNING, "mesh not exist, traffic(tile:%d, snode:%d, enode:%d, dir:%d)", key.mesh, key.snode, key.enode, key.dir);
		return 0;

		// create
		pMesh = new stTrafficMesh();
		pMesh->mesh = key.mesh;

		mapData.emplace(key.mesh, pMesh);
	}
	else
	{
		pMesh = match->second;
	}

	// traffic key로 매칭되는 link idx 구하기
	unordered_map<uint64_t, uint64_t>::const_iterator it_idx = pMesh->mapMatch.find(key.llid);
	if (it_idx == pMesh->mapMatch.end()) {
		// 진행방향을 양방향으로 놓고 재 확인
		stTrafficKey newKey = key;
		newKey.dir = 0;
		it_idx = pMesh->mapMatch.find(newKey.llid);
	}

	if (it_idx != pMesh->mapMatch.end()) {
		// 매칭되는 link idx 등록
		KeyID linkId;
		linkId.llid = it_idx->second;
		// 방향성 적용
		linkId.dir = key.dir;
		pTraffic->setLinks.emplace(linkId.llid);
	}
	else {
		// 이러면 안되는데.

		// 일방통행 링크의 방향성과 traffic 방향성이 일치하지 않음
		LOG_TRACE(LOG_WARNING, "link not match with traffic and dir, traffic(tile:%d, snode:%d, enode:%d, dir:%d)", key.mesh, key.snode, key.enode, key.dir);
		return 0;
	}

	//pMesh->mapTraffic.emplace(pData->key.llid, const_cast<stTrafficInfo*>(pData));
	////pItem->mapTraffic.insert(pair<uint64_t, stTrafficInfo*>({ pData->key_id, const_cast<stTrafficInfo*>(pData) }));
	return pTraffic->setLinks.size();
}


int MapTraffic::AddData(IN const stLinkInfo * pData)
{
	if (pData != nullptr /*&& pData->info != NOT_USE*/)
	{
		stTrafficKey traffickey;
		traffickey.mesh = pData->link_id.tile_id;
		traffickey.snode = pData->snode_id.nid;
		traffickey.enode = pData->enode_id.nid;
		traffickey.dir = pData->link_id.dir;

		unordered_map<uint32_t, stTrafficMesh*>::const_iterator it = mapData.find(pData->link_id.tile_id);
		if (it == mapData.end()) {
			// create
			stTrafficMesh* pMesh = new stTrafficMesh();
			pMesh->mesh = pData->link_id.tile_id;
			pMesh->mapMatch.emplace(traffickey.llid, pData->link_id.llid);
			//pItem->mapTraffic.insert(pair<uint64_t, stTrafficInfo*>({ pData->key_id, const_cast<stTrafficInfo*>(pData) }));

			mapData.emplace(pMesh->mesh, pMesh);
		}
		else
		{
			it->second->mapMatch.emplace(traffickey.llid, pData->link_id.llid);
			//it->second->LinkMatch.insert(pair<uint64_t, stTrafficInfo*>({ pData->key_id, const_cast<stTrafficInfo*>(pData) }));
		}
	}
	else
	{
		return 0;
	}
}


bool MapTraffic::DeleteData(IN const uint64_t keyId)
{
	stTrafficKey stKey;
	memcpy(&stKey, &keyId, sizeof(stKey));

	unordered_map<uint32_t, stTrafficMesh*>::iterator it = mapData.find(stKey.mesh);
	if (it != mapData.end()) {
		// clear match
		it->second->mapMatch.clear();
		unordered_map<uint64_t, uint64_t>().swap(it->second->mapMatch);

		SAFE_DELETE(it->second);

		return true;
	}

	return false;
}


const stTrafficInfo * MapTraffic::GetTrafficInfo(IN const uint32_t ksId)
{
	unordered_map<uint32_t, stTrafficInfo*>::const_iterator it = mapTraffic.find(ksId);

	if (it != mapTraffic.end()) {
		return it->second;
	}
	
	return nullptr;
}


const unordered_map<uint32_t, stTrafficInfo*>* MapTraffic::GetTrafficMapData(void)
{
	return &mapTraffic;
}
