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
#include "../route/DataManager.h"

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
static int32_t maxGrade = 0;
static int32_t maxGid = 0;
static int32_t minSlop = 0;
static int32_t maxSlop = 0;
static int32_t maxCourseCD = 0;


// 해당 코스ID는 사용하지 말아야할 링크로 전달받음(데이터팀), 2024-05-08
#define NOT_USE_LINK_DATA_CODE	999

CFileForest::CFileForest()
{
	m_nDataType = TYPE_DATA_TREKKING;
	m_nFileType = TYPE_EXEC_NETWORK;
}

CFileForest::~CFileForest()
{

}


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
			{ "NODE_ID" },{ "NODE_TYP" },{ "CN_NUM" },{ "CN_NODE1" },{ "CN_ANGLE1" },
			{ "CN_NODE2" },{ "CN_ANGLE2" },{ "CN_NODE3" },{ "CN_ANGLE3" },{ "CN_NODE4" }, // 10
			{ "CN_ANGLE4" },{"CN_NODE5" },{ "CN_ANGLE5" },{ "CN_NODE6" },{ "CN_ANGLE6" },
			{ "CN_NODE7" },{ "CN_ANGLE7" },{ "CN_NODE8" },{ "CN_ANGLE8" },{ "CN_NODE9" }, // 20
			{ "CN_ANGLE9" },{ "CN_NODE10" },{ "CN_ANGLE10" },{ "CN_NODE11" },{ "CN_ANGLE11" },
			{ "CN_NODE12" },{ "CN_ANGLE12" },{ "CLIM_TYP" },{ "ENT_NAME" },{ "MNT_NM" }, // 30
			{ "NODE_Z" },{ "SIGN_ID" },
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
	else if (strstr(fname, "LINK") != nullptr) {
		static char szLinkField[128][32] = {
			{ "LINK_ID" },{ "F_NODE_ID" },{ "T_NODE_ID" },{ "COURS_TYP" },{ "COURS_CD" }, 
			{ "SC_TYP1" },{ "SC_CD1" },{ "SC_TYP2" },{ "SC_CD2" },{ "SC_TYP3" }, // 10
			{ "SC_CD3" },{ "SC_TYP4" },{ "SC_CD4" },{ "SC_TYP5" },{ "SC_CD5" }, 
			{ "SC_TYP6" },{ "SC_CD6" },{ "SC_TYP7" },{ "SC_CD7" },{ "SC_TYP8" }, // 20
			{ "SC_CD8" },{ "SC_TYP9" },{ "SC_CD9" },{ "SC_TYP10" },{ "SC_CD10" },
			{ "SC_TYP11" },{ "SC_CD11" },{ "SC_TYP12" },{ "SC_CD12" },{ "SC_TYP13" }, // 30
			{ "SC_CD13" },{ "SC_TYP14" },{ "SC_CD14" },{ "SC_TYP15" },{ "SC_CD15" },
			{ "SC_TYP16" },{ "SC_CD16" },{ "SC_TYP17" },{ "SC_CD17" },{ "DIR_CD" }, // 40
			{ "DIR_INFO" },{ "ROAD_INFO1" },{ "ROAD_INFO2" },{ "ROAD_INFO3" },{ "ROAD_INFO4" },
			{ "ROAD_INFO5" },{ "ROAD_INFO6" },{ "CONN" },{ "LINK_LEN" },{ "DIFF" }, // 50
			{ "SLOP" },{ "FW_TM" },{ "RE_TM" },{ "POPULAR" },{ "SP_REST" },
			{ "SP_R_CAU" },{ "AU_REST" },{ "AU_R_CAU" },{ "MNT_CD" },{ "STORUNST" }, // 60
			{ "FROR_CD" },{ "FRTP_CD" },{ "KOFTR_GROU" },{ "DMCLS_CD" },{ "AGCLS_CD" },
			{ "DNST_CD" },{ "LEGAL_EX" },{ "THEME" },{ "SIG_SK" },{ "SIG_KT" }, // 70
			{ "SIG_LG" },{ "AREA_TYP" },{ "MANA_ORG" },{"DP"}, {"MESH"}, // 75
			{ "G_ID" },{ "GRADE" },
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

	//데이터 얻기
	LOG_TRACE(LOG_DEBUG, "LOG, start, raw data parsing file : %s, rec cnt:%d", fname, nRecCount);

	SHPGeometry *pSHPObj = nullptr;

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
		bool isTestMesh = true;

		for (unsigned int idxCol = 0; idxCol < colCnt; idxCol++) {
			memset(chsTmp, 0x00, sizeof(chsTmp));
			shpReader.GetDataAsString(idxCol, chsTmp, 60);  //nLabelIdxToRead 번째 필드 값을 chsTmp로 12byte 읽어옴.

			pSHPObj = shpReader.GetGeometry();


			if (g_isUseTestMesh && !CheckDataInMesh(pSHPObj->point.point.x, pSHPObj->point.point.y)) {
				isTestMesh = false;
				break;
			}


			if (nShpType == 1) {				// 점 일때		
				if (!SetData_Node(idxCol, node, chsTmp))
				{
					LOG_TRACE(LOG_ERROR, "Error, Node data compile error, idx:%d,  msg:%s", idxRec, GetErrorMsg());
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
					if ((idxCol == 4) && (link.CourseCD == NOT_USE_LINK_DATA_CODE)) {
						// 사용하지 말아야할 링크로 전달받음(데이터팀), 2024-05-08
						break;
					}

					LOG_TRACE(LOG_ERROR, "Error, Link data compile error, idx:%d,  msg:%s", idxRec, GetErrorMsg());
					//return  false;  // 오류로 처리하지는 말자 2024.02.21
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


		if (g_isUseTestMesh && !isTestMesh) {
			continue;
		}


		if (nShpType == 1) {
			// NODE
			stNodeInfo* pNode = new stNodeInfo;

			pNode->node_id.tile_id = g_forestMeshId; // 단일 메쉬
			pNode->node_id.nid = node.NodeID;
			pNode->edgenode_id.llid = 0; // 미사용
			pNode->coord = node.NodeCoord;

			pNode->base.node_type = TYPE_NODE_DATA_TREKKING;
			pNode->base.point_type = node.NodeType;
			pNode->base.connnode_count = node.ConnectNum;

			pNode->trk.z_value = node.Zvalue;
			for (int ii = 0; ii < pNode->base.connnode_count; ii++)
			{
				//pNode->connnodes[ii].tile_id = node.MeshID;
				pNode->connnodes[ii].nid = node.ConnNode[ii];
			}

			pNode->trk.ent_info = (node.EntNameIDX == 0) ? 0 : 1;
			pNode->trk.mnt_info = (node.MntNameIDX == 0) ? 0 : 1;
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
			// 미사용 링크
			if (link.Dp != 0) {
				continue;
			}

			stLinkInfo* pLink = new stLinkInfo;

			pLink->link_id.tile_id = g_forestMeshId; // 단일 메쉬
			pLink->link_id.nid = link.LinkID;
			pLink->link_id.dir = 0; // link.CourseDir;

			pLink->snode_id.tile_id = g_forestMeshId; // 단일 메쉬
			pLink->snode_id.nid = link.FromNodeID;
			pLink->enode_id.tile_id = g_forestMeshId; // 단일 메쉬
			pLink->enode_id.nid = link.ToNodeID;

			pLink->length = link.LinkLen;
			pLink->name_idx = link.CourseDirNameIDX;
			//pLink->vtPts.assign(link.LinkVertex.begin(), link.LinkVertex.end());
			pLink->setVertex(&link.LinkVertex.front(), link.LinkVertex.size());

			pLink->base.link_type = TYPE_LINK_DATA_TREKKING;
			pLink->trk.course_type = link.CourseType;
			if (link.RoadInfo[0] > 0) {
				for (int ii = 0; ii < 6; ii++) {
					if (link.RoadInfo[ii] > 0) {
						pLink->trk.road_info |= (1 << (link.RoadInfo[ii] - 1));
					}
				}
			}
			else {
				pLink->trk.road_info = 0;
			}

			// 방면방향정보, 0:미정의, 1:정, 2:역
			pLink->trk.dir_cd = link.CourseDir;

			// 난이도
			if (link.Diff > 0) {
				pLink->trk.diff = link.Diff;
			} else {
				pLink->trk.diff = 0;
			}

			// 경사도
			pLink->trk.slop = link.Slop;

			// 정방향 이동 시간
			pLink->trk.fw_tm = link.FwTime;

			// 역방향 이동 시간
			pLink->trk.bw_tm = link.BwTime;

			// 인기도등급, 이제는 인기도 대신 인기도 등급으로 사용하자(2024-06-25~)
			if (link.Grade > 0) {
				pLink->trk.pop_grade = 10 - link.Grade; // link.Popular
			} else {
				pLink->trk.pop_grade = 10;
			}

			// 법정탐방로
			pLink->trk.legal = link.LeGalEx; // 법정탐방로 여부, 0:비법정, 1:법정

			// 확장
#if defined(USE_MOUNTAIN_DATA)
			// 숲 링크 그룹 ID
			pLink->trk_ext.group_id = link.Gid;
			if (maxGid < link.Gid) {
				maxGid = link.Gid;
			}

			// 코스 정보
			pLink->trk_ext.course_cnt = link.CourseCount;
			if (pLink->trk_ext.course_cnt > 0) {
				pLink->trk_ext.course_cd = link.CourseCD;

				map<uint64_t, set<uint32_t>>::iterator itCBL;
				map<uint32_t, set<uint64_t>>::iterator itLBC;

				stCourseInfo coursInfo;
				int32_t courseCD;

				// 중용 코스 정보 추가
				for (int connIdx = 0; connIdx < link.CourseCount; connIdx++) {
					coursInfo.course_type = link.ConnCourseType[connIdx];
					coursInfo.course_cd = courseCD = link.ConnCourseCD[connIdx];

					// 툴에서 바로 사용하기 위한 용도
					m_pDataMgr->AddLinkDataByCourse(courseCD, pLink->link_id.llid); // 코스 CD로 Link ID 검색용
					m_pDataMgr->AddCourseDataByLink(pLink->link_id.llid, coursInfo.course_id); // 링크 ID로 코스 정보(type,cd) 검색용

					// 저장하기 위한 용도
					m_mapCourse.AddLinkDataByCourse(courseCD, pLink->link_id.llid); // 코스 CD로 Link ID 검색용
					m_mapCourse.AddCourseDataByLink(pLink->link_id.llid, coursInfo.course_id); // 링크 ID로 코스 정보(type,cd) 검색용

					if (maxCourseCD < courseCD) {
						maxCourseCD = courseCD;
					}
				} // for
			}

			// 임시, 다중 메쉬 링크 적용
			if (link.Mesh >= 0xFFFFFF) {
				LOG_TRACE(LOG_ERROR, "------------>>> forest mesh id too big, %d / %d", link.Mesh, 0xFFFFFF);
			}
			pLink->trk_ext.trk_ext_reserved = link.Mesh;
#endif // #if defined(USE_MOUNTAIN_DATA)

			m_mapLink.insert({ pLink->link_id.llid, pLink });

			if (m_nLinkIdx++ % g_cntLogPrint == 0) {
				LOG_TRACE(LOG_DEBUG, "LOG, link data processing, %lld / %lld", m_nLinkIdx, nRecCount);
			}
		}
	} // for

	// 최대 코스 CD 값
	if (nShpType == 2) {		 // 선 일때
		LOG_TRACE(LOG_DEBUG, "LOG, Max walking time on one-link, FW:%d, BW:%d, Diff:%d, Pop:%d, Grade:%d, Gid:%d, Course:%d, Slop:%d-%d", maxFwTm, maxBwTm, maxDiff, maxPop, maxGrade, maxGid, maxCourseCD, minSlop, maxSlop);// 최대 코스 CD 값
	}

//#if defined(USE_PROJ4_LIB)
//	releaseProj4();
//#endif

	shpReader.Close();

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
			// 테스트 메쉬에서는 노드 끊김이 있을 수 있으니 무시하자
			if (!g_isUseTestMesh) {
				LOG_TRACE(LOG_ERROR, "Failed, can't find snode, mesh:%d, node:%d", itLink->second->snode_id.tile_id, itLink->second->enode_id.nid);
			}
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
			// 테스트 메쉬에서는 노드 끊김이 있을 수 있으니 무시하자
			if (!g_isUseTestMesh) {
				LOG_TRACE(LOG_ERROR, "Failed, can't find enode, mesh:%d, node:%d", itLink->second->enode_id.tile_id, itLink->second->enode_id.nid);
			}
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
	newMesh->mesh_id.tile_id = g_forestMeshId; // 단일 메쉬
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


bool CFileForest::SaveData(IN const char* szFilePath)
{
	//char szFileName[MAX_PATH] = { 0, };
	//sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_TREKKING], g_szTypeExec[TYPE_EXEC_NETWORK]);

	// save link/node data
	bool ret = CFileBase::SaveData(szFilePath);

	// save course data
	if (ret == true) {
		ret = SaveCourseData(szFilePath);
	}

	return ret;
}


bool CFileForest::SaveCourseData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[m_nDataType], g_szTypeExec[TYPE_EXEC_COURSE]);

	// save course data
	if (!m_pDataMgr) {
		LOG_TRACE(LOG_ERROR, "Failed, not initialized data manager");
		return false;
	}
	else if (szFileName == nullptr || strlen(szFileName) <= 0) {
		LOG_TRACE(LOG_ERROR, "Failed, file name not exist");
		return false;
	}

	size_t offFile = 0;
	size_t offItem = 0;
	size_t retWrite = 0;
	size_t retRead = 0;


	// 이전 링크/노드 단계에서 경로 체크 완료
//	// path check
//	char* pTok = strrchr((char*)szFilePath, '/');
//	if (pTok != nullptr) {
//		char szPath[MAX_PATH] = { 0, };
//		strncpy(szPath, szFilePath, (pTok - szFilePath));
//#if defined(_WIN32)
//		// check directory created
//		if (_access_s(szPath, 0) != 0) {
//			LOG_TRACE(LOG_DEBUG, "save directory was not created : %s", szPath);
//
//			if (_mkdir(szPath) != 0) {
//				LOG_TRACE(LOG_ERROR, "Error, Can't create save directory : %s", szPath);
//				return false;
//			}
//		}
//#else
//		// check directory created
//		if (access(szPath, W_OK) != 0) {
//			LOG_TRACE(LOG_DEBUG, "save directory was not created : %s", szPath);
//
//			if (mkdir(szPath, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
//				LOG_TRACE(LOG_ERROR, "Error, Can't create save directory : %s", szPath);
//				return false;
//			}
//		}
//#endif
//	}

	/////////////////////////////////////////
	// data
	FILE* fp = fopen(szFileName, "wb+");
	if (!fp)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't open file for save data, file:%s", szFileName);
		return false;
	}


	// write base
	offFile = WriteBase(fp);

	// check offset 
	if (sizeof(FileBase) != offFile)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file base offset not match, base:%d vs current:%d", sizeof(FileBase), offFile);
		fclose(fp);
		return false;
	}


	// write header
	offFile += WriteCourseHeader(fp);

	// check offset 
	if (m_fileCourseHeader.offIndex != offFile)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file header index offset not match, header:%d vs current:%d", m_fileCourseHeader.offIndex, offFile);
		fclose(fp);
		return false;
	}


	// write index
	offFile += WriteCourseIndex(fp);

	// check offset 
	if (m_fileCourseHeader.offBody != offFile)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file header body offset not match, header:%d vs current:%d", m_fileCourseHeader.offBody, offFile);
		fclose(fp);
		return false;
	}


	// write body
	offFile += WriteCourseBody(fp, offFile);

	// check offset 
	if (offFile <= 0)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file body size zero");
		fclose(fp);
		return false;
	}


	//// re-write index for body size and offset
	//fseek(fp, m_fileHeader.offIndex, SEEK_SET);
	//for (uint32_t idx = 0; idx < m_fileHeader.cntIndex; idx++)
	//{
	//	FileIndex fileIndex;
	//	memcpy(&fileIndex, &m_vtCourseIndex[idx], sizeof(fileIndex));
	//	if ((retRead = fwrite(&fileIndex, sizeof(fileIndex), 1, fp)) != 1)
	//	{
	//		LOG_TRACE(LOG_ERROR, "Failed, can't re-write file index data, idx:%d", idx);
	//		fclose(fp);
	//		return false;
	//	}
	//}


	fclose(fp);


	//LOG_TRACE(LOG_DEBUG, "Save data, total mesh cnt:%lld", cntTotalMesh);
	//LOG_TRACE(LOG_DEBUG, "Save data, total node cnt:%lld", cntTotalNode);
	//LOG_TRACE(LOG_DEBUG, "Save data, total link cnt:%lld", cntTotalLink);



	// release memory
	LOG_TRACE(LOG_DEBUG, "LOG, Release data");
	Release();

	return true;
}


size_t CFileForest::WriteCourseHeader(FILE* fp)
{
	size_t offFile = 0;
	size_t retWrite = 0;
	const size_t sizeHeader = sizeof(FileHeader);

	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	// 인덱스 별로 course by code, course by link 데이터를 저장
	m_fileCourseHeader.cntIndex = 2;
	m_fileCourseHeader.offIndex = sizeof(FileBase) + sizeHeader;
	m_fileCourseHeader.offBody = m_fileCourseHeader.offIndex + sizeof(FileIndex) * m_fileCourseHeader.cntIndex;

	if ((retWrite = fwrite(&m_fileCourseHeader, sizeHeader, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't write header, written:%d", retWrite);
		return 0;
	}
	offFile += sizeHeader;

	LOG_TRACE(LOG_DEBUG, "Save data, header, mesh cnt:%lld", m_fileCourseHeader.cntIndex);

	return offFile;
}


size_t CFileForest::WriteCourseIndex(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileIndex fileIndex;

	size_t offFile = 0;
	size_t retWrite = 0;
	const size_t sizeIndex = sizeof(fileIndex);

	// 인덱스 별로 course by code, course by link 데이터를 저장
	// 1st, course by code
	int idx = 0;
	memset(&fileIndex, 0x00, sizeof(fileIndex));

	fileIndex.idxTile = idx;
	if ((retWrite = fwrite(&fileIndex, sizeIndex, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't write index[%d] - course by code, written:%d", idx, retWrite);
		return 0;
	}
	offFile += sizeIndex;

	// add body data info buff
	m_vtCourseIndex.emplace_back(fileIndex);


	// 2nd, course by link
	idx++;
	memset(&fileIndex, 0x00, sizeof(fileIndex));

	fileIndex.idxTile = idx;
	if ((retWrite = fwrite(&fileIndex, sizeIndex, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't write index[%d] - course by link, written:%d", idx, retWrite);
		return 0;
	}
	offFile += sizeIndex;

	// add body data info buff
	m_vtCourseIndex.emplace_back(fileIndex);



	LOG_TRACE(LOG_DEBUG, "Save data, course index, cnt:%lld", idx + 1);

	return offFile;
}


size_t CFileForest::WriteCourseBody(FILE* fp, IN const uint32_t fileOff)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	stMeshInfo* pMesh = nullptr;

	FileBody fileBody;

	size_t offFile = fileOff;
	size_t retWrite = 0;
	long offItem = 0;
	const size_t sizeFileBody = sizeof(fileBody);



	KeyID link_id;
	stCourseInfo coursInfo;
	uint32_t coursCD;
	uint32_t cntItem;

	int idx = 0;

	// 1st, write course by code
	memset(&fileBody, 0x00, sizeFileBody);
	fileBody.idTile = idx;

	if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't write course body, idx:%d", idx);
		return 0;
	}
	offItem = sizeFileBody;

	// write course by id
	const unordered_map<uint32_t, set<uint64_t>>* pCourseData = m_mapCourse.GetLinkDataByCourse();
	for (const auto& item : *pCourseData) {
		coursInfo.course_cd = item.first;
		coursCD = coursInfo.course_cd;

		// write course id
		if ((retWrite = fwrite(&coursCD, sizeof(coursCD), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't write link course, cours_type:%d, cours_cd:%d", coursInfo.course_type, coursInfo.course_cd);
			return 0;
		}
		offItem += sizeof(coursCD);

		// write course link count
		cntItem = item.second.size();
		if ((retWrite = fwrite(&cntItem, sizeof(cntItem), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't write course link count, cours_type:%d, cours_cd:%d, count:%d", coursInfo.course_type, coursInfo.course_cd, cntItem);
			return 0;
		}
		offItem += sizeof(cntItem);

		// write course link id
		for (auto& link : item.second)
		{
			link_id.llid = link;

			if ((retWrite = fwrite(&link_id.llid, sizeof(link_id.llid), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't write course link, cours_type:%d, cours_cd:%d, link_tile:%d, link_id:%d", coursInfo.course_type, coursInfo.course_cd, link_id.tile_id, link_id.nid);
				return 0;
			}
			offItem += sizeof(link_id.llid);
		} // for - write course link id
		fileBody.cntData++;
	} // for - write course by code

	// re-write course body size & off
	if (offItem > sizeFileBody)
	{
		fileBody.szData = offItem;

		fseek(fp, offItem * -1, SEEK_CUR);

		if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't re-write course body data, idx:%d", idx);
			return 0;
		}

		fseek(fp, offItem - sizeFileBody, SEEK_CUR);
	}

	// re-set index size & off buff
	m_vtCourseIndex[idx].szBody = fileBody.szData;
	m_vtCourseIndex[idx].offBody = offFile;

	// update file offset
	offFile += offItem;




	// 2nd, write course by link
	idx++;
	memset(&fileBody, 0x00, sizeFileBody);
	fileBody.idTile = idx;

	if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't write course body, idx:%d", idx);
		return 0;
	}
	offItem = sizeFileBody;

	const unordered_map<uint64_t, set<uint32_t>>* pLinkData = m_mapCourse.GetCourseDataByLink();
	for (const auto& item : *pLinkData) {
		// mesh check
		link_id.llid = item.first;
		pMesh = GetMeshDataById(link_id.tile_id);
		if (!pMesh)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't access course link mesh, tile_id:%d, link_id:%d", link_id.tile_id, link_id.nid);
			return 0;
		}

		// write link id
		if ((retWrite = fwrite(&link_id.llid, sizeof(link_id.llid), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't write link course, tile_id:%d, link_id:%d", link_id.tile_id, link_id.nid);
			return 0;
		}
		offItem += sizeof(link_id.llid);

		// write link course count
		cntItem = item.second.size();
		if ((retWrite = fwrite(&cntItem, sizeof(cntItem), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't write link course count, tile_id:%d, link_id:%d, count:%d", link_id.tile_id, link_id.nid, cntItem);
			return 0;
		}
		offItem += sizeof(cntItem);

		// write link course cd
		for (auto& course : item.second)
		{
			if ((retWrite = fwrite(&course, sizeof(course), 1, fp)) != 1)
			{
				coursInfo.course_id = course;
				LOG_TRACE(LOG_ERROR, "Failed, can't write link course, tile_id:%d, link_id:%d, cours_type:%d, cours_cd:%d", link_id.tile_id, link_id.nid, coursInfo.course_type, coursInfo.course_cd);
				return 0;
			}
			offItem += sizeof(course);
		} // for - write link course cd
		fileBody.cntData++;
	} // for - write course by link

	// re-write course body size & off
	if (offItem > sizeFileBody)
	{
		fileBody.szData = offItem;

		fseek(fp, offItem * -1, SEEK_CUR);

		if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't re-write course body data, idx:%d", idx);
			return 0;
		}

		fseek(fp, offItem - sizeFileBody, SEEK_CUR);
	}

	// re-set index size & off buff
	m_vtCourseIndex[idx].szBody = fileBody.szData;
	m_vtCourseIndex[idx].offBody = offFile;

	// update file offset
	offFile += offItem;





	// re-write index for body size and offset
	fseek(fp, m_fileCourseHeader.offIndex, SEEK_SET);
	for (int idx = 0; idx < m_fileCourseHeader.cntIndex; idx++)
	{
		FileIndex fileIndex;
		memcpy(&fileIndex, &m_vtCourseIndex[idx], sizeof(fileIndex));
		if ((retWrite = fwrite(&fileIndex, sizeof(fileIndex), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't re-write file index data, idx:%d", idx);
			return 0;
		}
	}

	return offFile;
}


bool CFileForest::LoadData(IN const char* szFilePath)
{
	//char szFileName[MAX_PATH] = { 0, };
	//sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_TREKKING], g_szTypeExec[TYPE_EXEC_NETWORK]);

	bool ret = CFileBase::LoadData(szFilePath);

	if (ret == true) {
		ret = LoadCourseData(szFilePath);
	}

	return ret;
}


bool CFileForest::LoadCourseData(IN const char* szFilePath)
{
	size_t offFile = 0;
	size_t offItem = 0;
	size_t retRead = 0;

	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[m_nDataType], g_szTypeExec[TYPE_EXEC_COURSE]);

	// load course data
	FILE* fp = fopen(szFileName, "rb");
	if (!fp)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't open file for load course data, file:%s", szFileName);
		return false;
	}

	strcpy(m_szDataPath, szFileName);

	// read base
	retRead = ReadBase(fp);

	// check offset 
	if (retRead <= 0)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file base size zero");
		fclose(fp);
		return false;
	}


	// read course header
	retRead = ReadCourseHeader(fp);

	// check offset 
	if (retRead <= 0)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file course header size zero");
		fclose(fp);
		return false;
	}


	// read course index
	retRead = ReadCourseIndex(fp);

	// check offset 
	if (retRead <= 0)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file course index size zero");
		fclose(fp);
		return false;
	}

	// for cache
#if defined(USE_DATA_CACHE)
	//if (m_nFileType != TYPE_DATA_MESH) // 메쉬는 캐싱 인덱스이기에 바디를 읽어둠
	return true;
#endif

	// read course body
	retRead = ReadCourseBody(fp);

	// check offset 
	if (retRead <= 0)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file course body size zero");
		fclose(fp);
		return false;
	}

	fclose(fp);

	return true;
}


size_t CFileForest::ReadCourseHeader(FILE* fp)
{
	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeHeader = sizeof(m_fileCourseHeader);

	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	if ((retRead = fread(&m_fileCourseHeader, sizeHeader, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't read course header, cnt:%d", retRead);
		return 0;
	}

	offFile = sizeHeader;


	LOG_TRACE(LOG_DEBUG, "Read data, course header, cnt index:%d", m_fileCourseHeader.cntIndex);

	return offFile;
}


size_t CFileForest::ReadCourseIndex(FILE* fp)
{
	size_t offFile = 0;
	size_t retRead = 0;
	FileIndex fileIndex;
	const size_t sizeIndex = sizeof(fileIndex);

	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	fseek(fp, m_fileCourseHeader.offIndex, SEEK_SET);

	for (int idxTile = 0; idxTile < m_fileCourseHeader.cntIndex; idxTile++)
	{
		if ((retRead = fread(&fileIndex, sizeIndex, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read course index, idx:%d", idxTile);
			return 0;
		}

		m_vtCourseIndex.emplace_back(fileIndex);
		offFile += sizeIndex;
	}

	LOG_TRACE(LOG_DEBUG, "Read data, course index, type:%s, size:%d", GetDataTypeName(), offFile);

	return offFile;
}


size_t CFileForest::ReadCourseBody(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody = { 0, };

	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeBody = sizeof(FileBody);

	// read course body
	int idx = 0;
	int totalCourseByCode = 0;
	int totalCourseByLink = 0;;

#if defined(USE_MOUNTAIN_DATA)
	stCourseInfo courseInfo;
#endif
	uint32_t cntData;
	uint32_t courseCD;
	uint64_t linkID;

	// 1st, course by code
	if ((m_vtCourseIndex[idx].szBody > 0) && (m_vtCourseIndex[idx].offBody > 0)) 
	{
		fseek(fp, m_vtCourseIndex[idx].offBody, SEEK_SET);
		if ((retRead = fread(&fileBody, sizeBody, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read cousrse by code body, offset:%d", m_vtCourseIndex[idx].offBody);
			return 0;
		}

		// body
		for (int ii = 0; ii < fileBody.cntData; ii++)
		{
			// read course code
			if ((retRead = fread(&courseCD, sizeof(courseCD), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read course code, idx:%d", ii);
				return 0;
			}

			// read course link count
			if ((retRead = fread(&cntData, sizeof(cntData), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read course code count, course_cd:%d", courseCD);
				return 0;
			}

			// read course matching link
			for (int jj = 0; jj < cntData; jj++) {
				if ((retRead = fread(&linkID, sizeof(linkID), 1, fp)) != 1)
				{
					LOG_TRACE(LOG_ERROR, "Failed, can't read course link, course_cd:%d, idx:%d", courseCD, jj);
					return 0;
				}

				m_pDataMgr->AddLinkDataByCourse(courseCD, linkID);
			}

			totalCourseByCode++;
		}

		offFile += m_vtCourseIndex[idx].szBody;
#if defined(_DEBUG)
		//LOG_TRACE(LOG_DEBUG, "Data loading, course by code data loaded, cnt:%lld", totalCourseByCode);
#endif
	}
	
	// 2nd, course by link
	idx++;

	if ((m_vtCourseIndex[idx].szBody > 0) && (m_vtCourseIndex[idx].offBody > 0))
	{
		fseek(fp, m_vtCourseIndex[idx].offBody, SEEK_SET);
		if ((retRead = fread(&fileBody, sizeBody, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read cousrse by link body, offset:%d", m_vtCourseIndex[idx].offBody);
			return 0;
		}

		// body
		for (int ii = 0; ii < fileBody.cntData; ii++)
		{
			// read link id
			if ((retRead = fread(&linkID, sizeof(linkID), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read link id, idx:%d", ii);
				return 0;
			}

			// read link course count
			if ((retRead = fread(&cntData, sizeof(cntData), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read link course code count, link_id:%d", linkID);
				return 0;
			}

			// read link matching course
			set<uint32_t> courseSet;
			for (int jj = 0; jj < cntData; jj++) {
				if ((retRead = fread(&courseCD, sizeof(courseCD), 1, fp)) != 1)
				{
					LOG_TRACE(LOG_ERROR, "Failed, can't read course code, link_id:%d, idx:%d", jj);
					return 0;
				}

				m_pDataMgr->AddCourseDataByLink(linkID, courseCD);
			}

			totalCourseByLink++;
		} // for

		offFile += m_vtCourseIndex[idx].szBody;
#if defined(_DEBUG)
		//LOG_TRACE(LOG_DEBUG, "Data loading, course by link data loaded, cnt:%lld", totalCourseByLink);
#endif
	}


	LOG_TRACE(LOG_DEBUG, "Data loading, course by code cnt:%lld, course by link cnt:%lld", m_vtCourseIndex.size(), totalCourseByCode, totalCourseByLink);

	return offFile;
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


// 코스CD가 숫자만이 아닌 문자'-'섞여 있기에 '-'를 '0'으로 치환해 숫자로 사용,
// 현재까지는 '-'를 없애거나 '0'으로 치환해도 중복되는 값을 없을 것이라고 함(데이터팀), 2024-05-08 
int getCourseCD(IN char* course_cd)
{
	int ret_course_cd = 0;

	if ((course_cd != nullptr) && (strlen(course_cd) > 0))
	{
		char* ptr = strstr(course_cd, "-");
		if (ptr != nullptr) {
			
			char szPrev[64] = { 0, };
			strcpy(szPrev, course_cd);
			
			*ptr = '0';

			//LOG_TRACE(LOG_DEBUG, "course_id replace %s -> %s", szPrev, course_cd);
		}
		ret_course_cd = atoi(course_cd);
	}

	return ret_course_cd;
}


bool CFileForest::SetData_Link(int idx, stForestLink &getLink_Dbf, char* colData)
{
	bool ret = true;
	int32_t nValue = atoi(trim(colData));
	double fValue;

	switch (idx)
	{
	case 0:		getLink_Dbf.LinkID = nValue;		break;
	case 1:		getLink_Dbf.FromNodeID = nValue;	break;
	case 2:		getLink_Dbf.ToNodeID = nValue;		break;
	case 3:		// 코스타입, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
		if (nValue < 1 || TYPE_TRE_COUNT <= nValue) {
			ret = false; m_strErrMsg = "CourseType value not defined : " + string(colData);
		} else {
			getLink_Dbf.CourseType = nValue;
		} break;
	case 4:	// 코스 코드
		if (nValue == NOT_USE_LINK_DATA_CODE)
		{
			ret = false; m_strErrMsg = "Not use this link, cosrse_id: " + string(colData);
		} else if (nValue < 0 || 1048575 < nValue) {
			ret = false; m_strErrMsg = "CourseCode value not defined : " + string(colData);
		}
		if (nValue > 0) {
			getLink_Dbf.CourseCD = nValue;
			getLink_Dbf.ConnCourseType[getLink_Dbf.CourseCount] = getLink_Dbf.CourseType;
			getLink_Dbf.ConnCourseCD[getLink_Dbf.CourseCount] = getLink_Dbf.CourseCD;
			getLink_Dbf.CourseCount++;
		} break;
	case 5:	case 7: case 9: case 11: case 13: case 15: case 17: case 19: case 21: case 23: case 25: case 27: case 29: case 31: case 33: case 35: case 37:
		// 중용코스타입, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
		if (nValue < 0 || 7 < nValue) {
			ret = false; m_strErrMsg = "ConnCourseType type value not defined : " + string(colData);
		} else {
			getLink_Dbf.ConnCourseType[getLink_Dbf.CourseCount] = nValue;
		} break;
	case 6: case 8: case 10: case 12: case 14: case 16: case 18: case 20: case 22: case 24: case 26: case 28: case 30: case 32: case 34: case 36: case 38: 
		// 중용코스코드
		nValue = getCourseCD(colData);
		if (nValue < 0 || 1048575 < nValue) {
			ret = false; m_strErrMsg = "CourseCode value not defined : " + string(colData);
		} else if (nValue > 0) {
			getLink_Dbf.ConnCourseCD[getLink_Dbf.CourseCount] = nValue;
			getLink_Dbf.CourseCount++;
		} break;
	case 39: // 방면방향정보, 0:미정의, 1:정, 2:역
		if (nValue < 0 || 2 < nValue) {
			ret = false; m_strErrMsg = "CourseDir value not defined : " + string(colData);
		} else {
			getLink_Dbf.CourseDir = nValue;
		} break;
	case 40: // 진행방면정보 인덱스
		getLink_Dbf.CourseDirNameIDX = AddNameDic(colData);	break;
	case 41: case 42: case 43: case 44: case 45: case 46: // 노면정보, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드, 12:철구조물
		if (nValue < 0 || 12 < nValue) {
			ret = false; m_strErrMsg = "RoadInfo value not defined : " + string(colData);
		} else {
			getLink_Dbf.RoadInfo[idx - 33] = nValue;
		} break;
	case 47:	// 연결로 여부, 0:기타, 1:연결로
		if (nValue < 0 || 1 < nValue) {
			ret = false; m_strErrMsg = "ConnectType value not defined : " + string(colData);
		} else {
			getLink_Dbf.ConnectType = nValue;
		} break;
	case 48:	fValue = atof(trim(colData)); // 링크 길이
		if (fValue < 0 || /*200*/ 300 < fValue) {
			ret = false; m_strErrMsg = "LinkLen not defined : " + string(colData);
		} else {
			getLink_Dbf.LinkLen = fValue;
		} break;
	case 49:	// 0~100 난이도(숫자가 클수록 어려움)
		if (nValue < 0 || 100 < nValue) {
			ret = false; m_strErrMsg = "Diff value not defined : " + string(colData);
		} else {
			getLink_Dbf.Diff = nValue;
			if (maxDiff < nValue) maxDiff = nValue;
		} break;
	case 50:	// 경사도 +/- 127(버텍스 순서기준 정방향 경사도)
		//nValue /= 2; // 경사도가 +/- 127을 넘어 223까지 나오므로 2로 나눠쓰자 2024-02-21
		if (minSlop > nValue) minSlop = nValue;
		if (maxSlop < nValue) maxSlop = nValue;
		if (nValue < -100 || 100 < nValue) {
			m_strErrMsg = "Slop value not defined : " + string(colData);
			getLink_Dbf.Slop = 100; // 최소/최대 경사도는 100%로 제한함.
		} else {
			getLink_Dbf.Slop = nValue;
		} break;
	case 51:	// 정방향 이동 소요시간 (초), 0-1020
		if (nValue < 0 || 1023 < nValue) {
			ret = false; m_strErrMsg = "FwTime value not defined : " + string(colData);
		} else {
			getLink_Dbf.FwTime = nValue;
			if (maxFwTm < nValue) maxFwTm = nValue;
		} break;
	case 52:	// 역방향 이동 소요시간 (분->초), 0-1020
		if (nValue < 0 || 1023 < nValue) {
			ret = false; m_strErrMsg = "BwTime value not defined : " + string(colData);
		} else {
			getLink_Dbf.BwTime = nValue;
			if (maxBwTm < nValue) maxBwTm = nValue;
		} break;
	case 53:	// 인기도 지수, 0-4095
		nValue /= 2; // 인기도가 4095을 넘어 5636까지 나오므로 2로 나눠쓰자 2024-02-21
		if (nValue < 0 || 4095 < nValue) {
			ret = false; m_strErrMsg = "Popular value not defined : " + string(colData);
		} else {
			getLink_Dbf.Popular = nValue;
			if (maxPop < nValue) maxPop = nValue;
		} break;
	case 54:	// 여름/가을 시즌 통제기간 MMDDMMDD(시작일 MMDD + 종료일 MMDD)
		if (nValue < 0 || 99999999 < nValue) {
			ret = false; m_strErrMsg = "SFRestrict value not defined : " + string(colData);
		} else {
			getLink_Dbf.SFRestrict = nValue;
		} break;
	case 55:	// 여름/가을 시즌 통제사유, 0:기타, 1:산불방지, 2:휴식년, 3:정비, 4:사유지, 5:위험지역
		if (nValue < 0 || 5 < nValue) {
			ret = false; m_strErrMsg = "SFRestrictCause value not defined : " + string(colData);
		} else {
			getLink_Dbf.SFRestrictCause = nValue;
		} break;
	case 56:	// 겨울/봄 시즌 통제기간 MMDDMMDD(시작일 MMDD + 종료일 MMDD)
		if (nValue < 0 || 99999999 < nValue) {
			ret = false; m_strErrMsg = "WSRestrict value not defined : " + string(colData);
		} else {
			getLink_Dbf.WSRestrict = nValue;
		} break;
	case 57:	// 겨울/봄 시즌 통제사유, 0:기타, 1:산불방지, 2:휴식년, 3:정비, 4:사유지, 5:위험지역
		if (nValue < 0 || 5 < nValue) {
			ret = false; m_strErrMsg = "WSRestrictCause value not defined : " + string(colData);
		} else {
			getLink_Dbf.WSRestrictCause = nValue;
		} break;
	case 58:	// 산 코드 (5자리)
		if (nValue < 0 || 99999 < nValue) {
			ret = false; m_strErrMsg = "MntCD value not defined : " + string(colData);
		} else {
			getLink_Dbf.MntCD = nValue;
		} break;
	case 59:	// 입목존재코드, 0:비산림, 1:입목지, 2:무립목지
		if (nValue < 0 || 2 < nValue) {
			ret = false; m_strErrMsg = "Storunst value not defined : " + string(colData);
		} else {
			getLink_Dbf.Storunst = nValue;
		} break;
	case 60:	// 임종코드, 0:무립목지/비산림, 1:인공림, 2:천연림
		if (nValue < 0 || 2 < nValue) {
			ret = false; m_strErrMsg = "FrorCD value not defined : " + string(colData);
		} else {
			getLink_Dbf.FrorCD = nValue;
		} break;
	case 61:	// 임상코드, 0:무립목지/비산림, 1:침엽수림, 2:활엽수림, 3:혼효림, 4:죽림
		if (nValue < 0 || 4 < nValue) {
			ret = false; m_strErrMsg = "FrtpCD value not defined : " + string(colData);
		} else {
			getLink_Dbf.FrtpCD = nValue;
		} break;
	case 62:	// 수종그룹코드 10~99
		if (nValue < 0 || 99 < nValue) {
			ret = false; m_strErrMsg = "KoftrGrCD value not defined : " + string(colData);
		} else {
			getLink_Dbf.KoftrGrCD = nValue;
		} break;
	case 63:	// 경급코드, 0:치수, 1:소경목, 2:중경목, 3:대경목
		if (nValue < 0 || 3 < nValue) {
			ret = false; m_strErrMsg = "DmclsCD value not defined : " + string(colData);
		} else {
			getLink_Dbf.DmclsCD = nValue;
		} break;
	case 64:	// 영급코드, 0:미정의, 1:1영급~9:9영급
		if (nValue < 0 || 9 < nValue) {
			ret = false; m_strErrMsg = "AgclsCD value not defined : " + string(colData);
		} else {
			getLink_Dbf.AgclsCD = nValue;
		} break;
	case 65:	// 밀도코드,
		if (nValue < 0 || 9 < nValue) {
			ret = false; m_strErrMsg = "DnstCD value not defined : " + string(colData);
		}
		else {
			getLink_Dbf.DnstCD = nValue;
		} break;
	case 66:	// 법정탐방로 여부, 0:비법정, 1:법정
		if (nValue < 0 || 1 < nValue) {
			ret = false; m_strErrMsg = "LeGalEx value not defined : " + string(colData);
		} else {
			getLink_Dbf.LeGalEx = nValue;
		} break;
	case 67:	// 0:기타, 1:계곡, 2:강, 3:바다, 4:도시, 5:산, 6:숲, 7:섬, 8:공원
		if (nValue < 0 || 8 < nValue) {
			ret = false; m_strErrMsg = "Theme value not defined : " + string(colData);
		} else {
			getLink_Dbf.Theme = nValue;
		} break;
	case 68:	// 0:미확인, 1:통신가능, 2:통신불가
		if (nValue < 0 || 2 < nValue) {
			ret = false; m_strErrMsg = "SigSK value not defined : " + string(colData);
		} else {
			getLink_Dbf.SigSK = nValue;
		} break;
	case 69:	// 0:미확인, 1:통신가능, 2:통신불가
		if (nValue < 0 || 2 < nValue) {
			ret = false; m_strErrMsg = "SigKT value not defined : " + string(colData);
		} else {
			getLink_Dbf.SigKT = nValue;
		} break;
	case 70:	// 0:미확인, 1:통신가능, 2:통신불가
		if (nValue < 0 || 2 < nValue) {
			ret = false; m_strErrMsg = "SigLG value not defined : " + string(colData);
		} else {
			getLink_Dbf.SigLG = nValue;
		} break;
	case 71:	// 0:기타, 1 : 국립공원, 2 : 도립공원, 3 : 군립공원, 4 : 시립공원
		if (nValue < 0 || 4 < nValue) {
			ret = false; m_strErrMsg = "AreaType value not defined : " + string(colData);
		} else {
			getLink_Dbf.AreaType = nValue;
		} break;
	case 72:	// 0:기타, 1:국립공원공단, 2:행정안전부, 3:문화체육관광부, 4:산림청, 5:지자체, 6:체육회, 7:비영리단체
		if (nValue < 0 || 7 < nValue) {
			ret = false; m_strErrMsg = "ManaOrg value not defined : " + string(colData);
		} else {
			getLink_Dbf.ManaOrg = nValue;
		} break;
	case 73:	// DP
		if (nValue < 0 || 1 < nValue) {
			ret = false; m_strErrMsg = "Dp value not defined : " + string(colData);
		} else {
			getLink_Dbf.Dp = nValue;
		} break;
	case 74:	// MESH
		if (nValue < 0 || 1048575 < nValue) {
			ret = false; m_strErrMsg = "Mesh value not defined : " + string(colData);
		} else {
			getLink_Dbf.Mesh = nValue;
		} break;
	case 75:	// G_ID
		if (nValue < 0 || 4095 < nValue) {
			ret = false; m_strErrMsg = "Gid value not defined : " + string(colData);
		} else {
			getLink_Dbf.Gid = nValue;
		} break;
	case 76:	// GRADE
		if (nValue < 0 || 10 < nValue) {
			ret = false; m_strErrMsg = "Popular Grade value not defined : " + string(colData);
		} else {
			getLink_Dbf.Grade = nValue;
			if (maxGrade < nValue) maxGrade = nValue;
		} break;
	default:	break;
	}

	return ret;
}
