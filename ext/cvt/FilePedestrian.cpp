#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FilePedestrian.h"

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


CFilePedestrian::CFilePedestrian()
{
	m_nDataType = TYPE_DATA_PEDESTRIAN;
	m_nFileType = TYPE_EXEC_NETWORK;
}

CFilePedestrian::~CFilePedestrian()
{

}

bool CFilePedestrian::ParseData(IN const char* fname)
{
	XSHPReader shpReader;

	m_nLinkIdx = 0;
	m_nNodeIdx = 0;

	if (!shpReader.Open(fname)) {
		LOG_TRACE(LOG_ERROR, "Error, Cannot Find shp File!");
		return false;
	}

	//전체 지도영역 얻기
	shpReader.GetEnvelope((SBox*)GetMeshRegion());

	//전체 데이터 카운터 얻기
	long nRecCount = shpReader.GetRecordCount();
	int nFieldCount = shpReader.GetFieldCount();

	//int nLabelIdxToRead = 1;
	char chsTmp[61];
	int nPoint;
	SPoint* pPoints;
	int colCnt = shpReader.GetFieldCount();

	// 데이터 필드 무결성 확보를 위한 검사
	// 다른 데이터에도 추가해 주자
	if (strstr(fname, "NODE") != nullptr) {
		static char szLinkField[128][32] = {
			{ "U_ID" },{ "MESH_CD" },{ "NODE_ID" },{ "NODE_TYP" },{ "NODE_NM" },
			{ "CONN_CNT" },{ "EDGE_MESH" },{ "EDGE_NODE" },{ "FCT_INFO" },{ "FCT_PHASE" },
			{ "IO_INFO" },{ "IO_PHASE" },{ "ELEVATION" },
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
			{ "U_ID" },{ "MESH_CD" },{ "F_NODE_ID" },{ "T_NODE_ID" },{ "B_ROAD_TYP" },
			{ "W_ROAD_TYP" },{ "FCT_TYP" },{ "INOUT_TYP" },{ "LANE_CNT" },{ "ROAD_NM" },
			{ "LINK_LEN" },{ "SIDEWALK" },{ "W_CHARGE" },{ "B_CONTROL" },
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
	else if (nShpType == 3) {
		nShpType = 2; //선
		m_mapLink.reserve(nRecCount);
	}
	else if (nShpType == 1) {
		nShpType = 1; //점
		m_mapNode.reserve(nRecCount);
	}


	//데이터 얻기
	LOG_TRACE(LOG_DEBUG, "LOG, start, raw data parsing file : %s", fname);

	for (int idxRec = 0; idxRec < nRecCount; idxRec++) {
		stWalkMesh mesh;
		stWalkLink link;
		stWalkNode node;

		//속성가져오기
		if (shpReader.Fetch(idxRec) == false) { //error to read..
			LOG_TRACE(LOG_ERROR, "Error : Record %d is not available!!", idxRec);
			continue;
		}

		//Setting DBF Data
		for (int idxCol = 0; idxCol < colCnt; idxCol++) {
			memset(chsTmp, 0x00, sizeof(chsTmp));
			shpReader.GetDataAsString(idxCol, chsTmp, 60);  //nLabelIdxToRead 번째 필드 값을 chsTmp로 12byte 읽어옴.

			SHPGeometry *pSHPObj = shpReader.GetGeometry();

			if (nShpType == 1) {				// 점 일때				
				if (!SetData_Node(idxCol, node, chsTmp)) {
					LOG_TRACE(LOG_ERROR, "Error, %s", GetErrorMsg());
					return  false;
				}
				node.NodeCoord = pSHPObj->point.point;
			} else if (nShpType == 2) {		 // 선 일때
				if (!SetData_Link(idxCol, link, chsTmp)) {
					LOG_TRACE(LOG_ERROR, "Error, %s", GetErrorMsg());
					return  false;
				}
				if (idxCol == 0) {
					nPoint = SHP_GetNumPartPoints(pSHPObj, 0); //파트 0에 대한	   //선형 갯수..					 																	 
					pPoints = SHP_GetPartPoints(pSHPObj, 0); //파트 0에 대한	     //실제선형데이터..

					for (int idxObj = 0; idxObj < nPoint; idxObj++) {
						link.LinkVertex.emplace_back(pPoints[idxObj]);
					}
				}
			} else if (nShpType == 3) { // 면
				mesh.MeshID = atoi(trim(chsTmp));
				memcpy(&mesh.meshBox, &pSHPObj->mpoint.Box, sizeof(SBox));
			}
		} // for


		// 테스트 메쉬가 있으면 정의된 메쉬만 확인하자
		if (g_isUseTestMesh && !g_arrTestMesh.empty()) {
			if ((nShpType == 1) && checkTestMesh(node.MeshID)) {
				continue;
			} else if ((nShpType == 2) && checkTestMesh(link.MeshID)) {
				continue;
			}
		}


		if (nShpType == 1) {
			// NODE
			stNodeInfo* pNode = new stNodeInfo;

			pNode->node_id.tile_id = node.MeshID;
			pNode->node_id.nid = node.NodeID;
			pNode->edgenode_id.tile_id = node.AdjEdgeMesh;
			pNode->edgenode_id.nid = node.AdjEdgeNode;
			pNode->coord = node.NodeCoord;

			pNode->base.node_type = TYPE_NODE_DATA_PEDESTRIAN;
			pNode->base.point_type = node.NodeType;
			pNode->base.connnode_count = node.ConnectNum;

			pNode->ped.fct_phase = node.FctPhase;
			pNode->ped.gate_phase = node.GatePhase;

			pNode->ped.name_info = (node.NodeNameIDX == 0) ? 0 : 1;
			pNode->ped.fct_info = (node.FctNameIDX == 0) ? 0 : 1;
			pNode->ped.gate_info = (node.GateNameIDX == 0) ? 0 : 1;

#if defined(USE_FOREST_DATA) && defined(USE_PEDESTRIAN_DATA)
			// 진/출입구 명칭 인덱스 정보
			if ((pNode->ped.fct_info != 0) || (pNode->ped.gate_info != 0)) {
				stExtendInfo* pInfo = new stExtendInfo();
				pInfo->keyId = pNode->node_id;

				if (pNode->ped.fct_info != 0) {
					pInfo->vtType.emplace_back(TYPE_WNODEEX_FCT);
					pInfo->vtValue.emplace_back(node.FctNameIDX);
				}

				if (pNode->ped.gate_info != 0) {
					pInfo->vtType.emplace_back(TYPE_WNODEEX_IO);
					pInfo->vtValue.emplace_back(node.GateNameIDX);
				}

				m_pDataMgr->AddNodeExData(pInfo, TYPE_DATA_PEDESTRIAN);
			}
#endif

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

			pLink->link_id.tile_id = link.MeshID;
			pLink->link_id.nid = m_nLinkIdx++; // 링크 ID를 IDX로 변경
			pLink->snode_id.tile_id = link.MeshID;
			pLink->snode_id.nid = link.FromNodeID;
			pLink->enode_id.tile_id = link.MeshID;
			pLink->enode_id.nid = link.ToNodeID;

			pLink->length = link.LinkLen;
			//pLink->vtPts.assign(link.LinkVertex.begin(), link.LinkVertex.end());
			pLink->setVertex(&link.LinkVertex.front(), link.LinkVertex.size());

			pLink->name_idx = link.RoadNameIDX;

			pLink->base.link_type = TYPE_LINK_DATA_PEDESTRIAN;
			pLink->ped.bicycle_type = link.BicycleType;
			pLink->ped.walk_type = link.WalkType;
			pLink->ped.facility_type = link.FacilityType;
			pLink->ped.gate_type = link.GateType;
			pLink->ped.lane_count = link.LaneCount;
			pLink->ped.side_walk = link.SideWalk;
			pLink->ped.walk_charge = link.WalkCharge;
			pLink->ped.bicycle_control = link.BicycleControl;
			
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

bool CFilePedestrian::GenServiceData()
{
	unordered_map<uint64_t, uint32_t>::iterator itNodeIndex;
	map<uint32_t, stMeshInfo*>::iterator itMesh;
	unordered_map<uint64_t, stNodeInfo*>::iterator itNode;
	unordered_map<uint64_t, stLinkInfo*>::iterator itLink;
	bool findnode;

	if (m_mapNode.empty() || m_mapLink.empty()) {
		LOG_TRACE(LOG_ERROR, "LOG, start, link s/e node indexing");
		return false;
	}

	LOG_TRACE(LOG_DEBUG, "LOG, start, link s/e node indexing");

	LOG_TRACE(LOG_DEBUG, "LOG, start, node connected link indexing");

	for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++) {
		//// snode ID를 IDX로 변경
		//if ((itNodeIndex = m_mapNodeIndex.find(itLink->second->snode_id.llid)) != m_mapNodeIndex.end()) {
		//	itLink->second->snode_id.nid = itNodeIndex->second;

			// snode에 접속되는 인접 노드의 정보 업데이트
			findnode = false;
			if ((itNode = m_mapNode.find(itLink->second->snode_id.llid)) != m_mapNode.end()) {
				// 구획 변경점 상대 정보(메쉬)가 없을 경우
				if (itNode->second->base.point_type == TYPE_NODE_EDGE && itNode->second->base.connnode_count <= 0) {
					continue;
				}

				for (uint32_t ii = 0; ii < itNode->second->base.connnode_count; ii++)
				{
					if (itNode->second->connnodes[ii].llid == 0) {
						itNode->second->connnodes[ii] = itLink->second->link_id;

						findnode = true;
						break;
					}
				}

				if (!findnode)
				{
					LOG_TRACE(LOG_ERROR, "Failed, can't find connected node info what match with link snode, mesh:%d, link:%d, snode:%d", itLink->second->snode_id.tile_id, itLink->second->link_id.nid, itLink->second->snode_id.nid);
					continue;
				}
			}
			else {
				LOG_TRACE(LOG_ERROR, "Failed, can't find node info what match with link snode, mesh:%d, link:%d, snode:%d", itLink->second->snode_id.tile_id, itLink->second->link_id.nid, itLink->second->snode_id.nid);
				continue;
			}
	//	}
	//	else {
	//// 테스트 메쉬에서는 노드 끊김이 있을 수 있으니 무시하자
	//		if (!g_isUseTestMesh) {
	//			LOG_TRACE(LOG_ERROR, "Failed, can't find snode, mesh:%d, node:%d", itLink->second->snode_id.tile_id, itLink->second->enode_id.nid);
	//		}
	//		itLink->second->snode_id.llid = 0;
	//	}

		

		//// enode ID를 IDX로 변경
		//if ((itNodeIndex = m_mapNodeIndex.find(itLink->second->enode_id.llid)) != m_mapNodeIndex.end()) {
		//	itLink->second->enode_id.nid = itNodeIndex->second;

			// enode에 접속되는 노드의 접속 노드 정보 업데이트
			findnode = false;
			if ((itNode = m_mapNode.find(itLink->second->enode_id.llid)) != m_mapNode.end()) {
				// 구획 변경점 상대 정보(메쉬)가 없을 경우
				if (itNode->second->base.point_type == TYPE_NODE_EDGE && itNode->second->base.connnode_count <= 0) {
					continue;
				}

				for (uint32_t ii = 0; ii < itNode->second->base.connnode_count; ii++)
				{
					if (itNode->second->connnodes[ii].llid == 0) {
						itNode->second->connnodes[ii] = itLink->second->link_id;

						findnode = true;
						break;
					}
				}

				if (!findnode)
				{
					LOG_TRACE(LOG_ERROR, "Failed, can't find connected node info what match with link enode, mesh:%d, link:%d, enode:%d", itLink->second->enode_id.tile_id, itLink->second->link_id.nid, itLink->second->enode_id.nid);
					continue;
				}
			}
			if (!findnode)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't find node info what match with link enode, mesh:%d, link:%d, enode:%d", itLink->second->enode_id.tile_id, itLink->second->link_id.nid, itLink->second->enode_id.nid);
				continue;
			}
//		}
//		else {
//// 테스트 메쉬에서는 노드 끊김이 있을 수 있으니 무시하자
//			if (!g_isUseTestMesh) {
//				LOG_TRACE(LOG_ERROR, "Failed, can't find enode, mesh:%d, node:%d", itLink->second->enode_id.tile_id, itLink->second->enode_id.nid);
//			}
//			itLink->second->enode_id.llid = 0;
//		}
	} // for


	LOG_TRACE(LOG_DEBUG, "LOG, start, edge node indexing");

	for (itNode = m_mapNode.begin(); itNode != m_mapNode.end(); itNode++)
	{
		if (itNode->second->edgenode_id.llid <= 0) {
			continue;
		}

		// 구획 변교점 노드 ID를 IDX로 변경
		// 구획변경점을 기본 사용하기에 이제는 적용하지 않는다.
#if 0
		if ((itNodeIndex = m_mapNodeIndex.find(itNode->second->edgenode_id.llid)) != m_mapNodeIndex.end()) {
			itNode->second->edgenode_id.nid = itNodeIndex->second;
		}
		else {
			LOG_TRACE(LOG_ERROR, "Failed, can't find edge node, mesh:%d, node:%d", itNode->second->edgenode_id.tile_id, itNode->second->edgenode_id.nid);
			continue;
		}
#endif
	}

	

	// 인덱스 매칭
#if 0 // 인덱스로 관리 하지 않고, id를 그대로 사용하므로 인덱스 매칭 안함, 2024-01-09~ 
	LOG_TRACE(LOG_DEBUG, "LOG, start, link s/e node index matching");

	for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++)
	{
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
	} // for
#endif

	int32_t cntProc = 0;

	// 링크 결합, 구획변경점을 기본 사용하기에 이제는 링크 결합하지 않는다.
#if 0
	LOG_TRACE(LOG_DEBUG, "LOG, start, link merge");

	for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++)
	{
		MergeLink(itLink->second, &m_mapLink, &m_mapNode);

		if (++cntProc % g_cntLogPrint == 0) {
			LOG_TRACE(LOG_DEBUG, "LOG, processing, link merge: %d/%d", cntProc, m_mapLink.size());
		}
	}
#endif


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
			//if (itNode->second->node_id.tile_id != 153303 && itNode->second->node_id.tile_id != 153102)
				LOG_TRACE(LOG_ERROR, "Failed, add node data, mesh:%d, node:%d", itNode->second->node_id.tile_id, itNode->second->node_id.nid);

			//stMeshInfo* pMeshExt = new stMeshInfo;
			//pMeshExt->mesh_id.tile_id = itNode->second->node_id.tile_id;

			//if (AddMeshDataByNode(pMeshExt, itNode->second)) {
			//	LOG_TRACE(LOG_DEBUG, "add new mesh, when mesh info not exist, mesh:%d, node:%d", pMeshExt->mesh_id.tile_id, itNode->second->node_id.nid);
			//}
		}
	}


	LOG_TRACE(LOG_DEBUG, "LOG, start, add to link class");

	for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++)
	{
		if (!AddLinkData(itLink->second)) {
			//if (itLink->second->link_id.tile_id != 153303 && itNode->second->node_id.tile_id != 153102)
				LOG_TRACE(LOG_ERROR, "Failed, add link data, mesh:%d, link:%d", itLink->second->link_id.tile_id, itLink->second->link_id.nid);

			stMeshInfo* pMeshExt = new stMeshInfo;
			pMeshExt->mesh_id.tile_id = itLink->second->link_id.tile_id;

			//if (AddMeshDataByLink(pMeshExt, itLink->second)) {
			//	LOG_TRACE(LOG_DEBUG, "add new mesh, when mesh info not exist, mesh:%d, link:%d", pMeshExt->mesh_id.tile_id, itLink->second->link_id.nid);
			//}
		}
	}

#if defined(TEST_SPATIALINDEX)
	// 검색 트리 구성 필요
	m_pDataMgr->CreateSpatialindex(TYPE_DATA_PEDESTRIAN);
#endif

	LOG_TRACE(LOG_DEBUG, "LOG, finished");


	return CFileBase::GenServiceData();
}


void CFilePedestrian::AddDataFeild(IN const int idx, IN const int type, IN const char* colData)
{

}


void CFilePedestrian::AddDataRecord()
{

}


bool CFilePedestrian::Initialize()
{
	return CFileBase::Initialize();
}

void CFilePedestrian::Release()
{
	CFileBase::Release();
}


bool  CFilePedestrian::SetData_Node(int idx, stWalkNode &getNode_Dbf, char* colData)
{
	//uint32_t point_type : 4; // 노드 타입, 1:교차점, 2:단점, 3:더미점, 4:구획변경점, 5:속성변화점, 6:지하철진출입, 7:철도진출입, 8:지하도진출입, 9:지하상가진출입, 10:건물진출입
	//uint32_t connnode_count : 3; // 접속 노드 수
	//uint32_t facility_phase : 2; // 시설물 위상, 0:미정의, 1:지하, 2:지상, 3:지상위
	//uint32_t gate_phase : 2; // 입출구 위상, 0:미정의, 1:지하, 2:지상, 3:지상위

	bool ret = true;
	switch (idx)
	{
	case 1:		getNode_Dbf.MeshID = atoi(trim(colData));	break;
	case 2:		getNode_Dbf.NodeID = atoi(trim(colData));	break;		
	case 3:		getNode_Dbf.NodeType = atoi(trim(colData));
		if (getNode_Dbf.NodeType < 1 || 10 < getNode_Dbf.NodeType) {
			ret = false; m_strErrMsg = "node type value not defined : " + string(colData);
		} break;
	case 4:		getNode_Dbf.NodeNameIDX = AddNameDic(colData);	break;
	case 5:		getNode_Dbf.ConnectNum = atoi(trim(colData));
		if (getNode_Dbf.ConnectNum < 0 || 8 < getNode_Dbf.ConnectNum) {
			ret = false; m_strErrMsg = "connected node value not defined : " + string(colData);
		} break;
	case 6:		getNode_Dbf.AdjEdgeMesh = atoi(trim(colData));		break;
	case 7:		getNode_Dbf.AdjEdgeNode = atoi(trim(colData));		break;
	case 8:		getNode_Dbf.FctNameIDX = AddNameDic(colData);	break;
	case 9:		getNode_Dbf.FctPhase = atoi(trim(colData));
		if (getNode_Dbf.FctPhase < 0 || 3 < getNode_Dbf.FctPhase) {
			ret = false; m_strErrMsg = "facility phase value not defined : " + string(colData);
		} break;
	case 10:	getNode_Dbf.GateNameIDX = AddNameDic(colData);	break;
	case 11:	getNode_Dbf.GatePhase = atoi(trim(colData));
		if (getNode_Dbf.GatePhase < 0 || 3 < getNode_Dbf.GatePhase) {
			ret = false; m_strErrMsg = "gate phase value not defined : " + string(colData);
		} break;

	default:	break;
	}

	return ret;
}

bool CFilePedestrian::SetData_Link(int idx, stWalkLink &getLink_Dbf, char* colData)
{
	//uint32_t ped.bicycle_type : 2; // 자전거도로 타입, 1:자전거전용, 2:보행자/차량겸용 자전거도로, 3:보행도로
	//uint32_t ped.walk_type : 3; //보행자도로 타입, 1:복선도로, 2:차량겸용도로, 3:자전거전용도로, 4:보행전용도로, 5:가상보행도로
	//uint32_t ped.facility_type : 4; // 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
	//uint32_t ped.gate_type : 4; // 진입로 타입, 0:미정의, 1:경사로, 2:계단, 3:에스컬레이터, 4:계단/에스컬레이터, 5:엘리베이터, 6:단순연결로, 7:횡단보도, 8:무빙워크, 9:징검다리, 10:의사횡단
	//uint32_t ped.lane_count : 6; // 차선수, 63
	//uint32_t ped.side_walk : 2; // 인도(보도) 여부, 0:미조사, 1:있음, 2:없음
	//uint32_t ped.walk_charge : 2; // 보행자도로 유료 여부, 0:무료, 1:유료
	//uint32_t ped.bicycle_control : 2; // 자전거도로 규제 코드, 0:양방향, 1:정방향, 2:역방향, 3:통행불가
	
	bool ret = true;

	switch (idx)
	{
	case 0:		getLink_Dbf.LinkID = atoi(trim(&colData[6]));	break;
	case 1:		getLink_Dbf.MeshID = atoi(trim(colData));	break;
	case 2:		getLink_Dbf.FromNodeID = atoi(trim(colData));		break;
	case 3:		getLink_Dbf.ToNodeID = atoi(trim(colData));		break;
	case 4:		getLink_Dbf.BicycleType = atoi(trim(colData));	
		if (getLink_Dbf.BicycleType < 1 || 3 < getLink_Dbf.BicycleType) {
			ret = false; m_strErrMsg = "Bicycle type value not defined : " + string(colData);
		} break;
	case 5:		getLink_Dbf.WalkType = atoi(trim(colData));
		if (getLink_Dbf.WalkType < 1 || 5 < getLink_Dbf.WalkType) {
			ret = false; m_strErrMsg = "Walk type value not defined : " + string(colData);
		} break;
	case 6:		getLink_Dbf.FacilityType = atoi(trim(colData));
		if (getLink_Dbf.FacilityType < 0 || 15 < getLink_Dbf.FacilityType) {
			ret = false; m_strErrMsg = "facility type value not defined : " + string(colData);
		} break;
	case 7:		getLink_Dbf.GateType = atoi(trim(colData));
		if (getLink_Dbf.GateType < 0 || 10 < getLink_Dbf.GateType) {
			ret = false; m_strErrMsg = "gate type value not defined : " + string(colData);
		} break;
	case 8:		getLink_Dbf.LaneCount = atoi(trim(colData));
		if (getLink_Dbf.LaneCount < 0 || 32 < getLink_Dbf.LaneCount) {
			ret = false; m_strErrMsg = "lane count value not defined : " + string(colData);
		} break;
	case 9:		getLink_Dbf.RoadNameIDX = AddNameDic(colData);	break;
	case 10:	getLink_Dbf.LinkLen = atof(trim(colData));		break;
	case 11:	getLink_Dbf.SideWalk = atoi(trim(colData));
		if (getLink_Dbf.SideWalk < 0 || 2 < getLink_Dbf.SideWalk) {
			ret = false; m_strErrMsg = "side walk value not defined : " + string(colData);
		} break;
	case 12:	getLink_Dbf.WalkCharge = atoi(trim(colData));
		if (getLink_Dbf.WalkCharge < 0 || 1 < getLink_Dbf.WalkCharge) {
			ret = false; m_strErrMsg = "walk charge value not defined : " + string(colData);
		} break;
	case 13:	getLink_Dbf.BicycleControl = atoi(trim(colData));
		if (getLink_Dbf.BicycleControl < 0 || 3 < getLink_Dbf.BicycleControl) {
			ret = false; m_strErrMsg = "bicycle control value not defined : " + string(colData);
		} break;

	default:	break;
	}

	return ret;
}
