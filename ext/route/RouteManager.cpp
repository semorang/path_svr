#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "RouteManager.h"
#include "MMPoint.hpp"
#include "../tms/GnKmeansClassfy.h"
#include "../utils/UserLog.h"
#include "../utils/GeoTools.h"
#include "../utils/CatmullRom.h"
#include "../utils/convexhull.h"

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
					LOG_TRACE(LOG_DEBUG, "departure projection, link_id:%d, range_lv:%d, dist:%d", keyDeparture.nid, ii, retDist);
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
						LOG_TRACE(LOG_DEBUG, "waypoint projection, link_id:%d, idx:%d, range_lv:%d, dist:%.1f", keyWaypoint.nid, keyWaypoints.size(), ii, retDist);
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
					LOG_TRACE(LOG_DEBUG, "destination projection, link_id:%d, range_lv:%d, dist:%d", keyDestination.nid, ii, retDist);
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


void CRouteManager::SetRouteCost(IN const uint32_t type, IN const RpCost* pCost)
{
	m_pRoutePlan->SetRouteCost(type, pCost);
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


#if defined(USE_TSP_MODULE)
bool CRouteManager::GetBestWaypointResult(IN TspOptions* pOpt, IN const RouteTable** ppResultTables, OUT vector<uint32_t>& vtBestWaypoints)
{
#if !defined(USE_REAL_ROUTE_TSP)
	vector<stCity> vtRawData;
#else
	struct stWaypoints {
		int nId;
		double x;
		double y;
	};
	vector<stWaypoints> vtRawData;
#endif

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
		for (const auto & coord : vtRawData) {
			vtViaResult.emplace_back(coord.nId);
		}
	}
	else if (pOpt->algorityhmType == 2) {
		//auto result = mst_manhattan_branch_and_bound2(ppResultTables, vtRawData.size(), 0);
		auto result = mst_manhattan_branch_and_bound(ppResultTables, vtRawData.size(), 0);
		vtViaResult.assign(result.path.begin(), result.path.end());
	}
#if !defined(USE_REAL_ROUTE_TSP)
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
#endif // #if !defined(USE_REAL_ROUTE_TSP)
	else {
		Environment newWorld;

		// 최대 세대 
		const int32_t maxGeneration = pOpt->loopCount;
		// 세대당 최대 개체수
		const int32_t maxPopulation = pOpt->individualSize;
		// 염색체당 최대 유전자수
		const int32_t maxGene = pOpt->geneSize;

		newWorld.SetOption(pOpt);

#if 0//defined(USE_REAL_ROUTE_TSP)
		// 실거리 테이블의 경우, 정방향 or 역방향 결과로 통일하자 --> 2023-05-11 [ii][jj] != [jj][ii]일 경우, 순위 경쟁에서 이상한 결과를 내보낼 경우 있음, 아직 원인 못찾음
		for (int ii = 0; ii < maxGene; ii++) {
			for (int jj = 0; jj < maxGene; jj++) {
				if ((ii == jj) || (ii < jj)) break;

				const_cast<RouteTable**>(ppResultTables)[jj][ii].nTotalDist = ppResultTables[ii][jj].nTotalDist;
				/*const_cast<RouteTable**>(ppResultTables)[jj][ii].nTotalTime = ppResultTables[ii][jj].nTotalTime;*/
			} // for jj
		} // for ii
#endif
		newWorld.SetCostTable(ppResultTables, maxGene);
		
		// 신세계
		newWorld.Genesis(maxGene, maxPopulation);

		const uint32_t MAX_SAME_RESULT = 500;// maxGeneration; // 같은 값으로 100회 이상 진행되면 최적값에 수렴하는것으로 판단하고 종료
		uint32_t repeatGeneration = 0;
		double topCost = DBL_MAX;
		double bestCost = DBL_MAX;

		// 반복횟수
		for (int ii = 0; ii < maxGeneration; ii++) {
			// 평가
			topCost = newWorld.Evaluation();
			//newWorld.Print();

			if (topCost < bestCost) {
				bestCost = topCost;
				repeatGeneration = 0;
			}
			else if (topCost == bestCost) {
				repeatGeneration++;
				if (MAX_SAME_RESULT < repeatGeneration) {
					LOG_TRACE(LOG_DEBUG, "best result fitness value repeating %d count, finish generation.", repeatGeneration);
					break;
				}
			}

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
	
	if (vtRawData.size() != vtViaResult.size()) {
		LOG_TRACE(LOG_DEBUG, "result count not match with orignal count, ori:%d != ret:%d", vtRawData.size(), vtViaResult.size());
		return false;
	}

	LOG_TRACE(LOG_DEBUG_LINE, "index  :", vtViaResult.size());
	for (idx = 0; idx < vtViaResult.size(); idx++)
	{
		LOG_TRACE(LOG_DEBUG_CONTINUE, "%3d|", idx);
	}
	LOG_TRACE(LOG_DEBUG_CONTINUE, " => \n", vtViaResult.size());

	uint32_t totalDist = 0;
	uint32_t idxFirst = 0;
	uint32_t idxPrev = 0;
	idx = 0;

	LOG_TRACE(LOG_DEBUG_LINE, "result :");
	for (const auto& item : vtViaResult)
	{
		LOG_TRACE(LOG_DEBUG_CONTINUE, "%3d>", item); //→
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
	int wayType = 1;// 원점 회귀:1, 아닌지:0 에 따라 갯수 변경됨

	vtBestWaypoints.clear();
	
	vtBestWaypoints.reserve(vtRawData.size());
	

	// 첫번째값을 출발지로 변경
	int newIdx = vtViaResult[curIdx];

	LOG_TRACE(LOG_DEBUG, "departure will change, idx(%d) %.5f, %.5f -> idx(%d) %.5f, %.5f",
		curIdx, vtRawData[curIdx].x, vtRawData[curIdx].y,
		newIdx, vtRawData[newIdx].x, vtRawData[newIdx].y);

	SetDeparture(vtRawData[newIdx].x, vtRawData[newIdx].y);

	vtBestWaypoints.emplace_back(newIdx);
	curIdx++;


	// 경유지 변경
	for (int ii = 1; ii <= vtViaResult.size() - 2; ii++) {
		newIdx = vtViaResult[ii];

		LOG_TRACE(LOG_DEBUG, "waypoint will change, idx(%d) %.5f, %.5f -> idx(%d) %.5f, %.5f",
			ii, vtRawData[ii].x, vtRawData[ii].y,
			newIdx, vtRawData[newIdx].x, vtRawData[newIdx].y);

		SetWaypoint(vtRawData[newIdx].x, vtRawData[newIdx].y);

		vtBestWaypoints.emplace_back(newIdx);
		curIdx++;
	}

	// 마지막값을 목적지로 변경
	newIdx = vtViaResult[curIdx];

	LOG_TRACE(LOG_DEBUG, "destination will change, idx(%d) %.5f, %.5f -> idx(%d) %.5f, %.5f",
		curIdx, vtRawData[curIdx].x, vtRawData[curIdx].y,
		newIdx, vtRawData[newIdx].x, vtRawData[newIdx].y);

	SetDestination(vtRawData[newIdx].x, vtRawData[newIdx].y);

	vtBestWaypoints.emplace_back(newIdx);
	curIdx++;
	
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

int CRouteManager::Table(TspOptions* pOpt, IN RouteTable** ppResultTables, OUT vector<uint32_t>& vtBestWaypoints)
{
	int ret = -1;

#if defined(USE_TSP_MODULE)
	ret = DoTabulate(pOpt, ppResultTables, vtBestWaypoints);
#endif 

	return ret;
}

int CRouteManager::DoRouting(/*Packet*/)
{
	int ret = -1;

	const uint32_t uid = 12345678;

	m_routeResult.Init();

	if (m_ptWaypoints.empty()) {
		if ((ret = m_pRoutePlan->DoRoute(uid, m_ptDeparture, m_ptDestination, keyDeparture, keyDestination, m_nRouteOpt, m_nAvoidOpt, &m_routeResult)) == ROUTE_RESULT_SUCCESS)
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

			RouteSummary summary;
			// for (int ii = m_vtRouteResult.size() - 1; ii >= 0; --ii) {
			for (int ii = 0; ii < m_vtRouteResult.size(); ii++) {
				// summarys
				summary.TotalDist = m_vtRouteResult[ii].TotalLinkDist;
				summary.TotalTime = m_vtRouteResult[ii].TotalLinkTime;
				m_routeResult.RouteSummarys.emplace_back(summary);

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


int CRouteManager::GetTable(OUT RouteTable** ppResultTables)
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
	
#if defined(USE_TSP_MODULE)	
	ret = m_pRoutePlan->DoTabulate(&reqInfo, ppResultTables);
#endif 

	return ret;
}


int CRouteManager::GetCluster_for_geoyoung(IN const int32_t cntCluster, OUT vector<stDistrict>& vtCluster)
{
	int ret = -1;

	vector<SPoint> vtPois;
	// 1209	
	array<int, 19> arrClusterCnt = {
		91, 68, 30, 57, 60, 60, 57, 64, 56, 67, 60, 65, 68, 70, 71, 77, 61, 62, 65
	};


	// add original coordinates
	int cntVias = GetWayPointCount();
	LOG_TRACE(LOG_DEBUG, "GetCluster_for_geoyoung, vias : %d", cntVias);

	vector<SPoint> vtOrigins;
	vtOrigins.reserve(cntVias + 2);
	vtOrigins.push_back({GetDeparture()->x, GetDeparture()->y});
	for (int ii=0; ii<cntVias; ii++) {
		vtOrigins.push_back({GetWaypoint(ii)->x, GetWaypoint(ii)->y});
	}
	vtOrigins.push_back({GetDestination()->x, GetDestination()->y});


	int cntPois = vtOrigins.size();
	char szGroup[128] = { 0, };
	int cntAdded = 0;

	LOG_TRACE(LOG_DEBUG, "GetCluster_for_geoyoung, pois : %d", cntPois);

	vtCluster.clear();
	vtCluster.reserve(cntCluster);

	LOG_TRACE(LOG_DEBUG, "GetCluster_for_geoyoung, clusters : %d", cntCluster);


	if (cntCluster != 19 || cntPois != 1209) {
		LOG_TRACE(LOG_WARNING, "GetCluster_for_geoyoung, req arg are not for geoyoung setting");
		return ret;
	}

	// geoyoung cluster
	int idCluster = 0;
	for (auto const& items : arrClusterCnt) {
		stDistrict tmpDistrict;
		sprintf(szGroup, "%02d", idCluster++);
		tmpDistrict.name = szGroup;

		LOG_TRACE(LOG_DEBUG, "GetCluster_for_geoyoung, cluster id : %d", items);

		// cluster pois
		for (int jj=0; jj<items; jj++) {
			stPoi tmpPoi;
			tmpPoi.coord = vtOrigins[cntAdded];
			tmpDistrict.pois.emplace_back(tmpPoi);

			cntAdded++;
		} // for
		tmpDistrict.center.x = 0;
		tmpDistrict.center.y = 0;
		vtCluster.emplace_back(tmpDistrict);
	} // for

	LOG_TRACE(LOG_DEBUG, "GetCluster_for_geoyoung, added : %d", cntAdded);

	// 배송처 권역
	for (auto& cluster : vtCluster) {
		vector<SPoint> coords;
		vector<SPoint> border;
		vector<SPoint> expend;
		vector<SPoint> slice;
		for (const auto& poi : cluster.pois) {
			coords.emplace_back(poi.coord);
		}

		// make border
		GetBoundary(coords, cluster.border);
	}

	ret = ROUTE_RESULT_SUCCESS;

	return ret;
}


int CRouteManager::GetCluster(IN const int32_t cntCluster, IN RouteTable** ppTables, OUT vector<stDistrict>& vtCluster)
{
	int ret = -1;

	vector<SPoint> vtPois;
	vtPois.reserve(m_ptWaypoints.size() + 2);

	// start 
	vtPois.emplace_back(m_ptDeparture);

	// waypoint
	if (!m_ptWaypoints.empty()) {
		for (int ii = 0; ii < m_ptWaypoints.size(); ii++) {
			vtPois.emplace_back(m_ptWaypoints[ii]);
		}
	}

	// end
	vtPois.emplace_back(m_ptDestination);

	if (vtPois.empty() || cntCluster <= 1 || ppTables == nullptr) {
		LOG_TRACE(LOG_WARNING, "GetClustering, request failed, poi cnt:%d, cluster cnt:%d, table:%p", vtPois.size(), cntCluster, ppTables);
		return -1;
	} 

	uint32_t cntPois = vtPois.size();
	
	RouteTable** ppResultTables = ppTables;
	double** ppWeightMatrix = nullptr;
	ret = ROUTE_RESULT_SUCCESS;

	if (ret == ROUTE_RESULT_SUCCESS) {
		// wm 생성
		ppWeightMatrix = new double*[cntPois];
		for (int ii = 0; ii < cntPois; ii++) {
			ppWeightMatrix[ii] = new double[cntPois];

			// copy data
			for (int jj = 0; jj < cntPois; jj++) {
				ppWeightMatrix[ii][jj] = ppResultTables[ii][jj].nTotalDist;
			}
		} // for
	}


	// k-means
	CGnKmeansClassfy kmc;

	// set wm
	kmc.SetWeightMatrix(ppWeightMatrix);

	SPoint centerAll = getPolygonCenter(vtPois);

	// 최초 클러스터 데이터
	kmc.SetDataCount(cntPois);
	int ii = 0;
	for (const auto& poi : vtPois) {
		kmc.AddData(ii++, poi.x, poi.y);
	}

	int32_t remainedCluster = cntCluster;

#if 1 // k-means 기본 분배
	kmc.Run(remainedCluster);

	// 클러스터 그룹핑
	vtCluster.reserve(remainedCluster);

	char szGroup[128] = { 0, };
	for (int ii = 0; ii < remainedCluster; ii++) {
		stDistrict tmpDistrict;

		sprintf(szGroup, "%02d", kmc.m_vtResult[ii].group);

		tmpDistrict.name = szGroup;
		tmpDistrict.center.x = kmc.m_vtResult[ii].x;
		tmpDistrict.center.y = kmc.m_vtResult[ii].y;
		tmpDistrict.pois.reserve(kmc.m_vtResult[ii].idx);

		vtCluster.emplace_back(tmpDistrict);
	} // for

	stDistrict* pDistrict = nullptr;
	for (const auto& item : kmc.m_vtInput) {
		pDistrict = &vtCluster.at(item.group);

		if (pDistrict) {
			stPoi newPoi;
			newPoi.coord.x = item.x;
			newPoi.coord.y = item.y;

			pDistrict->pois.emplace_back(newPoi);
		}
		else {
			LOG_TRACE(LOG_WARNING, "new cluster index not exist, idx:%d", item.group);
		}
	} // for
#else // 밀도 균등을 위해 반복 처리
	// 밀도 균등을 위해 반복 처리
	// 클러스터 수만큼 반복하여, 평균치에 가까운 녀석을 순차적으로 빼면서 반복
	const int32_t cntItemAvg = cntPois / MAX_CLUSTER_COUNT;
	const int32_t cntItemBand = cntItemAvg * 5 / 100; // 위아래 5%까지 허용

	for (; 0 < remainedCluster - 2;)
	{
		kmc.Run(remainedCluster);

		// 클러스터 그룹핑
		vector<stDistrict> tmpDistricts;
		tmpDistricts.reserve(kmc.m_vtResult.size());

		for (int ii = 0; ii < remainedCluster; ii++) {
			stDistrict tmpDistrict;
			tmpDistricts.emplace_back(tmpDistrict);
		} // for

		for (const auto& item : kmc.m_vtInput) {
			stDistrict* pCluster = &tmpDistricts.at(item.group);

			if (pCluster) {
				stPoi newPoi;
				newPoi.coord.x = item.x;
				newPoi.coord.y = item.y;

				pCluster->pois.emplace_back(newPoi);
				pCluster->center.x = kmc.m_vtResult[item.group].x;
				pCluster->center.y = kmc.m_vtResult[item.group].y;
			}
			else {
				LOG_TRACE(LOG_WARNING, "new cluster index not exist, idx:%d", item.group);
			}
		} // for

		  // 밀도 균등 클러스터 선택
		int32_t diff = INT32_MAX;
		int32_t cntAddedCluster = 0; // 등록된 클러스터 갯수
		int32_t diffNearCluster = INT32_MAX; // 평균치와 가장 가까운 클러스터 아이템 갯수
		vector<stDistrict>::iterator candidateIt = tmpDistricts.end(); // 평균치와 가장 가까운 클러스터
		char szNum[10] = { 0, };
		for (auto it = tmpDistricts.begin(); it != tmpDistricts.end();) {
			// 밀도 균등 허용치에 포함되면 사용
			diff = it->pois.size() - cntItemAvg;
			if (abs(diff) <= cntItemBand) {
				sprintf(&it->name[0], "%02d", m_tmsInfoNew.size());
				vtCluster.emplace_back(*it);

				cntAddedCluster++;

				// 삭제
				it = tmpDistricts.erase(it);
				candidateIt = tmpDistricts.end();
			}
			else {
				if ((cntAddedCluster <= 0) && ((0 <= diff) && (diff < diffNearCluster))) {
					// 예비 클러스터
					candidateIt = it;

					diffNearCluster = diff;
				}

				// 삭제 안하고 증가
				it++;
			}
		} // for

		  // 밀도 균등 클러스터가 없으면 예비 클러스터에서 사용
		if ((cntAddedCluster <= 0) && (candidateIt != tmpDistricts.end())) {
			// 가까운 순으로 정렬
			multimap<double, int>mapDist;
			for (int ii = 0; ii < candidateIt->pois.size(); ii++) { // diffNearCluster
				mapDist.emplace(getRealWorldDistance(candidateIt->center.x, candidateIt->center.y, candidateIt->pois[ii].coord.x, candidateIt->pois[ii].coord.y), ii);
				//mapDist.emplace(10000000.f - getRealWorldDistance(centerAll.x, centerAll.y, candidateIt->pois[ii].coord.x, candidateIt->pois[ii].coord.y), ii);
			} // for

			  // 밀도 균등 평균치 까지만 저장
			stDistrict tmpAddDistrict; // 저장될 아이템들
			stDistrict tmpRemainDistrict; // 다시 재사용될 아이템들
			sprintf(&tmpAddDistrict.name[0], "%02d", m_tmsInfoNew.size());
			tmpAddDistrict.center = candidateIt->center;

			for (const auto & dist : mapDist) {
				// 밀도 균등 평균치 까지만 저장
				if (tmpAddDistrict.pois.size() < cntItemAvg) {
					tmpAddDistrict.pois.emplace_back(candidateIt->pois[dist.second]);
				}
				else {
					tmpRemainDistrict.pois.emplace_back(candidateIt->pois[dist.second]);
				}
			} // for

			  // 원본 삭제
			tmpDistricts.erase(candidateIt);

			// 클러스터 추가
			vtCluster.emplace_back(tmpAddDistrict);
			cntAddedCluster++;

			// 재사용 추가
			//tmpDistricts.emplace_back(tmpRemainDistrict);
		}

		// 남은 값을 다시 넣어 클러스터링 반복 진행
		kmc.ReSet();
		int ii = 0;
		for (auto& district : tmpDistricts) {
			for (auto& item : district.pois) {
				kmc.AddData(ii++, item.coord.x, item.coord.y);
			}
		} // for

		remainedCluster -= cntAddedCluster;
	} // for

	  // 마지막 남은 아이템들을 모아 클러스터링
	if (remainedCluster != 1) {
		LOG_TRACE(LOG_WARNING, "last cluster count should remained only one, now:%d", remainedCluster);
	}
	else //if (0) {
		kmc.Run(remainedCluster);

	// 클러스터 그룹핑
	stDistrict tmpDistrict;
	sprintf(&tmpDistrict.name[0], "%02d", m_tmsInfoNew.size());
	tmpDistrict.center.x = kmc.m_vtResult[0].x;
	tmpDistrict.center.y = kmc.m_vtResult[0].y;

	for (const auto& item : kmc.m_vtInput) {
		stPoi newPoi;
		newPoi.coord.x = item.x;
		newPoi.coord.y = item.y;
		tmpDistrict.pois.emplace_back(newPoi);
	} // for
	vtCluster.emplace_back(tmpDistrict);
#endif


	// 배송처 권역
	for (auto& cluster : vtCluster) {
		vector<SPoint> coords;
		vector<SPoint> border;
		vector<SPoint> expend;
		vector<SPoint> slice;
		for (const auto& poi : cluster.pois) {
			coords.emplace_back(poi.coord);
		}

		// make border
		GetBoundary(coords, cluster.border);
	}

	// release
	kmc.m_vtInput.clear();
	kmc.m_vtResult.clear();

	for (int ii = 0; ii < cntPois; ii++) {
		SAFE_DELETE_ARR(ppWeightMatrix[ii]);
	}
	SAFE_DELETE_ARR(ppWeightMatrix);

	return ret;
}


int CRouteManager::GetBoundary(IN const vector<SPoint>& vtPois, OUT vector<SPoint>& vtBoundary)
{
	if (vtPois.empty()) {
		LOG_TRACE(LOG_WARNING, "GetClusterBoundary, request failed, poi cnt:%d", vtPois.size());
		return -1;
	}

	vector<SPoint> coords;
	vector<SPoint> border;
	vector<SPoint> expend;
	vector<SPoint> slice;

	for (const auto& poi : vtPois) {
		LOG_TRACE(LOG_TEST, "poi, x:%.5f, y:%.5f", poi.x, poi.y);
		coords.emplace_back(poi);
	}

	// make border
	ConvexHull(coords, border);

	// make closed
	if ((border.front().x != border.back().x) || (border.front().y != border.back().y)) {
		border.emplace_back(border.front());
	}

	// 면적 조사
	double area = GetPolygonArea(border);
	if (area <= 1) {
		vtBoundary.assign(border.begin(), border.end());
	} else {
		// 2023.7.26
		// catmullline의 2DVector포함 빌드시 node 동작 안하는 이슈 발생

		// 임시로 직선 바운더리 사용
		// cluster.border = border;

		
		// expend border
		GetBorderOfPolygon(border, -0.002, expend);

		// 외곽선 일정거리로 나누기
		GetSlicedLine(expend, 1000, slice);

		// 외곽선 부드럽게
		GetCatmullLine(slice, 0, vtBoundary);
	}

	return 0;
}


int CRouteManager::DoTabulate(TspOptions* pOpt, IN RouteTable** ppResultTables, OUT vector<uint32_t>& vtBestWaypoints)
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
		GetBestWaypointResult(pOpt, const_cast<const RouteTable**>(resultTables), vtBestWaypoints);
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