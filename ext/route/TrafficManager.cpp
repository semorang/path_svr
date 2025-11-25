#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "TrafficManager.h"

#include "../utils/UserLog.h"

#if defined(USE_INAVI_STATIC_DATA)
#include "../thlib/ReadTTLSpd.h"
#endif

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CTrafficManager::CTrafficManager()
{
	currentTimestamp = 0;

	m_pDataMgr = nullptr;
}


CTrafficManager::~CTrafficManager()
{
	Release();
}


bool CTrafficManager::Initialize(void)
{
	bool ret = false;

	return ret;
}


void CTrafficManager::Release(void)
{

}


void CTrafficManager::SetDataMgr(IN CDataManager* pDataMgr)
{
	if (pDataMgr) {
		m_pDataMgr = pDataMgr;
	}
}


//INAVI is using Big endian data format for Real-time traffic information file (*.rtt)
//so that, we need to convert from little endian to big endian when ever reading over 2 byte integer
uint32_t swap_uint32(uint32_t val) { //little endian to big endian
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}


uint64_t swap_uint64(uint64_t val)
{
	val = (val & 0x00000000FFFFFFFF) << 32 | (val & 0xFFFFFFFF00000000) >> 32;
	val = (val & 0x0000FFFF0000FFFF) << 16 | (val & 0xFFFF0000FFFF0000) >> 16;
	val = (val & 0x00FF00FF00FF00FF) << 8 | (val & 0xFF00FF00FF00FF00) >> 8;
	return val;
}

//uint64_t ntoh64(const uint64_t *input)
//{
//	uint64_t rval;
//	uint8_t *data = (uint8_t *)&rval;
//
//	data[0] = *input >> 56;
//	data[1] = *input >> 48;
//	data[2] = *input >> 40;
//	data[3] = *input >> 32;
//	data[4] = *input >> 24;
//	data[5] = *input >> 16;
//	data[6] = *input >> 8;
//	data[7] = *input >> 0;
//
//	return rval;
//}


bool CTrafficManager::ParsingTTL(IN FILE* fp, IN const int32_t size, IN OUT uint32_t& timestamp)
{
	bool ret = false;

	const int timeGap = TRAFFIC_DATA_ALIVE;
	uint32_t tmReq = timestamp;
	uint32_t tmData = 0;
	stTrafficTTLItem trfItem;

	size_t sizeRead;
	size_t sizeItem = sizeof(trfItem);
	uint32_t nItemCount = size / sizeof(trfItem);

	if (tmReq <= 0) {
		tmReq = time(NULL);
	}

	for (int ii = 0; ii < nItemCount; ii++) {
		if ((sizeRead = fread(&trfItem, sizeItem, 1, fp)) != 1) {
			continue;
		}

		trfItem.id = swap_uint64(trfItem.id);

		if (ii == 0 && (trfItem.id / 1000000000.f < 10.f)) {
			tmData = trfItem.id;
			timestamp = tmData;

			if (tmReq < tmData) { // 데이터가 요청 시각보다 미래 데이터면 무시
				break;
			} else if (tmReq - timeGap > tmData) { // 데이터가 허용 범위 시각보다 과거 데이터면 무시
				break;
			}

			ret = true;
		} else {
			m_pDataMgr->UpdateTrafficTTLData(trfItem.id, trfItem.speed, tmData);
		}
	}

	if (ret == true) {
		m_pDataMgr->CheckTrafficAlive(tmReq - timeGap);
	}

	return ret;
}


bool CTrafficManager::ParsingKS(IN FILE* fp, IN const int32_t size, IN OUT uint32_t& timestamp)
{
	bool ret = false;

#if defined(USE_P2P_DATA)
	const int timeGap = 24 * 60 * 60; // 24시간
#else
	const int timeGap = TRAFFIC_DATA_ALIVE;
#endif
	uint32_t tmNow = timestamp;
	uint32_t tmData = 0;
	stTrafficKSItem trfItem;

	size_t sizeRead;
	size_t sizeItem = sizeof(trfItem);
	uint32_t nItemCount = size / sizeof(trfItem);

	if (tmNow <= 0) {
		tmNow = time(NULL);
	}

	for (int ii = 0; ii < nItemCount; ii++) {
		if ((sizeRead = fread(&trfItem, sizeItem, 1, fp)) != 1) {
			continue;
		}

		trfItem.id = swap_uint32(trfItem.id);

		if ((ii == 0) && (trfItem.id != 0) && (trfItem.speed == SPEED_NOT_AVALABLE)) {
			tmData = trfItem.id;// swap_uint32(trfItem.id);
			timestamp = tmData;

			if (tmNow < tmData) { // 데이터가 요청 시각보다 미래 데이터면 무시
				break;
			} else if (tmNow > tmData + timeGap) { // 데이터가 허용 범위 시각보다 과거 데이터면 무시 
				break;
			}

			ret = true;
		} else {
			m_pDataMgr->UpdateTrafficKSData(trfItem.id, trfItem.speed, tmData);
		}
	}

	return ret ;
}


bool CTrafficManager::LoadData(IN const char* szFilePath)
{
	bool  ret = false;

	if (m_pDataMgr != nullptr) {
		m_pDataMgr->LoadStaticData(szFilePath);

		ret = true;
	}

	return ret;
}


size_t CTrafficManager::Update(IN const char* szFileName, IN const int type, IN const uint32_t timestamp)
{
	uint32_t timenow = timestamp;

	FILE* fp = fopen(szFileName, "rb");
	if (fp) {
		if ((type == TYPE_SPEED_REAL_TTL) || (type == TYPE_SPEED_REAL_KS)) {
			stTrafficDataHeader trfHeader;
			size_t dwRead = 0;
			int nReadValue = 0;

			// header - type
			if ((dwRead = fread(&nReadValue, sizeof(nReadValue), 1, fp)) != 1) {
				fclose(fp);
				return 0;
			}
			trfHeader.type = swap_uint32(nReadValue);

			// header - size
			if ((dwRead = fread(&nReadValue, sizeof(nReadValue), 1, fp)) != 1) {
				fclose(fp);
				return 0;
			}
			trfHeader.size = swap_uint32(nReadValue);

			// body
			bool ret = false;
			if (trfHeader.type == TYPE_TRAFFIC_TTL_ID) {
				ret = trfHeader.timestamp = ParsingTTL(fp, trfHeader.size, timenow);
			} else if (trfHeader.type == TYPE_TRAFFIC_KS_ID) {
				ret = trfHeader.timestamp = ParsingKS(fp, trfHeader.size, timenow);
			}

			// 요청시각 이전 15분부터 ~ 요청시각 까지 로딩
			if (ret == true) {
				time_t timer = timenow;
				struct tm* tmData = localtime(&timer);
				LOG_TRACE(LOG_KEY_TRAFFIC, LOG_DEBUG, "traffic data read: %04d-%02d-%02d %02d:%02d:%02d", tmData->tm_year + 1900, tmData->tm_mon + 1, tmData->tm_mday, tmData->tm_hour, tmData->tm_min, tmData->tm_sec);
			}

			fclose(fp);
		} else { // KS - GET으로 받은 파일 포맷일 경우,
				 //stRealtimeTrafficDataItem rtItem;
				 //uint8_t byTemp;
				 //size_t dwRead = 0;
				 //uint32_t ksLinkID;
				 //int nCount = 0;

				 //// header
				 //dwRead = fread(&rtItem.timestamp, 4, 1, fp);
				 //timenow = rtItem.timestamp = swap_uint32(rtItem.timestamp);
				 //dwRead = fread(&byTemp, 1, 1, fp);
				 //for (;;) {
				 //	if (dwRead != 1) break;
				 //	dwRead = fread(&ksLinkID, 4, 1, fp); //big endian
				 //	rtItem.kslink_id = swap_uint32(ksLinkID); //little endian
				 //	dwRead = fread(&rtItem.bySpeed, 1, 1, fp);
				 //	nCount++;
				 //	umap_RealTrafficInfo[ksLinkID] = rtItem;
				 //} // for
		}
	}

	//if (nCount > 0) {
	//	set_realtime_traffic_data(umap_RealTrafficInfo);
	//}//if

	return timenow;
}
