#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FileTrekking.h"

#include "../shp/shpio.h"
#include "../utils/UserLog.h"
#include "../utils/Strings.h"
#include "../route/MMPoint.hpp"

#include <algorithm>

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CFileTrekking::CFileTrekking()
{
	m_nFileType = TYPE_DATA_TREKKING;
}

CFileTrekking::~CFileTrekking()
{

}

static const uint32_t g_cntLogPrint = 100000;

bool CFileTrekking::ParseData(IN const char* fname)
{
	XSHPReader shpReader;

	m_nLinkIdx = 0;
	m_nNodeIdx = 0;

	if (!shpReader.Open(fname)) {
		LOG_TRACE(LOG_ERROR, "Error,Cannot Find shp File!");
		return false;
	}

	int nShpType = (int)shpReader.GetShpObjectType(); //shpPoint=1,shpPolyLine=3,shpPolygon=5

	if (nShpType == 5) nShpType = 3; //면
	else if (nShpType == 3) nShpType = 2; //선
	else if (nShpType == 1) nShpType = 1; //점

	//전체 지도영역 얻기
	shpReader.GetEnvelope((SBox*)GetMeshRegion());

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
			{ "MESH" },{ "NODE_ID" },{ "NODE_K" },{ "EDGE_MESH" },{ "EDGE_NODE" },
			{ "CN_NUM" },{ "CN_ID1" },{ "CN_ANGLE1" },{ "CN_ID2" },{ "CN_ANGLE2" },
			{ "CN_ID3" },{ "CN_ANGLE3" },{ "CN_ID4" },{ "CN_ANGLE4" },{ "CN_ID5" },
			{ "CN_ANGLE5" },{ "CN_ID6" },{ "CN_ANGLE6" },{ "CN_ID7" },{ "CN_ANGLE7" },
			{ "CN_ID8" },{ "CN_ANGLE8" },{ "Z" },
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
			{ "MESH_CD" },{ "LINK_ID" },{ "F_NODE_ID" },{ "T_NODE_ID" },{ "COURS_TYP" },
			{ "COURS_CD" },{ "SC_TYP1" },{ "SC_CD1" },{ "SC_TYP2" },{ "SC_CD2" },
			{ "SC_TYP3" },{ "SC_CD3" },{ "SC_TYP4" },{ "SC_CD4" },{ "SC_TYP5" },
			{ "SC_CD5" },{ "SC_TYP6" },{ "SC_CD6" },{ "SC_TYP7" },{ "SC_CD7" },
			{ "SC_TYP8" },{ "SC_CD8" },{ "DIR_CD" },{ "DIR_INFO" },{ "ROAD_INFO1" },
			{ "ROAD_INFO2" },{ "ROAD_INFO3" },{ "ROAD_INFO4" },{ "ROAD_INFO5" },{ "ROAD_INFO6" },
			{ "CONN" },{ "LINK_LEN" },{ "DIFF" },{ "FW_TM" },{ "RE_TM" },
			{ "POPULAR" },{ "SP_REST" },{ "AU_REST" },{ "VEG_TYP1" },{ "VEG_RATE1" },
			{ "VEG_TYP2" },{ "VEG_RATE2" },{ "MNT_NM" },
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
		stMesh mesh;
		stTrekkingLink link;
		stTrekkingNode node;

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
						link.LinkVertex.emplace_back(pPoints[idxObj]);
						//link.LinkVertex.push_back(pPoints[idxObj]);
					}
				}
			}
			else if (nShpType == 3) { // 면
				mesh.MeshID = atoi(trim(chsTmp));
				memcpy(&mesh.meshBox, &pSHPObj->mpoint.Box, sizeof(SBox));
			}
		} // for


#if defined(_USE_TEST_MESH)
		bool isContinue = true;
		for (const auto& item : g_arrTestMesh) {
			if ((nShpType == 1 && item == node.MeshID) || (nShpType == 2 && item == link.MeshID)) {
				isContinue = false;
				break;
			}
		}
		if (isContinue) continue;
#endif


		if (nShpType == 1) {
			// NODE
			stNodeInfo* pNode = new stNodeInfo;

			pNode->node_id.tile_id = node.MeshID;
			pNode->node_id.nid = node.NodeID;
			pNode->edgenode_id.tile_id = node.AdjEdgeMesh;
			pNode->edgenode_id.nid = node.AdjEdgeNode;
			pNode->coord = node.NodeCoord;

			pNode->base.node_type = TYPE_DATA_TREKKING;
			pNode->base.point_type = node.NodeType;
			pNode->base.connnode_count = node.ConnectNum;

			pNode->tre.z_value = node.Zvalue;
			//for (int ii = 0; ii < pNode->connnode_count; ii++)
			//{
			//	pNode->connnodes[ii].tile_id = node.MeshID;
			//	pNode->connnodes[ii].nid = node.ConnNode[ii];
			//}

			// 원래의 ID와 변경된 IDX 의 매칭 테이블
			m_mapNodeIndex.insert(pair<uint64_t, uint32_t>({ pNode->node_id.llid, m_nNodeIdx }));

			// 노드 ID를 IDX로 변경
			pNode->node_id.nid = m_nNodeIdx++; 

			static uint32_t s_nMaxConnNodeNum = 0;
			if (s_nMaxConnNodeNum < node.ConnectNum) {
				s_nMaxConnNodeNum = node.ConnectNum;

				LOG_TRACE(LOG_DEBUG, "LOG, node max connected num : %d", s_nMaxConnNodeNum);
			}

			m_mapNode.insert({ pNode->node_id.llid, pNode });

			if (m_nNodeIdx % g_cntLogPrint == 0) {
				LOG_TRACE(LOG_DEBUG, "LOG, node data processing, %lld / %lld", m_nNodeIdx, nRecCount);
			}
		}
		else if (nShpType == 2) {
			stLinkInfo* pLink = new stLinkInfo;

			pLink->link_id.tile_id = link.MeshID;
			pLink->link_id.nid = m_nLinkIdx++; // 링크 ID를 IDX로 변경
			pLink->snode_id.tile_id = link.MeshID;
			pLink->snode_id.nid = link.FromNodeID;
			pLink->enode_id.tile_id = link.MeshID;
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
						//if (link.RoadInfo[ii] == 5 ||
						//	link.RoadInfo[ii] == 6 ||
						//	link.RoadInfo[ii] == 7 ||
						//	link.RoadInfo[ii] == 8 ||
						//	link.RoadInfo[ii] == 9)
						//{
						//	int test = 0;
						//}
						pLink->tre.road_info |= (1 << (link.RoadInfo[ii] - 1));
					}
				}
			}
			else {
				pLink->tre.road_info = 0;
			}
			if (link.Diff > 0) {
				pLink->tre.diff = link.Diff;
			}
			else {
				pLink->tre.diff = 0;
			}
			pLink->tre.diff = link.Diff;
			if (link.Popular > 0) {
				pLink->tre.popular = link.Popular;
			}
			else {
				pLink->tre.popular = 0;
			}
			
			m_mapLink.insert({ pLink->link_id.llid, pLink });

			if (m_nLinkIdx % g_cntLogPrint == 0) {
				LOG_TRACE(LOG_DEBUG, "LOG, link data processing, %lld / %lld", m_nLinkIdx, nRecCount);
			}
		}
		//else if (nShpType == 3) {
		//	stMeshInfo* pMesh = new stMeshInfo;

		//	pMesh->mesh_id.tile_id = mesh.MeshID;
		//	memcpy(&pMesh->mesh_box, &mesh.meshBox, sizeof(SBox));

		//	m_mapMesh.insert(pair<uint32_t, stMeshInfo*>((uint32_t)pMesh->mesh_id.tile_id, pMesh));
		//}
	}

	shpReader.Close();

	if (!m_mapNode.empty() && !m_mapLink.empty()) {
		return GenServiceData();
	}

	return true;
}

bool CFileTrekking::GenServiceData()
{
	unordered_map<uint64_t, uint32_t>::iterator itNodeIndex;
	map<uint32_t, stMeshInfo*>::iterator itMesh;
	unordered_map<uint64_t, stNodeInfo*>::iterator itNode;
	unordered_map<uint64_t, stLinkInfo*>::iterator itLink;


	LOG_TRACE(LOG_DEBUG, "LOG, start, link s/e node indexing");
	LOG_TRACE(LOG_DEBUG, "LOG, start, node connected link indexing");

	for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++) {

		// snode ID를 IDX로 변경
		if ((itNodeIndex = m_mapNodeIndex.find(itLink->second->snode_id.llid)) != m_mapNodeIndex.end()) {
			itLink->second->snode_id.nid = itNodeIndex->second;

			// snode에 접속되는 인접 노드의 정보 업데이트
			bool findnode = false;
			if ((itNode = m_mapNode.find(itLink->second->snode_id.llid)) != m_mapNode.end()) {

				for (uint32_t ii = 0; ii < itNode->second->base.connnode_count; ii++)
				{
					if (itNode->second->connnodes[ii].llid == 0) {
						itNode->second->connnodes[ii] = itLink->second->link_id;

						findnode = true;
						break;
					}
				}
			}
			else {
				LOG_TRACE(LOG_ERROR, "Failed, can't find node info what match with link snode, mesh:%d, link:%d, snode:%d", itLink->second->snode_id.tile_id, itLink->second->link_id.nid, itLink->second->snode_id.nid);
				continue;
			}

			if (!findnode)
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

		

		// enode ID를 IDX로 변경
		if ((itNodeIndex = m_mapNodeIndex.find(itLink->second->enode_id.llid)) != m_mapNodeIndex.end()) {
			itLink->second->enode_id.nid = itNodeIndex->second;

			// enode에 접속되는 노드의 접속 노드 정보 업데이트
			bool findnode = false;
			if ((itNode = m_mapNode.find(itLink->second->enode_id.llid)) != m_mapNode.end()) {

				for (int ii = 0; ii < itNode->second->base.connnode_count; ii++)
				{
					if (itNode->second->connnodes[ii].llid == 0) {
						itNode->second->connnodes[ii] = itLink->second->link_id;

						findnode = true;
						break;
					}
				}
			}
			else {
				LOG_TRACE(LOG_ERROR, "Failed, can't find node info what match with link enode, mesh:%d, link:%d, enode:%d", itLink->second->enode_id.tile_id, itLink->second->link_id.nid, itLink->second->enode_id.nid);
				continue;
			}

			if (!findnode)
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



	LOG_TRACE(LOG_DEBUG, "LOG, start, edge node indexing");

	for (itNode = m_mapNode.begin(); itNode != m_mapNode.end(); itNode++)
	{
		if (itNode->second->edgenode_id.llid <= 0) {
			continue;
		}

		// 구획 변교점 노드 ID를 IDX로 변경
		if ((itNodeIndex = m_mapNodeIndex.find(itNode->second->edgenode_id.llid)) != m_mapNodeIndex.end()) {
			itNode->second->edgenode_id.nid = itNodeIndex->second;
		}
		else {
			LOG_TRACE(LOG_ERROR, "Failed, can't find edge node, mesh:%d, node:%d", itNode->second->edgenode_id.tile_id, itNode->second->edgenode_id.nid);
			continue;
		}
	}

	
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
	


	// 링크 결합
	LOG_TRACE(LOG_DEBUG, "LOG, start, link merge");

	int32_t cntProc = 0;
	for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++)
	{
		MergeLink(itLink->second, &m_mapLink, &m_mapNode);

		if (++cntProc % g_cntLogPrint == 0) {
			LOG_TRACE(LOG_DEBUG, "LOG, processing, link merge: %d/%d", cntProc, m_mapLink.size());
		}
	}



	// 각도 설정
	LOG_TRACE(LOG_DEBUG, "LOG, start, link angle");

	int cntLinkVtx;
	MMPoint<double> coord1;
	MMPoint<double> coord2;

	cntProc = 0;
	for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++)
	{
		cntLinkVtx = itLink->second->getVertexCount();

		// ang
		if (cntLinkVtx >= 2) {
			// snode ang
			coord1 = { itLink->second->getVertexX(0), itLink->second->getVertexY(0) };
			coord2 = { itLink->second->getVertexX(1), itLink->second->getVertexY(1) };
			itLink->second->base.snode_ang = coord1.azimuth(coord2);
			
			// enode ang
			coord1 = { itLink->second->getVertexX(cntLinkVtx - 1), itLink->second->getVertexY(cntLinkVtx - 1) };
			coord2 = { itLink->second->getVertexX(cntLinkVtx - 2), itLink->second->getVertexY(cntLinkVtx - 2) };
			itLink->second->base.enode_ang = coord1.azimuth(coord2);
		}

		if (++cntProc % g_cntLogPrint == 0) {
			LOG_TRACE(LOG_DEBUG, "LOG, processing, link angle: %d/%d", cntProc, m_mapLink.size());
		}
	}



	// 클래스에 추가

	LOG_TRACE(LOG_DEBUG, "LOG, start, add data to class");

	//for (itMesh = m_mapMesh.begin(); itMesh != m_mapMesh.end(); itMesh++) {
	//	if (!AddMeshData(itMesh->second)) {
	//		LOG_TRACE(LOG_ERROR, "Failed, add mesh data, mesh:%d", itMesh->second->mesh_id.tile_id);
	//	}
	//}


	LOG_TRACE(LOG_DEBUG, "LOG, start, add to node class");

	for (itNode = m_mapNode.begin(); itNode != m_mapNode.end(); itNode++)
	{
		if (!AddNodeData(itNode->second)) {
			if (itNode->second->node_id.tile_id != 153303 && itNode->second->node_id.tile_id != 153102)
				LOG_TRACE(LOG_ERROR, "Failed, add node data, mesh:%d, node:%d", itNode->second->node_id.tile_id, itNode->second->node_id.nid);

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
			if (itLink->second->link_id.tile_id != 153303 && itNode->second->node_id.tile_id != 153102)
				LOG_TRACE(LOG_ERROR, "Failed, add link data, mesh:%d, link:%d", itLink->second->link_id.tile_id, itLink->second->link_id.nid);

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


void CFileTrekking::AddDataFeild(IN const int idx, IN const int type, IN const char* colData)
{

}


void CFileTrekking::AddDataRecord()
{

}


bool CFileTrekking::Initialize()
{
	return CFileBase::Initialize();
}

void CFileTrekking::Release()
{
	CFileBase::Release();
}


bool CFileTrekking::OpenFile(IN const char* szFilePath)
{
	return CFileBase::OpenFile(szFilePath);
}


bool CFileTrekking::SaveData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_TREKKING], g_szTypeExec[TYPE_DATA_TREKKING]);

	return CFileBase::SaveData(szFileName);
}


bool CFileTrekking::LoadData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_TREKKING], g_szTypeExec[TYPE_DATA_TREKKING]);

	return CFileBase::LoadData(szFileName);
}


bool  CFileTrekking::SetData_Node(int idx, stTrekkingNode &getNode_Dbf, char* colData)
{
	// 노드 타입, 1:교차점, 2:단점, 3:더미점, 4:구획변경점, 5:속성변화점

	bool ret = true;
	switch (idx)
	{
	case 0:		getNode_Dbf.MeshID = atoi(trim(colData));	break;
	case 1:		getNode_Dbf.NodeID = atoi(trim(colData));	break;		
	case 2:		getNode_Dbf.NodeType = atoi(trim(colData));
		if (getNode_Dbf.NodeType < 1 || 4 < getNode_Dbf.NodeType) {
			ret = false; m_strErrMsg = "node type value not defined : " + string(colData);
		} break;
	case 3:		getNode_Dbf.AdjEdgeMesh = atoi(trim(colData));		break;
	case 4:		getNode_Dbf.AdjEdgeNode = atoi(trim(colData));		break;
	case 5:		getNode_Dbf.ConnectNum = atoi(trim(colData));
		if (getNode_Dbf.ConnectNum < 0 || 8 < getNode_Dbf.ConnectNum) {
			ret = false; m_strErrMsg = "connected node value not defined : " + string(colData);
		} break;
	case 22:	getNode_Dbf.Zvalue = atoi(trim(colData)); break;

		// 연결 노드 정보는 나중에 자동으로 찾아 넣도록 한다. -- 보행자 데이터와 동일하게 진행
	//case 6: case 8: case 10: case 12: case 14: case 16: case 18: case 20:
	//	getNode_Dbf.ConnNode[(idx - 6) / 2] = atoi(trim(colData));	break;
	//case 7: case 9: case 11: case 13: case 15: case 17: case 19: case 21:
	//	getNode_Dbf.ConnNodeAng[(idx - 7) / 2] = atoi(trim(colData)); break;

	default:	break;
	}

	return ret;
}

bool CFileTrekking::SetData_Link(int idx, stTrekkingLink &getLink_Dbf, char* colData)
{
	// 코스타입, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스

	bool ret = true;

	switch (idx)
	{
	case 0:		getLink_Dbf.MeshID = atoi(trim(colData));		break;
	case 1:		getLink_Dbf.LinkID = atoi(trim(colData));		break;
	case 2:		getLink_Dbf.FromNodeID = atoi(trim(colData));	break;
	case 3:		getLink_Dbf.ToNodeID = atoi(trim(colData));		break;
	case 4:		getLink_Dbf.CourseType = atoi(trim(colData));
		if (getLink_Dbf.CourseType < 1 || 4 < getLink_Dbf.CourseType) {
			ret = false; m_strErrMsg = "course type value not defined : " + string(colData);
		} break;
	case 5:		getLink_Dbf.CourseNameCD = atoi(trim(colData));	break;
	case 6:	case 8: case 10: case 12: case 14: case 16: case 18: case 20:
		getLink_Dbf.ConnCourseType[(idx - 6) / 2] = atoi(trim(colData));
		if (getLink_Dbf.ConnCourseType[(idx - 6) / 2] < 0 || 4 < getLink_Dbf.ConnCourseType[(idx - 6) / 2]) {
			ret = false; m_strErrMsg = "ConnCourseType type value not defined : " + string(colData);
		} break;
	case 7: case 9: case 11: case 13: case 15: case 17: case 19: case 21:
		getLink_Dbf.ConnCourseTypeNameIDX[(idx - 7) / 2] = atoi(trim(colData));	break;
	case 22:		getLink_Dbf.CourseDir = atoi(trim(colData));
		if (getLink_Dbf.CourseDir < 0 || 3 < getLink_Dbf.CourseDir) {
			ret = false; m_strErrMsg = "course dir value not defined : " + string(colData);
		} break;
	case 23:		getLink_Dbf.CourseDirNameIDX = atoi(trim(colData));	break;
	case 24: case 25: case 26: case 27: case 28: case 29:
		getLink_Dbf.RoadInfo[idx - 24] = atoi(trim(colData));		break;
	case 30:	getLink_Dbf.ConnectType = atoi(trim(colData));
		if (getLink_Dbf.ConnectType < 0 || 1 < getLink_Dbf.ConnectType) {
			ret = false; m_strErrMsg = "connect type value not defined : " + string(colData);
		} break;
	case 31:	getLink_Dbf.LinkLen = atof(trim(colData)); break;
	case 32:	getLink_Dbf.Diff = atoi(trim(colData));
		if (getLink_Dbf.Diff < 0 || 50 < getLink_Dbf.Diff) {
			ret = false; m_strErrMsg = "difficult value not defined : " + string(colData);
		} break;
	case 33:	getLink_Dbf.FwTime = atoi(trim(colData));
		if (getLink_Dbf.FwTime < 0 || 240 < getLink_Dbf.FwTime) {
			ret = false; m_strErrMsg = "forward travel value not defined : " + string(colData);
		} break;
	case 34:	getLink_Dbf.BwTime = atoi(trim(colData));
		if (getLink_Dbf.BwTime < 0 || 240 < getLink_Dbf.BwTime) {
			ret = false; m_strErrMsg = "back travel value not defined : " + string(colData);
		} break;
	case 35:	getLink_Dbf.Popular = atoi(trim(colData)); break;
		if (getLink_Dbf.Popular < 0 || 10 < getLink_Dbf.Popular) {
			ret = false; m_strErrMsg = "Popular value not defined : " + string(colData);
		} break;
	case 36:	getLink_Dbf.SFRestrict = atoi(trim(colData)); break;
		if (getLink_Dbf.SFRestrict < 0 || 12311231 < getLink_Dbf.SFRestrict) {
			ret = false; m_strErrMsg = "summer/fall travel restrict value not defined : " + string(colData);
		} break;
	case 37:	getLink_Dbf.WSRestrict = atoi(trim(colData)); break;
		if (getLink_Dbf.WSRestrict < 0 || 12311231 < getLink_Dbf.WSRestrict) {
			ret = false; m_strErrMsg = "winter/spring restrict value not defined : " + string(colData);
		} break;
	case 38: case 39:
		getLink_Dbf.VegType[idx - 38] = atoi(trim(colData)); break;
	case 40: case 41:
		getLink_Dbf.VegRate[idx - 40] = atoi(trim(colData)); break;
	case 42:	getLink_Dbf.MntNameIDX = AddNameDic(colData);	break;

	default:	break;
	}

	return ret;
}
