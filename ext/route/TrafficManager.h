#pragma once

#include "MapDef.h"

#if defined(__USE_TEMPLETE)
#include "MapBase.h"
#endif
#include "MapTraffic.h"

#include "DataManager.h"

#pragma pack (push, 1)
typedef struct {
	uint32_t kslink_id;
	uint32_t timestamp;
	uint8_t bySpeed;
}stRealtimeTrafficDataItem;
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

	std::unordered_map<uint32_t, stRealtimeTrafficDataItem> umap_RealTrafficInfo;

protected:
	

private:
	CDataManager* m_pDataMgr;

public:
	bool Initialize(void);
	void Release(void);

	void SetDataMgr(IN CDataManager* pDataMgr);

	size_t LoadData(IN const char* szFileName);
	size_t Update(IN const uint64_t timestamp);



};

