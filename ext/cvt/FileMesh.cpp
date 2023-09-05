#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FileMesh.h"

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


CFileMesh::CFileMesh()
{
	m_nFileType = TYPE_DATA_MESH;
}

CFileMesh::~CFileMesh()
{

}

static const uint32_t g_cntLogPrint = 100000;

bool CFileMesh::ParseData(IN const char* fname)
{
	XSHPReader shpReader;

	if (!shpReader.Open(fname)) {
		LOG_TRACE(LOG_ERROR, "Error,Cannot Find shp File!");
		return false;
	}

	//전체 지도영역 얻기
	shpReader.GetEnvelope((SBox*)GetMeshRegion());

	//전체 데이터 카운터 얻기
	long nRecCount = shpReader.GetRecordCount();
	int nFieldCount = shpReader.GetFieldCount();

	//int nLabelIdxToRead = 1;
	char chsTmp[128];
	const int colCnt = shpReader.GetFieldCount();

	// 데이터 필드 무결성 확보를 위한 검사
	// 다른 데이터에도 추가해 주자
	if (strstr(fname, "MESH") != nullptr) {
		static char szLinkField[128][32] = {
			{ "MESH_ID" },{ "MESH_INFO" },
		};

		DBF_FIELD_INFO fieldInfo;

		for (int ii = 0; ii < colCnt; ii++) {
			if (shpReader.GetFieldInfo(ii, fieldInfo) == false) {
				LOG_TRACE(LOG_ERROR, "can't get dbf filed info, idx : %d ", ii);
				return false;
			}
			else if (strcmp(szLinkField[ii], trim(fieldInfo.szName)) != 0) {
				LOG_TRACE(LOG_ERROR, "dbf field name not matched the expected name, filedName:%s vs exprectedName:%s", fieldInfo.szName, szLinkField[ii]);
				return false;
			}
		}
	}

	int nShpType = (int)shpReader.GetShpObjectType(); //shpPoint=1,shpPolyLine=3,shpPolygon=5

	if (nShpType == 5) {
		nShpType = 3; //면
	}
	else {
		LOG_TRACE(LOG_ERROR, "Failed, file type not mesh, type:%d", nShpType);

		shpReader.Close();
		return false;
	}


	//데이터 얻기
	LOG_TRACE(LOG_DEBUG, "LOG, start, raw data parsing file : %s, rec cnt:%d", fname, nRecCount);

	for (int idxRec = 0; idxRec < nRecCount; idxRec++) {
		stWalkMesh mesh;

		//속성가져오기
		if (shpReader.Fetch(idxRec) == false) { //error to read..
			LOG_TRACE(LOG_ERROR, "Error : Record %d is not available!!", idxRec);
			continue;
		}

		//Setting DBF Data
		for (unsigned int idxCol = 0; idxCol < colCnt; idxCol++) {
			memset(chsTmp, 0x00, sizeof(chsTmp));
			shpReader.GetDataAsString(idxCol, chsTmp, sizeof(chsTmp));  //nLabelIdxToRead 번째 필드 값을 chsTmp로 12byte 읽어옴.

			SHPGeometry *pSHPObj = shpReader.GetGeometry();

			if (nShpType == 3) { // 면
				if (idxCol == 0) { // id
					mesh.MeshID = atoi(trim(chsTmp));
					memcpy(&mesh.meshBox, &pSHPObj->mpoint.Box, sizeof(SBox));
				}
				else if (idxCol == 1) { // 인접 메쉬들
					char *pDat = chsTmp;
					char token[] = { "," };
					int32_t idNeighbor;
					
					char* pTok = strsep(&pDat, token);
					while (pTok != nullptr) {
						idNeighbor = atoi(pTok);
						if ((idNeighbor > 0) && (idNeighbor != mesh.MeshID)) {
							mesh.vtNeighbors.emplace_back(idNeighbor);
						}
						pTok = strsep(&pDat, token);
					} // while
				} // else if
			} // if
		} // for


#if defined(_USE_TEST_MESH)
		bool isContinue = true;
		for (const auto& item : g_arrTestMesh) {
			if (item == mesh.MeshID) {
				isContinue = false;
				break;
			}
		}
		if (isContinue) continue;
#endif


		if (nShpType == 3) {
			stMeshInfo* pMesh = new stMeshInfo;

			pMesh->mesh_id.tile_id = mesh.MeshID;
			for (vector<uint32_t>::const_iterator it = mesh.vtNeighbors.begin(); it != mesh.vtNeighbors.end(); it++) {
				pMesh->neighbors.emplace(*it);
			}			
			memcpy(&pMesh->mesh_box, &mesh.meshBox, sizeof(SBox));

			m_mapMesh.insert(pair<uint32_t, stMeshInfo*>((uint32_t)pMesh->mesh_id.tile_id, pMesh));
		}
	} // for

	shpReader.Close();

	return GenServiceData();
}

bool CFileMesh::GenServiceData()
{
	unordered_map<uint64_t, uint32_t>::iterator itNodeIndex;
	map<uint32_t, stMeshInfo*>::iterator itMesh;


	// 근접 메쉬 확인
	LOG_TRACE(LOG_DEBUG, "LOG, start, check mesh neighbor");

	// 이제는 메쉬 shp에 이웃 메쉬 정보가 있으니 따로 처리할 필요 없음 (2022-11-24)
	//SetNeighborMesh();


	// 클래스에 추가
	LOG_TRACE(LOG_DEBUG, "LOG, start, add data to class");

	for (itMesh = m_mapMesh.begin(); itMesh != m_mapMesh.end(); itMesh++) {
		if (!AddMeshData(itMesh->second)) {
			LOG_TRACE(LOG_ERROR, "Failed, add mesh data, mesh:%d", itMesh->second->mesh_id.tile_id);
		}
	}


	Release();
	
	LOG_TRACE(LOG_DEBUG, "LOG, finished");

	return true;
}


void CFileMesh::AddDataFeild(IN const int idx, IN const int type, IN const char* colData)
{

}


void CFileMesh::AddDataRecord()
{

}


bool CFileMesh::Initialize()
{
	return CFileBase::Initialize();
}

void CFileMesh::Release()
{
	CFileBase::Release();
}


#if defined(USE_DATA_CACHE)
bool CFileMesh::AddIndexData(IN const FileIndex& pData)
{
	if (m_pDataMgr) {
		return m_pDataMgr->AddIndexData(pData);
	}

	return false;
}
#endif // #if defined(USE_DATA_CACHE)


bool CFileMesh::SaveData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_MESH], g_szTypeExec[TYPE_DATA_MESH]);

	return CFileBase::SaveData(szFileName);
}


size_t CFileMesh::WriteIndex(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileIndex fileIndex;
	stMeshInfo* pMesh = nullptr;

	size_t offFile = 0;
	size_t retWrite = 0;
	size_t retRead = 0;
	const size_t sizeIndex = sizeof(fileIndex);

	for (uint32_t idx = 0; idx < GetMeshCount(); idx++)
	{
		int idxNeighbor = 0;
		pMesh = GetMeshData(idx);
		if (!pMesh)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't access mesh, idx:%d", idx);
			return 0;
		}

		fileIndex.idxTile = idx;
		fileIndex.idTile = pMesh->mesh_id.tile_id;
		if (pMesh->neighbors.size() > 8) {
			LOG_TRACE(LOG_ERROR, "Error, --------------- Mesh neighbor count overflow, id:%d, %d", pMesh->mesh_id.tile_id, pMesh->neighbors.size());
		}
		for (unordered_set<uint32_t>::const_iterator it = pMesh->neighbors.begin(); it != pMesh->neighbors.end(); it++) {
			fileIndex.idNeighborTile[idxNeighbor++] = *it;
		}
		memcpy(&fileIndex.rtTile, &pMesh->mesh_box, sizeof(fileIndex.rtTile));
		memcpy(&fileIndex.rtData, &pMesh->data_box, sizeof(fileIndex.rtData));
		fileIndex.szBody = 0;
		fileIndex.offBody = 0;

		if ((retWrite = fwrite(&fileIndex, sizeIndex, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't write index[%d], written:%d", idx, retWrite);
			return 0;
		}
		offFile += sizeIndex;

		// add body data info buff
		m_vtIndex.emplace_back(fileIndex);
	}

	LOG_TRACE(LOG_DEBUG, "Save data, mesh index, mesh cnt:%lld", m_vtIndex.size());

	return offFile;
}


size_t CFileMesh::WriteBody(FILE* fp, IN const uint32_t fileOff)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	size_t offFile = fileOff;
	size_t retWrite = 0;
	size_t retRead = 0;
	long offItem = 0;

	stMeshInfo* pMesh = nullptr;

	FileBody fileBody;

	uint32_t ii, jj;
	const size_t sizeFileBody = sizeof(fileBody);

	// write body - mesh
	for (ii = 0; ii < GetMeshCount(); ii++)
	{
		pMesh = GetMeshData(ii);
		if (!pMesh)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't access mesh, idx:%d", ii);
			return 0;
		}


		// write body
		memset(&fileBody, 0x00, sizeFileBody);

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
		fileBody.szData = sizeFileBody;

		offItem = sizeFileBody;

#if defined(_DEBUG)
		LOG_TRACE(LOG_DEBUG, "Save data, mesh:%d, cnt:%lld", pMesh->mesh_id.tile_id, jj);
#endif


		// re-write body size & off
		if (offItem > sizeFileBody)
		{
			fileBody.szData = offItem;

			fseek(fp, offItem * -1, SEEK_CUR);

			if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't re-write mesh[%d], id:%d", ii, pMesh->mesh_id.tile_id);
				return 0;
			}

			fseek(fp, offItem - sizeFileBody, SEEK_CUR);
		}

		// re-set index size & off buff
		m_vtIndex[ii].szBody = fileBody.szData;
		m_vtIndex[ii].offBody = offFile;

		// update file offset
		offFile += offItem;
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

	LOG_TRACE(LOG_DEBUG, "Save data, mesh cnt:%d", GetMeshCount());

	return offFile;
}


bool CFileMesh::LoadData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_MESH], g_szTypeExec[TYPE_DATA_MESH]);

	return CFileBase::LoadData(szFileName);
}


size_t CFileMesh::ReadIndex(FILE* fp)
{
	size_t offFile = 0;
	size_t retRead = 0;
	FileIndex fileIndex;
	const size_t sizeIndex = sizeof(fileIndex);

	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	fseek(fp, m_fileHeader.offIndex, SEEK_SET);

	for (uint32_t idxTile = 0; idxTile < m_fileHeader.cntIndex; idxTile++)
	{
		if ((retRead = fread(&fileIndex, sizeIndex, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read index, idx:%d", idxTile);
			return 0;
		}

		m_vtIndex.emplace_back(fileIndex);
		offFile += sizeIndex;

#if defined(USE_DATA_CACHE)
		AddIndexData(fileIndex);
#endif
	}

	LOG_TRACE(LOG_DEBUG, "Read data, index, type:%s, size:%d", GetDataTypeName(), offFile);

	return offFile;
}


size_t CFileMesh::ReadBody(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody = { 0, };

	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeBody = sizeof(FileBody);

	stMeshInfo* pMesh = nullptr;

	for (vector<FileIndex>::const_iterator it = m_vtIndex.begin(); it != m_vtIndex.end(); it++)
		//for (uint32_t idxTile = 0; idxTile < fileHeader.cntIndex; idxTile++)
	{
		// read body
		if (it->szBody <= 0) {
			continue;
		}
		else if (it->offBody <= 0)
		{
			LOG_TRACE(LOG_ERROR, "Failed, index body info invalid, off:%d, size:%d", it->offBody, it->szBody);
			return 0;
		}

		fseek(fp, it->offBody, SEEK_SET);
		if ((retRead = fread(&fileBody, sizeBody, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read body, offset:%d", it->offBody);
			return 0;
		}

		if (it->idTile != it->idTile)
		{
			LOG_TRACE(LOG_ERROR, "Failed, index tile info not match with body, index tile id:%d vs body tile id:%d", it->idTile, it->idTile);
			return 0;
		}

		// mesh
		pMesh = new stMeshInfo;
		pMesh->mesh_id.tile_id = it->idTile;
		memcpy(&pMesh->mesh_box, &it->rtTile, sizeof(pMesh->mesh_box));
		memcpy(&pMesh->data_box, &it->rtData, sizeof(pMesh->data_box));
		for (int ii = 0; ii < 8; ii++)
		{
			if (it->idNeighborTile[ii] <= 0) {
				continue;
			}
			pMesh->neighbors.emplace(it->idNeighborTile[ii]);
		} // for
		AddMeshData(pMesh);

		offFile += it->szBody;
#if defined(_DEBUG)
		//LOG_TRACE(LOG_DEBUG, "Data loading, link data loaded, mesh:%d, link cnt:%lld", pMesh->mesh_id.tile_id, fileBody.link.cntLink);
#endif
	}

	LOG_TRACE(LOG_DEBUG, "Data loading, mesh cnt:%lld", m_vtIndex.size());

	return offFile;
}