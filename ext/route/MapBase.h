#pragma once

#include "../include/MapDef.h"

template <typename T>
class MapClass
{
public:
	MapClass();
	~MapClass();

	bool Initialize(void);
	void Release(void);

	int32_t AddData(IN const T* pData);
	bool DeleteData(IN const KeyID keyId);
	T* GetDataById(IN const KeyID keyId);
	uint32_t GetDataCount(void);

private:
	std::map<uint64_t, T*> mapData;
};


class MapBase
{
public:
	MapBase();
	~MapBase();

	virtual bool Initialize(void);
	virtual void Release(void);

	virtual const uint32_t GetCount(void) const;


	void SetVersion(IN const uint32_t nMajor, IN const uint32_t nMinor, IN const uint32_t nPatch, IN const uint32_t nBuild);
	int32_t GetVersion(IN const uint32_t nLevel); // lv, 0:major, 1:minor; 2:patch, 3:build
	const char* GetVersionString(void);

private:
	char m_szVersion[256];
	uint32_t m_nVersion[4]; // version
};