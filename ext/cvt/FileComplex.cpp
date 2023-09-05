#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FileComplex.h"

#include "../shp/shpio.h"
#include "../utils/UserLog.h"
#include "../utils/GeoTools.h"
#include "../utils/Strings.h"
#include "../route/MMPoint.hpp"
#include "../route/DataManager.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CFileComplex::CFileComplex()
{
	m_nCpxIdx = 0;
	m_nFileType = TYPE_DATA_COMPLEX;
}

CFileComplex::~CFileComplex()
{
	if (!m_mapComplex.empty()) {
		m_mapComplex.clear();
		unordered_map<uint64_t, stComplex>().swap(m_mapComplex);
	}
}

static const uint32_t g_cntLogPrint = 1000;

bool CFileComplex::ParseData(IN const char* fname)
{
	bool ret = false;

	XSHPReader shpReader;

	if (!shpReader.Open(fname)) {
		LOG_TRACE(LOG_ERROR, "Error,Cannot Find shp File!");
		return ret;
	}

	int nShpType = (int)shpReader.GetShpObjectType(); //shpPoint=1,shpPolyLine=3,shpPolygon=5

	if (nShpType == 5) {
		nShpType = 3; //면

		 // 여러 파일일 경우, 이전에 등록된 id에 이어서 증가
		//if (m_pDataMgr) {
		//	m_nCpxIdx = m_pDataMgr->GetComplexDataCount();
		//}
	} else if (nShpType == 3) {
		nShpType = 2; //선
	} else if (nShpType == 1) {
		nShpType = 1; //점
	}


	//전체 지도영역 얻기
	shpReader.GetEnvelope((SBox*)GetMeshRegion());

	//전체 데이터 카운터 얻기
	long nRecCount = shpReader.GetRecordCount();
	int nFieldCount = shpReader.GetFieldCount();

	//int nLabelIdxToRead = 1;
	char chsTmp[61];
	//int nPart, nPoint;
	SPoint* pPoints;
	unsigned int colCnt = shpReader.GetFieldCount();
	
	// 데이터 필드 무결성 확보를 위한 검사
	// 다른 데이터에도 추가해 주자
	if (strstr(fname, "COMPLEX") != nullptr) {
		static char szComplexField[128][32] = {
			{ "GID" },{ "EQB_MAN_SN" },{ "SIG_CD" },{ "MESH_ID" },{ "NB_MESH_ID" },
		};

		DBF_FIELD_INFO fieldInfo;

		for (int ii = 0; ii < colCnt; ii++) {
			if (shpReader.GetFieldInfo(ii, fieldInfo) == false) {
				LOG_TRACE(LOG_ERROR, "can't get dbf filed info, idx : %d ", ii);
				return false;
			}
			else if (strcmp(szComplexField[ii], trim(fieldInfo.szName)) != 0) {
				LOG_TRACE(LOG_ERROR, "dbf field name not matched the expected name, filedName:%s vs exprectedName:%s", fieldInfo.szName, szComplexField[ii]);
				return false;
			}
		}
	}

	// 분할된 파일 대비
	if (!m_mapComplex.empty()) {
		m_mapComplex.reserve(nRecCount + m_mapComplex.size());
	}
	else {
		m_mapComplex.reserve(nRecCount);
	}


	//데이터 얻기
	LOG_TRACE(LOG_DEBUG, "LOG, start, raw data parsing file : %s", fname);

	
	SHPGeometry* pSHPObj = nullptr;
	string strKey;

	for (int idxRec = 0; idxRec < nRecCount; idxRec++) {
		stComplex complex;

		ret = false;

		//속성가져오기
		if (shpReader.Fetch(idxRec) == false) { //error to read..
			LOG_TRACE(LOG_ERROR, "ERROR,Record %d is not available!!", idxRec);
			continue;
		}

		//Setting DBF Data
		for (unsigned int idxCol = 0; idxCol < colCnt; idxCol++) {
			memset(chsTmp, 0x00, sizeof(chsTmp));
			shpReader.GetDataAsString(idxCol, chsTmp, 60);  //nLabelIdxToRead 번째 필드 값을 chsTmp로 12byte 읽어옴.

			pSHPObj = shpReader.GetGeometry();

			if (nShpType == 3) { // 면
				memcpy(&complex.Box, &pSHPObj->polygon.Box, sizeof(SBox));
				if (!SetData_Complex(idxCol, complex, chsTmp))
				{
					LOG_TRACE(LOG_ERROR, "Error, %s", GetErrorMsg());
					break;
				}
				if (idxCol == 0)
				{
					//nPoint = SHP_GetNumPartPoints(pSHPObj, 0); //파트 0에 대한	   //선형 갯수..					 																	 
					pPoints = SHP_GetPartPoints(pSHPObj, 0); //파트 0에 대한	     //실제선형데이터..

					complex.vtParts.assign(pSHPObj->polygon.Parts, pSHPObj->polygon.Parts + pSHPObj->polygon.NumParts);
					complex.vtVertex.assign(pPoints, pPoints + pSHPObj->polygon.NumPoints);
				}

				ret = true;
			}
			else {
				LOG_TRACE(LOG_ERROR, "Unknown type, type:%s", nShpType);
				break;
			}
		}

		if (ret == false) {
			break;
		}


#if defined(_USE_TEST_MESH)
		bool isContinue = true;
		for (const auto& item : g_arrTestMesh) {
			if (item == complex.MeshID) {
				isContinue = false;
				break;
			}
		}
		if (isContinue) continue;
#endif


		strKey = complex.CpxId + complex.SdSggCode;
		complex.keyCpx = AddStringId(strKey.c_str(), complex.MeshID);
		if (complex.keyCpx.llid == NULL_VALUE) {
			LOG_TRACE(LOG_ERROR, "Error, can't find complex id, mesh:%d, id:%s", complex.MeshID, complex.CpxId.c_str());
			break;
		}

		complex.keyCpx.ent.type = 1; // 타입, 0:빌딩, 1:단지

		if (m_mapComplex.find(complex.keyCpx.llid) != m_mapComplex.end()) {
			// 중복
			LOG_TRACE(LOG_WARNING, "complex id duplicated, str:'%s', mesh:%d, id:%d", complex.CpxId.c_str(), complex.keyCpx.tile_id, complex.keyCpx.nid);
			continue;
		}

		m_mapComplex.emplace(complex.keyCpx.llid, complex);
	}

	shpReader.Close();

	return true;
}

bool CFileComplex::GenServiceData()
{
	stPolygonInfo* pPoly = nullptr;
	int cntLinks = 0;

	// 클래스에 추가
	LOG_TRACE(LOG_DEBUG, "LOG, start, add to complex class");

	vector<KeyID> vtLinks;
	vtLinks.reserve(0xFFFF);
	int cntProc = 0;
	int totProc = m_mapComplex.size();
	for (unordered_map<uint64_t, stComplex>::const_iterator itCpx = m_mapComplex.begin(); itCpx != m_mapComplex.end(); itCpx++) {
		pPoly = new stPolygonInfo;

		pPoly->poly_id = itCpx->second.keyCpx;
		pPoly->cpx.code = itCpx->second.Code;
		memcpy(&pPoly->data_box, &itCpx->second.Box, sizeof(itCpx->second.Box));
		//pPoly->vtPts.assign(itCpx->second.vtParts.begin(), itCpx->second.vtParts.end());
		if (!itCpx->second.vtParts.empty()) {
			if (itCpx->second.vtParts.size() > 0xFFFF) {
				LOG_TRACE(LOG_ERROR, "Failed, add complex data, mesh:%d, cpx:%d, parts size:%d", pPoly->poly_id.tile_id, pPoly->poly_id.nid, itCpx->second.vtParts.size());
				return 0;
			}
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_PART, &itCpx->second.vtParts.front(), itCpx->second.vtParts.size());
		}
		//pPoly->vtVtx.assign(itCpx->second.vtVertex.begin(), itCpx->second.vtVertex.end());
		if (!itCpx->second.vtVertex.empty()) {
			if (itCpx->second.vtVertex.size() > 0xFFFF) {
				LOG_TRACE(LOG_ERROR, "Failed, add complex data, mesh:%d, cpx:%d, vertex size:%d", pPoly->poly_id.tile_id, pPoly->poly_id.nid, itCpx->second.vtVertex.size());
				return 0;
			}
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_VTX, &itCpx->second.vtVertex.front(), itCpx->second.vtVertex.size());
		}
		//pPoly->vtJoinedMesh.assign(itCpx->second.vtJoinedMesh.begin(), itCpx->second.vtJoinedMesh.end());
		if (!itCpx->second.vtJoinedMesh.empty()) {
			if (itCpx->second.vtJoinedMesh.size() > 0xFFFF) {
				LOG_TRACE(LOG_ERROR, "Failed, add complex data, mesh:%d, cpx:%d, joined mesh size:%d", pPoly->poly_id.tile_id, pPoly->poly_id.nid, itCpx->second.vtJoinedMesh.size());
				return 0;
			}
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_MESH, &itCpx->second.vtJoinedMesh.front(), itCpx->second.vtJoinedMesh.size());
		}

		if (!itCpx->second.vtEntrance.empty()) {
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_ENT, &itCpx->second.vtEntrance.front(), itCpx->second.vtEntrance.size());
		}

		vtLinks.clear();
		cntLinks = getLinksInComplex(itCpx->second, vtLinks);
		if (cntLinks && !vtLinks.empty()) {
			if (vtLinks.size() > 0xFFFF) {
				LOG_TRACE(LOG_ERROR, "Failed, add complex data, mesh:%d, cpx:%d, link size:%d", pPoly->poly_id.tile_id, pPoly->poly_id.nid, vtLinks.size());
				return 0;
			}
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_LINK, &vtLinks.front(), vtLinks.size());
		}

		//if (cntLinks > 0) {
		//	LOG_TRACE(LOG_DEBUG, "links in complex region, mesh:%d, cpx:%d, pit in link count:%d", pCpx->poly_id.tile_id, pCpx->poly_id.nid, cntLinks);
		//}
		
		//// 단지 파트 공유에 등록된 단지내 링크 정보를 각 파트에 재 할당
		//stComplexShare* pCpxIds = nullptr;
		//if (GetCpxShare(itCpx->second->poly_id.nid, &pCpxIds) <= 0) {
		//	LOG_TRACE(LOG_ERROR, "Failed, can't find complex share id from complex id, id:%d", itCpx->second->poly_id.nid);
		//}
		//else {
		//	for (vector<KeyID>::const_iterator itParts = pCpxIds->vtCpxParts.begin(); itParts != pCpxIds->vtCpxParts.end(); itParts++) {
		//		if (itCpx->second->poly_id == *itParts && !pCpxIds->setLinks.empty()) {
		//			itCpx->second->vtLink.assign(pCpxIds->setLinks.begin(), pCpxIds->setLinks.end());
		//			LOG_TRACE(LOG_DEBUG, "complex shared links assign to each complex parts, mesh:%d, cpx:%d, copyed:%d", itCpx->second->poly_id.tile_id, itCpx->second->poly_id.nid, itCpx->second->vtLink.size());
		//			break;
		//		}
		//	}
		//}

		if (!AddPolygonData(pPoly)) {
			LOG_TRACE(LOG_ERROR, "Failed, add complex data, mesh:%d, cpx:%d", pPoly->poly_id.tile_id, pPoly->poly_id.nid);

			stMeshInfo* pMeshExt = new stMeshInfo;
			pMeshExt->mesh_id.tile_id = pPoly->poly_id.tile_id;

			if (AddMeshData(pMeshExt) && AddPolygonData(pPoly)) {
				LOG_TRACE(LOG_DEBUG, "add new mesh, when mesh info not exist, mesh:%d, cpx:%d", pMeshExt->mesh_id.tile_id, pPoly->poly_id.nid);
			}
		}

		if (++cntProc % g_cntLogPrint == 0) {
			LOG_TRACE(LOG_DEBUG, "LOG, processing, complex: %d/%d", cntProc, totProc);
		}
	}

	// Entrance에서 사용하니까 릴리즈시점 조절 필요
	//Release();
	if (!m_mapComplex.empty()) {
		m_mapComplex.clear();
		unordered_map<uint64_t, stComplex>().swap(m_mapComplex);
	}

	LOG_TRACE(LOG_DEBUG, "LOG, finished");

	return true;
}


void CFileComplex::AddDataFeild(IN const int idx, IN const int type, IN const char* colData)
{

}


void CFileComplex::AddDataRecord()
{

}


bool CFileComplex::LoadData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_COMPLEX], g_szTypeExec[TYPE_DATA_COMPLEX]);

	return CFileBase::LoadData(szFileName);
}


size_t CFileComplex::ReadBody(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody = { 0, };
	FilePolygon fileCpx = { 0, };

	stMeshInfo* pMesh;
	stPolygonInfo* pCpx;

	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeBody = sizeof(FileBody);

	for (int idx = 0; idx < m_vtIndex.size(); idx++)
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


		// complex
		for (uint32_t ii = 0; ii < fileBody.area.cntPolygon; ii++)
		{
			// read complex
			if ((retRead = fread(&fileCpx, sizeof(fileCpx), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read complex, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			//pMesh->complexs.push_back(fileCpx.polygon_id);

			pCpx = new stPolygonInfo;

			pCpx->poly_id = fileCpx.polygon_id;
			memcpy(&pCpx->data_box, &fileCpx.rtBox, sizeof(fileCpx.rtBox));
			pCpx->sub_info = fileCpx.sub_info;

			// read parts
			//uint32_t part;
			//for (uint32_t jj = 0; jj < fileCpx.parts; jj++)
			//{
			//	if ((retRead = fread(&part, sizeof(part), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't read parts, mesh_id:%d, complex idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
			//		return 0;
			//	}
			//	pCpx->vtPts.push_back(part);
			//}
			if (fileCpx.parts > 0 && !pCpx->readAttribute(TYPE_POLYGON_DATA_ATTR_PART, fp, fileCpx.parts))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read parts, mesh_id:%d, parts idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			// read vertext
			//SPoint point;
			//for (uint32_t jj = 0; jj < fileCpx.points; jj++)
			//{
			//	if ((retRead = fread(&point, sizeof(point), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't read point, mesh_id:%d, complex idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
			//		return 0;
			//	}
			//	pCpx->vtVtx.push_back(point);
			//}
			if (fileCpx.points > 0 && !pCpx->readAttribute(TYPE_POLYGON_DATA_ATTR_VTX, fp, fileCpx.points))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read points, mesh_id:%d, points idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			// read links
			//KeyID linkid;
			//for (uint32_t jj = 0; jj < fileCpx.links; jj++)
			//{
			//	if ((retRead = fread(&linkid, sizeof(linkid), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't read link id, mesh_id:%d, complex idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
			//		return 0;
			//	}
			//	pCpx->vtLink.push_back(linkid);
			//}
			if (fileCpx.links > 0 && !pCpx->readAttribute(TYPE_POLYGON_DATA_ATTR_LINK, fp, fileCpx.links))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read link, mesh_id:%d, links idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			// read joined mesh
			//uint32_t joined;
			//for (uint32_t jj = 0; jj < fileCpx.joins; jj++)
			//{
			//	if ((retRead = fread(&joined, sizeof(joined), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't read joined mesh, mesh_id:%d, complex idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
			//		return 0;
			//	}
			//	pCpx->vtJoinedMesh.push_back(joined);
			//}
			if (fileCpx.meshes > 0 && !pCpx->readAttribute(TYPE_POLYGON_DATA_ATTR_MESH, fp, fileCpx.meshes))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read joined mesh, mesh_id:%d, joins idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			AddPolygonData(pCpx);
		}

		offFile += m_vtIndex[idx].szBody;

#if defined(_DEBUG)
		LOG_TRACE(LOG_DEBUG, "Data loading, complex data loaded, mesh:%d, cnt:%lld", pMesh->mesh_id.tile_id, fileBody.area.cntPolygon);
#endif
	}

	return offFile;
}


bool CFileComplex::LoadDataByIdx(IN const uint32_t idx)
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
	FilePolygon fileCpx = { 0, };

	stMeshInfo* pMesh;
	stPolygonInfo* pCpx;
	
	size_t offFile = 0;
	size_t offItem = 0;
	size_t retRead = 0;

	// read index
	pMesh = new stMeshInfo;
	//memset(pMesh, 0x00, sizeof(stMeshInfo));




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


	// complex
	for (uint32_t ii = 0; ii < fileBody.area.cntPolygon; ii++)
	{
		// read complex
		if ((retRead = fread(&fileCpx, sizeof(fileCpx), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read complex, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return false;
		}

		//pMesh->complexs.push_back(fileCpx.polygon_id);

		pCpx = new stPolygonInfo;

		pCpx->poly_id = fileCpx.polygon_id;
		memcpy(&pCpx->data_box, &fileCpx.rtBox, sizeof(fileCpx.rtBox));
		pCpx->sub_info = fileCpx.sub_info;

		// read parts
		//uint32_t part;
		//for (uint32_t jj = 0; jj < fileCpx.parts; jj++)
		//{
		//	if ((retRead = fread(&part, sizeof(part), 1, fp)) != 1)
		//	{
		//		LOG_TRACE(LOG_ERROR, "Failed, can't read parts, mesh_id:%d, complex idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
		//		fclose(fp);
		//		return false;
		//	}
		//	pCpx->vtPts.push_back(part);
		//}
		if (fileCpx.parts > 0 && !pCpx->readAttribute(TYPE_POLYGON_DATA_ATTR_PART, fp, fileCpx.parts))
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read parts, mesh_id:%d, parts idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return 0;
		}

		// read vertext
		//SPoint point;
		//for (uint32_t jj = 0; jj < fileCpx.points; jj++)
		//{
		//	if ((retRead = fread(&point, sizeof(point), 1, fp)) != 1)
		//	{
		//		LOG_TRACE(LOG_ERROR, "Failed, can't read point, mesh_id:%d, complex idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
		//		fclose(fp);
		//		return false;
		//	}
		//	pCpx->vtVtx.push_back(point);
		//}
		if (fileCpx.points > 0 && !pCpx->readAttribute(TYPE_POLYGON_DATA_ATTR_VTX, fp, fileCpx.points))
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read points, mesh_id:%d, points idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return 0;
		}

		// read links
		//KeyID linkid;
		//for (uint32_t jj = 0; jj < fileCpx.links; jj++)
		//{
		//	if ((retRead = fread(&linkid, sizeof(linkid), 1, fp)) != 1)
		//	{
		//		LOG_TRACE(LOG_ERROR, "Failed, can't read link id, mesh_id:%d, complex idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
		//		return 0;
		//	}
		//	pCpx->vtLink.push_back(linkid);
		//}
		if (fileCpx.links > 0 && !pCpx->readAttribute(TYPE_POLYGON_DATA_ATTR_LINK, fp, fileCpx.links))
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read link, mesh_id:%d, links idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return 0;
		}

		// read joined mesh
		//uint32_t joined;
		//for (uint32_t jj = 0; jj < fileCpx.joins; jj++)
		//{
		//	if ((retRead = fread(&joined, sizeof(joined), 1, fp)) != 1)
		//	{
		//		LOG_TRACE(LOG_ERROR, "Failed, can't read joined mesh, mesh_id:%d, complex idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
		//		fclose(fp);
		//		return false;
		//	}
		//	pCpx->vtJoinedMesh.push_back(joined);
		//}
		if (fileCpx.meshes > 0 && !pCpx->readAttribute(TYPE_POLYGON_DATA_ATTR_MESH, fp, fileCpx.meshes))
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read joined mesh, mesh_id:%d, joins idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return 0;
		}

		AddPolygonData(pCpx);
	}

	fclose(fp);

	return true;
}


bool CFileComplex::Initialize()
{
	return CFileBase::Initialize();
}

void CFileComplex::Release()
{
	m_nCpxIdx = 0;

	if (!m_mapComplex.empty()) {
		m_mapComplex.clear();
		unordered_map<uint64_t, stComplex>().swap(m_mapComplex);
	}

	if (!m_mapStringId.empty()) {
		m_mapStringId.clear();
		unordered_map<string, KeyID>().swap(m_mapStringId);
	}

	CFileBase::Release();
}


bool CFileComplex::SaveData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_COMPLEX], g_szTypeExec[TYPE_DATA_COMPLEX]);

	return CFileBase::SaveData(szFileName);
}


//size_t CFileComplex::WriteHeader(FILE* fp, FileHeader* pHeader)
//{
//	FileBase stFileBase = { 0, };
//	size_t offFile = 0;
//	size_t retWrite = 0;
//	size_t retRead = 0;
//	const size_t sizeHeader = sizeof(FileHeader);
//
//	if (!fp || !pHeader) {
//		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p, pHeader:%p", fp, pHeader);
//		return 0;
//	}
//
//	memcpy(&pHeader->rtMap, GetMeshRegion(), sizeof(pHeader->rtMap));
//	pHeader->cntIndex = GetMeshCount();
//	pHeader->offIndex = sizeHeader;
//	pHeader->offBody = sizeHeader + sizeof(FileIndex) * pHeader->cntIndex;
//
//	if ((retWrite = fwrite(pHeader, sizeHeader, 1, fp)) != 1)
//	{
//		LOG_TRACE(LOG_ERROR, "Failed, can't write header, written:%d", retWrite);
//		fclose(fp);
//		return 0;
//	}
//	offFile += sizeHeader;
//
//	LOG_TRACE(LOG_DEBUG, "Save data, header, mesh cnt:%lld", pHeader->cntIndex);
//
//	return offFile;
//}


//size_t CFileComplex::WriteIndex(FILE* fp, vector<FileIndex>* pvtFileIndex)
//{
//	size_t offFile = 0;
//	size_t retWrite = 0;
//	size_t retRead = 0;
//	const size_t sizeIndex = sizeof(FileIndex);
//	FileIndex fileIndex;
//	stMeshInfo* pMesh = nullptr;
//
//	if (!fp || !pvtFileIndex) {
//		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p, pHeader:%p", fp, pvtFileIndex);
//		return 0;
//	}
//
//	for (uint32_t idx = 0; idx < GetMeshCount(); idx++)
//	{
//		pMesh = GetMeshData(idx);
//		if (!pMesh)
//		{
//			LOG_TRACE(LOG_ERROR, "Failed, can't access mesh, idx:%d", idx);
//			return 0;
//		}
//
//		fileIndex.idxTile = idx;
//		fileIndex.idTile = pMesh->mesh_id.tile_id;
//		memcpy(&fileIndex.rtTile, &pMesh->mesh_box, sizeof(fileIndex.rtTile));
//		memcpy(&fileIndex.rtData, &pMesh->data_box, sizeof(fileIndex.rtData));
//		fileIndex.szBody = 0;
//		fileIndex.offBody = 0;
//
//		if ((retWrite = fwrite(&fileIndex, sizeIndex, 1, fp)) != 1)
//		{
//			LOG_TRACE(LOG_ERROR, "Failed, can't write index[%d], written:%d", idx, retWrite);
//			return 0;
//		}
//		offFile += sizeIndex;
//
//		// add body data info buff
//		pvtFileIndex->push_back(fileIndex);
//	}
//
//	LOG_TRACE(LOG_DEBUG, "Save data, mesh index, mesh cnt:%lld", pvtFileIndex->size());
//
//	return offFile;
//}


size_t CFileComplex::WriteBody(FILE* fp, IN const uint32_t fileOff)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	stMeshInfo* pMesh = nullptr;
	stPolygonInfo* pCpx = nullptr;

	FileBody fileBody;
	FilePolygon fileCpx = { 0, };

	size_t offFile = fileOff;
	size_t retWrite = 0;
	size_t retRead = 0;
	long offItem = 0;
	const size_t sizeFileBody = sizeof(fileBody);

	uint32_t ii, jj;

	// write body - complex, vertex
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


		// write complex
# if defined(USE_OPTIMAL_POINT_API)
		for (jj = 0; jj < pMesh->complexs.size(); jj++)
		{
			pCpx = m_pDataMgr->GetComplexDataById(pMesh->complexs[jj]);
			if (!pCpx)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't access complex, idx:%d, tile_id:%d, id:%d", ii, pMesh->complexs[ii].tile_id, pMesh->complexs[ii].nid);
				return 0;
			}
			else if (pMesh->mesh_id.tile_id != pCpx->poly_id.tile_id) {
				// 중첩된 메쉬 정보이므로 자신이 아니면 저장하지 않는다.
				continue;
			}

			fileCpx.polygon_id = pCpx->poly_id;
			memcpy(&fileCpx.rtBox, &pCpx->data_box, sizeof(fileCpx.rtBox));
			//fileCpx.parts = pCpx->vtPts.size();
			fileCpx.parts = pCpx->getAttributeCount(TYPE_POLYGON_DATA_ATTR_PART);
			//fileCpx.points = pCpx->vtVtx.size();
			fileCpx.points = pCpx->getAttributeCount(TYPE_POLYGON_DATA_ATTR_VTX);
			//fileCpx.links = pCpx->vtLink.size();
			fileCpx.links = pCpx->getAttributeCount(TYPE_POLYGON_DATA_ATTR_LINK);
			//fileCpx.joins = pCpx->vtJoinedMesh.size();
			fileCpx.meshes = pCpx->getAttributeCount(TYPE_POLYGON_DATA_ATTR_MESH);
			fileCpx.sub_info = pCpx->sub_info;

			if ((retWrite = fwrite(&fileCpx, sizeof(fileCpx), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't write complex[%d], tile_id:%d, id:%d", jj, fileCpx.polygon_id.tile_id, fileCpx.polygon_id.nid);
				return 0;
			}

			fileBody.area.cntPolygon++;
			offItem += sizeof(fileCpx);


			// write vertex parts
			//for (kk = 0; kk < fileCpx.parts; kk++)
			//{
			//	if ((retWrite = fwrite(&pCpx->vtPts[kk], sizeof(uint32_t), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't write complex[%d] parts[%d], ", jj, kk);
			//		return 0;
			//	}
			//	offItem += sizeof(uint32_t);
			//}
			if (fileCpx.parts > 0)
			{
				if (!(retWrite = pCpx->writeAttribute(TYPE_POLYGON_DATA_ATTR_PART, fp))) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write complex[%d], parts", jj);
					return 0;
				}
				offItem += retWrite;
			}

			// write vertex
			//for (kk = 0; kk < fileCpx.points; kk++)
			//{
			//	if ((retWrite = fwrite(&pCpx->vtVtx[kk], sizeof(SPoint), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't write complex[%d] vertex[%d], ", jj, kk);
			//		return 0;
			//	}
			//	offItem += sizeof(SPoint);
			//}
			if (fileCpx.points > 0)
			{
				if (!(retWrite = pCpx->writeAttribute(TYPE_POLYGON_DATA_ATTR_VTX, fp))) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write complex[%d], vertex", jj);
					return 0;
				}
				offItem += retWrite;
			}

			// write links id
			//for (kk = 0; kk < fileCpx.links; kk++)
			//{
			//	if ((retWrite = fwrite(&pCpx->vtLink[kk], sizeof(uint64_t), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't write complex[%d], links[%d], ", jj, kk);
			//		return 0;
			//	}
			//	offItem += sizeof(uint64_t);
			//}
			if (fileCpx.links > 0)
			{
				if (!(retWrite = pCpx->writeAttribute(TYPE_POLYGON_DATA_ATTR_LINK, fp))) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write complex[%d], links", jj);
					return 0;
				}
				offItem += retWrite;
			}

			// write joined mesh
			//for (kk = 0; kk < fileCpx.joins; kk++)
			//{
			//	if ((retWrite = fwrite(&pCpx->vtJoinedMesh[kk], sizeof(uint32_t), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't write complex[%d] joined mesh[%d], ", jj, kk);
			//		return 0;
			//	}
			//	offItem += sizeof(uint32_t);
			//}
			if (fileCpx.meshes > 0)
			{
				if (!(retWrite = pCpx->writeAttribute(TYPE_POLYGON_DATA_ATTR_MESH, fp))) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write complex[%d] joined mesh", jj);
					return 0;
				}
				offItem += retWrite;
			}

		} // write complex

		LOG_TRACE(LOG_DEBUG, "Save data, complex, cnt:%lld", jj);
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


uint32_t CFileComplex::getComplexCode(IN const char* code)
{
	uint32_t nCode = TYPE_COMPLEX_NOT;

	if (code && strlen(code) > 0 && strlen(code) < 50)
	{
		if (strcmp(code, "APC") == 0) {
			nCode = TYPE_COMPLEX_APC; //주상복합단지
		} 
		else if (strcmp(code, "APL") == 0) {
			nCode = TYPE_COMPLEX_APL; //아파트형공장단지
		} 
		else if (strcmp(code, "APT") == 0) {
			nCode = TYPE_COMPLEX_APT; //아파트단지
		}
		else if (strcmp(code, "B01") == 0) {
			nCode = TYPE_COMPLEX_B01; //공공기관단지
		}
		else if (strcmp(code, "B04") == 0) {
			nCode = TYPE_BUILDING_B04; //교육기관단지
		}
		else if (strcmp(code, "B11") == 0) {
			nCode = TYPE_COMPLEX_B11; //편의시설단지
		}
		else if (strcmp(code, "ETC") == 0) {
			nCode = TYPE_COMPLEX_ETC; //기타단지
		}
		else if (strcmp(code, "OFT") == 0) {
			nCode = TYPE_COMPLEX_OFT; //오피스텔단지
		}
		else if (strcmp(code, "ROH") == 0) {
			nCode = TYPE_COMPLEX_ROH; //연립주택단지
		}
		else if (strcmp(code, "SCH") == 0) {
			nCode = TYPE_COMPLEX_SCH; //학교단지
		}
		else {
			nCode = TYPE_COMPLEX_NOT;
			//nCode = TYPE_COMPLEX_ETC;
		}			
	}
	
	return nCode;
}


bool  CFileComplex::SetData_Complex(int idx, stComplex &getComplex_Dbf, char* colData)
{
	bool ret = true;

	switch (idx)
	{
	case 0:		getComplex_Dbf.GID = atoi(trim(colData)); break; // GID
	case 1:		getComplex_Dbf.CpxId = trim(colData);		break;
	case 2:		getComplex_Dbf.SdSggCode = trim(colData); break;
	case 3:		getComplex_Dbf.MeshID = atoi(trim(colData)); break;
	case 4: {
		if (trim(colData) > 0) {
			char* pTok = strsep(&colData, "|");
			uint32_t idJoinedMesh;
			while (pTok) {
				idJoinedMesh = atoi(trim(pTok));
				getComplex_Dbf.vtJoinedMesh.emplace_back(idJoinedMesh);
				pTok = strsep(&colData, "|");
			}
		}
	} break; // case 4:

	default:	break;
	} // switch

	return ret;
}


uint32_t CFileComplex::getLinksInComplex(IN const stComplex& complex, OUT vector<KeyID>& vtLinks) // 단지내 도로 정보 처리
{
	int cntInCpxLink = 0;

#if defined(USE_OPTIMAL_POINT_API) && defined(USE_VEHICLE_DATA)
	if (complex.vtJoinedMesh.empty()) {
		return cntInCpxLink;
	}
	
	// 현재 메쉬 및 인접 메쉬 확인
	//vector<uint32_t> vtMeshs;
	//vtMeshs.emplace_back((uint32_t)pCpx->poly_id.tile_id); // 단지 데이터의 중첩 메쉬에 자신이 이미 포함됨

	// 이웃 메쉬가 생겼으니 그걸 사용하자
	//stMeshInfo* pMesh = m_pDataMgr->GetMeshDataById(pCpx->poly_id.tile_id);
	//stMeshInfo* pMeshNeighbor;
	//for (unordered_set<uint32_t>::const_iterator it = pMesh->neighbors.begin(); it != pMesh->neighbors.end(); it++) {
	//	pMeshNeighbor = m_pDataMgr->GetMeshDataById(*it);
	//	
	//	// 각 메쉬에 단지 폴리곤 영역이 포함 되는지 확인
	//	if (isInPitBox(pCpx->data_box, pMeshNeighbor->mesh_box) == true) {
	//		vtMeshs.emplace_back(pMeshNeighbor->mesh_id);
	//	}
	//}
	//for (int ii = complex.vtJoinedMesh.size() - 1; ii >= 0; --ii) {
	//	vtMeshs.emplace_back(complex.vtJoinedMesh[ii]);
	//}

	uint32_t cntPtx = complex.vtParts.size();
	uint32_t cntVtx = complex.vtVertex.size();

	for (vector<uint32_t>::const_iterator itMesh = complex.vtJoinedMesh.begin(); itMesh != complex.vtJoinedMesh.end(); itMesh++) {

		stMeshInfo* pMesh = m_pDataMgr->GetMeshDataById(*itMesh);

		if (pMesh && !pMesh->vlinks.empty()) {
			stLinkInfo* pLink;
			bool isInCpxLink = false;

			// 해당 메쉬내 링크(단지내도로레벨(9) 속성) 확인
			for (vector<KeyID>::const_iterator it = pMesh->vlinks.begin(); it != pMesh->vlinks.end(); it++) {
				isInCpxLink = false;
				pLink = m_pDataMgr->GetVLinkDataById(*it);
				if (pLink != nullptr && pLink->sub_info != NOT_USE && pLink->veh.level >= 9) {
					// s link 확인
					int nLinkIdx = 0;
					//if (isInBox(pLink->vtPts[nLinkIdx].x, pLink->vtPts[nLinkIdx].y, complex.Box)) {
					if (isInBox(pLink->getVertexX(nLinkIdx), pLink->getVertexY(nLinkIdx), complex.Box)) {
						uint32_t offPolygon = 0;
						//uint32_t cntPolygon = pCpx->vtVtx.size();
						uint32_t cntPolygon = cntVtx;
						//for (vector<uint32_t>::const_reverse_iterator itParts = pCpx->vtPts.rbegin(); itParts != pCpx->vtPts.rend(); itParts++) {
						for (int32_t itParts = cntPtx - 1; itParts >= 0; --itParts) {
							offPolygon = complex.vtParts[itParts];
							cntPolygon -= offPolygon;
							//isInCpxLink = isPointInPolygon(pLink->vtPts[nLinkIdx].x, pLink->vtPts[nLinkIdx].y, &pCpx->vtVtx[offPolygon], cntPolygon);
							//isInCpxLink = isPointInPolygon(pLink->vtPts[nLinkIdx].x, pLink->vtPts[nLinkIdx].y, &complex.vtVertex[offPolygon], cntPolygon);
							isInCpxLink = isPointInPolygon(pLink->getVertexX(nLinkIdx), pLink->getVertexY(nLinkIdx), &complex.vtVertex[offPolygon], cntPolygon);
							if (isInCpxLink) {
								//complex.vtLink.emplace_back(pLink->link_id);
								vtLinks.emplace_back(pLink->link_id);
								// 공동 데이터로 쓰기위해 우선 임시 저장
								//pCpxIds->setLinks.emplace(pLink->link_id);
								cntInCpxLink++;
								break;
							}
							cntPolygon = cntVtx - cntPolygon;
						} // for vtx
					}

					// 만족 안되었으면 e link도 확인
					nLinkIdx = pLink->getVertexCount() - 1;
					//if (!isInCpxLink && isInBox(pLink->vtPts[nLinkIdx].x, pLink->vtPts[nLinkIdx].y, complex.Box)) {
					if (!isInCpxLink && isInBox(pLink->getVertexX(nLinkIdx), pLink->getVertexY(nLinkIdx), complex.Box)) {
						uint32_t offPolygon = 0;
						//uint32_t cntPolygon = pCpx->vtVtx.size();
						uint32_t cntPolygon = cntVtx;
						//for (vector<uint32_t>::const_reverse_iterator itParts = pCpx->vtPts.rbegin(); itParts != pCpx->vtPts.rend(); itParts++) {
						for (int32_t itParts = cntPtx - 1; itParts >= 0; --itParts) {
							offPolygon = complex.vtParts[itParts];
							cntPolygon -= offPolygon;
							//isInCpxLink = isPointInPolygon(pLink->vtPts[nLinkIdx].x, pLink->vtPts[nLinkIdx].y, &pCpx->vtVtx[offPolygon], cntPolygon);
							//isInCpxLink = isPointInPolygon(pLink->vtPts[nLinkIdx].x, pLink->vtPts[nLinkIdx].y, &complex.vtVertex[offPolygon], cntPolygon);
							isInCpxLink = isPointInPolygon(pLink->getVertexX(nLinkIdx), pLink->getVertexY(nLinkIdx), &complex.vtVertex[offPolygon], cntPolygon);
							if (isInCpxLink) {
								//complex.vtLink.emplace_back(pLink->link_id);
								vtLinks.emplace_back(pLink->link_id);
								// 공동 데이터로 쓰기위해 우선 임시 저장
								//pCpxIds->setLinks.emplace(pLink->link_id);
								cntInCpxLink++;
								break;
							}
							cntPolygon = cntVtx - cntPolygon;
						} // for vtx
					}
				}

				if (isInCpxLink && pLink != nullptr && pLink->veh.link_dtype == 0) {
					pLink->veh.link_dtype = 3; // 단지내 도로 확정					
				}
			} // for links

			
		} // mesh
	} // for
#endif // # if defined(USE_OPTIMAL_POINT_API) && defined(USE_VEHICLE_DATA)

	return cntInCpxLink;
}

//bool CFileComplex::AddMeshDataByComplex(IN const stMeshInfo * pInfo, IN const stPolygonInfo * pData)
//{
//	if (AddMeshData(pInfo)) {
//		if (m_pDataMgr) {
//			return m_pDataMgr->AddMeshDataByComplex(pInfo, pData);
//		}
//	}
//
//	return false;
//}


bool CFileComplex::AddEntranceData(IN const KeyID keyId, IN const stEntranceInfo& entInfo)
{
	bool ret = false;

	if (keyId.llid != NULL_VALUE) {		
		unordered_map<uint64_t, stComplex>::iterator it = m_mapComplex.find(keyId.llid);
		if (it != m_mapComplex.end()) {
			it->second.vtEntrance.emplace_back(entInfo);
			ret = true;
		}
	}

	return ret;
}


const stComplex* CFileComplex::GetComplexData(IN const KeyID keyId) const
{
	const stComplex* pCpx = nullptr;

	if (keyId.llid != NULL_VALUE) {
		unordered_map<uint64_t, stComplex>::const_iterator it = m_mapComplex.find(keyId.llid);
		if (it != m_mapComplex.end()) {
			pCpx = &it->second;
		}
	}

	return pCpx;
}


const KeyID CFileComplex::AddStringId(IN const char* szStringId, IN const uint32_t meshId)
{
	KeyID retKey = { NULL_VALUE, };

	if (szStringId && ((retKey = GetIdFromStringId(szStringId)).llid == NULL_VALUE)) {
		string strId(szStringId);
		retKey.ent.nid = m_nCpxIdx++;
		retKey.ent.tile_id = meshId;
		retKey.ent.type = 1; // complex

		m_mapStringId.emplace(strId, retKey);
	}

	return retKey;
}


const KeyID CFileComplex::GetIdFromStringId(IN const char* szStringId)
{
	KeyID retKey = { NULL_VALUE, };

	if (szStringId && !m_mapStringId.empty()) {
		unordered_map<string, KeyID>::const_iterator it = m_mapStringId.find(szStringId);
		if (it != m_mapStringId.end()) {
			retKey = it->second;
		}
	}

	return retKey;
}


// const int32_t CFileComplex::GetCpxShare(IN const uint32_t cpxId, stComplexShare** pIds)
//{
//	uint32_t cntIds = 0;
//
//	stComplexShare* pParts = nullptr;
//
//	unordered_map<uint32_t, stComplexShare>::iterator it = m_mapCpxShare.find(cpxId);
//	if (it != m_mapCpxShare.end()) {
//		cntIds = it->second.vtCpxParts.size();
//		*pIds = &(it->second);
//	}
//
//	return cntIds;
//}