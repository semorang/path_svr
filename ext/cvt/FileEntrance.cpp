#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FileEntrance.h"

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


CFileEntrance::CFileEntrance()
{
	m_nEntType = TYPE_ENT_NONE;
	m_nEntIdx = 0;
	m_nFileType = TYPE_DATA_ENTRANCE;

	m_pFileCpx = nullptr;
	m_pFileBld = nullptr;
}

CFileEntrance::~CFileEntrance()
{

}

static const uint32_t g_cntLogPrint = 100000;

bool CFileEntrance::ParseData(IN const char* fname)
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

	if (!m_pFileCpx || !m_pFileBld) {
		LOG_TRACE(LOG_ERROR, "failed, complex container & building container pointer not set");
		return ret;
	}

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
		LOG_TRACE(LOG_ERROR, "Failed, entrance file header not exist", lineCount);
		return ret;
	}

	if (strcmp(pTok, "B_ENT_ID") == 0) {
		m_nEntType = TYPE_ENT_BUILDING;
	} else if ((strcmp(pTok, "SID_CD") == 0) || (strcmp(pTok, "SIG_CD") == 0) || (strcmp(pTok, "SIG") == 0)) { // SIG 인지 SID 인지 확인 후 하나만 사용
	//} else if ((strcmp(pTok, "SIG") == 0) {
		m_nEntType = TYPE_ENT_COMPLEX;
	} else {
		LOG_TRACE(LOG_ERROR, "Failed, entrance file type not defined");
		return ret;
	}
	
	while (fgets(szBuff, maxBuffCount - 1, fp) != nullptr)
	{
		lineCount++;

		if (strlen(szBuff) <= 0) {
			continue;
		}

		pLine = szBuff;

		stEntrance ent_data;
		
		// TYPE
		ent_data.MatchType = m_nEntType;

		// ID
		ent_data.Id = m_nEntIdx;

		if (m_nEntType == TYPE_ENT_BUILDING) // 건물
		{
			// B_ENT_ID
			pTok = strsep(&pLine, token);
			lenTok = strlen(pTok);
			if (lenTok <= 0) {
				LOG_TRACE(LOG_ERROR, "Failed, entrance building B_ENT_ID emtpy, line:%d", lineCount);
				continue;
			}

			// MID // 건물 고유 ID
			pTok = strsep(&pLine, token);
			lenTok = strlen(pTok);
			if (lenTok <= 0) {
				LOG_TRACE(LOG_ERROR, "Failed, entrance building id emtpy, line:%d", lineCount);
				continue;
			}

			// MID // 건물 폴리곤과 매칭되는 Key
			ent_data.MatchId = m_pFileBld->GetIdFromStringId(pTok);
			if (ent_data.MatchId.llid == NULL_VALUE) {
#if !defined(_USE_TEST_MESH)
				LOG_TRACE(LOG_ERROR, "Failed, entrance building id not match with building contianer datas, id:%s, line:%d", pTok, lineCount);
#endif
				continue;
			}

			//EQB_MAN_SN
			pTok = strsep(&pLine, token);

			// DJ_TFID // 건물에 매칭되는 단지ID
			//pTok = strsep(&pLine, token);
			//lenTok = strlen(pTok);

			// BUL_MAN_NO // 도로명주소 데이터 건물 일련번호
			pTok = strsep(&pLine, token);

			// SIG_CD // 시군구 코드
			pTok = strsep(&pLine, token);
			lenTok = strlen(pTok);
			if (lenTok <= 0) {
				LOG_TRACE(LOG_ERROR, "Failed, entrance building SGG code empty, line:%d", lineCount);
				continue;
			}
			ent_data.SdSggCode = atoi(pTok);

			// B_ENT_CD // 건물 입구점 코드
			pTok = strsep(&pLine, token);
			lenTok = strlen(pTok);
			if (lenTok <= 0) {
				LOG_TRACE(LOG_ERROR, "Failed, entrance building code empty, line:%d", lineCount);
				continue;
			}
			ent_data.EntCode = atoi(pTok);

			// DJ_YN // 단지 포함 여부
			//pTok = strsep(&pLine, token);

			// 입구점 좌표 - 공통
		} 
		else if (m_nEntType == TYPE_ENT_COMPLEX) // 단지
		{
			// 시도, 시군구 코드
			pTok = strsep(&pLine, token);
			lenTok = strlen(pTok);
			if (lenTok <= 0) {
				LOG_TRACE(LOG_ERROR, "Failed, entrance complex SID_CD emtpy, line:%d", lineCount);
				continue;
			}
			ent_data.SdSggCode = atoi(pTok);
			string strKey = string(pTok);

			// 단지 ID
			pTok = strsep(&pLine, token);
			ent_data.CpxId = atoi(pTok);

			strKey.insert(0, pTok);
			ent_data.MatchId = m_pFileCpx->GetIdFromStringId(strKey.c_str());
			if (ent_data.MatchId.llid == NULL_VALUE) {
#if !defined(_USE_TEST_MESH)
				LOG_TRACE(LOG_ERROR, "Failed, entrance complex id not match with complex container datas, id:%s, line:%d", pTok, lineCount);
#endif
				continue;
			}

			// 건물 입구점 코드
			pTok = strsep(&pLine, token);
			ent_data.EntCode = atoi(pTok);

			// 입구점 좌표 - 공통
		}
		

		// XPOS // 입구점 X좌표
		pTok = strsep(&pLine, token);
		lenTok = strlen(pTok);
		if (lenTok <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, entrance x position emtpy, line:%d", lineCount);
			continue;
		}
		ent_data.x = atof(pTok);


		// YPOS // 입구점 Y좌표
		pTok = strsep(&pLine, token);
		lenTok = strlen(pTok);
		if (lenTok <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, entrance y position, line:%d", lineCount);
			continue;
		}
		ent_data.y = atof(pTok);

		if (m_nEntType == TYPE_ENT_BUILDING) // 건물
		{
			// ENT_FLAG // 자동/수동
			pTok = strsep(&pLine, token);
			lenTok = strlen(pTok);
			// 수동이 아닐경우 데이터 제외
			if ((lenTok <= 0) || (atoi(pTok) != 1)) {
				continue;
			}
		}

		// add to container
		m_vtEntrance.emplace_back(ent_data);
		
		m_nEntIdx++;
	}

	return GenServiceData();
}


bool CFileEntrance::GenServiceData()
{
	// 클래스에 추가
	LOG_TRACE(LOG_DEBUG, "LOG, start, add entrance to polygon class");

	static const int32_t nMaxEntDist = 3000; // 건물/단지와 최대 1000m 이상 차이가 나면 오류

	stEntranceInfo entInfo;


	for (vector<stEntrance>::const_iterator it = m_vtEntrance.begin(); it != m_vtEntrance.end(); it++)
	{
		if (it->MatchId.llid != NULL_VALUE) {	

			/*int32_t nDist = getRealWorldDistance((pPoly->data_box.Xmin + pPoly->data_box.Xmax) / 2, (pPoly->data_box.Ymin + pPoly->data_box.Ymax) / 2, it->x, it->y);
			if (nDist > nMaxEntDist) {
				LOG_TRACE(LOG_WARNING, "Failed, entrance point too far from polygon, id:%lld, dist:%d, poly-> tile:%d, id:%d, x:%.6f, y:%.6f | ent-> type:%d, id:%d, code:%d, x:%.6f, y:%.6f", pPoly->poly_id.llid, nDist, pPoly->poly_id.tile_id, pPoly->poly_id.nid, (pPoly->data_box.Xmin + pPoly->data_box.Xmax) / 2, (pPoly->data_box.Ymin + pPoly->data_box.Ymax) / 2, it->MatchType, it->Id, it->EntCode, it->x, it->y);
				continue;
			}*/
				
			memset(&entInfo, 0x00, sizeof(entInfo));

			entInfo.poly_type = it->MatchType; // 0:미지정, 1:건물, 2:단지
			entInfo.ent_code = it->EntCode;
			entInfo.x = it->x;
			entInfo.y = it->y;

			//if (!m_pDataMgr->AddEntranceData(it->MatchId, &entInfo)) {
			//	LOG_TRACE(LOG_WARNING, "Failed, can't add building entrance data to data manager, id:%d, tile:%d, nid:%d, ", it->Id, it->MatchId.tile_id, it->MatchId.nid);
			//	continue;
			//}
			if (it->MatchType == TYPE_ENT_BUILDING) { // 건물
				m_pFileBld->AddEntranceData(it->MatchId, entInfo);
			}
			else if (it->MatchType == TYPE_ENT_COMPLEX) {
				m_pFileCpx->AddEntranceData(it->MatchId, entInfo);
			}
			else {
				LOG_TRACE(LOG_ERROR, "Failed, entrance type not defined, id:%s, sgg:%d", it->Id, it->SdSggCode);
				continue;
			}
		} // if 

		//if (!it->CpxStr.empty()) {
		//	entInfo.poly_type = 1; // 단지

		//	uint32_t cpxId = m_pFileCpx->GetIdFromStringId(it->CpxStr.c_str());
		//	stComplexShare* pCpxIds = nullptr;
		//	if (cpxId != NULL_VALUE && m_pFileCpx->GetCpxShare(cpxId, &pCpxIds) <= 0) {
		//		LOG_TRACE(LOG_ERROR, "Failed, can't find entrance complex id from complex string, id:%s", it->CpxStr.c_str());
		//		continue;
		//	}

		//	// 연결된 단지 모두에 입구점 정보 추가
		//	for (vector<KeyID>::const_iterator itParts = pCpxIds->vtCpxParts.begin(); itParts != pCpxIds->vtCpxParts.end(); itParts++) {
		//		if (!m_pDataMgr->AddEntranceData(*itParts, &entInfo)) {
		//			LOG_TRACE(LOG_ERROR, "Failed, can't add complex entrance data to data manager, id:%s, tile:%d, nid:%d, ", it->CpxStr.c_str(), itParts->tile_id, itParts->nid);
		//			continue;
		//		}
		//	}
		//}
	} // for

	if ((!m_nEntType || m_nEntType == TYPE_ENT_COMPLEX) && m_pFileCpx) {
		m_pFileCpx->GenServiceData();
	}

	if ((!m_nEntType || m_nEntType == TYPE_ENT_BUILDING) && m_pFileBld) {
		m_pFileBld->GenServiceData();
	}

	Release();

	LOG_TRACE(LOG_DEBUG, "LOG, finished");

	return true;
}


void CFileEntrance::AddDataFeild(IN const int idx, IN const int type, IN const char* colData)
{

}


void CFileEntrance::AddDataRecord()
{

}


bool CFileEntrance::LoadData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_ENTRANCE], g_szTypeExec[TYPE_DATA_ENTRANCE]);

	return CFileBase::LoadData(szFileName);
}


size_t CFileEntrance::ReadBody(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody = { 0, };
	FileEntrance fileEnt = { 0, };

	stMeshInfo* pMesh;
	stPolygonInfo* pPoly;

	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeBody = sizeof(FileBody);

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

		if (m_vtIndex[idx].idTile != fileBody.idTile)
		{
			LOG_TRACE(LOG_ERROR, "Failed, index tile info not match with body, index tile id:%d vs body tile id:%d", m_vtIndex[idx].idTile, fileBody.idTile);
			return 0;
		}


		// mesh
		pMesh = GetMeshDataById(m_vtIndex[idx].idTile);
		if (!pMesh) {
			pMesh = new stMeshInfo;

			pMesh->mesh_id.tile_id = m_vtIndex[idx].idTile;
			memcpy(&pMesh->mesh_box, &m_vtIndex[idx].rtTile, sizeof(pMesh->mesh_box));
			memcpy(&pMesh->data_box, &m_vtIndex[idx].rtData, sizeof(pMesh->data_box));
			for (int ii = 0; ii < 8; ii++) {
				if (m_vtIndex[idx].idNeighborTile[ii] <= 0) {
					continue;
				}
				pMesh->neighbors.emplace(m_vtIndex[idx].idNeighborTile[ii]);
			}
			AddMeshData(pMesh);
		}
		else
		{
			// extend rect
			//memcpy(&pMesh->data_box, &m_vtIndex[idx].rtData, sizeof(pMesh->data_box));
			for (int ii = 0; ii < 8; ii++) {
				if (m_vtIndex[idx].idNeighborTile[ii] <= 0) {
					continue;
				}
				pMesh->neighbors.emplace(m_vtIndex[idx].idNeighborTile[ii]);
			} // for
		}


		// complex entrance
		for (int32_t ii = 0; ii < fileBody.ent.cntComplex; ii++)
		{
			// read complex entrance count
			if ((retRead = fread(&fileEnt, sizeof(fileEnt), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read complex entrance, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			// read complex entrance	
			//stEntranceInfo entInfo;
			//for (uint32_t jj = 0; jj < fileEnt.cntEntrance; jj++)
			//{
			//	if ((retRead = fread(&entInfo, sizeof(entInfo), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't read complex entrance, mesh_id:%d, complex entrance idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
			//		return 0;
			//	}
			//	
			//	AddEntranceData(fileEnt.polygon_id, &entInfo);
			//}

			pPoly = m_pDataMgr->GetComplexDataById(fileEnt.polygon_id);
			if (fileEnt.cntEntrance > 0 && pPoly != nullptr && !pPoly->readAttribute(TYPE_POLYGON_DATA_ATTR_ENT, fp, fileEnt.cntEntrance))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read complex entrance, mesh_id:%d, complex entrance idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}
		}

		offFile += m_vtIndex[idx].szBody;


		// building entrance
		for (int32_t ii = 0; ii < fileBody.ent.cntBuilding; ii++)
		{
			// read building entrance count
			if ((retRead = fread(&fileEnt, sizeof(fileEnt), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read building entrance, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			// read building entrance
			//stEntranceInfo entInfo;
			//for (uint32_t jj = 0; jj < fileEnt.cntEntrance; jj++)
			//{
			//	if ((retRead = fread(&entInfo, sizeof(entInfo), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't read building entrance, mesh_id:%d, complex entrance idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
			//		return 0;
			//	}

			//	AddEntranceData(fileEnt.polygon_id, &entInfo);
			//}
			pPoly = m_pDataMgr->GetBuildingDataById(fileEnt.polygon_id);
			if (fileEnt.cntEntrance > 0 && pPoly != nullptr && !pPoly->readAttribute(TYPE_POLYGON_DATA_ATTR_ENT, fp, fileEnt.cntEntrance))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read building entrance, mesh_id:%d, building entrance idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}
		}

		offFile += m_vtIndex[idx].szBody;

#if defined(_DEBUG)
		LOG_TRACE(LOG_DEBUG, "Data loading, entrance data loaded, mesh:%d, cpx:%d, bld:%d", pMesh->mesh_id.tile_id, fileBody.ent.cntComplex, fileBody.ent.cntBuilding);
#endif
	}

	return offFile;
}


bool CFileEntrance::LoadDataByIdx(IN const uint32_t idx)
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
	FileEntrance fileEnt = { 0, };

	stMeshInfo* pMesh;
	stPolygonInfo* pPoly;

	const size_t sizeBody = sizeof(FileBody);

	size_t offFile = 0;
	size_t offItem = 0;
	size_t retRead = 0;

	// read body
	if (m_vtIndex[idx].offBody <= 0 || m_vtIndex[idx].szBody <= 0)
	{
		LOG_TRACE(LOG_ERROR, "Failed, index body info invalid, off:%d, size:%d", m_vtIndex[idx].offBody, m_vtIndex[idx].szBody);
		fclose(fp);
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


	// mesh
	pMesh = GetMeshDataById(m_vtIndex[idx].idTile);
	if (!pMesh) {
		pMesh = new stMeshInfo;

		pMesh->mesh_id.tile_id = m_vtIndex[idx].idTile;
		memcpy(&pMesh->mesh_box, &m_vtIndex[idx].rtTile, sizeof(pMesh->mesh_box));
		memcpy(&pMesh->data_box, &m_vtIndex[idx].rtData, sizeof(pMesh->data_box));
		for (int ii = 0; ii < 8; ii++) {
			if (m_vtIndex[idx].idNeighborTile[ii] <= 0) {
				continue;
			}
			pMesh->neighbors.emplace(m_vtIndex[idx].idNeighborTile[ii]);
		}
		AddMeshData(pMesh);
	}


	// complex entrance
	for (int32_t ii = 0; ii < fileBody.ent.cntComplex; ii++)
	{
		// read complex entrance count
		if ((retRead = fread(&fileEnt, sizeof(fileEnt), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read complex entrance, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return 0;
		}
		
		// read complex entrance
		//stEntranceInfo entInfo;
		//for (uint32_t jj = 0; jj < fileEnt.cntEntrance; jj++)
		//{
		//	if ((retRead = fread(&entInfo, sizeof(entInfo), 1, fp)) != 1)
		//	{
		//		LOG_TRACE(LOG_ERROR, "Failed, can't read complex entrance, mesh_id:%d, complex entrance idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
		//		return 0;
		//	}

		//	AddEntranceData(fileEnt.polygon_id, &entInfo);
		//}

		pPoly = m_pDataMgr->GetComplexDataById(fileEnt.polygon_id, false);
		if (fileEnt.cntEntrance > 0 && pPoly != nullptr) {
			if (!pPoly->readAttribute(TYPE_POLYGON_DATA_ATTR_ENT, fp, fileEnt.cntEntrance)) {
				LOG_TRACE(LOG_ERROR, "Failed, can't read complex entrance, mesh_id:%d, complex entrance idx:%d", m_vtIndex[idx].idTile, ii);
				fclose(fp);
				return 0;
			}
		}
#if defined(USE_DATA_CACHE)
		// 다른 타일의 데이터라 못읽었으면 파일offset만 이동 
		else if (fileEnt.cntEntrance > 0 && pPoly == nullptr) {
			fseek(fp, fileEnt.cntEntrance * sizeof(stEntranceInfo), SEEK_CUR);
		}
#endif
	}

	offFile += m_vtIndex[idx].szBody;


	// building entrance
	for (int32_t ii = 0; ii < fileBody.ent.cntBuilding; ii++)
	{
		// read building entrance count
		if ((retRead = fread(&fileEnt, sizeof(fileEnt), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read building entrance, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return 0;
		}

		// read building entrance
		//stEntranceInfo entInfo;
		//for (uint32_t jj = 0; jj < fileEnt.cntEntrance; jj++)
		//{
		//	if ((retRead = fread(&entInfo, sizeof(entInfo), 1, fp)) != 1)
		//	{
		//		LOG_TRACE(LOG_ERROR, "Failed, can't read building entrance, mesh_id:%d, complex entrance idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
		//		return 0;
		//	}

		//	AddEntranceData(fileEnt.polygon_id, &entInfo);
		//}

		pPoly = m_pDataMgr->GetBuildingDataById(fileEnt.polygon_id, false);
		if (fileEnt.cntEntrance > 0 && pPoly != nullptr) {
			if (!pPoly->readAttribute(TYPE_POLYGON_DATA_ATTR_ENT, fp, fileEnt.cntEntrance)) {
				LOG_TRACE(LOG_ERROR, "Failed, can't read building entrance, mesh_id:%d, building entrance idx:%d", m_vtIndex[idx].idTile, ii);
				fclose(fp);
				return 0;
			}
		}
#if defined(USE_DATA_CACHE)
		// 다른 타일의 데이터라 못읽었으면 파일offset만 이동 
		else if (fileEnt.cntEntrance > 0 && pPoly == nullptr) {
			fseek(fp, fileEnt.cntEntrance * sizeof(stEntranceInfo), SEEK_CUR);
		}
#endif
	}

	offFile += m_vtIndex[idx].szBody;

#if defined(_DEBUG)
	LOG_TRACE(LOG_DEBUG, "Data loading, entrance data loaded, mesh:%d, cpx:%d, bld:%d", pMesh->mesh_id.tile_id, fileBody.ent.cntBuilding, fileBody.ent.cntBuilding);
#endif

	fclose(fp);

	return true;
}


bool CFileEntrance::Initialize()
{
	return CFileBase::Initialize();
}


void CFileEntrance::Release()
{	
	if (!m_vtEntrance.empty()) {
		m_vtEntrance.clear();
		vector<stEntrance>().swap(m_vtEntrance);
	}

	if ((!m_nEntType || m_nEntType == TYPE_ENT_COMPLEX) && m_pFileCpx) {
		m_pFileCpx->Release();
	}

	if ((!m_nEntType || m_nEntType == TYPE_ENT_BUILDING) && m_pFileBld) {
		m_pFileBld->Release();
	}

	m_nEntIdx = 0;
	m_nEntType = TYPE_ENT_NONE;

	CFileBase::Release();
}


bool CFileEntrance::SaveData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_ENTRANCE], g_szTypeExec[TYPE_DATA_ENTRANCE]);

	return CFileBase::SaveData(szFileName);
}


size_t CFileEntrance::WriteBody(FILE* fp, IN const uint32_t fileOff)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	stMeshInfo* pMesh = nullptr;
	stPolygonInfo* pPoly = nullptr;

	FileBody fileBody;
	FileEntrance fileEnt;

	size_t offFile = fileOff;
	size_t retWrite = 0;
	size_t retRead = 0;
	long offItem = 0;
	const size_t sizeFileBody = sizeof(fileBody);

	uint32_t ii, jj;

	// write body - entrance
	for (ii = 0; ii < GetMeshCount(); ii++)
	{
		pMesh = GetMeshData(ii);
		if (!pMesh)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't access mesh, idx:%d", ii);
			return 0;
		}


		// write body
		memset(&fileBody, 0x00, sizeof(fileBody));

		jj = 0;
		fileBody.idTile = pMesh->mesh_id.tile_id;
		if (pMesh->neighbors.size() > 8) {
			LOG_TRACE(LOG_ERROR, "Error, --------------- Mesh neighbor count overflow, id:%d, %d", pMesh->mesh_id.tile_id, pMesh->neighbors.size());
		}

		if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't write body[%d], written:%d", ii, retWrite);
			return 0;
		}
		offItem = sizeFileBody;


		// write complex entrance
# if defined(USE_OPTIMAL_POINT_API)
		for (jj = 0; jj < pMesh->complexs.size(); jj++)
		{
			pPoly = m_pDataMgr->GetComplexDataById(pMesh->complexs[jj]);
			if (!pPoly)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't access complex, idx:%d, tile_id:%d, id:%d", ii, pMesh->complexs[ii].tile_id, pMesh->complexs[ii].nid);
				return 0;
			}

			//if ((fileEnt.cntEntrance = pPoly->vtEnt.size()) <= 0) {
			if ((fileEnt.cntEntrance = pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_ENT)) <= 0) {
				continue;
			}
			fileEnt.polygon_id = pPoly->poly_id;

			if ((retWrite = fwrite(&fileEnt, sizeof(fileEnt), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't write complex entrance, idx:%d, tile_id:%d, id:%d", jj, pPoly->poly_id.tile_id, pPoly->poly_id.nid);
				return 0;
			}

			fileBody.ent.cntComplex++;
			offItem += sizeof(fileEnt);

			// write entrance
			//for (kk = 0; kk < fileEnt.cntEntrance; kk++)
			//{
			//	if ((retWrite = fwrite(&pPoly->vtEnt[kk], sizeof(stEntranceInfo), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't write complex[%d] entrance[%d], ", jj, kk);
			//		return 0;
			//	}
			//	offItem += sizeof(stEntranceInfo);
			//}
			if (fileEnt.cntEntrance > 0)
			{
				if (!(retWrite = pPoly->writeAttribute(TYPE_POLYGON_DATA_ATTR_ENT, fp))) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write complex[%d], entrance", jj);
					return 0;
				}
				offItem += retWrite;
			}
		} // write complex entrance

		if (jj > 0) {
			LOG_TRACE(LOG_DEBUG, "Save data, complex entrance, cnt:%lld", jj);
		}

		// write building entrance
		for (jj = 0; jj < pMesh->buildings.size(); jj++)
		{
			pPoly = m_pDataMgr->GetBuildingDataById(pMesh->buildings[jj]);
			if (!pPoly)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't access building, idx:%d, tile_id:%d, id:%d", ii, pMesh->buildings[ii].tile_id, pMesh->buildings[ii].nid);
				return 0;
			}

			//if ((fileEnt.cntEntrance = pPoly->vtEnt.size()) <= 0) {
			if ((fileEnt.cntEntrance = pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_ENT)) <= 0) {
				continue;
			}
			fileEnt.polygon_id = pPoly->poly_id;

			if ((retWrite = fwrite(&fileEnt, sizeof(fileEnt), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't write building entrance, idx:%d, tile_id:%d, id:%d", jj, pPoly->poly_id.tile_id, pPoly->poly_id.nid);
				return 0;
			}

			fileBody.ent.cntBuilding++;
			offItem += sizeof(fileEnt);

			// write entrance
			//for (kk = 0; kk < fileEnt.cntEntrance; kk++)
			//{
			//	if ((retWrite = fwrite(&pPoly->vtEnt[kk], sizeof(stEntranceInfo), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't write building[%d] entrance[%d], ", jj, kk);
			//		return 0;
			//	}
			//	offItem += sizeof(stEntranceInfo);
			//}
			if (fileEnt.cntEntrance > 0)
			{
				if (!(retWrite = pPoly->writeAttribute(TYPE_POLYGON_DATA_ATTR_ENT, fp))) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write building[%d], entrance", jj);
					return 0;
				}
				offItem += retWrite;
			}
		} // write complex entrance

		if (jj > 0) {
			LOG_TRACE(LOG_DEBUG, "Save data, building entrance, cnt:%lld", jj);
		}
#endif // # if defined(USE_OPTIMAL_POINT_API)


		// re-write body size & off
		if (offItem > sizeFileBody)
		{
			fileBody.szData = offItem;

			fseek(fp, offItem * -1, SEEK_CUR);

			if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't re-write complex[%d], tile_id:%d", ii, fileBody.idTile);
				return 0;
			}

			fseek(fp, offItem - sizeFileBody, SEEK_CUR);
		}

		// re-set index size & off buff
		m_vtIndex[ii].szBody = fileBody.szData;
		m_vtIndex[ii].offBody = offFile;

		// update file offset
		offFile += offItem;
	}


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


void CFileEntrance::SetFileComplex(CFileComplex* pCpx)
{
	m_pFileCpx = pCpx;
}


void CFileEntrance::SetFileBuilding(CFileBuilding* pBld)
{
	m_pFileBld = pBld;
}

