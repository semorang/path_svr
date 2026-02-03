#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "RouteManager.h"
#include "MMPoint.hpp"
#include "../tms/GnKmeansClassfy.h"
#include "../utils/UserLog.h"
#include "../utils/GeoTools.h"
#include "../utils/CatmullRom.h"
//#include "../utils/convexhull.h"

#define VLINK 1

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

CRouteManager::CRouteManager()
{
	m_pDataMgr = nullptr;
	m_pRoutePlan = new CRoutePlan();
	m_pTmsMgr = new CTMSManager();

	Initialize();

	m_vtRouteOpt.clear();
	vector<uint32_t>().swap(m_vtRouteOpt);
	m_vtAvoidOpt.clear();
	vector<uint32_t>().swap(m_vtAvoidOpt);

	m_nTimestampOpt = 0;
	m_nTrafficOpt = 0;
	m_nMobilityOpt = 0;
	m_nFreeOpt = 0;
#if defined(USE_P2P_DATA)	
	m_nCandiateOpt = 0;
#endif
	m_routeSubOpt.option = 0;

	m_nDepartureDirIgnore = 0;
	m_nWaypointDirIgnore = 0;
	m_nDestinationDirIgnore = 0;
}


CRouteManager::~CRouteManager()
{
	Release();

	SAFE_DELETE(m_pRoutePlan);
	SAFE_DELETE(m_pTmsMgr);
}


bool CRouteManager::Initialize(void)
{
	if (m_pRoutePlan) {
		m_pRoutePlan->Initialize();
	}

	if (m_pTmsMgr) {
		m_pTmsMgr->Initialize();
	}

	InitPoints();

	m_vtRouteOpt.clear();
	vector<uint32_t>().swap(m_vtRouteOpt);
	m_vtAvoidOpt.clear();
	vector<uint32_t>().swap(m_vtAvoidOpt);

	m_nTimestampOpt = 0;
	m_nTrafficOpt = 0;
	m_nMobilityOpt = 0;
	m_nFreeOpt = 0;
#if defined(USE_P2P_DATA)	
	m_nCandiateOpt = 0;
#endif
	m_routeSubOpt.option = 0;

	m_nDepartureDirIgnore = 0;
	m_nWaypointDirIgnore = 0;
	m_nDestinationDirIgnore = 0;

	return true;
}

void CRouteManager::Release(void)
{
	if (m_pRoutePlan) {
		m_pRoutePlan->Release();
	}

	if (m_pTmsMgr) {
		m_pTmsMgr->Release();
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
		m_pTmsMgr->SetDataMgr(pDataMgr);
	}
}


void CRouteManager::InitPoints(void)
{
	m_linkDeparture.init();
	m_linkDestination.init();
	if (!m_linkWaypoints.empty()) {
		m_linkWaypoints.clear();
		vector<RouteLinkInfo>().swap(m_linkWaypoints);
	}
}


int getCourseLinkMatchType(IN const stRouteSubOption subOption)
{
	int trkMatchType = TYPE_LINK_MATCH_NONE;

	if (subOption.mnt.course_id == 0) {
		if (subOption.mnt.course_type == TYPE_TRE_HIKING) { // 등산로
			trkMatchType = TYPE_TRE_HIKING;
		} else if (subOption.mnt.course_type == TYPE_TRE_TRAIL) { // 걷기길
			trkMatchType = TYPE_TRE_TRAIL;
		} else if (subOption.mnt.course_type == TYPE_TRE_BIKE) { // 자전거길
			trkMatchType = TYPE_TRE_BIKE;
		} else if (subOption.mnt.course_type == TYPE_TRE_CROSS) { // 코스
			trkMatchType = TYPE_TRE_CROSS;
		} else {
			trkMatchType = TYPE_TRE_HIKING; // 등산로
		}

		//if (subOption.mnt.course_type == 1) { // 등산로
		//	trkMatchType = TYPE_TRE_HIKING;
		//}
		//else if (subOption.mnt.course_type == 2) { // 둘레길
		//	trkMatchType = TYPE_TRE_TRAIL;
		//}
		//else if (subOption.mnt.course_type == 3) { // 자전거길
		//	trkMatchType = TYPE_TRE_BIKE;
		//}
		//else if (subOption.mnt.course_type == 4) { // 종주길
		//	trkMatchType = TYPE_TRE_CROSS;
		//}
		//else if (subOption.mnt.course_type == 5) { // 추천길
		//	trkMatchType = TYPE_TRE_RECOMMENDED;
		//}
		//else if (subOption.mnt.course_type == 6) { // MTB
		//	trkMatchType = TYPE_TRE_MTB;
		//}
		//else if (subOption.mnt.course_type == 7) { // 인기길
		//	trkMatchType = TYPE_TRE_POPULAR;
		//}
		//else {
		//	trkMatchType = TYPE_TRE_HIKING; // 등산로
		//}
	}

	return trkMatchType;
}


KeyID CRouteManager::SetDeparture(IN const double lng, IN const double lat, IN const int matchType)
{
	return SetPosition(lng, lat, matchType, m_linkDeparture);
}


KeyID CRouteManager::SetWaypoint(IN const double lng, IN const double lat, IN const int matchType)
{
	KeyID keyWaypoint = { NULL_VALUE };

	if (m_linkWaypoints.size() < MAX_WAYPOINT) {
		RouteLinkInfo linkWaypoint;
		keyWaypoint = SetPosition(lng, lat, matchType, linkWaypoint);
		m_linkWaypoints.emplace_back(linkWaypoint);
	}

	return keyWaypoint;
}


KeyID CRouteManager::SetDestination(IN const double lng, IN const double lat, IN const int matchType)
{
	return SetPosition(lng, lat, matchType, m_linkDestination);
}


void CRouteManager::SetRouteOption(IN const vector<uint32_t>& options, IN const vector<uint32_t>& avoids, IN const uint32_t timestamp, IN const uint32_t traffic, IN const uint32_t mobility)
{
	m_vtRouteOpt.clear();
	m_vtRouteOpt.assign(options.begin(), options.end());
	m_vtAvoidOpt.clear();
	m_vtAvoidOpt.assign(avoids.begin(), avoids.end());
	m_nTimestampOpt = timestamp;
	m_nTrafficOpt = traffic;
	m_nMobilityOpt = mobility;
}


void CRouteManager::AddRouteOption(IN const uint32_t option, IN const uint32_t avoid, IN const uint32_t mobility)
{
	m_vtRouteOpt.push_back(option);
	m_vtAvoidOpt.push_back(avoid);
	m_nMobilityOpt = mobility;
}


void CRouteManager::SetRouteSubOption(IN const uint64_t sub)
{
	m_routeSubOpt.option = sub;

#if defined(USE_FOREST_DATA)
	if (m_routeSubOpt.mnt.course_type != 0 && m_vtAvoidOpt[0] == 0) {
		if (m_routeSubOpt.mnt.course_type == TYPE_TRE_TRAIL) { // 걷기 - 보행자길 사용
			m_nMobilityOpt = TYPE_MOBILITY_PEDESTRIAN;
		}
		else if (m_routeSubOpt.mnt.course_type == TYPE_TRE_BIKE) { // 3:자전거길 - 보행자길 사용
			m_nMobilityOpt = TYPE_MOBILITY_BICYCLE;
		}
	}
	//if (m_routeSubOpt.mnt.course_type != 0 && m_vtAvoidOpt[0] == 0) {
	//	// 코스 타입 //0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
	//	m_vtAvoidOpt[0] = ROUTE_AVOID_HIKING | ROUTE_AVOID_TRAIL | ROUTE_AVOID_BIKE | ROUTE_AVOID_CROSS | ROUTE_AVOID_RECOMMEND | ROUTE_AVOID_MTB | ROUTE_AVOID_POPULAR;
	//		if () {
	//			m_nMobilityOpt = TYPE_MOBILITY_BICYCLE;
	//			m_vtAvoidOpt[0] = ROUTE_AVOID_NONE;
	//		}
	//		else { // 코스 탐색은 숲길 자전거 네트워크 사용
	//			m_vtAvoidOpt[0] -= ROUTE_AVOID_BIKE;
	//		}

	//	if (m_routeSubOpt.mnt.course_type == 1) { // 1:등산로
	//		m_vtAvoidOpt[0] -= ROUTE_AVOID_HIKING;
	//	} else if (m_routeSubOpt.mnt.course_type == 2) { // 2:둘레길
	//		m_vtAvoidOpt[0] -= ROUTE_AVOID_TRAIL;
	//	}
	//	else if (m_routeSubOpt.mnt.course_type == 3) { // 3:자전거길, 보행자길 사용
	//		if (m_routeSubOpt.mnt.course_id == 0) {
	//			m_nMobilityOpt = TYPE_MOBILITY_BICYCLE;
	//			m_vtAvoidOpt[0] = ROUTE_AVOID_NONE;
	//		}
	//		else { // 코스 탐색은 숲길 자전거 네트워크 사용
	//			m_vtAvoidOpt[0] -= ROUTE_AVOID_BIKE;
	//		}
	//	}
	//	else if (m_routeSubOpt.mnt.course_type == 4) { // 4:종주코스
	//		m_vtAvoidOpt[0] -= ROUTE_AVOID_CROSS;
	//	}
	//	else if (m_routeSubOpt.mnt.course_type == 5) { // 5:추천코스
	//		m_vtAvoidOpt[0] -= ROUTE_AVOID_RECOMMEND;
	//	}
	//	else if (m_routeSubOpt.mnt.course_type == 6) { // 6:MTB코스, 보행자길 사용
	//		if (m_routeSubOpt.mnt.course_id == 0) {
	//			m_nMobilityOpt = TYPE_MOBILITY_BICYCLE;
	//			m_vtAvoidOpt[0] = ROUTE_AVOID_NONE;
	//		}
	//		else { // 코스 탐색은 숲길 자전거 네트워크 사용
	//			m_vtAvoidOpt[0] -= ROUTE_AVOID_MTB;
	//		}
	//	}
	//	else if (m_routeSubOpt.mnt.course_type == 7) { // 7:인기코스
	//		m_vtAvoidOpt[0] -= ROUTE_AVOID_POPULAR;
	//	}
	//}
#endif
}


void CRouteManager::SetRouteFreeOption(IN const uint32_t free)
{
	m_nFreeOpt = free;
}


#if defined(USE_P2P_DATA)	
void CRouteManager::SetCandidateOption(IN const uint32_t candidate)
{
	m_nCandiateOpt = candidate;
}
#endif


void CRouteManager::SetRouteTruckOption(IN const TruckOption* pOption)
{
	if (pOption != nullptr) {
		m_routeTruckOpt.setTruckOption(pOption);
	}
}


void CRouteManager::SetRouteDirOption(IN const uint32_t departuretDir, IN const uint32_t waypointDir, IN const uint32_t destinationDir)
{
	m_nDepartureDirIgnore = departuretDir;
	m_nWaypointDirIgnore = waypointDir;
	m_nDestinationDirIgnore = destinationDir;
}


void CRouteManager::SetRouteCost(IN const DataCost* pCost, IN const uint32_t cntCost)
{
	m_pRoutePlan->SetRouteCost(pCost, cntCost);
}


const int CRouteManager::GetWayPointCount(void) const
{
	return m_linkWaypoints.size();
}


KeyID CRouteManager::GetPositionLink(IN const double lng, IN const double lat, IN const int32_t matchType, IN const int32_t dataType)
{
	KeyID retKey = {NULL_VALUE};

	if (m_pDataMgr != nullptr) {
		double retDist = INT_MAX;
		SPoint newPos;

		for (int ii = 0; ii< MAX_SEARCH_RANGE; ii++) {
			stLinkInfo* pLink = m_pDataMgr->GetLinkDataByPointAround(lng, lat, searchRange[ii], newPos.x, newPos.y, retDist, matchType, dataType);
			if (pLink) {
				retKey = pLink->link_id;

				if (ii > 0) {
					LOG_TRACE(LOG_DEBUG, "location link projection, link id:%d, range level:%d, dist:%d", retKey.nid, ii, retDist);
				}
				break;
			}
		} // for
	}

	return retKey;
}


const SPoint* CRouteManager::GetDeparture(bool useMatch)
{
	if ((useMatch == true) && (m_linkDeparture.MatchCoord.has())){
		return &m_linkDeparture.MatchCoord;
	} else if (m_linkDeparture.Coord.has()) {
		return &m_linkDeparture.Coord;
	}
	return nullptr;
}

const SPoint* CRouteManager::GetWaypoint(IN const uint32_t idx, bool useMatch)
{
	if (MAX_WAYPOINT <= idx) {
		return nullptr;
	}

	if ((useMatch == true) && (m_linkWaypoints[idx].MatchCoord.has())) {
		return &m_linkWaypoints[idx].MatchCoord;
	} else if (m_linkWaypoints[idx].Coord.has()) {
		return &m_linkWaypoints[idx].Coord;
	}
	return nullptr;
}

const SPoint* CRouteManager::GetDestination(bool useMatch)
{
	if ((useMatch == true) && (m_linkDestination.MatchCoord.has())) {
		return &m_linkDestination.MatchCoord;
	} else if (m_linkDestination.Coord.has()) {
		return &m_linkDestination.Coord;
	}
	return nullptr;
}


const uint32_t CRouteManager::GetRouteResultsCount(void) const
{
	return m_vtRouteResult.size();
}


const RouteResultInfo* CRouteManager::GetRouteResult(void) const
{
	if (m_vtRouteResult.empty()) {
		return nullptr;
	} else {
		return &m_vtRouteResult[0];
	}
	
}


const RouteResultInfo* CRouteManager::GetMultiRouteResult(IN const uint32_t idx) const
{
	if (idx < m_vtRouteResult.size()) {
		return &m_vtRouteResult[idx];
	}

	return nullptr;
}


const vector<RouteResultInfo>* CRouteManager::GetMultiRouteResults(void) const
{
	if (!m_vtRouteResult.empty()) {
		return &m_vtRouteResult;
	}

	return nullptr;
}


#if defined(USE_TSP_MODULE)
int CRouteManager::GetBestWaypointResult(IN const RouteDistMatrix& RDM, IN OUT BestWaypoints& TSP)
{
	int ret = -1;

	vector<stWaypoint> vtRawData;
	stWaypoint newWay;
	int32_t idx = 0;

	if (m_linkDeparture.Coord.has()) {
		newWay.id = idx++;
		newWay.position = m_linkDeparture.Coord;
		vtRawData.push_back(newWay);
	}

	for (const auto& way : m_linkWaypoints) {
		newWay.id = idx++;
		newWay.position = way.Coord;
		vtRawData.push_back(newWay);
	}

	if (m_linkDestination.Coord.has()) {
		newWay.id = idx++;
		newWay.position = m_linkDestination.Coord;
		vtRawData.push_back(newWay);
	}

#if 0
	LOG_TRACE(LOG_DEBUG, "start, best waypoint result, cnt:%d", vtRawData.size());

	if (pOpt->algorityhmType == 1) {
		for (const auto& coord : vtRawData) {
			vtBestWays.emplace_back(coord.nId);
		}
	}
	else if (pOpt->algorityhmType == 2) {
		//auto result = mst_manhattan_branch_and_bound2(ppResultTables, vtRawData.size(), 0);
		auto result = mst_manhattan_branch_and_bound(ppResultTables, vtRawData.size(), 0);
		vtBestWays.assign(result.path.begin(), result.path.end());
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

		env.getBest(vtBestWays);
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

		int cntUsableGene = maxGene;
		newWorld.SetCostTable(ppResultTables, cntUsableGene);

		// 신세계
		newWorld.Genesis(maxGene, maxPopulation);

		const uint32_t MAX_SAME_RESULT = maxGene * 10; // 500;// maxGeneration; // 같은 값으로 100회 이상 진행되면 최적값에 수렴하는것으로 판단하고 종료
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
					LOG_TRACE(LOG_DEBUG, "best result fitness value:%.3f, repeating count:%d", newWorld.GetBestDist(), repeatGeneration);
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
		} // for

		newWorld.GetBest(vtBestWays);
	}
	
	if (vtRawData.size() != vtBestWays.size()) {
		LOG_TRACE(LOG_DEBUG, "result count not match with orignal count, ori:%d != ret:%d", vtRawData.size(), vtBestWays.size());
		return ret;
	}

	LOG_TRACE(LOG_DEBUG_LINE, "index  :", vtBestWays.size());
	for (idx = 0; idx < vtBestWays.size(); idx++)
	{
		LOG_TRACE(LOG_DEBUG_CONTINUE, "%3d| ", idx);
	}
	LOG_TRACE(LOG_DEBUG_CONTINUE, " => \n", vtBestWays.size());

	uint32_t totalDist = 0;
	uint32_t idxFirst = 0;
	uint32_t idxPrev = 0;
	idx = 0;

	LOG_TRACE(LOG_DEBUG_LINE, "result :");
	for (const auto& item : vtBestWays)
	{
		LOG_TRACE(LOG_DEBUG_CONTINUE, "%3d\u2192", item); // →
		
		if (idx >= 1) {
			totalDist += ppResultTables[idxPrev][item].nTotalDist;
		}
		else {
			idxFirst = idx;
		}

		idxPrev = item;
		idx++;
	}

	// 원점 회귀 확인
	if ((pOpt->firstFix >= 0) && (pOpt->recursive)) {
		totalDist += ppResultTables[idxPrev][idxFirst].nTotalDist;
		LOG_TRACE(LOG_DEBUG_CONTINUE, " %d => %d\n", idxFirst, totalDist);
	} else {
		LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");
	}
#else
	m_pTmsMgr->GetBestway(&TSP.option, RDM.vtDistMatrix, TSP.vtWaypoints, TSP.vtBestWays, TSP.totalDist, TSP.totalTime);	
#endif
	// 기존 포인터 삭제
	InitPoints();

	int oldIdx = 0;
	int newIdx = 0;

	for (oldIdx = 0; oldIdx < vtRawData.size(); oldIdx++) {
		newIdx = TSP.vtBestWays[oldIdx];

		if (oldIdx == 0) {
			// 첫번째값을 출발지로 변경
			LOG_TRACE(LOG_DEBUG, "departure will change, idx(%d) %.5f, %.5f -> idx(%d) %.5f, %.5f",
				oldIdx, vtRawData[oldIdx].position.x, vtRawData[oldIdx].position.y,
				newIdx, vtRawData[newIdx].position.x, vtRawData[newIdx].position.y);

			SetDeparture(vtRawData[newIdx].position.x, vtRawData[newIdx].position.y);
		} else if (oldIdx == vtRawData.size() - 1) {
			// 마지막값을 목적지로 변경
			LOG_TRACE(LOG_DEBUG, "destination will change, idx(%d) %.5f, %.5f -> idx(%d) %.5f, %.5f",
				oldIdx, vtRawData[oldIdx].position.x, vtRawData[oldIdx].position.y,
				newIdx, vtRawData[newIdx].position.x, vtRawData[newIdx].position.y);

			SetDestination(vtRawData[newIdx].position.x, vtRawData[newIdx].position.y);
		} else {
			// 경유지 변경
			LOG_TRACE(LOG_DEBUG, "waypoint will change, idx(%d) %.5f, %.5f -> idx(%d) %.5f, %.5f",
				oldIdx, vtRawData[oldIdx].position.x, vtRawData[oldIdx].position.y,
				newIdx, vtRawData[newIdx].position.x, vtRawData[newIdx].position.y);

			SetWaypoint(vtRawData[newIdx].position.x, vtRawData[newIdx].position.y);
		}
	}

	ret = ROUTE_RESULT_SUCCESS;

	return ret;
}
#endif // #if defined(USE_TSP_MODULE)


int CRouteManager::Route(const int opt/*packet*/)
{
	int ret = -1;

	if (opt != 0) {
		ret = MultiRoute();
	}
	else {
		ret = SingleRoute();
	}

	return ret;
}


int CRouteManager::SingleRoute()
{
	int ret = -1;

#if defined(USE_MOUNTAIN_DATA)
	if (m_routeSubOpt.mnt.course_id != 0) {
		ret = DoCourse();
	} else {
		ret = DoComplexRouting();
	}
#else
	ret = DoRouting();
#endif

#if defined(USE_P2P_DATA)
	if ((ret == ROUTE_RESULT_SUCCESS) && (m_nCandiateOpt)) {
		GetCandidateRoute();
	}
#endif

	return ret;
}


const int CRouteManager::GetCandidateRoute(void)
{
	// 다중 경로가 있어도 1번째 경로의 대안 경로만 생성하자

	vector<RouteLinkInfo> vtMPP;
	m_pRoutePlan->GetMostProbableLink(&m_vtRouteResult[0], vtMPP);

	for (const auto& candidateLink : vtMPP) {

	}

	int ret = -1;
	const uint32_t uid = 12345678;
	const int32_t maxRouteCount = 3; // 3;
	const int32_t newRouteCount = min(maxRouteCount, static_cast<int>(vtMPP.size()));

	RequestRouteInfo reqInfos[maxRouteCount];
	vector<RouteInfo> vtRouteInfos[maxRouteCount];
	vector<RouteResultInfo> vtRouteResults[maxRouteCount];
	CRoutePlan routePlans[maxRouteCount];

	for (int ii = 0; ii < newRouteCount; ii++) {
		routePlans[ii].Initialize();
		routePlans[ii].SetDataMgr(m_pDataMgr);
		routePlans[ii].SetRouteCost(m_pRoutePlan->GetRouteCost());

		reqInfos[ii].RequestId = uid;
#if defined(USE_P2P_DATA)
		reqInfos[ii].RequestMode = 100; // 대안경로
#endif
		reqInfos[ii].RequestTime = m_nTimestampOpt;
		reqInfos[ii].RequestTraffic = m_nTrafficOpt;
		reqInfos[ii].MobilityOption = m_nMobilityOpt;
		reqInfos[ii].FreeOption = m_nFreeOpt;
		reqInfos[ii].RouteOption = m_vtRouteOpt[0];
		reqInfos[ii].AvoidOption = m_vtAvoidOpt[0];
		reqInfos[ii].RouteSubOption = m_routeSubOpt;
		reqInfos[ii].RouteTruckOption = m_routeTruckOpt;
		reqInfos[ii].StartDirIgnore = m_nDepartureDirIgnore;
		reqInfos[ii].WayDirIgnore = m_nWaypointDirIgnore;
		reqInfos[ii].EndDirIgnore = m_nDestinationDirIgnore;

		RouteLinkInfo linkInfo;
		// 시작 지점 정보 변경
		// start 

		linkInfo.init();
		linkInfo.KeyType = TYPE_KEY_LINK;
		linkInfo.LinkDataType = m_linkDeparture.LinkDataType;
		linkInfo.LinkId = vtMPP[ii].LinkId;
		linkInfo.Coord = vtMPP[ii].Coord;
		linkInfo.MatchCoord = vtMPP[ii].Coord;
		//linkInfo.Payed; // 유료 링크
		linkInfo.LinkSplitIdx = 0; // 링크와 직교 접점 좌표의 링크 버텍스 idx
		linkInfo.LinkDistToS = 0; // s로의 거리
		linkInfo.LinkDistToE = 0; // e로의 거리
		linkInfo.LinkDir = vtMPP[ii].LinkDir;
		linkInfo.LinkGuideType = LINK_GUIDE_TYPE_DEPARTURE;
		vector<SPoint> LinkVtxToS; // 좌표점에서 s버텍스, 종료링크의 경우는 FromS로 이해할것
		vector<SPoint> LinkVtxToE; // 좌표점에서 e버텍스, 종료링크의 경우는 FromS로 이해할것

		reqInfos[ii].vtPoints.emplace_back(vtMPP[ii].Coord);
		reqInfos[ii].vtKeyId.emplace_back(vtMPP[ii].LinkId);
		reqInfos[ii].vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
#if defined(USE_PEDESTRIAN_DATA)
#if defined(USE_MOUNTAIN_DATA)
		if (m_linkDeparture.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
		} else
#endif
		{
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
		}
#else
		reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_VEHICLE);
#endif

		// 위 벡터들 통합
		reqInfos[ii].vtPointsInfo.emplace_back(linkInfo);


		// waypoint
		for (const auto& waypoint : m_linkWaypoints) {
			reqInfos[ii].vtPoints.emplace_back(waypoint.Coord);
			reqInfos[ii].vtKeyId.emplace_back(waypoint.LinkId);
			reqInfos[ii].vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
#if defined(USE_PEDESTRIAN_DATA)
#if defined(USE_MOUNTAIN_DATA)
			if (waypoint.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
				reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
			} else
#endif
			{
				reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
			}
#else
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_VEHICLE);
#endif

			// 위 벡터들 통합
			reqInfos[ii].vtPointsInfo.emplace_back(waypoint);
		} // for

		// end
		reqInfos[ii].vtPoints.emplace_back(m_linkDestination.Coord);
		reqInfos[ii].vtKeyId.emplace_back(m_linkDestination.LinkId);
		reqInfos[ii].vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
#if defined(USE_PEDESTRIAN_DATA)
#if defined(USE_MOUNTAIN_DATA)
		if (m_linkDestination.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
		} else
#endif
		{
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
		}
#else
		reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_VEHICLE);
#endif

		// 위 벡터들 통합
		reqInfos[ii].vtPointsInfo.emplace_back(m_linkDestination);
	}

//#if defined(USE_MULTIPROCESS)
//#pragma omp parallel for
//#endif
	for (int ii = 0; ii < newRouteCount; ii++) {
		KeyID keyId = { 0, };

		vtRouteInfos[ii].resize(reqInfos[ii].vtPoints.size());
		vtRouteResults[ii].resize(reqInfos[ii].vtPoints.size() - 1);

		if ((ret = m_pRoutePlan->DoRoutes(&reqInfos[ii], &vtRouteInfos[ii], &vtRouteResults[ii])) == ROUTE_RESULT_SUCCESS) {
			RouteResultInfo result;
			result.Init();

			result.ResultCode = vtRouteResults[ii][0].ResultCode; // 경로 결과 코드, 0:성공, 1~:실패

			result.reqInfo.SetOption(&vtRouteResults[ii][0].reqInfo);
			//result.RequestMode = vtRouteResults[ii][0].RequestMode; // 요청 모드
			//result.RequestId = vtRouteResults[ii][0].RequestId; // 요청 ID
			//result.RouteOption = vtRouteResults[ii][0].RouteOption; // 경로 옵션
			//result.RouteAvoid = vtRouteResults[ii][0].RouteAvoid; // 경로 회피

			result.StartResultLink = vtRouteResults[ii][vtRouteResults[ii].size() - 1].StartResultLink;
			result.EndResultLink = vtRouteResults[ii][0].EndResultLink;

			RouteSummary summary;
			for (const auto& route : vtRouteResults[ii]) {
				// summarys
				if (route.LinkInfo.empty()) {
					continue; // 이종간 경로 탐색 무시된 케이스
				}

				summary.TotalDist = static_cast<uint32_t>(route.TotalLinkDist);
				summary.TotalTime = route.TotalLinkTime;
				result.RouteSummarys.emplace_back(summary);

				// 경로선
				boxMerge(result.RouteBox, route.RouteBox);
				linkMerge(result.LinkVertex, route.LinkVertex);

				result.LinkInfo.reserve(result.LinkInfo.size() + route.LinkInfo.size());
				for (const auto& link : route.LinkInfo) {
					// 경유지가 있을 경우, 전체 경로를 합치면, 개별 offset 및 거리, 시간이 누적되어 변경되어야 함.
					// 작업 추가 필요
					result.LinkInfo.emplace_back(link);
				} // for

				result.TotalLinkDist += route.TotalLinkDist; // 경로 전체 거리
				result.TotalLinkCount += route.TotalLinkCount; // 경로 전체 링크 수
				result.TotalLinkTime += route.TotalLinkTime;; // 경로 전체 소요 시간 (초)
			} // for

			m_vtRouteResult.emplace_back(result);
		}
	} // for

	return 0;
}


int CRouteManager::MultiRoute(const int opt/*packet*/)
{
	int ret = -1;

	const int reqCount = m_vtRouteOpt.size();

#if defined(USE_MOUNTAIN_DATA)
	if (m_routeSubOpt.mnt.course_id != 0) {
		ret = DoCourse();
	}
	else {
		//int options[reqCount] = { ROUTE_OPT_RECOMMENDED, ROUTE_OPT_SHORTEST, ROUTE_OPT_COMFORTABLE };
		//int avoids[reqCount] = { 126, 126, 126 };
		ret = DoMultiComplexRouting(reqCount/*, options, avoids*/);
	}
#else
	//const int reqCount = 3;
	//int options[reqCount] = { ROUTE_OPT_RECOMMENDED,ROUTE_OPT_SHORTEST,ROUTE_OPT_COMFORTABLE };
	//int avoids[reqCount] = { 30, 30, 30 };
	//const int reqCount = ROUTE_OPT_COUNT;// 5;
	//int options[reqCount] = { ROUTE_OPT_SHORTEST, ROUTE_OPT_RECOMMENDED, ROUTE_OPT_COMFORTABLE, ROUTE_OPT_FASTEST, ROUTE_OPT_MAINROAD };
	//int avoids[reqCount] = { 0, 0, 0, 0, 0 };
	ret = DoMultiRouting(reqCount/*, options, avoids*/);
#endif

	return ret;
}


int CRouteManager::DoRouting(/*Packet*/)
{
	int ret = -1;

	const uint32_t uid = 12345678;

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "DoRouting, request ways: %d", m_linkWaypoints.size() + 2);

#if 0
	if (m_linkWaypoints.empty()) {
		if ((ret = m_pRoutePlan->DoRoute(uid, m_linkDeparture.Coord, m_linkDestination.Coord, m_linkDeparture.LinkId, m_linkDestination.LinkId, m_nRouteOpt, m_nAvoidOpt, m_nDepartureDirIgnore, m_nDestinationDirIgnore, &m_routeResult)) == ROUTE_RESULT_SUCCESS)
		{
			;
		}
	}
	else
#endif
	{
		RequestRouteInfo reqInfo;
		reqInfo.RequestId = uid;
		reqInfo.RequestTime = m_nTimestampOpt;
		reqInfo.RequestTraffic = m_nTrafficOpt;
		reqInfo.MobilityOption = m_nMobilityOpt;
		reqInfo.FreeOption = m_nFreeOpt;
		reqInfo.RouteOption = m_vtRouteOpt[0];
		reqInfo.AvoidOption = m_vtAvoidOpt[0];
		reqInfo.RouteSubOption = m_routeSubOpt;
		reqInfo.RouteTruckOption = m_routeTruckOpt;
		reqInfo.StartDirIgnore = m_nDepartureDirIgnore;
		reqInfo.WayDirIgnore = m_nWaypointDirIgnore;
		reqInfo.EndDirIgnore = m_nDestinationDirIgnore;

		// start 
		reqInfo.vtPoints.emplace_back(m_linkDeparture.Coord);
		reqInfo.vtKeyId.emplace_back(m_linkDeparture.LinkId);
#if defined(USE_MOUNTAIN_DATA)
		if (m_linkDestination.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
			reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
		}
		else {
			reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
		}
#elif defined(USE_PEDESTRIAN_DATA)
		reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
#elif defined(USE_VEHICLE_DATA)
		reqInfo.vtLinkDataType.emplace_back(TYPE_DATA_VEHICLE);
#endif
		reqInfo.vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭

		// 위 벡터들 통합
		reqInfo.vtPointsInfo.emplace_back(m_linkDeparture);

		// waypoint
		if (!m_linkWaypoints.empty()) {
			for (const auto& via : m_linkWaypoints) { //(int ii = 0; ii < m_linkWaypoints.size(); ii++) {
				reqInfo.vtPoints.emplace_back(via.Coord);
				reqInfo.vtKeyId.emplace_back(via.LinkId);
#if defined(USE_MOUNTAIN_DATA)
				if (via.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
					reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
				}
				else {
					reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
				}
#elif defined(USE_PEDESTRIAN_DATA)
				reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
#elif defined(USE_VEHICLE_DATA)
				reqInfo.vtLinkDataType.emplace_back(TYPE_DATA_VEHICLE);
#endif
				reqInfo.vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭

				// 위 벡터들 통합
				reqInfo.vtPointsInfo.emplace_back(via);
			} // for
		}

		// end
		reqInfo.vtPoints.emplace_back(m_linkDestination.Coord);
		reqInfo.vtKeyId.emplace_back(m_linkDestination.LinkId);
#if defined(USE_MOUNTAIN_DATA)
		if (m_linkDestination.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
			reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
		}
		else {
			reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
		}
#elif defined(USE_PEDESTRIAN_DATA)
		reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
#elif defined(USE_VEHICLE_DATA)
		reqInfo.vtLinkDataType.emplace_back(TYPE_DATA_VEHICLE);
#endif
		reqInfo.vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
													   
		// 위 벡터들 통합
		reqInfo.vtPointsInfo.emplace_back(m_linkDestination);


		if (!m_vtRouteInfo.empty()) {
			m_vtRouteInfo.clear();
			vector<RouteInfo>().swap(m_vtRouteInfo);
		}

		if (!m_vtRouteResult.empty()) {
			m_vtRouteResult.clear();
			vector<RouteResultInfo>().swap(m_vtRouteResult);
		}

		vector<RouteInfo> vtRouteInfo;
		vector<RouteResultInfo> vtRouteResult;

		if ((ret = m_pRoutePlan->DoRoutes(&reqInfo, &vtRouteInfo, &vtRouteResult)) == ROUTE_RESULT_SUCCESS)
		{
			RouteResultInfo result;
			result.Init();

			result.ResultCode = vtRouteResult[0].ResultCode; // 경로 결과 코드, 0:성공, 1~:실패

			result.reqInfo.SetOption(&vtRouteResult[0].reqInfo);
			//result.RequestMode = vtRouteResult[0].RequestMode; // 요청 모드
			//result.RequestId = vtRouteResult[0].RequestId; // 요청 ID
			//result.RequestTime = vtRouteResult[0].RequestTime; // 요청 시각
			//result.RouteOption = vtRouteResult[0].RouteOption; // 경로 옵션
			//result.RouteAvoid = vtRouteResult[0].RouteAvoid; // 경로 회피

			result.StartResultLink = vtRouteResult[vtRouteResult.size() - 1].StartResultLink;
			result.EndResultLink = vtRouteResult[0].EndResultLink;

			RouteSummary summary;
			// for (int ii = vtRouteResult.size() - 1; ii >= 0; --ii) {
			for (auto& route : vtRouteResult) {
				// summarys
				summary.TotalDist = static_cast<uint32_t>(route.TotalLinkDist);
				summary.TotalTime = route.TotalLinkTime;
				result.RouteSummarys.emplace_back(summary);

				// 경로선
				boxMerge(result.RouteBox, route.RouteBox);
				linkMerge(result.LinkVertex, route.LinkVertex);

				int offVtx = result.LinkVertex.size() - route.LinkVertex.size();
				result.LinkInfo.reserve(result.LinkInfo.size() + route.LinkInfo.size());
				for (auto& link : route.LinkInfo) {
					// 경유지가 있을 경우, 전체 경로를 합치면, 개별 offset 및 거리, 시간이 누적되어 변경되어야 함.
					// 작업 추가 필요
					link.vtx_off = offVtx + link.vtx_off;
					result.LinkInfo.emplace_back(link);
				} // for

				result.TotalLinkDist += route.TotalLinkDist; // 경로 전체 거리
				result.TotalLinkCount += route.TotalLinkCount; // 경로 전체 링크 수
				result.TotalLinkTime += route.TotalLinkTime;; // 경로 전체 소요 시간 (초)
			} // for

			m_vtRouteResult.emplace_back(result);
		}
	}

	LOG_TRACE(LOG_DEBUG, timeStart, "DoRouting, result: %d", ret);

	return ret;
}


int CRouteManager::DoMultiRouting(IN const int32_t routeCount/*, IN const int32_t routeOptions[], IN const int32_t routeAvoids[]*/)
{
	int ret = -1;

	const uint32_t uid = 12345678;
	const int32_t route_count = 5; // 최대치

	RequestRouteInfo reqInfos[route_count];

	vector<RouteInfo> vtRouteInfos[route_count];
	vector<RouteResultInfo> vtRouteResults[route_count];

	//vector<int32_t> routeOptions;
	//vector<uint32_t> routeAvoids;

	CRoutePlan routePlans[route_count];

	m_vtRouteResult.resize(routeCount);

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "DoMultiRouting, request cnt: %d", routeCount);

	for (int ii = 0; ii < routeCount; ii++) {
		routePlans[ii].Initialize();
		routePlans[ii].SetDataMgr(m_pDataMgr);
		routePlans[ii].SetRouteCost(m_pRoutePlan->GetRouteCost());

		reqInfos[ii].RequestId = uid;
		reqInfos[ii].RequestTime = m_nTimestampOpt;
		reqInfos[ii].RequestTraffic = m_nTrafficOpt;
		reqInfos[ii].MobilityOption = m_nMobilityOpt;
		reqInfos[ii].FreeOption = m_nFreeOpt;
		reqInfos[ii].RouteOption = m_vtRouteOpt[ii];
		reqInfos[ii].AvoidOption = m_vtAvoidOpt[ii];
		reqInfos[ii].RouteSubOption = m_routeSubOpt;
		reqInfos[ii].RouteTruckOption = m_routeTruckOpt;
		reqInfos[ii].StartDirIgnore = m_nDepartureDirIgnore;
		reqInfos[ii].WayDirIgnore = m_nWaypointDirIgnore;
		reqInfos[ii].EndDirIgnore = m_nDestinationDirIgnore;

		// start 
		reqInfos[ii].vtPoints.emplace_back(m_linkDeparture.Coord);
		reqInfos[ii].vtKeyId.emplace_back(m_linkDeparture.LinkId);
		reqInfos[ii].vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
#if defined(USE_PEDESTRIAN_DATA)
#if defined(USE_MOUNTAIN_DATA)
		if (m_linkDeparture.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
		} else
#endif
		{
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
		}
#else
		reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_VEHICLE);
#endif

		// 위 벡터들 통합
		reqInfos[ii].vtPointsInfo.emplace_back(m_linkDeparture);


		// waypoint
		for (const auto& waypoint : m_linkWaypoints) {
			reqInfos[ii].vtPoints.emplace_back(waypoint.Coord);
			reqInfos[ii].vtKeyId.emplace_back(waypoint.LinkId);
			reqInfos[ii].vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
#if defined(USE_PEDESTRIAN_DATA)
#if defined(USE_MOUNTAIN_DATA)
			if (waypoint.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
				reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
			} else
#endif
			{
				reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
			}
#else
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_VEHICLE);
#endif

			// 위 벡터들 통합
			reqInfos[ii].vtPointsInfo.emplace_back(waypoint);
		} // for

		// end
		reqInfos[ii].vtPoints.emplace_back(m_linkDestination.Coord);
		reqInfos[ii].vtKeyId.emplace_back(m_linkDestination.LinkId);
		reqInfos[ii].vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
#if defined(USE_PEDESTRIAN_DATA)
#if defined(USE_MOUNTAIN_DATA)
		if (m_linkDestination.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
		} else
#endif
		{
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
		}
#else
		reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_VEHICLE);
#endif

		// 위 벡터들 통합
		reqInfos[ii].vtPointsInfo.emplace_back(m_linkDestination);
	}


#if defined(USE_MULTIPROCESS)
#pragma omp parallel for
#endif
	for (int ii = 0; ii < routeCount; ii++) {
		KeyID keyId = { 0, };

		vtRouteInfos[ii].resize(reqInfos[ii].vtPoints.size());
		vtRouteResults[ii].resize(reqInfos[ii].vtPoints.size() - 1);

		if ((ret = m_pRoutePlan->DoRoutes(&reqInfos[ii], &vtRouteInfos[ii], &vtRouteResults[ii])) == ROUTE_RESULT_SUCCESS) {
			m_vtRouteResult[ii].Init();

			m_vtRouteResult[ii].ResultCode = vtRouteResults[ii][0].ResultCode; // 경로 결과 코드, 0:성공, 1~:실패

			m_vtRouteResult[ii].reqInfo.SetOption(&vtRouteResults[ii][0].reqInfo);
			//m_vtRouteResult[ii].RequestMode = vtRouteResults[ii][0].RequestMode; // 요청 모드
			//m_vtRouteResult[ii].RequestId = vtRouteResults[ii][0].RequestId; // 요청 ID
			//m_vtRouteResult[ii].RequestTime = vtRouteResults[ii][0].RequestTime; // 요청 ID
			//m_vtRouteResult[ii].RequestTraffic = vtRouteResults[ii][0].RequestTraffic; 
			//m_vtRouteResult[ii].RouteOption = vtRouteResults[ii][0].RouteOption; // 경로 옵션
			//m_vtRouteResult[ii].RouteAvoid = vtRouteResults[ii][0].RouteAvoid; // 경로 회피

			m_vtRouteResult[ii].StartResultLink = vtRouteResults[ii][vtRouteResults[ii].size() - 1].StartResultLink;
			m_vtRouteResult[ii].EndResultLink = vtRouteResults[ii][0].EndResultLink;

			RouteSummary summary;
			// for (int ii = m_vtRouteResult.size() - 1; ii >= 0; --ii) {
			for (const auto& route : vtRouteResults[ii]) {
				// summarys
				if (route.LinkInfo.empty()) {
					continue; // 이종간 경로 탐색 무시된 케이스
				}

				summary.TotalDist = static_cast<uint32_t>(route.TotalLinkDist);
				summary.TotalTime = route.TotalLinkTime;
				m_vtRouteResult[ii].RouteSummarys.emplace_back(summary);

				// 경로선
				boxMerge(m_vtRouteResult[ii].RouteBox, route.RouteBox);
				linkMerge(m_vtRouteResult[ii].LinkVertex, route.LinkVertex);

				m_vtRouteResult[ii].LinkInfo.reserve(m_vtRouteResult[ii].LinkInfo.size() + route.LinkInfo.size());
				for (const auto& link : route.LinkInfo) {
					// 경유지가 있을 경우, 전체 경로를 합치면, 개별 offset 및 거리, 시간이 누적되어 변경되어야 함.
					// 작업 추가 필요
					m_vtRouteResult[ii].LinkInfo.emplace_back(link);
				} // for

				m_vtRouteResult[ii].TotalLinkDist += route.TotalLinkDist; // 경로 전체 거리
				m_vtRouteResult[ii].TotalLinkCount += route.TotalLinkCount; // 경로 전체 링크 수
				m_vtRouteResult[ii].TotalLinkTime += route.TotalLinkTime;; // 경로 전체 소요 시간 (초)
			} // for
		}
	} // for

	LOG_TRACE(LOG_DEBUG, timeStart, "DoMultiRouting, result: %d", ret);

	return ret;
}


int insertLinkCourseInfo(IN IN CDataManager* pDataMgr, IN const KeyID linkId, IN const int32_t linkType, OUT vector<unordered_set<uint32_t>>& vtCourse)
{
	int cntCourse = 0;

#if defined(USE_FOREST_DATA)
	// 코스 ID 매칭 확인
	stLinkInfo* pLinkCourse = nullptr;
	unordered_set<uint32_t> setCourse;

	if (pDataMgr == nullptr || linkId.llid == 0) {
		return cntCourse;
	}

	if ((linkType == TYPE_LINK_DATA_TREKKING) && (linkId.llid != 0)) {
		if ((pLinkCourse = pDataMgr->GetFLinkDataById(linkId)) != nullptr) {
			if (pLinkCourse->trk_ext.course_cnt > 0) {
				set<uint32_t>* pSet = pDataMgr->GetCourseByLink(pLinkCourse->link_id.llid);
				for (const auto& course : *pSet) {
					setCourse.emplace(course);
					cntCourse++;
				}
			}
		}
	}

	vtCourse.emplace_back(setCourse);
#endif

	return cntCourse;
}


int CRouteManager::DoComplexRouting(/*Packet*/)
{
	int ret = -1;

	const uint32_t uid = 12345678;

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "DoComplexRouting, request ways: %d", m_linkWaypoints.size() + 2);

	RequestRouteInfo reqInfo;
	reqInfo.RequestId = uid;
	reqInfo.RequestTime = m_nTimestampOpt;
	reqInfo.RequestTraffic = m_nTrafficOpt;
	reqInfo.MobilityOption = m_nMobilityOpt;
	reqInfo.FreeOption = m_nFreeOpt;
	reqInfo.RouteOption = m_vtRouteOpt[0];
	reqInfo.AvoidOption = m_vtAvoidOpt[0];
	reqInfo.RouteSubOption = m_routeSubOpt;
	reqInfo.RouteTruckOption = m_routeTruckOpt;
	reqInfo.StartDirIgnore = m_nDepartureDirIgnore;
	reqInfo.WayDirIgnore = m_nWaypointDirIgnore;
	reqInfo.EndDirIgnore = m_nDestinationDirIgnore;

	// start 
	reqInfo.vtPoints.emplace_back(m_linkDeparture.Coord);
	reqInfo.vtKeyId.emplace_back(m_linkDeparture.LinkId);
	reqInfo.vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
#if defined(USE_FOREST_DATA)
	if (m_linkDeparture.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
		reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
	} else {
		reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
	}

	// 코스 ID 매칭 확인
	insertLinkCourseInfo(m_pDataMgr, m_linkDeparture.LinkId, m_linkDeparture.LinkDataType, reqInfo.vtCourse);
#endif

	// 위 벡터들 통합
	reqInfo.vtPointsInfo.emplace_back(m_linkDeparture);


	// waypoint
	for (const auto& waypoint : m_linkWaypoints) {
		reqInfo.vtPoints.emplace_back(waypoint.Coord);
		reqInfo.vtKeyId.emplace_back(waypoint.LinkId);
		reqInfo.vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
#if defined(USE_FOREST_DATA)
		if (waypoint.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
			reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
		}
		else {
			reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
		}

		// 코스 ID 매칭 확인
		insertLinkCourseInfo(m_pDataMgr, waypoint.LinkId, waypoint.LinkDataType, reqInfo.vtCourse);
#endif

		// 위 벡터들 통합
		reqInfo.vtPointsInfo.emplace_back(waypoint);
	} // for


	// end
	reqInfo.vtPoints.emplace_back(m_linkDestination.Coord);
	reqInfo.vtKeyId.emplace_back(m_linkDestination.LinkId);
	reqInfo.vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
#if defined(USE_FOREST_DATA)
	if (m_linkDestination.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
		reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
	} else {
		reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
	}

	// 코스 ID 매칭 확인
	insertLinkCourseInfo(m_pDataMgr, m_linkDestination.LinkId, m_linkDestination.LinkDataType, reqInfo.vtCourse);
#endif

	// 위 벡터들 통합
	reqInfo.vtPointsInfo.emplace_back(m_linkDestination);

	if (!m_vtRouteInfo.empty()) {
		m_vtRouteInfo.clear();
		vector<RouteInfo>().swap(m_vtRouteInfo);
	}

	if (!m_vtRouteResult.empty()) {
		m_vtRouteResult.clear();
		vector<RouteResultInfo>().swap(m_vtRouteResult);
	}

	vector<RouteInfo> vtRouteInfo;
	vector<RouteResultInfo> vtRouteResult;

	vtRouteInfo.resize(reqInfo.vtPoints.size());
	vtRouteResult.resize(reqInfo.vtPoints.size() - 1);

	// 입구점 여부 확인 필요 
	//vector<ComplexPointInfo> vtComplexPointInfo(reqInfo.vtPoints.size());



	// 숲길인데 산바운더리 입구점이 없으면 링크 확장으로 입구점 검색
	m_pRoutePlan->DoEdgeRoute(&reqInfo, m_vtRouteInfo/*, vtComplexPointInfo*/);


	// 경탐 수행

#if 0 // 각 지점간 이종 링크의 가장 가까운 입구점을 찾아 경유지로 선정하고 경로 탐색 
	KeyID keyId = { 0, };
	RequestRouteInfo reqInfoNew;

	// 각 지점간 이종 링크 경유 확인 (숲길-보행자도로)
	InitPoints();
	
	reqInfoNew.RequestId = reqInfo.RequestId;
	reqInfoNew.RouteOption = reqInfo.RouteOption;
	reqInfoNew.AvoidOption = reqInfo.AvoidOption;
	reqInfoNew.StartDirIgnore = reqInfo.StartDirIgnore;
	reqInfoNew.WayDirIgnore = reqInfo.WayDirIgnore;
	reqInfoNew.EndDirIgnore = reqInfo.EndDirIgnore;

	for (int ii = 0; ii < reqInfo.vtPoints.size(); ii++) {
		reqInfoNew.vtPoints.push_back({ reqInfo.vtPoints[ii].x, reqInfo.vtPoints[ii].y });
		reqInfoNew.vtKeyType.emplace_back(reqInfo.vtKeyType[ii]); // node type
		reqInfoNew.vtKeyId.emplace_back(reqInfo.vtKeyId[ii]);
		reqInfoNew.vtLinkDataType.emplace_back(reqInfo.vtLinkDataType[ii]);

		// 마지막이면 
		if (ii == reqInfo.vtPoints.size() - 1) {
			continue;
		}

		// 입구점 계산 무시 케이스 (보행자-보행자, 동일 산 바운더리)
		//if (vtComplexRouteInfo[ii].optimalPointInfo.nType == vtOptPoint[ii + 1].nType) {
		if (((reqInfo.vtLinkDataType[ii] == TYPE_LINK_DATA_PEDESTRIAN) && (reqInfo.vtLinkDataType[ii + 1] == TYPE_LINK_DATA_PEDESTRIAN)) ||
			((reqInfo.vtLinkDataType[ii] == TYPE_LINK_DATA_TREKKING) && (reqInfo.vtLinkDataType[ii + 1] == TYPE_LINK_DATA_TREKKING) &&
			 (vtComplexRouteInfo[ii].optimalPointInfo.id != 0) && (vtComplexRouteInfo[ii].optimalPointInfo.id == vtComplexRouteInfo[ii + 1].optimalPointInfo.id))) {
			continue;
		}

		// 동일 산 바운더리 무시, 현재 데이터에 산코드(MNT)가 누락되어 우선 미사용
		//if (vtComplexRouteInfo[ii].optimalPointInfo.id == vtOptPoint[ii + 1].id) {
		//	continue;
		//}

		// 첫번째 지점이 입구점 보유 시, 두 지점간 가장 가까운 입구점 계산
		double dwDistMin = INT_MAX;
		int idxMin = -1;
		int idxNow = 0;
		for (const auto& ent : vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint) {
			double dwDistS = getRealWorldDistance(reqInfo.vtPoints[ii].x, reqInfo.vtPoints[ii].y, ent.x, ent.y);
			double dwDistE = getRealWorldDistance(ent.x, ent.y, reqInfo.vtPoints[ii+1].x, reqInfo.vtPoints[ii+1].y);

			if (dwDistS + dwDistE < dwDistMin) {
				dwDistMin = dwDistS + dwDistE;
				idxMin = idxNow;
			}
			idxNow++;
		}

		// 가까운 입구점을 경유지로 추가
		if (idxMin >= 0) {
			// 첫번째 지점과 링크 타입이 다를 경우, 첫번째 링크에 맞는 링크 매칭 한번, 현재 링크에 맞는 매칭 한번, 총 2개의 링크를 셋 한다.
			//keyId = GetPositionLink(vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].x, vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].y, TYPE_OPTIMAL_ENTRANCE_MOUNTAIN, TYPE_LINK_DATA_TREKKING);
			keyId.tile_id = g_forestMeshId;
			keyId.nid = vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].nID_1;
			keyId.dir = 0;
			if (keyId.llid != NOT_USE) {
				reqInfoNew.vtPoints.push_back({ vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].x, vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].y });
				reqInfoNew.vtKeyType.emplace_back(TYPE_KEY_NODE); // node type
				reqInfoNew.vtKeyId.emplace_back(keyId);
				reqInfoNew.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
			}

			//keyId = GetPositionLink(vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].x, vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].y, TYPE_OPTIMAL_ENTRANCE_MOUNTAIN, TYPE_LINK_DATA_PEDESTRIAN);
			keyId.tile_id = vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].nID_2 / 100000;
			keyId.nid = vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].nID_2 % 100000;
			keyId.dir = 0;
			if (keyId.llid != NOT_USE) {
				reqInfoNew.vtPoints.push_back({ vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].x, vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].y });
				reqInfoNew.vtKeyType.emplace_back(TYPE_KEY_NODE); // node type
				reqInfoNew.vtKeyId.emplace_back(keyId);
				reqInfoNew.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
			}
		}

		// 두번째 지점도 입구점을 보유 시, 두 지점간 가장 가까운 입구점 계산
		dwDistMin = INT_MAX;
		idxMin = -1;
		idxNow = 0;
		for (const auto& ent : vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint) {
			double dwDistS = getRealWorldDistance(reqInfo.vtPoints[ii + 1].x, reqInfo.vtPoints[ii + 1].y, ent.x, ent.y);
			double dwDistE = getRealWorldDistance(ent.x, ent.y, reqInfo.vtPoints[ii].x, reqInfo.vtPoints[ii].y);

			if (dwDistS + dwDistE < dwDistMin) {
				dwDistMin = dwDistS + dwDistE;
				idxMin = idxNow;
			}
			idxNow++;
		}

		// 가까운 입구점을 경유지로 추가
		if (idxMin >= 0) {
			// 첫번째 지점과 링크 타입이 다를 경우, 첫번째 링크에 맞는 링크 매칭 한번, 현재 링크에 맞는 매칭 한번, 총 2개의 링크를 셋 한다.
			//keyId = GetPositionLink(vtOptPoint[ii+1].vtEntryPoint[idxMin].x, vtOptPoint[ii+1].vtEntryPoint[idxMin].y, TYPE_OPTIMAL_ENTRANCE_NONE, TYPE_LINK_DATA_PEDESTRIAN);
			keyId.tile_id = vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].nID_2 / 100000;
			keyId.nid = vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].nID_2 % 100000;
			keyId.dir = 0;
			if (keyId.llid != NOT_USE) {
				reqInfoNew.vtPoints.push_back({ vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].x, vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].y });
				reqInfoNew.vtKeyType.emplace_back(TYPE_KEY_NODE); // node type
				reqInfoNew.vtKeyId.emplace_back(keyId);
				reqInfoNew.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
			}

			//keyId = GetPositionLink(vtOptPoint[ii+1].vtEntryPoint[idxMin].x, vtOptPoint[ii+1].vtEntryPoint[idxMin].y, TYPE_OPTIMAL_ENTRANCE_MOUNTAIN, TYPE_LINK_DATA_TREKKING);
			keyId.tile_id = g_forestMeshId;
			keyId.nid = vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].nID_1;
			keyId.dir = 0;
			if (keyId.llid != NOT_USE) {
				reqInfoNew.vtPoints.push_back({ vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].x, vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].y });
				reqInfoNew.vtKeyType.emplace_back(TYPE_KEY_NODE); // node type
				reqInfoNew.vtKeyId.emplace_back(keyId);
				reqInfoNew.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
			}
		}
	}

	// 마지막
	//reqInfoNew.vtPoints.emplace_back(reqInfo.vtPoints.back());
	//reqInfoNew.vtKeyType.emplace_back(TYPE_KEY_LINK); // link type
	//reqInfoNew.vtKeyId.emplace_back(reqInfo.vtKeyId.back());
	//reqInfoNew.vtIdDataType.emplace_back(reqInfo.vtIdDataType.back());


	// 지점 재 설정
	for (int ii = 0; ii < reqInfoNew.vtPoints.size(); ii++)
	{
		if (ii == 0) { // 출발지
			memcpy(&m_linkDeparture.Coord, &reqInfoNew.vtPoints[ii], sizeof(m_linkDeparture.Coord));
			m_linkDeparture.LinkId = reqInfoNew.vtKeyId[ii];
			m_linkDeparture.KeyType = reqInfoNew.vtKeyType[ii];
			m_linkDeparture.LinkDataType = reqInfoNew.vtLinkDataType[ii];
		}
		else if (ii == reqInfoNew.vtPoints.size() - 1) { // 목적지
			memcpy(&m_linkDestination.Coord, &reqInfoNew.vtPoints[ii], sizeof(m_linkDestination.Coord));
			m_linkDestination.LinkId = reqInfoNew.vtKeyId[ii];
			m_linkDestination.KeyType = reqInfoNew.vtKeyType[ii];
			m_linkDestination.LinkDataType = reqInfoNew.vtLinkDataType[ii];
		}
		else { // 경유지
			RouteLinkInfo linkInfo;
			linkInfo.Coord = reqInfoNew.vtPoints[ii];
			linkInfo.LinkId = reqInfoNew.vtKeyId[ii];
			linkInfo.KeyType = reqInfoNew.vtKeyType[ii];
			linkInfo.LinkDataType = reqInfoNew.vtLinkDataType[ii];
			m_linkWaypoints.emplace_back(linkInfo);
		}
	}


	if ((ret = m_pRoutePlan->DoComplexRoutes(&reqInfoNew, &m_vtRouteInfo, &m_vtRouteResult)) == 0)
#else
	if ((ret = m_pRoutePlan->DoComplexRoutesEx(&reqInfo/*, vtComplexPointInfo*/, &vtRouteInfo, &vtRouteResult)) == 0)
#endif
	{
		RouteResultInfo result;
		result.Init();

		result.ResultCode = vtRouteResult[0].ResultCode; // 경로 결과 코드, 0:성공, 1~:실패

		result.reqInfo.SetOption(&vtRouteResult[0].reqInfo);
		//result.RequestMode = vtRouteResult[0].RequestMode; // 요청 모드
		//result.RequestId = vtRouteResult[0].RequestId; // 요청 ID
		//result.RouteOption = vtRouteResult[0].RouteOption; // 경로 옵션
		//result.RouteAvoid = vtRouteResult[0].RouteAvoid; // 경로 회피

		result.StartResultLink = vtRouteResult[vtRouteResult.size() - 1].StartResultLink;
		result.EndResultLink = vtRouteResult[0].EndResultLink;

		RouteSummary summary;
		// for (int ii = vtRouteResult.size() - 1; ii >= 0; --ii) {
		for (const auto& route : vtRouteResult) {
			// summarys
			if (route.LinkInfo.empty()) {
				continue; // 이종간 경로 탐색 무시된 케이스
			}

			summary.TotalDist = static_cast<uint32_t>(route.TotalLinkDist);
			summary.TotalTime = route.TotalLinkTime;
			result.RouteSummarys.emplace_back(summary);

			// 경로선
			boxMerge(result.RouteBox, route.RouteBox);
			linkMerge(result.LinkVertex, route.LinkVertex);

			result.LinkInfo.reserve(result.LinkInfo.size() + route.LinkInfo.size());
			for (const auto& link : route.LinkInfo) {
				// 경유지가 있을 경우, 전체 경로를 합치면, 개별 offset 및 거리, 시간이 누적되어 변경되어야 함.
				// 작업 추가 필요
				result.LinkInfo.emplace_back(link);
			}

			result.TotalLinkDist += route.TotalLinkDist; // 경로 전체 거리
			result.TotalLinkCount += route.TotalLinkCount; // 경로 전체 링크 수
			result.TotalLinkTime += route.TotalLinkTime;; // 경로 전체 소요 시간 (초)
		} // for

		m_vtRouteResult.emplace_back(result);
	}

	LOG_TRACE(LOG_DEBUG, timeStart, "DoComplexRouting, result: %d", ret);

	return ret;
}


int CRouteManager::DoMultiComplexRouting(IN const int32_t routeCount/*, IN const int32_t routeOptions[], IN const int32_t routeAvoids[]*/)
{
	int ret = -1;

	const uint32_t uid = 12345678;
	const int32_t route_count = 3;

	RequestRouteInfo reqInfos[route_count];

	vector<RouteInfo> vtRouteInfos[route_count];
	vector<RouteResultInfo> vtRouteResults[route_count];

	//vector<int32_t> routeOptions;
	//vector<uint32_t> routeAvoids;

	CRoutePlan routePlans[route_count];

	m_vtRouteResult.resize(routeCount);

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "DoMultiComplexRouting, request cnt: %d", routeCount);

	for (int ii = 0; ii < routeCount; ii++) {
		routePlans[ii].Initialize();
		routePlans[ii].SetDataMgr(m_pDataMgr);
		routePlans[ii].SetRouteCost(m_pRoutePlan->GetRouteCost());

		reqInfos[ii].RequestId = uid;
		reqInfos[ii].RequestTime = m_nTimestampOpt;
		reqInfos[ii].RequestTraffic = m_nTrafficOpt;
		reqInfos[ii].MobilityOption = m_nMobilityOpt;
		reqInfos[ii].FreeOption = m_nFreeOpt;
		reqInfos[ii].RouteOption = m_vtRouteOpt[ii];
		reqInfos[ii].AvoidOption = m_vtAvoidOpt[ii];
		reqInfos[ii].RouteSubOption = m_routeSubOpt;
		reqInfos[ii].RouteTruckOption = m_routeTruckOpt;
		reqInfos[ii].StartDirIgnore = m_nDepartureDirIgnore;
		reqInfos[ii].WayDirIgnore = m_nWaypointDirIgnore;
		reqInfos[ii].EndDirIgnore = m_nDestinationDirIgnore;

		// start 
		reqInfos[ii].vtPoints.emplace_back(m_linkDeparture.Coord);
		reqInfos[ii].vtKeyId.emplace_back(m_linkDeparture.LinkId);
		reqInfos[ii].vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
#if defined(USE_FOREST_DATA)
		if (m_linkDeparture.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
		} else {
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
		}

		// 코스 ID 매칭 확인
		insertLinkCourseInfo(m_pDataMgr, m_linkDeparture.LinkId, m_linkDeparture.LinkDataType, reqInfos[ii].vtCourse);
#endif

		// 위 벡터들 통합
		reqInfos[ii].vtPointsInfo.emplace_back(m_linkDeparture);


		// waypoint
		for (const auto& waypoint : m_linkWaypoints) {
			reqInfos[ii].vtPoints.emplace_back(waypoint.Coord);
			reqInfos[ii].vtKeyId.emplace_back(waypoint.LinkId);
			reqInfos[ii].vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
#if defined(USE_FOREST_DATA)
			if (waypoint.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
				reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
			} else {
				reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
			}

			// 코스 ID 매칭 확인
			insertLinkCourseInfo(m_pDataMgr, waypoint.LinkId, waypoint.LinkDataType, reqInfos[ii].vtCourse);
#endif

			// 위 벡터들 통합
			reqInfos[ii].vtPointsInfo.emplace_back(waypoint);
		} // for

		  // end
		reqInfos[ii].vtPoints.emplace_back(m_linkDestination.Coord);
		reqInfos[ii].vtKeyId.emplace_back(m_linkDestination.LinkId);
		reqInfos[ii].vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭
#if defined(USE_FOREST_DATA)
		if (m_linkDestination.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
		} else {
			reqInfos[ii].vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
		}

		// 코스 ID 매칭 확인
		insertLinkCourseInfo(m_pDataMgr, m_linkDestination.LinkId, m_linkDestination.LinkDataType, reqInfos[ii].vtCourse);
#endif

		// 위 벡터들 통합
		reqInfos[ii].vtPointsInfo.emplace_back(m_linkDestination);
	}

#if defined(USE_MULTIPROCESS)
#pragma omp parallel for
#endif
	for (int ii = 0; ii < routeCount; ii++) {
		vtRouteInfos[ii].resize(reqInfos[ii].vtPoints.size());
		vtRouteResults[ii].resize(reqInfos[ii].vtPoints.size() - 1);

		// 입구점 여부 확인 필요 
		//vector<ComplexPointInfo> vtComplexPointInfo(reqInfos[ii].vtPoints.size());

		// 숲길인데 산바운더리 입구점이 없으면 링크 확장으로 입구점 검색
		routePlans[ii].DoEdgeRoute(&reqInfos[ii], vtRouteInfos[ii]/*, vtComplexPointInfo*/);

		// 경탐 수행
#if 0 // 각 지점간 이종 링크의 가장 가까운 입구점을 찾아 경유지로 선정하고 경로 탐색 
		KeyID keyId = { 0, };
		RequestRouteInfo reqInfoNew;

		// 각 지점간 이종 링크 경유 확인 (숲길-보행자도로)
		InitPoints();

		reqInfoNew.RequestId = reqInfo.RequestId;
		reqInfoNew.RouteOption = reqInfo.RouteOption;
		reqInfoNew.AvoidOption = reqInfo.AvoidOption;
		reqInfoNew.StartDirIgnore = reqInfo.StartDirIgnore;
		reqInfoNew.WayDirIgnore = reqInfo.WayDirIgnore;
		reqInfoNew.EndDirIgnore = reqInfo.EndDirIgnore;

		for (int ii = 0; ii < reqInfo.vtPoints.size(); ii++) {
			reqInfoNew.vtPoints.push_back({ reqInfo.vtPoints[ii].x, reqInfo.vtPoints[ii].y });
			reqInfoNew.vtKeyType.emplace_back(reqInfo.vtKeyType[ii]); // node type
			reqInfoNew.vtKeyId.emplace_back(reqInfo.vtKeyId[ii]);
			reqInfoNew.vtLinkDataType.emplace_back(reqInfo.vtLinkDataType[ii]);

			// 마지막이면 
			if (ii == reqInfo.vtPoints.size() - 1) {
				continue;
			}

			// 입구점 계산 무시 케이스 (보행자-보행자, 동일 산 바운더리)
			//if (vtComplexRouteInfo[ii].optimalPointInfo.nType == vtOptPoint[ii + 1].nType) {
			if (((reqInfo.vtLinkDataType[ii] == TYPE_LINK_DATA_PEDESTRIAN) && (reqInfo.vtLinkDataType[ii + 1] == TYPE_LINK_DATA_PEDESTRIAN)) ||
				((reqInfo.vtLinkDataType[ii] == TYPE_LINK_DATA_TREKKING) && (reqInfo.vtLinkDataType[ii + 1] == TYPE_LINK_DATA_TREKKING) &&
					(vtComplexRouteInfo[ii].optimalPointInfo.id != 0) && (vtComplexRouteInfo[ii].optimalPointInfo.id == vtComplexRouteInfo[ii + 1].optimalPointInfo.id))) {
				continue;
			}

			// 동일 산 바운더리 무시, 현재 데이터에 산코드(MNT)가 누락되어 우선 미사용
			//if (vtComplexRouteInfo[ii].optimalPointInfo.id == vtOptPoint[ii + 1].id) {
			//	continue;
			//}

			// 첫번째 지점이 입구점 보유 시, 두 지점간 가장 가까운 입구점 계산
			double dwDistMin = INT_MAX;
			int idxMin = -1;
			int idxNow = 0;
			for (const auto& ent : vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint) {
				double dwDistS = getRealWorldDistance(reqInfo.vtPoints[ii].x, reqInfo.vtPoints[ii].y, ent.x, ent.y);
				double dwDistE = getRealWorldDistance(ent.x, ent.y, reqInfo.vtPoints[ii + 1].x, reqInfo.vtPoints[ii + 1].y);

				if (dwDistS + dwDistE < dwDistMin) {
					dwDistMin = dwDistS + dwDistE;
					idxMin = idxNow;
				}
				idxNow++;
			}

			// 가까운 입구점을 경유지로 추가
			if (idxMin >= 0) {
				// 첫번째 지점과 링크 타입이 다를 경우, 첫번째 링크에 맞는 링크 매칭 한번, 현재 링크에 맞는 매칭 한번, 총 2개의 링크를 셋 한다.
				//keyId = GetPositionLink(vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].x, vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].y, TYPE_OPTIMAL_ENTRANCE_MOUNTAIN, TYPE_LINK_DATA_TREKKING);
				keyId.tile_id = g_forestMeshId;
				keyId.nid = vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].nID_1;
				keyId.dir = 0;
				if (keyId.llid != NOT_USE) {
					reqInfoNew.vtPoints.push_back({ vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].x, vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].y });
					reqInfoNew.vtKeyType.emplace_back(TYPE_KEY_NODE); // node type
					reqInfoNew.vtKeyId.emplace_back(keyId);
					reqInfoNew.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
				}

				//keyId = GetPositionLink(vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].x, vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].y, TYPE_OPTIMAL_ENTRANCE_MOUNTAIN, TYPE_LINK_DATA_PEDESTRIAN);
				keyId.tile_id = vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].nID_2 / 100000;
				keyId.nid = vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].nID_2 % 100000;
				keyId.dir = 0;
				if (keyId.llid != NOT_USE) {
					reqInfoNew.vtPoints.push_back({ vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].x, vtComplexRouteInfo[ii].optimalPointInfo.vtEntryPoint[idxMin].y });
					reqInfoNew.vtKeyType.emplace_back(TYPE_KEY_NODE); // node type
					reqInfoNew.vtKeyId.emplace_back(keyId);
					reqInfoNew.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
				}
			}

			// 두번째 지점도 입구점을 보유 시, 두 지점간 가장 가까운 입구점 계산
			dwDistMin = INT_MAX;
			idxMin = -1;
			idxNow = 0;
			for (const auto& ent : vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint) {
				double dwDistS = getRealWorldDistance(reqInfo.vtPoints[ii + 1].x, reqInfo.vtPoints[ii + 1].y, ent.x, ent.y);
				double dwDistE = getRealWorldDistance(ent.x, ent.y, reqInfo.vtPoints[ii].x, reqInfo.vtPoints[ii].y);

				if (dwDistS + dwDistE < dwDistMin) {
					dwDistMin = dwDistS + dwDistE;
					idxMin = idxNow;
				}
				idxNow++;
			}

			// 가까운 입구점을 경유지로 추가
			if (idxMin >= 0) {
				// 첫번째 지점과 링크 타입이 다를 경우, 첫번째 링크에 맞는 링크 매칭 한번, 현재 링크에 맞는 매칭 한번, 총 2개의 링크를 셋 한다.
				//keyId = GetPositionLink(vtOptPoint[ii+1].vtEntryPoint[idxMin].x, vtOptPoint[ii+1].vtEntryPoint[idxMin].y, TYPE_OPTIMAL_ENTRANCE_NONE, TYPE_LINK_DATA_PEDESTRIAN);
				keyId.tile_id = vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].nID_2 / 100000;
				keyId.nid = vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].nID_2 % 100000;
				keyId.dir = 0;
				if (keyId.llid != NOT_USE) {
					reqInfoNew.vtPoints.push_back({ vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].x, vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].y });
					reqInfoNew.vtKeyType.emplace_back(TYPE_KEY_NODE); // node type
					reqInfoNew.vtKeyId.emplace_back(keyId);
					reqInfoNew.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
				}

				//keyId = GetPositionLink(vtOptPoint[ii+1].vtEntryPoint[idxMin].x, vtOptPoint[ii+1].vtEntryPoint[idxMin].y, TYPE_OPTIMAL_ENTRANCE_MOUNTAIN, TYPE_LINK_DATA_TREKKING);
				keyId.tile_id = g_forestMeshId;
				keyId.nid = vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].nID_1;
				keyId.dir = 0;
				if (keyId.llid != NOT_USE) {
					reqInfoNew.vtPoints.push_back({ vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].x, vtComplexRouteInfo[ii + 1].optimalPointInfo.vtEntryPoint[idxMin].y });
					reqInfoNew.vtKeyType.emplace_back(TYPE_KEY_NODE); // node type
					reqInfoNew.vtKeyId.emplace_back(keyId);
					reqInfoNew.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
				}
			}
		}

		// 마지막
		//reqInfoNew.vtPoints.emplace_back(reqInfo.vtPoints.back());
		//reqInfoNew.vtKeyType.emplace_back(TYPE_KEY_LINK); // link type
		//reqInfoNew.vtKeyId.emplace_back(reqInfo.vtKeyId.back());
		//reqInfoNew.vtIdDataType.emplace_back(reqInfo.vtIdDataType.back());


		// 지점 재 설정
		for (int ii = 0; ii < reqInfoNew.vtPoints.size(); ii++)
		{
			if (ii == 0) { // 출발지
				memcpy(&m_linkDeparture.Coord, &reqInfoNew.vtPoints[ii], sizeof(m_linkDeparture.Coord));
				m_linkDeparture.LinkId = reqInfoNew.vtKeyId[ii];
				m_linkDeparture.KeyType = reqInfoNew.vtKeyType[ii];
				m_linkDeparture.LinkDataType = reqInfoNew.vtLinkDataType[ii];
			}
			else if (ii == reqInfoNew.vtPoints.size() - 1) { // 목적지
				memcpy(&m_linkDestination.Coord, &reqInfoNew.vtPoints[ii], sizeof(m_linkDestination.Coord));
				m_linkDestination.LinkId = reqInfoNew.vtKeyId[ii];
				m_linkDestination.KeyType = reqInfoNew.vtKeyType[ii];
				m_linkDestination.LinkDataType = reqInfoNew.vtLinkDataType[ii];
			}
			else { // 경유지
				RouteLinkInfo linkInfo;
				linkInfo.Coord = reqInfoNew.vtPoints[ii];
				linkInfo.LinkId = reqInfoNew.vtKeyId[ii];
				linkInfo.KeyType = reqInfoNew.vtKeyType[ii];
				linkInfo.LinkDataType = reqInfoNew.vtLinkDataType[ii];
				m_linkWaypoints.emplace_back(linkInfo);
			}
	}


		if ((ret = m_pRoutePlan->DoComplexRoutes(&reqInfoNew, &vtRouteInfos[ii], &m_vtRouteResult)) == 0)
#else
		if ((ret = routePlans[ii].DoComplexRoutesEx(&reqInfos[ii]/*, vtComplexPointInfo*/, &vtRouteInfos[ii], &vtRouteResults[ii])) == 0)
#endif
		{
			m_vtRouteResult[ii].Init();

			m_vtRouteResult[ii].ResultCode = vtRouteResults[ii][0].ResultCode; // 경로 결과 코드, 0:성공, 1~:실패

			m_vtRouteResult[ii].reqInfo.SetOption(&vtRouteResults[ii][0].reqInfo);
			//m_vtRouteResult[ii].RequestMode = vtRouteResults[ii][0].RequestMode; // 요청 모드
			//m_vtRouteResult[ii].RequestId = vtRouteResults[ii][0].RequestId; // 요청 ID
			//m_vtRouteResult[ii].RouteOption = vtRouteResults[ii][0].RouteOption; // 경로 옵션
			//m_vtRouteResult[ii].RouteAvoid = vtRouteResults[ii][0].RouteAvoid; // 경로 회피

			m_vtRouteResult[ii].StartResultLink = vtRouteResults[ii][vtRouteResults[ii].size() - 1].StartResultLink;
			m_vtRouteResult[ii].EndResultLink = vtRouteResults[ii][0].EndResultLink;

			RouteSummary summary;
			// for (int ii = m_vtRouteResult.size() - 1; ii >= 0; --ii) {
			for (const auto& route : vtRouteResults[ii]) {
				// summarys
				if (route.LinkInfo.empty()) {
					continue; // 이종간 경로 탐색 무시된 케이스
				}

				summary.TotalDist = static_cast<uint32_t>(route.TotalLinkDist);
				summary.TotalTime = route.TotalLinkTime;
				m_vtRouteResult[ii].RouteSummarys.emplace_back(summary);

				// 경로선
				boxMerge(m_vtRouteResult[ii].RouteBox, route.RouteBox);
				linkMerge(m_vtRouteResult[ii].LinkVertex, route.LinkVertex);

				m_vtRouteResult[ii].LinkInfo.reserve(m_vtRouteResult[ii].LinkInfo.size() + route.LinkInfo.size());
				for (const auto& link : route.LinkInfo) {
					// 경유지가 있을 경우, 전체 경로를 합치면, 개별 offset 및 거리, 시간이 누적되어 변경되어야 함.
					// 작업 추가 필요
					m_vtRouteResult[ii].LinkInfo.emplace_back(link);
				}

				m_vtRouteResult[ii].TotalLinkDist += route.TotalLinkDist; // 경로 전체 거리
				m_vtRouteResult[ii].TotalLinkCount += route.TotalLinkCount; // 경로 전체 링크 수
				m_vtRouteResult[ii].TotalLinkTime += route.TotalLinkTime;; // 경로 전체 소요 시간 (초)
			}
		}
	}

	LOG_TRACE(LOG_DEBUG, timeStart, "DoMultiComplexRouting, result: %d", ret);

	return ret;
}


void CRouteManager::SetLimitPointCount(IN const int32_t nCount)
{
	if (m_pTmsMgr) {
		return m_pTmsMgr->SetLimitPointCount(nCount);
	}
}
const int32_t CRouteManager::GetLimitPointCount(void) const
{
	if (m_pTmsMgr) {
		return m_pTmsMgr->GetLimitPointCount();
	}
}


int CRouteManager::ParsingBaseOption(IN const char* szRequest, OUT BaseOption& option)
{
	int ret = RESULT_FAILED;

	if (szRequest == nullptr || strlen(szRequest) <= 0) {
		LOG_TRACE(LOG_WARNING, "ParsingBaseOption request failed, request string is null or zero size");
		return ret;
	}

	uint32_t crc = m_pTmsMgr->ParsingRequestBaseOption(szRequest, option);

	return ret;
}


int32_t CRouteManager::ParsingRequestRoute(IN const char* szRequest, OUT BaseOption& baseOpt, OUT vector<stWaypoint>& vtOrigin)
{
	int ret = RESULT_FAILED;

	if (szRequest == nullptr || strlen(szRequest) <= 0) {
		LOG_TRACE(LOG_WARNING, "ParsingRequestRoute request failed, request string is null or zero size");
		return ret;
	}

	uint32_t crc = m_pTmsMgr->ParsingRequestRoute(szRequest, baseOpt, vtOrigin);

	return ret;
}


int CRouteManager::ParsingWeightMatrix(IN const char* szRequest, OUT RouteDistMatrix& RDM, OUT BaseOption& option)
{
	int ret = RESULT_FAILED;

	if (szRequest == nullptr || strlen(szRequest) <= 0) {
		LOG_TRACE(LOG_WARNING, "ParsingWeightMatrix request failed, request string is null or zero size");
		return ret;
	}

	time_t timeStart = timeStart = LOG_TRACE(LOG_DEBUG, "ParsingWeightMatrix request: %s", szRequest);

	ret = m_pTmsMgr->ParsingRequestWeightMatrix(szRequest, option, RDM.vtOrigin, RDM.vtDestination, RDM.vtDistMatrix, RDM.typeCreate);

	LOG_TRACE(LOG_DEBUG, timeStart, "ParsingWeightMatrix, result: %d", ret);

	return ret;
}


int CRouteManager::GetWeightMatrix(IN OUT RouteDistMatrix& RDM, const IN BaseOption& option)
{
	int ret = RESULT_FAILED;
	time_t timeStart = TICK_COUNT();
	uint32_t crc = 0;

	timeStart = LOG_TRACE(LOG_DEBUG, "GetWeightMatrix request");

	const int cntOrigins = RDM.vtOrigin.size();
	const int cntDestinations = RDM.vtDestination.size();

	// 서버 사양에 따른 개수 제한 필요 (메모리 피크 등)
	static const int32_t MAX_TMS_LIMIT_POINT_COUNT = 1000; // 현재(2025-10-21) 기준 최대 1000개 이하만 처리하자 
	const int32_t nMaxExceedCount = GetLimitPointCount();

	if (cntOrigins <= 0) {
		ret = ROUTE_RESULT_FAILED_WRONG_PARAM;
		return ret;
	} else if ((cntOrigins > MAX_TMS_LIMIT_POINT_COUNT) || (cntDestinations > MAX_TMS_LIMIT_POINT_COUNT)) {
		ret = TMS_RESULT_FAILED_EXCEEDS_COUNT;
		return ret;
	} else 	if (nMaxExceedCount > 0) {
		if (cntDestinations <= 0) { // N x N 테이블 요청일 경우
			if (cntOrigins > nMaxExceedCount) {
				ret = TMS_RESULT_FAILED_EXCEEDS_COUNT;
				return ret;
			}
		} else if ((cntOrigins * cntDestinations) > (nMaxExceedCount * 2)) { // N x M 테이블 요청일 경우
			ret = TMS_RESULT_FAILED_EXCEEDS_COUNT;
			return ret;
		}
	}


	// get table
	LOGTIME timeNow;
	time_t tmNow = LOG_TIME(timeNow, RDM.tmCreate);

	bool isRead = false;
	bool isWritten = false;
	const int32_t sizeItem = sizeof(stDistMatrix); // sizeof(stDistMatrix::nTotalDist) + sizeof(stDistMatrix::nTotalTime) + sizeof(stDistMatrix::dbTotalCost);

	RequestRouteInfo reqInfo;
	reqInfo.RequestId = option.userId;
	reqInfo.RouteMode = option.route_mode;
	reqInfo.RouteCost = option.route_cost;
	reqInfo.RequestTime = option.timestamp;
	reqInfo.RequestTraffic = option.traffic;
	reqInfo.MobilityOption = option.mobility;
	reqInfo.FreeOption = option.free;

	reqInfo.RouteSubOption.rdm.distance_type = option.distance_type;
	reqInfo.RouteSubOption.rdm.compare_type = option.compare_type;
	reqInfo.RouteSubOption.rdm.expand_method = option.expand_method;

	reqInfo.RouteTruckOption.setTruckOption(&option.truck);

	// add rdm timestamp
	RDM.strUser = reqInfo.RequestId;
	RDM.tmCreate = time(NULL); // current time

	if ((RDM.typeCreate != 0) && (!RDM.vtDistMatrix.empty())) {
		// 선행 RDM 데이터 획득했으면 여기서 완료
		LOG_TRACE(LOG_DEBUG, "restore distance matrix, success, type:%d", RDM.typeCreate);
		ret = RESULT_OK;
	} else if (option.distance_type == 0) {
		// 직선거리 테이블 생성
		ret = DoDistanceMatrix(RDM);
	} else if (cntDestinations != 0) {
		// 출/도착지 정보가 각기 존재하면 NxN이 아닌 NxM으로 처리하고, 캐시 정보를 사용하지 않는다.
		ret = DoWeightMatrix(RDM, reqInfo);
	} else {
		//if (RDM.vtDistMatrix.empty() && (option.fileCache == 1 || option.fileCache == 3)) { // read, read-write
		//	BaseOption optionFile;
		//	ret = m_pTmsMgr->LoadWeightMatrix(szFilePath, cntOrigins, sizeItem, crc, optionFile, RDM.vtOrigin, RDM.vtDistMatrix);
		//	if (ret == RESULT_OK) {
		//		isRead = true;
		//		memcpy(&option, &optionFile, sizeof(BaseOption)); // 저장된 rdm 옵션으로 변경

		//		LOG_TRACE(LOG_DEBUG, "LoadWeightMatrix, success, cache:%s", szFilePath);
		//	}
		//}

		// 캐쉬된 WM이 없으면 새로 생성
		if (RDM.vtDistMatrix.empty() && !isRead) {
			ret = DoWeightMatrix(RDM, reqInfo);

			const int MAX_RDM_BUFF = 1024;
			char szDirName[MAX_RDM_BUFF] = { 0, };
			char szFileName[MAX_RDM_BUFF] = { 0, };
			char szFilePath[MAX_RDM_BUFF] = { 0, };

			// make epoch time
			int64_t epochtime = static_cast<int64_t>(tmNow) / 1000; // 밀리초라면 /1000, 초라면 그대로

			// make dir usr/rdm/yyyy/mm/dd
			sprintf(szDirName, "%s/usr/rdm/%04d/%02d/%02d", m_pDataMgr->GetDataPath(), timeNow.year, timeNow.month, timeNow.day);
			checkDirectory(szDirName, true);

			// 테이블 데이터 미리 읽어 저장하면서 사용하자
			if ((ret == RESULT_OK) && (option.artifact == 1 || option.artifact == 3)) { // write, read-write)) {
				sprintf(szFileName, "_t%010lld_d%04d%02d%02d%02d%02d%02d_u%s.rdm", epochtime, timeNow.year, timeNow.month, timeNow.day, timeNow.hour, timeNow.minute, timeNow.second, RDM.strUser.c_str());
				sprintf(szFilePath, "%s/%s", szDirName, szFileName);
				int32_t retSize = m_pTmsMgr->SaveWeightMatrix(szFilePath, &option, cntOrigins, sizeItem, crc, RDM.vtOrigin, RDM.vtDistMatrix);
				if (retSize > 0) {
					isWritten = true;
					RDM.infoRouteMatrix.type = "file";
					RDM.infoRouteMatrix.form = "bin";
					RDM.infoRouteMatrix.data = szFileName;
					RDM.infoRouteMatrix.size = retSize;
					RDM.infoRouteMatrix.created = epochtime;
				}
			}

			// 테이블 데이터 미리 읽어 저장하면서 사용하자
			if ((ret == RESULT_OK) && (option.artifact == 2 || option.artifact == 3)) { // Route line)) {
				sprintf(szFileName, "_t%010lld_d%04d%02d%02d%02d%02d%02d_u%s.rpm", epochtime, timeNow.year, timeNow.month, timeNow.day, timeNow.hour, timeNow.minute, timeNow.second, RDM.strUser.c_str());
				sprintf(szFilePath, "%s/%s", szDirName, szFileName);
				int32_t retSize = m_pTmsMgr->SaveWeightMatrixRouteLine(szFilePath, RDM.vtPathMatrix);
				if (retSize > 0) {
					isWritten = true;
					RDM.infoPathMatrix.type = "file";
					RDM.infoPathMatrix.form = "bin";
					RDM.infoPathMatrix.data = szFileName;
					RDM.infoPathMatrix.size = retSize;
					RDM.infoPathMatrix.created = epochtime;
				}
			}
		} else {
			ret = RESULT_OK;
		}
	}

	LOG_TRACE(LOG_DEBUG, timeStart, "GetWeightMatrix, result: %d", ret);

	return ret;
}


int CRouteManager::ParsingWeightMatrixRoute(IN const char* szRequest, OUT RouteDistMatrix& RDM, OUT BaseOption& option)
{
	int ret = RESULT_FAILED;

	if (szRequest == nullptr || strlen(szRequest) <= 0) {
		LOG_TRACE(LOG_WARNING, "ParsingWeightMatrixRoute request failed, request string is null or zero size");
		return ret;
	}

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "ParsingWeightMatrixRoute request, lenth: %d", strlen(szRequest));

	ret = m_pTmsMgr->ParsingRequestWeightMatrixRoute(szRequest, option, RDM.vtOrigin, RDM.vtDestination, RDM.vtDistMatrix);

	LOG_TRACE(LOG_DEBUG, timeStart, "ParsingWeightMatrixRoute, result: %d", ret);

	return ret;
}


int CRouteManager::ParsingWeightMatrixRoutePathIndex(IN const char* szRequest, OUT RoutePathMatrixIndex& RPI)
{
	int ret = RESULT_FAILED;

	if (szRequest == nullptr || strlen(szRequest) <= 0) {
		LOG_TRACE(LOG_WARNING, "ParsingWeightMatrixRoutePathIndex request failed, request string is null or zero size");
		return ret;
	}

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "ParsingWeightMatrixRoutePathIndex request, lenth: %d", strlen(szRequest));

	ret = m_pTmsMgr->ParsingRequestWeightMatrixRoutePathIndex(szRequest, RPI.strFileName, RPI.sizeFile, RPI.vtPathFileIndex);

	LOG_TRACE(LOG_DEBUG, timeStart, "ParsingWeightMatrixRoutePathIndex, result: %d", ret);

	return ret;
}


int CRouteManager::ParsingWeightMatrixRoutePathData(IN const char* szRequest, OUT vector<WeightMatrixPath>& vtPathMatrixData)
{
	int ret = RESULT_FAILED;

	if (szRequest == nullptr || strlen(szRequest) <= 0) {
		LOG_TRACE(LOG_WARNING, "ParsingWeightMatrixRoutePathData request failed, request string is null or zero size");
		return ret;
	}

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "ParsingWeightMatrixRoutePathData request, lenth: %d", strlen(szRequest));

	ret = m_pTmsMgr->ParsingRequestWeightMatrixRoutePathData(szRequest, vtPathMatrixData);

	LOG_TRACE(LOG_DEBUG, timeStart, "ParsingWeightMatrixRoutePathData, result: %d", ret);

	return ret;
}


int CRouteManager::GetCluster_for_geoyoung(IN const int32_t cntCluster, OUT vector<stCluster>& vtCluster)
{
	int ret = -1;

	vector<SPoint> vtPois;
	// 1209	
	array<int, 19> arrClusterCnt = {
		91, 68, 30, 57, 60, 60, 57, 64, 56, 67, 60, 65, 68, 70, 71, 77, 61, 62, 65
	};


	// add original coordinates
	int cntVias = GetWayPointCount();
	time_t timeStart = LOG_TRACE(LOG_DEBUG, "GetCluster_for_geoyoung, vias : %d", cntVias);

	vector<SPoint> vtOrigins;
	vtOrigins.reserve(cntVias + 2);
	vtOrigins.push_back({GetDeparture()->x, GetDeparture()->y});
	for (int ii=0; ii<cntVias; ii++) {
		vtOrigins.push_back({GetWaypoint(ii)->x, GetWaypoint(ii)->y});
	}
	vtOrigins.push_back({GetDestination()->x, GetDestination()->y});


	int cntPois = vtOrigins.size();
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
	for (const auto& items : arrClusterCnt) {
		stCluster cluster;
		cluster.id = idCluster++;

		LOG_TRACE(LOG_DEBUG, "GetCluster_for_geoyoung, cluster id : %d", items);

		// cluster pois
		for (int jj=0; jj<items; jj++) {
			cluster.vtCoord.emplace_back(vtOrigins[cntAdded]);
			cntAdded++;
		} // for

		// make border
		GetBoundary(cluster.vtCoord, cluster.vtBorder, cluster.center);

		vtCluster.emplace_back(cluster);
	} // for

	ret = ROUTE_RESULT_SUCCESS;

	LOG_TRACE(LOG_DEBUG, timeStart, "GetCluster_for_geoyoung, result: %d", ret);

	return ret;
}


int CRouteManager::GetCluster(IN const char* szRequest, IN RouteDistMatrix& RDM, OUT Cluster& CLUST)
{
	int ret = RESULT_FAILED;

	if (szRequest == nullptr || strlen(szRequest) <= 0) {
		LOG_TRACE(LOG_WARNING, "GetCluster request failed, request string is null or zero size");
		return ret;
	}

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "GetCluster request : %s", szRequest);

	if (!CLUST.vtCluster.empty()) {
		CLUST.vtCluster.clear();
		vector<stCluster>().swap(CLUST.vtCluster);
	}

	// get table
	// 저장된 RDM, RND 요청 정보 확인
	if ((ret = ParsingWeightMatrixRoute(szRequest, RDM, CLUST.option.tspOption.baseOption)) != RESULT_OK) {
		ret = ParsingWeightMatrix(szRequest, RDM, CLUST.option.tspOption.baseOption);
		if (RDM.vtDistMatrix.empty()) {
			ret = GetWeightMatrix(RDM, CLUST.option.tspOption.baseOption);
		}
	}

	uint32_t crc = m_pTmsMgr->ParsingRequestCluster(szRequest, CLUST.option, RDM.vtOrigin);
	const int cntOrigins = RDM.vtOrigin.size();

	if (cntOrigins <= 0) {
		return ROUTE_RESULT_FAILED_WRONG_PARAM;
	}

	// get cluster
	//#define USE_GN_KMEANS_ALORITHM // k-means 알고리즘 사용
#if defined(USE_KMEANS_ALORITHM)
		double** ppWeightMatrix = nullptr;

		// 실거리 테이블이 있으면 사용, 없으면 직선거리 적용
		if (ppTables != nullptr) {
			// wm 생성
			ppWeightMatrix = new double*[cntPois];
			for (int ii = 0; ii < cntPois; ii++) {
				ppWeightMatrix[ii] = new double[cntPois];

				// copy data
				for (int jj = 0; jj < cntPois; jj++) {
					ppWeightMatrix[ii][jj] = ppTables[ii][jj].nTotalDist;
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
			} else {
				LOG_TRACE(LOG_WARNING, "new cluster index not exist, idx:%d", item.group);
			}
		} // for
#else // 밀도 균등을 위해 반복 처리
		// 밀도 균등을 위해 반복 처리
		// 클러스터 수만큼 반복하여, 평균치에 가까운 녀석을 순차적으로 빼면서 반복
		const int32_t cntItemAvg = cntPois / MAX_CLUSTER_COUNT;
		const int32_t cntItemBand = cntItemAvg * 5 / 100; // 위아래 5%까지 허용

		for (; 0 < remainedCluster - 2;) {
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
				} else {
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
				} else {
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

				for (const auto& dist : mapDist) {
					// 밀도 균등 평균치 까지만 저장
					if (tmpAddDistrict.pois.size() < cntItemAvg) {
						tmpAddDistrict.pois.emplace_back(candidateIt->pois[dist.second]);
					} else {
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
		} else //if (0) {
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

		// release
		kmc.m_vtInput.clear();
		kmc.m_vtResult.clear();


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

		ret = RESULT_OK;

		for (int ii = 0; ii < cntPois; ii++) {
			if (ppWeightMatrix) {
				SAFE_DELETE_ARR(ppWeightMatrix[ii]);
			}
		}
		SAFE_DELETE_ARR(ppWeightMatrix);

#else // #define USE_KMEANS_ALORITHM // k-means 알고리즘 사용
	ret = m_pTmsMgr->GetCluster(&CLUST.option, RDM.vtDistMatrix, RDM.vtOrigin, CLUST.vtCluster, CLUST.vtEndPoint);
#endif // #define USE_KMEANS_ALORITHM // k-means 알고리즘 사용


	if (!RDM.vtDistMatrix.empty()) {
		RDM.vtDistMatrix.clear();
		vector<vector<stDistMatrix>>().swap(RDM.vtDistMatrix);
	}

	LOG_TRACE(LOG_DEBUG, timeStart, "GetCluster, result: %d", ret);

	return ret;
}


int CRouteManager::GetGroup(IN const char* szRequest, IN OUT Cluster& CLUST)
{
	int ret = RESULT_FAILED;

	if (szRequest == nullptr || strlen(szRequest) <= 0) {
		LOG_TRACE(LOG_WARNING, "GetGroup request failed, request string is null or zero size");
		return ret;
	}

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "GetGroup request : %s", szRequest);

	uint32_t crc = m_pTmsMgr->ParsingRequestGroup(szRequest, CLUST.option, CLUST.vtDistrict);
	const int cntOrigins = CLUST.vtDistrict.size();

	if (cntOrigins <= 0) {
		return ROUTE_RESULT_FAILED_WRONG_PARAM;
	}

	// get cluster
	ret = m_pTmsMgr->GetGroup(&CLUST.option, CLUST.vtDistrict, CLUST.vtCluster);

	LOG_TRACE(LOG_DEBUG, timeStart, "GetGroup, result: %d", ret);

	return ret;
}


int CRouteManager::GetBestway(IN const char* szRequest, IN RouteDistMatrix& RDM, OUT BestWaypoints& TSP)
{
	int ret = RESULT_FAILED;

	if (szRequest == nullptr || strlen(szRequest) <= 0) {
		LOG_TRACE(LOG_WARNING, "GetBestway request failed, request string is null or zero size");
		return ret;
	}

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "GetBestway, request: %s", szRequest);

	// 사용자 RDM 정보가 있으면 우선 사용
	if (!RDM.vtOrigin.empty() && !RDM.vtDistMatrix.empty()) {
		ret = RESULT_OK;
	} else {
		ret = m_pTmsMgr->ParsingRequestBestway(szRequest, TSP.option, RDM.vtOrigin, RDM.vtDistMatrix);
	}

	if (RDM.vtDistMatrix.empty()) {
		// get table
		ret = ParsingWeightMatrix(szRequest, RDM, TSP.option.baseOption);
		if (RDM.vtDistMatrix.empty()) {
			ret = GetWeightMatrix(RDM, TSP.option.baseOption);
		}
	}
	
	if (ret == RESULT_OK) {
		stWaypoint waypoint;
		int idx = 0;
		for (const auto& pt : RDM.vtOrigin) {
			waypoint.id = idx++;
			waypoint.position = pt.position;
			waypoint.layoverTime = pt.layoverTime;

			TSP.vtWaypoints.emplace_back(waypoint);
		}
		
		ret = m_pTmsMgr->GetBestway(&TSP.option, RDM.vtDistMatrix, TSP.vtWaypoints, TSP.vtBestWays, TSP.totalDist, TSP.totalTime);
		if (ret == RESULT_OK) {
			const int cntBestWays = TSP.vtBestWays.size();
			TSP.vtBestDist.clear(); TSP.vtBestDist.reserve(cntBestWays);
			TSP.vtBestTime.clear(); TSP.vtBestTime.reserve(cntBestWays);
			
			for (int curr = 0; curr < TSP.vtBestWays.size(); curr++) {
				if (curr <= 0) {
					TSP.vtBestDist.push_back(0);
					TSP.vtBestTime.push_back(0);
				} else {
					int prev = TSP.vtBestWays[curr - 1];
					int next = TSP.vtBestWays[curr];
					TSP.vtBestDist.push_back(RDM.vtDistMatrix[prev][next].nTotalDist);
					TSP.vtBestTime.push_back(RDM.vtDistMatrix[prev][next].nTotalTime);
				}
			}
		}
	}

	LOG_TRACE(LOG_DEBUG, timeStart, "GetBestway, result: %d, dist:%.2f, time:%d", ret, TSP.totalDist, TSP.totalTime);

	return ret;
}


int CRouteManager::GetBoundary(IN const vector<SPoint>& vtPois, OUT vector<SPoint>& vtBoundary, OUT SPoint& center)
{
	if (vtPois.empty()) {
		LOG_TRACE(LOG_WARNING, "GetClusterBoundary, request failed, poi cnt:%d", vtPois.size());
		return -1;
	}

	vector<SPoint> coords;
	vector<SPoint> border;

	for (const auto& poi : vtPois) {
		LOG_TRACE(LOG_TEST, "poi, x:%.5f, y:%.5f", poi.x, poi.y);
		coords.emplace_back(poi);
	}

	// make border
	m_pTmsMgr->GetBoundary(coords, vtBoundary, center);

	return 0;
}


int CRouteManager::DoCourse(/*Packet*/)
{
	int ret = ROUTE_RESULT_SUCCESS;

	const uint32_t uid = 12345678;

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "DoCourse, request");

	RequestRouteInfo reqInfo;
	reqInfo.RequestId = uid;
	reqInfo.RequestTime = m_nTimestampOpt;
	reqInfo.RequestTraffic = m_nTrafficOpt;
	reqInfo.MobilityOption = m_nMobilityOpt;
	reqInfo.FreeOption = m_nFreeOpt;
	reqInfo.RouteOption = ROUTE_OPT_SHORTEST;//m_vtRouteOpt[0]; // 추천은 속성 영향을 안받는 짧은 길 사용
	reqInfo.AvoidOption = m_vtAvoidOpt[0];
	reqInfo.RouteSubOption = m_routeSubOpt;
	reqInfo.RouteTruckOption = m_routeTruckOpt;
	reqInfo.StartDirIgnore = m_nDepartureDirIgnore;
	reqInfo.WayDirIgnore = m_nWaypointDirIgnore;
	reqInfo.EndDirIgnore = m_nDestinationDirIgnore;

	// start 
	reqInfo.vtPoints.emplace_back(m_linkDeparture.Coord);
	reqInfo.vtKeyId.emplace_back(m_linkDeparture.LinkId);
#if defined(USE_MOUNTAIN_DATA)
	if (m_linkDestination.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
		reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
	} else {
		reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
#elif defined(USE_VEHICLE_DATA)
	reqInfo.vtLinkDataType.emplace_back(TYPE_DATA_VEHICLE);
#endif
	reqInfo.vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭

	// 위 벡터들 통합
	reqInfo.vtPointsInfo.emplace_back(m_linkDeparture);

	// waypoint
	if (!m_linkWaypoints.empty()) {
		for (const auto& via : m_linkWaypoints) { //(int ii = 0; ii < m_linkWaypoints.size(); ii++) {
			reqInfo.vtPoints.emplace_back(via.Coord);
			reqInfo.vtKeyId.emplace_back(via.LinkId);
#if defined(USE_MOUNTAIN_DATA)
			if (via.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
				reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
			} else {
				reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
			}
#elif defined(USE_PEDESTRIAN_DATA)
			reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
#elif defined(USE_VEHICLE_DATA)
			reqInfo.vtLinkDataType.emplace_back(TYPE_DATA_VEHICLE);
#endif
			reqInfo.vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭

			// 위 벡터들 통합
			reqInfo.vtPointsInfo.emplace_back(via);
		} // for
	}

	// end
	reqInfo.vtPoints.emplace_back(m_linkDestination.Coord);
	reqInfo.vtKeyId.emplace_back(m_linkDestination.LinkId);
#if defined(USE_MOUNTAIN_DATA)
	if (m_linkDestination.LinkDataType == TYPE_LINK_DATA_TREKKING) { // 링크 속성으로 데이터 타입 변경
		reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_TREKKING);
	} else {
		reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	reqInfo.vtLinkDataType.emplace_back(TYPE_LINK_DATA_PEDESTRIAN);
#elif defined(USE_VEHICLE_DATA)
	reqInfo.vtLinkDataType.emplace_back(TYPE_DATA_VEHICLE);
#endif
	reqInfo.vtKeyType.emplace_back(TYPE_KEY_LINK); // 기본은 링크 매칭

	// 위 벡터들 통합
	reqInfo.vtPointsInfo.emplace_back(m_linkDestination);


	// 지점들이 정상 매칭되지 않았으면 실패 리턴
	for (const auto& key : reqInfo.vtKeyId) {
		if (key.llid == 0) {
			ret = ROUTE_RESULT_FAILED_COURSE;
			break;
		}
	}

	// 출도착지의 링크 속성이 자전거길 일경우 mobility를 자전거로 변경
	stLinkInfo* pLink = m_pDataMgr->GetLinkDataById(reqInfo.vtKeyId[0], reqInfo.vtLinkDataType[0]);
	if (pLink != nullptr) {		
		if ((pLink->trk.course_type == TYPE_TRE_BIKE) && (reqInfo.MobilityOption != TYPE_MOBILITY_BICYCLE)) {
			LOG_TRACE(LOG_DEBUG, "mobility option change, old:%d -> new:%d", m_nMobilityOpt, TYPE_MOBILITY_BICYCLE);
			reqInfo.MobilityOption = m_nMobilityOpt = TYPE_MOBILITY_BICYCLE;
		}
	}


	if (ret == ROUTE_RESULT_SUCCESS) {
		if (!m_vtRouteInfo.empty()) {
			m_vtRouteInfo.clear();
			vector<RouteInfo>().swap(m_vtRouteInfo);
		}

		if (!m_vtRouteResult.empty()) {
			m_vtRouteResult.clear();
			vector<RouteResultInfo>().swap(m_vtRouteResult);
		}

		vector<RouteInfo> vtRouteInfo;
		vector<RouteResultInfo> vtRouteResult;

		if ((ret = m_pRoutePlan->DoCourse(&reqInfo, &vtRouteInfo, &vtRouteResult)) == ROUTE_RESULT_SUCCESS) {
			RouteResultInfo result;

			result.ResultCode = vtRouteResult[0].ResultCode; // 경로 결과 코드, 0:성공, 1~:실패

			result.reqInfo.SetOption(&vtRouteResult[0].reqInfo);
			//result.RequestMode = vtRouteResult[0].RequestMode; // 요청 모드
			//result.RequestId = vtRouteResult[0].RequestId; // 요청 ID
			//result.RouteOption = vtRouteResult[0].RouteOption; // 경로 옵션
			//result.RouteAvoid = vtRouteResult[0].RouteAvoid; // 경로 회피

			result.StartResultLink = vtRouteResult[vtRouteResult.size() - 1].StartResultLink;
			result.EndResultLink = vtRouteResult[0].EndResultLink;

			RouteSummary summary;
			// for (int ii = vtRouteResult.size() - 1; ii >= 0; --ii) {
			for (const auto& route : vtRouteResult) {
				// summarys
				summary.TotalDist = static_cast<uint32_t>(route.TotalLinkDist);
				summary.TotalTime = route.TotalLinkTime;
				result.RouteSummarys.emplace_back(summary);

				// 경로선
				boxMerge(result.RouteBox, route.RouteBox);
				linkMerge(result.LinkVertex, route.LinkVertex);

				result.LinkInfo.reserve(result.LinkInfo.size() + route.LinkInfo.size());
				for (const auto& link : route.LinkInfo) {
					// 경유지가 있을 경우, 전체 경로를 합치면, 개별 offset 및 거리, 시간이 누적되어 변경되어야 함.
					// 작업 추가 필요
					result.LinkInfo.emplace_back(link);
				} // for

				result.TotalLinkDist += route.TotalLinkDist; // 경로 전체 거리
				result.TotalLinkCount += route.TotalLinkCount; // 경로 전체 링크 수
				result.TotalLinkTime += route.TotalLinkTime;; // 경로 전체 소요 시간 (초)
			} // for

			m_vtRouteResult.emplace_back(result);
		}
	}

	LOG_TRACE(LOG_DEBUG, timeStart, "DoCourse, result: %d", ret);

	return ret;
}


int CRouteManager::DoWeightMatrix(IN OUT RouteDistMatrix& RDM, OUT RequestRouteInfo& reqInfo)
{
	int ret = -1;

	// 좌표 설정
	KeyID sID, eID;//, wID;

	const vector<stWaypoint>* pvtOrigin = &RDM.vtOrigin;
	if (pvtOrigin == nullptr) {
		return ret;
	}

	const vector<stWaypoint>* pvtDestination = &RDM.vtDestination;

	int cntOrigin = pvtOrigin->size();
	int cntDestination = pvtDestination->size();

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "DoWeightMatrix, requst origins: %d, destinations: %d", cntOrigin, cntDestination);

	// 목적지 정보가 없으면 출발지 정보를 목적지 정보로 사용한다.
	if (pvtDestination->empty()) {
		pvtDestination = pvtOrigin;
		cntDestination = cntOrigin;
	}

#if defined(DEMO_FOR_IPO)
	int matchLinkType = TYPE_LINK_MATCH_CARSTOP;
#elif defined(DEMO_FOR_HANJIN)
	int matchLinkType = TYPE_LINK_MATCH_CARSTOP;
#else
	int matchLinkType = TYPE_LINK_MATCH_FOR_TABLE;
#endif

	Initialize();

//	// start link info
//	for (int ii = 0; ii < cntOrigin; ii++) {
//		SPoint coord = { pvtOrigins->at(ii).position.x, pvtOrigins->at(ii).position.y };
//		if (ii == 0) { // start
//			// get optimal start point
//#if defined(USE_OPTIMAL_POINT_API)
//			if (m_pDataMgr->GetOptimalPointDataByPoint(coord.x, coord.y, &optStartInfo, 0, 0, TYPE_LINK_MATCH_FOR_TABLE) > 0) {
//				sID = SetDeparture(optStartInfo.vtEntryPoint[0].x, optStartInfo.vtEntryPoint[0].y, matchLinkType);
//			} else
//#endif
//			{
//				sID = SetDeparture(coord.x, coord.y, matchLinkType);
//			}
//
//			if (sID.llid == NULL_VALUE) {
//				LOG_TRACE(LOG_DEBUG, "failed, set start projection, %.6f, %.6f", coord.x, coord.y);
//			}
//
//			// get optimal via point
//			// alreay applyed when set way point 
//		} else if (ii == cntOrigin - 1) { // end
//			// get optimal end point
//#if defined(USE_OPTIMAL_POINT_API)
//			if (m_pDataMgr->GetOptimalPointDataByPoint(coord.x, coord.y, &optEndInfo, 0, 0, matchLinkType) > 0) {
//				eID = SetDestination(optEndInfo.vtEntryPoint[0].x, optEndInfo.vtEntryPoint[0].y, matchLinkType);
//			} else
//#endif
//			{
//				eID = SetDestination(coord.x, coord.y, matchLinkType);
//			}
//
//			if (eID.llid == NULL_VALUE) {
//				LOG_TRACE(LOG_DEBUG, "failed, set end projection, %.6f, %.6f", pvtOrigins->at(cntOrigin - 1).position.x, pvtOrigins->at(cntOrigin - 1).position.y);
//			}
//		} else { // via
//#if defined(USE_OPTIMAL_POINT_API)
//			if (m_pDataMgr->GetOptimalPointDataByPoint(coord.x, coord.y, &optEndInfo, 0, 0, matchLinkType) > 0) {
//				wID = SetWaypoint(optEndInfo.vtEntryPoint[0].x, optEndInfo.vtEntryPoint[0].y, matchLinkType);
//			} else
//#endif
//			{
//				wID = SetWaypoint(coord.x, coord.y, matchLinkType);
//			}
//
//			if (wID.llid == NULL_VALUE) {
//				LOG_TRACE(LOG_DEBUG, "failed, set waypoint projection, %.6f, %.6f", coord.x, coord.y);
//			}
//		}
//	} // for

	RouteLinkInfo routeLinkInfo;
	SPoint coord;

	// origin link info
	vector<RouteLinkInfo> vtOriginLinkInfos;
	vtOriginLinkInfos.reserve(cntOrigin);

	for (int ii = 0; ii < cntOrigin; ii++) {
		routeLinkInfo.init();

		coord.x = pvtOrigin->at(ii).position.x;
		coord.y = pvtOrigin->at(ii).position.y;

		// get optimal end point
#if defined(USE_OPTIMAL_POINT_API)
		stOptimalPointInfo optInfo;
		stReqOptimal reqOpt;
		if (m_pDataMgr->GetOptimalPointDataByPoint(&optInfo, coord.x, coord.y, reqOpt, matchLinkType) > 0) {
			sID = SetPosition(optInfo.vtEntryPoint[0].x, optInfo.vtEntryPoint[0].y, matchLinkType, routeLinkInfo);
		} else
#endif
		{
			sID = SetPosition(coord.x, coord.y, matchLinkType, routeLinkInfo);
		}

		if (sID.llid == NULL_VALUE) {
			LOG_TRACE(LOG_DEBUG, "failed, set start projection, %.6f, %.6f", coord.x, coord.y);
		}

		vtOriginLinkInfos.emplace_back(routeLinkInfo);
	} // for


	// destination link info
	vector<RouteLinkInfo> vtDestLinkInfos;
	vtDestLinkInfos.reserve(cntDestination);
	
	for (int ii = 0; ii < cntDestination; ii++) {		
		routeLinkInfo.init();

		coord.x = pvtDestination->at(ii).position.x;
		coord.y = pvtDestination->at(ii).position.y;

		// get optimal end point
#if defined(USE_OPTIMAL_POINT_API)
		stOptimalPointInfo optInfo;
		stReqOptimal reqOpt;
		if (m_pDataMgr->GetOptimalPointDataByPoint(&optInfo, coord.x, coord.y, reqOpt, matchLinkType) > 0) {
			eID = SetPosition(optInfo.vtEntryPoint[0].x, optInfo.vtEntryPoint[0].y, matchLinkType, routeLinkInfo);
		} else
#endif
		{
			eID = SetPosition(coord.x, coord.y, matchLinkType, routeLinkInfo);
		}

		if (eID.llid == NULL_VALUE) {
			LOG_TRACE(LOG_DEBUG, "failed, set end projection, %.6f, %.6f", coord.x, coord.y);
		}

		vtDestLinkInfos.emplace_back(routeLinkInfo);
	} // for


	if (reqInfo.RequestId.empty()){
		reqInfo.RequestId = "13578642";
	}
	if (reqInfo.RequestTime == 0) {
		reqInfo.RequestTime = time(NULL);
	}

#if defined(DEMO_FOR_IPO)
	vector<uint32_t>vtRouteOpt = { ROUTE_OPT_RECOMMENDED, ROUTE_OPT_COMFORTABLE, ROUTE_OPT_SHORTEST, ROUTE_OPT_MAINROAD };
#else
	vector<uint32_t>vtRouteOpt = { ROUTE_OPT_RECOMMENDED, ROUTE_OPT_COMFORTABLE, ROUTE_OPT_SHORTEST, ROUTE_OPT_MAINROAD };
#endif
	//vector<uint32_t>vtRouteOpt = { ROUTE_OPT_RECOMMENDED, ROUTE_OPT_COMFORTABLE, ROUTE_OPT_SHORTEST, ROUTE_OPT_MAINROAD };
	vector<uint32_t>vtAvoidOpt = { ROUTE_AVOID_NONE, ROUTE_AVOID_NONE, ROUTE_AVOID_NONE, ROUTE_AVOID_NONE };

	SetRouteOption(vtRouteOpt, vtAvoidOpt, reqInfo.RequestTime, reqInfo.RequestTraffic, reqInfo.MobilityOption);
	SetRouteSubOption(reqInfo.RouteSubOption.option);
	SetRouteFreeOption(reqInfo.FreeOption);
	SetRouteTruckOption(reqInfo.RouteTruckOption.option());

	//reqInfo.RequestTime = m_nTimestampOpt;
	//reqInfo.RequestTraffic = m_nTrafficOpt;
	//reqInfo.MobilityOption = m_nMobilityOpt;
	reqInfo.RouteOption = m_vtRouteOpt[0];
	reqInfo.AvoidOption = m_vtAvoidOpt[0];
	//reqInfo.RouteSubOption = m_routeSubOpt;

	reqInfo.StartDirIgnore = m_nDepartureDirIgnore;
	reqInfo.WayDirIgnore = m_nWaypointDirIgnore;
	reqInfo.EndDirIgnore = m_nDestinationDirIgnore;

	//// start 
	//reqInfo.vtPoints.emplace_back(m_linkDeparture.Coord);
	//reqInfo.vtKeyId.emplace_back(m_linkDeparture.LinkId);
	//reqInfo.vtKeyType.emplace_back(m_linkDeparture.KeyType);
	//reqInfo.vtLinkDataType.emplace_back(m_linkDeparture.LinkDataType);

	//// waypoint
	//if (!m_linkWaypoints.empty()) {
	//	for (int ii = 0; ii < m_linkWaypoints.size(); ii++) {
	//		reqInfo.vtPoints.emplace_back(m_linkWaypoints[ii].Coord);
	//		reqInfo.vtKeyId.emplace_back(m_linkWaypoints[ii].LinkId);
	//		reqInfo.vtKeyType.emplace_back(m_linkWaypoints[ii].KeyType);
	//		reqInfo.vtLinkDataType.emplace_back(m_linkWaypoints[ii].LinkDataType);
	//	}
	//}

	//// end
	//reqInfo.vtPoints.emplace_back(m_linkDestination.Coord);
	//reqInfo.vtKeyId.emplace_back(m_linkDestination.LinkId);
	//reqInfo.vtKeyType.emplace_back(m_linkDestination.KeyType);
	//reqInfo.vtLinkDataType.emplace_back(m_linkDestination.LinkDataType);

	//check cost
	if (!reqInfo.RouteCost.empty()) {
		DataCost routeCost;
		int cnt = m_pDataMgr->GetDataCost(reqInfo.RouteCost.c_str(), routeCost);
		if (cnt > 0) {
			SetRouteCost(&routeCost, cnt);
		}
	}

	ret = m_pRoutePlan->DoWeightMatrix(&reqInfo, vtOriginLinkInfos, vtDestLinkInfos, RDM);
	if (ret != ROUTE_RESULT_SUCCESS) {
		LOG_TRACE(LOG_WARNING, "failed, get weight matrix");
	}

	// release
	Release();

	LOG_TRACE(LOG_DEBUG, timeStart, "DoWeightMatrix, result: %d", ret);

	return ret;
}


int CRouteManager::DoDistanceMatrix(IN OUT RouteDistMatrix& RDM)
{
	int ret = ROUTE_RESULT_SUCCESS;

	const vector<stWaypoint>* pvtOrigins = &RDM.vtOrigin;
	if (pvtOrigins == nullptr) {
		return ret;
	}

	const vector<stWaypoint>* pvtDestination = &RDM.vtDestination;

	int cntOrigin = pvtOrigins->size();
	int cntDestination = pvtDestination->size();

	time_t timeStart = LOG_TRACE(LOG_DEBUG, "DoDistanceMatrix, requst origins: %d, destinations: %d", cntOrigin, cntDestination);

	// 목적지 정보가 없으면 출발지 정보를 목적지 정보로 사용한다.
	if (pvtDestination->empty()) {
		pvtDestination = pvtOrigins;
		cntDestination = cntOrigin;
	}

	int matchLinkType = TYPE_LINK_MATCH_FOR_TABLE;
	KeyID matchID;

	for (int ii = 0; ii < cntOrigin; ii++) {
		SPoint coord = { pvtOrigins->at(ii).position.x, pvtOrigins->at(ii).position.y };

		// get optimal start point
		RouteLinkInfo pointLinkInfo;
#if defined(USE_OPTIMAL_POINT_API)
		stOptimalPointInfo optEndInfo;
		stReqOptimal reqOpt;
		if (m_pDataMgr->GetOptimalPointDataByPoint(&optEndInfo, coord.x, coord.y, reqOpt, matchLinkType) > 0) {
			matchID = SetPosition(optEndInfo.vtEntryPoint[0].x, optEndInfo.vtEntryPoint[0].y, matchLinkType, pointLinkInfo);
		} else
#endif
		{
			matchID = SetPosition(coord.x, coord.y, matchLinkType, pointLinkInfo);
			coord.x = pointLinkInfo.MatchCoord.x;
			coord.y = pointLinkInfo.MatchCoord.y;
		}

		if (matchID.llid == NULL_VALUE) {
			LOG_TRACE(LOG_DEBUG, "failed, set DoDistanceMatrix projection, %.6f, %.6f", coord.x, coord.y);
		}

		vector<stDistMatrix> vtRowsMatrix;

		for (int jj = 0; jj < cntDestination; jj++) {
			stDistMatrix distMatrix;
			if (ii != jj) {
				distMatrix.nTotalDist = getRealWorldDistance(coord.x, coord.y, pvtDestination->at(jj).position.x, pvtDestination->at(jj).position.y);
				distMatrix.dbTotalCost = distMatrix.nTotalTime = distMatrix.nTotalDist; // 다른 속성은 거리 정보를 사용
			}
			vtRowsMatrix.emplace_back(distMatrix);
		}

		RDM.vtDistMatrix.emplace_back(vtRowsMatrix);
	} // for

	LOG_TRACE(LOG_DEBUG, timeStart, "DoDistanceMatrix, result: %d", ret);

	return ret;
}


KeyID CRouteManager::SetPosition(IN const double lng, IN const double lat, IN const int matchType, OUT RouteLinkInfo& pointLinkInfo)
{
	if (m_pDataMgr != nullptr) {
		double retDist = INT_MAX;
		pointLinkInfo.Coord = { lng, lat };

		int newMatchType = matchType;
		int newLinkType = TYPE_LINK_DATA_NONE;

		stLinkInfo* pLink = nullptr;

#if defined(USE_FOREST_DATA)
		if (m_routeSubOpt.mnt.course_id == 0) {
			if ((m_routeSubOpt.mnt.course_type == TYPE_TRE_TRAIL) || (m_routeSubOpt.mnt.course_type == TYPE_TRE_BIKE)) { // 걷기, 자전거길
				newLinkType = TYPE_LINK_DATA_PEDESTRIAN; // 걷기/자전거 운동은 보행자 경로 사용하자
			} else {
				newMatchType = getCourseLinkMatchType(m_routeSubOpt);
			}
		} else {
			newMatchType = getCourseLinkMatchType(m_routeSubOpt);
		}
#elif defined(USE_PEDESTRIAN_DATA)
		newLinkType = TYPE_LINK_DATA_PEDESTRIAN;
#endif

		for (int ii = 0; ii< MAX_SEARCH_RANGE; ii++) {
			int curMaxDist = 10;
#if defined(USE_FOREST_DATA)
			if (m_routeSubOpt.mnt.course_id != 0) { // 코스 탐색
				pLink = m_pDataMgr->GetNearLinkDataByCourseId(m_routeSubOpt.mnt.course_id, lng, lat, courseRange[ii], pointLinkInfo.MatchCoord.x, pointLinkInfo.MatchCoord.y, retDist);
			} else if (newLinkType != TYPE_LINK_DATA_PEDESTRIAN) { // 숲길을 보행자길 보다 넓게 매칭(숲길 우선 매칭)
				if (m_routeSubOpt.mnt.course_type == TYPE_TRE_HIKING) {
					curMaxDist = hikingRange[ii];
				} else {
					curMaxDist = trailRange[ii];
				}
				pLink = m_pDataMgr->GetLinkDataByPointAround(lng, lat, curMaxDist, pointLinkInfo.MatchCoord.x, pointLinkInfo.MatchCoord.y, retDist, newMatchType, TYPE_LINK_DATA_TREKKING);
			}
#endif

#if defined(USE_FOREST_DATA) || defined(USE_PEDESTRIAN_DATA)
#	if defined(USE_FOREST_DATA)
			// 숲길보다 보행자 도로를 좁게 매칭하자(숲길 우선 매칭), 자전거는 보행자용 자전거 옵션 사용
			if ((pLink == nullptr) && ((m_routeSubOpt.mnt.course_type == TYPE_TRE_HIKING) || (newLinkType == TYPE_LINK_DATA_PEDESTRIAN))) {
#	else
			if ((pLink == nullptr) && (newLinkType == TYPE_LINK_DATA_PEDESTRIAN)) {
#	endif
				pLink = m_pDataMgr->GetLinkDataByPointAround(lng, lat, searchRange[ii], pointLinkInfo.MatchCoord.x, pointLinkInfo.MatchCoord.y, retDist, newMatchType, TYPE_LINK_DATA_PEDESTRIAN);
			}
#endif

#if defined(USE_VEHICLE_DATA)
			if (pLink == nullptr)
			{
				pLink = m_pDataMgr->GetLinkDataByPointAround(lng, lat, searchRange[ii], pointLinkInfo.MatchCoord.x, pointLinkInfo.MatchCoord.y, retDist, newMatchType, TYPE_LINK_DATA_VEHICLE);
			}
#endif	

			if (pLink) {
				pointLinkInfo.LinkId = pLink->link_id;
				pointLinkInfo.LinkDataType = pLink->base.link_type;
				pointLinkInfo.KeyType = TYPE_KEY_LINK;

				if (pLink->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) {
					pointLinkInfo.Payed = pLink->ped.walk_charge;
				} else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
					pointLinkInfo.Payed = pLink->veh.charge;
				}

				if (ii > 0) {
					LOG_TRACE(LOG_DEBUG, "position projection, link id:%lld, tile:%d, id:%d, range_lv:%d, dist:%.2f", pointLinkInfo.LinkId.llid, pointLinkInfo.LinkId.tile_id, pointLinkInfo.LinkId.nid, ii, retDist);
				}
				break;
			}
		} // for
	}

	return pointLinkInfo.LinkId;
}


#if 0
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
				pLink = m_pDataMgr->GetLinkDataById(pMesh[ii]->links[jj], TYPE_LINK_DATA_NONE);

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
#endif // #if 0

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