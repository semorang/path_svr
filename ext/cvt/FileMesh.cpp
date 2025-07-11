#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FileMesh.h"

#include "../shp/shpio.h"
#include "../utils/UserLog.h"
#include "../utils/Strings.h"
#include "../utils/GeoTools.h"
#include "../route/MMPoint.hpp"
#include "../route/DataManager.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static int32_t biggestMeshId = 0;
static int32_t smallestMeshId = INT_MAX;

bool g_isUseTestMesh = false;

unordered_set<int32_t> g_arrTestMesh = {
	185402, 185407, 185412, 185417, 186402, 186407, 186412, 186417,
	185401, 185406, 185411, 185416, 186401, 186406, 186411, 186416,
	185400, 185405, 185410, 185415, 186400, 186405, 186410, 186415, // 서울
	185303, 185308, 185313, 185318, 186303, 186308, 186313, 186318, // 성남
	185302, 185307, 185312, 185317, 186302, 186307, 186312, 186317, // 하남
	185301, 185306, 185311, 185316, 186301, 186306, 186311, 186316, // 과천
	185300, 185305, 185310, 185315, 186300, 186305, 186310, 186315, // 청계산
	185203, 185208, 185213, 185218, 186203, 186208, 186213, 186218, // 백운산
	185202, 185207, 185212, 185217, 186202, 186207, 186212, 186217,
	185201, 185206, 185211, 185216, 186201, 186206, 186211, 186216,
	// 80

	//223115, 223116, 223117, 223118, 224103 // 대구 p2p 실증지역 for KS 

#if defined(USE_FOREST_DATA)
	// 설악산 - 78
	//232612, 232613, 232710, 232711, 232712, 232713, 232810, 232811, 232812, 232813, 242110,
	//232617, 232618, 232715, 232716, 232717, 232718, 232815, 232816, 232817, 232818,
	//242115, 233602, 233603, 233700, 233701, 233702, 233703, 233800, 233801, 233802,
	//233803, 243100, 233607, 233608, 233750, 233705, 233706, 233707, 233708, 233805,
	//233806, 233807, 233808, 243105, 233612, 233613, 233710, 233711, 233712, 233713,
	//233810, 233811, 233812, 233813, 243110, 233617, 233618, 233715, 233716, 233717,
	//233718, 233815, 233816, 233817, 233818, 243115, 234602, 234603, 234700, 234701,
	//234702, 234703, 234800, 234801, 234802, 234803, 244100,
#endif

	// 홍도,흑산도 (섬)
	116310, 116315, 116316, 117213, 117218, 117310, 117315, 117316
};


CFileMesh::CFileMesh()
{
	m_nDataType = TYPE_DATA_MESH;
	m_nFileType = TYPE_EXEC_MESH;
}

CFileMesh::~CFileMesh()
{

}

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
			else if (strcmp(szLinkField[ii], strupper(trim(fieldInfo.szName))) != 0) {
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
	time_t timeStart = LOG_TRACE(LOG_DEBUG, "LOG, start, raw data parsing file : %s, rec cnt:%d", fname, nRecCount);

	// 전체 메쉬 등록
	stMeshInfo* pMeshGlobal = new stMeshInfo();
	pMeshGlobal->mesh_id.llid = GLOBAL_MESH_ID;
	//memcpy(&pMesh->mesh_box, GetMeshRegion(), sizeof(pMesh->mesh_box));
	//memcpy(&pMesh->data_box, GetMeshRegion(), sizeof(pMesh->data_box));
	
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

					if (smallestMeshId > mesh.MeshID) {
						smallestMeshId = mesh.MeshID;
					}

					if (biggestMeshId < mesh.MeshID) {
						biggestMeshId = mesh.MeshID;
					}
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


		// 테스트 메쉬가 있으면 정의된 메쉬만 확인하자
		if (g_isUseTestMesh && !g_arrTestMesh.empty()) {
			if (g_arrTestMesh.find(mesh.MeshID) == g_arrTestMesh.end()) {
				continue;
			}
		}


		if (nShpType == 3) {
			stMeshInfo* pMesh = new stMeshInfo();
			pMesh->mesh_id.tile_id = mesh.MeshID;
			for (vector<uint32_t>::const_iterator it = mesh.vtNeighbors.begin(); it != mesh.vtNeighbors.end(); it++) {
				pMesh->neighbors.emplace(*it);
			}
			memcpy(&pMesh->mesh_box, &mesh.meshBox, sizeof(SBox));
			memcpy(&pMesh->data_box, &mesh.meshBox, sizeof(SBox));

			m_mapMesh.insert(pair<uint32_t, stMeshInfo*>((uint32_t)pMesh->mesh_id.tile_id, pMesh));

			// global mesh rect update
			if (pMesh->mesh_id.tile_id != GLOBAL_MESH_ID) {
				extendDataBox(pMeshGlobal->mesh_box, pMesh->mesh_box);
				extendDataBox(pMeshGlobal->data_box, pMesh->data_box);
			}
		}
	} // for

	// 전체 메쉬 등록
	m_mapMesh.insert(pair<uint32_t, stMeshInfo*>((uint32_t)pMeshGlobal->mesh_id.tile_id, pMeshGlobal));


	shpReader.Close();

	if (g_isUseTestMesh) {
		LOG_TRACE(LOG_DEBUG, "LOG, Using test mesh option, test mesh cnt : %d", g_arrTestMesh.size());
	}
	LOG_TRACE(LOG_DEBUG, "LOG, min-max mesh_id: %d - %d, added cnt : %d", smallestMeshId, biggestMeshId, m_mapMesh.size());

	LOG_TRACE(LOG_DEBUG, timeStart, "LOG, finished");

	return GenServiceData();
}

bool CFileMesh::GenServiceData()
{
	unordered_map<uint64_t, uint32_t>::iterator itNodeIndex;
	map<uint32_t, stMeshInfo*>::iterator itMesh;


	// 근접 메쉬 확인
	//LOG_TRACE(LOG_DEBUG, "LOG, start, check mesh neighbor");
	// 이제는 메쉬 shp에 이웃 메쉬 정보가 있으니 따로 처리할 필요 없음 (2022-11-24)
	//SetNeighborMesh();


	// 클래스에 추가
	time_t timeStart = LOG_TRACE(LOG_DEBUG, "LOG, start, add data to class");

	for (itMesh = m_mapMesh.begin(); itMesh != m_mapMesh.end(); itMesh++) {
		if (!AddMeshData(itMesh->second)) {
			LOG_TRACE(LOG_ERROR, "Failed, add mesh data, mesh:%d", itMesh->second->mesh_id.tile_id);
		}
	}


	Release();
	
	LOG_TRACE(LOG_DEBUG, timeStart, "LOG, finished");

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

		memset(&fileIndex, 0x00, sizeIndex);
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

	LOG_TRACE(LOG_DEBUG, "Read data, body, mesh cnt:%lld", m_vtIndex.size());

	return offFile;
}