#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "MapExtend.h"
#include "../utils/UserLog.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

MapExtend::MapExtend()
{
}


MapExtend::~MapExtend()
{
	Release();
}


const unordered_map<uint32_t, stExtendMeshInfo>* MapExtend::GetMapData(void)
{
	return &m_mapData;
}


const stExtendMeshInfo* MapExtend::GetMapMeshData(IN const uint32_t tile_id)
{
	unordered_map<uint32_t, stExtendMeshInfo>::const_iterator it = m_mapData.find(tile_id);
	if (it != m_mapData.end()) {
		return &it->second;
	}

	return nullptr;
}


bool MapExtend::Initialize(void)
{
	return MapBase::Initialize();
}


void MapExtend::Release(void)
{
	if (!m_mapData.empty())
	{
		// mesh
		for (unordered_map<uint32_t, stExtendMeshInfo>::iterator itMesh = m_mapData.begin(); itMesh != m_mapData.end(); itMesh++)
		{
			// link
			for (unordered_map<uint64_t, stExtendInfo*>::iterator it = itMesh->second.mapLink.begin(); it != itMesh->second.mapLink.end(); it++) {
				delete it->second;
				it->second = nullptr;
			}

			// node
			for (unordered_map<uint64_t, stExtendInfo*>::iterator it = itMesh->second.mapNode.begin(); it != itMesh->second.mapNode.end(); it++) {
				delete it->second;
				it->second = nullptr;
			}
		}

		m_mapData.clear();
		unordered_map<uint32_t, stExtendMeshInfo>().swap(m_mapData);
	}

	MapBase::Release();
}


const uint32_t MapExtend::GetCount(void) const
{
	return m_mapData.size();
}


bool MapExtend::AddData(IN const stExtendInfo * pData, IN const int32_t nKeyType) // nDataType 0:None, 1:node, 2:link
{
	int ret = false;
	if (pData != nullptr) 
	{
		int32_t meshId = pData->keyId.tile_id;

		unordered_map<uint32_t, stExtendMeshInfo>::iterator itMesh = m_mapData.find(meshId);
		if (itMesh != m_mapData.end()) {
			// add
			if (nKeyType == TYPE_KEY_NODE) {
				itMesh->second.mapNode.emplace(pData->keyId.llid, const_cast<stExtendInfo*>(pData));
			} else if (nKeyType == TYPE_KEY_LINK) {
				itMesh->second.mapLink.emplace(pData->keyId.llid, const_cast<stExtendInfo*>(pData));
			} else {
				return false;
			}			
		} else {
			// create
			stExtendMeshInfo extMesh;
			if (nKeyType == TYPE_KEY_NODE) {
				extMesh.mapNode.emplace(pData->keyId.llid, const_cast<stExtendInfo*>(pData));
			} else if (nKeyType == TYPE_KEY_LINK) {
				extMesh.mapLink.emplace(pData->keyId.llid, const_cast<stExtendInfo*>(pData));
			} else {
				return false;
			}

			m_mapData.emplace(meshId, extMesh);
		}

		ret = true;
	}

	return ret;
}

bool MapExtend::AddData(IN const KeyID keyId, IN const int8_t type, IN const double value, IN const int32_t nKeyType) // nDataType 0:None, 1:node, 2:link
{
	int ret = false;

	unordered_map<uint32_t, stExtendMeshInfo>::iterator itMesh = m_mapData.find(keyId.tile_id);
	if (itMesh != m_mapData.end()) {
		// add
		unordered_map<uint64_t, stExtendInfo*>::iterator it;
		if (nKeyType == TYPE_KEY_NODE) {
			it = itMesh->second.mapNode.find(keyId.llid);
		} else if (nKeyType == TYPE_KEY_LINK) {
			it = itMesh->second.mapLink.find(keyId.llid);
		} else {
			return false;
		}

		if (((nKeyType == TYPE_KEY_NODE) && (it != itMesh->second.mapNode.end())) ||
			((nKeyType == TYPE_KEY_LINK) && (it != itMesh->second.mapLink.end()))) {
			// add
			it->second->vtType.emplace_back(type);
			it->second->vtValue.emplace_back(value);
			it->second->cntData++;
		} else {
			// create
			stExtendInfo* pItem = new stExtendInfo();
			pItem->keyId = keyId;
			pItem->vtType.emplace_back(type);
			pItem->vtValue.emplace_back(value);
			pItem->cntData++;

			ret = AddData(pItem, nKeyType);
		}
	} else {
		// create
		stExtendInfo* pItem = new stExtendInfo();
		pItem->keyId = keyId;
		pItem->vtType.emplace_back(type);
		pItem->vtValue.emplace_back(value);
		pItem->cntData++;

		ret = AddData(pItem, nKeyType);
	}

	ret = true;

	return ret;
}


stExtendInfo* MapExtend::GetExtendById(IN const KeyID keyId, IN const int32_t nKeyType) // nDataType 0:None, 1:node, 2:link
{
	stExtendInfo* pRet = nullptr;

	unordered_map<uint32_t, stExtendMeshInfo>::const_iterator itMesh = m_mapData.find(keyId.tile_id);
	if (itMesh != m_mapData.end()) {
		unordered_map<uint64_t, stExtendInfo*>::const_iterator it;
		if (nKeyType == TYPE_KEY_NODE) {
			it = itMesh->second.mapNode.find(keyId.llid);
			if (it != itMesh->second.mapNode.end()) {
				pRet = it->second;
			}
		} else if (nKeyType == TYPE_KEY_LINK) {
			it = itMesh->second.mapLink.find(keyId.llid);
			if (it != itMesh->second.mapLink.end()) {
				pRet = it->second;
			}
		} else {
			pRet = nullptr;
		}
	}
	
	return pRet;
}


double MapExtend::GetExtendDataById(IN const KeyID keyId, IN const int8_t valueType, IN const int32_t nKeyType) // nDataType 0:None, 1:node, 2:link
{
	double ret = 0.f;

	unordered_map<uint32_t, stExtendMeshInfo>::const_iterator itMesh = m_mapData.find(keyId.tile_id);
	if (itMesh != m_mapData.end()) {
		stExtendInfo* pItem = nullptr;
		unordered_map<uint64_t, stExtendInfo*>::const_iterator it;
		if (nKeyType == TYPE_KEY_NODE) {
			it = itMesh->second.mapNode.find(keyId.llid);
			if (it != itMesh->second.mapNode.end()) {
				pItem = it->second;
			}
		} else if (nKeyType == TYPE_KEY_LINK) {
			it = itMesh->second.mapLink.find(keyId.llid);
			if (it != itMesh->second.mapLink.end()) {
				pItem = it->second;
			}
		} else {
			pItem = nullptr;
		}

		if (pItem != nullptr) {
			for (int ii = 0; ii < pItem->vtType.size(); ii++) {
				if (pItem->vtType[ii] == valueType) {
					ret = pItem->vtValue[ii];
					break;
				}
			} // for
		}
	}

	return ret;
}