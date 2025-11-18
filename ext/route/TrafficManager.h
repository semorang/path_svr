#pragma once

#include "../include/MapDef.h"

#if defined(__USE_TEMPLETE)
#include "MapBase.h"
#endif
#include "MapTraffic.h"

#include "DataManager.h"

#pragma pack (push, 1)

typedef enum
{
	TYPE_TRAFFIC_KS_ID = 10000,
	TYPE_TRAFFIC_TTL_ID = 10001,
	TYPE_TRAFFIC_KSR_ID = 10003,
};

typedef struct
{
	uint32_t timestamp;
	uint32_t type;
	uint32_t size;
}stTrafficDataHeader;

typedef struct
{
	uint64_t id;
	uint8_t speed;
}stTrafficTTLItem;

typedef struct
{
	uint32_t id;
	uint32_t time;
	uint8_t speed;
}stTrafficKSItem;
#pragma pack (pop)



class CTrafficManager
{
public:
	CTrafficManager();
	~CTrafficManager();

protected:


private:
	uint64_t currentTimestamp;
	MapTraffic* m_pMapTraffic;

protected:
	

private:
	CDataManager* m_pDataMgr;

public:
	bool Initialize(void);
	void Release(void);

	void SetDataMgr(IN CDataManager* pDataMgr);

	bool LoadData(IN const char* szFilePath);
	size_t Update(IN const char* szFileName, IN const int type, IN const uint32_t timestamp = 0);

private:
	bool ParsingTTL(IN FILE* fp, IN const int32_t size, IN OUT uint32_t& timestamp);
	bool ParsingKS(IN FILE* fp, IN const int32_t size, IN OUT uint32_t& timestamp);
};
