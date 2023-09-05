#if defined(_WIN32)
#include "../stdafx.h"
#endif


#include "MapBase.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


template <typename T>
MapClass<T>::MapClass()
{
	Initialize();
}

template <typename T>
MapClass<T>::~MapClass()
{
	Release();
}

template <typename T>
int32_t MapClass<T>::AddData(IN const T * pData)
{
	if (pData != nullptr /*&& pData->info != NOT_USE*/) 
	{
		mapData.emplace(pData->bld_id.llid, const_cast<T*>(pData));
		return true;
	}
	else
	{
		return false;
	}
}

template <typename T>
bool MapClass<T>::DeleteData(IN const KeyID keyId)
{
	typename map<uint64_t, T*>::iterator it = mapData.find(keyId.llid);
	if (it != mapData.end()) {
		delete it->second;
		it->second = nullptr;
		mapData.erase(keyId.llid);
		return true;
	}

	return false;
}

template <typename T>
bool MapClass<T>::Initialize(void)
{

	return true;
}


template <typename T>
void MapClass<T>::Release(void)
{
	if (!mapData.empty())
	{
		for (typename map<uint64_t, T*>::iterator it = mapData.begin(); it != mapData.end(); it++)
		{
			delete it->second;
			it->second = nullptr;
		}

		mapData.clear();
		map<uint64_t, T*>().swap(mapData);
	}
}


template <typename T>
T * MapClass<T>::GetDataById(IN const KeyID keyId)
{
	typename map<uint64_t, T*>::iterator it = mapData.find(keyId.llid);
	if (it != mapData.end()) {
		return it->second;
	}
	
	return nullptr;
}

template <typename T>
uint32_t MapClass<T>::GetDataCount(void)
{
	return mapData.size();
}




MapBase::MapBase()
{

}
	

MapBase::~MapBase()
{
	
}


bool MapBase::Initialize(void)
{
	memset(m_szVersion, 0x00, sizeof(m_szVersion));
	memset(m_nVersion, 0x00, sizeof(m_nVersion));

	return true;
}


void MapBase::Release(void)
{

}


const uint32_t MapBase::GetCount(void) const
{
	return 0;
}


void MapBase::SetVersion(IN const uint32_t nMajor, IN const uint32_t nMinor, IN const uint32_t nPatch, IN const uint32_t nBuild)
{
	m_nVersion[0] = nMajor;
	m_nVersion[1] = nMinor;
	m_nVersion[2] = nPatch;
	m_nVersion[3] = nBuild;

	snprintf(m_szVersion, sizeof(m_szVersion), "%d.%d.%d.%d", m_nVersion[0], m_nVersion[1], m_nVersion[2], m_nVersion[3]);
}


int32_t MapBase::GetVersion(IN const uint32_t nLevel)
{
	int32_t nVersion = -1;

	if (nLevel <= 3) {
		nVersion = m_nVersion[nLevel];
	}

	return nVersion;
}


const char* MapBase::GetVersionString(void)
{
	return m_szVersion;
}
	