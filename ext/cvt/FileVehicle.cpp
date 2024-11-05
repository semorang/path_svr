#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FileVehicle.h"

#include "../shp/shpio.h"
#include "../utils/GeoTools.h"
#include "../utils/UserLog.h"
#include "../utils/Strings.h"
#include "../route/MMPoint.hpp"
#include "../route/DataManager.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CFileVehicle::CFileVehicle()
{
	m_nDataType = TYPE_DATA_VEHICLE;
	m_nFileType = TYPE_EXEC_NETWORK;
}

CFileVehicle::~CFileVehicle()
{

}

bool CFileVehicle::ParseData(IN const char* fname)
{
	XSHPReader shpReader;

	//m_nLinkIdx = 100000000;
	//m_nNodeIdx = 100000000;
	m_nLinkIdx = 0;
	m_nNodeIdx = 0;

	if (!shpReader.Open(fname)) {
		LOG_TRACE(LOG_ERROR, "Error, can't Find shp File: %s", fname);
		return false;
	}

#if defined(USE_PROJ4_LIB)
	initProj4("UTM52N");
#endif

	//전체 지도영역 얻기
	SBox mapBox;
	shpReader.GetEnvelope(&mapBox);

#if defined(USE_P2P_DATA) && defined(USE_PROJ4_LIB)
	translateUTM52NtoWGS84(mapBox.Xmin, mapBox.Ymin, mapBox.Xmin, mapBox.Ymin);
	translateUTM52NtoWGS84(mapBox.Xmax, mapBox.Ymax, mapBox.Xmax, mapBox.Ymax);
#endif

	SetMeshRegion(&mapBox);


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
	if (strstr(fname, "VLINK") != nullptr) {
#if defined(USE_P2P_DATA)
		static char szLinkField[128][29] = {
			{ "MESH" },{ "SNODE_ID" },{ "ENODE_ID" },{ "ROAD_K" },{ "ROAD_NUM" },
			{ "LANE_NUM" },{ "LINK_K" },{ "LEVEL" }, {"MAXSPEED"},{ "ROAD_DK1" },
			{ "ROAD_DK2" },{ "ROAD_DK3" },{ "PASS_CODE" },{ "K_CONTROL" },{ "EXPRESS" },
			{ "CHARGE" },{ "SAFE_Z" },{ "ROAD_NM" },{ "MAX_W" },{ "MAX_H" },
			{ "BRIDGE" },{ "TUNNEL" },{ "OVER_P" },{ "UNDER_P" },{ "HD_FLAG" },
			{ "LINK_LEN" },{ "TTLID_P" },{ "TTLID_N" },{ "REMARK" },
		};
#elif defined(USE_SAMSUNG_HEAVY)
		static char szLinkField[128][35] = {
			{ "SN" },{ "ID" },{ "Node1" },{ "Node2" },{ "관리자" },
			{ "도로종별" },{ "노선번호" },{ "중용1" },{ "중용번호1" },{ "중용2" },
			{ "중용번호2" },{ "중용3" },{ "중용번호3" },{ "중용4" },{ "중용번호4" },
			{ "중용5" },{ "중용번호5" },{ "중용6" },{ "중용번호6" },{ "유료" },
			{ "통행여부" },{ "일방" },{ "도로폭" },{ "차선수" },{ "링크종별" },
			{ "레벨치" },{ "링크내속성" },{ "링크길이" },{ "보간점수" },{ "전용" },
			{ "특수차" },{ "스쿨존" },{ "기타속성" },{ "속성명칭1" },{ "속성명칭2" },
		};
		//static char szNodeField[128][] = {
		//	{ "SN" },{ "ID" },{ "NUMBE" },{ "NAME1" },{ "노드종류" },
		//	{ "NAME1" },{ "NAME2" },{ "접속링크수" },{ "접속노드1" },{ "각도1" },
		//	{ "통행코드1" },{ "접속노드2" },{ "각도2" },{ "통행코드2" },{ "접속노드3" },
		//	{ "각도3" },{ "통행코드3" },{ "접속노드4" },{ "각도4" },{ "통행코드4" },
		//	{ "접속노드5" },{ "각도5" },{ "통행코드5" },{ "접속노드6" },{ "각도6" },
		//	{ "통행코드6" },{ "접속노드7" },{ "각도7" },{ "통행코드7" },{ "접속노드8" },
		//	{ "각도8" },{ "통행코드8" },
		//};
#elif defined(USE_OPTIMAL_POINT_API)
		static char szLinkField[128][32] = {
			{ "MESH" },{ "SNODE_ID" },{ "ENODE_ID" },{ "ROAD_K" },{ "ROAD_NUM" },
			{ "N_SR" },{ "SR_K1" },{ "SR_NUM1" },{ "SR_K2" },{ "SR_NUM2" },
			{ "SR_K3" },{ "SR_NUM3" },{ "LANE_NUM" },{ "LINK_K" },{ "LEVEL" },
			{ "ROAD_DK1" },{ "ROAD_DK2" },{ "ROAD_DK3" },{ "PASS_CODE" },{ "K_CONTROL" },
			{ "EXPRESS" },{ "CHARGE" },{ "SAFE_Z" },{ "ROAD_NM" },{ "TUNNEL" },
			{ "UNDER_P" },{ "LINK_LEN" },
		};
#else
		static char szLinkField[128][44] = {
			{ "MESH" },{ "SNODE_ID" },{ "ENODE_ID" },{ "ROAD_K" },{ "ROAD_NUM" },
			{ "N_SR" },{ "SR_K1" },{ "SR_NUM1" },{ "SR_K2" },{ "SR_NUM2" }, // 10
			{ "SR_K3" },{ "SR_NUM3" },{ "LANE_NUM" },{ "LINK_K" },{ "LEVEL" },
			{ "ROAD_DK1" },{ "ROAD_DK2" },{ "ROAD_DK3" },{ "PASS_CODE" },{ "K_CONTROL" }, // 20
			{ "EXPRESS" },{ "CHARGE" },{ "SAFE_Z" },{ "ROAD_NM" },{ "MAX_W" },
			{ "MAX_H" }, { "BRIDGE" },{ "TUNNEL" },{ "OVER_P" },{ "UNDER_P" }, // 30
			{ "BUS_ONLY" },{ "EXP_MD" },{ "EXP_MDIN" },{ "LANE_INFO" },{ "DIRECTION" },
			{ "IND_LANE" },{ "IND_DIR" },{ "TG_INFO" },{ "J_VIEW" },{ "TPEG" }, // 40
			{ "ENTRY_R" },{ "LINK_LEN" },{ "TTLID_P" },{ "TTLID_N" } // 44
		};
#endif

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

		// 여러 파일일 경우, 이전에 등록된 id에 이어서 증가
		if (m_pDataMgr) {
			m_nLinkIdx = m_pDataMgr->GetVLinkDataCount();
		}
	}
	else if (nShpType == 1) {
		nShpType = 1; //점
		m_mapNode.reserve(nRecCount);
	}


	//데이터 얻기
	LOG_TRACE(LOG_DEBUG, "LOG, start, raw data parsing file : %s, cnt : %d", fname, nRecCount);


	FILE* fpNodeLinkMatching = nullptr;
	char szNodeToLinkMatching[MAX_PATH] = { 0, };

	if (nShpType == 2) { // link
		// 중간 데이터 생성
		sprintf(szNodeToLinkMatching, "%s/%s", m_szWorkPath, "node-link-matching.bin");
		fpNodeLinkMatching = fopen(szNodeToLinkMatching, "wb");
		if (!fpNodeLinkMatching) {
			LOG_TRACE(LOG_ERROR, "Error, can't create node to link matching File!, %s", szNodeToLinkMatching);
			return false;
		}
	}

	for (int idxRec = 0; idxRec < nRecCount; idxRec++) {
		stWalkMesh mesh = { 0, };
		stVehicleLink link = { 0, };
		stVehicleNode node = { 0, };

		//속성가져오기
		if (shpReader.Fetch(idxRec) == false) { //error to read..
			LOG_TRACE(LOG_ERROR, "Error : Record %d is not available!!", idxRec);
			continue;
		}

#if	!defined(USE_SAMSUNG_HEAVY)
		// 테스트 메쉬가 있으면 정의된 메쉬만 확인하자
		if (g_isUseTestMesh && !g_arrTestMesh.empty()) {
			memset(chsTmp, 0x00, sizeof(chsTmp));
			shpReader.GetDataAsString(0, chsTmp, 60);  //nLabelIdxToRead 번째 필드 값을 chsTmp로 12byte 읽어옴.			
			int meshId = atoi(trim(chsTmp));
			if (g_arrTestMesh.find(meshId) == g_arrTestMesh.end()) {
				continue;
			}
		}
#endif


		//Setting DBF Data
		for (int idxCol = 0; idxCol < colCnt; idxCol++) {
			memset(chsTmp, 0x00, sizeof(chsTmp));
			shpReader.GetDataAsString(idxCol, chsTmp, 60);  //nLabelIdxToRead 번째 필드 값을 chsTmp로 12byte 읽어옴.

			SHPGeometry *pSHPObj = shpReader.GetGeometry();

			if (nShpType == 1) {				// 점 일때
				if (!SetData_Node(idxCol, node, chsTmp))
				{
					LOG_TRACE(LOG_ERROR, "Error, %s", GetErrorMsg());
					return  false;
				}
				if (idxCol == 0) {
#if defined(USE_P2P_DATA) && defined(USE_PROJ4_LIB)
					translateUTM52NtoWGS84(pSHPObj->point.point.x, pSHPObj->point.point.y, node.NodeCoord.x, node.NodeCoord.y);
#else
					node.NodeCoord = pSHPObj->point.point;
#endif
				}
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

					for (int idxObj = 0; idxObj < nPoint; idxObj++) {
#if defined(USE_P2P_DATA) && defined(USE_PROJ4_LIB)
						translateUTM52NtoWGS84(pPoints[idxObj].x, pPoints[idxObj].y, pPoints[idxObj].x, pPoints[idxObj].y);
#endif
						link.LinkVertex.emplace_back(pPoints[idxObj]);
					}
				}
			}
			else if (nShpType == 3) { // 면
				mesh.MeshID = atoi(trim(chsTmp));
#if defined(USE_P2P_DATA) && defined(USE_PROJ4_LIB)
				translateUTM52NtoWGS84(pSHPObj->mpoint.Box.Xmin, pSHPObj->mpoint.Box.Ymin, mesh.meshBox.Xmin, mesh.meshBox.Ymin);
				translateUTM52NtoWGS84(pSHPObj->mpoint.Box.Xmax, pSHPObj->mpoint.Box.Ymax, mesh.meshBox.Xmax, mesh.meshBox.Ymax);
#else
				memcpy(&mesh.meshBox, &pSHPObj->mpoint.Box, sizeof(SBox));
#endif			
			}
		} // for


		if (nShpType == 1) {
			// NODE
			stNodeInfo* pNode = new stNodeInfo;

			pNode->node_id.tile_id = node.MeshID;
			pNode->node_id.nid = node.NodeID;
			pNode->edgenode_id.tile_id = node.AdjEdgeMesh;
			pNode->edgenode_id.nid = node.AdjEdgeNode;
			pNode->coord = node.NodeCoord;
			pNode->name_idx = node.CrossNameIDX;

			pNode->base.node_type = TYPE_NODE_DATA_VEHICLE;
			pNode->base.point_type = node.NodeType;
			pNode->base.connnode_count = node.ConnectNum;

			// pass code
			memset(pNode->connnodes, 0, sizeof(pNode->connnodes));
			memset(pNode->conn_attr, -1, sizeof(pNode->conn_attr));
			for (int ii = 0; ii < node.ConnectNum; ii++) {
				pNode->connnodes[ii] = node.ConnectAttr[ii].NodeID;
				pNode->conn_attr[ii] = node.ConnectAttr[ii].PassCode;
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

			pLink->link_id.tile_id = link.MeshID;
			pLink->link_id.nid = m_nLinkIdx++; // 링크 ID를 IDX로 변경
			pLink->snode_id.tile_id = link.MeshID;
			pLink->snode_id.nid = link.SNodeID;
			pLink->enode_id.tile_id = link.MeshID;
			pLink->enode_id.nid = link.ENodeID;

			pLink->length = link.LinkLen;
			//pLink->vtPts.assign(link.LinkVertex.begin(), link.LinkVertex.end());
			pLink->setVertex(&link.LinkVertex.front(), link.LinkVertex.size());

			pLink->name_idx = link.RoadNameIDX;

			pLink->base.link_type = TYPE_LINK_DATA_VEHICLE;
			pLink->veh.road_type = link.RoadType;
			pLink->veh.lane_cnt = min((int)link.LaneCount, 15); // 최대 3bit
			pLink->veh.link_type = link.LinkType;
			// 최적지점에서는 단지폴리곤에 속한 단지내도로만 단지내도로로 인정
#if defined(USE_OPTIMAL_POINT_API)
			pLink->veh.link_dtype = link.DetailCode3 != 3 ? link.DetailCode3 : 0;
#else
			pLink->veh.link_dtype = link.DetailCode3; // 링크세부종별(dk3), 0:미정의, 1:고가도로,지하차도 옆길, 2:비포장도로, 3:단지내도로, 4:터널, 5:지하도로
#endif
			pLink->veh.level = link.Level;
			if (link.ControlCode == 2) {//  2:통행불가
				pLink->veh.pass_code = 2;
				pLink->link_id.dir = 3;
			} else if (link.ControlCode == 4) { // 4:일방통행_정
				pLink->veh.pass_code = 5;
				pLink->link_id.dir = 1;
			} else if (link.ControlCode == 5) { // 5:일방통행_역
				pLink->veh.pass_code = 6;
				pLink->link_id.dir = 2;
			}
			else {
				pLink->veh.pass_code = link.PassCode;
			}
			pLink->veh.car_only = link.CarOnly == 2 ? 0 : 1; // 자동차 전용
			pLink->veh.charge = link.Charge == 2 ? 0 : 1; // 유료도로
			pLink->veh.safe_zone = (link.SafeZone < 1 || 4 < link.SafeZone) ? 0 : link.SafeZone; // 보호구역
			pLink->veh.tunnel = link.Tunnel == 0 ? 0 : 1; // 터널
			pLink->veh.under_pass = link.UnderPass == 0 ? 0 : 1; // 지하차도

#if defined(USE_P2P_DATA)
			//pLink->sub_ext = (link.MeshID * 1000000000000) + (link.SNodeID * 1000000) + link.ENodeID; // 원본 ID 사용, (snode 6자리 + enode 6자리)

			pLink->veh.speed_f = pLink->veh.speed_b = link.MaxSpeed;
			pLink->veh.weight = link.MaxWeight;
			pLink->veh.height = link.MaxHeight;
			pLink->veh.bridge = link.Bridge;
			pLink->veh.over_pass = link.OverPass;
			pLink->veh.hd_flag = link.HdFlag;
#else			
			pLink->veh.weight = link.MaxWeight == 0 ? 0 : 1; // 높이 제한	
			pLink->veh.height = link.MaxHeight == 0 ? 0 : 1; // 무게 제한			
			pLink->veh.bridge = link.Bridge == 0 ? 0 : 1; // 교량			
			pLink->veh.under_pass = link.UnderPass == 0 ? 0 : 1; // 지하차도
			pLink->veh.toll = link.TollInfo == 0 ? 0 : 1; // 톨게이트
			pLink->veh.restriction = (link.EntryRestriction < 1 || 5 < link.EntryRestriction) ? 0 : link.EntryRestriction;// 진입 제한
#endif

			m_mapLink.insert({ pLink->link_id.llid, pLink });

			// 현재(2024-07) 까지는 7레벨 이하의 도로는 교통정보 미제공하므로 등록하지 말자
			if ((link.TTLID_P > 0 || link.TTLID_N > 0) && pLink->veh.level < 7) {
				// add ttl traffic data 
				int32_t ttl_nid = link.TTLID_P % TILEID_BYTE_COUNT;
				m_pDataMgr->AddTrafficTTLData(ttl_nid, pLink->link_id.tile_id, pLink->link_id.nid, DIR_POSITIVE); // TTL 정

				ttl_nid = link.TTLID_N % TILEID_BYTE_COUNT;
				m_pDataMgr->AddTrafficTTLData(ttl_nid, pLink->link_id.tile_id, pLink->link_id.nid, DIR_NAGATIVE); // TTL 역
			}


			// 중간 데이터 생성
			// mesh_id + snode_id + enode_id, link_nid
			if (fpNodeLinkMatching) {
				// write mesh_id + snode_id + enode_id
				sprintf(szNodeToLinkMatching, "%06d%06d%06d", pLink->link_id.tile_id, pLink->snode_id.nid, pLink->enode_id.nid);
				uint64_t nodeId = atoll(szNodeToLinkMatching);
				if (!fwrite(&nodeId, sizeof(nodeId), 1, fpNodeLinkMatching)) {
					LOG_TRACE(LOG_WARNING, "ERROR, node_id write failed for node_id to link_id matching file write, value:%s", szNodeToLinkMatching);
				} else if (!fwrite(&pLink->link_id.llid, sizeof(pLink->link_id.llid), 1, fpNodeLinkMatching)) {
					LOG_TRACE(LOG_WARNING, "ERROR, link_id write failed for node_id to link_id matching file write, value:%lld", pLink->link_id.llid);
				}
			}

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
	} // for

#if defined(USE_PROJ4_LIB)
	releaseProj4();
#endif

	if (fpNodeLinkMatching) {
		fclose(fpNodeLinkMatching);
	}

	shpReader.Close();

	if (!m_mapNode.empty() && !m_mapLink.empty()) {
		return GenServiceData();
	}

	return true;
}

bool CFileVehicle::GenServiceData()
{
	unordered_map<uint64_t, uint32_t>::iterator itNodeIndex;
	map<uint32_t, stMeshInfo*>::iterator itMesh;
	unordered_map<uint64_t, stNodeInfo*>::iterator itNode;
	unordered_map<uint64_t, stLinkInfo*>::iterator itLink;
	bool findnode;

	if (m_mapNode.empty() || m_mapLink.empty()) {
		LOG_TRACE(LOG_ERROR, "Failed, node link data empty, node cnt:%d, link cnt:%d", m_mapNode.size(), m_mapLink.size());
		return false;
	}

	LOG_TRACE(LOG_DEBUG, "LOG, start, link s/e node indexing");

	LOG_TRACE(LOG_DEBUG, "LOG, start, node connected link indexing");

	// 노드의 통행 코드를 링크에 적용
	// 노드의 통행코드가 노드ID로 저장되어 있어, 노드 ID를 IDX로 변경하기 전에 우선 진행해야 함
	for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++) {
		// 링크의 snode 통행코드
		findnode = false;

		if ((itNode = m_mapNode.find(itLink->second->snode_id.llid)) != m_mapNode.end()) {
			// 구획 변경점 상대 정보(메쉬)가 없을 경우
			if (itNode->second->base.point_type == TYPE_NODE_EDGE && itNode->second->base.connnode_count <= 0) {
				continue;
			}

			// 인접 노드의 정보를 링크 정보로 업데이트
			for (uint32_t ii = 0; ii < itNode->second->base.connnode_count; ii++)
			{
				// 반대편 노드와 매칭되는 인덱스의 패스 코드가 현재 링크의 패스 코드임.
				if (itNode->second->connnodes[ii].llid == itLink->second->enode_id.llid) {
					//itLink->second->veh.snode_pass = itNode->second->conn_attr[ii];
					itNode->second->connnodes[ii] = itLink->second->link_id;

					findnode = true;
					break;
				}
			} // for

			if (!findnode)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't find connected node info what match with link snode, mesh:%d, link:%d, snode:%d", itLink->second->link_id.tile_id, itLink->second->link_id.nid, itLink->second->snode_id.nid);
				continue;
			}
		}


		// 링크의 enode 통행코드
		findnode = false;

		if ((itNode = m_mapNode.find(itLink->second->enode_id.llid)) != m_mapNode.end()) {
			// 구획 변경점 상대 정보(메쉬)가 없을 경우
			if (itNode->second->base.point_type == TYPE_NODE_EDGE && itNode->second->base.connnode_count <= 0) {
				continue;
			}

			// 인접 노드의 정보를 링크 정보로 업데이트
			for (uint32_t ii = 0; ii < itNode->second->base.connnode_count; ii++)
			{
				// 반대편 노드와 매칭되는 인덱스의 패스 코드가 현재 링크의 패스 코드임.
				if (itNode->second->connnodes[ii].llid == itLink->second->snode_id.llid) {
					//itLink->second->veh.enode_pass = itNode->second->conn_attr[ii];
					itNode->second->connnodes[ii] = itLink->second->link_id;

					findnode = true;
					break;
				}
			} // for

			if (!findnode)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't find connected node info what match with link enode, mesh:%d, link:%d, enode:%d", itLink->second->link_id.tile_id, itLink->second->link_id.nid, itLink->second->enode_id.nid);
				continue;
			}
		}
	} // for


//	for (itLink = m_mapLink.begin(); itLink != m_mapLink.end(); itLink++) {
//
//		// snode ID를 IDX로 변경
//		//if ((itNodeIndex = m_mapNodeIndex.find(itLink->second->snode_id.llid)) != m_mapNodeIndex.end()) {
//		//	itLink->second->snode_id.nid = itNodeIndex->second;
//
//			// snode에 접속되는 인접 노드의 정보 업데이트
//			bool findnode = false;
//			if ((itNode = m_mapNode.find(itLink->second->snode_id.llid)) != m_mapNode.end()) {
//				
//				for (uint32_t ii = 0; ii < itNode->second->base.connnode_count; ii++)
//				{
//					if (itNode->second->connnodes[ii].llid == 0) {
//						itNode->second->connnodes[ii] = itLink->second->link_id;
//
//						findnode = true;
//						break;
//					}
//				}
//
//				if (!findnode)
//				{
//					LOG_TRACE(LOG_ERROR, "Failed, can't find connected node info what match with link snode, mesh:%d, link:%d, snode:%d", itLink->second->snode_id.tile_id, itLink->second->link_id.nid, itLink->second->snode_id.nid);
//					continue;
//				}
//			}
//			else {
//				LOG_TRACE(LOG_ERROR, "Failed, can't find node info what match with link snode, mesh:%d, link:%d, snode:%d", itLink->second->snode_id.tile_id, itLink->second->link_id.nid, itLink->second->snode_id.nid);
//				continue;
//			}
//	//	}
//	//	else {
//	//#if !defined(_USE_TEST_MESH)
//	//		LOG_TRACE(LOG_ERROR, "Failed, can't find snode, mesh:%d, node:%d", itLink->second->snode_id.tile_id, itLink->second->enode_id.nid);
//	//#endif
//	//		itLink->second->snode_id.llid = 0;
//	//	}
//
//		
//
//		// enode ID를 IDX로 변경
//		//if ((itNodeIndex = m_mapNodeIndex.find(itLink->second->enode_id.llid)) != m_mapNodeIndex.end()) {
//		//	itLink->second->enode_id.nid = itNodeIndex->second;
//
//			// enode에 접속되는 노드의 접속 노드 정보 업데이트
//			findnode = false;
//			if ((itNode = m_mapNode.find(itLink->second->enode_id.llid)) != m_mapNode.end()) {
//
//				for (uint32_t ii = 0; ii < itNode->second->base.connnode_count; ii++)
//				{
//					if (itNode->second->connnodes[ii].llid == 0) {
//						itNode->second->connnodes[ii] = itLink->second->link_id;
//
//						findnode = true;
//						break;
//					}
//				}
//
//				if (!findnode)
//				{
//					LOG_TRACE(LOG_ERROR, "Failed, can't find connected node info what match with link enode, mesh:%d, link:%d, enode:%d", itLink->second->enode_id.tile_id, itLink->second->link_id.nid, itLink->second->enode_id.nid);
//					continue;
//				}
//			}
//			if (!findnode)
//			{
//				LOG_TRACE(LOG_ERROR, "Failed, can't find node info what match with link enode, mesh:%d, link:%d, enode:%d", itLink->second->enode_id.tile_id, itLink->second->link_id.nid, itLink->second->enode_id.nid);
//				continue;
//			}
////		}
////		else {
////#if !defined(_USE_TEST_MESH)
////			LOG_TRACE(LOG_ERROR, "Failed, can't find enode, mesh:%d, node:%d", itLink->second->enode_id.tile_id, itLink->second->enode_id.nid);
////#endif
////			itLink->second->enode_id.llid = 0;
////		}
//	}

	LOG_TRACE(LOG_DEBUG, "LOG, start, edge node indexing");

	for (itNode = m_mapNode.begin(); itNode != m_mapNode.end(); itNode++)
	{
		if (itNode->second->edgenode_id.llid <= 0) {
			continue;
		}

		// 구획 변교점 노드 ID를 IDX로 변경
		//if ((itNodeIndex = m_mapNodeIndex.find(itNode->second->edgenode_id.llid)) != m_mapNodeIndex.end()) {
		//	itNode->second->edgenode_id.nid = itNodeIndex->second;
		//}
		//else {
		//	LOG_TRACE(LOG_WARNING, "Failed, can't find edge node, mesh:%d, node:%d", itNode->second->edgenode_id.tile_id, itNode->second->edgenode_id.nid);
		//	continue;
		//}
	} // for


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
			LOG_TRACE(LOG_WARNING, "Failed, can't find snode by link, mesh:%d, link:%d", itLink->second->link_id.tile_id, itLink->second->link_id.nid);
			continue;
		}
		else if (itNode->second->node_id.llid <= 0)
		{
			LOG_TRACE(LOG_WARNING, "Failed, snode not indexed, mesh:%d, link:%d", itNode->second->node_id.tile_id, itNode->second->node_id.nid);
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
			LOG_TRACE(LOG_WARNING, "Failed, can't find enode by link, mesh:%d, link:%d", itLink->second->link_id.tile_id, itLink->second->link_id.nid);
			continue;
		}
		else if (itNode->second->node_id.llid <= 0)
		{
			LOG_TRACE(LOG_WARNING, "Failed, enode not indexed, mesh:%d, link:%d", itNode->second->node_id.tile_id, itNode->second->node_id.nid);
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
	} // for

	if (cntReversLinkVtx > 0) {
		LOG_TRACE(LOG_DEBUG, "LOG, link vertex re-matching with s/e node total count:%d", cntReversLinkVtx);
	}


	int32_t cntProc = 0;

	// 링크 결합, 구획변경점을 기본 사용하기에 이제는 링크 결합하지 않는다.
#if 0 // !defined(USE_P2P_DATA)
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
#if defined(USE_P2P_DATA) // p2p는 각도 필드에 링크 속성을 확장해서 사용
			itLink->second->veh_ext.snode_ang = coord1.azimuth(coord2);
#else
			itLink->second->base.snode_ang = coord1.azimuth(coord2);
#endif

			// enode ang			
			coord1 = { itLink->second->getVertexX(cntLinkVtx - 1), itLink->second->getVertexY(cntLinkVtx - 1) };
			coord2 = { itLink->second->getVertexX(cntLinkVtx - 2), itLink->second->getVertexY(cntLinkVtx - 2) };
#if defined(USE_P2P_DATA) // p2p는 각도 필드에 링크 속성을 확장해서 사용
			itLink->second->veh_ext.enode_ang = coord1.azimuth(coord2);
#else
			itLink->second->base.enode_ang = coord1.azimuth(coord2);
#endif
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


void CFileVehicle::AddDataFeild(IN const int idx, IN const int type, IN const char* colData)
{

}


void CFileVehicle::AddDataRecord()
{

}


bool CFileVehicle::Initialize()
{
	return CFileBase::Initialize();
}


void CFileVehicle::Release()
{
	CFileBase::Release();
}


bool  CFileVehicle::SetData_Node(int idx, stVehicleNode &getNode_Dbf, char* colData)
{
	bool ret = true;
	switch (idx)
	{
#if	defined(USE_SAMSUNG_HEAVY)
	case 0:		break; // SN
	case 1:		getNode_Dbf.MeshID = atoi(trim(colData));	break; // ID --> MESH
	case 2:		getNode_Dbf.NodeID = atoi(trim(colData));	break; // NUMBE --> NODE_ID
	case 3:		getNode_Dbf.CrossNameIDX = AddNameDic(colData);	break; // NAME1 --> CROSSNAME
	case 4: {	trim(colData); // 노드종류 --> NODE_K
		if (strcmp(colData, "교차점") == 0) getNode_Dbf.NodeType = TYPE_NODE_COROSS; // 교차점
		else if (strcmp(colData, "통행금지점") == 0) getNode_Dbf.NodeType = TYPE_NODE_END; // 단점		
		else if (strcmp(colData, "속성변화점") == 0) getNode_Dbf.NodeType = TYPE_NODE_ATTRIBUTE; // 속성변화점
		else { ret = false; m_strErrMsg = "node type value not defined : " + string(colData); }
		} break;
	case 5:		break; // NAME1_1
	case 6:		break; // NAME2
	case 7:		getNode_Dbf.ConnectNum = atoi(trim(colData)); // 접속링크수 --> CN_NUM
		if (getNode_Dbf.ConnectNum < 0 || 8 < getNode_Dbf.ConnectNum) {
			ret = false; m_strErrMsg = "connected node count not defined : " + string(colData);
		} break;
	case 8:	case 11: case 14: case 17: case 20: case 23: case 26: case 29: // 접속노드1~8
		getNode_Dbf.ConnectAttr[(idx - 8) / 3].NodeID.tile_id = getNode_Dbf.MeshID;
		getNode_Dbf.ConnectAttr[(idx - 8) / 3].NodeID.nid = atoi(trim(colData)); break;
	case 9:	case 12: case 15: case 18: case 21: case 24: case 27: case 30: // 각도1~8
		getNode_Dbf.ConnectAttr[(idx - 10) / 3].Angle = atoi(trim(colData)); break;
	case 10: case 13: case 16: case 19: case 22: case 25: case 28: case 31: // 통행코드1~8
		//if (getNode_Dbf.NodeType == TYPE_NODE_END && idx == 10) { // 통행금지점-단점 ==> 유턴 허용 구간으로 정의
		//	strcpy(colData, "10000000");
		//}
		getNode_Dbf.ConnectAttr[(idx - 9) / 3].PassCode = setPassCode(trim(colData)); break;
#else
	case 0:		getNode_Dbf.MeshID = atoi(trim(colData));	break;
	case 1:		getNode_Dbf.NodeID = atoi(trim(colData));	break;
	case 2:		getNode_Dbf.NodeType = atoi(trim(colData));
		if (getNode_Dbf.NodeType < 1 || 10 < getNode_Dbf.NodeType) {
			ret = false; m_strErrMsg = "node type value not defined : " + string(colData);
		} break;
	case 3:		getNode_Dbf.AdjEdgeMesh = atoi(trim(colData));		break;
	case 4:		getNode_Dbf.AdjEdgeNode = atoi(trim(colData));		break;
	case 5:		getNode_Dbf.ConnectNum = atoi(trim(colData));
		if (getNode_Dbf.ConnectNum < 0 || 8 < getNode_Dbf.ConnectNum) {
			ret = false; m_strErrMsg = "connected node value not defined : " + string(colData);
		} break;
	case 6:	case 9: case 12: case 15: case 18: case 21: case 24: case 27:
		getNode_Dbf.ConnectAttr[(idx - 6) / 3].NodeID.tile_id = getNode_Dbf.MeshID;
		getNode_Dbf.ConnectAttr[(idx - 6) / 3].NodeID.nid = atoi(trim(colData)); break;
	case 7:	case 10: case 13: case 16: case 19: case 22: case 25: case 28:
		getNode_Dbf.ConnectAttr[(idx - 7) / 3].PassCode = setPassCode(trim(colData)); break;
	case 8:	case 11: case 14: case 17: case 20: case 23: case 26: case 29:
		getNode_Dbf.ConnectAttr[(idx - 8) / 3].Angle = atoi(trim(colData)); break;
	case 30:		getNode_Dbf.CrossNameIDX = AddNameDic(colData);	break;
#endif // #if	defined(USE_SAMSUNG_HEAVY)

	default:	break;
	}

	return ret;
}

bool CFileVehicle::SetData_Link(int idx, stVehicleLink &getLink_Dbf, char* colData)
{
	bool ret = true;

	switch (idx)
	{
#if defined(USE_P2P_DATA)
	case  0:		getLink_Dbf.MeshID = atoi(trim(colData));	break; // MESH
	case  1:		getLink_Dbf.SNodeID = atoi(trim(colData));		break; // SNODE_ID
	case  2:		getLink_Dbf.ENodeID = atoi(trim(colData));		break; // ENODE_ID
	case  3:		getLink_Dbf.RoadType = atoi(trim(colData));		break; // ROAD_K
	case  4:		getLink_Dbf.RoadNum = atoi(trim(colData));		break; // ROAD_NUM
	case  5:		getLink_Dbf.LaneCount = atoi(trim(colData)); //  // LANE_NUM
		if (getLink_Dbf.LaneCount < 0 || 32 < getLink_Dbf.LaneCount) {
			ret = false; m_strErrMsg = "lane count value wrong value : " + string(colData);
		} break;
	case  6:		getLink_Dbf.LinkType = atoi(trim(colData));		break; // LINK_K
	case  7:		getLink_Dbf.Level = atoi(trim(colData));		break; // LEVEL
	case  8:		getLink_Dbf.MaxSpeed = atoi(trim(colData));		break; // MAXSPEED
	case  9:		getLink_Dbf.DetailCode1 = atoi(trim(colData));		break; // ROAD_DK1
	case 10:		getLink_Dbf.DetailCode2 = atoi(trim(colData));		break; // ROAD_DK2
	case 11:		getLink_Dbf.DetailCode3 = atoi(trim(colData));		break; // ROAD_DK3
	case 12:		getLink_Dbf.PassCode = atoi(trim(colData));		break; // PASS_CODE
	case 13:		getLink_Dbf.ControlCode = atoi(trim(colData));		break; // K_CONTROL
	case 14:		getLink_Dbf.CarOnly = atoi(trim(colData));		break; // EXPRESS
	case 15:		getLink_Dbf.Charge = atoi(trim(colData));		break; // CHARGE
	case 16:		getLink_Dbf.SafeZone = atoi(trim(colData));		break; // SAFE_Z
	case 17:		getLink_Dbf.RoadNameIDX = AddNameDic(colData);	break; // ROAD_NM
	case 18:		getLink_Dbf.MaxWeight = atoi(trim(colData));		break; // MAX_W
	case 19:		getLink_Dbf.MaxHeight = atoi(trim(colData));		break; // MAX_H
	case 20:		getLink_Dbf.Bridge = atoi(trim(colData));		break; // BRIDGE
	case 21:		getLink_Dbf.Tunnel = atoi(trim(colData));		break; // TUNNEL
	case 22:		getLink_Dbf.OverPass = atoi(trim(colData));		break; // OVER_P
	case 23:		getLink_Dbf.UnderPass = atoi(trim(colData));		break; // UNDER_P
	case 24:		getLink_Dbf.HdFlag = atoi(trim(colData));		break; // HD_FLAG
	case 25:		getLink_Dbf.LinkLen = atoi(trim(colData));		break; // LINK_LEN
	case 26:		getLink_Dbf.TTLID_P = atoll(trim(colData));		break; // TTLID_P
	case 27:		getLink_Dbf.TTLID_N = atoll(trim(colData));		break; // TTLID_N
#elif defined(USE_SAMSUNG_HEAVY)
	case 0:		break; // SN
	case 1:		getLink_Dbf.MeshID = atoi(trim(colData));	break; // ID -> MESH
	case 2:		getLink_Dbf.SNodeID = atoi(trim(colData));		break; // Node1 -> SNODE_ID
	case 3:		getLink_Dbf.ENodeID = atoi(trim(colData));		break; // Node2 -> ENODE_ID
	case 4:		break; // 관리자
	case 5:		getLink_Dbf.RoadType = atoi(trim(colData));		break; // 도로종별 -> ROAD_K
	case 6:		getLink_Dbf.RoadNum = atoi(trim(colData));		break; // 노선번호 -> ROAD_NUM
	case 7:		getLink_Dbf.SubRoadType1 = atoi(trim(colData));		break; // 중용1 -> SR_K1
	case 8:		getLink_Dbf.SubRoadNum1 = atoi(trim(colData));		break; // 중용번호1 -> SRNUM1
	case 9:		getLink_Dbf.SubRoadType2 = atoi(trim(colData));		break; // 중용2 -> SR_K2
	case 10:		getLink_Dbf.SubRoadNum2 = atoi(trim(colData));		break; // 중용번호2 -> SRNUM2
	case 11:		getLink_Dbf.SubRoadType3 = atoi(trim(colData));		break; // 중용3 -> SR_K3
	case 12:		getLink_Dbf.SubRoadNum3 = atoi(trim(colData));		break; // 중용번호3 -> SRNUM3
	case 13:		break; // 중용4 -> SR_K4
	case 14:		break; // 중용번호4 -> SRNUM4
	case 15:		break; // 중용5 -> SR_K5
	case 16:		break; // 중용번호5 -> SRNUM5
	case 17:		break; // 중용6 -> SR_K6
	case 18:		break; // 중용번호6 -> SRNUM6
	case 19:		getLink_Dbf.Charge = atoi(trim(colData));		break; // 유료 -> CHARGE
	case 20:		getLink_Dbf.PassCode = atoi(trim(colData));		break; // 통행여부 -> PASS_CODE
	case 21:		getLink_Dbf.ControlCode = atoi(trim(colData));		break; // 일방 -> K_CONTROL
	case 22:		break; // 도로폭
	case 23:		getLink_Dbf.LaneCount = atoi(trim(colData));	break; // 차선수 --> LANE_NUM
	case 24:		getLink_Dbf.LinkType = atoi(trim(colData));	break; // 링크종별 --> LINK_K
	case 25:		getLink_Dbf.Level = atoi(trim(colData));		break; // 레벨치 --> LEVEL
	case 26:		break; // 링크내속성
	case 27:		getLink_Dbf.LinkLen = atof(trim(colData));		break; // 링크길이 --> LINK_LEN
	case 28:		break; // 보간점수 --> VERTEX_CNT
	case 29:		getLink_Dbf.CarOnly = atof(trim(colData));		break; // 전용 --> EXPRESS
	case 30:		break; // 특수차 --> 
	case 31:		getLink_Dbf.SafeZone = atof(trim(colData));		break; // 스쿨존 --> SAFE_Z
	case 32:		break; // 기타속성 --> 
	case 33:		break; // 속성명칭1 --> 
	case 34:		break; // 속성명칭2 --> 
#elif defined(USE_OPTIMAL_POINT_API)
	case 0:		getLink_Dbf.MeshID = atoi(trim(colData));	break;
	case 1:		getLink_Dbf.SNodeID = atoi(trim(colData));		break;
	case 2:		getLink_Dbf.ENodeID = atoi(trim(colData));		break;
	case 3:		getLink_Dbf.RoadType = atoi(trim(colData));		break;
	case 4:		getLink_Dbf.RoadNum = atoi(trim(colData));		break;
	case 5:		getLink_Dbf.SubRoadCount = atoi(trim(colData));		break;
	case 6:		getLink_Dbf.SubRoadType1 = atoi(trim(colData));		break;
	case 7:		getLink_Dbf.SubRoadNum1 = atoi(trim(colData));		break;
	case 8:		getLink_Dbf.SubRoadType2 = atoi(trim(colData));		break;
	case 9:		getLink_Dbf.SubRoadNum2 = atoi(trim(colData));		break;
	case 10:		getLink_Dbf.SubRoadType3 = atoi(trim(colData));		break;
	case 11:		getLink_Dbf.SubRoadNum3 = atoi(trim(colData));		break;
	case 12:		getLink_Dbf.LaneCount = atoi(trim(colData));	break;
	case 13:		getLink_Dbf.LinkType = atoi(trim(colData));		break;
	case 14:		getLink_Dbf.Level = atoi(trim(colData));		break;
	case 15:		getLink_Dbf.DetailCode1 = atoi(trim(colData));		break;
	case 16:		getLink_Dbf.DetailCode2 = atoi(trim(colData));		break;
	case 17:		getLink_Dbf.DetailCode3 = atoi(trim(colData));		break;
	case 18:		getLink_Dbf.PassCode = atoi(trim(colData));		break;
	case 19:		getLink_Dbf.ControlCode = atoi(trim(colData));		break;
	case 20:		getLink_Dbf.CarOnly = atoi(trim(colData));		break;
	case 21:		getLink_Dbf.Charge = atoi(trim(colData));		break;
	case 22:		getLink_Dbf.SafeZone = atoi(trim(colData));		break;
	case 23:		getLink_Dbf.RoadNameIDX = AddNameDic(colData);	break;
	case 24:		getLink_Dbf.Tunnel = atoi(trim(colData));		break;
	case 25:		getLink_Dbf.UnderPass = atoi(trim(colData));		break;
	case 26:		getLink_Dbf.LinkLen = atof(trim(colData));		break;
#else
	case 0:		getLink_Dbf.MeshID = atoi(trim(colData));	break;
	case 1:		getLink_Dbf.SNodeID = atoi(trim(colData));		break;
	case 2:		getLink_Dbf.ENodeID = atoi(trim(colData));		break;
	case 3:		getLink_Dbf.RoadType = atoi(trim(colData));		break;
	case 4:		getLink_Dbf.RoadNum = atoi(trim(colData));		break;
	case 5:		getLink_Dbf.SubRoadCount = atoi(trim(colData));		break;
	case 6:		getLink_Dbf.SubRoadType1 = atoi(trim(colData));		break;
	case 7:		getLink_Dbf.SubRoadNum1 = atoi(trim(colData));		break;
	case 8:		getLink_Dbf.SubRoadType2 = atoi(trim(colData));		break;
	case 9:		getLink_Dbf.SubRoadNum2 = atoi(trim(colData));		break;
	case 10:		getLink_Dbf.SubRoadType3 = atoi(trim(colData));		break;
	case 11:		getLink_Dbf.SubRoadNum3 = atoi(trim(colData));		break;
	case 12:		getLink_Dbf.LaneCount = atoi(trim(colData));	break;
	case 13:		getLink_Dbf.LinkType = atoi(trim(colData));		break;
	case 14:		getLink_Dbf.Level = atoi(trim(colData));		break;
	case 15:		getLink_Dbf.DetailCode1 = atoi(trim(colData));		break;
	case 16:		getLink_Dbf.DetailCode2 = atoi(trim(colData));		break;
	case 17:		getLink_Dbf.DetailCode3 = atoi(trim(colData));		break;
	case 18:		getLink_Dbf.PassCode = atoi(trim(colData));		break;
	case 19:		getLink_Dbf.ControlCode = atoi(trim(colData));		break;
	case 20:		getLink_Dbf.CarOnly = atoi(trim(colData));		break;
	case 21:		getLink_Dbf.Charge = atoi(trim(colData));		break;
	case 22:		getLink_Dbf.SafeZone = atoi(trim(colData));		break;
	case 23:		getLink_Dbf.RoadNameIDX = AddNameDic(colData);	break;

	// 아래로, 경로 최적화 차량 네트워크 v1.0.2
	case 24:		getLink_Dbf.MaxWeight = atoi(trim(colData));		break;
	case 25:		getLink_Dbf.MaxHeight = atoi(trim(colData));		break;
	case 26:		getLink_Dbf.Bridge = atoi(trim(colData));	break;
	case 27:		getLink_Dbf.Tunnel = atoi(trim(colData));		break;
	case 28:		getLink_Dbf.OverPass = atoi(trim(colData));		break;
	case 29:		getLink_Dbf.UnderPass = atoi(trim(colData));		break;
	case 30:		getLink_Dbf.BusOnly = atoi(trim(colData));		break;
	case 31:		getLink_Dbf.ExpressMode = atoi(trim(colData));		break;
	case 32:		getLink_Dbf.ExpressModeSub = atoi(trim(colData));		break;
	case 33:		getLink_Dbf.LaneInfo = atoi(trim(colData));		break;
	case 34:		getLink_Dbf.DirInfo = atoi(trim(colData));		break;
	case 35:		getLink_Dbf.IndicatorLane = atoi(trim(colData));		break;
	case 36:		getLink_Dbf.IndicatorDir = atoi(trim(colData));		break;
	case 37:		getLink_Dbf.TollInfo= atoi(trim(colData));		break;
	case 38:		getLink_Dbf.JunctionInfo = atoi(trim(colData));	break;
	case 39:		getLink_Dbf.Tpeg = atoi(trim(colData));		break;
	case 40:		getLink_Dbf.EntryRestriction = atoi(trim(colData));		break;
	case 41:		getLink_Dbf.LinkLen = atof(trim(colData));		break;
	case 42:		getLink_Dbf.TTLID_P = atoll(trim(colData));		break; //_tcstoui64_l(strSearch, NULL, 10, NULL) };
	case 43:		getLink_Dbf.TTLID_N = atoll(trim(colData));		break; //_tcstoui64_l(strSearch, NULL, 10, NULL) };
#endif // #if defined(USE_P2P_DATA)

	default:	break;
	}

	return ret;
}


// 8자리 통행 코드를 2비트씩 하위부터 채워넣는다
const uint16_t setPassCode(const char* szPassCode) {
	uint16_t ret = 0;

	if (szPassCode && strlen(szPassCode) == 8) {
		int valCode;
		char szCode[2] = { 0, };
		for (int ii = 0; ii < 8; ii++) {
			szCode[0] = szPassCode[ii];
			valCode = atoi(szCode);
			ret |= valCode << (ii * 2);
		}
	}

	return ret;
}


// 8자리 통행 코드에서 현재 인덱스의 통행 코드를 가져온다.
const uint16_t getPassCode(IN const uint16_t currentLinkPassCode, IN const int32_t idxCurrentPass)
{
	uint16_t passCode = 0;
	uint16_t passMask = 0;

	// 링크에 할당된 통행코드는 2비트씩 8방향의 통행 코드를 가지므로 현위치의 값을 가져올 수 있는 마스킹 값 구성
	passMask = (0x3) << (idxCurrentPass * 2);

	passCode = (currentLinkPassCode & passMask) >> (idxCurrentPass * 2);

	return passCode;
}


static const int maxProcessDepth = 3;

const int32_t getNextPassCode(IN const KeyID currentLinkId, IN const KeyID nextLinkId, IN const stNodeInfo* pNode, IN const int32_t processDepth, IN CDataManager* pDataMgr)
{
	int32_t retPass = PASS_CODE_DISABLE;

	if (pNode && pNode->base.point_type == TYPE_NODE_EDGE) {
		retPass = PASS_CODE_ENABLE;
	} else if (currentLinkId.llid != NULL_VALUE && nextLinkId.llid != NULL_VALUE && pNode) {
		// 링크의 통행 코드
		int16_t codeLinkPass = 0;

		// 링크의 통행코드에서 링크 자신의 인덱스 위치
		int32_t idxLinkPassOffset = 0;

		// 현재 링크의 통과 코드에서 요청된 idx의 통과 코드값 계산
		int32_t idxCurrentLinkPass = -1;
		int32_t idxNextLinkPass = -1;

		for (int ii = 0; ii < pNode->base.connnode_count; ii++) {
			if (pNode->connnodes[ii] == currentLinkId) {
				idxCurrentLinkPass = ii;
			}
			if (pNode->connnodes[ii] == nextLinkId) {
				idxNextLinkPass = ii;
			}
		}

		if (idxCurrentLinkPass < 0 || idxNextLinkPass < 0) {
			LOG_TRACE(LOG_WARNING, "can't find link matched pass, current link id:%lld%(%d,%d), next link id:%lld(%d,%d)", currentLinkId.llid, currentLinkId.tile_id, currentLinkId.nid, nextLinkId.llid, nextLinkId.tile_id, nextLinkId.nid);
		}
		else {
			// 통행코드는 자신의 현재 위치(링크) 다음 방향 값을 첫 인덱스를 시작해 자신을 가장 마지막 인덱스로 구성함.
			// 그러나 요청되는 다음 인덱스는 노드 연결 구성시 정북 기준 순으로 저장되므로 요청된 인덱스의 순서 변경이 필요함
			idxLinkPassOffset = (pNode->base.connnode_count - 1) - idxCurrentLinkPass;

			codeLinkPass = pNode->conn_attr[idxCurrentLinkPass];


			// 통행 코드 확인
			if (codeLinkPass == 0) {
				// 전방향 통과 가능
				retPass = PASS_CODE_ENABLE;
			}
			else { // if (codeLinkPass != 0) {
				   // 현재 링크 패스 인덱스로부터 다음 링크의 패스 인덱스 계산
				int idxCurrentPass = (idxLinkPassOffset + idxNextLinkPass);
				if (idxCurrentPass >= pNode->base.connnode_count) {
					idxCurrentPass %= (pNode->base.connnode_count);
				}

				retPass = getPassCode(codeLinkPass, idxCurrentPass);
			}
		}		
	}

	// 지정된 depth 만큼 추적
	if ((retPass != PASS_CODE_DISABLE) && (pDataMgr != nullptr) && (0 < processDepth) && (processDepth < maxProcessDepth)) {
		stLinkInfo* pNextLink = nullptr;
		stNodeInfo* pNextNode = nullptr;
		KeyID nextNodeId = { 0 };

		pNextLink = pDataMgr->GetVLinkDataById(nextLinkId);
		if (pNextLink != nullptr) {
			// 양방의 경우, 현재 링크 노드의 반대 노드 구하기
			// 일방의 경우에는 현재 링크가 다음 노드로 진입 가능한지 여부 확인
			if (pNextLink->veh.pass_code == 1) { // 양방향
				if (pNextLink->enode_id == pNode->node_id) {
					nextNodeId = pNextLink->snode_id;
				}
				else {
					nextNodeId = pNextLink->enode_id;
				}
			}
			else if ((pNextLink->veh.pass_code == 5) && (pNextLink->snode_id == pNode->node_id)) { // 정
				nextNodeId = pNextLink->enode_id;
			}
			else if ((pNextLink->veh.pass_code == 6) && (pNextLink->enode_id == pNode->node_id)) { // 역
				nextNodeId = pNextLink->snode_id;
			}
			else {
				retPass = PASS_CODE_DISABLE;
			}

			if (nextNodeId.llid != 0) {
				pNextNode = pDataMgr->GetVNodeDataById(nextNodeId);
			}
		}

		// 출발지의 방향성으로 설정시, 진출이 불가한 (다음 링크에서 회전 불가 등) 경우 확인
		if (pNextNode && (pNextNode->base.connnode_count >= 1)) {
			for (int ii = 0; ii < pNextNode->base.connnode_count; ii++) {
				if ((retPass = getNextPassCode(nextLinkId, pNextNode->connnodes[ii], pNextNode, processDepth + 1)) != PASS_CODE_DISABLE) {
					break;
				}
			} // for
		}
	}

	return retPass;
}


// 대상 노드의 링크가 진출 가능한지 확인 
const int32_t getNextPassCode(IN const stNodeInfo* pNode, IN const KeyID nextLinkId, IN CDataManager* pDataMgr)
{
	int32_t retPass = PASS_CODE_DISABLE;

	if (nextLinkId.llid != NULL_VALUE && pNode) {
		stLinkInfo* pNextLink = nullptr;

		for (int ii = 0; ii < pNode->base.connnode_count; ii++) {
			pNextLink = pDataMgr->GetLinkDataById(nextLinkId, pNode->base.node_type);

			if (pNextLink != nullptr) {
				// 양방의 경우, 현재 링크 노드의 반대 노드 구하기
				// 일방의 경우에는 현재 링크가 다음 노드로 진입 가능한지 여부 확인
				if (pNextLink->veh.pass_code == 1) { // 양방향
					retPass = PASS_CODE_ENABLE; //진출 가능
				}
				else if ((pNextLink->veh.pass_code == 5) && (pNextLink->snode_id == pNode->node_id)) { // 정
					retPass = PASS_CODE_ENABLE; //진출 가능
				}
				else if ((pNextLink->veh.pass_code == 6) && (pNextLink->enode_id == pNode->node_id)) { // 역
					retPass = PASS_CODE_ENABLE; //진출 가능
				}
				else {
					retPass = PASS_CODE_DISABLE;
				}
			}
		}
	}

	return retPass;
}


const int32_t getPrevPassCode(IN const KeyID currentLinkId, IN const KeyID prevLinkId, IN const stNodeInfo* pNode, IN const int32_t processDepth, IN CDataManager* pDataMgr)
{
	int32_t retPass = PASS_CODE_DISABLE;

	if (currentLinkId.llid != NULL_VALUE && prevLinkId.llid != NULL_VALUE && pNode) {
		// 링크의 통행 코드
		int16_t codeLinkPass = 0;

		// 링크의 통행코드에서 링크 자신의 인덱스 위치
		int32_t idxLinkPassOffset = 0;

		// 현재 링크의 통과 코드에서 요청된 idx의 통과 코드값 계산
		int32_t idxCurrentLinkPass = -1;
		int32_t idxPrevLinkPass = -1;

		for (int ii = 0; ii < pNode->base.connnode_count; ii++) {
			if (pNode->connnodes[ii] == prevLinkId) {
				idxPrevLinkPass = ii;
			}
			if (pNode->connnodes[ii] == currentLinkId) {
				idxCurrentLinkPass = ii;
			}
		}

		if (idxCurrentLinkPass < 0 || idxPrevLinkPass < 0) {
			LOG_TRACE(LOG_WARNING, "can't find link matched pass, current link id:%lld%(%d,%d), prev link id:%lld(%d,%d)", currentLinkId.llid, currentLinkId.tile_id, currentLinkId.nid, prevLinkId.llid, prevLinkId.tile_id, prevLinkId.nid);
		}
		else {
			// 통행코드는 자신의 현재 위치(링크) 다음 방향 값을 첫 인덱스를 시작해 자신을 가장 마지막 인덱스로 구성함.
			// 그러나 요청되는 다음 인덱스는 노드 연결 구성시 정북 기준 순으로 저장되므로 요청된 인덱스의 순서 변경이 필요함
			idxLinkPassOffset = (pNode->base.connnode_count - 1) - idxPrevLinkPass;

			codeLinkPass = pNode->conn_attr[idxPrevLinkPass];


			// 통행 코드 확인
			if (codeLinkPass == 0) {
				// 전방향 통과 가능
				retPass = PASS_CODE_ENABLE;
			}
			else { // if (codeLinkPass != 0) {
				   // 현재 링크 패스 인덱스로부터 다음 링크의 패스 인덱스 계산
				int idxCurrentPass = (idxLinkPassOffset + idxCurrentLinkPass);
				if (idxCurrentPass >= pNode->base.connnode_count) {
					idxCurrentPass %= (pNode->base.connnode_count);
				}

				retPass = getPassCode(codeLinkPass, idxCurrentPass);
			}
		}
	}

	// 지정된 depth 만큼 역 추적
	if ((retPass != PASS_CODE_DISABLE) && (pDataMgr != nullptr) && (0 < processDepth) && (processDepth < maxProcessDepth)) {
		stLinkInfo* pPrevLink = nullptr;
		stNodeInfo* pPrevNode = nullptr;
		KeyID prevNodeId = { 0 };

		pPrevLink = pDataMgr->GetVLinkDataById(prevLinkId);
		if (pPrevLink != nullptr) {			
			// 양방의 경우, 현재 링크 노드의 반대 노드 구하기
			// 일방의 경우에는 다음 링크가 현재 노드로 진입 가능한지 여부 확인
			if (pPrevLink->veh.pass_code == 1) { // 양방향
				if (pPrevLink->enode_id == pNode->node_id) {
					prevNodeId = pPrevLink->snode_id;
				} else {
					prevNodeId = pPrevLink->enode_id;
				}
			} else if ((pPrevLink->veh.pass_code == 5) && (pPrevLink->enode_id == pNode->node_id)) { // 정
				prevNodeId = pPrevLink->snode_id;
			} else if ((pPrevLink->veh.pass_code == 6) && (pPrevLink->snode_id == pNode->node_id)) { // 역
				prevNodeId = pPrevLink->enode_id;
			} else {
				retPass = PASS_CODE_DISABLE;
			}

			if (prevNodeId.llid != 0) {
				pPrevNode = pDataMgr->GetVNodeDataById(prevNodeId);
			}
		}
		
		// 목적지의 방향성으로 설정시, 진입이 불가한 (이전 링크에서 회전 불가 등) 경우 확인
		if (pPrevNode && (pPrevNode->base.connnode_count >= 1)) {
			for (int ii = 0; ii < pPrevNode->base.connnode_count; ii++) {
				if ((retPass = getPrevPassCode(prevLinkId, pPrevNode->connnodes[ii], pPrevNode, processDepth + 1)) != PASS_CODE_DISABLE) {
					break;
				}
			} // for
		}		
	}

	return retPass;
}


// 대상 노드로 이전 링크가 진입 가능한지 확인 
const int32_t getPrevPassCode(IN const stNodeInfo* pNode, IN const KeyID prevLinkId, IN CDataManager* pDataMgr)
{
	int32_t retPass = PASS_CODE_DISABLE;

	if (prevLinkId.llid != NULL_VALUE && pNode) {
		stLinkInfo* pPrevLink = nullptr;

		for (int ii = 0; ii < pNode->base.connnode_count; ii++) {
			pPrevLink = pDataMgr->GetLinkDataById(prevLinkId, pNode->base.node_type);

			if (pPrevLink != nullptr) {
				// 양방의 경우, 현재 링크 노드의 반대 노드 구하기
				// 일방의 경우에는 다음 링크가 현재 노드로 진입 가능한지 여부 확인
				if (pPrevLink->veh.pass_code == 1) { // 양방향
					retPass = PASS_CODE_ENABLE; //진입 가능
				}
				else if ((pPrevLink->veh.pass_code == 5) && (pPrevLink->enode_id == pNode->node_id)) { // 정
					retPass = PASS_CODE_ENABLE; //진입 가능
				}
				else if ((pPrevLink->veh.pass_code == 6) && (pPrevLink->snode_id == pNode->node_id)) { // 역
					retPass = PASS_CODE_ENABLE; //진입 가능
				}
				else {
					retPass = PASS_CODE_DISABLE; // 진입 불가
				}
			}
		}
	}

	return retPass;
}