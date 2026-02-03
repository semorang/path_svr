#if defined(_WINDOWS) || (_WIN32)
#include "../stdafx.h"
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "FileBase.h"
#include "FileName.h"

#include "../shp/shpio.h"
#include "../utils/UserLog.h"
#include "../utils/GeoTools.h"
#include "../utils/Strings.h"
#include "../route/MMPoint.hpp"
#include "../route/DataManager.h"

#include <queue>

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

CFileBase::CFileBase()
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	m_nDataType = -1;
	m_nFileType = -1;
	m_nLinkIdx = 0;
	m_nNodeIdx = 0;

	m_pDataMgr = nullptr;
	m_pNameMgr = nullptr;

	memset(&m_rtBox, 0x00, sizeof(m_rtBox));
	memset(&m_fileHeader, 0x00, sizeof(m_fileHeader));

	memset(&m_szDataPath, 0x00, sizeof(m_szDataPath));
	memset(&m_szSrcPath, 0x00, sizeof(m_szSrcPath));
	memset(&m_szWorkPath, 0x00, sizeof(m_szWorkPath));
	memset(&m_szDstPath, 0x00, sizeof(m_szDstPath));

//#if defined (USE_PEDESTRIAN_DATA)
//	m_trackShpMgr.SetFileManager(this);
//#endif
}

CFileBase::~CFileBase()
{
	if (!m_vtIndex.empty()) {
		vector<FileIndex>().swap(m_vtIndex);
		m_vtIndex.clear();
	}

	//_CrtDumpMemoryLeaks();
}


int32_t linkProjection(IN stLinkInfo* pData, IN const double& lng, IN const double& lat, IN const int32_t nMaxDist, OUT double& retLng, OUT double& retLat, OUT double& retDist, IN OUT double& retIr)
{
	int32_t idxMatchLine = -1;
	const stLinkInfo* pLink = pData;
	SPoint reqPoint = { lng, lat };
	double minDist = nMaxDist;
	double minLng, minLat, minIr;

	if (pLink == nullptr)
	{
		LOG_TRACE(LOG_ERROR, "Failed, param value null");
		return -1;
	}

	if (pLink->sub_info == NOT_USE || pLink->getVertexCount() <= 1)
	{
		LOG_TRACE(LOG_WARNING, "Failed, link not usable, type:%d, size:%d", pLink->sub_info, pLink->getVertexCount());
		return -1;
	}

	for (unsigned int idxVT = 0; idxVT < pLink->getVertexCount() - 1; ++idxVT) {
		minIr = 0.f;
		//SPoint sVertex = { pLink->vtPts[idxVT].x, pLink->vtPts[idxVT].y };
		//SPoint eVertex = { pLink->vtPts[idxVT + 1].x, pLink->vtPts[idxVT + 1].y };

		//bool isOk = LineCollision(sVertex, sVertex, reqCoord, reqCoord);
		//Point2T<double> ptResult = Perpendicular(sVertex, eVertex, ptTarget);
		getClosestPoint(pLink->getVertexX(idxVT), pLink->getVertexY(idxVT), pLink->getVertexX(idxVT + 1), pLink->getVertexY(idxVT + 1), lng, lat, &minLng, &minLat, &minIr);

		double dbDistance = getRealWorldDistance(lng, lat, minLng, minLat);

		if (dbDistance < minDist && 0 <= idxVT) {
			idxMatchLine = idxVT;
			minDist = dbDistance;

			retLng = minLng;
			retLat = minLat;
			retDist = minDist;
			retIr = minIr;
		}
	}

	return idxMatchLine;
}


void boxMerge(IN OUT SBox& lhs, IN const SBox& rhs)
{
	if (lhs.Xmin == 0.f || lhs.Xmin > rhs.Xmin) lhs.Xmin = rhs.Xmin;
	if (lhs.Ymin == 0.f || lhs.Ymin > rhs.Ymin) lhs.Ymin = rhs.Ymin;
	if (lhs.Xmax == 0.f || lhs.Xmax < rhs.Xmax) lhs.Xmax = rhs.Xmax;
	if (lhs.Ymax == 0.f || lhs.Ymax < rhs.Ymax) lhs.Ymax = rhs.Ymax;
}


enum
{
	CASE_MERGE_DEFAULT = 0,
	CASE_MERGE_TAIL_HEAD,
	CASE_MERGE_TAIL_TAIL,
	CASE_MERGE_HEAD_HEAD,
	CASE_MERGE_HEAD_TAIL
}; // 0:default, 1: tail + head, 2: tail + tail, 3: head + head, 4: head + tail


int linkMerge(IN OUT vector<SPoint>& lhs, IN const vector<SPoint>& rhs, IN const bool isAllowDuplicates)
{
	size_t lhsCnt = lhs.size();
	size_t rhsCnt = rhs.size();

	int mergeCase = CASE_MERGE_DEFAULT;

	if (lhsCnt <= 0 || rhsCnt <= 0) {
		mergeCase = CASE_MERGE_DEFAULT;
	}
	// 유턴 후 다시 지나가는 길이 있을 수 있으니 4가지 케이스 처리 우선 필요
	else if (lhs.at(0).equal(rhs.at(rhsCnt - 1)) && lhs.at(lhsCnt - 1).equal(rhs.at(0))) { // l-head + r-tail && l-tail + r-head ==> tail + head
		mergeCase = CASE_MERGE_TAIL_HEAD;
	} 
	else if (lhs.at(0).equal(rhs.at(0)) && lhs.at(lhsCnt - 1).equal(rhs.at(rhsCnt - 1))) { // l-head + r-head && l-tail + r-tail ==> tail + tail
		mergeCase = CASE_MERGE_TAIL_TAIL;
	} 
	else if (lhs.at(lhsCnt - 1).equal(rhs.at(rhsCnt - 1)) && lhs.at(0).equal(rhs.at(0))) { // l-tail + r-tail && l-head + r-head ==> head + head
		mergeCase = CASE_MERGE_HEAD_HEAD;
	} 
	else if (lhs.at(lhsCnt - 1).equal(rhs.at(0)) && lhs.at(0).equal(rhs.at(rhsCnt - 1))) { // l-tail + r-head && l-had + r-tail ==> head + tail
		mergeCase = CASE_MERGE_HEAD_TAIL;
	}
	// 회귀 모델이 될 수 있기에 tail with head/tail 확인 구문이 우선나와야 함.	
	else if (lhs.at(lhsCnt - 1).equal(rhs.at(0))) { // tail with head
		mergeCase = CASE_MERGE_TAIL_HEAD;
	}	
	else if (lhs.at(lhsCnt - 1).equal(rhs.at(rhsCnt - 1))) { // tail with tail
		mergeCase = CASE_MERGE_TAIL_TAIL;
	}	
	else if (lhs.at(0).equal(rhs.at(0))) { // head with head
		mergeCase = CASE_MERGE_HEAD_HEAD;
	}	
	else if (lhs.at(0).equal(rhs.at(rhsCnt - 1))) { // head with tail
		mergeCase = CASE_MERGE_HEAD_TAIL;
	} 
	else {
		mergeCase = CASE_MERGE_DEFAULT;
	}


	switch (mergeCase) {
	case CASE_MERGE_TAIL_HEAD: // tail with head
	{
		if (!isAllowDuplicates) {
			lhs.pop_back();
		}

		lhs.insert(lhs.end(), rhs.begin(), rhs.end());
	} break;
	case CASE_MERGE_TAIL_TAIL: // tail with tail
	{
		if (!isAllowDuplicates) {
			lhs.pop_back();
		}

		lhs.insert(lhs.end(), rhs.rbegin(), rhs.rend());
	} break;
	case CASE_MERGE_HEAD_HEAD: // head with head
	{
		if (!isAllowDuplicates) {
			lhs.erase(lhs.begin());
		}

		// 먼저 들어온놈을 뒤집고, 나중에 들어온 놈을 뒤에 연결, 2024-05-30
		vector<SPoint>ptTmp;
		ptTmp.assign(lhs.rbegin(), lhs.rend());
		ptTmp.insert(ptTmp.end(), rhs.begin(), rhs.end());

		lhs.swap(ptTmp);

		// 그전에는 반대로 함
		//vector<SPoint>ptTmp;
		//ptTmp.assign(rhs.rbegin(), rhs.rend());
		////for (int ii = rhsCnt - 1; ii >= 0; --ii)
		////{
		////	ptTmp.push_back(rhs[ii]);
		////}
		//ptTmp.insert(ptTmp.end(), lhs.begin(), lhs.end());
		////for (int ii = 0; ii < lhsCnt; ii++)
		////{
		////	ptTmp.push_back(lhs[ii]);
		////}
		//lhs.swap(ptTmp);
	} break;
	case CASE_MERGE_HEAD_TAIL: // head with tail
	{
		//lhs.reserve(lhs.size() + rhs.size());
		//for (int ii = 0; ii < lhsCnt; ii++)
		//{
		//	rhs.push_back(lhs[ii]);
		//}
		//lhs.swap(rhs);

		// 회귀 모델이 될 수 있기에 tail with head 확인 구문이 우선나와야 함.

		// 먼저 들어온놈을 뒤집고, 나중에 들어온 놈을 뒤에 연결, 2024-05-30
		if (!isAllowDuplicates) {
			lhs.erase(lhs.begin());
		}

		vector<SPoint>ptTmp;
		ptTmp.assign(lhs.rbegin(), lhs.rend());
		ptTmp.insert(ptTmp.end(), rhs.rbegin(), rhs.rend());

		lhs.swap(ptTmp);

		// 그전에는 반대로 함
		//vector<SPoint>ptTmp;
		////ptTmp.reserve(lhs.size() + rhs.size());

		//ptTmp.assign(rhs.begin(), rhs.end());
		//ptTmp.insert(ptTmp.end(), lhs.begin(), lhs.end());
		//lhs.swap(ptTmp);
	} break;
	default: // CASE_MERGE_DEFAULT
	{
		if (lhsCnt <= 0) {
			lhs.assign(rhs.begin(), rhs.end());
		} else if (rhsCnt <= 0) {
			// do not thing
		} else {
			LOG_TRACE(LOG_ERROR, "Failed, vertext merge match point not exist, begin: %.5f, %.5f, end: %.5f, %.5f", rhs.at(0).x, rhs.at(0).y, rhs.at(rhsCnt - 1).x, rhs.at(rhsCnt - 1).y);
			lhs.insert(lhs.end(), rhs.begin(), rhs.end());
			
			return -1;
		}
	}
	}; // switch

	return lhs.size();
}


int linkMerge(IN OUT vector<SPoint>& lhs, IN const SPoint * pData, IN const uint32_t cntData, IN const bool isAllowDuplicates)
{
	size_t lhsCnt = lhs.size();
	size_t rhsCnt = cntData;
	const SPoint* rhs = pData;

	int mergeCase = CASE_MERGE_DEFAULT; 

	if (lhsCnt <= 0 || rhsCnt <= 0) {
		mergeCase = CASE_MERGE_DEFAULT;
	}
	// 유턴 후 다시 지나가는 길이 있을 수 있으니 4가지 케이스 처리 우선 필요
	else if (lhs.at(0).equal(rhs[rhsCnt-1]) && lhs.at(lhsCnt-1).equal(rhs[0])) { // l-head + r-tail && l-tail + r-head ==> tail + head
		mergeCase = CASE_MERGE_TAIL_HEAD;
	} 
	else if (lhs.at(0).equal(rhs[0]) && lhs.at(lhsCnt-1).equal(rhs[rhsCnt-1])) { // l-head + r-head && l-tail + r-tail ==> tail + tail
		mergeCase = CASE_MERGE_TAIL_TAIL;
	} 
	else if (lhs.at(lhsCnt - 1).equal(rhs[rhsCnt - 1]) && lhs.at(0).equal(rhs[0])) { // l-tail + r-tail && l-head + r-head ==> head + head
		mergeCase = CASE_MERGE_HEAD_HEAD;
	} 
	else if (lhs.at(lhsCnt - 1).equal(rhs[0]) && lhs.at(0).equal(rhs[rhsCnt - 1])) { // l-tail + r-head && l-head + r-tail ==> head + tail
		mergeCase = CASE_MERGE_HEAD_TAIL;
	}
	// 회귀 모델이 될 수 있기에 tail with head/tail 확인 구문이 우선나와야 함.
	else if (lhs.at(lhsCnt - 1).equal(rhs[0])) { // tail with head
		mergeCase = CASE_MERGE_TAIL_HEAD;
	}	
	else if (lhs.at(lhsCnt - 1).equal(rhs[rhsCnt - 1])) { // tail with tail
		mergeCase = CASE_MERGE_TAIL_TAIL;
	}	
	else if (lhs.at(0).equal(rhs[0])) { // head with head
		mergeCase = CASE_MERGE_HEAD_HEAD;
	}	
	else if (lhs.at(0).equal(rhs[rhsCnt - 1])) { // head with tail
		mergeCase = CASE_MERGE_HEAD_TAIL;
	}
	else {
		mergeCase = CASE_MERGE_DEFAULT;
	}


	switch (mergeCase) {
	case CASE_MERGE_TAIL_HEAD: // tail with head
	{ 
		if (!isAllowDuplicates) {
			lhs.pop_back();
		}

		lhs.insert(lhs.end(), rhs, rhs + rhsCnt);
	} break;
	case CASE_MERGE_TAIL_TAIL: // tail with tail
	{ 
		//lhs.insert(lhs.end(), rhs.rbegin(), rhs.rend());
		if (!isAllowDuplicates) {
			lhs.pop_back();
		}

		for (int ii = rhsCnt - 1; ii >= 0; --ii) {
			lhs.emplace_back(rhs[ii]);
		}
	} break;
	case CASE_MERGE_HEAD_HEAD: // head with head
	{
		if (!isAllowDuplicates) {
			lhs.erase(lhs.begin());
		}

		vector<SPoint>ptTmp;
		//ptTmp.assign(rhs.rbegin(), rhs.rend());
		for (int ii = rhsCnt - 1; ii >= 0; --ii) {
			ptTmp.emplace_back(rhs[ii]);
		}
		ptTmp.insert(ptTmp.end(), lhs.begin(), lhs.end());

		lhs.swap(ptTmp);
	} break;
	case CASE_MERGE_HEAD_TAIL: // head with tail
	{
		//lhs.reserve(lhs.size() + rhs.size());
		//for (int ii = 0; ii < lhsCnt; ii++)
		//{
		//	rhs.push_back(lhs[ii]);
		//}
		//lhs.swap(rhs);

		if (!isAllowDuplicates) {
			lhs.erase(lhs.begin());
		}

		vector<SPoint>ptTmp;
		//ptTmp.reserve(lhs.size() + rhs.size());

		ptTmp.assign(rhs, rhs + rhsCnt);
		ptTmp.insert(ptTmp.end(), lhs.begin(), lhs.end());

		lhs.swap(ptTmp);
	} break;
	default: // CASE_MERGE_DEFAULT
	{
		if (lhsCnt <= 0) {
			lhs.assign(rhs, rhs + cntData);
		} else if (rhsCnt <= 0) {
			// do not thing
		} else {
			LOG_TRACE(LOG_ERROR, "Failed, vertext merge match point not exist, begin: %.5f, %.5f, end: %.5f, %.5f", rhs[0].x, rhs[0].y, rhs[rhsCnt - 1].x, rhs[rhsCnt - 1].y);
			lhs.insert(lhs.end(), rhs, rhs + rhsCnt);

			return -1;
		}
	}
	}; // switch

	return lhs.size();
}


// 입력 Id가 구획변교점 노드와 매칭되는 노드 검색
stNodeInfo* getEdgeNodeByEdgeNodeId(unordered_map<uint64_t, stNodeInfo*>* pMap, KeyID idNode)
{
	unordered_map<uint64_t, stNodeInfo*>::iterator itNode;

	if (pMap != nullptr && idNode.llid > 0) {
		if ((itNode = pMap->find(idNode.llid)) != pMap->end()) {
			if (itNode->second->sub_info != NOT_USE && itNode->second->base.point_type == TYPE_NODE_EDGE)
				return itNode->second;
		}
	}

	return nullptr;
}

// 입력 Idx가 구획변교점 노드와 매칭되는 노드 검색
stNodeInfo* getEdgeNodeByEdgeNodeIdx(unordered_map<uint64_t, stNodeInfo*>* pMap, KeyID idxNode)
{
	unordered_map<uint64_t, stNodeInfo*>::iterator itNode;

	for (itNode = pMap->begin(); itNode != pMap->end(); itNode++)
	{
		if (itNode->second->sub_info != NOT_USE && itNode->second->base.point_type == TYPE_NODE_EDGE && itNode->second->node_id.nid == idxNode.nid)
			return itNode->second;
	}

	return nullptr;
}

// 입력 Idx를 노드로 가진 노드 검색
stNodeInfo* getNodeByNodeIdx(unordered_map<uint64_t, stNodeInfo*>* pMap, KeyID idxNode)
{
	unordered_map<uint64_t, stNodeInfo*>::iterator itNode;

	for (itNode = pMap->begin(); itNode != pMap->end(); itNode++)
	{
		if (itNode->second->sub_info != NOT_USE && itNode->second->node_id.nid == idxNode.nid)
			return itNode->second;
	}

	return nullptr;
}

// 입력 노드 Idx를 인접 노드로 가진 노드 검색
stNodeInfo* getConnNodeByNodeIdx(unordered_map<uint64_t, stNodeInfo*>* pMap, KeyID idxNode, KeyID idxExpeptedNode)
{
	unordered_map<uint64_t, stNodeInfo*>::iterator itNode;

	for (itNode = pMap->begin(); itNode != pMap->end(); itNode++)
	{
		for (uint32_t ii = 0; ii < itNode->second->base.connnode_count; ii++)
		{
			if (itNode->second->sub_info != NOT_USE &&
				itNode->second->connnodes[ii] == idxNode &&
				itNode->second->connnodes[ii] != idxExpeptedNode)
				return itNode->second;
		}
	}

	return nullptr;
}

// 서로의 연결 노드 변경 (pPrevNode와 pNextNode를 연결)
// pNde 정보를 pNextNode에, pEdgeNode정보를 pPrevNode에 매칭
bool changeConnectedNodeIdx(stNodeInfo* pPrevNode, stNodeInfo* pNode, stNodeInfo* pEdgeNode, stNodeInfo* pNextNode)
{
	int idxPrev = -1;
	int idxNext = -1;

	// prev
	for (uint32_t ii = 0; ii < pPrevNode->base.connnode_count; ii++)
	{
		if (pPrevNode->connnodes[ii] == pNode->node_id) 
		{
			idxPrev = ii;
			break;
		}
	}

	// next
	for (uint32_t ii = 0; ii < pNextNode->base.connnode_count; ii++)
	{
		if (pNextNode->connnodes[ii] == pEdgeNode->node_id)
		{
			idxNext = ii;
			break;
		}
	}

	if (idxPrev >= 0 && idxNext >= 0)
	{
		pPrevNode->connnodes[idxPrev] = pNextNode->node_id;
		pNextNode->connnodes[idxNext] = pPrevNode->node_id;

		// 버림 표시
		pNode->sub_info = NOT_USE;
		pEdgeNode->sub_info = NOT_USE;

		return true;
	}

	return false;
}

// 입력된 노드 정보가 엣지 노드와 일치하는 노드 반환
stNodeInfo* getEdgeNodeByNodeIdx(unordered_map<uint64_t, stNodeInfo*>* pMap, uint32_t idxNode)
{
	unordered_map<uint64_t, stNodeInfo*>::iterator itNode;
	unordered_map<uint64_t, stLinkInfo*>::iterator itLink;

	for (itNode = pMap->begin(); itNode != pMap->end(); itNode++)
	{
		if (itNode->second->sub_info != NOT_USE && itNode->second->edgenode_id.llid == idxNode)
		{
			return itNode->second;
		}
	}

	return nullptr;
}

// 입력된 노드 ID를 가진 링크 검색
stLinkInfo* getLinkByNodeId(unordered_map<uint64_t, stLinkInfo*>* pMap, uint64_t idNode)
{
	unordered_map<uint64_t, stLinkInfo*>::iterator itLink;

	for (itLink = pMap->begin(); itLink != pMap->end(); itLink++)
	{
		// snode
		if (itLink->second->sub_info != NOT_USE && itLink->second->snode_id.llid == idNode)
		{
			return &*itLink->second;
		}

		// enode
		if (itLink->second->sub_info != NOT_USE && itLink->second->enode_id.llid == idNode)
		{
			return &*itLink->second;
		}
	}

	return nullptr;
}

// 입력된 노드 Idx를 가진 링크 검색
stLinkInfo* getLinkByNodeIdx(unordered_map<uint64_t, stLinkInfo*>* pMap, KeyID idxNode, KeyID idxNextNode)
{
	unordered_map<uint64_t, stLinkInfo*>::iterator itLink;

	for (itLink = pMap->begin(); itLink != pMap->end(); itLink++)
	{
		if (itLink->second->sub_info != NOT_USE && 
			itLink->second->snode_id == idxNode && itLink->second->enode_id == idxNextNode)
		{
			return &*itLink->second;
		}
		else if (itLink->second->sub_info != NOT_USE &&
			itLink->second->snode_id == idxNextNode && itLink->second->enode_id == idxNode)
		{
			return &*itLink->second;
		}
	}

	return nullptr;
}


// 구획 변교점 정보 변환
bool CFileBase::MergeEdgePoint(IN const int nLinkStartEnd, IN stLinkInfo* pLink, IN unordered_map<uint64_t, stLinkInfo*>* pMapLink, IN unordered_map<uint64_t, stNodeInfo*>* pMapNode)
{
	KeyID idxLinkNextNode = { 0, };
	KeyID idxLinkPrevNode = { 0, };
	unordered_map<uint64_t, stNodeInfo*>::iterator itNode;
	unordered_map<uint64_t, stLinkInfo*>::iterator itLink;
	stNodeInfo* pNode = nullptr;

	if (nLinkStartEnd == 1) // snode
	{
		idxLinkNextNode = pLink->snode_id;
		idxLinkPrevNode = pLink->enode_id;
	}
	else // enode
	{
		idxLinkNextNode = pLink->enode_id;
		idxLinkPrevNode = pLink->snode_id;
	}

	// 링크 접속 노드 정보
	//pNode = getNodeByNodeIdx(pMapNode, idxLinkNextNode);
	itNode = pMapNode->find(idxLinkNextNode.llid);
	//if (pNode == nullptr)
	if (itNode == pMapNode->end())
	{
		LOG_TRACE(LOG_WARNING, "Failed, can't find connected node, idx:%d, mesh:%d, node:%d", idxLinkNextNode, pLink->link_id.tile_id, pLink->link_id.nid);
		return false;
	}
	pNode = itNode->second;

	// 1.교차첨, 2.단점
	if (pNode->base.point_type == TYPE_NODE_COROSS || pNode->base.point_type == TYPE_NODE_END || pNode->base.point_type == TYPE_NODE_DUMMY || pNode->sub_info == NOT_USE)
	{
		;
	}
	// 3.더미점
	else if (0)// (pNode->point_type == TYPE_NODE_DUMMY)
	{
		// 인접 노드 갯수가 2개 이하인것만
		if (pNode->base.connnode_count > 2)
		{
			LOG_TRACE(LOG_WARNING, "Warning, too many conned node count for attribute change point, count:%d", pNode->base.connnode_count);
			return false;
		}



		// 해당 더미점 노드를 인접 노드로 가진 노드 검색
		stNodeInfo* pNextNode = getConnNodeByNodeIdx(pMapNode, pNode->node_id, idxLinkPrevNode);
		if (pNextNode == nullptr)
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find dummy next node, idx:%d", pNode->node_id.nid);
			return false;
		}

		// 위 노드를 노드 s/e로 가진 가진 링크 검색
		stLinkInfo* pNextLink = getLinkByNodeIdx(pMapLink, pNode->node_id, pNextNode->node_id);
		if (pNextLink == nullptr)
		{
			LOG_TRACE(LOG_WARNING, "Failed, find dummy next link, mesh:%d, node:%d", pNextNode->node_id.tile_id, pNextNode->node_id.nid);
			return false;
		}

		// 링크 반대쪽 노드 정보       
		stNodeInfo* pPrevNode = getNodeByNodeIdx(pMapNode, idxLinkPrevNode);
		if (pPrevNode == nullptr)
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find prev node, idx:%d", idxLinkPrevNode);
			return false;
		}

		// 서로 다른 타입이면 유지
		if (pLink->sub_info != pNextLink->sub_info)
		{
			return false;
		}

		// 두 노드가 회귀 링크면 유지
		//
		// 코드 입력 필요
		//

		// 버텍스 결합
		//linkMerge(pLink->vtPts, pNextLink->vtPts);
		pLink->mergeVertex(pNextLink);

		// 버텍스 길이 결합
		pLink->length += pNextLink->length;

		// 버텍스 삭제
		pNextLink->sub_info = NOT_USE;
		pNextLink->length = 0;
		pNextLink->setVertex();


		//    prevNode -------------edgeNode/edgeNode-------------nextNode
		// 링크 양단 노드의 연결 정보 변경
		if (!changeConnectedNodeIdx(pPrevNode, pNode, pNode, pNextNode))
		{
			LOG_TRACE(LOG_WARNING, "Warning, can't change link info, link id:%d, prev node idx:%d, next node idx:%d", pLink->link_id.nid, pPrevNode->node_id.nid, pNextNode->node_id.nid);
		}

		// 링크의 s/e 노드 변경
		if (nLinkStartEnd == 1) // start
		{
			// 링크 s점 변경
			pLink->snode_id = pNextNode->node_id;
		}
		else // end
		{
			// 링크 e점 변경
			pLink->enode_id = pNextNode->node_id;
		}
	}
	// 4.구획변교점 
	else if (pNode->base.point_type == TYPE_NODE_EDGE)
	{
		// 인접 노드 갯수가 2개 이하인것만
		if (pNode->base.connnode_count > 1)
		{
			LOG_TRACE(LOG_WARNING, "Warning, too many conned node count for attribute change point, count:%d", pNode->base.connnode_count);
			return false;
		}

		// 구획변교점 노드와 매칭되는 노드 검색
		//stNodeInfo* pEdgeNode = getEdgeNodeByEdgeNodeIdx(pMapNode, pNode->edgenode_id);
		stNodeInfo* pEdgeNode = getEdgeNodeByEdgeNodeId(pMapNode, pNode->edgenode_id);
		if (pEdgeNode == nullptr)
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find edge node, idx:%d", pNode->edgenode_id.llid);
			return false;
		}

#if defined(USE_PEDESTRIAN_DATA) || defined(USE_FOREST_DATA) || defined(USE_VEHICLE_DATA)
		// 구획 변교점 노드와 매칭되는 노드의 링크는 1개여야 한다.
		if (pEdgeNode->base.connnode_count != 1) {
			LOG_TRACE(LOG_WARNING, "Failed, edge node connected shoulud have one node(link), cnt:%d", pEdgeNode->base.connnode_count);
			return false;
		}

		// 접속 노드는 이미 링크 IDX로 변경해뒀다
		stLinkInfo* pNextLink = nullptr;
		if ((itLink = pMapLink->find(pEdgeNode->connnodes[0].llid)) == pMapLink->end()) {
			LOG_TRACE(LOG_WARNING, "Failed, can't find next link, edge node connected idx:%d", pEdgeNode->connnodes[0].llid);
			return false;
		}
		else if ((pNextLink = itLink->second) == nullptr) {
			LOG_TRACE(LOG_WARNING, "Failed, can't find next link, edge node idx:%d", pEdgeNode->edgenode_id.llid);
			return false;
		}

		if (pNextLink->snode_id == pEdgeNode->node_id) {
			//pNextNode = getNodeByNodeIdx(pMapNode, pNextLink->enode_id);
			itNode = pMapNode->find(pNextLink->enode_id.llid);
		}
		else {
			//pNextNode = getNodeByNodeIdx(pMapNode, pNextLink->snode_id);
			itNode = pMapNode->find(pNextLink->snode_id.llid);
		}

		//if (pNextNode == nullptr)
		if (itNode == pMapNode->end())
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find next node, idx:%d", pEdgeNode->edgenode_id.llid);
			return false;
		}
		stNodeInfo* pNextNode = itNode->second;

#else
		// 매칭되는 노드를 인접 노드로 가지는 다음 노드 검색
		stNodeInfo* pNextNode = getConnNodeByNodeIdx(pMapNode, pEdgeNode->node_id, idxLinkPrevNode);
		if (pNextNode == nullptr)
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find next node, idx:%d", pEdgeNode->edgenode_id.llid);
			return false;
		}

		// 위 노드를 노드 s/e로 가진 가진 링크 검색
		stLinkInfo* pNextLink = getLinkByNodeIdx(pMapLink, pEdgeNode->node_id, pNextNode->node_id);
		if (pNextLink == nullptr)
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find next link, edge node idx:%d", pEdgeNode->edgenode_id.llid);
			return false;
		}
#endif

		// 버텍스 결합
		pLink->mergeVertex(pNextLink);
		//linkMerge(pLink->vtPts, pNextLink->vtPts);

		// 버텍스 길이 결합
		pLink->length += pNextLink->length;

		// 링크 버림 표시
		pNextLink->sub_info = NOT_USE;
		pNextLink->length = 0;
		pNextLink->setVertex();


		// 링크 반대쪽 노드 정보		
		//stNodeInfo* pPrevNode = getNodeByNodeIdx(pMapNode, idxLinkPrevNode);
		itNode = pMapNode->find(idxLinkPrevNode.llid);
		//if (pPrevNode == nullptr)
		if (itNode == pMapNode->end())
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find prev node, idx:%d", idxLinkPrevNode);
			return false;
		}
		stNodeInfo* pPrevNode = itNode->second;


		//    prevNode -------------edgeNode/edgeNode-------------nextNode
		// 링크 양단 노드의 연결 정보 변경
#if defined(USE_PEDESTRIAN_DATA) || defined(USE_FOREST_DATA) || defined(USE_VEHICLE_DATA)
		bool changed = false;
		for (uint32_t ii = 0; ii < pNextNode->base.connnode_count; ii++)
		{
			// 보행자 데이터는 접속정보에 이미 링크정보가 있으니까 링크정보로 비교
			if (pNextNode->connnodes[ii] == pNextLink->link_id) {
				pNextNode->connnodes[ii] = pLink->link_id;
				changed = true;
				break;
			}
		}

		// 버림 표시
		pNextLink->sub_info = NOT_USE;

		pNode->sub_info = NOT_USE;
		pEdgeNode->sub_info = NOT_USE;

		if (!changed)
#else
		if (!changeConnectedNodeIdx(pPrevNode, pNode, pEdgeNode, pNextNode))
#endif
		{
			LOG_TRACE(LOG_WARNING, "Warning, can't change link info, link id:%l, prev node idx:%l, next node idx:%l", pLink->link_id.llid, pPrevNode->node_id.llid, pNextNode->node_id.llid);
		}

		// 링크의 s/e 노드 변경
		if (nLinkStartEnd == 1) // start
		{
			// 링크 s점 변경
			pLink->snode_id = pNextNode->node_id;
		}
		else // end
		{
			// 링크 e점 변경
			pLink->enode_id = pNextNode->node_id;
		}

		return true;
	}

	return false;
}

void CFileBase::MergeLink(IN stLinkInfo* pLink, IN unordered_map<uint64_t, stLinkInfo*>* pMapLink, IN unordered_map<uint64_t, stNodeInfo*>* pMapNode)
{
	if (!pLink || !pMapLink || !pMapNode)
	{
		LOG_TRACE(LOG_ERROR, "Error, wrrong param, pLink:%X, pMapLink:%X, pMapNode:%X", pLink, pMapLink, pMapNode);
		return;
	}

	if (pLink->sub_info == NOT_USE)
	{
		//LOG_TRACE(LOG_DEBUG, "Merged link, idx:%d, mesh:%d, id:%d", pLink->link_idx, pLink->link_id.tile_id, pLink->link_id.nid);
		return;
	}

	// 테스트 메쉬에서는 노드 끊김이 있을 수 있으니 무시하자
	if (g_isUseTestMesh && (pLink->enode_id.llid == 0))
	{
		return;
	}


	if (MergeEdgePoint(1, pLink, pMapLink, pMapNode)) { // snode
		// 다음 노드도 구획 변경점이면 재귀
		MergeLink(pLink, pMapLink, pMapNode);
	}

	if (MergeEdgePoint(2, pLink, pMapLink, pMapNode)) { // enode
		// 다음 노드도 구획 변경점이면 재귀
		MergeLink(pLink, pMapLink, pMapNode);
	}
}

//bool CFileManager::FindStartEnd_INDEX(int netType, uint32_t meshID, int &sIndex, int &eIndex)
//{
//	if (netType == NETWORKTYPE::LINK) {
//		size_t numTable = m_vtLinkIdxTable.size();
//
//		if (numTable == 0) {
//			return false;
//		}
//
//		for (unsigned int idxTable = 0; idxTable < numTable; ++idxTable) {
//			if (m_vtLinkIdxTable[idxTable].meshID == meshID) {
//				sIndex = m_vtLinkIdxTable[idxTable].sIDX;
//				eIndex = m_vtLinkIdxTable[idxTable].eIDX;
//				return true;
//			}
//		}
//	}
//	else if (netType == NETWORKTYPE::NODE) {
//		size_t numTable = m_vtNodeIdxTable.size();
//
//		if (numTable == 0) {
//			return false;
//		}
//
//		for (unsigned int idxTable = 0; idxTable < numTable; ++idxTable) {
//			if (m_vtNodeIdxTable[idxTable].meshID == meshID) {
//				sIndex = m_vtNodeIdxTable[idxTable].sIDX;
//				eIndex = m_vtNodeIdxTable[idxTable].eIDX;
//				return true;
//			}
//		}
//	}
//
//	return false;
//}


void CFileBase::SetMeshBox(IN const SBox* pBox)
{
	if (m_pDataMgr) {
		return m_pDataMgr->SetMeshBox(pBox);
	}
}


//void CFileBase::SetNeighborMesh(void)
//{
//	if (m_pDataMgr) {
//		m_pDataMgr->SetNeighborMesh();
//	}
//}


bool CFileBase::AddMeshData(IN const stMeshInfo * pData)
{
	if (m_pDataMgr) {
		return m_pDataMgr->AddMeshData(pData);
	}

	return false;
}

//bool CFileBase::AddMeshDataByNode(IN const stMeshInfo * pInfo, IN const stNodeInfo * pData)
//{
//	if (AddMeshData(pInfo)) {
//		if (m_pDataMgr) {
//			return m_pDataMgr->AddMeshDataByNode(pInfo, pData);
//		}
//	}
//
//	return false;
//}
//
//bool CFileBase::AddMeshDataByLink(IN const stMeshInfo * pInfo, IN const stLinkInfo * pData)
//{
//	if (AddMeshData(pInfo)) {
//		if (m_pDataMgr) {
//			return m_pDataMgr->AddMeshDataByLink(pInfo, pData);
//		}
//	}
//
//	return false;
//}
//
//bool CFileBase::AddMeshDataByPolygon(IN const stMeshInfo * pInfo, IN const stPolygonInfo * pData)
//{
//	if (AddMeshData(pInfo)) {
//		if (m_pDataMgr) {
//			return m_pDataMgr->AddMeshDataByPolygon(pInfo, pData);
//		}
//	}
//
//	return false;
//}


bool CFileBase::AddNameData(IN const stNameInfo * pData)
{
	if (m_pDataMgr) {
		return m_pDataMgr->AddNameData(pData);
	}

	return false;
}


bool CFileBase::AddNodeData(IN const stNodeInfo * pData)
{
	bool ret = false;

	if (m_pDataMgr && (m_nFileType == TYPE_EXEC_NETWORK)) {
		if (m_nDataType == TYPE_DATA_VEHICLE) {
			ret = m_pDataMgr->AddVNodeData(pData);
		} else if (m_nDataType == TYPE_DATA_PEDESTRIAN) {
			ret = m_pDataMgr->AddWNodeData(pData);
		} else {
			ret = m_pDataMgr->AddFNodeData(pData);
		}
	}

	return ret;
}


bool CFileBase::AddLinkData(IN const stLinkInfo * pData)
{
	bool ret = false;
	if (m_pDataMgr && (m_nFileType == TYPE_EXEC_NETWORK)) {
		if (m_nDataType == TYPE_DATA_VEHICLE) {
			ret = m_pDataMgr->AddVLinkData(pData);
		} else if (m_nDataType == TYPE_DATA_PEDESTRIAN) {
			ret = m_pDataMgr->AddWLinkData(pData);
		} else {
			ret = m_pDataMgr->AddFLinkData(pData);
		}
	}

	return ret;
}


bool CFileBase::AddPolygonData(IN const stPolygonInfo * pData)
{
	bool ret = false;

#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
	if (m_pDataMgr) {
		if ((GetDataType() == TYPE_DATA_COMPLEX) || (GetDataType() == TYPE_DATA_MOUNTAIN)) {
			if (ret = m_pDataMgr->AddComplexData(pData)) {
				//if (!pData->vtJoinedMesh.empty()) {
				//	// 중첩 메쉬가 있으면 충첩된 메쉬에도 추가
				//	stMeshInfo* pJoinedMesh;
				//	for (int ii = pData->vtJoinedMesh.size() - 1; ii >= 0; --ii) {
				//		if (pData->poly_id.tile_id != pData->vtJoinedMesh[ii]) { // 자신은 패스
				//			pJoinedMesh = m_pDataMgr->GetMeshDataById(pData->vtJoinedMesh[ii]);
				//			if (pJoinedMesh != nullptr) {
				//				pJoinedMesh->complexs.emplace_back(pData->poly_id);
				//			}
				//		}
				//	} // for
				//}

				// 아래 구문은 for spatialindex 사용으로 인해 MapMesh::InsertComplex() 안으로 이동됨. 2025-08-13
//#if !defined(USE_DATA_CACHE) 
//				if (pData->getAttributeCount(TYPE_POLYGON_DATA_ATTR_MESH) > 1) { // 항상 자신을 포함하므로 1개 이상일때만 인접메쉬 존재
//					stMeshInfo* pJoinedMesh;
//					uint32_t idMesh;
//					const uint32_t* pJoined = pData->getAttributeMesh();
//					for (int ii = pData->getAttributeCount(TYPE_POLYGON_DATA_ATTR_MESH) - 1; ii >= 0; --ii) {
//						idMesh = pJoined[ii];
//						// 23-01-06 // 단독 데이터 로딩 시, 메쉬가 순차적으로 로딩되기에 나자신보다 나중에 로딩되는 메시는 메쉬 정보가 없어 누락될 수 있음
//						// 메쉬 파일을 따로 관리하는게 나을라나??
//						if ((pData->poly_id.tile_id != idMesh) && ((pJoinedMesh = m_pDataMgr->GetMeshDataById(idMesh)) != nullptr)) { // 자신은 패스
//							pJoinedMesh->complexs.emplace_back(pData->poly_id);
//						}
//					} // for
//				}
//#endif
			}
		}
		else if (GetDataType() == TYPE_DATA_BUILDING) {
			ret = m_pDataMgr->AddBuildingData(pData);
		}
	}
#endif // #if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)

	return ret;
}


bool CFileBase::AddEntranceData(IN const KeyID keyId, IN const stEntranceInfo * pData)
{
	//if (m_pDataMgr) {
	//	return m_pDataMgr->AddEntranceData(keyId, pData);
	//}

	return false;
}


//stLinkInfo * CFileManager::GetLinkDataBySENode(IN const KeyID idx1, IN const KeyID idx2)
//{
//	stLinkInfo* pData = nullptr;
//
//	if (m_pMapLink && idx1.nid >= 0 && idx2.nid >= 0)
//	{
//		if (!(pData = m_pMapLink->GetLinkBySENode(idx1, idx2)))	{
//			if (GetData(idx1.tile_id) && GetData(idx2.tile_id)) {
//				stNodeInfo* pNode = nullptr;
//				stNodeInfo* pNodeNext = nullptr;
//				
//				if ((pNode = GetNodeDataById(idx1)) && pNode->connnode_count > 1) {
//					for (int ii = 0; ii < pNode->connnode_count; ii++)
//					{
//						pNodeNext = GetNodeDataById(pNode->connnodes[ii]);
//						if (pNodeNext && pNodeNext->node_id != idx1)
//						{
//							GetData(pNodeNext->node_id.tile_id);
//						}
//					}
//				}
//
//				if ((pNode = GetNodeDataById(idx2)) && pNode->connnode_count > 1) {
//					for (int ii = 0; ii < pNode->connnode_count; ii++)
//					{
//						pNodeNext = GetNodeDataById(pNode->connnodes[ii]);
//						if (pNodeNext && pNodeNext->node_id != idx2)
//						{
//							GetData(pNodeNext->node_id.tile_id);
//						}
//					}
//				}
//
//				pData = m_pMapLink->GetLinkBySENode(idx1, idx2);
//			}
//		}
//	}
//		
//	return pData;
//}


//void CFileManager::GetConnectedLinkByLink(stLinkIDX _Link, std::vector<stLinkIDX> &ConnectedLink)
//stLinkIDX CFileManager::GetConnectedLinkByLink(stLinkIDX _Link)
//{
//	int sIDX = -1, eIDX = -1;
//	std::vector<stLinkIDX> vConnectedLinks;
//	bool isFind = FindStartEnd_INDEX(NETWORKTYPE::LINK, _Link.MeshID, sIDX, eIDX);
//
//	if (isFind == false) {
//		return stLinkIDX{ 0, };
//	}
//
//	int sNodeID = _Link.sNodeID;
//	int eNodeID = _Link.eNodeID;
//	stLinkIDX _getLink;
//
//	for (unsigned int idx = sIDX; idx < eIDX; ++idx) {
//		if ((m_vtLinkIDX[idx].sNodeID == eNodeID || m_vtLinkIDX[idx].eNodeID == eNodeID) && m_vtLinkIDX[idx].sNodeID != sNodeID) {
//			vConnectedLinks.push_back(m_vtLinkIDX[idx]);
//		}
//	}
//
//	return CalculateShortestDistance(vConnectedLinks);
//}

//stLinkIDX CFileManager::CalculateShortestDistance(std::vector<stLinkIDX> &vGetConnectedLinks)
//{
//	size_t numLinks = vGetConnectedLinks.size();
//	double minDistance = DBL_MAX;
//	int ShortestIdx = -1;
//
//	if (numLinks != 0) {
//		for (unsigned int idx = 0; idx < numLinks; ++idx) {
//			if (minDistance > vGetConnectedLinks[idx].LinkLength) {
//				minDistance = vGetConnectedLinks[idx].LinkLength;
//				ShortestIdx = idx;
//			}
//		}
//	}
//
//	return vGetConnectedLinks[ShortestIdx];
//}

//stLinkIDX* CFileManager::GetLinkInMesh(uint32_t meshID, uint32_t sNode, uint32_t eNode)
//{
//	int sIDX = -1, eIDX = -1;
//
//	bool isFind = FindStartEnd_INDEX(NETWORKTYPE::LINK, meshID, sIDX, eIDX);
//
//	if (isFind == true) {
//		for (unsigned int idx = sIDX; idx < eIDX; ++idx) {
//			if (m_vtLinkIDX[idx].sNodeID == sNode && m_vtLinkIDX[idx].eNodeID == eNode) {
//				return &m_vtLinkIDX[idx];
//			}
//		}
//	}
//
//	return nullptr;
//}

bool LineCollision(SPoint A, SPoint B, SPoint C, SPoint D, SPoint& out)
{
	if ((B.x - A.x)*(D.y - C.y) != (D.x - C.x)*(B.y - A.y))
	{
		float k = (C.x - A.x)*(D.y - C.y) - (D.x - C.x)*(C.y - A.y);
		float m = (C.x - A.x)*(B.y - A.y) - (B.x - A.x)*(C.y - A.y);
		if ((k >= 0) && (m >= 0))
		{
			float b = (B.x - A.x)*(D.y - C.y) - (D.x - C.x)*(B.y - A.y);
			if (k <= b && m <= b)
			{
				k /= b;
				out.x = A.x + (B.x - A.x)*k;
				out.y = A.y + (B.y - A.y)*k;
				return true;
			}
		}
	}
	return false;
}



bool CFileBase::GenServiceData()
{
	// 비어있는 메쉬 검사
	LOG_TRACE(LOG_DEBUG, "LOG, start mesh data generation, check empty mesh");

	stMeshInfo* pMesh = nullptr;
#if 1
	m_pDataMgr->ArrangementMesh();
#else
	map<uint32_t, stMeshInfo*>::iterator it;
	for (it = m_mapMesh.begin(); it != m_mapMesh.end(); it++) {
		pMesh = it->second;

		if (
#if defined(USE_FOREST_DATA)
			(pMesh->setFLinkDuplicateCheck.empty())
#endif 
#if defined(USE_PEDESTRIAN_DATA)
			(pMesh->setWLinkDuplicateCheck.empty())
#endif 
#if defined(USE_VEHICLE_DATA)
			(pMesh->setVLinkDuplicateCheck.empty())
#endif 
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
			&& ((pMesh->setCpxDuplicateCheck.empty() && pMesh->setBldDuplicateCheck.empty()))
#endif 
			)
		{
			LOG_TRACE(LOG_DEBUG, "LOG, remove empty mesh : %d", pMesh->mesh_id.tile_id);
			//m_pDataMgr->DeleteData(pMesh->mesh_id.tile_id);
			//cntMesh--;
			m_mapMesh.erase(it++);
		}
	}
#endif

	Release();

	LOG_TRACE(LOG_DEBUG, "LOG, end mesh data generation");

	return true;
}


bool CFileBase::Initialize()
{
	memset(m_szSrcPath, 0x00, sizeof(m_szSrcPath));
	memset(m_szWorkPath, 0x00, sizeof(m_szWorkPath));
	memset(m_szDstPath, 0x00, sizeof(m_szDstPath));

	return true;
}

void CFileBase::Release()
{
	if (!m_vtIndex.empty()) {
		m_vtIndex.clear();
		vector<FileIndex>().swap(m_vtIndex);
	}

	if (!m_mapMesh.empty()) {
		m_mapMesh.clear();
		map<uint32_t, stMeshInfo*>().swap(m_mapMesh);
	}

	if (!m_mapNodeIndex.empty()) {
		m_mapNodeIndex.clear();
		unordered_map<uint64_t, uint32_t>().swap(m_mapNodeIndex);
	}

	if (!m_mapNode.empty()) {
		m_mapNode.clear();
		unordered_map<uint64_t, stNodeInfo*>().swap(m_mapNode);
	}

	if (!m_mapLink.empty()) {
		m_mapLink.clear();
		unordered_map<uint64_t, stLinkInfo*>().swap(m_mapLink);
	}
}


const uint32_t CFileBase::GetDataType(void) const
{
	return m_nDataType;
}


char* CFileBase::GetDataTypeName(void) const
{
	return g_szTypeName[m_nFileType];
}


void CFileBase::SetDataManager(IN CDataManager* pDataMgr)
{
	m_pDataMgr = pDataMgr;
}


void CFileBase::SetMeshRegion(IN const SBox* pRegion)
{
	if (pRegion->Xmin == 0 && pRegion->Xmin == 0 &&
		pRegion->Xmax == 0 && pRegion->Ymax == 0) {
		return;
	}

	if (m_pDataMgr) {
		m_pDataMgr->SetMeshBox(pRegion);
	}
	else if (pRegion) {
		memcpy(&m_rtBox, pRegion, sizeof(m_rtBox));
	}
}


void CFileBase::SetNameManager(IN CFileName* pNameMgr)
{
	m_pNameMgr = pNameMgr;
}

// mesh
uint32_t CFileBase::GetMeshCount(void)
{
	if (m_pDataMgr) {
		return m_pDataMgr->GetMeshCount();
	}

	return 0;
}

stMeshInfo * CFileBase::GetMeshData(IN const uint32_t idx)
{
	if (m_pDataMgr) {
		return m_pDataMgr->GetMeshData(idx);
	}

	return nullptr;
}

stMeshInfo * CFileBase::GetMeshDataById(IN const uint32_t id, IN const bool force)
{
	if (m_pDataMgr) {
		return m_pDataMgr->GetMeshDataById(id, force);
	}

	return nullptr;
}

const SBox* CFileBase::GetMeshRegion(IN const int32_t idx) const
{
	if (m_pDataMgr) {
		return m_pDataMgr->GetMeshRegion(idx);
	}

	return &m_rtBox;
}


bool CFileBase::ParseData(IN const char* fname)
{
//#if defined (USE_PEDESTRIAN_DATA)
//	return m_trackShpMgr.ParseData(fname);
//#endif

	XSHPReader shpReader;

	if (!shpReader.Open(fname)) {
		LOG_TRACE(LOG_ERROR, "Error, Can't Find shp File : %s", fname);
		return false;
	}

	int nShpType = (int)shpReader.GetShpObjectType(); //shpPoint=1,shpPolyLine=3,shpPolygon=5

	if (nShpType == 5) nShpType = 3; //면
	else if (nShpType == 3) nShpType = 2; //선
	else if (nShpType == 1) nShpType = 1; //점

	//전체 지도영역 얻기
	shpReader.GetEnvelope(&m_rtBox);

	//전체 데이터 카운터 얻기
	long nRecCount = shpReader.GetRecordCount();
	int nFieldCount = shpReader.GetFieldCount();

	//int nLabelIdxToRead = 1;
	char chsTmp[12];
	//int nPoint;
	//SPoint* pPoints;
	//DBF_FIELD_INFO dbinfo;
	int colCnt = shpReader.GetFieldCount();

	//데이터 얻기
	for (int idxRec = 0; idxRec < nRecCount; idxRec++) {

	//	//_MeshRaw _mesh = { 0, };
	//	//_LinkRaw _link = { 0, };
	//	//_NodeRaw _node = { 0, };

	//	_MeshRaw _mesh;
	//	_LinkRaw _link;
	//	_NodeRaw _node;

		//속성가져오기
		if (shpReader.Fetch(idxRec) == false) { //error to read..
			LOG_TRACE(LOG_ERROR, "Error : Record %d is not available!!\n", idxRec);
			continue;
		}

		//Setting DBF Data
		for (unsigned int idxCol = 0; idxCol < colCnt; idxCol++) {
			memset(chsTmp, 0x00, 12);
			shpReader.GetDataAsString(idxCol, chsTmp, 12);  //nLabelIdxToRead 번째 필드 값을 chsTmp로 12byte 읽어옴.

			SHPGeometry *pSHPObj = shpReader.GetGeometry();

	//		if (nShpType == 1) {				// 점 일때				
	//			SetData_Node(idxCol, _node, chsTmp);
	//			_node.nodePoint = pSHPObj->point.point;
	//		}
	//		else if (nShpType == 2) {		 // 선 일때
	//			SetData_Link(idxCol, _link, chsTmp);
	//			if (idxCol == 0)
	//			{
	//				nPoint = SHP_GetNumPartPoints(pSHPObj, 0); //파트 0에 대한	   //선형 갯수..					 																	 
	//				pPoints = SHP_GetPartPoints(pSHPObj, 0); //파트 0에 대한	     //실제선형데이터..

	//				for (unsigned int idxObj = 0; idxObj < nPoint; idxObj++) {
	//					memset(_link.szName, 0x0, sizeof(_link.szName));
	//					_link.vtPoints.push_back(pPoints[idxObj]);
	//				}
	//			}
	//		}
	//		else if (nShpType == 3) { // 면
	//			_mesh.MeshID = atoi(trim(chsTmp));
	//			memcpy(&_mesh.meshBox, &pSHPObj->mpoint.Box, sizeof(SBox));
	//			m_vtMeshRaw.insert(std::pair<int, SBox>(atoi(chsTmp), pSHPObj->mpoint.Box));
	//		}

			AddDataFeild(idxCol, nShpType, trim(chsTmp));
		}
	//	if (nShpType == 1)				m_vtNodeRaw.push_back(_node);
	//	else if (nShpType == 2)		m_vtLinkRaw.push_back(_link);

		AddDataRecord();
	}

	shpReader.Close();

	return true;
}


bool CFileBase::SetPath(IN const char* szSrcPath, IN const char* szWorkPath, IN const char* szDstPath)
{
	if (szSrcPath != nullptr) {
		strcpy(m_szSrcPath, szSrcPath);
	}
	if (szWorkPath != nullptr) {
		strcpy(m_szWorkPath, szWorkPath);
	}
	if (szDstPath != nullptr) {
		strcpy(m_szDstPath, szDstPath);
	}

	return true;
}


//bool CFileManager::OpenFile(IN const vector<string>* pvtFilePath)
bool CFileBase::OpenFile(IN const char* szFilePath)
{
	return ParseData(szFilePath);
}


bool CFileBase::SaveData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[m_nDataType], g_szTypeExec[m_nFileType]);


	if (!m_pDataMgr) {
		LOG_TRACE(LOG_ERROR, "Failed, not initialized data manager");
		return false;
	}
	else if (szFileName == nullptr || strlen(szFileName) <= 0) {
		LOG_TRACE(LOG_ERROR, "Failed, file name not exist");
		return false;
	}
	
	// path check
	if (checkDirectory(szFilePath) == false) {
		LOG_TRACE(LOG_ERROR, "Error, Can't create save directory : %s", szFilePath);
		return false;
	}

	size_t offFile = 0;
	size_t offItem = 0;
	size_t retWrite = 0;
	size_t retRead = 0;

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
	offFile += WriteHeader(fp);

	// check offset 
	if (m_fileHeader.offIndex != offFile)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file header index offset not match, header:%d vs current:%d", m_fileHeader.offIndex, offFile);
		fclose(fp);
		return false;
	}


	// write index
	offFile += WriteIndex(fp);

	// check offset 
	if (m_fileHeader.offBody != offFile)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file header body offset not match, header:%d vs current:%d", m_fileHeader.offBody, offFile);
		fclose(fp);
		return false;
	}


	// write body
	offFile += WriteBody(fp, offFile);
	
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
	//	memcpy(&fileIndex, &m_vtIndex[idx], sizeof(fileIndex));
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


size_t CFileBase::WriteBase(FILE* fp)
{
	size_t offFile = 0;
	size_t retWrite = 0;
	size_t retRead = 0;
	FileBase stFileBase = { 0, };
	const size_t sizeBase = sizeof(FileBase);

	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	// 클래스별 타입
	strcpy(stFileBase.szType, GetDataTypeName());		// 데이터 타입

	stFileBase.version[0] = FILE_VERSION_MAJOR;
	stFileBase.version[1] = FILE_VERSION_MINOR;
	stFileBase.version[2] = FILE_VERSION_PATCH;

	// 생성 시각
#if defined(_WIN32)
	SYSTEMTIME st, lt;
	GetSystemTime(&st);
	GetLocalTime(&lt);

	stFileBase.version[3] = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
#else
	time_t timer = time(NULL);
	struct tm* tmNow = localtime(&timer);

	stFileBase.version[3] = (tmNow->tm_year + 1900) * 10000 + (tmNow->tm_mon + 1) * 100 + tmNow->tm_mday;
#endif

	// 기본 헤더 사이즈
	stFileBase.szHeader = sizeof(m_fileHeader);

	// 저장
	if ((retWrite = fwrite(&stFileBase, sizeBase, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't write base, written:%d", retWrite);
		return 0;
	}
	offFile += sizeBase;


	LOG_TRACE(LOG_DEBUG, "Save data, base, type:%s, ver:%d.%d.%d.%d, header:%d",
		stFileBase.szType, stFileBase.version[0], stFileBase.version[1], stFileBase.version[2], stFileBase.version[3], stFileBase.szHeader);

	return offFile;
}


size_t CFileBase::WriteHeader(FILE* fp)
{
	size_t offFile = 0;
	size_t retWrite = 0;
	const size_t sizeHeader = sizeof(FileHeader);

	if (!fp ) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	memset(&m_fileHeader, 0x00, sizeof(m_fileHeader));
	memcpy(&m_fileHeader.rtMap, GetMeshRegion(), sizeof(m_fileHeader.rtMap));
	m_fileHeader.cntIndex = GetMeshCount();
	m_fileHeader.offIndex = sizeof(FileBase) + sizeHeader;
	m_fileHeader.offBody = m_fileHeader.offIndex + sizeof(FileIndex) * m_fileHeader.cntIndex;

	if ((retWrite = fwrite(&m_fileHeader, sizeHeader, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't write header, written:%d", retWrite);
		return 0;
	}
	offFile += sizeHeader;

	LOG_TRACE(LOG_DEBUG, "Save data, header, mesh cnt:%lld", m_fileHeader.cntIndex);

	return offFile;
}


size_t CFileBase::WriteIndex(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileIndex fileIndex;
	stMeshInfo* pMesh = nullptr;

	size_t offFile = 0;
	size_t retWrite = 0;
	const size_t sizeIndex = sizeof(fileIndex);

	int cntMesh = GetMeshCount();
	for (int idx = 0; idx < cntMesh; idx++)
	{
		int idxNeighbor = 0;
		memset(&fileIndex, 0x00, sizeof(fileIndex));

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


size_t CFileBase::WriteBody(FILE* fp, IN const uint32_t fileOff)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	size_t offFile = fileOff;
	size_t retWrite = 0;
	long offItem = 0;

	stMeshInfo* pMesh = nullptr;
	stNodeInfo* pNode = nullptr;
	stLinkInfo* pLink = nullptr;

	FileBody fileBody;
	FileNode fileNode = { 0, };
	FileLink fileLink = { 0, };

	uint32_t ii, jj = 0;
	uint32_t cntNode, cntLink;
	uint32_t retNode = 0, retLink = 0;
	const size_t sizeFileBody = sizeof(fileBody);

	// write body - node, link, vertex
	int cntMesh = GetMeshCount();
	for (ii = 0; ii < cntMesh; ii++)
	{
		offItem = 0;
		pMesh = GetMeshData(ii);
		if (!pMesh)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't access mesh, idx:%d", ii);
			return 0;
		}

#if defined(USE_VEHICLE_DATA)
		if (m_nDataType == TYPE_DATA_VEHICLE) {
#ifdef TEST_SPATIALINDEX
			cntNode = pMesh->setVNodeDuplicateCheck.size();
			cntLink = pMesh->setVLinkDuplicateCheck.size();
#else
			cntNode = pMesh->vnodes.size();
			cntLink = pMesh->vlinks.size();
#endif
		}
#endif
#if defined(USE_PEDESTRIAN_DATA)
		if (m_nDataType == TYPE_DATA_PEDESTRIAN) {
#ifdef TEST_SPATIALINDEX
			cntNode = pMesh->setWNodeDuplicateCheck.size();
			cntLink = pMesh->setWLinkDuplicateCheck.size();
#else
			cntNode = pMesh->wnodes.size();
			cntLink = pMesh->wlinks.size();
#endif
		}
#endif
#if defined(USE_FOREST_DATA)
		if (m_nDataType == TYPE_DATA_TREKKING) {
#ifdef TEST_SPATIALINDEX
			cntNode = pMesh->setFNodeDuplicateCheck.size();
			cntLink = pMesh->setFLinkDuplicateCheck.size();
#else
			cntNode = pMesh->nodes.size();
			cntLink = pMesh->links.size();
#endif
		}
#endif
		
		if ((cntNode > 0) || (cntLink > 0)) {
			// write body
			memset(&fileBody, 0x00, sizeFileBody);
			fileBody.idTile = pMesh->mesh_id.tile_id;
			fileBody.net.cntNode = cntNode;
			fileBody.net.cntLink = cntLink;

			if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1) {
				LOG_TRACE(LOG_ERROR, "Failed, can't write body[%d], written:%d", ii, retWrite);
				return 0;
			}
			offItem = sizeFileBody;

			// write node
#ifdef TEST_SPATIALINDEX
			set<KeyID>* pSetNode = nullptr;
#if defined(USE_VEHICLE_DATA)
			if (m_nDataType == TYPE_DATA_VEHICLE) {
				pSetNode = &pMesh->setVNodeDuplicateCheck;
			}
#endif
#if defined(USE_PEDESTRIAN_DATA)
			if (m_nDataType == TYPE_DATA_PEDESTRIAN) {
				pSetNode = &pMesh->setWNodeDuplicateCheck;
			}
#endif
#if defined(USE_FOREST_DATA)
			if (m_nDataType == TYPE_DATA_TREKKING) {
				pSetNode = &pMesh->setFNodeDuplicateCheck;
		}
#endif
			for (const auto key : *pSetNode) {
#else
			for (jj = 0; jj < cntNode; jj++) {
#endif
#if defined(USE_VEHICLE_DATA)
				if (m_nDataType == TYPE_DATA_VEHICLE) {
#ifdef TEST_SPATIALINDEX
					pNode = m_pDataMgr->GetVNodeDataById(key);
#else
					pNode = m_pDataMgr->GetVNodeDataById(pMesh->vnodes[jj]);
#endif
				}
#endif
#if defined(USE_PEDESTRIAN_DATA)
				if (m_nDataType == TYPE_DATA_PEDESTRIAN) {
#ifdef TEST_SPATIALINDEX
					pNode = m_pDataMgr->GetWNodeDataById(key);
#else
					pNode = m_pDataMgr->GetWNodeDataById(pMesh->wnodes[jj]);
#endif
				}
#endif
#if defined(USE_FOREST_DATA)
				if (m_nDataType == TYPE_DATA_TREKKING) {
#ifdef TEST_SPATIALINDEX
					pNode = m_pDataMgr->GetFNodeDataById(key);
#else
					pNode = m_pDataMgr->GetFNodeDataById(pMesh->nodes[jj]);
#endif
				}
#endif

				if (!pNode) {
					LOG_TRACE(LOG_ERROR, "Failed, can't access node, idx:%d, tile_id:%d", jj, pMesh->mesh_id.tile_id);
					return 0;
				}

				if (pNode->sub_info == NOT_USE)
					continue;

				fileNode.node_id = pNode->node_id;
				fileNode.sub_info = pNode->sub_info;
				fileNode.edgenode_id = pNode->edgenode_id;
				memcpy(&fileNode.coord, &pNode->coord, sizeof(fileNode.coord));
				memcpy(fileNode.connnodes, pNode->connnodes, sizeof(fileNode.connnodes));
				memcpy(fileNode.conn_attr, pNode->conn_attr, sizeof(fileNode.conn_attr));

				if ((retWrite = fwrite(&fileNode, sizeof(fileNode), 1, fp)) != 1) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write node[%d], tile_id:%d, id:%d", jj, fileNode.node_id.tile_id, fileNode.node_id.nid);
					return 0;
				}

				offItem += sizeof(fileNode);

				retNode++;
			} // write node

			// write link
#ifdef TEST_SPATIALINDEX
			set<KeyID>* pSetLink = nullptr;
#if defined(USE_VEHICLE_DATA)
			if (m_nDataType == TYPE_DATA_VEHICLE) {
				pSetLink = &pMesh->setVLinkDuplicateCheck;
			}
#endif
#if defined(USE_PEDESTRIAN_DATA)
			if (m_nDataType == TYPE_DATA_PEDESTRIAN) {
				pSetLink = &pMesh->setWLinkDuplicateCheck;
			}
#endif
#if defined(USE_FOREST_DATA)
			if (m_nDataType == TYPE_DATA_TREKKING) {
				pSetLink = &pMesh->setFLinkDuplicateCheck;
			}
#endif
			for (const auto key : *pSetLink) {
#else
			for (jj = 0; jj < cntLink; jj++) {
#endif
	#if defined(USE_VEHICLE_DATA)
				if (m_nDataType == TYPE_DATA_VEHICLE) {
#ifdef TEST_SPATIALINDEX
					pLink = m_pDataMgr->GetVLinkDataById(key);
#else
					pLink = m_pDataMgr->GetVLinkDataById(pMesh->vlinks[jj]);
#endif
				}
	#endif
	#if defined(USE_PEDESTRIAN_DATA)
				if (m_nDataType == TYPE_DATA_PEDESTRIAN) {
	#ifdef TEST_SPATIALINDEX
					pLink = m_pDataMgr->GetWLinkDataById(key);
	#else
					pLink = m_pDataMgr->GetWLinkDataById(pMesh->wlinks[jj]);
	#endif
				}
	#endif
#if defined(USE_FOREST_DATA)
				if (m_nDataType == TYPE_DATA_TREKKING) {
#ifdef TEST_SPATIALINDEX
					pLink = m_pDataMgr->GetFLinkDataById(key);
#else
					pLink = m_pDataMgr->GetFLinkDataById(pMesh->links[jj]);
#endif
				}
#endif

				if (!pLink) {
					LOG_TRACE(LOG_ERROR, "Failed, can't access link, idx:%d, tile_id:%d", jj, pMesh->mesh_id.tile_id);
					return 0;
				}

				if (pLink->sub_info == NOT_USE)
					continue;

				fileLink.link_id = pLink->link_id;
				fileLink.sub_info = pLink->sub_info;
				fileLink.snode_id = pLink->snode_id;
				fileLink.enode_id = pLink->enode_id;
				fileLink.length = pLink->length;
				fileLink.name_idx = pLink->name_idx;
				fileLink.cntVertex = pLink->getVertexCount();
	#if defined(USE_P2P_DATA) || defined(USE_MOUNTAIN_DATA)
				fileLink.sub_ext = pLink->sub_ext;
	#endif

				if ((retWrite = fwrite(&fileLink, sizeof(fileLink), 1, fp)) != 1) {
					LOG_TRACE(LOG_ERROR, "Failed, can't write link[%d], tile_id:%d, id:%d", jj, fileLink.link_id.tile_id, fileLink.link_id.nid);
					return 0;
				}

				offItem += sizeof(fileLink);


				// write vertex
				if (fileLink.cntVertex > 0) {
					if (!(retWrite = pLink->writeVertex(fp))) {
						LOG_TRACE(LOG_ERROR, "Failed, can't write link[%d] vertex, tile_id:%d, id:%d, cnt:%d", jj, pLink->link_id.tile_id, pLink->link_id.nid, fileLink.cntVertex);
						return 0;
					}
					offItem += retWrite;
				}

				retLink++;
			} // write link


			// re-write body size & off
			if (offItem > sizeFileBody) {
				fileBody.szData = offItem;

				fseek(fp, offItem * -1, SEEK_CUR);

				if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1) {
					LOG_TRACE(LOG_ERROR, "Failed, can't re-write node[%d], tile_id:%d, id:%d", ii, fileNode.node_id.tile_id, fileNode.node_id.nid);
					return 0;
				}

				fseek(fp, offItem - sizeFileBody, SEEK_CUR);
			}
		}		
		// re-set index size & off buff
		m_vtIndex[ii].szBody = offItem;
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
		if ((retWrite = fwrite(&fileIndex, sizeof(fileIndex), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't re-write file index data, idx:%d", idx);
			return 0;
		}
	}

	LOG_TRACE(LOG_DEBUG, "Save data, nodes, links : (before):%u,%u, (after)%u,%u", cntNode, cntLink, retNode, retLink);

	return offFile;
}


bool CFileBase::LoadData(IN const char* szFilePath)
{
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[m_nDataType], g_szTypeExec[m_nFileType]);

	size_t offFile = 0;
	size_t offItem = 0;
	size_t retRead = 0;


	// load data
	FILE* fp = fopen(szFileName, "rb");
	if (!fp)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't open file for load data, file:%s", szFileName);
		return false;
	}

	strcpy(m_szDataPath, szFileName);

	// read base
	retRead = ReadBase(fp);

	// check offset 
	if (retRead <= 0)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file base size zero, file:%s", szFileName);
		fclose(fp);
		return false;
	}


	// read header
	retRead = ReadHeader(fp);

	// check offset 
	if (retRead <= 0)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file header size zero, file:%s", szFileName);
		fclose(fp);
		return false;
	}


	// read index
	retRead = ReadIndex(fp);

	// check offset 
	if (retRead <= 0)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file index size zero, file:%s", szFileName);
		fclose(fp);
		return false;
	}

	// for cache
#if defined(USE_DATA_CACHE)
	//if (m_nDataType != TYPE_DATA_MESH) // 메쉬는 캐싱 인덱스이기에 바디를 읽어둠
	if (m_nDataType != TYPE_DATA_TRAFFIC) {
		return true;
	}
#endif

	// read body
	retRead = ReadBody(fp);

	// check offset 
	if (retRead <= 0)
	{
		LOG_TRACE(LOG_ERROR, "Failed, file body size zero, file:%s", szFileName);
		fclose(fp);
		return false;
	}

	fclose(fp);


	SetMeshRegion(&m_fileHeader.rtMap);

	return true;
}


bool CFileBase::LoadDataByIdx(IN const uint32_t idx)
{
	FILE* fp = fopen(m_szDataPath, "rb");
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, can't open file for load data, file:%s", m_szDataPath);
		return false;
	} else if (idx < 0 || m_vtIndex.size() <= idx) {
		LOG_TRACE(LOG_ERROR, "Failed, request load data idx range, max:%d, req idx:%d", m_vtIndex.size(), idx);
		fclose(fp);
		return false;
	}

	FileBody fileBody = { 0, };
	FileNode fileNode = { 0, };
	FileLink fileLink = { 0, };

	stMeshInfo* pMesh = nullptr;
	stNodeInfo* pNode = nullptr;
	stLinkInfo* pLink = nullptr;

	size_t offFile = 0;
	size_t offItem = 0;
	size_t retRead = 0;

	// read index

	// read body
	if (m_vtIndex[idx].offBody <= 0 /*|| m_vtIndex[idx].szBody <= 0*/) // 메쉬내 하위데이터가 없어도 메쉬 자체는 포함하자
	{
		LOG_TRACE(LOG_ERROR, "Failed, index body info invalid, off:%d, size:%d", m_vtIndex[idx].offBody, m_vtIndex[idx].szBody);
		fclose(fp);
		return false;
	}


	// 내용이 없으면 메쉬만 포함하자
	if (m_vtIndex[idx].szBody <= 0) {
		fclose(fp);
		return true;
	}


	fseek(fp, m_vtIndex[idx].offBody, SEEK_SET);
	if ((retRead = fread(&fileBody, sizeof(fileBody), 1, fp)) != 1) {
		LOG_TRACE(LOG_ERROR, "Failed, can't read body, offset:%d", m_vtIndex[idx].offBody);
		fclose(fp);
		return false;
	}

	if (m_vtIndex[idx].idTile != fileBody.idTile) {
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
	

	// node
	for (uint32_t ii = 0; ii < fileBody.net.cntNode; ii++)
	{
		// read node
		if ((retRead = fread(&fileNode, sizeof(fileNode), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read node, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return false;
		}

		//pMesh->nodes.emplace_back(fileNode.node_id);

		pNode = new stNodeInfo;
		pNode->node_id = fileNode.node_id;
		pNode->sub_info = fileNode.sub_info;
		pNode->edgenode_id = fileNode.edgenode_id;
		memcpy(&pNode->coord, &fileNode.coord, sizeof(pNode->coord));
		memcpy(pNode->connnodes, fileNode.connnodes, sizeof(KeyID) * pNode->base.connnode_count);
		memcpy(pNode->conn_attr, fileNode.conn_attr, sizeof(uint16_t) * pNode->base.connnode_count);

		AddNodeData(pNode);
	}

	// link
	for (uint32_t ii = 0; ii < fileBody.net.cntLink; ii++)
	{
		// read link
		if ((retRead = fread(&fileLink, sizeof(fileLink), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read link, mesh_id:%d, idx:%d", m_vtIndex[idx].idTile, ii);
			fclose(fp);
			return false;
		}

		//pMesh->links.emplace_back(fileLink.link_id);

		pLink = new stLinkInfo;
		pLink->link_id = fileLink.link_id;
		pLink->sub_info = fileLink.sub_info;
		pLink->snode_id = fileLink.snode_id;
		pLink->enode_id = fileLink.enode_id;
		pLink->length = fileLink.length;
		pLink->name_idx = fileLink.name_idx;
#if defined(USE_P2P_DATA) || defined(USE_MOUNTAIN_DATA)
		pLink->sub_ext = fileLink.sub_ext;
#endif

		if (!pLink->readVertex(fp, fileLink.cntVertex)) {
			LOG_TRACE(LOG_ERROR, "Failed, can't read vertex, mesh_id:%d, link_id:%d, link_idx:%d", pLink->link_id.tile_id, pLink->link_id.nid, ii);
			fclose(fp);
			return false;
		}
		//for (uint32_t jj = 0; jj < fileLink.cntVertex; jj++)
		//{
		//	// read link
		//	SPoint ptVertex;
		//	if ((retRead = fread(&ptVertex, sizeof(ptVertex), 1, fp)) != 1)
		//	{
		//		LOG_TRACE(LOG_ERROR, "Failed, can't read vertex, mesh_id:%d, link idx:%d, idx:%d", m_vtIndex[idx].idTile, ii, jj);
		//		fclose(fp);
		//		return false;
		//	}
		//	pLink->vtPts.emplace_back(ptVertex);
		//}

		AddLinkData(pLink);
	} // for

	fclose(fp);

#ifdef TEST_SPATIALINDEX
	if (pLink && fileBody.net.cntLink) {
		if (pLink->base.link_type == TYPE_LINK_DATA_TREKKING) { // 숲길
			m_pDataMgr->CreateSpatialindex(pMesh, TYPE_DATA_TREKKING);
		} else if (pLink->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) { // 보행자/자전거
			m_pDataMgr->CreateSpatialindex(pMesh, TYPE_DATA_PEDESTRIAN);
		} else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) { // 자동차
			m_pDataMgr->CreateSpatialindex(pMesh, TYPE_DATA_VEHICLE);
		}
	}
#endif

	return true;
}


size_t CFileBase::ReadBase(FILE* fp)
{
	size_t offFile = 0;
	size_t retRead = 0;
	FileBase stFileBase = { 0, };
	const size_t sizeBase = sizeof(FileBase);

	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	if ((retRead = fread(&stFileBase, sizeBase, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't read base, read:%d", retRead);
		return 0;
	}

	offFile = sizeBase;

	LOG_TRACE(LOG_DEBUG, "Read data, base, type:%s, ver:%d.%d.%d.%d, size:%d",
		stFileBase.szType, stFileBase.version[0], stFileBase.version[1], stFileBase.version[2], stFileBase.version[3], sizeBase);

	if (m_pDataMgr != nullptr) {
		m_pDataMgr->SetVersion(GetDataType(), stFileBase.version[0], stFileBase.version[1], stFileBase.version[2], stFileBase.version[3]);
	}

	return offFile;
}


size_t CFileBase::ReadHeader(FILE* fp)
{
	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeHeader = sizeof(m_fileHeader);

	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	if ((retRead = fread(&m_fileHeader, sizeHeader, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't read header, cnt:%d", retRead);
		return 0;
	}

	offFile = sizeHeader;	


	LOG_TRACE(LOG_DEBUG, "Read data, header, size:%d, index cnt:%d", sizeHeader, m_fileHeader.cntIndex);

	return offFile;
}


size_t CFileBase::ReadIndex(FILE* fp)
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
	}

	LOG_TRACE(LOG_DEBUG, "Read data, index, type:%s, size:%d", GetDataTypeName(), offFile);

	return offFile;
}


size_t CFileBase::ReadBody(FILE* fp)
{	
#if 0 // using omp
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody = { 0, };
	FileNode fileNode = { 0, };
	FileLink fileLink = { 0, };

	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeBody = sizeof(FileBody);

	stMeshInfo* pMesh;
	stNodeInfo* pNode;
	stLinkInfo* pLink;

	size_t nTotalLinks = 0;
	size_t nTotalNodes = 0;

	//for (vector<FileIndex>::const_iterator it = m_vtIndex.begin(); it != m_vtIndex.end(); it++)
	for (int ii = 0; ii < m_vtIndex.size(); ii++)
	{
		// read body
		if (m_vtIndex[ii].szBody <= 0) {
			continue;
		}
		else if (m_vtIndex[ii].offBody <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, index body info invalid, off:%d, size:%d", m_vtIndex[ii].offBody, m_vtIndex[ii].szBody);
			return 0;
		}

		fseek(fp, m_vtIndex[ii].offBody, SEEK_SET);
		if ((retRead = fread(&fileBody, sizeBody, 1, fp)) != 1) {
			LOG_TRACE(LOG_ERROR, "Failed, can't read body, offset:%d", m_vtIndex[ii].offBody);
			return 0;
		}

		if (m_vtIndex[ii].idTile != m_vtIndex[ii].idTile) {
			LOG_TRACE(LOG_ERROR, "Failed, index tile info not match with body, index tile id:%d vs body tile id:%d", m_vtIndex[ii].idTile, m_vtIndex[ii].idTile);
			return 0;
		}

		// mesh
		pMesh = GetMeshDataById(m_vtIndex[ii].idTile);
		if (!pMesh) {
			pMesh = new stMeshInfo;

			pMesh->mesh_id.tile_id = m_vtIndex[ii].idTile;
			memcpy(&pMesh->mesh_box, &m_vtIndex[ii].rtTile, sizeof(pMesh->mesh_box));
			memcpy(&pMesh->data_box, &m_vtIndex[ii].rtData, sizeof(pMesh->data_box));
			for (int ii = 0; ii < 8; ii++) {
				if (m_vtIndex[ii].idNeighborTile[ii] <= 0) {
					continue;
				}
				pMesh->neighbors.emplace(m_vtIndex[ii].idNeighborTile[ii]);
			} // for
			AddMeshData(pMesh);
		}
		else {
			// extend rect
			//memcpy(&pMesh->data_box, &m_vtIndex[idx].rtData, sizeof(pMesh->data_box));
			for (int ii = 0; ii < 8; ii++) {
				if (m_vtIndex[ii].idNeighborTile[ii] <= 0) {
					continue;
				}
				pMesh->neighbors.emplace(m_vtIndex[ii].idNeighborTile[ii]);
			} // for
		}


		// node
		for (uint32_t ii = 0; ii < fileBody.link.cntNode; ii++) {
			// read node
			if ((retRead = fread(&fileNode, sizeof(fileNode), 1, fp)) != 1) {
				LOG_TRACE(LOG_ERROR, "Failed, can't read node, mesh_id:%d, idx:%d", m_vtIndex[ii].idTile, ii);
				return 0;
			}

			//pMesh->nodes.push_back(fileNode.node_id);

			pNode = new stNodeInfo;
			pNode->node_id = fileNode.node_id;
			pNode->sub_info = fileNode.sub_info;
			pNode->edgenode_id = fileNode.edgenode_id;
			memcpy(&pNode->coord, &fileNode.coord, sizeof(pNode->coord));
			memcpy(pNode->connnodes, fileNode.connnodes, sizeof(KeyID) * pNode->base.connnode_count);
			memcpy(pNode->conn_attr, fileNode.conn_attr, sizeof(uint16_t) * pNode->base.connnode_count);

			AddNodeData(pNode);
		}
		nTotalNodes += fileBody.link.cntNode;
#if defined(_DEBUG)
		//LOG_TRACE(LOG_DEBUG, "Data loading, node data loaded, mesh:%d, node cnt:%lld", pMesh->mesh_id.tile_id, fileBody.link.cntNode);
#endif

		// link
		for (uint32_t ii = 0; ii < fileBody.link.cntLink; ii++) {
			// read link
			if ((retRead = fread(&fileLink, sizeof(fileLink), 1, fp)) != 1) {
				LOG_TRACE(LOG_ERROR, "Failed, can't read link, mesh_id:%d, idx:%d", m_vtIndex[ii].idTile, ii);
				return 0;
			}

			if (!fileLink.cntVertex) {
				LOG_TRACE(LOG_WARNING, "Warning, link vertex empty, mesh_id:%d, link_id:%d, link_idx:%d, idx:%d", fileLink.link_id.tile_id, fileLink.link_id.nid, ii);
				continue;
			}
			//pMesh->links.emplace_back(fileLink.link_id);

			pLink = new stLinkInfo;
			pLink->link_id = fileLink.link_id;
			pLink->sub_info = fileLink.sub_info;
			//pLink->link_idx = stLink.link_idx;
			pLink->snode_id = fileLink.snode_id;
			pLink->enode_id = fileLink.enode_id;
			pLink->length = fileLink.length;
			pLink->name_idx = fileLink.name_idx;
#if defined(USE_P2P_DATA) || defined(USE_MOUNTAIN_DATA)
			pLink->sub_ext = fileLink.sub_ext;
#endif

			if (!pLink->readVertex(fp, fileLink.cntVertex)) {
				LOG_TRACE(LOG_ERROR, "Failed, can't read vertex, mesh_id:%d, link_id:%d, link_idx:%d, idx:%d", pLink->link_id.tile_id, pLink->link_id.nid, ii);
				return 0;
			}
			//for (uint32_t jj = 0; jj < fileLink.cntVertex; jj++)
			//{
			//	// read link
			//	SPoint ptVertex;
			//	if ((retRead = fread(&ptVertex, sizeof(ptVertex), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't read vertex, mesh_id:%d, link idx:%d, idx:%d", m_vtIndex[ii].idTile, ii, jj);
			//		return 0;
			//	}
			//	pLink->vtPts.emplace_back(ptVertex);
			//}

			AddLinkData(pLink);
		} // for

		offFile += m_vtIndex[ii].szBody;

		nTotalLinks += fileBody.link.cntLink;
#if defined(_DEBUG)
		//LOG_TRACE(LOG_DEBUG, "Data loading, link data loaded, mesh:%d, link cnt:%lld", pMesh->mesh_id.tile_id, fileBody.link.cntLink);
#endif
	} // for

	LOG_TRACE(LOG_DEBUG, "Data loading, mesh cnt:%lld, node cnt:%lld link cnt:%lld", m_vtIndex.size(), nTotalNodes, nTotalLinks);

	return offFile;

#else // using omp
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody = { 0, };
	FileNode fileNode = { 0, };
	FileLink fileLink = { 0, };

	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeBody = sizeof(FileBody);

	stMeshInfo* pMesh = nullptr;
	stNodeInfo* pNode = nullptr;
	stLinkInfo* pLink = nullptr;

	size_t nTotalLinks = 0;
	size_t nTotalNodes = 0;

	for (vector<FileIndex>::const_iterator it = m_vtIndex.begin(); it != m_vtIndex.end(); it++)
	{
		if (!checkTestMesh(it->idTile)) {
			continue;
		}

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
		pMesh = GetMeshDataById(it->idTile);
		if (!pMesh) {
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
		}
		else
		{
			// extend rect
			//memcpy(&pMesh->data_box, &m_vtIndex[idx].rtData, sizeof(pMesh->data_box));
			for (int ii = 0; ii < 8; ii++) {
				if (it->idNeighborTile[ii] <= 0) {
					continue;
				}
				pMesh->neighbors.emplace(it->idNeighborTile[ii]);
			} // for
		}


		// node
		for (uint32_t ii = 0; ii < fileBody.net.cntNode; ii++)
		{
			// read node
			if ((retRead = fread(&fileNode, sizeof(fileNode), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read node, mesh_id:%d, idx:%d", it->idTile, ii);
				return 0;
			}

			//pMesh->nodes.push_back(fileNode.node_id);

			pNode = new stNodeInfo;
			pNode->node_id = fileNode.node_id;
			pNode->sub_info = fileNode.sub_info;
			pNode->edgenode_id = fileNode.edgenode_id;
			memcpy(&pNode->coord, &fileNode.coord, sizeof(pNode->coord));
			memcpy(pNode->connnodes, fileNode.connnodes, sizeof(KeyID) * pNode->base.connnode_count);
			memcpy(pNode->conn_attr, fileNode.conn_attr, sizeof(uint16_t) * pNode->base.connnode_count);

			AddNodeData(pNode);
		}
		nTotalNodes += fileBody.net.cntNode;
#if defined(_DEBUG)
		//LOG_TRACE(LOG_DEBUG, "Data loading, node data loaded, mesh:%d, node cnt:%lld", pMesh->mesh_id.tile_id, fileBody.link.cntNode);
#endif

		// link
		for (uint32_t ii = 0; ii < fileBody.net.cntLink; ii++)
		{
			// read link
			if ((retRead = fread(&fileLink, sizeof(fileLink), 1, fp)) != 1)
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't read link, mesh_id:%d, idx:%d", it->idTile, ii);
				return 0;
			}

			if (!fileLink.cntVertex) {
				LOG_TRACE(LOG_WARNING, "Warning, link vertex empty, mesh_id:%d, link_id:%d, link_idx:%d, idx:%d", fileLink.link_id.tile_id, fileLink.link_id.nid, ii);
				continue;
			}
			//pMesh->links.emplace_back(fileLink.link_id);

			pLink = new stLinkInfo;
			pLink->link_id = fileLink.link_id;
			pLink->sub_info = fileLink.sub_info;
			//pLink->link_idx = stLink.link_idx;
			pLink->snode_id = fileLink.snode_id;
			pLink->enode_id = fileLink.enode_id;
			pLink->length = fileLink.length;
			pLink->name_idx = fileLink.name_idx;
#if defined(USE_P2P_DATA) || defined(USE_MOUNTAIN_DATA)
			pLink->sub_ext = fileLink.sub_ext;
#endif

			if (!pLink->readVertex(fp, fileLink.cntVertex)) {
				LOG_TRACE(LOG_ERROR, "Failed, can't read vertex, mesh_id:%d, link_id:%d, link_idx:%d, idx:%d", pLink->link_id.tile_id, pLink->link_id.nid, ii);
				return 0;
			}
			//for (uint32_t jj = 0; jj < fileLink.cntVertex; jj++)
			//{
			//	// read link
			//	SPoint ptVertex;
			//	if ((retRead = fread(&ptVertex, sizeof(ptVertex), 1, fp)) != 1)
			//	{
			//		LOG_TRACE(LOG_ERROR, "Failed, can't read vertex, mesh_id:%d, link idx:%d, idx:%d", it->idTile, ii, jj);
			//		return 0;
			//	}
			//	pLink->vtPts.emplace_back(ptVertex);
			//}

			AddLinkData(pLink);
		} // for

		offFile += it->szBody;

		nTotalLinks += fileBody.net.cntLink;

#ifdef TEST_SPATIALINDEX
		if (fileBody.net.cntLink) {
			if (pLink->base.link_type == TYPE_LINK_DATA_TREKKING) { // 숲길
				m_pDataMgr->CreateSpatialindex(pMesh, TYPE_DATA_TREKKING);
			} else if (pLink->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) { // 보행자/자전거
				m_pDataMgr->CreateSpatialindex(pMesh, TYPE_DATA_PEDESTRIAN);
			} else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) { // 자동차
				m_pDataMgr->CreateSpatialindex(pMesh, TYPE_DATA_VEHICLE);
			}
		}
#endif

#if defined(_DEBUG)
		//LOG_TRACE(LOG_DEBUG, "Data loading, link data loaded, mesh:%d, link cnt:%lld", pMesh->mesh_id.tile_id, fileBody.link.cntLink);
#endif
	} // for

	LOG_TRACE(LOG_DEBUG, "Read data, body, node cnt:%lld, link cnt:%lld", nTotalNodes, nTotalLinks);

	return offFile;
#endif // // using omp
}


void CFileBase::AddDataFeild(IN const int idx, IN const int type, IN const char* colData)
{

}


void CFileBase::AddDataRecord()
{

}


uint32_t CFileBase::AddNameDic(IN char* pData)
{
	if (m_pNameMgr) {
		return m_pNameMgr->AddData(pData);
	}
	
	return 0;
}


const char* CFileBase::GetErrorMsg()
{
	if (!m_strErrMsg.empty())
		return m_strErrMsg.c_str();

	return nullptr;
}


bool CFileBase::CheckDataInMesh(IN const double x, IN const double y)
{
	bool ret = false;

	if (g_isUseTestMesh && !g_arrTestMesh.empty()) {
		stMeshInfo* pMesh = nullptr;

		for (const auto& meshId : g_arrTestMesh) {
			pMesh = GetMeshDataById(meshId);
			if (pMesh && isInBox(x, y, pMesh->mesh_box)) {
				ret = true;
				break;
			}
		} // for
	}

	return ret;
}

