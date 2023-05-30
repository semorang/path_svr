#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "RouteManager.h"
#include "MMPoint.hpp"
#include "../utils/UserLog.h"
#include "../utils/GeoTools.h"

#define VLINK 1

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CRouteManager::CRouteManager()
{
	m_pDataMgr = nullptr;
	m_pRoutePlan = new CRoutePlan();


	memset(&m_ptDeparture, 0x00, sizeof(SPoint));
	memset(&m_ptNewDeparture, 0x00, sizeof(SPoint));
	//memset(m_ptWaypoint, 0x00, sizeof(SPoint) * 10);
	memset(&m_ptDestination, 0x00, sizeof(SPoint));
	memset(&m_ptNewDestination, 0x00, sizeof(SPoint));

	m_nRouteOpt = 0;
	m_nAvoidOpt = 0;
}


CRouteManager::~CRouteManager()
{
	Release();

	if (m_pRoutePlan) {
		delete m_pRoutePlan;
	}
}


bool CRouteManager::Initialize(void)
{
	if (m_pRoutePlan) {
		m_pRoutePlan->Initialize();
	}

	m_routeResult.Init();

	InitPoints();

	m_nRouteOpt = 0;
	m_nAvoidOpt = 0;

	return true;
}

void CRouteManager::Release(void)
{
	if (m_pRoutePlan) {
		m_pRoutePlan->Release();
	}

	if (!m_vtRouteInfo.empty()) {
		m_vtRouteInfo.clear();
		vector<RouteInfo>().swap(m_vtRouteInfo);
	}

	if (!m_vtRouteResult.empty()) {
		m_vtRouteResult.clear();
		vector<RouteResultInfo>().swap(m_vtRouteResult);
	}
}


void CRouteManager::SetDataMgr(CDataManager* pDataMgr)
{
	if (pDataMgr)
	{
		m_pDataMgr = pDataMgr;
		m_pRoutePlan->SetDataMgr(pDataMgr);
	}
}


void CRouteManager::InitPoints(void)
{
	memset(&m_ptDeparture, 0x00, sizeof(m_ptDeparture));
	memset(&m_ptNewDeparture, 0x00, sizeof(m_ptNewDeparture));
	if (!m_ptWaypoints.empty()) {
		m_ptWaypoints.clear();
		vector<SPoint>().swap(m_ptWaypoints);
	}

	memset(&m_ptDestination, 0x00, sizeof(m_ptDestination));
	memset(&m_ptNewDestination, 0x00, sizeof(m_ptNewDestination));
	if (!m_ptNewWaypoints.empty()) {
		m_ptNewWaypoints.clear();
		vector<SPoint>().swap(m_ptNewWaypoints);
	}

	keyDeparture.llid = NULL_VALUE;
	keyDestination.llid = NULL_VALUE;
	if (!keyWaypoints.empty()) {
		keyWaypoints.clear();
		vector<KeyID>().swap(keyWaypoints);
	}
}


KeyID CRouteManager::SetDeparture(IN const double lng, IN const double lat, IN const int matchType)
{
	if (m_pDataMgr != nullptr) {
		double retDist = INT_MAX;
		m_ptDeparture.x = m_ptNewDeparture.x = lng;
		m_ptDeparture.y = m_ptNewDeparture.y = lat;

		for(int ii=0; ii< MAX_SEARCH_RANGE; ii++) {
			stLinkInfo* pLink = m_pDataMgr->GetLinkDataByPointAround(lng, lat, searchRange[ii], m_ptNewDeparture.x, m_ptNewDeparture.y, retDist, matchType);
			if (pLink) {
				keyDeparture = pLink->link_id;

				if (ii > 0) {
					LOG_TRACE(LOG_DEBUG, "departure projection, link id:%d, range level:%d, dist:%d", keyDeparture.nid, ii, retDist);
				}
				break;
			}
		} // for
	}

	return keyDeparture;
}


KeyID CRouteManager::SetWaypoint(IN const double lng, IN const double lat, IN const int matchType)
{
	KeyID keyWaypoint = { NULL_VALUE };

	if (m_pDataMgr != nullptr) {
		if (m_ptWaypoints.size() < MAX_WAYPOINT)
		{
			double retDist = INT_MAX;
			SPoint waypoint = { lng, lat };
			m_ptWaypoints.emplace_back(waypoint);

			for (int ii = 0; ii< MAX_SEARCH_RANGE; ii++) {
				stLinkInfo* pLink = m_pDataMgr->GetLinkDataByPointAround(lng, lat, searchRange[ii], waypoint.x, waypoint.y, retDist, matchType);
				if (pLink) {
					keyWaypoint = pLink->link_id;

					if (ii > 0) {
						LOG_TRACE(LOG_DEBUG, "waypoint projection, link id:%d, idx:%d, range level:%d, dist:%d", keyWaypoint.nid, keyWaypoints.size(), ii, retDist);
					}

					break;
				}
			} // for

			keyWaypoints.emplace_back(keyWaypoint);
			m_ptNewWaypoints.emplace_back(waypoint);
		}
	}

	return keyWaypoint;
}


KeyID CRouteManager::SetDestination(IN const double lng, IN const double lat, IN const int matchType)
{
	if (m_pDataMgr != nullptr) {
		double retDist = INT_MAX;
		m_ptDestination.x = m_ptNewDestination.x = lng;
		m_ptDestination.y = m_ptNewDestination.y = lat;

		for(int ii=0; ii< MAX_SEARCH_RANGE; ii++) {

			stLinkInfo* pLink = m_pDataMgr->GetLinkDataByPointAround(lng, lat, searchRange[ii], m_ptNewDestination.x, m_ptNewDestination.y, retDist, matchType);
			if (pLink) {
				keyDestination = pLink->link_id;

				if (ii > 0) {
					LOG_TRACE(LOG_DEBUG, "destination projection, link id:%d, range level:%d, dist:%d", keyDestination.nid, ii, retDist);
				}
				break;
			}
		} // for
	}

	return keyDestination;
}


void CRouteManager::SetRouteOption(IN const uint32_t route, IN const uint32_t avoid)
{
	m_nRouteOpt = route;
	m_nAvoidOpt = avoid;
}


int CRouteManager::GetWayPointCount(void)
{
	return m_ptWaypoints.size();
}


KeyID CRouteManager::GetPositionLink(IN const double lng, IN const double lat, IN const int matchType)
{
	KeyID retKey = {NULL_VALUE};

	if (m_pDataMgr != nullptr) {
		double retDist = INT_MAX;
		SPoint newPos;

		for (int ii = 0; ii< MAX_SEARCH_RANGE; ii++) {
			stLinkInfo* pLink = m_pDataMgr->GetLinkDataByPointAround(lng, lat, searchRange[ii], newPos.x, newPos.y, retDist, matchType);
			if (pLink) {
				keyDestination = pLink->link_id;

				if (ii > 0) {
					LOG_TRACE(LOG_DEBUG, "location link projection, link id:%d, range level:%d, dist:%d", keyDestination.nid, ii, retDist);
				}
				break;
			}
		} // for
	}

	return retKey;
}


SPoint* CRouteManager::GetDeparture(void)
{
	if (m_ptDeparture.x == 0 || m_ptDeparture.y == 0)
	{
		return nullptr;
	}
	return &m_ptDeparture;
}

SPoint* CRouteManager::GetWaypoint(IN const uint32_t idx)
{
	if (MAX_WAYPOINT <= idx)
	{
		return nullptr;
	}

	if (m_ptWaypoints[idx].x == 0 || m_ptWaypoints[idx].y == 0)
	{
		return nullptr;
	}
	return &m_ptWaypoints[idx];
}

SPoint* CRouteManager::GetDestination(void)
{
	if (m_ptDestination.x == 0 || m_ptDestination.y == 0)
	{
		return nullptr;
	}
	return &m_ptDestination;
}


const RouteResultInfo* CRouteManager::GetRouteResult(void) const
{
	return &m_routeResult;
}


const uint32_t CRouteManager::GetRouteResultsCount(void) const
{
	return m_vtRouteInfo.size();
}


const RouteResultInfo* CRouteManager::GetRouteResults(IN const uint32_t idx) const
{
	if (idx < m_vtRouteInfo.size()) {
		return &m_vtRouteResult[idx];
	}

	return nullptr;
}


const size_t CRouteManager::GetRouteProbablePath(OUT vector<RouteProbablePath*>& vtJctInfo, IN const double length, IN const int32_t expansion, IN const double branchLength)
{
	// 경로선 노드의 경로선 외 링크 정보
	const RouteResultInfo* pRouteResult = GetRouteResult();

	stLinkInfo* pLinkBefore = nullptr;
	stLinkInfo* pLinkNext = nullptr;

	for (int ii = 0; ii < pRouteResult->LinkInfo.size() - 1; ii++) {
#if defined(USE_TREKKING_DATA)
		pLinkBefore = m_pDataMgr->GetLinkDataById(pRouteResult->LinkInfo[ii].link_id);
		pLinkNext = m_pDataMgr->GetLinkDataById(pRouteResult->LinkInfo[ii + 1].link_id);
#elif defined(USE_PEDESTRIAN_DATA)
		pLinkBefore = m_pDataMgr->GetWLinkDataById(pRouteResult->LinkInfo[ii].link_id);
		pLinkNext = m_pDataMgr->GetWLinkDataById(pRouteResult->LinkInfo[ii + 1].link_id);
#elif defined(USE_VEHICLE_DATA)
		pLinkBefore = m_pDataMgr->GetVLinkDataById(pRouteResult->LinkInfo[ii].link_id);
		pLinkNext = m_pDataMgr->GetVLinkDataById(pRouteResult->LinkInfo[ii + 1].link_id);
#else
		pLinkBefore = m_pDataMgr->GetLinkDataById(pRouteResult->LinkInfo[ii].link_id);
		pLinkNext = m_pDataMgr->GetLinkDataById(pRouteResult->LinkInfo[ii + 1].link_id);
#endif

		if (pLinkBefore == nullptr) {
			LOG_TRACE(LOG_WARNING, "failed, can't find fore link, idx:%d, id:%d", ii, pRouteResult->LinkInfo[ii].link_id);
			// return 0;
			continue;
		} else if (pLinkNext == nullptr) {
			LOG_TRACE(LOG_WARNING, "failed, can't find next link, idx:%d, id:%d", ii + 1, pRouteResult->LinkInfo[ii + 1].link_id);
			// return 0;
			continue;
		}

		stNodeInfo* pJctNode = nullptr;
		stLinkInfo* pJctLink = nullptr;

		// get junction node
		// <--- --->, <--- <---
		if (((pLinkBefore->getVertexX(0) == pLinkNext->getVertexX(0)) &&
			(pLinkBefore->getVertexY(0) == pLinkNext->getVertexY(0))) ||
			((pLinkBefore->getVertexX(0) == pLinkNext->getVertexX(pLinkNext->getVertexCount() - 1)) &&
				(pLinkBefore->getVertexY(0) == pLinkNext->getVertexY(pLinkNext->getVertexCount() - 1)))) {
#if defined(USE_TREKKING_DATA)
			pJctNode = m_pDataMgr->GetNodeDataById(pLinkBefore->snode_id);
#elif defined(USE_PEDESTRIAN_DATA)
			pJctNode = m_pDataMgr->GetWNodeDataById(pLinkBefore->snode_id);
#elif defined(USE_VEHICLE_DATA)
			pJctNode = m_pDataMgr->GetVNodeDataById(pLinkBefore->snode_id);
#else
			pJctNode = m_pDataMgr->GetNodeDataById(pLinkBefore->snode_id);
#endif			
		}
		// ---> <--- , ---> --->
		else if (((pLinkBefore->getVertexX(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexX(pLinkNext->getVertexCount() - 1)) &&
			(pLinkBefore->getVertexY(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexY(pLinkNext->getVertexCount() - 1))) ||
			((pLinkBefore->getVertexX(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexX(0)) &&
				(pLinkBefore->getVertexY(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexY(0)))) {
#if defined(USE_TREKKING_DATA)
			pJctNode = m_pDataMgr->GetNodeDataById(pLinkBefore->enode_id);
#elif defined(USE_PEDESTRIAN_DATA)
			pJctNode = m_pDataMgr->GetWNodeDataById(pLinkBefore->enode_id);
#elif defined(USE_VEHICLE_DATA)
			pJctNode = m_pDataMgr->GetVNodeDataById(pLinkBefore->enode_id);
#else
			pJctNode = m_pDataMgr->GetNodeDataById(pLinkBefore->enode_id);
#endif							
		}

		if (pJctNode == nullptr) {
			LOG_TRACE(LOG_WARNING, "failed, can't find junction node info, fore link:%d, next link:%d", pLinkBefore->link_id.nid, pLinkNext->link_id.nid);
			continue;
		}
		
		RouteProbablePath* pJctInfo = new RouteProbablePath;
		pJctInfo->LinkId = pLinkBefore->link_id;
		pJctInfo->NodeId = pJctNode->node_id;
		
		if (pJctNode->base.connnode_count > 2) {
			for (int jj = 0; jj<pJctNode->base.connnode_count; jj++) {
				// 경로에 사용된 링크는 제외
				if (pJctNode->connnodes[jj] == pLinkBefore->link_id || pJctNode->connnodes[jj] == pLinkNext->link_id) {
					continue;
				}

				// 경로선 외 링크
#if defined(USE_TREKKING_DATA)
				pJctLink = m_pDataMgr->GetLinkDataById(pJctNode->connnodes[jj]);
#elif defined(USE_PEDESTRIAN_DATA)
				pJctLink = m_pDataMgr->GetWLinkDataById(pJctNode->connnodes[jj]);
#elif defined(USE_VEHICLE_DATA)
				pJctLink = m_pDataMgr->GetVLinkDataById(pJctNode->connnodes[jj]);
#else
				pJctLink = m_pDataMgr->GetLinkDataById(pJctNode->connnodes[jj]);
#endif				

				stLinkInfo* pLinkInfo = new stLinkInfo;
				pLinkInfo->link_id = pJctLink->link_id;
				pLinkInfo->length = pJctLink->length;
				pLinkInfo->snode_id = pJctNode->node_id;
				
				bool isReverse = false;

				// 링크 방향성 // 버텍스 순서 변경
				// enode, * <---
				if ((pJctNode->coord.x == pJctLink->getVertexX(pJctLink->getVertexCount() - 1)) &&
					(pJctNode->coord.y == pJctLink->getVertexY(pJctLink->getVertexCount() - 1))) {
					isReverse = true;
				}
				//// snode, <--- --->, <--- <---
				//else if (((vtxNextNode.x == pJctLink->getVertexX(0)) &&
				//	(vtxNextNode.y == pJctLink->getVertexY(0))) ||
				//	((vtxNextNode.x == pJctLink->getVertexX(pJctLink->getVertexCount() - 1)) &&
				//		(vtxNextNode.y == pJctLink->getVertexY(pJctLink->getVertexCount() - 1)))) {
				//	;
				//}			

				// 노드로부터 일정 거리 만큼만 사용
				if (branchLength <= 0 || pJctLink->length <= branchLength || pJctLink->getVertexCount() <= 2)  {
					pLinkInfo->setVertex(pJctLink->getVertex(), pJctLink->getVertexCount());
					if (isReverse) {
						pLinkInfo->reverseVertex();
					}
					pJctInfo->JctLinks.push_back(pLinkInfo);
				}
				else {
					// 거리 따져서 링크 계산하는 코드는 앱에서 진행하자
					double jctLength = 0.f;
					int idxVtx = -1;
					int cntVertex = 1; // 기본 시작 vtx는 포함됨
					
					if (isReverse) {
						for (idxVtx = pJctLink->getVertexCount() - 2; idxVtx >= 0; --idxVtx, cntVertex++) {
							jctLength += getRealWorldDistance(pJctLink->getVertexX(idxVtx + 1), pJctLink->getVertexY(idxVtx + 1), pJctLink->getVertexX(idxVtx), pJctLink->getVertexY(idxVtx));
							if (jctLength >= branchLength) {
								cntVertex++;
								break;
							}
						} // for kk
					}
					else {
						for (idxVtx = 1; idxVtx < pJctLink->getVertexCount() - 1; idxVtx++, cntVertex++) {
							jctLength += getRealWorldDistance(pJctLink->getVertexX(idxVtx - 1), pJctLink->getVertexY(idxVtx - 1), pJctLink->getVertexX(idxVtx), pJctLink->getVertexY(idxVtx));
							if (jctLength >= branchLength) {
								cntVertex++;
								break;
							}
						} // for kk
					}

					if (idxVtx >= 0 && cntVertex > 0) {						
						if (isReverse) {
							pLinkInfo->setVertex(pJctLink->getVertex() + idxVtx, cntVertex);
							pLinkInfo->reverseVertex();
						}
						else {
							pLinkInfo->setVertex(pJctLink->getVertex(), cntVertex);
						}
						pJctInfo->JctLinks.push_back(pLinkInfo);
					}
				}
			} // for jj
		}
		else {
			// 정션 없지만 link_info 인덱스 맞추기 위해 삽입
			;
		}

		vtJctInfo.emplace_back(pJctInfo);
	} // for ii

	return vtJctInfo.size();
}


#if defined(USE_TSP_MODULE)
bool CRouteManager::GetBestWaypointResult(TspOptions* pOpt, IN const RouteTable** ppResultTables)
{
	vector<stCity> vtRawData;
	int32_t idx = 0;

	if (m_ptDeparture.has()) {
		vtRawData.push_back({ idx++, m_ptDeparture.x, m_ptDeparture.y });
	}

	for (vector<SPoint>::iterator it = m_ptWaypoints.begin(); it != m_ptWaypoints.end(); it++) {
		vtRawData.push_back({ idx++, it->x, it->y });
	}

	if (m_ptDestination.has()) {
		vtRawData.push_back({ idx++, m_ptDestination.x, m_ptDestination.y });
	}

	LOG_TRACE(LOG_DEBUG, "start, best waypoint result, cnt:%d", vtRawData.size());


	vector<int32_t> vtViaResult;
	vtViaResult.reserve(vtRawData.size());

	if (pOpt->algorityhmType == 1) {
		auto result = mst_manhattan_branch_and_bound(ppResultTables, vtRawData.size(), 0);
		vtViaResult.assign(result.path.begin(), result.path.end());
	}
	else if (pOpt->algorityhmType == 2) {
		auto result = mst_manhattan_branch_and_bound2(ppResultTables, vtRawData.size(), 0);
		vtViaResult.assign(result.path.begin(), result.path.end());
	}
	else if (pOpt->algorityhmType == 3) {
		InitURandom();
		TEnvironment env;

		env.Npop = vtRawData.size();
		env.Nch = vtRawData.size() / 3.2;

		LOG_TRACE(LOG_DEBUG, "init, best waypoint result");
		env.define(vtRawData, ppResultTables);

		LOG_TRACE(LOG_DEBUG, "building, best waypoint result");
		env.doIt();

		LOG_TRACE(LOG_DEBUG, "end, best waypoint result");
		env.printOn(0);

		env.getBest(vtViaResult);
	}
	else {
		Environment newWorld;

		// 최대 세대 
		const int32_t maxGeneration = pOpt->loopCount;
		// 세대당 최대 개체수
		const int32_t maxPopulation = pOpt->individualSize;
		// 염색체당 최대 유전자수
		const int32_t maxGene = pOpt->geneSize;

		newWorld.SetOption(pOpt);

		newWorld.SetCostTable(ppResultTables, vtRawData.size());
		
		// 신세계
		newWorld.Genesis(maxGene, maxPopulation);

		// 반복횟수
		for (int ii = 0; ii < maxGeneration; ii++) {
			// 평가
			newWorld.Evaluation();
			//newWorld.Print();

			// 부모 선택
			vector<Parents> pairs;
			newWorld.Selection(pairs);
			//newWorld.Print();

			// 자식 생성
			newWorld.Crossover(pairs);
			//newWorld.Print();
		}

		newWorld.GetBest(vtViaResult);
	}
	
	LOG_TRACE(LOG_DEBUG, "rusult cnt: %d", vtViaResult.size());
	if (vtRawData.size() != vtViaResult.size()) {
		LOG_TRACE(LOG_DEBUG, "result count not match with orignal count, ori:%d != ret:%d", vtRawData.size(), vtViaResult.size());
		return false;
	}

	LOG_TRACE(LOG_DEBUG_LINE, "rusult id: ");
	uint32_t totalDist = 0;
	uint32_t idxFirst = 0;
	uint32_t idxPrev = 0;
	idx = 0;
	for (const auto& item : vtViaResult)
	{
		LOG_TRACE(LOG_DEBUG_CONTINUE, "%2d→", item);
		if (idx >= 1) {
			totalDist += ppResultTables[idxPrev][item].nTotalDist;
		}
		else {
			idxFirst = idx;
		}

		idxPrev = item;
		idx++;
	}
	totalDist += ppResultTables[idxPrev][idxFirst].nTotalDist;
	LOG_TRACE(LOG_DEBUG_CONTINUE, "%d => %d\n", idxFirst, totalDist);

	// 기존 포인터 삭제
	InitPoints();


	int curIdx = 0;


	// 첫번째값을 출발지로 변경
	int newIdx = vtViaResult[curIdx];

	LOG_TRACE(LOG_DEBUG, "departure will change, idx(%d) %.5f, %.5f -> idx(%d) %.5f, %.5f",
		curIdx, vtRawData[curIdx].x, vtRawData[curIdx].y,
		newIdx, vtRawData[newIdx].x, vtRawData[newIdx].y);

	SetDeparture(vtRawData[newIdx].x, vtRawData[newIdx].y);
	curIdx++;


	// 마지막값을 목적지로 변경
	curIdx = vtViaResult.size() - 1;
	newIdx = vtViaResult[curIdx];

	LOG_TRACE(LOG_DEBUG, "destination will change, idx(%d) %.5f, %.5f -> idx(%d) %.5f, %.5f",
		curIdx, vtRawData[curIdx].x, vtRawData[curIdx].y,
		newIdx, vtRawData[newIdx].x, vtRawData[newIdx].y);

	SetDestination(vtRawData[newIdx].x, vtRawData[newIdx].y);

	curIdx--;


	// 경유지 변경
	for (int ii = 1; ii <= curIdx; ii++) {
		newIdx = vtViaResult[ii];

		LOG_TRACE(LOG_DEBUG, "waypoint will change, idx(%d) %.5f, %.5f -> idx(%d) %.5f, %.5f",
			ii, vtRawData[ii].x, vtRawData[ii].y,
			newIdx, vtRawData[newIdx].x, vtRawData[newIdx].y);

		SetWaypoint(vtRawData[newIdx].x, vtRawData[newIdx].y);
	}
	
	return true;
}
#endif // #if defined(USE_TSP_MODULE)


int CRouteManager::Route(/*packet*/)
{
	int ret = -1;

	ret = SingleRoute();

	return ret;
}

int CRouteManager::SingleRoute()
{
	int ret = -1;


	ret = DoRouting();

	return ret;
}

int CRouteManager::Table(TspOptions* pOpt, IN RouteTable** ppResultTables)
{
	int ret = -1;

#if defined(USE_TSP_MODULE)
	ret = DoTabulate(pOpt, ppResultTables);
#endif 

	return ret;
}

int CRouteManager::DoRouting(/*Packet*/)
{
	int ret = -1;

	const uint32_t uid = 12345678;

	m_routeResult.Init();

	if (m_ptWaypoints.empty()) {
		if ((ret = m_pRoutePlan->DoRoute(uid, m_ptDeparture, m_ptDestination, keyDeparture, keyDestination, m_nRouteOpt, m_nAvoidOpt, &m_routeResult)) == 0)
		{
			;
		}
	}
	else {
		RequestRouteInfo reqInfo;
		reqInfo.RequestId = uid;
		reqInfo.RouteOption = m_nRouteOpt;
		reqInfo.AvoidOption = m_nAvoidOpt;

		// start 
		reqInfo.vtPoints.emplace_back(m_ptDeparture);
		reqInfo.vtIdLinks.emplace_back(keyDeparture);

		// waypoint
		if (!m_ptWaypoints.empty()) {
			for (int ii = 0; ii < m_ptWaypoints.size(); ii++) {
				reqInfo.vtPoints.emplace_back(m_ptWaypoints[ii]);
				reqInfo.vtIdLinks.emplace_back(keyWaypoints[ii]);
			}
		}

		// end
		reqInfo.vtPoints.emplace_back(m_ptDestination);
		reqInfo.vtIdLinks.emplace_back(keyDestination);


		if (!m_vtRouteInfo.empty()) {
			m_vtRouteInfo.clear();
			vector<RouteInfo>().swap(m_vtRouteInfo);
		}

		if (!m_vtRouteResult.empty()) {
			m_vtRouteResult.clear();
			vector<RouteResultInfo>().swap(m_vtRouteResult);
		}

		if ((ret = m_pRoutePlan->DoRoutes(&reqInfo, &m_vtRouteInfo, &m_vtRouteResult)) == 0)
		{
			m_routeResult.ResultCode = m_vtRouteResult[0].ResultCode; // 경로 결과 코드, 0:성공, 1~:실패

			m_routeResult.RequestMode = m_vtRouteResult[0].RequestMode; // 요청 모드
			m_routeResult.RequestId = m_vtRouteResult[0].RequestId; // 요청 ID

			m_routeResult.RouteOption = m_vtRouteResult[0].RouteOption; // 경로 옵션
			m_routeResult.RouteAvoid = m_vtRouteResult[0].RouteAvoid; // 경로 회피

			m_routeResult.StartResultLink = m_vtRouteResult[m_vtRouteResult.size() - 1].StartResultLink;
			m_routeResult.EndResultLink = m_vtRouteResult[0].EndResultLink;

			// for (int ii = m_vtRouteResult.size() - 1; ii >= 0; --ii) {
			for (int ii = 0; ii < m_vtRouteResult.size() - 1; ii++) {
				// 경로선
				boxMerge(m_routeResult.RouteBox, m_vtRouteResult[ii].RouteBox);
				linkMerge(m_routeResult.LinkVertex, m_vtRouteResult[ii].LinkVertex);

				m_routeResult.LinkInfo.reserve(m_routeResult.LinkInfo.size() + m_vtRouteResult[ii].LinkInfo.size());
				for (int jj = 0; jj < m_vtRouteResult[ii].LinkInfo.size(); jj++) {
					// 경유지가 있을 경우, 전체 경로를 합치면, 개별 offset 및 거리, 시간이 누적되어 변경되어야 함.
					// 작업 추가 필요
					m_routeResult.LinkInfo.emplace_back(m_vtRouteResult[ii].LinkInfo[jj]);
				}

				m_routeResult.TotalLinkDist += m_vtRouteResult[ii].TotalLinkDist; // 경로 전체 거리
				m_routeResult.TotalLinkCount += m_vtRouteResult[ii].TotalLinkCount; // 경로 전체 링크 수
				m_routeResult.TotalLinkTime += m_vtRouteResult[ii].TotalLinkTime;; // 경로 전체 소요 시간 (초)
			}
		}
	}


	return ret;
}


int CRouteManager::DoTabulate(TspOptions* pOpt, IN RouteTable** ppResultTables)
{
	int ret = -1;

	const uint32_t uid = 12345678;

	m_routeResult.Init();

	RequestRouteInfo reqInfo;
	reqInfo.RequestId = uid;
	reqInfo.RouteOption = m_nRouteOpt;
	reqInfo.AvoidOption = m_nAvoidOpt;

	// start 
	reqInfo.vtPoints.emplace_back(m_ptDeparture);
	reqInfo.vtIdLinks.emplace_back(keyDeparture);

	// waypoint
	if (!m_ptWaypoints.empty()) {
		for (int ii = 0; ii < m_ptWaypoints.size(); ii++) {
			reqInfo.vtPoints.emplace_back(m_ptWaypoints[ii]);
			reqInfo.vtIdLinks.emplace_back(keyWaypoints[ii]);
		}
	}

	// end
	reqInfo.vtPoints.emplace_back(m_ptDestination);
	reqInfo.vtIdLinks.emplace_back(keyDestination);



	// 지점 개수 만큼의 결과 테이블(n * n) 생성
	// create route table rows
	const int32_t cntPoints = reqInfo.vtPoints.size();
	RouteTable** resultTables = nullptr;
	
	if (ppResultTables != nullptr) {
		resultTables = const_cast<RouteTable**>(ppResultTables);
		ret = ROUTE_RESULT_SUCCESS;
	}
	else {
		resultTables = new RouteTable*[cntPoints];

		// create route table cols 
		for (int ii = 0; ii < cntPoints; ii++) {
			resultTables[ii] = new RouteTable[cntPoints];
		} // for

#if defined(USE_TSP_MODULE)
		ret = m_pRoutePlan->DoTabulate(&reqInfo, resultTables);
#endif
	}


	// get route table
	if (ret == ROUTE_RESULT_SUCCESS)
	{
#if defined(USE_TSP_MODULE)
		GetBestWaypointResult(pOpt, const_cast<const RouteTable**>(resultTables));
#endif

		if (!m_vtRouteResult.empty()) {

			m_routeResult.ResultCode = m_vtRouteResult[0].ResultCode; // 경로 결과 코드, 0:성공, 1~:실패

			m_routeResult.RequestMode = m_vtRouteResult[0].RequestMode; // 요청 모드
			m_routeResult.RequestId = m_vtRouteResult[0].RequestId; // 요청 ID

			m_routeResult.RouteOption = m_vtRouteResult[0].RouteOption; // 경로 옵션
			m_routeResult.RouteAvoid = m_vtRouteResult[0].RouteAvoid; // 경로 회피

			//m_routeResult.StartResultLink = m_vtRouteResult[m_vtRouteResult.size() - 1].StartResultLink;
			//m_routeResult.EndResultLink = m_vtRouteResult[0].EndResultLink;

			//for (int ii = m_vtRouteResult.size() - 1; ii >= 0; --ii) {
			//	// 경로선
			//	boxMerge(m_routeResult.RouteBox, m_vtRouteResult[ii].RouteBox);
			//	linkMerge(m_routeResult.LinkVertex, m_vtRouteResult[ii].LinkVertex);

			//	m_routeResult.LinkInfo.reserve(m_routeResult.LinkInfo.size() + m_vtRouteResult[ii].LinkInfo.size());
			//	for (int jj = 0; jj < m_vtRouteResult[ii].LinkInfo.size(); jj++) {
			//		// 경유지가 있을 경우, 전체 경로를 합치면, 개별 offset 및 거리, 시간이 누적되어 변경되어야 함.
			//		// 작업 추가 필요
			//		m_routeResult.LinkInfo.emplace_back(m_vtRouteResult[ii].LinkInfo[jj]);
			//	}

			//	m_routeResult.TotalLinkDist += m_vtRouteResult[ii].TotalLinkDist; // 경로 전체 거리
			//	m_routeResult.TotalLinkCount += m_vtRouteResult[ii].TotalLinkCount; // 경로 전체 링크 수
			//	m_routeResult.TotalLinkTime += m_vtRouteResult[ii].TotalLinkTime;; // 경로 전체 소요 시간 (초)
			//}
		}
	}


	// destroyed route table
	if (resultTables) {
		for (int ii = 0; ii < cntPoints; ii++) {
			SAFE_DELETE_ARR(resultTables[ii]);
		}
		SAFE_DELETE_ARR(resultTables);
	}

	return ret;
}


KeyID CRouteManager::Projection(IN const double lng, IN const double lat)
{
	KeyID idxLink = { 0, };

	//현재 성능최적화 적용 안됨......(데이터내에 모든 링크정보(vtx)와 비교함,  약 3.5초 소요되네...)    
	//모든 링크에 Vtx 정보와 비교하여 최단거리에 해당하는 링크를 선정.

	size_t cntMesh = 8, cntLink, cntVtx;
	stMeshInfo** pMesh = (stMeshInfo**)malloc(sizeof(stMeshInfo*) * cntMesh);
	cntMesh = m_pDataMgr->GetMeshDataByPoint(lng, lat, cntMesh, pMesh);

	if (!cntMesh) // find all
	{
		LOG_TRACE(LOG_ERROR, "Failed, PitInMesh, lng:%.f, lat:%.f", lng, lat);
		return idxLink;
	}
	else
	{
		//pMesh->links.size();
		//std::multimap<uint32_t, stLinkIDX>::iterator iter;
		double retLng, retLat;
		double ir;
		stLinkInfo* pLink;
		PT reqCoord = { lng, lat };
		double minDistance = INT_MAX;

		for (unsigned int ii = 0; ii < cntMesh; ii++)
		{
			cntLink = pMesh[ii]->links.size();
			//for (vector<KeyID>::iterator it = pMesh->links.begin(); it != pMesh->links.end(); it++) {
			for (unsigned int jj = 0; jj < cntLink; jj++)
			{
#if defined(USE_TREKKING_DATA)
				pLink = m_pDataMgr->GetWLinkDataById(pMesh[ii]->links[jj]);
				if (pLink == nullptr) {
					pLink = m_pDataMgr->GetLinkDataById(pMesh[ii]->links[jj]);
				}
#elif defined(USE_PEDESTRIAN_DATA)
				pLink = m_pDataMgr->GetWLinkDataById(pMesh[ii]->links[jj]);
#elif defined(USE_VEHICLE_DATA)
				pLink = m_pDataMgr->GetVLinkDataById(pMesh[ii]->links[jj]);
#else
				pLink = m_pDataMgr->GetLinkDataById(pMesh[ii]->links[jj]);
#endif

				if (pLink == nullptr)
				{
					LOG_TRACE(LOG_ERROR, "Failed, GetLink, mesh:%d, id:%d", pMesh[ii]->mesh_id.tile_id, pMesh[ii]->mesh_id.nid);
					continue;
				}
				cntVtx = pLink->getVertexCount();
				if (pLink->sub_info == NOT_USE || cntVtx <= 1)
				{
					continue;
				}

				for (unsigned int idxVT = 0; idxVT < cntVtx - 2; ++idxVT) {

					//PT sVertex(pLink->vtPts[idxVT].x, pLink->vtPts[idxVT].y);
					//PT eVertex(pLink->vtPts[idxVT + 1].x, pLink->vtPts[idxVT + 1].y);
					//getClosestPoint(sVertex.lon, sVertex.lat, eVertex.lon, eVertex.lat, lng, lat, &retLng, &retLat, &ir);
					getClosestPoint(pLink->getVertexX(idxVT), pLink->getVertexY(idxVT), pLink->getVertexX(idxVT + 1), pLink->getVertexY(idxVT + 1), lng, lat, &retLng, &retLat, &ir);

					PT Projection(retLng, retLat);

					double Distance = Projection.dist(reqCoord);

					if (minDistance == -1 || minDistance > Distance) {
						//idxLink = pLink->link_idx;
						idxLink = pLink->link_id;
						minDistance = Distance;
					}
				}
			}
		}
	}

	return idxLink;
}

//int CRouteManager::setNode(/*FLAG, Packet*/)
//{
//	int ret = -1;
//
//	ret = LinkProjection();
//
//	return ret;
//}
//KeyID CRouteManager::Projection(IN const double lng, IN const double lat)
//{
//	return KeyID();
//}
//int CRouteManager::SearchOneDirection()
//{
//
//	return 0;
//}



#if defined(USE_SHOW_ROUTE_SATATUS)
void CRouteManager::SetRouteStatusFunc(IN const void* pHost, IN void(*fpDrawRouting)(const void*, const unordered_map<uint64_t, CandidateLink*>*))
{
	m_pRoutePlan->SetRouteStatusFunc(pHost, fpDrawRouting);
}
#endif

//void CRouteManager::GetClosestPoint(SPoint spt, SPoint ept, SPoint upt, SPoint& result)
//{
//	double thIR_PRECISION = 100000.0;
//	double dx = ept.x - spt.x;
//	double dy = ept.y - spt.y;
//	double l2 = (dx * dx) + (dy * dy);
//	double ir = ((((upt.x - spt.x) * dx) + ((upt.y - spt.y) * dy)) / l2 * thIR_PRECISION);
//	if (ir < 0) {
//		result.x = spt.x;
//		result.y = spt.y;
//	}
//	else if (ir > thIR_PRECISION) {
//		result.x = ept.x;
//		result.y = ept.y;
//	}
//	else {
//		result.x = (spt.x + ((ir * dx) / thIR_PRECISION));
//		result.y = (spt.y + ((ir * dy) / thIR_PRECISION));
//	}
//}