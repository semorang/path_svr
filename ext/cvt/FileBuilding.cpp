#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FileBuilding.h"

#include "../shp/shpio.h"
#include "../utils/UserLog.h"
#include "../route/MMPoint.hpp"
#include "../route/DataManager.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CFileBuilding::CFileBuilding()
{
	m_nBldIdx = 0;
	m_nFileType = TYPE_DATA_BUILDING;
	m_pFileCpx = nullptr;
}

CFileBuilding::~CFileBuilding()
{
	if (!m_mapBuilding.empty()) {
		m_mapBuilding.clear();
		unordered_map<uint64_t, stBuilding>().swap(m_mapBuilding);
	}
}

static const uint32_t g_cntLogPrint = 100000;

bool CFileBuilding::ParseData(IN const char* fname)
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
		//	m_nBldIdx = m_pDataMgr->GetBuildingDataCount();
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
	
	// 분할된 파일 대비
	if (!m_mapBuilding.empty()) {
		m_mapBuilding.reserve(nRecCount + m_mapBuilding.size());
	}
	else {
		m_mapBuilding.reserve(nRecCount);
	}
	

	//데이터 얻기
	LOG_TRACE(LOG_DEBUG, "LOG, start, raw data parsing file : %s", fname);

	
	SHPGeometry* pSHPObj = nullptr;
	string strKey;

	for (int idxRec = 0; idxRec < nRecCount; idxRec++) {
		stBuilding building;
	
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
				memcpy(&building.Box, &pSHPObj->polygon.Box, sizeof(SBox));
				if (!SetData_Building(idxCol, building, chsTmp))
				{
					LOG_TRACE(LOG_ERROR, "Error, %s", GetErrorMsg());
					break;
				}
				if (idxCol == 0)
				{
					//nPoint = SHP_GetNumPartPoints(pSHPObj, 0); //파트 0에 대한	   //선형 갯수..					 																	 
					pPoints = SHP_GetPartPoints(pSHPObj, 0); //파트 0에 대한	     //실제선형데이터..

					building.vtParts.assign(pSHPObj->polygon.Parts, pSHPObj->polygon.Parts + pSHPObj->polygon.NumParts);
					building.vtVertex.assign(pPoints, pPoints + pSHPObj->polygon.NumPoints);
				}

				ret = true;
			}
			else {
				LOG_TRACE(LOG_ERROR, "Unknown type, type:%s", nShpType);
				break;
			}
		} // for

		if (ret == false) {
			continue;
		}


#if defined(_USE_TEST_MESH)
		bool isContinue = true;
		for (const auto& item : g_arrTestMesh) {
			if (item == building.keyBld.tile_id) {
				isContinue = false;
				break;
			}
		}
		if (isContinue) continue;
#endif


		if (building.keyBld.tile_id <= 0) {
			// 메쉬 오류
			LOG_TRACE(LOG_ERROR, "Error, building mesh id not set, mesh:%d", building.keyBld.tile_id);
			continue;
		}

		building.keyBld = AddStringId(building.BldId.c_str(), building.keyBld.tile_id);
		if (building.keyBld.llid == NULL_VALUE) {
			LOG_TRACE(LOG_ERROR, "Error, can't find building id, mesh:%d, id:%s", building.keyBld.tile_id, building.BldId.c_str());
			continue;
		}

		building.keyBld.ent.type = 0; // 타입, 0:빌딩, 1:단지

		if (m_mapBuilding.find(building.keyBld.llid) != m_mapBuilding.end()) {
			// 중복
			LOG_TRACE(LOG_WARNING, "building id duplicated, str:'%s', mesh:%d, id:%d", building.BldId.c_str(), building.keyBld.tile_id, building.keyBld.nid);
			continue;
		}
		
		m_mapBuilding.emplace(building.keyBld.llid, building);
	}

	shpReader.Close();

	return true;
}

bool CFileBuilding::GenServiceData()
{
	map<uint32_t, stMeshInfo*>::const_iterator itMesh;

	stPolygonInfo* pPoly = nullptr;


	// 클래스에 추가
	LOG_TRACE(LOG_DEBUG, "LOG, start, add to building class");


	for (unordered_map<uint64_t, stBuilding>::const_iterator itBld = m_mapBuilding.begin(); itBld != m_mapBuilding.end(); itBld++) {
		pPoly = new stPolygonInfo;

		pPoly->poly_id = itBld->second.keyBld;
		pPoly->bld.code = itBld->second.Code;
		pPoly->bld.apt_type = itBld->second.NumType;
		pPoly->bld.apt_val = itBld->second.Num;
		pPoly->bld.height = itBld->second.Height;
		//pBld->bld.name = building.Name;
		memcpy(&pPoly->data_box, &itBld->second.Box, sizeof(itBld->second.Box));
		//pBld->vtPts.assign(building.vtParts.begin(), building.vtParts.end());
		if (!itBld->second.vtParts.empty()) {
			if (itBld->second.vtParts.size() > 0xFFFF) {
				LOG_TRACE(LOG_ERROR, "Failed, add building data, mesh:%d, bld:%d, parts size:%d", pPoly->poly_id.tile_id, pPoly->poly_id.nid, itBld->second.vtParts.size());
				return false;
			}
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_PART, &itBld->second.vtParts.front(), itBld->second.vtParts.size());
		}
		//pBld->vtVtx.assign(building.vtVertex.begin(), building.vtVertex.end());
		if (!itBld->second.vtVertex.empty()) {
			if (itBld->second.vtVertex.size() > 0xFFFF) {
				LOG_TRACE(LOG_ERROR, "Failed, add building data, mesh:%d, bld:%d, vertex size:%d", pPoly->poly_id.tile_id, pPoly->poly_id.nid, itBld->second.vtVertex.size());
				return false;
			}
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_VTX, &itBld->second.vtVertex.front(), itBld->second.vtVertex.size());
		}

		if (!itBld->second.vtEntrance.empty()) {
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_ENT, &itBld->second.vtEntrance.front(), itBld->second.vtEntrance.size());
		}
		
				
		if (!AddPolygonData(pPoly)) {
			LOG_TRACE(LOG_ERROR, "Failed, add building data, mesh:%d, bld:%d", pPoly->poly_id.tile_id, pPoly->poly_id.nid);

			stMeshInfo* pMeshExt = new stMeshInfo;
			pMeshExt->mesh_id.tile_id = pPoly->poly_id.tile_id;

			if (AddMeshData(pMeshExt) && AddPolygonData(pPoly)) {
				LOG_TRACE(LOG_DEBUG, "add new mesh, when mesh info not exist, mesh:%d, bld:%d", pMeshExt->mesh_id.tile_id, pPoly->poly_id.nid);
			}
		}
	}


	// Entrance에서 사용하니까 릴리즈시점 조절 필요
	//Release();
	if (!m_mapBuilding.empty()) {
		m_mapBuilding.clear();
		unordered_map<uint64_t, stBuilding>().swap(m_mapBuilding);
	}


	LOG_TRACE(LOG_DEBUG, "LOG, finished");

	return true;
}


void CFileBuilding::AddDataFeild(IN const int idx, IN const int type, IN const char* colData)
{

}


void CFileBuilding::AddDataRecord()
{

}


bool CFileBuilding::SaveData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_BUILDING], g_szTypeExec[TYPE_DATA_BUILDING]);

	return CFileBase::SaveData(szFileName);


	///////////////////////////////////////////
	//// data
	//FILE* fp = fopen(szFileName, "wb+");
	//if (!fp)
	//{
	//	LOG_TRACE(LOG_ERROR, "Failed, can't open file for save data, file:%s", szFilePath);
	//	return false;
	//}


	//// write base
	//offFile = WriteBase(fp);

	//// check offset 
	//if (sizeof(FileBase) != offFile)
	//{
	//	LOG_TRACE(LOG_ERROR, "Failed, file base offset not match, base:%d vs current:%d", sizeof(FileBase), offFile);
	//	fclose(fp);
	//	return false;
	//}


	//// write header
	//offFile += WriteHeader(fp);

	//// check offset 
	//if (m_fileHeader.offIndex != offFile)
	//{
	//	LOG_TRACE(LOG_ERROR, "Failed, file header offset not match, header:%d vs current:%d", m_fileHeader.offIndex, offFile);
	//	fclose(fp);
	//	return false;
	//}


	//// write index
	//offFile += WriteIndex(fp);

	//// check offset 
	//if (m_fileHeader.offBody != offFile)
	//{
	//	LOG_TRACE(LOG_ERROR, "Failed, file header body offset not match, header:%d vs current:%d", m_fileHeader.offBody, offFile);
	//	fclose(fp);
	//	return false;
	//}


	//// write body
	//offFile += WriteBody(fp, offFile);

	//// check offset 
	//if (offFile <= 0)
	//{
	//	LOG_TRACE(LOG_ERROR, "Failed, file body size zero");
	//	fclose(fp);
	//	return false;
	//}


	//// re-write index for body size and offset
	//fseek(fp, m_fileHeader.offIndex, SEEK_SET);
	//for (uint32_t idx = 0; idx < m_fileHeader.cntIndex; idx++)
	//{
	//	FileIndex fileIndex;
	//	memcpy(&fileIndex, &m_vtIndex[idx], sizeof(fileIndex));
	//	if ((retRead = fwrite(&fileIndex, sizeof(fileIndex), 1, fp)) != 1)
	//	{
	//		LOG_TRACE(LOG_ERROR, "Failed, can't re-write file index data, idx:%d", idx);
	//		fclose(fp);
	//		return false;
	//	}
	//}


	//fclose(fp);


	////LOG_TRACE(LOG_DEBUG, "Save data, total mesh cnt:%lld", cntTotalMesh);
	////LOG_TRACE(LOG_DEBUG, "Save data, total building cnt:%lld", cntTotalBld);



	//// release memory
	//LOG_TRACE(LOG_DEBUG, "LOG, Release data");
	//Release();

	//return true;
}


//size_t CFileBuilding::WriteHeader(FILE* fp, FileHeader* pHeader)
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


//size_t CFileBuilding::WriteIndex(FILE* fp, vector<FileIndex>* pvtFileIndex)
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


size_t CFileBuilding::WriteBody(FILE* fp, IN const uint32_t fileOff)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	stMeshInfo* pMesh = nullptr;
	stPolygonInfo* pBld = nullptr;

	FileBody fileBody;
	FilePolygon fileBld = { 0, };

	size_t offFile = fileOff;
	size_t retWrite = 0;
	size_t retRead = 0;
	long offItem = 0;
	const size_t sizeFileBody = sizeof(fileBody);

	uint32_t ii, jj, kk;

	// write body - building, vertex
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

		offItem = sizeFileBody;


		// write building
# if defined(USE_OPTIMAL_POINT_API)
		for (jj = 0; jj < pMesh->buildings.size(); jj++)
		{
			pBld = m_pDataMgr->GetBuildingDataById(pMesh->buildings[jj]);
			if (!pBld)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't access building, idx:%d, tile_id:%d, id:%d", ii, pMesh->buildings[ii].tile_id, pMesh->buildings[ii].nid);
				return 0;
			}

			fileBld.polygon_id = pBld->poly_id;
			memcpy(&fileBld.rtBox, &pBld->data_box, sizeof(fileBld.rtBox));
			//fileBld.parts = pBld->vtPts.size();
			fileBld.parts = pBld->getAttributeCount(TYPE_POLYGON_DATA_ATTR_PART);
			//fileBld.points = pBld->vtVtx.size();
			fileBld.points = pBld->getAttributeCount(TYPE_POLYGON_DATA_ATTR_VTX);
			fileBld.sub_info = pBld->sub_info;

			if ((retWrite = fwrite(&fileBld, sizeof(fileBld), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't write building[%d], tile_id:%d, id:%d", jj, fileBld.polygon_id.tile_id, fileBld.polygon_id.nid);
				return 0;
			}

			fileBody.area.cntPolygon++;
			offItem += sizeof(fileBld);


			// write vertex parts
			//for (kk = 0; kk < fileBld.parts; kk++)
			//{
			//	if ((retWrite = fwrite(&pBld->vtPts[kk], sizeof(uint32_t), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't write building[%d] parts[%d], ", jj, kk);
			//		return 0;
			//	}
			//	offItem += sizeof(uint32_t);
			//}
			if (fileBld.parts > 0)
			{
				if (!(retWrite = pBld->writeAttribute(TYPE_POLYGON_DATA_ATTR_PART, fp))) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write building[%d], parts", jj);
					return 0;
				}
				offItem += retWrite;
			}

			// write vertex
			//for (kk = 0; kk < fileBld.points; kk++)
			//{
			//	if ((retWrite = fwrite(&pBld->vtVtx[kk], sizeof(SPoint), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't write building[%d] vertex[%d], ", jj, kk);
			//		return 0;
			//	}
			//	offItem += sizeof(SPoint);
			//}
			if (fileBld.points > 0)
			{
				if (!(retWrite = pBld->writeAttribute(TYPE_POLYGON_DATA_ATTR_VTX, fp))) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write building[%d], vertex", jj);
					return 0;
				}
				offItem += retWrite;
			}

		} // write link

		LOG_TRACE(LOG_DEBUG, "Save data, building, cnt:%lld", jj);
#endif // # if defined(USE_OPTIMAL_POINT_API)


		// re-write body size & off
		if (offItem > sizeFileBody)
		{
			fileBody.szData = offItem;

			fseek(fp, offItem * -1, SEEK_CUR);

			if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't re-write building[%d], tile_id:%d", ii, fileBody.idTile);
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


bool CFileBuilding::LoadData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_BUILDING], g_szTypeExec[TYPE_DATA_BUILDING]);

	return CFileBase::LoadData(szFileName);


	// load name dic
	size_t offFile = 0;
	size_t offItem = 0;
	size_t retRead = 0;

	/////////////////////////////////////////
	// name dic
	//LoadNameData(szFilePath);


	// load data
	FILE* fp = fopen(szFilePath, "rb");
	if (!fp)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't open file for load data, file:%s", szFilePath);
		return false;
	}

	strcpy(m_szDataPath, szFilePath);

	FileHeader fileHeader = { 0, };
	FileIndex fileIndex = { 0, };
	FileBody fileBody = { 0, };
	FilePolygon fileBld = { 0, };

	// read header
	if ((retRead = fread(&fileHeader, sizeof(fileHeader), 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't read header, cnt:%d", retRead);
		fclose(fp);
		return false;
	}

	memcpy(&m_rtBox, &fileHeader.rtMap, sizeof(m_rtBox));

	// stMeshInfo* pMesh;
	// stPolygonInfo* pBld;

	// read index
	for (uint32_t idxTile = 0; idxTile < fileHeader.cntIndex; idxTile++)
	{
		fseek(fp, fileHeader.offIndex + sizeof(fileIndex) * idxTile, SEEK_SET);
		if ((retRead = fread(&fileIndex, sizeof(fileIndex), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read index, idx:%d", idxTile);
			fclose(fp);
			return false;
		}

		m_vtIndex.push_back(fileIndex);



		continue; // for using cache



		//pMesh = new stMeshInfo;
		////memset(pMesh, 0x00, sizeof(stMeshInfo));




		//// read body
		//if (stIndex.offBody <= 0 || stIndex.szBody <= 0)
		//{
		//	LOG_TRACE(LOG_ERROR, "Failed, index body info invalid, off:%d, size:%d", stIndex.offBody, stIndex.szBody);
		//	fclose(fp);
		//	return false;
		//}

		//fseek(fp, stIndex.offBody, SEEK_SET);
		//if ((retRead = fread(&stBody, sizeof(stBody), 1, fp)) != 1)
		//{
		//	LOG_TRACE(LOG_ERROR, "Failed, can't read body, offset:%d", stIndex.offBody);
		//	fclose(fp);
		//	return false;
		//}

		//if (stIndex.idTile != stBody.idTile)
		//{
		//	LOG_TRACE(LOG_ERROR, "Failed, index tile info not match with body, index tile id:%d vs body tile id:%d", stIndex.idTile, stBody.idTile);
		//	fclose(fp);
		//	return false;
		//}


		//// mesh
		//pMesh->mesh_id.tile_id = stIndex.idTile;
		//memcpy(&pMesh->mesh_box, &stIndex.rtTile, sizeof(pMesh->mesh_box));
		//memcpy(&pMesh->data_box, &stIndex.rtData, sizeof(pMesh->data_box));
		//for (int ii = 0; ii < 8; ii++)
		//{
		//	if (stBody.idNeighborTile[ii] <= 0)
		//		break;
		//	pMesh->vtNeighbor.push_back(stBody.idNeighborTile[ii]);
		//}
		//AddMeshData(pMesh);


		//// building
		//for (int ii = 0; ii < stBody.bld.cntBld; ii++)
		//{
		//	// read link
		//	if ((retRead = fread(&stBld, sizeof(stBld), 1, fp)) != 1)
		//	{
		//		LOG_TRACE(LOG_ERROR, "Failed, can't read bulding, mesh_id:%d, idx:%d", stIndex.idTile, ii);
		//		fclose(fp);
		//		return false;
		//	}

		//	pMesh->buildings.push_back(stBld.poly_id);

		//	pBld = new stPolygonInfo;
		//	
		//	pBld->poly_id = stBld.poly_id;
		//	memcpy(&pBld->data_box, &stBld.rtBox, sizeof(stBld.rtBox));
		//	pBld->type = stBld.type;
		//	pBld->height = stBld.height;
		//	pBld->name_type = stBld.name_type;
		//	pBld->name_val = stBld.name_val;
		//	
		//	// read parts
		//	uint32_t part;
		//	for (int jj = 0; jj < stBld.parts; jj++)
		//	{				
		//		if ((retRead = fread(&part, sizeof(part), 1, fp)) != 1)
		//		{
		//			LOG_TRACE(LOG_ERROR, "Failed, can't read parts, mesh_id:%d, buildig idx:%d, idx:%d", stIndex.idTile, ii, jj);
		//			fclose(fp);
		//			return false;
		//		}
		//		pBld->vtPts.push_back(part);
		//	}

		//	// read vertext
		//	SPoint point;
		//	for (int jj = 0; jj < stBld.points; jj++)
		//	{
		//		if ((retRead = fread(&point, sizeof(point), 1, fp)) != 1)
		//		{
		//			LOG_TRACE(LOG_ERROR, "Failed, can't read point, mesh_id:%d, buildig idx:%d, idx:%d", stIndex.idTile, ii, jj);
		//			fclose(fp);
		//			return false;
		//		}
		//		pBld->vtVtx.push_back(point);
		//	}
		//	AddBuildingData(pBld);
		//}
		//LOG_TRACE(LOG_DEBUG, "Data loading, building data loaded, mesh:%d, link cnt:%lld", pMesh->mesh_id.tile_id, stBody.bld.cntBld);
	}

	fclose(fp);

	return true;
}


size_t CFileBuilding::ReadBody(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody = { 0, };
	FilePolygon fileBld = { 0, };

	stMeshInfo* pMesh;
	stPolygonInfo* pBld;

	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeFileBody = sizeof(FileBody);

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
		if ((retRead = fread(&fileBody, sizeFileBody, 1, fp)) != 1)
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


		// building
		for (uint32_t ii = 0; ii < fileBody.area.cntPolygon; ii++)
		{
			// read building
			if ((retRead = fread(&fileBld, sizeof(fileBld), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read building, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			//pMesh->buildings.push_back(fileBld.polygon_id);

			pBld = new stPolygonInfo;

			pBld->poly_id = fileBld.polygon_id;
			memcpy(&pBld->data_box, &fileBld.rtBox, sizeof(fileBld.rtBox));
			pBld->sub_info = fileBld.sub_info;

			// read parts
			//uint32_t part;
			//for (uint32_t jj = 0; jj < fileBld.parts; jj++)
			//{
			//	if ((retRead = fread(&part, sizeof(part), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't read parts, mesh_id:%d, buildig idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
			//		return 0;
			//	}
			//	pBld->vtPts.push_back(part);
			//}
			if (fileBld.parts > 0 && !pBld->readAttribute(TYPE_POLYGON_DATA_ATTR_PART, fp, fileBld.parts))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read parts, mesh_id:%d, buildig idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			// read vertext
			//SPoint point;
			//for (uint32_t jj = 0; jj < fileBld.points; jj++)
			//{
			//	if ((retRead = fread(&point, sizeof(point), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't read point, mesh_id:%d, buildig idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
			//		return 0;
			//	}
			//	pBld->vtVtx.push_back(point);
			//}
			if (fileBld.points > 0 && !pBld->readAttribute(TYPE_POLYGON_DATA_ATTR_VTX, fp, fileBld.points))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read points, mesh_id:%d, buildig idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			AddPolygonData(pBld);
		}

		offFile += m_vtIndex[idx].szBody;

#if defined(_DEBUG)
		LOG_TRACE(LOG_DEBUG, "Data loading, building data loaded, mesh:%d, cnt:%lld", pMesh->mesh_id.tile_id, fileBody.area.cntPolygon);
#endif
	}

	return offFile;
}


bool CFileBuilding::LoadDataByIdx(IN const uint32_t idx)
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
	FilePolygon fileBld = { 0, };

	stMeshInfo* pMesh;
	stPolygonInfo* pBld;

	size_t offItem = 0;
	size_t offFile = 0;
	size_t retRead = 0;
	size_t sizeFileBody = sizeof(FileBody);

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
	if ((retRead = fread(&fileBody, sizeFileBody, 1, fp)) != 1)
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


	// building
	for (uint32_t ii = 0; ii < fileBody.area.cntPolygon; ii++)
	{
		// read building
		if ((retRead = fread(&fileBld, sizeof(fileBld), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read building, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return false;
		}

		//pMesh->buildings.push_back(fileBld.polygon_id);

		pBld = new stPolygonInfo;

		pBld->poly_id = fileBld.polygon_id;
		memcpy(&pBld->data_box, &fileBld.rtBox, sizeof(fileBld.rtBox));
		pBld->sub_info = fileBld.sub_info;

		// read parts
		//uint32_t part;
		//for (uint32_t jj = 0; jj < fileBld.parts; jj++)
		//{
		//	if ((retRead = fread(&part, sizeof(part), 1, fp)) != 1)
		//	{
		//		LOG_TRACE(LOG_ERROR, "Failed, can't read parts, mesh_id:%d, buildig idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
		//		fclose(fp);
		//		return false;
		//	}
		//	pBld->vtPts.push_back(part);
		//}
		if (fileBld.parts > 0 && !pBld->readAttribute(TYPE_POLYGON_DATA_ATTR_PART, fp, fileBld.parts))
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read parts, mesh_id:%d, buildig idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return 0;
		}

		// read vertext
		//SPoint point;
		//for (uint32_t jj = 0; jj < fileBld.points; jj++)
		//{
		//	if ((retRead = fread(&point, sizeof(point), 1, fp)) != 1)
		//	{
		//		LOG_TRACE(LOG_ERROR, "Failed, can't read point, mesh_id:%d, buildig idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
		//		fclose(fp);
		//		return false;
		//	}
		//	pBld->vtVtx.push_back(point);
		//}
		if (fileBld.points > 0 && !pBld->readAttribute(TYPE_POLYGON_DATA_ATTR_VTX, fp, fileBld.points))
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read points, mesh_id:%d, buildig idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return 0;
		}

		AddPolygonData(pBld);
	}

	fclose(fp);

	return true;
}


bool CFileBuilding::Initialize()
{
	return CFileBase::Initialize();
}

void CFileBuilding::Release()
{
	m_nBldIdx = 0;

	if (!m_mapBuilding.empty()) {
		m_mapBuilding.clear();
		unordered_map<uint64_t, stBuilding>().swap(m_mapBuilding);
	}

	if (!m_mapStringId.empty()) {
		m_mapStringId.clear();
		unordered_map<string, KeyID>().swap(m_mapStringId);
	}	

	CFileBase::Release();
}


uint32_t CFileBuilding::getBuildingCode(IN const char* code)
{
	uint32_t nCode = TYPE_BUILDING_NOT;

	if (code && strlen(code) > 0 && strlen(code) < 50)
	{
		if (strcmp(code, "APT") == 0) {
			nCode = TYPE_BUILDING_APT; //아파트
		} else if (strcmp(code, "SCH") == 0) {
			nCode = TYPE_BUILDING_SCH; //학교
		} else if (strcmp(code, "OFT") == 0) {
			nCode = TYPE_BUILDING_OFT; //오피스텔
		}
		else if (strcmp(code, "B01") == 0) {
			nCode = TYPE_BUILDING_B01; //공공기관
		}
		else if (strcmp(code, "B02") == 0) {
			nCode = TYPE_BUILDING_B02; //치안기관
		}
		else if (strcmp(code, "B03") == 0) {
			nCode = TYPE_BUILDING_B03; //공원
		}
		else if (strcmp(code, "B04") == 0) {
			nCode = TYPE_BUILDING_B04; //교육기관
		}
		else if (strcmp(code, "B05") == 0) {
			nCode = TYPE_BUILDING_B05; //언론기관
		}
		else if (strcmp(code, "B06") == 0) {
			nCode = TYPE_BUILDING_B06; //금융기관
		}
		else if (strcmp(code, "B07") == 0) {
			nCode = TYPE_BUILDING_B07; //문화
		}
		else if (strcmp(code, "B08") == 0) {
			nCode = TYPE_BUILDING_B08; //관광
		}
		else if (strcmp(code, "B09") == 0) {
			nCode = TYPE_BUILDING_B09; //레져
		}
		else if (strcmp(code, "B10") == 0) {
			nCode = TYPE_BUILDING_B10; //음식점
		}
		else if (strcmp(code, "B11") == 0) {
			nCode = TYPE_BUILDING_B11; //편의
		}
		else if (strcmp(code, "B12") == 0) {
			nCode = TYPE_BUILDING_B12; //복지
		}
		else if (strcmp(code, "B13") == 0) {
			nCode = TYPE_BUILDING_B13; //기업
		}
		else if (strcmp(code, "B14") == 0) {
			nCode = TYPE_BUILDING_B14; //농공시설
		}
		else if (strcmp(code, "B15") == 0) {
			nCode = TYPE_BUILDING_B15; //자동차관련
		}
		else if (strcmp(code, "B16") == 0) {
			nCode = TYPE_BUILDING_B16; //교통시설
		}
		else if (strcmp(code, "B17") == 0) {
			nCode = TYPE_BUILDING_B17; //지하철시설
		}
		else if (strcmp(code, "B18") == 0) {
			nCode = TYPE_BUILDING_B18; //도로시설
		}
		else if (strcmp(code, "B19") == 0) {
			nCode = TYPE_BUILDING_B19; //주택관련
		}
		else if (strcmp(code, "ETC") == 0) {
			nCode = TYPE_BUILDING_ETC; //주택외건물
		}
		else if (strcmp(code, "E2D") == 0) {
			nCode = TYPE_BUILDING_E2D; //주택외건물2D
		}
		else if (strcmp(code, "IND") == 0) {
			nCode = TYPE_BUILDING_IND; //공업단지내건물
		}
		else if (strcmp(code, "STA") == 0) {
			nCode = TYPE_BUILDING_STA; //기차역
		}
		else if (strcmp(code, "APC") == 0) {
			nCode = TYPE_BUILDING_APC; //주상복합
		}
		else if (strcmp(code, "ROH") == 0) {
			nCode = TYPE_BUILDING_ROH; //연립주택
		}
		else if (strcmp(code, "APL") == 0) {
			nCode = TYPE_BUILDING_APL; //아파트형공장
		}
		else if (strcmp(code, "APS") == 0) {
			nCode = TYPE_BUILDING_APS; //아파트 상가
		}
		else {
			nCode = TYPE_BUILDING_NOT;
		}			
	}
	
	return nCode;
}

uint32_t CFileBuilding::getNameType(IN char* name, OUT uint32_t & type)
{
	uint32_t nVal = 0;
	uint32_t nType = TYPE_BLD_NAME_NOT;
	size_t nLen = strlen(name);

	if (nLen > 0) {
		if (nLen == 1) {
			if ((48 <= name[0] && name[0] <= 57)) { // 0-9
				nType = TYPE_BLD_NAME_INT;
			}
			else if ((65 <= name[0] && name[0] <= 90) || // A-Za-z
				(97 <= name[0] && name[0] <= 122)) {
				nType = TYPE_BLD_NAME_ENG;
			}
		}
		else {
			for (int ii = 0; ii < nLen; ii++) {
				if ((48 <= name[ii] && name[ii] <= 57)) { // 0-9
					nType = TYPE_BLD_NAME_INT;
				}
			}
		}

		if (nType == TYPE_BLD_NAME_INT) {
			nVal = atoi(name);
		}
		else if (nType == TYPE_BLD_NAME_ENG) {
			nVal = name[0];
		}
		else {
			nType = TYPE_BLD_NAME_DIC;
			nVal = AddNameDic(name);
		}
	}

	type = nType;
	return nVal;
}

bool  CFileBuilding::SetData_Building(int idx, stBuilding &getBuilding_Dbf, char* colData)
{
	bool ret = true;

	switch (idx)
	{
	case 1:		getBuilding_Dbf.Code = getBuildingCode(trim(colData));		break;
	case 2: {
		uint32_t numType = 0;
		//getBuilding_Dbf.Num = getNameType(trim(colData), getBuilding_Dbf.NumType); break;
		getBuilding_Dbf.Num = getNameType(trim(colData), numType);
		getBuilding_Dbf.NumType = numType;
	} break;
	case 3:		getBuilding_Dbf.Height = atoi(trim(colData));	break;
	case 4:		getBuilding_Dbf.BldId = trim(colData); break;
	case 5:		getBuilding_Dbf.keyBld.tile_id = atoi(trim(colData)); break; // mesh id
	//case 11:	getBuilding_Dbf.Name = AddNameDic(trim(colData));
	default:	break;
	}

	return ret;
}

//bool CFileBuilding::AddMeshDataByBuilding(IN const stMeshInfo * pInfo, IN const stPolygonInfo * pData)
//{
//	if (AddMeshData(pInfo)) {
//		if (m_pDataMgr) {
//			return m_pDataMgr->AddMeshDataByBuilding(pInfo, pData);
//		}
//	}
//
//	return false;
//}

void CFileBuilding::SetFileComplex(CFileComplex* pCpx)
{
	m_pFileCpx = pCpx;
}


bool CFileBuilding::AddEntranceData(IN const KeyID keyId, IN const stEntranceInfo& entInfo)
{
	bool ret = false;

	if (keyId.llid != NULL_VALUE) {
		unordered_map<uint64_t, stBuilding>::iterator it = m_mapBuilding.find(keyId.llid);
		if (it != m_mapBuilding.end()) {
			it->second.vtEntrance.emplace_back(entInfo);
			ret = true;
		}
	}

	return ret;
}


const KeyID CFileBuilding::AddStringId(IN const char* szStringId, IN const uint32_t meshId)
{
	KeyID retKey = { NULL_VALUE, };

	if (szStringId && ((retKey = GetIdFromStringId(szStringId)).llid == NULL_VALUE)) {
		string strId(szStringId);
		retKey.ent.nid = m_nBldIdx++;
		retKey.ent.tile_id = meshId;
		retKey.ent.type = 0; // building

		m_mapStringId.emplace(strId, retKey);
	}

	return retKey;
}


const KeyID CFileBuilding::GetIdFromStringId(IN const char* szStringId)
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