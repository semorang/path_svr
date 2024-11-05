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
	m_nDataType = TYPE_DATA_TRAFFIC;
	m_nFileType = TYPE_EXEC_TRAFFIC;
}

CFileTraffic::~CFileTraffic()
{

}

bool CFileTraffic::ParseData(IN const char* fname)
{
	bool ret = false;

	FILE* fp = nullptr;
	fp = fopen(fname, "rt");

	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Error, Cannot Find shp File!");
		return ret;
	}


	// 중간 데이터 로딩 - // mesh_id + snode_id + enode_id, link_nid
	char szNodeToLinkMatching[MAX_PATH] = { 0, };
	sprintf(szNodeToLinkMatching, "%s/%s", m_szWorkPath, "node-link-matching.bin");

	LOG_TRACE(LOG_DEBUG, "LOG, start, node link matching data file : %s", szNodeToLinkMatching);

	FILE* fpNodeLinkMatching = fopen(szNodeToLinkMatching, "rb");
	if (!fpNodeLinkMatching) {
		LOG_TRACE(LOG_ERROR, "Error, can't open node to link matching File!, %s", szNodeToLinkMatching);
		return false;
	}

	// make node to link matching map file
	unordered_map<uint64_t, uint64_t> umapNodeLinkMatching;
	uint64_t nodeId, linkId;
	while (fread(&nodeId, sizeof(nodeId), 1, fpNodeLinkMatching)) {
		// link_id
		if (fread(&linkId, sizeof(linkId), 1, fpNodeLinkMatching)) {
			// add
			umapNodeLinkMatching.emplace(nodeId, linkId);
		}
	}
	// close file
	if (fpNodeLinkMatching) {
		fclose(fpNodeLinkMatching);
	}

	LOG_TRACE(LOG_DEBUG, "LOG, finished, node link matching data file");


	//데이터 얻기
	LOG_TRACE(LOG_DEBUG, "LOG, start, raw data parsing file : %s", fname);


	uint32_t lineCount = 0;
	size_t lenTok = 0;
	const int32_t maxBuffCount = 1024;
	char szBuff[maxBuffCount] = { 0, };
	char token[] = { ",|" };
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

	stTrafficInfoKS trafficInfo;

	while (fgets(szBuff, maxBuffCount - 1, fp) != nullptr)
	{
		lineCount++;

		if (strlen(szBuff) <= 0) {
			continue;
		}

		pLine = szBuff;

		// MESH
		uint32_t meshId = 0;
		pTok = strsep(&pLine, token);
		lenTok = strlen(pTok);
		if (lenTok <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, traffic mesh id empty, line:%d", lineCount);
			continue;
		}
		meshId = atoi(pTok);

		// SNODE_ID
		uint32_t snodeId = 0;
		pTok = strsep(&pLine, token);
		lenTok = strlen(pTok);
		if (lenTok <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, traffic snode id empty, line:%d", lineCount);
			continue;
		}
		snodeId = atoi(pTok);

		// ENODE_ID
		uint32_t enodeId = 0;
		pTok = strsep(&pLine, token);
		lenTok = strlen(pTok);
		if (lenTok <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, traffic enode id empty, line:%d", lineCount);
			continue;
		}
		enodeId = atoi(pTok);

		// DIR
		uint8_t dir = 0;
		pTok = strsep(&pLine, token);
		lenTok = strlen(pTok);
		if (lenTok <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, traffic dir empty, line:%d", lineCount);
			continue;
		}
		dir = atoi(pTok);

		// KSLINK_ID
		uint32_t ks_id = 0;
		pTok = strsep(&pLine, token);
		lenTok = strlen(pTok);
		if (lenTok <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, traffic KS link id empty, line:%d", lineCount);
			continue;
		}
		ks_id = atoi(pTok);


		// get link_id
		KeyID linkId = { 0, };
		char szMeshNodeId[32] = { 0, };
		sprintf(szMeshNodeId, "%06d%06d%06d", meshId, snodeId, enodeId);
		uint64_t meshnodeId = atoll(szMeshNodeId);
		unordered_map<uint64_t, uint64_t>::const_iterator it = umapNodeLinkMatching.find(meshnodeId);
		if (it == umapNodeLinkMatching.end()) {
			// 테스트 메쉬가 있으면 정의된 메쉬만 확인하자
			if (g_isUseTestMesh && !g_arrTestMesh.empty()) {
				int meshId = meshnodeId / 1000000000000;
				if (g_arrTestMesh.find(meshId) != g_arrTestMesh.end()) {
					LOG_TRACE(LOG_WARNING, "ERROR, can't find mesh+snode+enode id in mapping data, data:%lld, line:%d", meshnodeId, lineCount);
				}
			}
			continue;
		}
		linkId.llid = it->second;

		// add to container
		m_pDataMgr->AddTrafficKSData(ks_id, linkId.tile_id, linkId.nid, dir);
	}

	if (!umapNodeLinkMatching.empty()) {
		umapNodeLinkMatching.clear();
		unordered_map<uint64_t, uint64_t>().swap(umapNodeLinkMatching);
	}

	LOG_TRACE(LOG_DEBUG, "LOG, finished, raw data parsing file");

	return GenServiceData();
}


bool CFileTraffic::GenServiceData()
{
	// 클래스에 추가
	LOG_TRACE(LOG_DEBUG, "LOG, start, add traffic data to class");

	// ks 데이터는 메쉬 구분이 안되기에 전역 메쉬로 포함한다.
	const int meshId = 0;
	if (m_pDataMgr->GetMeshDataById(meshId) == nullptr) {
		stMeshInfo* pMesh = new stMeshInfo();
		pMesh->mesh_id.tile_id = meshId;
		m_pDataMgr->AddMeshData(pMesh);
	}

	Release();

	LOG_TRACE(LOG_DEBUG, "LOG, finished");

	return true;
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

	uint8_t value8;
	uint32_t value32;
	uint64_t value64;

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

		if (m_vtIndex[idx].idTile <= 0) {
			// traffic ks
			for (int32_t ii = 0; ii < fileBody.traffic.cntTraffic; ii++) {
				// read ks id
				if ((retRead = fread(&value32, sizeof(value32), 1, fp)) != 1) {
					LOG_TRACE(LOG_ERROR, "Failed, can't read traffic ks is, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
					return 0;
				}
				uint32_t ks_id = value32;

				// read link count
				if ((retRead = fread(&value32, sizeof(value32), 1, fp)) != 1) {
					LOG_TRACE(LOG_ERROR, "Failed, can't read traffic link count, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
					return 0;
				}

				// read link
				int32_t cntLinks = value32;
				for (int32_t jj = 0; jj < cntLinks; jj++) {
					if ((retRead = fread(&value64, sizeof(value64), 1, fp)) != 1) {
						LOG_TRACE(LOG_ERROR, "Failed, can't read traffic match link, mesh_id:%d, ks_id:%lld, idx:%d", m_vtIndex[idx].idTile, ks_id, jj);
						return 0;
					}
					KeyID link = { value64 };
					// add to traffic map
					m_pDataMgr->AddTrafficKSData(ks_id, link.tile_id, link.nid, link.dir);
				}				
			} // for

			offFile += m_vtIndex[idx].szBody;
		} else {
			// traffic ttl
			for (int32_t ii = 0; ii < fileBody.traffic.cntTraffic; ii++) {
				// read ttl id
				if ((retRead = fread(&value32, sizeof(value32), 1, fp)) != 1) {
					LOG_TRACE(LOG_ERROR, "Failed, can't read traffic ttl id, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
					return 0;
				}
				uint32_t ttl_nid = value32;

				// read link dir
				if ((retRead = fread(&value8, sizeof(value8), 1, fp)) != 1) {
					LOG_TRACE(LOG_ERROR, "Failed, can't read traffic ttl dir, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
					return 0;
				}
				uint8_t dir = value8;

				// read link id
				if ((retRead = fread(&value32, sizeof(value32), 1, fp)) != 1) {
					LOG_TRACE(LOG_ERROR, "Failed, can't read traffic link id, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
					return 0;
				}
				uint32_t link_nid = value32;
				
				// add to traffic ttl map
				m_pDataMgr->AddTrafficTTLData(ttl_nid, m_vtIndex[idx].idTile, link_nid, dir);
			} // for
		}
		offFile += m_vtIndex[idx].szBody;

#if defined(_DEBUG)
		//LOG_TRACE(LOG_DEBUG, "Data loading, traffic data loaded, mesh:%d, ks:%d, link:%d", m_vtIndex[idx].idTile, fileBody.traffic.cntTraffic, fileBody.traffic.cntMatching);
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
	CFileBase::Release();
}


bool CFileTraffic::LoadData(IN const char* szFilePath)
{
	bool ret = CFileBase::LoadData(szFilePath);

	if (m_pDataMgr != nullptr) {
		ret = m_pDataMgr->LoadStaticData(szFilePath);
	}

	return ret;
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

	uint32_t idx = 0;

	uint8_t value8;
	uint32_t value32;
	uint64_t value64;


	if (!m_pDataMgr->GetTrafficKSMapData()->empty()) {
		// write body
		memset(&fileBody, 0x00, sizeof(fileBody));

		// ks id는 메쉬 구별없이 한판으로 관리됨
		fileBody.idTile = 0;

		// write dummy body
		if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1) {
			LOG_TRACE(LOG_ERROR, "Failed, can't write ks dummy body[%d], idx:%d, written:%d", idx, retWrite);
			return 0;
		}
		offItem = sizeFileBody;

		// write body
		for (unordered_map<uint32_t, stTrafficInfoKS*>::const_iterator it = m_pDataMgr->GetTrafficKSMapData()->begin(); it != m_pDataMgr->GetTrafficKSMapData()->end(); it++) {
			// ks id
			value32 = it->second->ks_id;
			if ((retWrite = fwrite(&value32, sizeof(value32), 1, fp)) != 1) {
				LOG_TRACE(LOG_ERROR, "Failed, can't write traffic ks_id: %d", value32);
				return 0;
			}
			offItem += sizeof(value32);

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

				fileBody.traffic.cntMatching++;
			} // for
			fileBody.traffic.cntTraffic++;
		} // for

		// re-write body size & off
		if (offItem > sizeFileBody) {
			fileBody.szData = offItem;

			fseek(fp, offItem * -1, SEEK_CUR);

			if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1) {
				LOG_TRACE(LOG_ERROR, "Failed, can't re-write traffic info, body[%d], tile_id:%d", idx, fileBody.idTile);
				return 0;
			}

			fseek(fp, offItem - sizeFileBody, SEEK_CUR);
		}

		// re-set index
		m_vtIndex[idx].idxTile = idx;
		m_vtIndex[idx].idTile = fileBody.idTile;
		m_vtIndex[idx].szBody = fileBody.szData;
		m_vtIndex[idx].offBody = offFile;

		// update file offset
		offFile += offItem;
		idx++;
	}


	// write ks & ttl
	for (unordered_map<uint32_t, stTrafficMesh*>::const_iterator it = m_pDataMgr->GetTrafficMeshData()->begin(); it != m_pDataMgr->GetTrafficMeshData()->end(); it++) {

		fileBody.idTile = it->first;
		fileBody.traffic.cntTraffic = 0;
		fileBody.traffic.cntMatching = 0;

		// write dummy body 
		if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1) {
			LOG_TRACE(LOG_ERROR, "Failed, can't write ks dummy body[%d], idx:%d, written:%d", idx, retWrite);
			return 0;
		}
		offItem = sizeFileBody;

		for (unordered_map<uint32_t, stTrafficInfoTTL*>::const_iterator itTTL = it->second->mapTrafficTTL.begin(); itTTL != it->second->mapTrafficTTL.end(); itTTL++) {
			// write ttl id
			value32 = itTTL->second->ttl_nid; // use only values excluding tile value;
			if ((retWrite = fwrite(&value32, sizeof(value32), 1, fp)) != 1) {
				LOG_TRACE(LOG_ERROR, "Failed, can't write traffic ttl id, body[%d], ttl_nid:%d, written:%d", idx, itTTL->second->ttl_nid, retWrite);
				return 0;
			}
			offItem += sizeof(value32);

			// write ttl dir
			value8 = itTTL->second->dir;
			if ((retWrite = fwrite(&value8, sizeof(value8), 1, fp)) != 1) {
				LOG_TRACE(LOG_ERROR, "Failed, can't write traffic ttl dir, body[%d], ttl_nid:%d, dir:%d, written:%d", idx, itTTL->second->ttl_nid, itTTL->second->dir, retWrite);
				return 0;
			}
			offItem += sizeof(value8);

			// wrtie link id
			value32 = itTTL->second->link_nid;
			if ((retWrite = fwrite(&value32, sizeof(value32), 1, fp)) != 1) {
				LOG_TRACE(LOG_ERROR, "Failed, can't write traffic ttl link, body[%d], ttl_nid:%d, link_nid:%d, written:%d", idx, itTTL->second->ttl_nid, itTTL->second->link_nid, retWrite);
				return 0;
			}
			offItem += sizeof(value32);

			fileBody.traffic.cntTraffic++;
			fileBody.traffic.cntMatching++;
		} // for


		// re-write body size & off
		if (offItem > sizeFileBody) {
			fileBody.szData = offItem;

			fseek(fp, offItem * -1, SEEK_CUR);
			if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1) {
				LOG_TRACE(LOG_ERROR, "Failed, can't re-write traffic info, body[%d], tile_id:%d", idx, fileBody.idTile);
				return 0;
			}
			fseek(fp, offItem - sizeFileBody, SEEK_CUR);
		}

		// re-set index size & off buff
		m_vtIndex[idx].idxTile = idx;
		m_vtIndex[idx].idTile = fileBody.idTile;
		m_vtIndex[idx].szBody = fileBody.szData;
		m_vtIndex[idx].offBody = offFile;

		// update file offset
		offFile += offItem;
		idx++;
	} // for


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
