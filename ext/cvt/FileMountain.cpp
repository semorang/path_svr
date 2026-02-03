#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FileMountain.h"

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

#define atoll(s) _atoi64(s)
#endif

#define MAX_ENT_VALUE	67108863 // 26bit

using namespace std;

CFileMountain::CFileMountain()
{
	m_nDataType = TYPE_DATA_MOUNTAIN;
	m_nFileType = TYPE_EXEC_POLYGON;
	m_nMntIdx = 0;
}


CFileMountain::~CFileMountain()
{
	if (!m_mapBoundary.empty()) {
		m_mapBoundary.clear();
		unordered_map<uint64_t, stMntBoundary>().swap(m_mapBoundary);
	}
}

bool CFileMountain::ParseData(IN const char* fname)
{
	bool ret = false;

	XSHPReader shpReader;

	if (!shpReader.Open(fname)) {
		LOG_TRACE(LOG_ERROR, "Error,Cannot Find shp File!");
		return ret;
	}

	int nShpType = (int)shpReader.GetShpObjectType(); //shpPoint=1,shpPolyLine=3,shpPolygon=5

	/*
	NULL 0
	POINT 1
	ARC 3
	POLYGON 5
	MULTIPOINT 8
	POINTZ 11
	ARCZ 13
	POLYGONZ 15
	MULTIPOINTZ 18
	POINTM 21
	ARCM 23
	POLYGONM 25
	MULTIPOINTM 28
	MULTIPATCH 31
	*/

	if (nShpType == 5 || nShpType == 15) {
		nShpType = 3; //면
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
	const int32_t cntUserBndField = 7;
	const int32_t cntUserEntField = 8;
	if (strstr(fname, "BND") != nullptr) {
		static char szBoundayrField[cntUserBndField][32] = {
			{ "F_NAME" },{ "국립,도" },{ "CODE" },{ "배지 발" },{ "산" },{ "GIRD" },{ "인기도" }, // GIRD가 맞는건가??
			/*{ "F_NAME" },{ "국립,도" },{ "CODE" },{ "배지 발" },{ "산" }, { "GRID" }, {"인기도"},*/
		};

		// 캐릭터 변환
		//char szBoundayrFieldConv[32] = { 0, };
		//for (auto & name : szBoundayrField) {
		//	memset(szBoundayrFieldConv, 0x00, sizeof(szBoundayrFieldConv));
		//	MultiByteToUTF8(name, szBoundayrFieldConv);
		//	memcpy(name, szBoundayrFieldConv, sizeof(szBoundayrFieldConv));
		//}
		
		DBF_FIELD_INFO fieldInfo;
		
		for (int ii = 0; ii < cntUserBndField; ii++) {
			if (shpReader.GetFieldInfo(ii, fieldInfo) == false) {
				LOG_TRACE(LOG_ERROR, "can't get dbf filed info, idx : %d ", ii);
				return false;
			}
			else if (strcmp(szBoundayrField[ii], strupper(trim(fieldInfo.szName))) != 0) {
				LOG_TRACE(LOG_ERROR, "dbf field name not matched the expected name, filedName:%s vs exprectedName:%s", fieldInfo.szName, szBoundayrField[ii]);
				//return false;
			}
		}
	} else if (strstr(fname, "ENT") != nullptr) {
		static char szEntranceField[cntUserEntField][32] = {
			{ "ENT_ID" },{ "F_NODE_ID" },{ "W_NODE_ID" },{ "ENT_TYP" },{ "ENT_NAME" },{ "MNT_CD" },{ "DP" },{ "G_ID" },
		};

		DBF_FIELD_INFO fieldInfo;

		for (int ii = 0; ii < cntUserEntField; ii++) {
			if (shpReader.GetFieldInfo(ii, fieldInfo) == false) {
				LOG_TRACE(LOG_ERROR, "can't get ent filed info, idx : %d ", ii);
				return false;
			}
			else if (strcmp(szEntranceField[ii], strupper(trim(fieldInfo.szName))) != 0) {
				LOG_TRACE(LOG_ERROR, "dbf field name not matched the expected name, filedName:%s vs exprectedName:%s", fieldInfo.szName, szEntranceField[ii]);
				return false;
			}
		}
	}

	// 분할된 파일 대비
	if (!m_mapBoundary.empty()) {
		m_mapBoundary.reserve(nRecCount + m_mapBoundary.size());
	}
	else {
		m_mapBoundary.reserve(nRecCount + 1);
	}


	//데이터 얻기
	LOG_TRACE(LOG_DEBUG, "LOG, start, raw data parsing file : %s", fname);

	
	SHPGeometry* pSHPObj = nullptr;
	string strKey;

	for (int idxRec = 0; idxRec < nRecCount; idxRec++) {
		ret = false;

		//속성가져오기
		if (shpReader.Fetch(idxRec) == false) { //error to read..
			LOG_TRACE(LOG_ERROR, "ERROR,Record %d is not available!!", idxRec);
			continue;
		}

		pSHPObj = shpReader.GetGeometry();

		//Setting DBF Data
		if (nShpType == 3) { // 면 (산 바운더리)
			stMntBoundary boundary;
			memcpy(&boundary.Box, &pSHPObj->polygon.Box, sizeof(SBox));

			for (unsigned int idxCol = 0; idxCol < colCnt; idxCol++) {
				memset(chsTmp, 0x00, sizeof(chsTmp));
				shpReader.GetDataAsString(idxCol, chsTmp, 60);  //nLabelIdxToRead 번째 필드 값을 chsTmp로 12byte 읽어옴.

				if (!SetData_Boundary(idxCol, boundary, chsTmp))
				{
					LOG_TRACE(LOG_ERROR, "Error, %s", GetErrorMsg());
					break;
				}

				if (idxCol == 0)
				{
					pPoints = SHP_GetPartPoints(pSHPObj, 0); //파트 0에 대한	     //실제선형데이터..

					boundary.vtParts.assign(pSHPObj->polygon.Parts, pSHPObj->polygon.Parts + pSHPObj->polygon.NumParts);
					boundary.vtVertex.assign(pPoints, pPoints + pSHPObj->polygon.NumPoints);
				}

				ret = true;
			} // for

			boundary.keyMnt.poly.tile_id = g_forestMeshId; // 단일 메쉬
			boundary.keyMnt.poly.nid = boundary.Code;
			boundary.keyMnt.poly.type = TYPE_POLYGON_MOUNTAIN;  // 폴리곤 타입, 0:미지정, 1:빌딩, 2:단지, 3:산바운더리

			if (m_mapBoundary.find(boundary.keyMnt.llid) != m_mapBoundary.end()) {
				// 중복
				LOG_TRACE(LOG_WARNING, "mountain id duplicated, code:'%d', mesh:%d, id:%d", boundary.Code, boundary.keyMnt.tile_id, boundary.keyMnt.nid);
				continue;
			}

			m_mapBoundary.emplace(boundary.keyMnt.llid, boundary);
		} else if (nShpType == 1) { // 점 (입구점)
			stMntEntrance entrance;

			for (unsigned int idxCol = 0; idxCol < colCnt; idxCol++) {
				memset(chsTmp, 0x00, sizeof(chsTmp));
				shpReader.GetDataAsString(idxCol, chsTmp, 60);  //nLabelIdxToRead 번째 필드 값을 chsTmp로 12byte 읽어옴.

				if (!SetData_Entrance(idxCol, entrance, chsTmp))
				{
					LOG_TRACE(LOG_ERROR, "Error, %s", GetErrorMsg());
					break;
				}

				if (idxCol == 0)
				{
					entrance.x = pSHPObj->point.point.x;
					entrance.y = pSHPObj->point.point.y;
				}

				ret = true;
			} // for

			if (entrance.ENT_TYP != 1 && entrance.ENT_TYP != 2) {
				// 현재(2024-06-24) 등산로 대응되는 입구점만 사용
				continue;
			} else if (entrance.MNT_CD == 0 && entrance.G_ID == 0) {
				LOG_TRACE(LOG_WARNING, "Should Check Data, entrance doesn't contain g_id & mnt_cd information, id:%d", MAX_ENT_VALUE, entrance.ENT_ID);
				continue;
			}

			entrance.keyMnt.ent.tile_id = g_forestMeshId; // 단일 메쉬
			entrance.keyMnt.ent.nid = entrance.ENT_ID;
			entrance.keyMnt.ent.type = TYPE_ENT_MOUNTAIN;
			if (m_mapEntrance.find(entrance.keyMnt.llid) != m_mapEntrance.end()) {
				// 중복
				LOG_TRACE(LOG_WARNING, "Mountain Entrance id duplicated, code:'%d', mesh:%d, id:%d", entrance.ENT_ID, entrance.keyMnt.tile_id, entrance.keyMnt.nid);
				continue;
			}

			// g_id에 포함되는 mnt_cd 등록
			if (entrance.G_ID > 0 && entrance.MNT_CD > 0) {
				unordered_map<uint32_t, unordered_set<uint32_t>>::iterator itMnt = m_mapMntGidMcd.find(entrance.G_ID);
				if (itMnt != m_mapMntGidMcd.end()) {
					// add
					itMnt->second.emplace(entrance.MNT_CD); // set으로 중복 제거
				} else {
					// create
					m_mapMntGidMcd.emplace(entrance.G_ID, unordered_set<uint32_t>{entrance.MNT_CD});
				}
			}

			m_mapEntrance.emplace(entrance.keyMnt.llid, entrance);
		}
		else {
			LOG_TRACE(LOG_ERROR, "Unknown type, type:%s", nShpType);
			break;
		}

		if (ret == false) {
			break;
		}
	} // for

	shpReader.Close();

	if (!m_mapEntrance.empty() && !m_mapBoundary.empty()) {
		ret = GenServiceData();
	}

	return ret;
}


bool CFileMountain::GenServiceData()
{
	stPolygonInfo* pPoly = nullptr;
	int cntLinks = 0;

	// 클래스에 추가
	LOG_TRACE(LOG_DEBUG, "LOG, start, add to mountain class");


	// 산코드가 없는 전체 메쉬 바운더리 생성
	stMntBoundary nullBnd;
	memcpy(&nullBnd.Box, GetMeshRegion(), sizeof(nullBnd.Box));
	nullBnd.keyMnt.tile_id = g_forestMeshId;
	nullBnd.keyMnt.nid = 0;
	nullBnd.keyMnt.poly.type = TYPE_POLYGON_NONE;
	nullBnd.vtParts.emplace_back(1);
	nullBnd.vtVertex.emplace_back(SPoint{ nullBnd.Box.Xmin, nullBnd.Box.Ymin });
	nullBnd.vtVertex.emplace_back(SPoint{ nullBnd.Box.Xmax, nullBnd.Box.Ymin });
	nullBnd.vtVertex.emplace_back(SPoint{ nullBnd.Box.Xmax, nullBnd.Box.Ymax });
	nullBnd.vtVertex.emplace_back(SPoint{ nullBnd.Box.Xmin, nullBnd.Box.Ymax });
	m_mapBoundary.emplace(nullBnd.keyMnt.llid, nullBnd);

	KeyID findMnt;
	KeyID findNode;
	stNodeInfo* pNode = nullptr;
	findNode.llid = findMnt.llid = 0;
	
	findNode.tile_id = findMnt.tile_id = g_forestMeshId; // 단일 메쉬


	for (unordered_map<uint64_t, stMntEntrance>::const_iterator itEnt = m_mapEntrance.begin(); itEnt != m_mapEntrance.end(); itEnt++) {
		// g_id에 속하는 모든 mnt_cd에 입구점 등록
		vector<KeyID> vtGidMnt;
		if (itEnt->second.G_ID != 0) {
			unordered_map<uint32_t, unordered_set<uint32_t>>::iterator itMnt = m_mapMntGidMcd.find(itEnt->second.G_ID);
			if (itMnt != m_mapMntGidMcd.end()) {
				for (const auto& mnt : itMnt->second) {
					findMnt.poly.nid = mnt;
					findMnt.poly.type = TYPE_POLYGON_MOUNTAIN; // 폴리곤 타입, 0:미지정, 1:빌딩, 2:단지, 3:산바운더리
					vtGidMnt.emplace_back(findMnt);
				}
			}
		} else if (itEnt->second.MNT_CD != 0) {
			findMnt.poly.nid = itEnt->second.MNT_CD;
			findMnt.poly.type = TYPE_POLYGON_MOUNTAIN; // 폴리곤 타입, 0:미지정, 1:빌딩, 2:단지, 3:산바운더리
			vtGidMnt.emplace_back(findMnt);
		} else {
			// 여기에 떨어지면 데이터 검토 필요
			LOG_TRACE(LOG_WARNING, "Should Check Data, entrance doesn't contain g_id information", MAX_ENT_VALUE, itEnt->second.ENT_ID);
			continue;
		}
		
		// 매칭되는 산 바운더리가 없으면, 전체 메쉬에 추가
		if (vtGidMnt.empty()) {
			findMnt.poly.nid = 0;
			findMnt.poly.type = TYPE_POLYGON_NONE; // 폴리곤 타입, 0:미지정, 1:빌딩, 2:단지, 3:산바운더리
			vtGidMnt.emplace_back(findMnt);
		}


		// g_id에 속하는 모든 mnt_cd에 입구점 등록
		for (const auto& mnt : vtGidMnt) {
			// find mountain boundary data
			unordered_map<uint64_t, stMntBoundary>::iterator itBnd = m_mapBoundary.find(mnt.llid);
			if (itBnd != m_mapBoundary.end()) {
				stEntranceInfo entInfo;
				entInfo.mnt.poly_type = findMnt.poly.type;  // 폴리곤 타입, 0:미지정, 1:빌딩, 2:단지, 3:산바운더리
				entInfo.mnt.ent_code = TYPE_OPTIMAL_ENTRANCE_MOUNTAIN; // 0:미지정,1:차량 입구점,2:택시 승하차 지점(건물),3:택시 승하차 지점(건물군),4:택배 차량 하차 거점,5:보행자 입구점,6:배달 하차점(차량, 오토바이),7:배달 하차점(자전거, 도보),8:숲길 입구점

				if (itEnt->second.ENT_ID >= MAX_ENT_VALUE) {
					LOG_TRACE(LOG_WARNING, "max entrance id value over than %d, now:%d", MAX_ENT_VALUE, itEnt->second.ENT_ID);
					entInfo.mnt.ent_id = MAX_ENT_VALUE;
				} else {
					entInfo.mnt.ent_id = itEnt->second.ENT_ID;
				}

				//entInfo.mnt.mnt_code = itEnt->second.MNT_CD; // 산코드
				entInfo.mnt.fnode_id = itEnt->second.F_NODE_ID; // 숲길 노드 ID
				entInfo.mnt.wnode_id = itEnt->second.W_NODE_ID; // 보행자 노드 ID
				entInfo.x = itEnt->second.x;
				entInfo.y = itEnt->second.y;
				//entInfo.mnt.name_idx = 0; // 차후 한글 코드 변환 후 적용 하길 바람, 23.12.04

				itBnd->second.vtEntrance.emplace_back(entInfo);
			} else {
				LOG_TRACE(LOG_WARNING, "error, entrance matching polygon not exist, ent_id, ent_mnt:%d, type:%d", MAX_ENT_VALUE, itEnt->second.ENT_ID, itEnt->second.MNT_CD, itEnt->second.ENT_TYP);
			}
		}

		// 숲길 노드를 찾아 숲길 입구점 속성을 부여하고, 연결된 보행자 노드 ID를 endgenode_id에 적용
		findNode.nid = itEnt->second.F_NODE_ID;
		stNodeInfo* pNode = m_pDataMgr->GetFNodeDataById(findNode);
		if ((pNode != nullptr) && (pNode->base.point_type != TYPE_NODE_EDGE) && (pNode->edgenode_id.llid == 0)) {
			pNode->trk.entrance = 1;
			pNode->edgenode_id.llid = itEnt->second.W_NODE_ID;
		}
	} // for


	int cntProc = 0;
	int totProc = m_mapBoundary.size();
	int emptyEnt = 0;

	for (unordered_map<uint64_t, stMntBoundary>::const_iterator itMnt = m_mapBoundary.begin(); itMnt != m_mapBoundary.end(); itMnt++) {
		pPoly = new stPolygonInfo;

		pPoly->poly_id = itMnt->second.keyMnt;
		pPoly->sub_info = 0;
		pPoly->cpx.name_idx = itMnt->second.FNameIdx;
		memcpy(&pPoly->data_box, &itMnt->second.Box, sizeof(itMnt->second.Box));
		//pPoly->vtPts.assign(itMnt->second.vtParts.begin(), itMnt->second.vtParts.end());
		if (!itMnt->second.vtParts.empty()) {
			if (itMnt->second.vtParts.size() > 0xFFFF) {
				LOG_TRACE(LOG_ERROR, "Failed, add mountain data, mesh:%d, cpx:%d, parts size:%d", pPoly->poly_id.tile_id, pPoly->poly_id.nid, itMnt->second.vtParts.size());
				return 0;
			}
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_PART, &itMnt->second.vtParts.front(), itMnt->second.vtParts.size());
		}
		//pPoly->vtVtx.assign(itMnt->second.vtVertex.begin(), itMnt->second.vtVertex.end());
		if (!itMnt->second.vtVertex.empty()) {
			if (itMnt->second.vtVertex.size() > 0xFFFF) {
				LOG_TRACE(LOG_ERROR, "Failed, add mountain data, mesh:%d, cpx:%d, vertex size:%d", pPoly->poly_id.tile_id, pPoly->poly_id.nid, itMnt->second.vtVertex.size());
				return 0;
			}
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_VTX, &itMnt->second.vtVertex.front(), itMnt->second.vtVertex.size());
		}
		//pPoly->vtJoinedMesh.assign(itMnt->second.vtJoinedMesh.begin(), itMnt->second.vtJoinedMesh.end());
		if (!itMnt->second.vtJoinedMesh.empty()) {
			if (itMnt->second.vtJoinedMesh.size() > 0xFFFF) {
				LOG_TRACE(LOG_ERROR, "Failed, add mountain data, mesh:%d, cpx:%d, joined mesh size:%d", pPoly->poly_id.tile_id, pPoly->poly_id.nid, itMnt->second.vtJoinedMesh.size());
				return 0;
			}
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_MESH, &itMnt->second.vtJoinedMesh.front(), itMnt->second.vtJoinedMesh.size());
		}

		if (!itMnt->second.vtEntrance.empty()) {
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_ENT, &itMnt->second.vtEntrance.front(), itMnt->second.vtEntrance.size());
		}
		else {
			emptyEnt++;
		}

		if (!AddPolygonData(pPoly)) {
			LOG_TRACE(LOG_ERROR, "Failed, add mountain data, mesh:%d, cpx:%d", pPoly->poly_id.tile_id, pPoly->poly_id.nid);

			stMeshInfo* pMeshExt = new stMeshInfo;
			pMeshExt->mesh_id.tile_id = pPoly->poly_id.tile_id;

			if (AddMeshData(pMeshExt) && AddPolygonData(pPoly)) {
				LOG_TRACE(LOG_DEBUG, "add new mesh, when mesh info not exist, mesh:%d, cpx:%d", pMeshExt->mesh_id.tile_id, pPoly->poly_id.nid);
			}
		}

		if (++cntProc % g_cntLogPrint == 0) {
			LOG_TRACE(LOG_DEBUG, "LOG, processing, mountain: %d/%d", cntProc, totProc);
		}
	} // for

#if defined(USE_OPTIMAL_POINT_API) && defined(TEST_SPATIALINDEX)
	// 검색 트리 구성 필요
	m_pDataMgr->CreateSpatialindex(TYPE_DATA_COMPLEX);
#endif

	Release();
	if (!m_mapBoundary.empty()) {
		m_mapBoundary.clear();
		unordered_map<uint64_t, stMntBoundary>().swap(m_mapBoundary);
	}

	LOG_TRACE(LOG_DEBUG, "polyon dosen't have entrance info:%d, but %d polyong have emtrance info", emptyEnt, cntProc);

	LOG_TRACE(LOG_DEBUG, "LOG, finished");

	return true;
}


void CFileMountain::AddDataFeild(IN const int idx, IN const int type, IN const char* colData)
{

}


void CFileMountain::AddDataRecord()
{

}


size_t CFileMountain::ReadBody(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody = { 0, };
	FilePolygon fileMnt = { 0, };

	stMeshInfo* pMesh;
	stPolygonInfo* pMnt;

	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeBody = sizeof(FileBody);

	for (int idx = 0; idx < m_vtIndex.size(); idx++)
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


		// mountain
		for (uint32_t ii = 0; ii < fileBody.area.cntPolygon; ii++)
		{
			// read mountain
			if ((retRead = fread(&fileMnt, sizeof(fileMnt), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read mountain, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			pMnt = new stPolygonInfo;
			pMnt->poly_id = fileMnt.polygon_id;
			memcpy(&pMnt->data_box, &fileMnt.rtBox, sizeof(fileMnt.rtBox));
			pMnt->sub_info = fileMnt.sub_info;

			// read parts
			if (fileMnt.parts > 0 && !pMnt->readAttribute(TYPE_POLYGON_DATA_ATTR_PART, fp, fileMnt.parts))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read parts, mesh_id:%d, parts idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			// read vertext
			if (fileMnt.points > 0 && !pMnt->readAttribute(TYPE_POLYGON_DATA_ATTR_VTX, fp, fileMnt.points))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read points, mesh_id:%d, points idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			// read entrance
			if (fileMnt.links > 0 && !pMnt->readAttribute(TYPE_POLYGON_DATA_ATTR_ENT, fp, fileMnt.links))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read link, mesh_id:%d, links idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			// read joined mesh
			if (fileMnt.meshes > 0 && !pMnt->readAttribute(TYPE_POLYGON_DATA_ATTR_MESH, fp, fileMnt.meshes))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read joined mesh, mesh_id:%d, joins idx:%d", m_vtIndex[idx].idTile, ii);
				return 0;
			}

			AddPolygonData(pMnt);
		}

		offFile += m_vtIndex[idx].szBody;

#ifdef TEST_SPATIALINDEX
		m_pDataMgr->CreateSpatialindex(pMesh, TYPE_DATA_COMPLEX);
#endif

#if defined(_DEBUG)
		LOG_TRACE(LOG_DEBUG, "Data loading, mountain data loaded, mesh:%d, cnt:%lld", pMesh->mesh_id.tile_id, fileBody.area.cntPolygon);
#endif
	}

	return offFile;
}


bool CFileMountain::LoadDataByIdx(IN const uint32_t idx)
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
	FilePolygon fileMnt = { 0, };

	stMeshInfo* pMesh = nullptr;
	stPolygonInfo* pMnt = nullptr;
	
	size_t offFile = 0;
	size_t offItem = 0;
	size_t retRead = 0;

	// read index

	// read body
	if (m_vtIndex[idx].offBody <= 0 || m_vtIndex[idx].szBody <= 0)
	{
		//LOG_TRACE(LOG_ERROR, "Failed, index body info invalid, off:%d, size:%d", m_vtIndex[idx].offBody, m_vtIndex[idx].szBody);
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
	pMesh = GetMeshDataById(m_vtIndex[idx].idTile, false);
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


	// mountain
	for (uint32_t ii = 0; ii < fileBody.area.cntPolygon; ii++)
	{
		// read mountain
		if ((retRead = fread(&fileMnt, sizeof(fileMnt), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read mountain, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return false;
		}

		//pMesh->complexs.push_back(fileMnt.polygon_id);

		pMnt = new stPolygonInfo;

		pMnt->poly_id = fileMnt.polygon_id;
		memcpy(&pMnt->data_box, &fileMnt.rtBox, sizeof(fileMnt.rtBox));
		pMnt->sub_info = fileMnt.sub_info;

		// read parts
		if (fileMnt.parts > 0 && !pMnt->readAttribute(TYPE_POLYGON_DATA_ATTR_PART, fp, fileMnt.parts))
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read parts, mesh_id:%d, parts idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return 0;
		}

		// read vertext
		if (fileMnt.points > 0 && !pMnt->readAttribute(TYPE_POLYGON_DATA_ATTR_VTX, fp, fileMnt.points))
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read points, mesh_id:%d, points idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return 0;
		}

		// read entrance
		if (fileMnt.links > 0 && !pMnt->readAttribute(TYPE_POLYGON_DATA_ATTR_ENT, fp, fileMnt.links))
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read link, mesh_id:%d, links idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return 0;
		}

		// read joined mesh
		if (fileMnt.meshes > 0 && !pMnt->readAttribute(TYPE_POLYGON_DATA_ATTR_MESH, fp, fileMnt.meshes))
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read joined mesh, mesh_id:%d, joins idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return 0;
		}

		AddPolygonData(pMnt);
	}

	fclose(fp);

#ifdef TEST_SPATIALINDEX
	m_pDataMgr->CreateSpatialindex(pMesh, TYPE_DATA_COMPLEX);
#endif

	return true;
}


bool CFileMountain::Initialize()
{
	return CFileBase::Initialize();
}

void CFileMountain::Release()
{
	m_nMntIdx = 0;

	if (!m_mapBoundary.empty()) {
		m_mapBoundary.clear();
		unordered_map<uint64_t, stMntBoundary>().swap(m_mapBoundary);
	}

	if (!m_mapEntrance.empty()) {
		m_mapEntrance.clear();
		unordered_map<uint64_t, stMntEntrance>().swap(m_mapEntrance);
	}

	if (!m_mapStringId.empty()) {
		m_mapStringId.clear();
		unordered_map<string, KeyID>().swap(m_mapStringId);
	}

	CFileBase::Release();
}


size_t CFileMountain::WriteBody(FILE* fp, IN const uint32_t fileOff)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	stMeshInfo* pMesh = nullptr;
	stPolygonInfo* pMnt = nullptr;

	FileBody fileBody;
	FilePolygon fileMnt = { 0, };

	size_t offFile = fileOff;
	size_t retWrite = 0;
	size_t retRead = 0;
	long offItem = 0;
	const size_t sizeFileBody = sizeof(fileBody);

	uint32_t ii, jj;

	// write body - mountain, vertex
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


		// write mountain
#if defined(USE_FOREST_DATA)
#ifdef TEST_SPATIALINDEX
		for (const auto key : pMesh->setCpxDuplicateCheck)
#else
		for (const auto key : pMesh->complexs)
#endif
		{
			pMnt = m_pDataMgr->GetComplexDataById(key);
			if (!pMnt)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't access mountain, idx:%d, tile_id:%d, id:%d", ii, key.tile_id, key.nid);
				return 0;
			}
			else if (pMesh->mesh_id.tile_id != pMnt->poly_id.tile_id) {
				// 중첩된 메쉬 정보이므로 자신이 아니면 저장하지 않는다.
				continue;
			}

			fileMnt.polygon_id = pMnt->poly_id;
			memcpy(&fileMnt.rtBox, &pMnt->data_box, sizeof(fileMnt.rtBox));
			fileMnt.parts = pMnt->getAttributeCount(TYPE_POLYGON_DATA_ATTR_PART);
			fileMnt.points = pMnt->getAttributeCount(TYPE_POLYGON_DATA_ATTR_VTX);
			fileMnt.links = pMnt->getAttributeCount(TYPE_POLYGON_DATA_ATTR_LINK);
			//fileMnt.links = pMnt->getAttributeCount(TYPE_POLYGON_DATA_ATTR_LINK);
			fileMnt.links = pMnt->getAttributeCount(TYPE_POLYGON_DATA_ATTR_ENT); // 산바운더리에서는 link를 입구점으로 사용하자
			fileMnt.meshes = pMnt->getAttributeCount(TYPE_POLYGON_DATA_ATTR_MESH);
			fileMnt.sub_info = pMnt->sub_info;

			if ((retWrite = fwrite(&fileMnt, sizeof(fileMnt), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't write mountain[%d], tile_id:%d, id:%d", jj, fileMnt.polygon_id.tile_id, fileMnt.polygon_id.nid);
				return 0;
			}

			fileBody.area.cntPolygon++;
			offItem += sizeof(fileMnt);


			// write vertex parts
			if (fileMnt.parts > 0)
			{
				if (!(retWrite = pMnt->writeAttribute(TYPE_POLYGON_DATA_ATTR_PART, fp))) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write mountain[%d], parts", jj);
					return 0;
				}
				offItem += retWrite;
			}

			// write vertex
			if (fileMnt.points > 0)
			{
				if (!(retWrite = pMnt->writeAttribute(TYPE_POLYGON_DATA_ATTR_VTX, fp))) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write mountain[%d], vertex", jj);
					return 0;
				}
				offItem += retWrite;
			}

			// write entrance
			if (fileMnt.links > 0)
			{
				if (!(retWrite = pMnt->writeAttribute(TYPE_POLYGON_DATA_ATTR_ENT, fp))) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write mountain[%d], entrance", jj);
					return 0;
				}
				offItem += retWrite;
			}

			if (fileMnt.meshes > 0)
			{
				if (!(retWrite = pMnt->writeAttribute(TYPE_POLYGON_DATA_ATTR_MESH, fp))) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write mountain[%d] joined mesh", jj);
					return 0;
				}
				offItem += retWrite;
			}

		} // write mountain
#endif //#if defined(USE_FOREST_DATA)

		// 숲길은 단일 메쉬라 대표 메쉬 제회한 모든 메쉬는 데이터가 없음 
		if (jj != 0) {
			LOG_TRACE(LOG_DEBUG, "Save data, mountain, cnt:%lld", jj);
		}

		// re-write body size & off
		if (offItem > sizeFileBody)
		{
			fileBody.szData = offItem;

			fseek(fp, offItem * -1, SEEK_CUR);

			if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't re-write mountain[%d], tile_id:%d", ii, fileBody.idTile);
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


bool  CFileMountain::SetData_Boundary(int idx, stMntBoundary &getMnt_Dbf, char* colData)
{
	bool ret = true;

	switch (idx)
	{
	case 0:		getMnt_Dbf.FNameIdx = AddNameDic(colData);	break;
	case 1:		getMnt_Dbf.Category = trim(colData); break;
	case 2:		getMnt_Dbf.Code = atoi(trim(colData)); break;
	case 3:		getMnt_Dbf.BeajiBal = atoi(trim(colData)); break;
	case 4:		getMnt_Dbf.MName = trim(colData); break;
	//case 5:		getMnt_Dbf.Task = atoi(trim(colData)); break;

	default:	break;
	} // switch

	return ret;
}


bool  CFileMountain::SetData_Entrance(int idx, stMntEntrance &getMnt_Dbf, char* colData)
{
	bool ret = true;

	switch (idx)
	{
	case 0:		getMnt_Dbf.ENT_ID = atoi(trim(colData)); break;
	case 1:		getMnt_Dbf.F_NODE_ID = atoll(trim(colData)); break;
	case 2:		getMnt_Dbf.W_NODE_ID = atoll(trim(colData)); break;
	case 3:		getMnt_Dbf.ENT_TYP = atoi(trim(colData)); break;
	case 4:		getMnt_Dbf.ENT_NAME = trim(colData); break;
	case 5:		getMnt_Dbf.MNT_CD = atoi(trim(colData)); break;
	case 6:		getMnt_Dbf.DP = atoi(trim(colData)); break;
	case 7:		getMnt_Dbf.G_ID = atoi(trim(colData)); break;

	default:	break;
	} // switch

	return ret;
}


bool CFileMountain::AddEntranceData(IN const KeyID keyId, IN const stEntranceInfo& entInfo)
{
	bool ret = false;

	if (keyId.llid != NULL_VALUE) {		
		unordered_map<uint64_t, stMntBoundary>::iterator it = m_mapBoundary.find(keyId.llid);
		if (it != m_mapBoundary.end()) {
			it->second.vtEntrance.emplace_back(entInfo);
			ret = true;
		}
	}

	return ret;
}


const stMntBoundary* CFileMountain::GetMountainData(IN const KeyID keyId) const
{
	const stMntBoundary* pMnt = nullptr;

	if (keyId.llid != NULL_VALUE) {
		unordered_map<uint64_t, stMntBoundary>::const_iterator it = m_mapBoundary.find(keyId.llid);
		if (it != m_mapBoundary.end()) {
			pMnt = &it->second;
		}
	}

	return pMnt;
}
