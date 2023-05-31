#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "TrafficManager.h"




#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CTrafficManager::CTrafficManager()
{
	currentTimestamp = 0;

	m_pDataMgr = nullptr;
	m_pMapTraffic = nullptr;
}


CTrafficManager::~CTrafficManager()
{
	Release();

}


bool CTrafficManager::Initialize(void)
{

	return true;
}


void CTrafficManager::Release(void)
{
	if (!umap_RealTrafficInfo.empty())
	{
		umap_RealTrafficInfo.clear();
		unordered_map<uint32_t, stRealtimeTrafficDataItem>().swap(umap_RealTrafficInfo);
	}
}


void CTrafficManager::SetDataMgr(IN CDataManager* pDataMgr)
{
	if (pDataMgr) {
		m_pDataMgr = pDataMgr;
	}
}


size_t CTrafficManager::Update(IN const uint64_t timestamp)
{
	for (unordered_map<uint32_t, stRealtimeTrafficDataItem>::const_iterator it = umap_RealTrafficInfo.begin(); it != umap_RealTrafficInfo.end(); it++)
	{
		stTrafficInfo* pTrafficInfo = const_cast<stTrafficInfo*>(m_pDataMgr->GetTrafficInfo(it->second.kslink_id));
		if (pTrafficInfo) {
			pTrafficInfo->speed = it->second.bySpeed;
			pTrafficInfo->time_stamp = it->second.timestamp;

			for (unordered_set<uint64_t>::const_iterator link = pTrafficInfo->setLinks.begin(); link != pTrafficInfo->setLinks.end(); link++) {
				KeyID linkId{ *link };
				int dir = linkId.dir; // 1:정, 2:역
				linkId.dir = 0; // 우선, 방향성 적용해제 후 검사
				stLinkInfo* pLink = m_pDataMgr->GetVLinkDataById(linkId);
				if (!pLink) {
					// 방향성 적용 후 재 검사
					linkId.dir = dir;
					pLink = m_pDataMgr->GetVLinkDataById(linkId);
				}

#if defined(USE_P2P_DATA)
				if (pLink) {
					if (dir == 1) { // 정
						pLink->veh.speed_f = pTrafficInfo->speed;
						pLink->veh.rtt_f = 1;
					}
					else { // 역
						pLink->veh.speed_b = pTrafficInfo->speed;
						pLink->veh.rtt_b = 1;
					}					
				}
#endif // #if defined(USE_P2P_DATA)
			}
		}
	}

	currentTimestamp = timestamp;

	return true;
}

//INAVI is using Big endian data format for Real-time traffic information file (*.rtt)
//so that, we need to convert from little endian to big endian when ever reading over 2 byte integer
uint32_t swap_uint32(uint32_t val) { //little endian to big endian
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}


size_t CTrafficManager::LoadData(IN const char* szFileName)
{
	size_t timenow = 0;

	FILE* fp = fopen(szFileName, "rb");
	if (fp) {
		stRealtimeTrafficDataItem rtItem;
		uint8_t byTemp;
		uint32_t dwRead = 0;
		uint32_t ksLinkID;
		int nCount = 0;
		dwRead = fread(&rtItem.timestamp, 4, 1, fp);
		timenow = rtItem.timestamp = swap_uint32(rtItem.timestamp);
		dwRead = fread(&byTemp, 1, 1, fp);
		//std::unordered_map<uint32_t, stRealtimeTrafficDataItem> umap_RealTrafficInfo;
		for (;;) {
			if (dwRead != 1) break;
			dwRead = fread(&ksLinkID, 4, 1, fp); //big endian
			rtItem.kslink_id = swap_uint32(ksLinkID); //little endian
			dwRead = fread(&rtItem.bySpeed, 1, 1, fp);
			nCount++;
			umap_RealTrafficInfo[ksLinkID] = rtItem;
		} // for

		fclose(fp);
	}

	//if (nCount > 0) {
	//	set_realtime_traffic_data(umap_RealTrafficInfo);
	//}//if

	return timenow;
}
