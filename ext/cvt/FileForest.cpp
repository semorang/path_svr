#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FileForest.h"

#include "../shp/shpio.h"
#include "../utils/GeoTools.h"
#include "../utils/UserLog.h"
#include "../utils/Strings.h"
#include "../route/MMPoint.hpp"

#include <algorithm>

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



static int32_t maxFwTm = 0;
static int32_t maxBwTm = 0;
static int32_t maxDiff = 0;
static int32_t maxPop = 0;



CFileForest::CFileForest()
{
	m_nFileType = TYPE_DATA_TREKKING;
}

CFileForest::~CFileForest()
{

}

static const uint32_t g_cntLogPrint = 100000;

bool CFileForest::ParseData(IN const char* fname)
{
	XSHPReader shpReader;

	m_nLinkIdx = 0;
	m_nNodeIdx = 0;

	if (!shpReader.Open(fname)) {
		LOG_TRACE(LOG_ERROR, "Error,Cannot Find shp File!");
		return false;
	}

//#if defined(USE_PROJ4_LIB)
//	initProj4("UTMK");
//#endif

	int nShpType = (int)shpReader.GetShpObjectType(); //shpPoint=1,shpPolyLine=3,shpPolygon=5

	if (nShpType == 5) nShpType = 3; //면
	else if (nShpType == 3) nShpType = 2; //선
	else if (nShpType == 1) nShpType = 1; //점

	//전체 지도영역 얻기
	SBox mapBox;
	shpReader.GetEnvelope(&mapBox);

//#if defined(USE_PROJ4_LIB)
//	translateUTMKtoWGS84(mapBox.Xmin, mapBox.Ymin, mapBox.Xmin, mapBox.Ymin);
//	translateUTMKtoWGS84(mapBox.Xmax, mapBox.Ymax, mapBox.Xmax, mapBox.Ymax);
//#endif

	SetMeshRegion(&mapBox);


	//전체 데이터 카운터 얻기
	long nRecCount = shpReader.GetRecordCount();
	int nFieldCount = shpReader.GetFieldCount();

	//int nLabelIdxToRead = 1;
	char chsTmp[61];
	int nPoint;
	SPoint* pPoints;
	unsigned int colCnt = shpReader.GetFieldCount();
	
	// 데이터 필드 무결성 확보를 위한 검사
	// 다른 데이터에도 추가해 주자
	if (strstr(fname, "NODE") != nullptr) {
		static char szLinkField[128][32] = {
			{ "NODE_ID" },{ "NODE_TYP" },{ "CN_NUM" },
			{ "CN_NODE1" },{ "CN_ANGLE1" },{ "CN_NODE2" },{ "CN_ANGLE2" },
			{ "CN_NODE3" },{ "CN_ANGLE3" },{ "CN_NODE4" },{ "CN_ANGLE4" },
			{ "CN_NODE5" },{ "CN_ANGLE5" },{ "CN_NODE6" },{ "CN_ANGLE6" },
			{ "CN_NODE7" },{ "CN_ANGLE7" },{ "CN_NODE8" },{ "CN_ANGLE8" },
			{ "CN_NODE9" },{ "CN_ANGLE9" },{ "CN_NODE10" },{ "CN_ANGLE10" },
			{ "CN_NODE11" },{ "CN_ANGLE11" },{ "CN_NODE12" },{ "CN_ANGLE12" },
			{ "CLIM_TYP" },{ "ENT_NAME" },{ "MNT_NM" },{ "NODE_Z" },{ "SIGN_ID" },
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
	else if (strstr(fname, "LINK") != nullptr) {
		static char szLinkField[128][32] = {
			{ "LINK_ID" },{ "F_NODE_ID" },{ "T_NODE_ID" },{ "COURS_TYP" },{ "COURS_CD" },
			{ "SC_TYP1" },{ "SC_CD1" },{ "SC_TYP2" },{ "SC_CD2" },
			{ "SC_TYP3" },{ "SC_CD3" },{ "SC_TYP4" },{ "SC_CD4" },
			{ "SC_TYP5" },{ "SC_CD5" },{ "SC_TYP6" },{ "SC_CD6" },
			{ "SC_TYP7" },{ "SC_CD7" },{ "SC_TYP8" },{ "SC_CD8" },
			{ "SC_TYP9" },{ "SC_CD9" },{ "SC_TYP10" },{ "SC_CD10" },
			{ "SC_TYP11" },{ "SC_CD11" },{ "SC_TYP12" },{ "SC_CD12" },
			{ "SC_TYP13" },{ "SC_CD13" },
			{ "DIR_CD" },{ "DIR_INFO" },
			{ "ROAD_INFO1" },{ "ROAD_INFO2" },{ "ROAD_INFO3" },{ "ROAD_INFO4" },
			{ "ROAD_INFO5" },{ "ROAD_INFO6" },
			{ "CONN" },{ "LINK_LEN" },{ "DIFF" },{"SLOP"},
			{ "FW_TM" },{ "RE_TM" },{ "POPULAR" },
			{ "SP_REST" },{ "SP_R_CAU" },{ "AU_REST" },{ "AU_R_CAU" },{ "MNT_CD" },
			{ "STORUNST" },{ "FROR_CD" },{ "FRTP_CD" },{ "KOFTR_GROU" },{ "DMCLS_CD" },
			{ "AGCLS_CD" },{ "DNST_CD" },{ "LEGAL_EX" },{ "THEME" },{ "SIG_SK" },
			{ "SIG_KT" },{ "SIG_LG" },{ "AREA_TYP" },{ "MANA_ORG" },
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

	//데이터 얻기
	LOG_TRACE(LOG_DEBUG, "LOG, start, raw data parsing file : %s, rec cnt:%d", fname, nRecCount);

	for (int idxRec = 0; idxRec < nRecCount; idxRec++) {
		stMesh mesh = { 0, };
		stForestLink link = { 0, };
		stForestNode node = { 0, };

		//속성가져오기
		if (shpReader.Fetch(idxRec) == false) { //error to read..
			LOG_TRACE(LOG_ERROR, "ERROR,Record %d is not available!!", idxRec);
			continue;
		}

		//Setting DBF Data
		for (unsigned int idxCol = 0; idxCol < colCnt; idxCol++) {
			memset(chsTmp, 0x00, sizeof(chsTmp));
			shpReader.GetDataAsString(idxCol, chsTmp, 60);  //nLabelIdxToRead 번째 필드 값을 chsTmp로 12byte 읽어옴.

			SHPGeometry *pSHPObj = shpReader.GetGeometry();

			if (nShpType == 1) {				// 점 일때				
				if (!SetData_Node(idxCol, node, chsTmp))
				{
					LOG_TRACE(LOG_ERROR, "Error, %s", GetErrorMsg());
					return  false;
				}
//#if defined(USE_PROJ4_LIB)
//				translateUTMKtoWGS84(pSHPObj->point.point.x, pSHPObj->point.point.y, node.NodeCoord.x, node.NodeCoord.y);
//#else
//				node.NodeCoord = pSHPObj->point.point;
//#endif
				node.NodeCoord = pSHPObj->point.point;
			}
			else if (nShpType == 2) {		 // 선 일때
				if (!SetData_Link(idxCol, link, chsTmp))
				{
					LOG_TRACE(LOG_ERROR, "Error, %s", GetErrorMsg());
					return  false;
				}
				if (idxCol == 0)
				{
					nPoint = SHP_GetNumPartPoints(pSHPObj, 0); //파트 0에 대한	   //선형 갯수..					 																	 
					pPoints = SHP_GetPartPoints(pSHPObj, 0); //파트 0에 대한	     //실제선형데이터..

					//for (unsigned int idxObj = 0; idxObj < nPoint; idxObj++) {
					for (int idxObj = nPoint - 1; idxObj >= 0; --idxObj) { // 선형이 거꾸로 되어 있나??
//#if defined(USE_PROJ4_LIB)
//						translateUTMKtoWGS84(pPoints[idxObj].x, pPoints[idxObj].y, pPoints[idxObj].x, pPoints[idxObj].y);
//#endif
						link.LinkVertex.emplace_back(pPoints[idxObj]);
					}
				}
			}
		} // for


		if (nShpType == 1) {
			// NODE
			stNodeInfo* pNode = new stNodeInfo;

			pNode->node_id.tile_id = 1; // 단일 메쉬, 메쉬id 1을 주자
			pNode->node_id.nid = node.NodeID;
			pNode->edgenode_id.llid = 0; // 미사용
			pNode->coord = node.NodeCoord;

			pNode->base.node_type = TYPE_DATA_TREKKING;
			pNode->base.point_type = node.NodeType;
			pNode->base.connnode_count = node.ConnectNum;

			pNode->tre.z_value = node.Zvalue;
			for (int ii = 0; ii < pNode->base.connnode_count; ii++)
			{
				//pNode->connnodes[ii].tile_id = node.MeshID;
				pNode->connnodes[ii].nid = node.ConnNode[ii];
			}

			// 원래의 ID와 변경된 IDX 의 매칭 테이블
			//m_mapNodeIndex.insert(pair<uint64_t, uint32_t>({ pNode->node_id.llid, m_nNodeIdx }));

			// 노드 ID를 IDX로 변경
			//pNode->node_id.nid = m_nNodeIdx++; 

			static uint32_t s_nMaxConnNodeNum = 0;
			if (s_nMaxConnNodeNum < node.ConnectNum) {
				s_nMaxConnNodeNum = node.ConnectNum;

				LOG_TRACE(LOG_DEBUG, "LOG, node max connected num : %d", s_nMaxConnNodeNum);
			}

			m_mapNode.insert({ pNode->node_id.llid, pNode });

			if (m_nNodeIdx++ % g_cntLogPrint == 0) {
				LOG_TRACE(LOG_DEBUG, "LOG, node data processing, %lld / %lld", m_nNodeIdx, nRecCount);
			}
		}
		else if (nShpType == 2) {
			stLinkInfo* pLink = new stLinkInfo;

			pLink->link_id.tile_id = 1; // 단일 메쉬, 메쉬id 1을 주자
			pLink->link_id.nid = link.LinkID;
			pLink->link_id.dir = 0; // link.CourseDir;

			pLink->snode_id.tile_id = 1; // 단일 메쉬, 메쉬id 1을 주자
			pLink->snode_id.nid = link.FromNodeID;
			pLink->enode_id.tile_id = 1; // 단일 메쉬, 메쉬id 1을 주자
			pLink->enode_id.nid = link.ToNodeID;

			pLink->length = link.LinkLen;
			pLink->name_idx = link.CourseDirNameIDX;
			//pLink->vtPts.assign(link.LinkVertex.begin(), link.LinkVertex.end());
			pLink->setVertex(&link.LinkVertex.front(), link.LinkVertex.size());

			pLink->base.link_type = TYPE_DATA_TREKKING;
			pLink->tre.course_type = link.CourseType;
			if (link.RoadInfo[0] > 0) {
				for (int ii = 0; ii < 6; ii++) {
					if (link.RoadInfo[ii] > 0) {
						pLink->tre.road_info |= (1 << (link.RoadInfo[ii] - 1));
					}
				}
			}
			else {
				pLink->tre.road_info = 0;
			}

			// 방면방향정보, 0:미정의, 1:정, 2:역
			pLink->tre.dir_cd = link.CourseDir;

			// 난이도
			if (link.Diff > 0) {
				pLink->tre.diff = link.Diff;
			}
			else {
				pLink->tre.diff = 0;
			}

			// 경사도
			pLink->tre.slop = link.Slop;

			// 정방향 이동 시간
			pLink->tre.fw_tm = link.FwTime;

			// 역방향 이동 시간
			pLink->tre.bw_tm = link.BwTime;

			// 인기도
			if (link.Popular > 0) {
				pLink->tre.popular = link.Popular;// *100 / 4095;
			}
			else {
				pLink->tre.popular = 0;
			}

			
			m_mapLink.insert({ pLink->link_id.llid, pLink });

			if (m_nLinkIdx++ % g_cntLogPrint == 0) {
				LOG_TRACE(LOG_DEBUG, "LOG, link data processing, %lld / %lld", m_nLinkIdx, nRecCount);
			}
		}
	} // for

//#if defined(USE_PROJ4_LIB)
//	releaseProj4();
//#endif

	shpReader.Close();

	// 최대 주행 시간
	if (nShpType == 2) {
		LOG_TRACE(LOG_DEBUG, "LOG, Max walking time on one-link, FW:%d, BW:%d, Diff:%d, Pop:%d", maxFwTm, maxBwTm, maxDiff, maxPop);
	}

	if (!m_mapNode.empty() && !m_mapLink.empty()) {
		return GenServiceData();
	}

	return true;
}

bool CFileForest::GenServiceData()
{
	bool ret = false;

	unordered_map<uint64_t, stNodeInfo*>::iterator itNode;
	unordered_map<uint64_t, stLinkInfo*>::iterator itLink;


	LOG_TRACE(LOG_DEBUG, "LOG, start, link s/e node indexing");
	LOG_TRACE(LOG_DEBUG, "LOG, start, node connected link indexing");

	for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++) {
		//// snode ID를 IDX로 변경
		// 숲길은 단일 메쉬니까 index처리 안하고 id를 그냥 사용
		// 노드의 connected node 정보를 연결된 link 정보로 변경
		if (1) {
			// snode에 접속되는 인접 노드의 정보 업데이트
			ret = false;
			if ((itNode = m_mapNode.find(itLink->second->snode_id.llid)) != m_mapNode.end()) {
				for (uint32_t ii = 0; ii < itNode->second->base.connnode_count; ii++)
				{
					if (itNode->second->connnodes[ii].tile_id == 0 && (itNode->second->connnodes[ii].nid == itLink->second->enode_id.nid)) {
						itNode->second->connnodes[ii] = itLink->second->link_id;

						ret = true;
						break;
					}
				} // for
			}
			else {
				LOG_TRACE(LOG_ERROR, "Failed, can't find node info what match with link snode, mesh:%d, link:%d, snode:%d", itLink->second->snode_id.tile_id, itLink->second->link_id.nid, itLink->second->snode_id.nid);
				continue;
			}

			if (!ret)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't find connected node info what match with link snode, mesh:%d, link:%d, snode:%d", itLink->second->snode_id.tile_id, itLink->second->link_id.nid, itLink->second->snode_id.nid);
				continue;
			}
		}
		else {
#if !defined(_USE_TEST_MESH)
			LOG_TRACE(LOG_ERROR, "Failed, can't find snode, mesh:%d, node:%d", itLink->second->snode_id.tile_id, itLink->second->enode_id.nid);
#endif
			itLink->second->snode_id.llid = 0;
		}



		//// enode ID를 IDX로 변경
		// 숲길은 단일 메쉬니까 index처리 안하고 id를 그냥 사용
		// 노드의 connected node 정보를 연결된 link 정보로 변경
		if (1) {
			// enode에 접속되는 노드의 접속 노드 정보 업데이트
			ret = false;
			if ((itNode = m_mapNode.find(itLink->second->enode_id.llid)) != m_mapNode.end()) {
				for (int ii = 0; ii < itNode->second->base.connnode_count; ii++)
				{
					if (itNode->second->connnodes[ii].tile_id == 0 && (itNode->second->connnodes[ii].nid == itLink->second->snode_id.nid)) {
						itNode->second->connnodes[ii] = itLink->second->link_id;

						ret = true;
						break;
					}
				} // for
			}
			else {
				LOG_TRACE(LOG_ERROR, "Failed, can't find node info what match with link enode, mesh:%d, link:%d, enode:%d", itLink->second->enode_id.tile_id, itLink->second->link_id.nid, itLink->second->enode_id.nid);
				continue;
			}

			if (!ret)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't find node info what match with link enode, mesh:%d, link:%d, enode:%d", itLink->second->enode_id.tile_id, itLink->second->link_id.nid, itLink->second->enode_id.nid);
				continue;
			}
		}
		else {
#if !defined(_USE_TEST_MESH)
			LOG_TRACE(LOG_ERROR, "Failed, can't find enode, mesh:%d, node:%d", itLink->second->enode_id.tile_id, itLink->second->enode_id.nid);
#endif
			itLink->second->enode_id.llid = 0;
		}
	}


	// 노드의 connected node가 모두 link 연결 되었는지 확인
	LOG_TRACE(LOG_DEBUG, "LOG, start, node and connected link check");

	for (itNode = m_mapNode.begin(); itNode != m_mapNode.end(); itNode++)
	{
		ret = true;
		for (int ii = 0; ii < itNode->second->base.connnode_count; ii++) {
			if (itNode->second->connnodes[ii].tile_id == 0) {
				LOG_TRACE(LOG_ERROR, "Failed, can't node matching with connected link, node:%d, idx:%d, connected node:%d", itNode->second->node_id.nid, ii, itNode->second->connnodes[ii].nid);
				ret = false;
				break;
			}
		}
	}


	//LOG_TRACE(LOG_DEBUG, "LOG, start, edge node indexing");

	//for (itNode = m_mapNode.begin(); itNode != m_mapNode.end(); itNode++)
	//{
	//	if (itNode->second->edgenode_id.llid <= 0) {
	//		continue;
	//	}

	//	// 구획 변교점 노드 ID를 IDX로 변경
	//	if ((itNodeIndex = m_mapNodeIndex.find(itNode->second->edgenode_id.llid)) != m_mapNodeIndex.end()) {
	//		itNode->second->edgenode_id.nid = itNodeIndex->second;
	//	}
	//	else {
	//		LOG_TRACE(LOG_ERROR, "Failed, can't find edge node, mesh:%d, node:%d", itNode->second->edgenode_id.tile_id, itNode->second->edgenode_id.nid);
	//		continue;
	//	}
	//}


	uint32_t cntReversLinkVtx = 0;

	// 인덱스 매칭
	LOG_TRACE(LOG_DEBUG, "LOG, start, link s/e node index matching");

	for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++)
	{
		int isReversLinkVtx = 0;

		// snode
		itNode = m_mapNode.find(itLink->second->snode_id.llid);
		if (itNode == m_mapNode.end())
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't find snode by link, mesh:%d, link:%d", itLink->second->link_id.tile_id, itLink->second->link_id.nid);
			continue;
		}
		else if (itNode->second->node_id.llid <= 0)
		{
			LOG_TRACE(LOG_ERROR, "Failed, snode not indexed, mesh:%d, link:%d", itNode->second->node_id.tile_id, itNode->second->node_id.nid);
			continue;
		}

		itLink->second->snode_id = itNode->second->node_id;

		// 버텍스 방향성 확인
		if (itNode->second->coord.x == itLink->second->getVertexX(itLink->second->getVertexCount() - 1))
		{
			// 거꾸로 돌리자
			//reverse(itLink->second->getVertex(), itLink->second->getVertex() + itLink->second->getVertexCount());
			itLink->second->reverseVertex();
			cntReversLinkVtx++;
			isReversLinkVtx++;
			//vector<SPoint> tmpVtx;
			//tmpVtx.insert(tmpVtx.begin(), itLink->second->vtPts.rbegin(), itLink->second->vtPts.rend());
			//itLink->second->vtPts.swap(tmpVtx);
		}



		// enode
		itNode = m_mapNode.find(itLink->second->enode_id.llid);
		if (itNode == m_mapNode.end())
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't find enode by link, mesh:%d, link:%d", itLink->second->link_id.tile_id, itLink->second->link_id.nid);
			continue;
		}
		else if (itNode->second->node_id.llid <= 0)
		{
			LOG_TRACE(LOG_ERROR, "Failed, enode not indexed, mesh:%d, link:%d", itNode->second->node_id.tile_id, itNode->second->node_id.nid);
			continue;
		}

		itLink->second->enode_id = itNode->second->node_id;



		// 버텍스 방향성 확인, 거꾸로 돌리자
		if (itNode->second->coord.x == itLink->second->getVertexX(0))
		{
			// 거꾸로 돌리자
			//reverse(itLink->second->getVertex(), itLink->second->getVertex() + itLink->second->getVertexCount());
			itLink->second->reverseVertex();
			cntReversLinkVtx;
			isReversLinkVtx;
			//vector<SPoint> tmpVtx;
			//tmpVtx.insert(tmpVtx.begin(), itLink->second->vtPts.rbegin(), itLink->second->vtPts.rend());
			//itLink->second->vtPts.swap(tmpVtx);
		}

		// 링크 버텍스가 2번 연속 바뀌었다면 이상한거지
		if (isReversLinkVtx > 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, link vertex ordering with s/e node miss match, mesh:%d, link:%d", itLink->second->link_id.tile_id, itLink->second->link_id.nid);
			continue;
		}
	}

	if (cntReversLinkVtx > 0) {
		LOG_TRACE(LOG_DEBUG, "LOG, link vertex re-matching with s/e node total count:%d", cntReversLinkVtx);
	}



	//// 링크 결합
	//LOG_TRACE(LOG_DEBUG, "LOG, start, link merge");

	//int32_t cntProc = 0;
	//for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++)
	//{
	//	MergeLink(itLink->second, &m_mapLink, &m_mapNode);

	//	if (++cntProc % g_cntLogPrint == 0) {
	//		LOG_TRACE(LOG_DEBUG, "LOG, processing, link merge: %d/%d", cntProc, m_mapLink.size());
	//	}
	//}


	// 각도 설정
	//LOG_TRACE(LOG_DEBUG, "LOG, start, link angle");

	//int cntLinkVtx;
	//MMPoint<double> coord1;
	//MMPoint<double> coord2;

	//cntProc = 0;
	//for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++)
	//{
	//	cntLinkVtx = itLink->second->getVertexCount();

	//	// ang
	//	if (cntLinkVtx >= 2) {
	//		// snode ang
	//		coord1 = { itLink->second->getVertexX(0), itLink->second->getVertexY(0) };
	//		coord2 = { itLink->second->getVertexX(1), itLink->second->getVertexY(1) };
	//		itLink->second->base.snode_ang = coord1.azimuth(coord2);
	//		
	//		// enode ang
	//		coord1 = { itLink->second->getVertexX(cntLinkVtx - 1), itLink->second->getVertexY(cntLinkVtx - 1) };
	//		coord2 = { itLink->second->getVertexX(cntLinkVtx - 2), itLink->second->getVertexY(cntLinkVtx - 2) };
	//		itLink->second->base.enode_ang = coord1.azimuth(coord2);
	//	}

	//	if (++cntProc % g_cntLogPrint == 0) {
	//		LOG_TRACE(LOG_DEBUG, "LOG, processing, link angle: %d/%d", cntProc, m_mapLink.size());
	//	}
	//}



	// 클래스에 추가

	LOG_TRACE(LOG_DEBUG, "LOG, start, add data to class");

	// 단일 메쉬
	stMeshInfo* newMesh = new stMeshInfo;
	newMesh->mesh_id.tile_id = 1; // 단일 메쉬id 1부여
	memcpy(&newMesh->mesh_box, GetMeshRegion(), sizeof(newMesh->mesh_box));
	memcpy(&newMesh->data_box, GetMeshRegion(), sizeof(newMesh->data_box));

	if (!AddMeshData(newMesh)) {
		LOG_TRACE(LOG_ERROR, "Failed, add mesh data, mesh:%d", newMesh->mesh_id.tile_id);
	}


	LOG_TRACE(LOG_DEBUG, "LOG, start, add to node class");

	for (itNode = m_mapNode.begin(); itNode != m_mapNode.end(); itNode++)
	{
		if (!AddNodeData(itNode->second)) {
			stMeshInfo* pMeshExt = new stMeshInfo;
			pMeshExt->mesh_id.tile_id = itNode->second->node_id.tile_id;

			//if (AddMeshDataByNode(pMeshExt, itNode->second)) {
			//	LOG_TRACE(LOG_DEBUG, "add new mesh, when mesh info not exist, mesh:%d, node:%d", pMeshExt->mesh_id.tile_id, itNode->second->node_id.nid);
			//}
		}
	}


	LOG_TRACE(LOG_DEBUG, "LOG, start, add to link class");

	for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++)
	{
		if (!AddLinkData(itLink->second)) {
			//stMeshInfo* pMeshExt = new stMeshInfo;
			//pMeshExt->mesh_id.tile_id = itLink->second->link_id.tile_id;

			//if (AddMeshDataByLink(pMeshExt, itLink->second)) {
			//	LOG_TRACE(LOG_DEBUG, "add new mesh, when mesh info not exist, mesh:%d, link:%d", pMeshExt->mesh_id.tile_id, itLink->second->link_id.nid);
			//}
		}
	}


	LOG_TRACE(LOG_DEBUG, "LOG, finished");


	return CFileBase::GenServiceData();
}


void CFileForest::AddDataFeild(IN const int idx, IN const int type, IN const char* colData)
{

}


void CFileForest::AddDataRecord()
{

}


bool CFileForest::Initialize()
{
	return CFileBase::Initialize();
}

void CFileForest::Release()
{
	CFileBase::Release();
}


bool CFileForest::OpenFile(IN const char* szFilePath)
{
	return CFileBase::OpenFile(szFilePath);
}


bool CFileForest::SaveData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_TREKKING], g_szTypeExec[TYPE_DATA_TREKKING]);

	return CFileBase::SaveData(szFileName);
}


bool CFileForest::LoadData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_TREKKING], g_szTypeExec[TYPE_DATA_TREKKING]);

	return CFileBase::LoadData(szFileName);
}


bool  CFileForest::SetData_Node(int idx, stForestNode &getNode_Dbf, char* colData)
{
	bool ret = true;
	switch (idx)
	{
	case 0:		getNode_Dbf.NodeID = atoi(trim(colData));	break;		
	case 1:		getNode_Dbf.NodeType = atoi(trim(colData)); // 노드 타입, 1:교차점, 2:단점,3:속성변화점
		if (getNode_Dbf.NodeType < 1 || 3 < getNode_Dbf.NodeType) {
			ret = false; m_strErrMsg = "node type value not defined : " + string(colData);
		} break;
	case 2:		getNode_Dbf.ConnectNum = atoi(trim(colData)); // 접속노드 수
		if (getNode_Dbf.ConnectNum < 0 || 12 < getNode_Dbf.ConnectNum) {
			ret = false; m_strErrMsg = "connected node value not defined : " + string(colData);
		} break;
	case 3: case 5: case 7: case 9: case 11: case 13: case 15: case 17: case 19: case 21: case 23: case 25:
		getNode_Dbf.ConnNode[(idx - 3) / 2] = atoi(trim(colData));	break; // 접속노드번호
	case 4: case 6: case 8: case 10: case 12: case 14: case 16: case 18: case 20: case 22: case 24: case 26:
		getNode_Dbf.ConnNodeAng[(idx - 4) / 2] = atoi(trim(colData)); break; // 접속노드각도
	case 27:	getNode_Dbf.ClimType = atoi(trim(colData)); break; // 등산기점 구분코드
	case 28:	getNode_Dbf.EntNameIDX = AddNameDic(colData); break; // 입구명칭
	case 29:	getNode_Dbf.MntNameIDX = AddNameDic(colData); break; // 봉우리명칭
	case 30:	getNode_Dbf.Zvalue = atoi(trim(colData)); break; // 노드높이
	case 31:	getNode_Dbf.SignID = atoi(trim(colData)); break; // 이정표ID

	default:	break;
	}

	return ret;
}


bool CFileForest::SetData_Link(int idx, stForestLink &getLink_Dbf, char* colData)
{
	bool ret = true;

	switch (idx)
	{
	case 0:		getLink_Dbf.LinkID = atoi(trim(colData));		break;
	case 1:		getLink_Dbf.FromNodeID = atoi(trim(colData));	break;
	case 2:		getLink_Dbf.ToNodeID = atoi(trim(colData));		break;
	case 3:		getLink_Dbf.CourseType = atoi(trim(colData)); // 코스타입, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
		if (getLink_Dbf.CourseType < 1 || TYPE_TRE_COUNT <= getLink_Dbf.CourseType) {
			ret = false; m_strErrMsg = "course type value not defined : " + string(colData);
		} break;
	case 4:		getLink_Dbf.CourseNameCD = atoi(trim(colData));	break; // 코스 명칭 코드
	case 5:	case 7: case 9: case 11: case 13: case 15: case 17: case 19: case 21: case 23: case 25: case 27: case 29:
		getLink_Dbf.ConnCourseType[(idx - 5) / 2] = atoi(trim(colData)); // 중용코스타입, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
		if (getLink_Dbf.ConnCourseType[(idx - 5) / 2] < 0 || 7 < getLink_Dbf.ConnCourseType[(idx - 5) / 2]) {
			ret = false; m_strErrMsg = "ConnCourseType type value not defined : " + string(colData);
		} break;
	case 6: case 8: case 10: case 12: case 14: case 16: case 18: case 20: case 22: case 24: case 26: case 28: case 30:
		getLink_Dbf.ConnCourseTypeNameIDX[(idx - 6) / 2] = atoi(trim(colData));	break; // 중용코스명칭 코드
	case 31:		getLink_Dbf.CourseDir = atoi(trim(colData)); // 방면방향정보, 0:미정의, 1:정, 2:역
		if (getLink_Dbf.CourseDir < 0 || 2 < getLink_Dbf.CourseDir) {
			ret = false; m_strErrMsg = "course dir value not defined : " + string(colData);
		} break;
	case 32:		getLink_Dbf.CourseDirNameIDX = atoi(trim(colData));	break; // 진행방면정보 인덱스
	case 33: case 34: case 35: case 36: case 37: case 38: // 노면정보, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드, 12:철구조물
		getLink_Dbf.RoadInfo[idx - 33] = atoi(trim(colData));
		if (getLink_Dbf.RoadInfo[idx - 33] < 0 || 12 < getLink_Dbf.RoadInfo[idx - 33]) {
			ret = false; m_strErrMsg = "road type value not defined : " + string(colData);
		} break;
	case 39:	getLink_Dbf.ConnectType = atoi(trim(colData)); // 연결로 여부, 0:기타, 1:연결로
		if (getLink_Dbf.ConnectType < 0 || 1 < getLink_Dbf.ConnectType) {
			ret = false; m_strErrMsg = "connect type value not defined : " + string(colData);
		} break;
	case 40:	getLink_Dbf.LinkLen = atof(trim(colData)); // 링크 길이
		if (getLink_Dbf.LinkLen < 0 || 200 < getLink_Dbf.LinkLen) {
			ret = false; m_strErrMsg = "link length not defined : " + string(colData);
		} break;
	case 41:	getLink_Dbf.Diff = atoi(trim(colData)); // 0~100 난이도(숫자가 클수록 어려움)
		if (maxDiff < getLink_Dbf.Diff) maxDiff = getLink_Dbf.Diff;		
		if (getLink_Dbf.Diff < 0 || 100 < getLink_Dbf.Diff) {
			ret = false; m_strErrMsg = "difficult value not defined : " + string(colData);
		} break;
	case 42:	getLink_Dbf.Slop = atoi(trim(colData)); // 경사도 +/- 128(버텍스 순서기준 정방향 경사도)
		if (getLink_Dbf.Slop < -127 || 127 < getLink_Dbf.Slop) {
			ret = false; m_strErrMsg = "slop value not defined : " + string(colData);
		} break;
	case 43:	getLink_Dbf.FwTime = atoi(trim(colData)); // 정방향 이동 소요시간 (초), 0-1020
		if (maxFwTm < getLink_Dbf.FwTime) maxFwTm = getLink_Dbf.FwTime;
		if (getLink_Dbf.FwTime < 0 || 1023 < getLink_Dbf.FwTime) {
			ret = false; m_strErrMsg = "forward travel value not defined : " + string(colData);
		} break;
	case 44:	getLink_Dbf.BwTime = atoi(trim(colData)); // 역방향 이동 소요시간 (분->초), 0-1020
		if (maxBwTm < getLink_Dbf.BwTime) maxBwTm = getLink_Dbf.BwTime;
		if (getLink_Dbf.BwTime < 0 || 1023 < getLink_Dbf.BwTime) {
			ret = false; m_strErrMsg = "back travel value not defined : " + string(colData);
		} break;
	case 45:	getLink_Dbf.Popular = atoi(trim(colData)); // 인기도 지수, 0-4095
		if (maxPop < getLink_Dbf.Popular) maxPop = getLink_Dbf.Popular;
		if (getLink_Dbf.Popular < 0 || 4095 < getLink_Dbf.Popular) {
			ret = false; m_strErrMsg = "Popular value not defined : " + string(colData);
		} break;

	default:	break;
	}

	return ret;
}
