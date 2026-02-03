#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FileVehicleEx.h"

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

using namespace std;

CFileVehicleEx::CFileVehicleEx()
{
	m_nDataType = TYPE_DATA_VEHICLE;
	m_nFileType = TYPE_EXEC_EXTEND;
}

CFileVehicleEx::~CFileVehicleEx()
{

}

bool CFileVehicleEx::ParseData(IN const char* fname)
{
	bool ret = false;

	if ((!fname) || (strstr(fname, "VLINK_IN") == nullptr)) {
		return ret;
	}

	FILE* fp = nullptr;
	fp = fopen(fname, "rt");

	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Error, Cannot open vehicle extend file : %s", fname);
		return ret;
	}

	// 중간 데이터 로딩 - // mesh_id + snode_id + enode_id, link_nid
	char szNodeToLinkMatching[MAX_PATH] = { 0, };
	sprintf(szNodeToLinkMatching, "%s/%s", m_szWorkPath, "node-link-matching.bin");

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "LOG, start, node link matching data file : %s", szNodeToLinkMatching);

	FILE* fpNodeLinkMatching = fopen(szNodeToLinkMatching, "rb");
	if (!fpNodeLinkMatching) {
		LOG_TRACE(LOG_ERROR, "Error, can't open node to link matching File!, %s", szNodeToLinkMatching);
		return false;
	}

	// make node to link matching map file
	unordered_map<uint64_t, uint64_t> umapNodeLinkMatching;
	uint64_t nodeId, linkId;
	while (fread(&nodeId, sizeof(nodeId), 1, fpNodeLinkMatching))
	{
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
	LOG_TRACE(LOG_DEBUG, "LOG, start, link ex data parsing file : %s", fname);

	uint32_t lineCount = 0;
	size_t lenTok = 0;
	const int32_t maxBuffCount = 1024;
	char szBuff[maxBuffCount] = { 0, };
	char szData[maxBuffCount] = { 0, };
	char token[] = { "," };
	char* pLine = nullptr;
	char* pTok = nullptr;

	while (fgets(szBuff, maxBuffCount - 1, fp) != nullptr)
	{
		lineCount++;

		stVehicleEx itemVehicleEx;

		// 21 = mesh(6) + snodeid(6) + enodeid(6) + dir(1) + attribute(2) 기본 정보
		if (strlen(szBuff) <= 21) {
			LOG_TRACE(LOG_WARNING, "ERROR, link ex data wrong, buff:%s, line:%d", szBuff, lineCount);
			continue;
		}


		// mesh + snode + enode
		int nOffset = 0;
		int nSize = 18;

		memset(szData, 0x00, sizeof(szData));
		strncpy(szData, szBuff + nOffset, nSize);
		lenTok = strlen(szData);
		if (lenTok < nSize) {
			LOG_TRACE(LOG_WARNING, "ERROR, mesh+snode+enode data wrong, line:%d", lineCount);
			continue;
		}
		uint64_t meshnodeId = atoll(szData);
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

		// add link_id
		itemVehicleEx.link_id.llid = it->second;


		// dir
		nOffset += nSize;
		nSize = 1;
		memset(szData, 0x00, sizeof(szData));
		strncpy(szData, szBuff + nOffset, nSize);
		lenTok = strlen(szData);
		if (lenTok < nSize) {
			LOG_TRACE(LOG_WARNING, "ERROR, dir data wrong, line:%d", lineCount);
			continue;
		}
		int32_t dir = atoi(szData);

		// add dir
		itemVehicleEx.nDirCode = dir;


		// attribute
		nOffset += nSize;
		nSize = 2;
		memset(szData, 0x00, sizeof(szData));
		strncpy(szData, szBuff + nOffset, nSize);
		lenTok = strlen(szData);
		if (lenTok < nSize) {
			LOG_TRACE(LOG_WARNING, "ERROR, attribute data wrong, data:%s, line:%d", szData, lineCount);
			continue;
		}
		int32_t attr = atoi(szData);

		// add attr
		itemVehicleEx.nAttribute = attr;

		/*
		01	중량제한	중량 제한 정보	T(톤) 단위
		02	높이제한	높이 제한 정보	M(미터) 단위
		03	교량	교량 명칭 정보	명칭
		04	터널	터널 명칭 정보	명칭
		05	고가도로	고가도로 명칭 정보	명칭
		06	지하차도	지하차도 명칭 정보	명칭
		07	버스전용차로	버스 전용 차로 시간 정보	버스 전용 차로 시간 (CODE로 구분)
		08	고속모드	내비게이션 고속모드 정보	명칭
		09	고속모드부가	고속모드 휴게소 상세 정보	고속모드 휴게소 상세 정보 (CODE로 구분)
		10	차선정보	차선 정보	링크의 진입 기준 차선 정보 (CODE로 구분)
		11	방면정보	방면 정보	링크의 진출 기준 방면 정보 (표지판)
		12	차로유도차선	차로 유도 차선 정보	링크의 진입 기준 컬러 차선 정보
		13	차로유도방면	차로 유도 방면 정보	링크의 진출 기준 차로 유도선 방면 정보 (노면)
		14	톨게이트	톨게이트 명칭 정보	명칭
		15	확대도	확대도 명칭 정보	확대도 파일 명칭
		16	TPEG	표준노드링크의 링크 ID	표준노드링크 LINK_ID (10자리)
		17	진입제한	진입제한 정보	진입제한 정보 (CODE로 구분)
		*/

		// 우선 트럭 제한 정보만 사용하자, (2024-07-09)
		bool isOk = false;
		switch (attr) {
		case 1:
		{
			// weight
			nOffset += nSize;
			nSize = strlen(szBuff) - nOffset;
			memset(szData, 0x00, sizeof(szData));
			strncpy(szData, szBuff + nOffset, nSize);
			lenTok = strlen(szData);
			if (lenTok <= 0) {
				LOG_TRACE(LOG_WARNING, "ERROR, weight data wrong, data:%s, line:%d", szData, lineCount);
				break;
			}
			double weight = atof(szData);
			if (weight <= 0.f) {
				LOG_TRACE(LOG_WARNING, "ERROR, weight value wrong, value:%f, line:%d", weight, lineCount);
				break;
			}

			// add weight
			itemVehicleEx.dWeight = weight;
			isOk = true;
		} break;

		case 2:
		{
			// height
			nOffset += nSize;
			nSize = strlen(szBuff) - nOffset;
			memset(szData, 0x00, sizeof(szData));
			strncpy(szData, szBuff + nOffset, nSize);
			lenTok = strlen(szData);
			if (lenTok <= 0) {
				LOG_TRACE(LOG_WARNING, "ERROR, weight data wrong, data:%s, line:%d", szData, lineCount);
				break;
			}
			double height = atof(szData);
			if (height <= 0.f) {
				LOG_TRACE(LOG_WARNING, "ERROR, height value wrong, value:%f, line:%d", height, lineCount);
				break;
			}

			// add weight
			itemVehicleEx.dHeight = height;
			isOk = true;
		} break;

		default:
			break;
		} // switch

		if (isOk) {
			m_vtVehicleEx.emplace_back(itemVehicleEx);
		}
	} // while

	if (!umapNodeLinkMatching.empty()) {
		umapNodeLinkMatching.clear();
		unordered_map<uint64_t, uint64_t>().swap(umapNodeLinkMatching);
	}

	LOG_TRACE(LOG_DEBUG, timeStart, "LOG, finished, link ex data parsing file");

	return GenServiceData();
}


bool CFileVehicleEx::GenServiceData()
{
	// 클래스에 추가
	time_t timeStart = LOG_TRACE(LOG_DEBUG, "LOG, start, add vehicle ex data to class");

	const int cntItem = m_vtVehicleEx.size();

	size_t tick_total = 0;

	for (const auto& item : m_vtVehicleEx) {
		stExtendInfo info;
		info.keyId = item.link_id;
		if (item.dWeight > 0.f) {
			info.vtType.emplace_back(TYPE_LINKEX_WEIGHT);
			info.vtValue.emplace_back(item.dWeight);
		}
		if (item.dHeight > 0.f) {
			info.vtType.emplace_back(TYPE_LINKEX_HEIGHT);
			info.vtValue.emplace_back(item.dHeight);
		}

		// 저장할때는 동일 링크라도 개별 라인으로 관리되기에 벡터에는 1개의 값만 존재함
		if (info.vtType.size() > 1) {
			LOG_TRACE(LOG_DEBUG, "------------------------ ERROR -------------------------");
		}
		m_pDataMgr->AddLinkExData(info.keyId, info.vtType[0], info.vtValue[0], TYPE_DATA_VEHICLE);
	} // for

	Release();

	LOG_TRACE(LOG_DEBUG, timeStart, "LOG, finished, add link ex data to map");

	return true;
}


size_t CFileVehicleEx::ReadBody(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody = { 0, };

	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeBody = sizeof(FileBody);

	size_t nTotalLinks = 0;
	size_t nTotalNodes = 0;
	size_t nTotalData = 0;

	for (int32_t idx = 0; idx < m_vtIndex.size(); idx++)
	{
		if (!checkTestMesh(m_vtIndex[idx].idTile)) {
			continue;
		}

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
		if ((retRead = fread(&fileBody, sizeBody, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read body, offset:%d", m_vtIndex[idx].offBody);
			return 0;
		}

		if (m_vtIndex[idx].idTile != fileBody.idTile)
		{
			LOG_TRACE(LOG_ERROR, "Failed, index tile info not match with body, index tile id:%d vs body tile id:%d", m_vtIndex[idx].idTile, fileBody.idTile);
			return 0;
		}

		if (fileBody.szData <= 0) {
			continue;
		}

		int readItems = 0;

		for (int ii = 0; ii < fileBody.net.cntNode; ii++) {
			stExtendInfo* pExtInfo = new stExtendInfo();
			int readItem = pExtInfo->read(fp);
			if (readItem > 0) {
				// add to map
				m_pDataMgr->AddLinkExData(pExtInfo, TYPE_NODE_DATA_VEHICLE);
				readItems += readItem;
			}
		} // for
		nTotalData += readItems;

		for (int ii = 0; ii < fileBody.net.cntLink; ii++) {
			stExtendInfo* pExtInfo = new stExtendInfo();
			int readItem = pExtInfo->read(fp);
			if (readItem > 0) {
				// add to map
				m_pDataMgr->AddLinkExData(pExtInfo, TYPE_DATA_VEHICLE);
				readItems += readItem;
			}
		} // for
		nTotalData += readItems;

		if (readItems != fileBody.szData) {
			LOG_TRACE(LOG_ERROR, "Failed, body data size not matching from read file size, tile_id:%d, body:%d vs file:%d", fileBody.idTile, fileBody.szData, readItems);
			return 0;
		}

		offFile += m_vtIndex[idx].szBody;
	} // for

	LOG_TRACE(LOG_DEBUG, "Read data, body, node cnt:%lld, link cnt:%lld, ext cnt:%lld", nTotalNodes, nTotalLinks, nTotalData);

	return offFile;
}


bool CFileVehicleEx::LoadDataByIdx(IN const uint32_t idx)
{
	FILE* fp = fopen(m_szDataPath, "rb");
	if (!fp)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't open file for load data, file:%s", m_szDataPath);
		return false;
	}
	else if (idx < 0 || m_vtIndex.size() <= idx) {
		LOG_TRACE(LOG_ERROR, "Failed, request load data idx range, max:%s, req idx:%d", m_vtIndex.size(), idx);
		fclose(fp);
		return false;
	}

	FileBody fileBody = { 0, };

	const size_t sizeBody = sizeof(FileBody);

	size_t offFile = 0;
	size_t offItem = 0;
	size_t retRead = 0;

	// read body
	if (m_vtIndex[idx].szBody <= 0) {
		fclose(fp);
		return true;
	} else if (m_vtIndex[idx].offBody <= 0) {
		LOG_TRACE(LOG_ERROR, "Failed, index body info invalid, off:%d", m_vtIndex[idx].offBody);
		return false;
	}

	fseek(fp, m_vtIndex[idx].offBody, SEEK_SET);
	if ((retRead = fread(&fileBody, sizeof(fileBody), 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't read body, offset:%d", m_vtIndex[idx].offBody);
		fclose(fp);
		return false;
	}

	if (m_vtIndex[idx].idTile != fileBody.idTile)
	{
		LOG_TRACE(LOG_ERROR, "Failed, index tile info not match with body, index tile id:%d vs body tile id:%d", m_vtIndex[idx].idTile, fileBody.idTile);
		fclose(fp);
		return false;
	}

	if (fileBody.szData <= 0) {
		return true;
	}

	int readItems = 0;

	for (int ii = 0; ii < fileBody.net.cntNode; ii++) {
		stExtendInfo* pExtInfo = new stExtendInfo();
		int readItem = pExtInfo->read(fp);
		if (readItem > 0) {
			// add to map
			m_pDataMgr->AddLinkExData(pExtInfo, TYPE_NODE_DATA_VEHICLE);
			readItems += readItem;
		}
	} // for

	for (int ii = 0; ii < fileBody.net.cntLink; ii++) {
		stExtendInfo* pExtInfo = new stExtendInfo();
		int readItem = pExtInfo->read(fp);
		if (readItem > 0) {
			// add to map
			m_pDataMgr->AddLinkExData(pExtInfo, TYPE_DATA_VEHICLE);
			readItems += readItem;
		}
	} // for


	if (readItems != fileBody.szData) {
		LOG_TRACE(LOG_ERROR, "Failed, body data size not matching from read file size, tile_id:%d, body:%d vs file:%d", fileBody.idTile, fileBody.szData, readItems);
		return 0;
	}

	offFile += m_vtIndex[idx].szBody;

	fclose(fp);

	return true;
}


bool CFileVehicleEx::Initialize()
{
	return CFileBase::Initialize();
}


void CFileVehicleEx::Release()
{	
	if (!m_vtVehicleEx.empty()) {
		m_vtVehicleEx.clear();
		vector<stVehicleEx>().swap(m_vtVehicleEx);
	}

	CFileBase::Release();
}


size_t CFileVehicleEx::WriteBody(FILE* fp, IN const uint32_t fileOff)
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

	// write body
	stMeshInfo* pMesh = nullptr;
	int cntMesh = GetMeshCount();
	for (int32_t ii = 0; ii < cntMesh; ii++) {
		offItem = 0;
		pMesh = GetMeshData(ii);
		if (!pMesh) {
			LOG_TRACE(LOG_ERROR, "Failed, can't access mesh, idx:%d", ii);
			return 0;
		}

		MapExtend* pMapLinkEx = m_pDataMgr->GetMapExtend();
		if (!pMapLinkEx) {
			LOG_TRACE(LOG_ERROR, "Failed, MapLinkEx pointer disabled");
			return 0;
		}

		const stExtendMeshInfo* pMapMesh = pMapLinkEx->GetMapMeshData(pMesh->mesh_id.tile_id);
		if (pMapMesh != nullptr) {
			if (!pMapMesh->mapNode.empty() || !pMapMesh->mapLink.empty()) {
				// write body
				memset(&fileBody, 0x00, sizeFileBody);
				fileBody.idTile = pMesh->mesh_id.tile_id;
				fileBody.net.cntNode = pMapMesh->mapNode.size();
				fileBody.net.cntLink = pMapMesh->mapLink.size();

				if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write body[%d], written:%d", ii, retWrite);
					return 0;
				}
				offItem = sizeFileBody;

				// write data
				for (const auto& item : pMapMesh->mapNode) {
					fileBody.szData += item.second->write(fp);
				}

				for (const auto& item : pMapMesh->mapLink) {
					fileBody.szData += item.second->write(fp);
				}

				offItem += fileBody.szData;

				// re-write body size & off
				if (offItem > sizeFileBody) {
					fseek(fp, offItem * -1, SEEK_CUR);

					if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1) {
						LOG_TRACE(LOG_ERROR, "Failed, can't re-write body[%d], written:%d", ii, retWrite);
						return 0;
					}

					fseek(fp, offItem - sizeFileBody, SEEK_CUR);
				}
			}			
		} else {
#if defined(__DEBUG)
			LOG_TRACE(LOG_DEBUG, "mesh dosen't have any extend data, mesh_id:%d", pMesh->mesh_id.tile_id);
#endif
		}

		// re-set index size & off buff
		m_vtIndex[ii].szBody = offItem;
		m_vtIndex[ii].offBody = offFile;

		// update file offset
		offFile += offItem;
	} // for


	// re-write index for body size and offset
	fseek(fp, m_fileHeader.offIndex, SEEK_SET);
	for (uint32_t idx = 0; idx < m_fileHeader.cntIndex; idx++)
	{
		if ((retRead = fwrite(&m_vtIndex[idx], sizeof(FileIndex), 1, fp)) != 1) {
			LOG_TRACE(LOG_ERROR, "Failed, can't re-write file index data, idx:%d", idx);
			return 0;
		}
	}

	return offFile;
}


