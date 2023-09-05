#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FileTraffic.h"

#include "../shp/shpio.h"
#include "../utils/UserLog.h"
#include "../utils/Strings.h"
#include "../route/MMPoint.hpp"
#include "../route/DataManager.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CFileTraffic::CFileTraffic()
{
	m_nEntIdx = 0;
	m_nFileType = TYPE_DATA_TRAFFIC;
}

CFileTraffic::~CFileTraffic()
{

}

static const uint32_t g_cntLogPrint = 100000;

bool CFileTraffic::ParseData(IN const char* fname)
{
	bool ret = false;

	m_nEntIdx = 0;

	FILE* fp = nullptr;
	fp = fopen(fname, "rt");

	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Error, Cannot Find shp File!");
		return ret;
	}


	//데이터 얻기
	LOG_TRACE(LOG_DEBUG, "LOG, start, raw data parsing file : %s", fname);


	uint32_t lineCount = 0;
	size_t lenTok = 0;
	const int32_t maxBuffCount = 1024;
	char szBuff[maxBuffCount] = { 0, };
	char token[] = { "," };
	char* pLine = nullptr;


	// read header line
	fgets(szBuff, maxBuffCount - 1, fp);

	pLine = szBuff;
	char* pTok = strsep(&pLine, token);
	lenTok = strlen(pTok);
	if (lenTok <= 0) {
		LOG_TRACE(LOG_ERROR, "Failed, traffic file header not exist", lineCount);
		return ret;
	}

	//if (strcmp(pTok, "B_ENT_ID") == 0) {
	//	m_nEntType = TYPE_ENT_BUILDING;
	//} else if ((strcmp(pTok, "SID_CD") == 0) || (strcmp(pTok, "SIG_CD") == 0) || (strcmp(pTok, "SIG") == 0)) { // SIG 인지 SID 인지 확인 후 하나만 사용
	////} else if ((strcmp(pTok, "SIG") == 0) {
	//	m_nEntType = TYPE_ENT_COMPLEX;
	//} else {
	//	LOG_TRACE(LOG_ERROR, "Failed, entrance file type not defined");
	//	return ret;
	//}
	//

	stTrafficInfo trafficInfo;

	while (fgets(szBuff, maxBuffCount - 1, fp) != nullptr)
	{
		lineCount++;

		if (strlen(szBuff) <= 0) {
			continue;
		}

		pLine = szBuff;

		stTraffic rtt_data;

		// MESH
		pTok = strsep(&pLine, token);
		lenTok = strlen(pTok);
		if (lenTok <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, traffic mesh id empty, line:%d", lineCount);
			continue;
		}
		rtt_data.Mesh = atoi(pTok);

		// SNODE_ID
		pTok = strsep(&pLine, token);
		lenTok = strlen(pTok);
		if (lenTok <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, traffic snode id emtpy, line:%d", lineCount);
			continue;
		}
		rtt_data.Snode_id = atoi(pTok);

		// ENODE_ID
		pTok = strsep(&pLine, token);
		lenTok = strlen(pTok);
		if (lenTok <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, traffic enode id emtpy, line:%d", lineCount);
			continue;
		}
		rtt_data.Enode_id = atoi(pTok);

		// DIR
		pTok = strsep(&pLine, token);
		lenTok = strlen(pTok);
		if (lenTok <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, traffic diremtpy, line:%d", lineCount);
			continue;
		}
		rtt_data.Dir_code = atoi(pTok);

		// KSLINK_ID
		pTok = strsep(&pLine, token);
		lenTok = strlen(pTok);
		if (lenTok <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, traffic KS link id emtpy, line:%d", lineCount);
			continue;
		}
		rtt_data.KSLink_id = atoi(pTok);

		// add to container
		m_vtTraffic.emplace_back(rtt_data);
		
		m_nEntIdx++;
	}

	return GenServiceData();
}


bool CFileTraffic::GenServiceData()
{
	// 클래스에 추가
	LOG_TRACE(LOG_DEBUG, "LOG, start, add entrance to polygon class");

	static const int32_t nMaxEntDist = 3000; // 건물/단지와 최대 1000m 이상 차이가 나면 오류

	stTrafficKey trafficKey;

	for (vector<stTraffic>::const_iterator it = m_vtTraffic.begin(); it != m_vtTraffic.end(); it++)
	{
		// key
		//int64_t key_id = it->Mesh;
		//key_id = key_id * 1000000 + it->Snode_id;
		//key_id = key_id * 1000000 + it->Enode_id;
		//key_id = key_id * 1000000 + it->Dir_code;

		//trafficInfo.key_id = key_id;

		trafficKey.mesh = it->Mesh;
		trafficKey.snode = it->Snode_id;
		trafficKey.enode = it->Enode_id;
		trafficKey.dir = it->Dir_code;

		// add data to map container
		m_pDataMgr->AddTrafficMatchData(trafficKey, it->KSLink_id);
	} // for

	Release();

	LOG_TRACE(LOG_DEBUG, "LOG, finished");

	return true;
}


void CFileTraffic::AddDataFeild(IN const int idx, IN const int type, IN const char* colData)
{

}


void CFileTraffic::AddDataRecord()
{

}


bool CFileTraffic::LoadData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_TRAFFIC], g_szTypeExec[TYPE_DATA_TRAFFIC]);

	return CFileBase::LoadData(szFileName);
}


size_t CFileTraffic::ReadBody(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody = { 0, };

	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeBody = sizeof(FileBody);

	stTrafficMesh* pTrafficMesh = nullptr;

	for (int32_t idx = 0; idx < m_vtIndex.size(); idx++)
	{
		// read body
		if (m_vtIndex[idx].szBody <= 0) {
			continue;
		}
		else if (m_vtIndex[idx].offBody <= 0)
		{
			LOG_TRACE(LOG_ERROR, "Failed, index body info invalid, off:%d, size:%d", m_vtIndex[idx].offBody, m_vtIndex[idx].szBody);
			return 0;
		}

		fseek(fp, m_vtIndex[idx].offBody, SEEK_SET);
		if ((retRead = fread(&fileBody, sizeof(fileBody), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read body, offset:%d", m_vtIndex[idx].offBody);
			return 0;
		}

		//if (m_vtIndex[idx].idTile != fileBody.idTile)
		//{
		//	LOG_TRACE(LOG_ERROR, "Failed, index tile info not match with body, index tile id:%d vs body tile id:%d", m_vtIndex[idx].idTile, fileBody.idTile);
		//	return 0;
		//}

		uint32_t value32;
		uint64_t value64;

		// traffic
		for (int32_t ii = 0; ii < fileBody.link.cntNode; ii++)
		{
			stTrafficInfo* pTraffic = new stTrafficInfo();

			// read ks link id
			if ((retRead = fread(&value32, sizeof(value32), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read traffic ks link, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}
			pTraffic->ks_id = value32;

			// read traffic key
			if ((retRead = fread(&value64, sizeof(value64), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read traffic key, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}
			pTraffic->key.llid = value64;

			// read link count
			if ((retRead = fread(&value32, sizeof(value32), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read traffic link count, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			// read link
			int32_t cntLinks = value32;
			for (int32_t jj = 0; jj < cntLinks; jj++) {
				if ((retRead = fread(&value64, sizeof(value64), 1, fp)) != 1)
				{
					LOG_TRACE(LOG_ERROR, "Failed, can't read traffic match link, mesh_id:%d, ks_id:%d, idx:%d", m_vtIndex[idx].idTile, pTraffic->ks_id, jj);
					return 0;
				}
				pTraffic->setLinks.emplace(value64);
			}

			// add to traffic map
			m_pDataMgr->AddTrafficData(pTraffic);
		} // for

		offFile += m_vtIndex[idx].szBody;

#if defined(_DEBUG)
		LOG_TRACE(LOG_DEBUG, "Data loading, traffic data loaded, mesh:%d, ks:%d, link:%d", m_vtIndex[idx].idTile, fileBody.link.cntNode, fileBody.link.cntLink);
#endif
	} // for

	return offFile;
}


bool CFileTraffic::Initialize()
{
	return CFileBase::Initialize();
}


void CFileTraffic::Release()
{	
	if (!m_vtTraffic.empty()) {
		m_vtTraffic.clear();
		vector<stTraffic>().swap(m_vtTraffic);
	}

	m_nEntIdx = 0;

	CFileBase::Release();
}


bool CFileTraffic::SaveData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_TRAFFIC], g_szTypeExec[TYPE_DATA_TRAFFIC]);

	return CFileBase::SaveData(szFileName);
}


size_t CFileTraffic::WriteBody(FILE* fp, IN const uint32_t fileOff)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody;

	size_t offFile = fileOff;
	size_t retWrite = 0;
	size_t retRead = 0;
	long offItem = 0;
	const size_t sizeFileBody = sizeof(fileBody);

	uint32_t ii = 0;

	uint32_t value32;
	uint64_t value64;

	
	// write body
	memset(&fileBody, 0x00, sizeof(fileBody));
	//jj = 0;

	fileBody.idTile = 0;

	if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't write body[%d], tile:%d, written:%d", ii, fileBody.idTile, retWrite);
		return 0;
	}
	offItem = sizeFileBody;


	// write body - traffic
	for (unordered_map<uint32_t, stTrafficInfo*>::const_iterator it = m_pDataMgr->GetTrafficMapData()->begin(); it != m_pDataMgr->GetTrafficMapData()->end(); it++)
	{
		// ks id
		value32 = it->second->ks_id;
		if ((retWrite = fwrite(&value32, sizeof(value32), 1, fp)) != 1) {
			LOG_TRACE(LOG_ERROR, "Failed, can't write traffic ks link, id:%d", value32);
			return 0;
		}
		offItem += sizeof(value32);

		// traffic key
		value64 = it->second->key.llid;
		if ((retWrite = fwrite(&value64, sizeof(value64), 1, fp)) != 1) {
			LOG_TRACE(LOG_ERROR, "Failed, can't write traffic traffic key, tile:%d, snode:%d, enode:%d, dir:%d", it->second->key.mesh, it->second->key.snode, it->second->key.enode, it->second->key.dir);
			return 0;
		}
		offItem += sizeof(value64);

		// match link count
		value32 = it->second->setLinks.size();
		if ((retWrite = fwrite(&value32, sizeof(value32), 1, fp)) != 1) {
			LOG_TRACE(LOG_ERROR, "Failed, can't write match link count, %d", value32);
			return 0;
		}
		offItem += sizeof(value32);

		// match link
		for (unordered_set<uint64_t>::const_iterator link = it->second->setLinks.begin(); link != it->second->setLinks.end(); link++) {
			value64 = *link;
			if ((retWrite = fwrite(&value64, sizeof(value64), 1, fp)) != 1) {
				LOG_TRACE(LOG_ERROR, "Failed, can't write match link, id:%d", value64);
				return 0;
			}
			offItem += sizeof(value64);

			fileBody.link.cntLink++;
		}

		fileBody.link.cntNode++; // ks link		

		//if (jj > 0) {
		//	LOG_TRACE(LOG_DEBUG, "Save data, complex entrance, cnt:%lld", jj);
		//}

	} // for

	  // re-write body size & off
	if (offItem > sizeFileBody)
	{
		fileBody.szData = offItem;

		fseek(fp, offItem * -1, SEEK_CUR);

		if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't re-write traffic info, body[%d], tile_id:%d", ii, fileBody.idTile);
			return 0;
		}

		fseek(fp, offItem - sizeFileBody, SEEK_CUR);
	}

	// re-set index size & off buff
	m_vtIndex[ii].szBody = fileBody.szData;
	m_vtIndex[ii].offBody = offFile;

	// update file offset
	offFile += offItem;

	// re-write index for body size and offset
	fseek(fp, m_fileHeader.offIndex, SEEK_SET);
	for (uint32_t idx = 0; idx < m_fileHeader.cntIndex; idx++)
	{
		FileIndex fileIndex;
		memcpy(&fileIndex, &m_vtIndex[idx], sizeof(fileIndex));
		if ((retRead = fwrite(&fileIndex, sizeof(fileIndex), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't re-write file index data, idx:%d", idx);
			return 0;
		}
	}

	return offFile;
}
