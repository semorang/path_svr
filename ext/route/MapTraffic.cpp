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
	if (!mapTrafficMeshData.empty())
	{
		for (unordered_map<uint32_t, stTrafficMesh*>::iterator it = mapTrafficMeshData.begin(); it != mapTrafficMeshData.end(); it++) {
			// clear match

			// link-ks matching
			if (it->second && !it->second->mapLinkToKS.empty()) {
				it->second->mapLinkToKS.clear();
				unordered_map<uint32_t, stKSLinkLink>().swap(it->second->mapLinkToKS);
			}

			// link-ttl matching
			if (it->second && !it->second->mapLinkToTTL.empty()) {
				it->second->mapLinkToTTL.clear();
				unordered_map<uint32_t, stTTLinLink>().swap(it->second->mapLinkToTTL);
			}


			// clear traffic TTL
			if (it->second && !it->second->mapTrafficTTL.empty()) {
				for (unordered_map<uint32_t, stTrafficInfoTTL*>::iterator itTTL = it->second->mapTrafficTTL.begin(); itTTL != it->second->mapTrafficTTL.end(); itTTL++) {
					SAFE_DELETE(itTTL->second);
				}
				it->second->mapTrafficTTL.clear();
				unordered_map<uint32_t, stTrafficInfoTTL*>().swap(it->second->mapTrafficTTL);
			}

			SAFE_DELETE(it->second);
		}

		// clear traffic KS
		if (!mapTrafficInfoKS.empty()) {
			for (unordered_map<uint32_t, stTrafficInfoKS*>::iterator itKS = mapTrafficInfoKS.begin(); itKS != mapTrafficInfoKS.end(); itKS++) {
				SAFE_DELETE(itKS->second);
			}
			mapTrafficInfoKS.clear();
			unordered_map<uint32_t, stTrafficInfoKS*>().swap(mapTrafficInfoKS);
		}
	}

	MapBase::Release();
}


const unordered_map<uint32_t, stTrafficMesh*>* MapTraffic::GetTrafficMeshData(void) const
{
	return &mapTrafficMeshData;
}


const uint8_t MapTraffic::GetSpeed(IN const KeyID link, IN const uint8_t dir, IN OUT uint8_t& type)
{
	uint8_t retSpeed = SPEED_NOT_AVALABLE;
	uint8_t retType = type;

	stTrafficMesh* pMesh = nullptr;
	unordered_map<uint32_t, stTrafficMesh*>::const_iterator itMesh = mapTrafficMeshData.find(link.tile_id);
	if (itMesh != mapTrafficMeshData.end()) {
		pMesh = itMesh->second;
	}

	if (pMesh == nullptr) {
		return retSpeed;
	}

	// link에 매칭되는 traffic key 구하기

	// 1st ttl speed
	if (type == TYPE_TRAFFIC_REAL || type == TYPE_TRAFFIC_REAL_STATIC) {
		if (!pMesh->mapLinkToTTL.empty() && !pMesh->mapTrafficTTL.empty()) {
			unordered_map<uint32_t, stTTLinLink>::const_iterator itTTL = pMesh->mapLinkToTTL.find(link.nid);
			if (itTTL != pMesh->mapLinkToTTL.end()) {
				uint32_t ttl_nid;
				if (dir == DIR_POSITIVE) {
					ttl_nid = itTTL->second.ttlIdPositive;
				} else { //if (dir == DIR_NAGATIVE) {
					ttl_nid = itTTL->second.ttlIdNagative;
				}

				unordered_map<uint32_t, stTrafficInfoTTL*>::const_iterator itTraffic = pMesh->mapTrafficTTL.find(ttl_nid);
				if (itTraffic != pMesh->mapTrafficTTL.end()) {
					retSpeed = itTraffic->second->speed;
					if (retSpeed != SPEED_NOT_AVALABLE) {
						retType = TYPE_SPEED_REAL_TTL;
					}
				}
			}
		}
	}

	// 2nd ks speed
	if ((type == TYPE_TRAFFIC_REAL || type == TYPE_TRAFFIC_REAL_STATIC) && (retSpeed == SPEED_NOT_AVALABLE)) {
		if (!pMesh->mapLinkToKS.empty() && !mapTrafficInfoKS.empty()) {
			unordered_map<uint32_t, stKSLinkLink>::const_iterator itKS = pMesh->mapLinkToKS.find(link.nid);
			if (itKS != pMesh->mapLinkToKS.end()) {
				uint32_t ks_nid;
				if (dir == DIR_POSITIVE) {
					ks_nid = itKS->second.ksIdPositive;
				} else { //if (dir == DIR_NAGATIVE) {
					ks_nid = itKS->second.ksIdNagative;
				}

				unordered_map<uint32_t, stTrafficInfoKS*>::const_iterator itTraffic = mapTrafficInfoKS.find(ks_nid);
				if (itTraffic != mapTrafficInfoKS.end()) {
					retSpeed = itTraffic->second->speed;
					if (retSpeed != SPEED_NOT_AVALABLE) {
						retType = TYPE_SPEED_REAL_KS;
					}
				}
			}
		}
	}

	type = retType;

	return retSpeed;
}


const uint64_t MapTraffic::GetTrafficId(IN const KeyID link, IN const uint8_t dir, IN const uint8_t type)
{
	uint64_t retId = 0;

	stTrafficMesh* pMesh = nullptr;
	unordered_map<uint32_t, stTrafficMesh*>::const_iterator itMesh = mapTrafficMeshData.find(link.tile_id);
	if (itMesh != mapTrafficMeshData.end()) {
		pMesh = itMesh->second;
	}

	if (pMesh == nullptr) {
		return retId;
	}

	// link에 매칭되는 traffic key 구하기
	if ((type == TYPE_SPEED_REAL_TTL) || (type == TYPE_SPEED_STATIC_TTL)) { // TTL
		if (!pMesh->mapLinkToTTL.empty() && !pMesh->mapTrafficTTL.empty()) {
			unordered_map<uint32_t, stTTLinLink>::const_iterator itTTL = pMesh->mapLinkToTTL.find(link.nid);
			if (itTTL != pMesh->mapLinkToTTL.end()) {
				uint32_t ttl_nid;
				if (dir == DIR_POSITIVE) {
					ttl_nid = itTTL->second.ttlIdPositive;
				} else { //if (dir == DIR_NAGATIVE) {
					ttl_nid = itTTL->second.ttlIdNagative;
				}

				retId = static_cast<uint64_t>(link.tile_id) * TILEID_BYTE_COUNT + ttl_nid;
			}
		}
	}
	else if ((type == TYPE_SPEED_REAL_KS) || (type == TYPE_SPEED_STATIC_KS)) { // KS
		if (!pMesh->mapLinkToKS.empty() && !pMesh->mapLinkToKS.empty()) {
			unordered_map<uint32_t, stKSLinkLink>::const_iterator itKS = pMesh->mapLinkToKS.find(link.nid);
			if (itKS != pMesh->mapLinkToKS.end()) {
				uint64_t ks_id;
				if (dir == DIR_POSITIVE) {
					ks_id = itKS->second.ksIdPositive;
				} else { //if (dir == DIR_NAGATIVE) {
					ks_id = itKS->second.ksIdNagative;
				}

				retId = ks_id;
			}
		}
	}

	return retId;
}


const uint32_t MapTraffic::GetCount(void) const
{
	return mapTrafficMeshData.size();
}


bool MapTraffic::AddKSData(IN const uint32_t ks_id, IN const uint32_t tile_nid, IN const uint32_t link_nid, IN const uint8_t dir) // add KS traffic link match
{
	bool ret = false;

	// ks to link 확인 
	unordered_map<uint32_t, stTrafficInfoKS*>::const_iterator it = mapTrafficInfoKS.find(ks_id);
	if (it != mapTrafficInfoKS.end()) {
		KeyID link;
		link.tile_id = tile_nid;
		link.nid = link_nid;
		link.dir = dir;
		
		// add traffic info data
		it->second->setLinks.emplace(link.llid);
	} else {
		// create
		KeyID link;
		link.tile_id = tile_nid;
		link.nid = link_nid;
		link.dir = dir;

		stTrafficInfoKS* pTraffic = nullptr;
		pTraffic = new stTrafficInfoKS();
		pTraffic->ks_id = ks_id;
		pTraffic->setLinks.emplace(link.llid);

		// add traffic info data
		mapTrafficInfoKS.emplace(ks_id, pTraffic);
	}

	
	// 메쉬 확인
	stTrafficMesh* pMesh = nullptr;
	unordered_map<uint32_t, stTrafficMesh*>::const_iterator itMesh = mapTrafficMeshData.find(tile_nid);
	if (itMesh != mapTrafficMeshData.end()) {
		pMesh = itMesh->second;
	} else {
		// create traffic mesh
		pMesh = new stTrafficMesh();
		pMesh->meshId = tile_nid;

		// add traffic mesh
		mapTrafficMeshData.emplace(pMesh->meshId, pMesh);
	}

	if (pMesh) {
		// link-ks 매칭 데이터 확인
		unordered_map<uint32_t, stKSLinkLink>::iterator itLink = pMesh->mapLinkToKS.find(link_nid);
		if (itLink != pMesh->mapLinkToKS.end()) {
			if (dir == DIR_POSITIVE) {
				if (itLink->second.ksIdPositive != 0) {
					// 이미 있으면....
					LOG_TRACE(LOG_KEY_TRAFFIC, LOG_WARNING, "link to ks matching duplicated, tile_id:%d, link_nid:%ld, ks_id:%d, dir:%ld", tile_nid, link_nid, ks_id, DIR_POSITIVE);
				}
				itLink->second.ksIdPositive = ks_id;
			} else { //if (keyTraffic.ttl.dir == DIR_NAGATIVE) {
				if (itLink->second.ksIdNagative != 0) {
					// 이미 있으면....
					LOG_TRACE(LOG_KEY_TRAFFIC, LOG_WARNING, "link to ks matching duplicated, tile_id:%d, link_nid:%ld, ks_id:%d, dir:%ld", tile_nid, link_nid, ks_id, DIR_NAGATIVE);
				}
				itLink->second.ksIdNagative = ks_id;
			}

			if (itLink->second.ksIdPositive == itLink->second.ksIdNagative) {
				// 정/역이 같으면....
				LOG_TRACE(LOG_KEY_TRAFFIC, LOG_WARNING, "link to ks matching duplicated, tile_id:%d, link_nid:%ld, ks_id_p:%d, ks_id_n:%d", tile_nid, link_nid, itLink->second.ksIdPositive, itLink->second.ksIdNagative);
			}
		} else {
			// create
			stKSLinkLink ksInLink;
			if (dir == DIR_POSITIVE) {
				ksInLink.ksIdPositive = ks_id;
			} else { //if (keyTraffic.ttl.dir == DIR_NAGATIVE) {
				ksInLink.ksIdNagative = ks_id;
			}

			// add traffic matching data
			pMesh->mapLinkToKS.emplace(link_nid, ksInLink);
		}

		ret = true;
	}

	return ret;
}


bool MapTraffic::UpdateKSData(IN const uint32_t ksId, IN const uint8_t speed, uint32_t timestamp) // add traffic info
{
	bool ret = false;

	unordered_map<uint32_t, stTrafficInfoKS*>::const_iterator it = mapTrafficInfoKS.find(ksId);
	if (it != mapTrafficInfoKS.end()) {
		it->second->speed = speed;
		it->second->timestamp = timestamp;

		ret = true;
	}

	return ret;
}


bool MapTraffic::CheckAliveKSData(IN const uint32_t timestamp) // 입력된 TIMESTAMP 이전의 값들은 초기화
{
	// multiprocess
	time_t startTime = LOG_TRACE(LOG_KEY_TRAFFIC, LOG_DEBUG, 0, "");

	for (unordered_map<uint32_t, stTrafficInfoKS*>::const_iterator itKS = mapTrafficInfoKS.begin(); itKS != mapTrafficInfoKS.end(); itKS++) {
		// KS
		if ((timestamp == 0) || ((itKS->second->speed != SPEED_NOT_AVALABLE) && (itKS->second->timestamp < timestamp))) {
			itKS->second->speed = SPEED_NOT_AVALABLE;
		}
	} // for

	LOG_TRACE(LOG_KEY_TRAFFIC, LOG_DEBUG, startTime, "ks alive check time");

	return true;
}


const stTrafficInfoKS * MapTraffic::GetTrafficInfoKS(IN const uint32_t ks_id)
{
	stTrafficInfoKS * pInfo = nullptr;

	unordered_map<uint32_t, stTrafficInfoKS*>::const_iterator itKS = mapTrafficInfoKS.find(ks_id);
	if (itKS != mapTrafficInfoKS.end()) {
		pInfo = itKS->second;
	}

	return pInfo;
}


const unordered_map<uint32_t, stTrafficInfoKS*>* MapTraffic::GetTrafficKSMapData(void) const
{
	return &mapTrafficInfoKS;
}


bool MapTraffic::AddTTLData(IN const uint32_t ttl_nid, IN const uint8_t ttl_dir, IN const uint32_t tile_nid, IN const uint32_t link_nid, IN const uint8_t link_dir)
{
	bool ret = false;
	// 메쉬 확인
	stTrafficMesh* pMesh = nullptr;

	unordered_map<uint32_t, stTrafficMesh*>::const_iterator itMesh = mapTrafficMeshData.find(tile_nid);
	if (itMesh != mapTrafficMeshData.end()) {
		pMesh = itMesh->second;
	} else {
		// create traffic mesh
		pMesh = new stTrafficMesh();
		pMesh->meshId = tile_nid;

		// add traffic mesh
		mapTrafficMeshData.emplace(pMesh->meshId, pMesh);
	}


	if (pMesh) {
		// traffic key의 map 확인 
		unordered_map<uint32_t, stTrafficInfoTTL*>::const_iterator it = pMesh->mapTrafficTTL.find(ttl_nid);
		if (it != pMesh->mapTrafficTTL.end()) {
			// 이미 있으면....
			LOG_TRACE(LOG_KEY_TRAFFIC, LOG_WARNING, "already exist ttl traffic info in mesh, mesh_id:%d, ttl_id:%d", tile_nid, ttl_nid);
		} else {
			// create
			stTrafficInfoTTL* pTraffic = nullptr;
			pTraffic = new stTrafficInfoTTL();
			pTraffic->ttl_nid = ttl_nid;
			pTraffic->link_nid = link_nid;
			pTraffic->ttl_dir = ttl_dir;
			pTraffic->link_dir = link_dir;

			// add traffic info data
			pMesh->mapTrafficTTL.emplace(ttl_nid, pTraffic);
		}


		// link-ttl 매칭 데이터 확인
		unordered_map<uint32_t, stTTLinLink>::iterator itLink = pMesh->mapLinkToTTL.find(link_nid);
		if (itLink != pMesh->mapLinkToTTL.end()) {
			if (ttl_dir == DIR_POSITIVE) {
				if (itLink->second.ttlIdPositive != 0) {
					// 이미 있으면....
					LOG_TRACE(LOG_KEY_TRAFFIC, LOG_WARNING, "link to ttl matching duplicated, tile_id:%d, link_nid:%ld, ttl_id:%d, dir:%ld", ttl_nid, link_nid, ttl_nid, DIR_POSITIVE);
				}
				itLink->second.ttlIdPositive = ttl_nid;
			} else { //if (keyTraffic.ttl.dir == DIR_NAGATIVE) {
				if (itLink->second.ttlIdNagative != 0) {
					// 이미 있으면....
					LOG_TRACE(LOG_KEY_TRAFFIC, LOG_WARNING, "link to ttl matching duplicated, tile_id:%d, link_nid:%ld, ttl_id:%d, dir:%ld", ttl_nid, link_nid, ttl_nid, DIR_NAGATIVE);
				}
				itLink->second.ttlIdNagative = ttl_nid;
			}

			if (itLink->second.ttlIdPositive == itLink->second.ttlIdNagative) {
				// 정/역이 같으면....
				LOG_TRACE(LOG_KEY_TRAFFIC, LOG_WARNING, "link to ttl matching duplicated, tile_id:%d, link_nid:%ld, ttl_id_p:%d, ttl_id_n:%d", ttl_nid, link_nid, itLink->second.ttlIdPositive, itLink->second.ttlIdNagative);
			}
		} else {
			// create
			stTTLinLink ttlInLink;
			if (ttl_dir == DIR_POSITIVE) {
				ttlInLink.ttlIdPositive = ttl_nid;
			} else { //if (keyTraffic.ttl.dir == DIR_NAGATIVE) {
				ttlInLink.ttlIdNagative = ttl_nid;
			}

			// add traffic matching data
			pMesh->mapLinkToTTL.emplace(link_nid, ttlInLink);
		}

		ret = true;
	}
	
	return ret;
}


bool MapTraffic::UpdateTTLData(IN const uint64_t ttlId, IN const uint8_t speed, uint32_t timestamp, OUT uint8_t& dir, OUT KeyID& link) // update traffic
{
	bool ret = false;

	// 메쉬 확인
	if (ttlId > TILEID_BYTE_COUNT) {
		stTrafficMesh* pMesh = nullptr;
		int32_t tile_nid = ttlId / TILEID_BYTE_COUNT;
		int32_t ttl_nid = ttlId % TILEID_BYTE_COUNT;

		unordered_map<uint32_t, stTrafficMesh*>::const_iterator itMesh = mapTrafficMeshData.find(tile_nid);
		if (itMesh != mapTrafficMeshData.end()) {
			pMesh = itMesh->second;

			if (pMesh) {
				// traffic key의 map 확인 
				unordered_map<uint32_t, stTrafficInfoTTL*>::const_iterator itTTL = pMesh->mapTrafficTTL.find(ttl_nid);
				if (itTTL != pMesh->mapTrafficTTL.end()) {
					if ((speed == SPEED_NOT_AVALABLE) && (itTTL->second->timestamp >= timestamp - TRAFFIC_DATA_ALIVE)) {
						// 지정 시간 이내면 이전 데이터 유지, 과거 데이터면 리셋
						//itTTL->second->speed = speed;
						//itTTL->second->timestamp = timestamp;
					} else {
						itTTL->second->speed = speed;
						itTTL->second->timestamp = timestamp;
					}

					dir = itTTL->second->ttl_dir;

					link.tile_id = tile_nid;
					link.nid = itTTL->second->link_nid;
					link.dir = itTTL->second->link_dir;

					ret = true;
				}
			}
		}
	}

	return ret;
}


bool MapTraffic::CheckAliveTTLData(IN const uint32_t timestamp, OUT vector<stTrafficInfoTTL>& vtCheckedResetLink) // 입력된 TIMESTAMP 이전의 값들은 초기화
{
	// multiprocess
	time_t startTime = LOG_TRACE(LOG_KEY_TRAFFIC, LOG_DEBUG, 0, "");

#if 1 // use omp
	vector<stTrafficMesh*> vtMesh;
	for (auto it : mapTrafficMeshData) {
		vtMesh.emplace_back(it.second);
	}

#pragma omp parallel for
	for (int ii = 0; ii < vtMesh.size(); ii++) {
		stTrafficInfoTTL checkedLink;
		for (const auto& itTTL : vtMesh[ii]->mapTrafficTTL) {
			if ((timestamp == 0) || ((itTTL.second->speed != SPEED_NOT_AVALABLE) && (itTTL.second->timestamp < timestamp))) {
				itTTL.second->speed = SPEED_NOT_AVALABLE;
				memcpy(&checkedLink, itTTL.second, sizeof(checkedLink));
				checkedLink.ttl_nid = itTTL.first; // mesh id 대체 
#pragma omp critical
				vtCheckedResetLink.emplace_back(checkedLink);
			}
		}
	}
#else
	for (auto it : mapTrafficMeshData) {
		for (unordered_map<uint32_t, stTrafficInfoTTL*>::const_iterator itTTL = it.second->mapTrafficTTL.begin(); itTTL != it.second->mapTrafficTTL.end(); itTTL++) {
			if ((timestamp == 0) || ((itTTL->second->speed != SPEED_NOT_AVALABLE) && (itTTL->second->timestamp < timestamp))) {
				itTTL->second->speed = SPEED_NOT_AVALABLE;
			}
		} // for
	} // for
#endif

	if (!vtCheckedResetLink.empty()) {
		LOG_TRACE(LOG_KEY_TRAFFIC, LOG_DEBUG, startTime, "ttl alive check time, reset:%d", vtCheckedResetLink.size());
	}

	return true;
}


const int32_t getBlockinTTL(IN const uint64_t ttlId)
{
	int32_t meshId = 0;
	if (ttlId > 0) {
		meshId = static_cast<int32_t>(ttlId / TILEID_BYTE_COUNT);
	}

	return meshId;
}

const int32_t getIteminTTL(IN const uint64_t ttlId)
{
	int32_t itemId = 0;
	if (ttlId > 0) {
		itemId = ttlId % TILEID_BYTE_COUNT;
	}

	return itemId;
}


const stTrafficInfoTTL * MapTraffic::GetTrafficInfoTTL(IN const uint64_t ttl_id)
{
	stTrafficInfoTTL * pInfo = nullptr;

	// find mesh
	int32_t meshId = getBlockinTTL(ttl_id);
	if (meshId > 0) {
		unordered_map<uint32_t, stTrafficMesh*>::const_iterator itMesh = mapTrafficMeshData.find(meshId);
		if (itMesh != mapTrafficMeshData.end()) {
			// find traffic TTL info
			unordered_map<uint32_t, stTrafficInfoTTL*>::const_iterator itTTL = itMesh->second->mapTrafficTTL.find(ttl_id);
			if (itTTL != itMesh->second->mapTrafficTTL.end()) {
				pInfo = itTTL->second;
			}
		}
	}

	return pInfo;
}


const unordered_map<uint32_t, stTrafficInfoTTL*>* MapTraffic::GetTrafficTTLMapData(IN const uint32_t meshId)
{
	unordered_map<uint32_t, stTrafficInfoTTL*>* pInfo = nullptr;

	// find mesh
	unordered_map<uint32_t, stTrafficMesh*>::const_iterator itMesh = mapTrafficMeshData.find(meshId);

	if (itMesh != mapTrafficMeshData.end()) {
		pInfo = &itMesh->second->mapTrafficTTL;
	}

	return pInfo;
}
