#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "RoutePlan.h"
#include "MMPoint.hpp"
#include "../utils/UserLog.h"
#include "../utils/GeoTools.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//auto CompareCanidate = [](const CandidateLink* lhs, const CandidateLink* rhs) {
//	if (lhs->costHeuristic > rhs->costHeuristic) {
//		return true;
//	}
//	else if (lhs->costHeuristic == rhs->costHeuristic) {
//		return lhs->depth > rhs->depth;
//	}
//	else {
//		return false;
//	}
//};
//priority_queue<CandidateLink*, vector<CandidateLink*>, decltype(CompareCanidate)> pqDijkstra{ CompareCanidate };



CRoutePlan::CRoutePlan()
{
#if defined(USE_SHOW_ROUTE_SATATUS)	
	m_pHost = nullptr;
	m_fpRoutingStatus = nullptr;
#endif
}


CRoutePlan::~CRoutePlan()
{
	Release();
}

void CRoutePlan::Initialize(void)
{
	//Release();

	//m_routeInfo.Init();
	//m_routeResult.Init();
}

void CRoutePlan::Release(void)
{
	//for (; !pqDijkstra.empty(); pqDijkstra.pop());
	//priority_queue<CandidateLink*>().swap(pqDijkstra);

	//if (!m_vtCandidateResult.empty()) {
	//	m_vtCandidateResult.clear();
	//	vector<CandidateLink*>().swap(m_vtCandidateResult);
	//}

	//if (!mRoutePass.empty()) {
	//	for (unordered_map<uint64_t, CandidateLink*>::const_iterator it = mRoutePass.begin(); it != mRoutePass.end(); it++) {
	//		if (it->second) {
	//			delete it->second;
	//		}
	//	}
	//	mRoutePass.clear();
	//	unordered_map<uint64_t, CandidateLink*>().swap(mRoutePass);
	//}
}

void CRoutePlan::SetDataMgr(IN CDataManager* pDataMgr)
{
	if (pDataMgr) {
		m_pDataMgr = pDataMgr;
	}
}

const bool CRoutePlan::SetVisitedLink(IN RouteInfo* pRouteInfo, IN const KeyID linkId)
{
	bool isSet = false;
	unordered_map<uint64_t, CandidateLink*>::const_iterator it = pRouteInfo->mRoutePass.find(linkId.llid);
	if (it != pRouteInfo->mRoutePass.end())
	{
		it->second->visited = true;
		isSet = true;
	}

	return isSet;
}

const bool CRoutePlan::IsVisitedLink(IN RouteInfo* pRouteInfo, IN const KeyID linkId)
{
	bool visited = false;
	//map<KeyID, CandidateLink>::iterator it = mRoutePass.find(linkId);
	//if (it != mRoutePass.end() && it->second.parentId == parentId)
	if (pRouteInfo->mRoutePass.find(linkId.llid) != pRouteInfo->mRoutePass.end())
	{
		visited = true;
	}

	return visited;
}

const bool CRoutePlan::IsAddedLink(IN RouteInfo* pRouteInfo, IN KeyID linkId)
{
	bool added = false;
	if (pRouteInfo->mRoutePass.find(linkId.llid) != pRouteInfo->mRoutePass.end())
	{
		added = true;
	}

	return added;
}


double GetCost(IN const stLinkInfo* pLink, IN const uint32_t opt, IN const double length, IN const uint32_t spd) // type, 0:보행자, 1:자전거 
{
	// 거리를 시간으로 환산	// 기본 이동 속도 = 5Km/h = 5000m/60M = 500m/6M = 500m/360s = 1.388..m/s => 1.3888/s;
	static const int32_t avgTreSpd = 3000; // 산행 이동 속도 = 3Km/h
	static const int32_t avgPedSpd = 4000; // 도보 이동 속도 = 4Km/h
	static const int32_t avgBycSpd = 20000; // 자전거 이동 속도 = 20Km/h
	static const int32_t avgCarSpd = 50000; // 차량 이동 속도 = 50Km/h

	if (pLink == nullptr)
		return VAL_MAX_COST;

	double secCost = 0;

	double delayValue = 0;
	double avgSpd = spd;
	double dist = length;

	if (spd <= 0) {
		if (pLink->base.link_type == TYPE_DATA_TREKKING)
		{
			avgSpd = avgTreSpd;
		}
		else if (pLink->base.link_type == TYPE_DATA_PEDESTRIAN)
		{
			if (opt == ROUTE_OPT_BIKE) {
				if (pLink->ped.bycicle_type == TYPE_BYC_WITH_WALK) {
					avgSpd = avgPedSpd * .8f; // 보행자 전용도로면 자전거를 끌고 가므로 보행 속도 보다 느리게 책정
				}
				else if (pLink->ped.bycicle_type == TYPE_BYC_WITH_CAR) {
					avgSpd = avgBycSpd * .8f; // 보행자/차량 도로면 자전거를 천천히 타고 가는 속도 속도로 책정
				}
				else {
					avgSpd = avgBycSpd;
				}
			}
			//else if (pLink->ped.walk_type == TYPE_WALK_WITH_BYC) {
			//	avgSpd = avgPedSpd * .8f; // 통행 불가 확인, 자전거 전용도로면 조심해서 걷기에 보행 속도 절반으로 책정
			//}
			else {
				avgSpd = avgPedSpd;
			}
		}
		else if (pLink->base.link_type == TYPE_DATA_VEHICLE)
		{
			if (opt != ROUTE_OPT_SHORTEST) {
				// 경로레벨, 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상, 4:일반도로6차선이상, 
				// 5:일반도로4차선이상, 6:일반도로2차선이상, 7:일반도로1차선이상, 8:SS도로, 9:GSS도로/단지내도로/통행금지도로/비포장도로
				switch (pLink->veh.level) {
				case 0:
					avgSpd = 90000; break;
				case 1:
					avgSpd = 70000; break;
				case 2:
					avgSpd = 60000; break;
				case 3:
					avgSpd = 50000; break;
				case 4:
					avgSpd = 50000; break;
				case 5:
					avgSpd = 40000; break;
				case 6:
					avgSpd = 30000; break;

				default:
					avgSpd = 20000; break;
					break;
				} // switch
			}
			else {
				avgSpd = avgCarSpd;
			}
		}
		else {
			avgSpd = avgPedSpd;
		}
	}

	double spdValue = avgSpd / 3600.f;


	if (dist <= 0) {
		dist = pLink->length;
	}

	if (dist > 0)
	{
		secCost = round(dist / spdValue);

		if (pLink->base.link_type == TYPE_DATA_TREKKING)
		{
			int nRoadType = pLink->tre.road_info;
			// 노면정보 코드, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드

			if (nRoadType > 0 && opt == ROUTE_OPT_COMFORTABLE)
			{
				if ((nRoadType & ROUTE_AVOID_DECK) == ROUTE_AVOID_DECK) { // 11:데크로드 = 1000 0000 0000 = 1024
					secCost *= 0.6f;
				}
				if ((nRoadType & ROUTE_AVOID_PALM) == ROUTE_AVOID_PALM) { // 10:야자수매트 = 100 0000 0000 = 512
					secCost *= 0.8f;
				}
				//if ((nRoadType & ROUTE_AVOID_TATTERED) == ROUTE_AVOID_TATTERED) { // 9:너덜길 = 1 0000 0000 = 256

				//}
				//if ((nRoadType & ROUTE_AVOID_ROPE) == ROUTE_AVOID_ROPE) { // 8:밧줄 = 1000 0000 = 128

				//}
				//if ((nRoadType & ROUTE_AVOID_LADDER) == ROUTE_AVOID_LADDER) { // 7;사다리 = 100 0000 = 64

				//}
				//if ((nRoadType & ROUTE_AVOID_RIDGE) == ROUTE_AVOID_RIDGE) { // 6:릿지 = 10 0000 = 32

				//}
				//if ((nRoadType & ROUTE_AVOID_ROCK) == ROUTE_AVOID_ROCK) { // 5:암릉 = 1 0000 = 16

				//}
				if ((nRoadType & ROUTE_AVOID_BRIDGE) == ROUTE_AVOID_BRIDGE) { // 4:교량 = 1000 = 8
					secCost *= 0.6f;
				}
				if ((nRoadType & ROUTE_AVOID_STAIRS) == ROUTE_AVOID_STAIRS) { // 3:계단 = 100 = 4			
					secCost *= 0.7f;
				}
				if ((nRoadType & ROUTE_AVOID_PAVE) == ROUTE_AVOID_PAVE) { // 2:포장길 = 10 = 2		
					secCost *= 0.5f;
				}
				//if ((nRoadType & ROUTE_AVOID_ALLEY) == ROUTE_AVOID_ALLEY) { // 1:오솔길 = 1
				//}
			}
		}
		else if (pLink->base.link_type == TYPE_DATA_PEDESTRIAN)
		{
			if (opt == ROUTE_OPT_BIKE)
			{
				// 자전거도로 타입, 1:자전거전용, 2:보행자/차량겸용 자전거도로, 3:보행도로
				if (pLink->ped.bycicle_type == TYPE_BYC_ONLY) {
					secCost *= 0.8;
				}
				else if (pLink->ped.bycicle_type == TYPE_BYC_WITH_CAR) {
					secCost *= 0.9;
				}
				else if (pLink->ped.bycicle_type == TYPE_BYC_WITH_WALK) {
					;
				}
				else {
					;
				}
			}
			else if (opt == ROUTE_OPT_COMFORTABLE)
			{
				//보행자도로 타입, 1:복선도록, 2:차량겸용도로, 3:자전거전용도로, 4:보행전용도로, 5:가상보행도로
				if (pLink->ped.walk_type == TYPE_WALK_SIDE) {
					//secCost *= 0.7;
				}
				else if (pLink->ped.walk_type == TYPE_WALK_WITH_CAR) {
					//secCost *= 0.9;
				}
				else if (pLink->ped.walk_type == TYPE_WALK_WITH_BYC) {
					;
				}
				else if (pLink->ped.walk_type == TYPE_WALK_ONLY) {
					;
				}
				else if (pLink->ped.walk_type == TYPE_WALK_THROUGH) {
					;
				}
				else {
					;
				}
			}


			switch (pLink->ped.walk_type) {
			case 1: // 1 : 경사로
				break;
			case 2: // 2 : 계단
				break;
			case 3: // 3 : 에스컬레이터
				break;
			case 4: // 4 : 계단 / 에스컬레이터
				break;
			case 5: // 5 : 엘리베이터
				break;
			case 6: // 6 : 단순연결로
				break;
			case 7: // 7 : 횡단보도
				if (opt == ROUTE_OPT_COMFORTABLE) {
					// 횡단 보도를 너무 자주 건너지 않도록 가중치 적용
					//secCost *= 1.25f;
				}
				break;
			case 8: // 8 : 무빙워크
				break;
			case 9: // 9 : 징검다리
				break;
			case 10: // 10 : 의사횡단
				break;
			default: // 0:미정의
				break;
			}
		}
		else if (pLink->base.link_type == TYPE_DATA_VEHICLE) {
			if (opt == ROUTE_OPT_RECOMMENDED || opt == ROUTE_OPT_COMFORTABLE || opt == ROUTE_OPT_MAINROAD || opt == ROUTE_OPT_AUTOMATION) {
				if (pLink->veh.car_only == 1) { // 자동차 전용도로
					secCost *= 0.7;
				} else if (pLink->veh.link_type == 2 || pLink->veh.lane_cnt >= 6) { // 분리도로는 좀 더 빠르게
					secCost *= 0.9;
				}
			}
		}
	}
	
	return secCost;
}

// 거리를 시간으로 환산
double GetTravelCost(IN const stLinkInfo* pLink, IN const stLinkInfo* pLinkPrev, IN const double cost, IN const int angle, IN const uint32_t type, IN const uint32_t opt, IN const uint32_t avoid) // type, 0:보행자, 1:자전거 
{
	double secCost = cost;
	double angCost = 0;
	double waitCost = 0;

	if (pLink != nullptr && pLink->length > 0)
	{
		if (pLink->base.link_type == TYPE_DATA_TREKKING)
		{
			// 회피 구간의 진행 시간을 크게 (하루) 늘림
			int nRoadType = pLink->tre.road_info;
			// 노면정보 코드, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드

			if (nRoadType > 0 && avoid > 0)
			{
				if ((nRoadType & ROUTE_AVOID_DECK) == ROUTE_AVOID_DECK) { // 11:데크로드 = 1000 0000 0000 = 1024
				}
				if ((nRoadType & ROUTE_AVOID_PALM) == ROUTE_AVOID_PALM) { // 10:야자수매트 = 100 0000 0000 = 512
				}
				if ((nRoadType & ROUTE_AVOID_TATTERED) == ROUTE_AVOID_TATTERED) { // 9:너덜길 = 1 0000 0000 = 256
				}
				if ((nRoadType & ROUTE_AVOID_ROPE) == ROUTE_AVOID_ROPE) { // 8:밧줄 = 1000 0000 = 128
					if ((avoid & ROUTE_AVOID_ROPE) == ROUTE_AVOID_ROPE) {
						waitCost = 3600;
					}
				}
				if ((nRoadType & ROUTE_AVOID_LADDER) == ROUTE_AVOID_LADDER) { // 7;사다리 = 100 0000 = 64
					if ((avoid & ROUTE_AVOID_LADDER) == ROUTE_AVOID_LADDER) {
						waitCost = 3600;
					}
				}
				if ((nRoadType & ROUTE_AVOID_RIDGE) == ROUTE_AVOID_RIDGE) { // 6:릿지 = 10 0000 = 32
					if ((avoid & ROUTE_AVOID_RIDGE) == ROUTE_AVOID_RIDGE) {
						waitCost = 3600;
					}
				}
				if ((nRoadType & ROUTE_AVOID_ROCK) == ROUTE_AVOID_ROCK) { // 5:암릉 = 1 0000 = 16
					if ((avoid & ROUTE_AVOID_ROCK) == ROUTE_AVOID_ROCK) {
						waitCost = 3600;
					}
				}
				if ((nRoadType & ROUTE_AVOID_BRIDGE) == ROUTE_AVOID_BRIDGE) { // 4:교량 = 1000 = 8
					if ((avoid & ROUTE_AVOID_BRIDGE) == ROUTE_AVOID_BRIDGE) {
						waitCost = 3600;
					}
				}
				if ((nRoadType & ROUTE_AVOID_STAIRS) == ROUTE_AVOID_STAIRS) { // 3:계단 = 100 = 4				
				}
				if ((nRoadType & ROUTE_AVOID_PAVE) == ROUTE_AVOID_PAVE) { // 2:포장길 = 10 = 2				
				}
				if ((nRoadType & ROUTE_AVOID_ALLEY) == ROUTE_AVOID_ALLEY) { // 1:오솔길 = 1
				}
			}			
		}
		else if (pLink->base.link_type == TYPE_DATA_PEDESTRIAN)
		{
			if (opt == ROUTE_OPT_BIKE)
			{
				// 자전거도로 타입, 1:자전거전용, 2:보행자/차량겸용 자전거도로, 3:보행도로
				if (pLink->ped.bycicle_type == TYPE_BYC_ONLY) {
					secCost *= 0.7;
				}
				else if (pLink->ped.bycicle_type == TYPE_BYC_WITH_CAR) {
					secCost *= 0.9;
				}
				else if (pLink->ped.bycicle_type == TYPE_BYC_WITH_WALK) {
					;
				}
				else {
					;
				}
			}
			else if (opt == ROUTE_OPT_COMFORTABLE)
			{
				//보행자도로 타입, 1:복선도록, 2:차량겸용도로, 3:자전거전용도로, 4:보행전용도로, 5:가상보행도로
				if (pLink->ped.walk_type == TYPE_WALK_SIDE) {
					if ((pLink->ped.gate_type == 6) || (pLink->ped.gate_type == 0)) {
						secCost *= 0.8;
					}
				}
				else if (pLink->ped.walk_type == TYPE_WALK_WITH_CAR) {
					secCost *= 0.9;
				}
				else if (pLink->ped.walk_type == TYPE_WALK_WITH_BYC) {
					;
				}
				else if (pLink->ped.walk_type == TYPE_WALK_ONLY) {
					if ((pLink->ped.gate_type == 6) || (pLink->ped.gate_type == 0)) {
						secCost *= 0.8;
					}
				}
				else if (pLink->ped.walk_type == TYPE_WALK_THROUGH) {
					;
				}
				else {
					;
				}


				// 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
				if (pLink->ped.facility_type == 1) {
					secCost *= 1.1f;
					//strcat(szInfo, "토끼굴");
				}
				else if (pLink->ped.facility_type == 2) {
					secCost *= 1.1f;
					//strcat(szInfo, "지하보도");
				}
				else if (pLink->ped.facility_type == 3) {
					secCost *= 1.1f;
					//strcat(szInfo, "육교");
				}
				else if (pLink->ped.facility_type == 4) {
					secCost *= 1.1f;
					//strcat(szInfo, "고가도로");
				}
				else if (pLink->ped.facility_type == 5) {
					secCost *= 1.1f;
					//strcat(szInfo, "교량");
				}
				else if (pLink->ped.facility_type == 6) {
					secCost *= 1.5f;
					//strcat(szInfo, "지하철역");
				}
				else if (pLink->ped.facility_type == 7) {
					secCost *= 1.2f;
					//strcat(szInfo, "철도");
				}
				else if (pLink->ped.facility_type == 8) {
					secCost *= 1.2f;
					//strcat(szInfo, "중앙버스정류장");
				}
				else if (pLink->ped.facility_type == 9) {
					secCost *= 1.2f;
					//strcat(szInfo, "지하상가");
				}
				else if (pLink->ped.facility_type == 10) {
					secCost *= 1.2f;
					//strcat(szInfo, "건물관통도로");
				}
				else if (pLink->ped.facility_type == 11) {
					secCost *= 1.2f;
					//strcat(szInfo, "단지도로_공원");
				}
				else if (pLink->ped.facility_type == 12) {
					secCost *= 1.2f;
					//strcat(szInfo, "단지도로_주거시설");
				}
				else if (pLink->ped.facility_type == 13) {
					secCost *= 1.2f;
					//strcat(szInfo, "단지도로_관광지");
				}
				else if (pLink->ped.facility_type == 14) {
					secCost *= 1.2f;
					//strcat(szInfo, "단지도로_기타");
				}
				else {
					//strcat(szInfo, "미정의");
				}
			}
			else if (opt == ROUTE_OPT_MAINROAD) {
				if (pLink->ped.gate_type != 7 && pLink->ped.lane_count >= 1) {
					secCost += secCost / (double)(pLink->ped.lane_count * 5);
				}

				//보행자도로 타입, 1:복선도록, 2:차량겸용도로, 3:자전거전용도로, 4:보행전용도로, 5:가상보행도로
				if (pLink->ped.walk_type == TYPE_WALK_SIDE) {
					if ((pLink->ped.gate_type == 6) || (pLink->ped.gate_type == 0)) {
						secCost *= 0.7;
					}
				}
				else if (pLink->ped.walk_type == TYPE_WALK_WITH_CAR) {
					secCost *= 0.9;
				}
				else if (pLink->ped.walk_type == TYPE_WALK_WITH_BYC) {
					;
				}
				else if (pLink->ped.walk_type == TYPE_WALK_ONLY) {
					if ((pLink->ped.gate_type == 6) || (pLink->ped.gate_type == 0)) {
						secCost *= 0.7;
					}
				}
				else if (pLink->ped.walk_type == TYPE_WALK_THROUGH) {
					;
				}
				else {
					;
				}


				// 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
				if (pLink->ped.facility_type == 1) {
					secCost *= 1.1f;
					//strcat(szInfo, "토끼굴");
				}
				else if (pLink->ped.facility_type == 2) {
					secCost *= 1.1f;
					//strcat(szInfo, "지하보도");
				}
				else if (pLink->ped.facility_type == 3) {
					secCost *= 1.1f;
					//strcat(szInfo, "육교");
				}
				else if (pLink->ped.facility_type == 4) {
					secCost *= 1.1f;
					//strcat(szInfo, "고가도로");
				}
				else if (pLink->ped.facility_type == 5) {
					secCost *= 1.1f;
					//strcat(szInfo, "교량");
				}
				else if (pLink->ped.facility_type == 6) {
					secCost *= 1.5f;
					//strcat(szInfo, "지하철역");
				}
				else if (pLink->ped.facility_type == 7) {
					secCost *= 1.2f;
					//strcat(szInfo, "철도");
				}
				else if (pLink->ped.facility_type == 8) {
					secCost *= 1.2f;
					//strcat(szInfo, "중앙버스정류장");
				}
				else if (pLink->ped.facility_type == 9) {
					secCost *= 1.2f;
					//strcat(szInfo, "지하상가");
				}
				else if (pLink->ped.facility_type == 10) {
					secCost *= 1.2f;
					//strcat(szInfo, "건물관통도로");
				}
				else if (pLink->ped.facility_type == 11) {
					secCost *= 1.2f;
					//strcat(szInfo, "단지도로_공원");
				}
				else if (pLink->ped.facility_type == 12) {
					secCost *= 1.2f;
					//strcat(szInfo, "단지도로_주거시설");
				}
				else if (pLink->ped.facility_type == 13) {
					secCost *= 1.2f;
					//strcat(szInfo, "단지도로_관광지");
				}
				else if (pLink->ped.facility_type == 14) {
					secCost *= 1.2f;
					//strcat(szInfo, "단지도로_기타");
				}
				else {
					//strcat(szInfo, "미정의");
				}
			}

			switch (pLink->ped.gate_type) {
			case 1: // 1 : 경사로
				secCost *= 2.f; // 경사로는 2배정도 힘들자나, 경사도가 있으면 경사도에 따라 차등 적용
				break;
			case 2: // 2 : 계단
				secCost *= 2.f; // 계단길도 2배정도 힘들자나
				if (type == ROUTE_OPT_BIKE) {
					secCost = pLink->length * 10.f; // 자전거는 계단길을 가지 않도록 하자 // x 10
				}
				break;
			case 3: // 3 : 에스컬레이터
				//secCost *= .6f; // 에스컬레이터는 편하니까 줄여주자
				if (type == ROUTE_OPT_BIKE) {
					secCost = pLink->length * 10.f; // 자전거는 에스컬레이터를 가지 않도록 하자
				}
				break;
			case 4: // 4 : 계단 / 에스컬레이터
				//secCost *= .8f; // 에스컬레이터는 편하니까 줄여주자.. 어떤 타입인지 잘 모르겠군...
				if (type == ROUTE_OPT_BIKE) {
					secCost = pLink->length * 10.f; // 자전거는 에스컬레이터를 가지 않도록 하자
				}
				break;
			case 5: // 5 : 엘리베이터
				waitCost = 30; // 엘리베이터 대기 시간을 30s 정도 주자
				//secCost *= .1f; // // 에스컬레이터는 편하니까 확 줄여주자
				if (type == ROUTE_OPT_BIKE) {
					secCost = pLink->length * 10.f;
				}
				break;
			case 6: // 6 : 단순연결로
				break;
			case 7: // 7 : 횡단보도
				if (opt == ROUTE_OPT_COMFORTABLE) {
					if (pLinkPrev->ped.lane_count > 1) {
						// 횡단 보도를 너무 자주 건너지 않도록 가중치 적용
						//secCost *= 1.25f;
						waitCost = pLinkPrev->ped.lane_count * 0.01;
					}
				}
				else if (opt == ROUTE_OPT_MAINROAD) {
					if (pLinkPrev->ped.lane_count > 1) {
						// 횡단 보도를 너무 자주 건너지 않도록 가중치 적용
						//secCost *= 1.25f;
						//waitCost = 1;
					}
				}
				else {
					waitCost = 60; // 횡단보도는 1min정도 기다리는 시간이 주어진다.
				}
				if (type == ROUTE_OPT_BIKE) {
					waitCost = 300; //5min
				}
				break;
			case 8: // 8 : 무빙워크
				secCost = pLink->length * .3f; // 편하니까 줄여주자..
				if (type == ROUTE_OPT_BIKE) {
					secCost = pLink->length * 10.f; // 자전거는 무빙워크 가지 않도록 하자
				}
				break;
			case 9: // 9 : 징검다리
				secCost = pLink->length * 5.f; // 징검다리는 조심하니까 5배 정도 느리게 진행
				if (type == ROUTE_OPT_BIKE) {
					secCost = pLink->length * 20.f; // 자전거는 징검다리를 건너지 않도록 하자
				}
				break;
			case 10: // 10 : 의사횡단
				waitCost = 30; // 의사 횡단이 머지...???
				break;

			default: { // 0:미정의
				if ((opt == ROUTE_OPT_COMFORTABLE)) {
					// 차선 가중치 - 차선이 많을수록 편한길
					if (pLink->ped.lane_count >= 1) {
						//secCost += secCost / (double)pLink->ped.lane_count;
						secCost += secCost / ((double)pLink->ped.lane_count * 0.5);
					}
				}
				else if ((opt == ROUTE_OPT_MAINROAD)) {
					// 차선 가중치 - 차선이 많을수록 편한길
					if (pLink->ped.lane_count >= 1) {
						secCost += secCost / ((double)pLink->ped.lane_count * 0.01);
					}
				}
			}
			break;

			} // switch



			


			if (pLink->ped.walk_type == TYPE_WALK_WITH_BYC) {
				// pLink->length * 3;
			}
			else if (pLink->ped.walk_type == TYPE_WALK_THROUGH) {
				// pLink->length * 1;
			}

			if ((opt == ROUTE_OPT_COMFORTABLE)) {
				if (/*(pLink->ped.gate_type == 0) && */(abs(angle) != 180)) {
					// 회전을 할수록 가중치 적용
					angCost = abs(180 - abs(angle)) / 180.f;
					secCost += angCost * 3;
				}
			}
			else if ((opt == ROUTE_OPT_MAINROAD)) {
				if ((pLink->ped.gate_type == 0) && (abs(angle) != 180)) {
					// 회전을 할수록 가중치 적용
					angCost = abs(180 - abs(angle)) / 180.f;
					secCost += angCost * 3;
				}
			}
		}
		else if (pLink->base.link_type == TYPE_DATA_VEHICLE)
		{
			if ((opt == ROUTE_OPT_FASTEST || opt == ROUTE_OPT_COMFORTABLE || opt == ROUTE_OPT_MAINROAD || opt == ROUTE_OPT_AUTOMATION)) {
				// 경로레벨, 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상, 4:일반도로6차선이상, 
				// 5:일반도로4차선이상, 6:일반도로2차선이상, 7:일반도로1차선이상, 8:SS도로, 9:GSS도로/단지내도로/통행금지도로/비포장도로
				if (pLink->length > 0)
				{
					int avgSpd;
					switch (pLink->veh.level) {
					case 0:
						avgSpd = 100; break;
					case 1:
						avgSpd = 80; break;
					case 2:
						avgSpd = 80; break;
					case 3:
						avgSpd = 70; break;
					case 4:
						avgSpd = 60; break;
					case 5:
						avgSpd = 50; break;
					case 6:
						avgSpd = 40; break;

					default:
						avgSpd = 30; break;
						break;
					}

					double spdValue = avgSpd / 3600.f;
					secCost += pLink->length / spdValue;
				}

				if (/*(pLink->ped.gate_type == 0) && */(abs(angle) != 180)) {
					// 회전을 할수록 가중치 적용
					angCost = abs(180 - abs(angle)) / 180.f;
					if (opt == ROUTE_OPT_AUTOMATION) {
						secCost += angCost * 100;
					}
					else {
						secCost += angCost * 3;
					}
				}
			} // if 
		}
	}

	return secCost + waitCost;
}


const int CRoutePlan::AddNewLinks(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const int dir)
{
	KeyID candidateId;
	int cntLinks = 0;

	candidateId.parents_id = pCurInfo->parentId.nid;
	candidateId.current_id = pCurInfo->linkId.nid;

	SetVisitedLink(pRouteInfo, candidateId);

	//if (dir == 0) // 양방향
	//{
	//	cntLinks += AddNewLinks(pRouteInfo, pCurInfo, 1); // 정으로
	//	cntLinks += AddNewLinks(pRouteInfo, pCurInfo, 2); // 역으로
	//	return cntLinks;
	//}

	int altDiff = 0; // 고도차이
	double altCost = 0.f; // 고도차 가중치
	int angDiff = 0;
	int dirTarget = 0;
	KeyID nextNodeId = { 0, };
	stNodeInfo* pNode = nullptr;
	stNodeInfo* pNodeNext = nullptr;
	stLinkInfo* pLink = nullptr;
	stLinkInfo* pLinkNext = nullptr;


	// 현재 링크 get
	candidateId = pCurInfo->linkId;
	//candidateId.dir = 0; // 입력시에 방향성 줬기에 원데이터에서는 방향성 빼고 검색

#if defined(USE_TREKKING_DATA)
	pNode = m_pDataMgr->GetWNodeDataById(pCurInfo->nodeId);
	pLink = m_pDataMgr->GetWLinkDataById(candidateId);
	if (pNode == nullptr && pLink == nullptr) {
		pNode = m_pDataMgr->GetNodeDataById(pCurInfo->nodeId);
		pLink = m_pDataMgr->GetLinkDataById(candidateId);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	pNode = m_pDataMgr->GetWNodeDataById(pCurInfo->nodeId);
	pLink = m_pDataMgr->GetWLinkDataById(candidateId);
#elif defined(USE_VEHICLE_DATA)
	pNode = m_pDataMgr->GetVNodeDataById(pCurInfo->nodeId);
	pLink = m_pDataMgr->GetVLinkDataById(candidateId);
#else
	pNode = m_pDataMgr->GetNodeDataById(pCurInfo->nodeId);
	pLink = m_pDataMgr->GetLinkDataById(candidateId);
#endif

	if (!pLink)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", candidateId.nid);
		return -1;
	}


	// 다음 노드 get
	if (dir == 3) { // 통행불가
		LOG_TRACE(LOG_DEBUG, "entrance disable link, link id:%d", pLink->link_id.nid);
		return -1;
	}
#if defined(USE_P2P_DATA)
	else if (pLink->veh.hd_flag == 0) { // HD 링크와 매칭 정보가 없으면 통행 불가
		return -1;
	}
	else if (dir == 1) { // e 정방향
		nextNodeId = pLink->enode_id;
		angDiff = pLink->veh_ext.enode_ang;
	}
	else if (dir == 2) { // s 역방향으로
		nextNodeId = pLink->snode_id;
		angDiff = pLink->veh_ext.snode_ang;
	}
#else
	else if (dir == 1) { // e 정방향
		nextNodeId = pLink->enode_id;
		angDiff = pLink->base.enode_ang;
	}
	else if (dir == 2) { // s 역방향으로
		nextNodeId = pLink->snode_id;
		angDiff = pLink->base.snode_ang;
	}
#endif
	else {
		// 다음 노드를 찾아야 함.
		if (pCurInfo->nodeId.nid == pLink->enode_id.nid) {
			nextNodeId = pLink->snode_id;
		}
		else if (pCurInfo->nodeId.nid == pLink->snode_id.nid) {
			nextNodeId = pLink->enode_id;
		}
		else {
			LOG_TRACE(LOG_ERROR, "Failed, can't find next node, link id:%d", pLink->link_id.nid);
			return -1;
		}
	}
	//nextNodeId.dir = 0;

#if defined(USE_TREKKING_DATA)
	if (pLink->base.link_type == TYPE_DATA_PEDESTRIAN) {
		pNodeNext = m_pDataMgr->GetWNodeDataById(nextNodeId);
	} else {
		pNodeNext = m_pDataMgr->GetNodeDataById(nextNodeId);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	pNodeNext = m_pDataMgr->GetWNodeDataById(nextNodeId);
#elif defined(USE_VEHICLE_DATA)
	pNodeNext = m_pDataMgr->GetVNodeDataById(nextNodeId);
#else
	pNodeNext = m_pDataMgr->GetNodeDataById(nextNodeId);
#endif

	if (!pNodeNext)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't find next node, next node id:%d", nextNodeId.nid);
		return -1;
	}


	// 아래 루틴 1, 2는 getNextPassCode(...) 함수로 대체가능, 중복 계산을 방지하기 위해 나눠 사용 중
	// 루틴 1, 링크의 통행 코드
	int16_t codeLinkPass = 0;
	int16_t retPassCode = 0;
	// 링크의 통행코드에서 링크 자신의 인덱스 위치
	int32_t idxLinkPassOffset = 0;

	if (pLink->base.link_type == TYPE_DATA_VEHICLE) {
		// 현재 링크의 통과 코드에서 요청된 idx의 통과 코드값 계산
		int idxLinkPass = -1;

		for (int ii = 0; ii < pNodeNext->base.connnode_count; ii++) {
			if (pNodeNext->connnodes[ii] == pLink->link_id) {
				idxLinkPass = ii;
				break;
			}
		}

		if (idxLinkPass < 0) {
			LOG_TRACE(LOG_WARNING, "can't find current link matched pass, link tile:%d, id:%d, cnt:%d, idx:%d", pLink->link_id.tile_id, pLink->link_id.nid);
			return -1;
		}

		// 통행코드는 자신의 현재 위치(링크) 다음 방향 값을 첫 인덱스를 시작해 자신을 가장 마지막 인덱스로 구성함.
		// 그러나 요청되는 다음 인덱스는 노드 연결 구성시 정북 기준 순으로 저장되므로 요청된 인덱스의 순서 변경이 필요함
		idxLinkPassOffset = (pNodeNext->base.connnode_count - 1) - idxLinkPass;

		codeLinkPass = pNodeNext->conn_attr[idxLinkPass];
	}
	// - // 루틴 1, 링크의 통행 코드



	double heuristicCost = 0.f;
	double toDestStraightDist = 0.f;
	double costReal = 0.f;
	double costTreavel = 0.f;
	KeyID nextCandidate = { 0, };
	CandidateLink* pItem = nullptr;

	// 노드에 연결된 다음 링크 정보
 	for (uint32_t ii = 0; ii < pNodeNext->base.connnode_count; ii++)
	{
		// 루틴 2, 통행 코드 확인
		if ((pLink->base.link_type == TYPE_DATA_VEHICLE) && (codeLinkPass != 0)) {

			int idxCurrentPass = (idxLinkPassOffset + ii);
			if (idxCurrentPass >= pNodeNext->base.connnode_count) {
				idxCurrentPass %= (pNodeNext->base.connnode_count);
			}
			retPassCode = getPassCode(codeLinkPass, idxCurrentPass);

			if (retPassCode < 0) {
				continue;
			}
			else if (retPassCode == 2) { // 통행불가
				continue;
			}
		}
		// - // 루틴 2, 통행 코드 확인

		//if (dir == 1) // 정방향
		//{

		//}
		//else if (dir == 2) // 역방향
		//{

		//}
		//else
		{
#if defined(USE_TREKKING_DATA)
			if (pLink->base.link_type == TYPE_DATA_PEDESTRIAN) {
				pLinkNext = m_pDataMgr->GetWLinkDataById(pNodeNext->connnodes[ii]);
			} else {
				pLinkNext = m_pDataMgr->GetLinkDataById(pNodeNext->connnodes[ii]);
			}
#elif defined(USE_PEDESTRIAN_DATA)
			pLinkNext = m_pDataMgr->GetWLinkDataById(pNodeNext->connnodes[ii]);
#elif defined(USE_VEHICLE_DATA)
			pLinkNext = m_pDataMgr->GetVLinkDataById(pNodeNext->connnodes[ii]);
#else
			pLinkNext = m_pDataMgr->GetLinkDataById(pNodeNext->connnodes[ii]);
#endif

			if (!pLinkNext) {
				LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", pNodeNext->connnodes[ii].llid);
				continue;
			}

			// 자기 자신은 제외
			if (pLink->link_id.nid == pLinkNext->link_id.nid && retPassCode != 1) {
				continue;
			}

#if defined(USE_P2P_DATA)
			if (pLinkNext->veh.hd_flag != 1) { // HD 링크와 매칭 정보가 없으면 통행 불가
				continue;
			}
#endif

			candidateId.parents_id = pLink->link_id.nid;
			candidateId.current_id = pLinkNext->link_id.nid;
			if (!IsVisitedLink(pRouteInfo, candidateId)) { // && !IsAddedLink(pLink->link_id)) {

				if (pNodeNext->coord.x == pLinkNext->getVertexX(0) && pNodeNext->coord.y == pLinkNext->getVertexY(0))
				{// 다음 링크의 snode 연결
					if (pLinkNext->link_id.dir == 2) { // 일방(역)에 시작점이 일치하면 불가
						continue;
					}

#if defined(USE_P2P_DATA)
					angDiff -= pLinkNext->veh_ext.snode_ang;
#else
					angDiff -= pLinkNext->base.snode_ang;
#endif
					dirTarget = 1; // 정
				}
				else
				{// 다음 링크의 enode 연결
					if (pLinkNext->link_id.dir == 1) { // 일방(정)에 종료점이 일치하면 불가
						continue;
					}

#if defined(USE_P2P_DATA)
					angDiff -= pLinkNext->veh_ext.enode_ang;
#else
					angDiff -= pLinkNext->base.enode_ang;
#endif
					dirTarget = 2; // 역
				}

				toDestStraightDist = getRealWorldDistance(pRouteInfo->EndLinkInfo.Coord.x, pRouteInfo->EndLinkInfo.Coord.y,
					pLinkNext->getVertexX(pLinkNext->getVertexCount() / 2), pLinkNext->getVertexY(pLinkNext->getVertexCount() / 2));

#if defined(USE_OPTIMAL_POINT_API)
#	if defined(_DEBUG)
				static const double fFactor = 0.10f;
				heuristicCost = toDestStraightDist * fFactor;
#	else
				heuristicCost = toDestStraightDist * VAL_HEURISTIC_VEHICLE_FACTOR;
#	endif
#else
				heuristicCost = toDestStraightDist * VAL_HEURISTIC_PEDESTRIAN_FACTOR;
#endif
				// 링크 타입 비교를 안하면 연산에 좀더 빠르려나
				//if (pLinkNext->base.link_type == TYPE_DATA_VEHICLE) {
				//	heuristicCost = static_cast<int>(toDestStraightDist * VAL_HEURISTIC_VEHICLE_FACTOR);
				//}
				//else // if (VAL_HEURISTIC_VEHICLE_FACTOR > 0)
				//{
				//	heuristicCost = static_cast<int>(toDestStraightDist * VAL_HEURISTIC_VEHICLE_FACTOR);
				//}

				// 고도 차이
				// 현재 숲길 데이터에만 존재
				if (pLink->base.link_type == TYPE_DATA_TREKKING && pNodeNext->node_id != pNode->node_id) {
					altDiff = pNodeNext->tre.z_value - pNode->tre.z_value;
					if (altDiff != 0) {
						altCost = get_road_slope(static_cast<int32_t>(pLink->length), altDiff);
					}
				}

				// 트리 등록
				costReal = GetCost(pLinkNext, pRouteInfo->RouteOption, 0, 0);
				costTreavel = GetTravelCost(pLinkNext, pLink, costReal, angDiff, pRouteInfo->RouteOption, pRouteInfo->RouteOption, pRouteInfo->AvoidOption);

				nextCandidate.parents_id = pCurInfo->parentId.nid;
				nextCandidate.current_id = pCurInfo->linkId.nid;

				pItem = new CandidateLink;
				pItem->candidateId = nextCandidate; // 후보 ID
				pItem->parentId = pCurInfo->linkId;	// 부모 링크 ID
				pItem->linkId = pLinkNext->link_id;  // 링크 ID
				pItem->nodeId = nextNodeId;	// 노드 ID
				pItem->distReal = pLinkNext->length;	// 실 거리
				pItem->timeReal = costReal; // 실 시간
				pItem->costTreavel = pCurInfo->costTreavel + costTreavel;	// 계산 비용
				pItem->costHeuristic = pCurInfo->costTreavel + heuristicCost;	// 가중치 계산 비용
				pItem->depth = pCurInfo->depth + 1;	// 탐색 깊이
				pItem->visited = false; // 방문 여부
				pItem->dir = dirTarget; // 탐색방향

				// 링크 방문 목록 등록
				pRouteInfo->mRoutePass.emplace(candidateId.llid, pItem);
				pRouteInfo->pqDijkstra.emplace(pItem);

				cntLinks++;

#if defined(USE_SHOW_ROUTE_SATATUS)
				//LOG_TRACE(LOG_DEBUG, "id:%lld(real cost:%f, heuristic cost:%f, lv:%d) ", newItem.linkId.llid, newItem.costReal, newItem.costHeuristic, newItem.depth);
#endif
			}
		}
	} // for

	return cntLinks;
}


// 단순 확장
const int CRoutePlan::Propagation(IN TableBaseInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const int dir)
{
	KeyID candidateId;
	int cntLinks = 0;

	candidateId.parents_id = pCurInfo->parentId.nid;
	candidateId.current_id = pCurInfo->linkId.nid;

	// SetVisitedLink(pRouteInfo, candidateId);
	 unordered_map<uint64_t, bool>::iterator it = pRouteInfo->mRoutePass.find(candidateId.llid);
	 if (it != pRouteInfo->mRoutePass.end()) {
		 it->second = true;
	 }


	int altDiff = 0; // 고도차이
	double altCost = 0.f; // 고도차 가중치
	int angDiff = 0;
	int dirTarget = 0;
	KeyID nextNodeId = { 0, };
	stNodeInfo* pNode = nullptr;
	stNodeInfo* pNodeNext = nullptr;
	stLinkInfo* pLink = nullptr;
	stLinkInfo* pLinkNext = nullptr;


	// 현재 링크 get
	candidateId = pCurInfo->linkId;
	//candidateId.dir = 0; // 입력시에 방향성 줬기에 원데이터에서는 방향성 빼고 검색

#if defined(USE_TREKKING_DATA)
	pNode = m_pDataMgr->GetWNodeDataById(pCurInfo->nodeId);
	pLink = m_pDataMgr->GetWLinkDataById(candidateId);
	if (pNode == nullptr && pLink == nullptr) {
		pNode = m_pDataMgr->GetNodeDataById(pCurInfo->nodeId);
		pLink = m_pDataMgr->GetLinkDataById(candidateId);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	pNode = m_pDataMgr->GetWNodeDataById(pCurInfo->nodeId);
	pLink = m_pDataMgr->GetWLinkDataById(candidateId);
#elif defined(USE_VEHICLE_DATA)
	pNode = m_pDataMgr->GetVNodeDataById(pCurInfo->nodeId);
	pLink = m_pDataMgr->GetVLinkDataById(candidateId);
#else
	pNode = m_pDataMgr->GetNodeDataById(pCurInfo->nodeId);
	pLink = m_pDataMgr->GetLinkDataById(candidateId);
#endif

	if (!pLink)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", candidateId.nid);
		return -1;
	}


	// 다음 노드 get
	if (dir == 3) { // 통행불가
		LOG_TRACE(LOG_DEBUG, "entrance disable link, link id:%d", pLink->link_id.nid);
		return -1;
	}
#if defined(USE_P2P_DATA)
	else if (pLink->veh.hd_flag == 0) { // HD 링크와 매칭 정보가 없으면 통행 불가
		return -1;
	}
	else if (dir == 1) { // e 정방향
		nextNodeId = pLink->enode_id;
		angDiff = pLink->veh_ext.enode_ang;
	}
	else if (dir == 2) { // s 역방향으로
		nextNodeId = pLink->snode_id;
		angDiff = pLink->veh_ext.snode_ang;
	}
#else
	else if (dir == 1) { // e 정방향
		nextNodeId = pLink->enode_id;
		angDiff = pLink->base.enode_ang;
	}
	else if (dir == 2) { // s 역방향으로
		nextNodeId = pLink->snode_id;
		angDiff = pLink->base.snode_ang;
	}
#endif
	else {
		// 다음 노드를 찾아야 함.
		if (pCurInfo->nodeId.nid == pLink->enode_id.nid) {
			nextNodeId = pLink->snode_id;
		}
		else if (pCurInfo->nodeId.nid == pLink->snode_id.nid) {
			nextNodeId = pLink->enode_id;
		}
		else {
			LOG_TRACE(LOG_ERROR, "Failed, can't find next node, link id:%d", pLink->link_id.nid);
			return -1;
		}
	}
	//nextNodeId.dir = 0;

#if defined(USE_TREKKING_DATA)
	if (pLink->base.link_type == TYPE_DATA_PEDESTRIAN) {
		pNodeNext = m_pDataMgr->GetWNodeDataById(nextNodeId);
	}
	else {
		pNodeNext = m_pDataMgr->GetNodeDataById(nextNodeId);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	pNodeNext = m_pDataMgr->GetWNodeDataById(nextNodeId);
#elif defined(USE_VEHICLE_DATA)
	pNodeNext = m_pDataMgr->GetVNodeDataById(nextNodeId);
#else
	pNodeNext = m_pDataMgr->GetNodeDataById(nextNodeId);
#endif

	if (!pNodeNext)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't find next node, next node id:%d", nextNodeId.nid);
		return -1;
	}


	// 아래 루틴 1, 2는 getNextPassCode(...) 함수로 대체가능, 중복 계산을 방지하기 위해 나눠 사용 중
	// 루틴 1, 링크의 통행 코드
	int16_t codeLinkPass = 0;
	int16_t retPassCode = 0;
	// 링크의 통행코드에서 링크 자신의 인덱스 위치
	int32_t idxLinkPassOffset = 0;

	if (pLink->base.link_type == TYPE_DATA_VEHICLE) {
		// 현재 링크의 통과 코드에서 요청된 idx의 통과 코드값 계산
		int idxLinkPass = -1;

		for (int ii = 0; ii < pNodeNext->base.connnode_count; ii++) {
			if (pNodeNext->connnodes[ii] == pLink->link_id) {
				idxLinkPass = ii;
				break;
			}
		}

		if (idxLinkPass < 0) {
			LOG_TRACE(LOG_WARNING, "can't find current link matched pass, link tile:%d, id:%d, cnt:%d, idx:%d", pLink->link_id.tile_id, pLink->link_id.nid);
			return -1;
		}

		// 통행코드는 자신의 현재 위치(링크) 다음 방향 값을 첫 인덱스를 시작해 자신을 가장 마지막 인덱스로 구성함.
		// 그러나 요청되는 다음 인덱스는 노드 연결 구성시 정북 기준 순으로 저장되므로 요청된 인덱스의 순서 변경이 필요함
		idxLinkPassOffset = (pNodeNext->base.connnode_count - 1) - idxLinkPass;

		codeLinkPass = pNodeNext->conn_attr[idxLinkPass];
	}
	// - // 루틴 1, 링크의 통행 코드



	double heuristicCost = 0.f;
	double toDestStraightDist = 0.f;
	double costReal = 0.f;
	double costTreavel = 0.f;
	KeyID nextCandidate = { 0, };
	CandidateLink* pItem = nullptr;

	// 노드에 연결된 다음 링크 정보
	for (uint32_t ii = 0; ii < pNodeNext->base.connnode_count; ii++)
	{
		// 루틴 2, 통행 코드 확인
		if ((pLink->base.link_type == TYPE_DATA_VEHICLE) && (codeLinkPass != 0)) {

			int idxCurrentPass = (idxLinkPassOffset + ii);
			if (idxCurrentPass >= pNodeNext->base.connnode_count) {
				idxCurrentPass %= (pNodeNext->base.connnode_count);
			}
			retPassCode = getPassCode(codeLinkPass, idxCurrentPass);

			if (retPassCode < 0) {
				continue;
			}
			else if (retPassCode == 2) { // 통행불가
				continue;
			}
		}
		// - // 루틴 2, 통행 코드 확인

		//if (dir == 1) // 정방향
		//{

		//}
		//else if (dir == 2) // 역방향
		//{

		//}
		//else
		{
#if defined(USE_TREKKING_DATA)
			if (pLink->base.link_type == TYPE_DATA_PEDESTRIAN) {
				pLinkNext = m_pDataMgr->GetWLinkDataById(pNodeNext->connnodes[ii]);
			}
			else {
				pLinkNext = m_pDataMgr->GetLinkDataById(pNodeNext->connnodes[ii]);
			}
#elif defined(USE_PEDESTRIAN_DATA)
			pLinkNext = m_pDataMgr->GetWLinkDataById(pNodeNext->connnodes[ii]);
#elif defined(USE_VEHICLE_DATA)
			pLinkNext = m_pDataMgr->GetVLinkDataById(pNodeNext->connnodes[ii]);
#else
			pLinkNext = m_pDataMgr->GetLinkDataById(pNodeNext->connnodes[ii]);
#endif

			if (!pLinkNext) {
				LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", pNodeNext->connnodes[ii].llid);
				continue;
			}

			// 자기 자신은 제외
			if (pLink->link_id.nid == pLinkNext->link_id.nid && retPassCode != 1) {
				continue;
			}

			// 낮은 레벨은 제외
			if (pLinkNext->veh.level >= USE_ROUTE_TABLE_LEVEL) { // 2차선 이상만 따져보자
				continue;
			}

#if defined(USE_P2P_DATA)
			if (pLinkNext->veh.hd_flag != 1) { // HD 링크와 매칭 정보가 없으면 통행 불가
				continue;
			}
#endif

			candidateId.parents_id = pLink->link_id.nid;
			candidateId.current_id = pLinkNext->link_id.nid;

			// 첫 방문일 경우에만 추가
			//if (!IsVisitedLink(pRouteInfo, candidateId)) { // && !IsAddedLink(pLink->link_id)) {
			if (pRouteInfo->mRoutePass.find(candidateId.llid) == pRouteInfo->mRoutePass.end()) 
			{
				if (pNodeNext->coord.x == pLinkNext->getVertexX(0) && pNodeNext->coord.y == pLinkNext->getVertexY(0))
				{// 다음 링크의 snode 연결
					if (pLinkNext->link_id.dir == 2) { // 일방(역)에 시작점이 일치하면 불가
						continue;
					}
#if defined(USE_P2P_DATA)
					angDiff -= pLinkNext->veh_ext.snode_ang;
#else
					angDiff -= pLinkNext->base.snode_ang;
#endif
					dirTarget = 1; // 정
				}
				else
				{// 다음 링크의 enode 연결
					if (pLinkNext->link_id.dir == 1) { // 일방(정)에 종료점이 일치하면 불가
						continue;
					}

#if defined(USE_P2P_DATA)
					angDiff -= pLinkNext->veh_ext.enode_ang;
#else
					angDiff -= pLinkNext->base.enode_ang;
#endif
					dirTarget = 2; // 역
				}

				toDestStraightDist = getRealWorldDistance(pRouteInfo->routeLinkInfo.Coord.x, pRouteInfo->routeLinkInfo.Coord.y,
					pLinkNext->getVertexX(pLinkNext->getVertexCount() / 2), pLinkNext->getVertexY(pLinkNext->getVertexCount() / 2));

#if defined(USE_OPTIMAL_POINT_API)
#	if defined(_DEBUG)
				static const double fFactor = 0.10f;
				heuristicCost = toDestStraightDist * fFactor;
#	else
				heuristicCost = toDestStraightDist * VAL_HEURISTIC_VEHICLE_FACTOR_FOR_TABLE;
#	endif
#else
				heuristicCost = toDestStraightDist * VAL_HEURISTIC_PEDESTRIAN_FACTOR;
#endif
				// 링크 타입 비교를 안하면 연산에 좀더 빠르려나
				//if (pLinkNext->base.link_type == TYPE_DATA_VEHICLE) {
				//	heuristicCost = static_cast<int>(toDestStraightDist * VAL_HEURISTIC_VEHICLE_FACTOR);
				//}
				//else // if (VAL_HEURISTIC_VEHICLE_FACTOR > 0)
				//{
				//	heuristicCost = static_cast<int>(toDestStraightDist * VAL_HEURISTIC_VEHICLE_FACTOR);
				//}

				// 고도 차이
				// 현재 숲길 데이터에만 존재
				if (pLink->base.link_type == TYPE_DATA_TREKKING && pNodeNext->node_id != pNode->node_id) {
					altDiff = pNodeNext->tre.z_value - pNode->tre.z_value;
					if (altDiff != 0) {
						altCost = get_road_slope(static_cast<int32_t>(pLink->length), altDiff);
					}
				}

				// 트리 등록
				costReal = GetCost(pLinkNext, pRouteInfo->routeOption, 0, 0);
				costTreavel = GetTravelCost(pLinkNext, pLink, costReal, angDiff, pRouteInfo->routeOption, pRouteInfo->routeOption, pRouteInfo->avoidOption);

				nextCandidate.parents_id = pCurInfo->parentId.nid;
				nextCandidate.current_id = pCurInfo->linkId.nid;

				pItem = new CandidateLink;
				pItem->candidateId = nextCandidate; // 후보 ID
				pItem->parentId = pCurInfo->linkId;	// 부모 링크 ID
				pItem->linkId = pLinkNext->link_id;  // 링크 ID
				pItem->nodeId = nextNodeId;	// 노드 ID

				//pItem->distReal = pLinkNext->length;	// 실 거리
				//pItem->timeReal = costReal; // 실 시간
				// 단순 확장은 누적 정보만 사용
				pItem->distReal = pCurInfo->distReal + pLinkNext->length;	// 실 거리
				pItem->timeReal = pCurInfo->timeReal + costReal; // 실 시간
				pItem->costTreavel = pCurInfo->costTreavel + costTreavel;	// 계산 비용
				pItem->costHeuristic = pCurInfo->costTreavel + heuristicCost;	// 가중치 계산 비용
				pItem->depth = pCurInfo->depth + 1;	// 탐색 깊이
				pItem->visited = false; // 방문 여부
				pItem->dir = dirTarget; // 탐색방향


				// 단순 확장은 히스토리 불필요
				//// 링크 방문 목록 등록
				pRouteInfo->mRoutePass.emplace(candidateId.llid, false);
				pRouteInfo->pqDijkstra.emplace(pItem);

				cntLinks++;

#if defined(USE_SHOW_ROUTE_SATATUS)
				//LOG_TRACE(LOG_DEBUG, "id:%lld(real cost:%f, heuristic cost:%f, lv:%d) ", newItem.linkId.llid, newItem.costReal, newItem.costHeuristic, newItem.depth);
#endif
			}
		}
	} // for

	return cntLinks;
}


// nFromOrTo, 1:노드(s/e)에서 정점으로, 2:정점에서 노드(s/e)로
// dir, 1:정방향(s->e), 2:역방향(e->s)
const int getLinkSplitDist(IN const stLinkInfo* pLink, IN const int nLinkSplitIdx, IN const double dbSplitX, IN const double dbSplitY, IN const int dir, IN const int nFromOrTo, OUT vector<SPoint>& vtVtx)
{
	int nDist = 0;

	if (pLink == nullptr || nLinkSplitIdx < 0 || pLink->getVertexCount() <= nLinkSplitIdx) {
		LOG_TRACE(LOG_ERROR, "Failed, getLinkSplitDist param error");
		return 0;
	}

	PT ptFirst, ptNext;



	if (nFromOrTo == 2) // pt -> to s/e
	{
		// 시작은 직교점
		ptFirst.lon = dbSplitX;
		ptFirst.lat = dbSplitY;
		vtVtx.emplace_back(SPoint{ dbSplitX, dbSplitY });

		if (dir == 2) // 역방향 pt -> s
		{
			// 해당 좌표의 직교점 부터 s까지 거리 계산
			for (int ii = nLinkSplitIdx; ii >= 0; --ii)
			{
				ptNext.lon = pLink->getVertexX(ii);
				ptNext.lat = pLink->getVertexY(ii);

				vtVtx.emplace_back(*pLink->getVertex(ii));
				nDist += ptFirst.dist(ptNext);

				ptFirst = ptNext;
			}
		}
		else // 정방향 pt -> e
		{
			// 해당 좌표의 직교점 부터 e까지 거리 계산
			for (int ii = nLinkSplitIdx + 1; ii < pLink->getVertexCount(); ii++)
			{
				ptNext.lon = pLink->getVertexX(ii);
				ptNext.lat = pLink->getVertexY(ii);

				vtVtx.emplace_back(*pLink->getVertex(ii));
				nDist += ptFirst.dist(ptNext);

				ptFirst = ptNext;
			}
		}
	}
	else if (nFromOrTo == 1) // from s/e -> pt
	{
		// 시작은 노드
		if (dir == 1) {
			// 정방향 from s
			ptFirst.lon = pLink->getVertexX(0);
			ptFirst.lat = pLink->getVertexY(0);
			vtVtx.emplace_back(*pLink->getVertex(0));

			// 해당 좌표의 직교점 부터 e까지 거리 계산
			for (int ii = 1; ii < nLinkSplitIdx + 1; ii++)
			{
				ptNext.lon = pLink->getVertexX(ii);
				ptNext.lat = pLink->getVertexY(ii);

				vtVtx.emplace_back(*pLink->getVertex(ii));
				nDist += ptFirst.dist(ptNext);

				ptFirst = ptNext;
			}
		}
		else if (dir == 2)
		{
			// 역방향 from e
			ptFirst.lon = pLink->getVertexX(pLink->getVertexCount() - 1);
			ptFirst.lat = pLink->getVertexY(pLink->getVertexCount() - 1);
			vtVtx.emplace_back(*pLink->getVertex(pLink->getVertexCount() - 1));

			for (int32_t ii = pLink->getVertexCount() - 2; ii > nLinkSplitIdx; --ii)
			{
				ptNext.lon = pLink->getVertexX(ii);
				ptNext.lat = pLink->getVertexY(ii);

				vtVtx.emplace_back(*pLink->getVertex(ii));
				nDist += ptFirst.dist(ptNext);

				ptFirst = ptNext;
			}
		}

		ptNext.lon = dbSplitX;
		ptNext.lat = dbSplitY;

		vtVtx.push_back({ dbSplitX, dbSplitY });
		nDist += ptFirst.dist(ptNext);
	}
	else {
		LOG_TRACE(LOG_ERROR, "Failed, not defined param value, fromto : %d", nFromOrTo);
	}


	return nDist;
}


const int getRouteDirFromPosition(IN const stLinkInfo* pLink, IN const int idxVtx, IN const double x, IN const double y)
{
	int dir = 0;

	// 규제코드, 1:통행가능, 2:통행불가, 3:공사구간, 4:공사계획구간, 5:일방통행_정, 6:일방통행_역 // 5,6은 control 코드 값임.
	if (pLink->veh.pass_code == 5) {
		dir = 1; // 방향성 일치, 오른쪽, 정방향
	}
	else if (pLink->veh.pass_code == 6) {
		dir = 2; // 방향성 불일치, 왼쪽, 역방향
	}
	//else if (pLink->veh.level <= 6) { // 일반도로 2차선 이상만 따지자 
	// --> 6차선 이상으로 임시 변경, 원래는 확장 불가능한지를 확인하고 방향성을 무시해야하지만 우선은 차선으로 구분하자(2022-12-14)
	else if ((pLink->veh.level <= 4) || ((pLink->veh.level >= 5) && (pLink->veh.lane_cnt > 2))) {
		//if (idxVtx >= pLink->getVertexCount() - 1) {
		//	idxVtx = pLink->getVertexCount() - 2;
		//}
		//SPoint st = { pLink->vtPts[idxVtx].x, pLink->vtPts[idxVtx].y };
		//SPoint ed = { pLink->vtPts[idxVtx + 1].x, pLink->vtPts[idxVtx + 1].y };
		SPoint pt = { x, y };

		if (isRightSide(pLink->getVertex(idxVtx), pLink->getVertex(idxVtx + 1), &pt)) {
			dir = 1; // 방향성 일치, 오른쪽, 정방향
		}
		else {
			dir = 2; // 방향성 불일치, 왼쪽, 역방향
		}
	}

	return dir;
}


const bool CRoutePlan::SetRouteLinkInfo(IN const SPoint& ptPosition, const IN KeyID keyLink, IN const bool isStart, OUT RouteLinkInfo* pRouteLinkInfo)
{
	if (pRouteLinkInfo == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, set start/end link info, output param null");
		return false;
	}
	else if (keyLink.llid <= 0) {
		LOG_TRACE(LOG_ERROR, "Failed, start/end link info not set, start:%d, end:%d", keyLink.llid);
		return false;
	}


	// 설정 좌표에서 최대 링크 매칭 거리
	stLinkInfo* pLink = nullptr;
	SPoint ptSplitLink;
	double retDist = searchRange[MAX_SEARCH_RANGE - 1];

	// 시작점 링크의 매칭 vertex를 구하자
#if defined(USE_TREKKING_DATA)
	pLink = m_pDataMgr->GetWLinkDataById(keyLink);
	if (pLink == nullptr) {
		pLink = m_pDataMgr->GetLinkDataById(keyLink);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	pLink = m_pDataMgr->GetWLinkDataById(keyLink);
#elif defined(USE_VEHICLE_DATA)
	pLink = m_pDataMgr->GetVLinkDataById(keyLink);
#else
	pLink = m_pDataMgr->GetLinkDataById(keyLink);
#endif

	int32_t nLinkMatchVtxIdx = m_pDataMgr->GetLinkVertexDataByPoint(ptPosition.x, ptPosition.y, searchRange[MAX_SEARCH_RANGE - 1], keyLink, ptSplitLink.x, ptSplitLink.y, retDist);
	if (nLinkMatchVtxIdx < 0) {
		LOG_TRACE(LOG_ERROR, "Failed, link vertex not match, x:%d, y:%d, link:%lld", ptSplitLink.x, ptSplitLink.y, keyLink.llid);

		return false;
	}

	// 시작점은 s 노드로의 진행은 역방향, // 종료점은 s 노드에서의 진입은 정방향
	pRouteLinkInfo->LinkDistToS = getLinkSplitDist(pLink, nLinkMatchVtxIdx, ptSplitLink.x, ptSplitLink.y, (isStart ? 2 : 1), (isStart ? 2 : 1), pRouteLinkInfo->LinkVtxToS);
	// 시작점은 e 노드로의 진행은 정방향, // 종료점은 e 노드에서의 진입은 역방향
	pRouteLinkInfo->LinkDistToE = getLinkSplitDist(pLink, nLinkMatchVtxIdx, ptSplitLink.x, ptSplitLink.y, (isStart ? 1 : 2), (isStart ? 2 : 1), pRouteLinkInfo->LinkVtxToE);

	pRouteLinkInfo->LinkId = keyLink;
	pRouteLinkInfo->Coord = ptPosition;
	pRouteLinkInfo->MatchCoord = ptSplitLink;
	pRouteLinkInfo->LinkSplitIdx = nLinkMatchVtxIdx;

	// 차량 네트워크는 방향성 따지자
#if defined(USE_P2P_DATA)
	if (pLink->veh.hd_flag == 2) { // 링크가 HD와 부분 매칭이면 역-진입 시, 탐색 실패가 나올 수 있어 양방향 탐색하도록 하자
		pRouteLinkInfo->LinkDir = 0;
	}
	else
#endif
	if (pLink->base.link_type == TYPE_DATA_VEHICLE) {
		pRouteLinkInfo->LinkDir = getRouteDirFromPosition(pLink, nLinkMatchVtxIdx, ptPosition.x, ptPosition.y);
	}
	else { // 방향성 기본			
		pRouteLinkInfo->LinkDir = 0;
	}

	return true;
}


const int CRoutePlan::DoRoute(IN const uint32_t reqId, IN const SPoint ptStart, IN const SPoint ptEnd, IN const KeyID sLink, IN const KeyID eLink, IN const uint32_t routeOpt, IN const uint32_t avoidOpt, OUT RouteResultInfo* pRouteResult)
{
	RouteInfo routeInfo;
	double retDist = MAX_SEARCH_DIST;

	Initialize();

	routeInfo.RequestId = reqId;

	if (pRouteResult == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, request info param null, pRouteResults:%p", pRouteResult);

		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_WRONG_PARAM;
		return pRouteResult->ResultCode; 
	}

	if ((retDist = getRealWorldDistance(ptStart.x, ptStart.y, ptEnd.x, ptEnd.y)) > MAX_SEARCH_DIST) {
		LOG_TRACE(LOG_ERROR, "Failed, max request route distance(%d) over than %d", retDist, MAX_SEARCH_DIST);

		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_DIST_OVER;
		return pRouteResult->ResultCode;
	}


	// 시작점 정보
	if (!SetRouteLinkInfo(ptStart, sLink, true, &routeInfo.StartLinkInfo)) {
		LOG_TRACE(LOG_ERROR, "Failed, set start link info");

		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_SET_START;
		return pRouteResult->ResultCode;;
	}

	// 종료점 정보
	if (!SetRouteLinkInfo(ptEnd, eLink, false, &routeInfo.EndLinkInfo)) {
		LOG_TRACE(LOG_ERROR, "Failed, set end link info");

		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_SET_END;
		return pRouteResult->ResultCode;;
	}


	// 경탐 옵션 
	routeInfo.RouteOption = routeOpt;
	// 회피 옵션
	routeInfo.AvoidOption = avoidOpt;


	// 함수로 대체
	int ret = MakeRoute(&routeInfo, pRouteResult);

	return ret;
}


const int CRoutePlan::DoRoutes(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults)
{
	if (pReqInfo == nullptr || pRouteInfos == nullptr || pRouteResults == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, request info param null, pReqInfo:%p, pRouteInfos:%p, pRouteResults:%p", pReqInfo, pRouteInfos, pRouteResults);

		return ROUTE_RESULT_FAILED;
	}

	
	int32_t cntPoints = pReqInfo->vtPoints.size();

	if (cntPoints < 2 || (pReqInfo->vtPoints.size() != pReqInfo->vtIdLinks.size())) {
		LOG_TRACE(LOG_WARNING, "Failed, route request point too short, points:%d, links:%d", pReqInfo->vtPoints.size(), pReqInfo->vtIdLinks.size());

		return ROUTE_RESULT_FAILED_WRONG_PARAM;
	}


	Initialize();

#if defined(USE_MULTIPROCESS)
	pRouteInfos->resize(cntPoints);
	pRouteResults->resize(cntPoints);
#endif

#pragma omp parallel for
	for (int ii = 0; ii < cntPoints - 1; ii++)
	{
#if defined(USE_MULTIPROCESS)
		RouteInfo& routeInfo = pRouteInfos->at(ii);
		RouteResultInfo& routeResult = pRouteResults->at(ii);
#else
		RouteInfo routeInfo;
		RouteResultInfo routeResult;
#endif

		double retDist = MAX_SEARCH_DIST;

		// 경탐 옵션 
		routeInfo.RouteOption = pReqInfo->RouteOption;
		// 회피 옵션
		routeInfo.AvoidOption = pReqInfo->AvoidOption;

		KeyID sLink = pReqInfo->vtIdLinks[ii];
		KeyID eLink = pReqInfo->vtIdLinks[ii + 1];

		SPoint ptStart = { pReqInfo->vtPoints[ii].x, pReqInfo->vtPoints[ii].y };
		SPoint ptEnd = { pReqInfo->vtPoints[ii + 1].x, pReqInfo->vtPoints[ii + 1].y };

		if ((retDist = getRealWorldDistance(ptStart.x, ptStart.y, ptEnd.x, ptEnd.y)) > MAX_SEARCH_DIST) {
			LOG_TRACE(LOG_ERROR, "Failed, max request route distance(%d) over than %d", retDist, MAX_SEARCH_DIST);

			routeResult.ResultCode = ROUTE_RESULT_FAILED_DIST_OVER;
			//return routeResult.ResultCode;
		}

		// 시작점 정보
		if (!SetRouteLinkInfo(ptStart, sLink, true, &routeInfo.StartLinkInfo)) {
			LOG_TRACE(LOG_ERROR, "Failed, set start link info");

			if (ii == 0) {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_START;
			}
			else {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_VIA;
			}
			//return routeResult.ResultCode;
		}

		// 종료점 정보
		if (!SetRouteLinkInfo(ptEnd, eLink, false, &routeInfo.EndLinkInfo)) {
			LOG_TRACE(LOG_ERROR, "Failed, set end link info");

			if (ii < cntPoints - 1) {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_END;
			}
			else {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_VIA;
			}

			//return routeResult.ResultCode;
		}


#if defined(USE_MULTIPROCESS)
		int ret = MakeRoute(&routeInfo, &routeResult);

		if (ret != ROUTE_RESULT_SUCCESS) {
			//return routeResult.ResultCode;
		}
#else
		if (pRouteInfos) {
			pRouteInfos->emplace_back(routeInfo);
		}

		int ret = MakeRoute(&routeInfo, &routeResult);

		if (ret != ROUTE_RESULT_SUCCESS) {
			return routeResult.ResultCode;
		}

		if (pRouteResults) {
			pRouteResults->emplace_back(routeResult);
		}
#endif


		if (ii == 0) {
			routeResult.LinkInfo.front().type = LINK_GUIDE_TYPE_DEPARTURE; // 출발지
		}

		if (ii == cntPoints - 2) {
			routeResult.LinkInfo.back().type = LINK_GUIDE_TYPE_DESTINATION; // 도착지
		}
		else {
			routeResult.LinkInfo.back().type = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
		}

	} // for
	

	return ROUTE_RESULT_SUCCESS;
}


#if defined(USE_TSP_MODULE)
//const int CRoutePlan::DoTabulate(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults)
const int CRoutePlan::DoTabulate(IN const RequestRouteInfo* pReqInfo, OUT RouteTable** ppResultTables)
{
	//if (pReqInfo == nullptr || pRouteInfos == nullptr || pRouteResults == nullptr) {
	//	LOG_TRACE(LOG_ERROR, "Failed, request info param null, pReqInfo:%p, pRouteInfos:%p, pRouteResults:%p", pReqInfo, pRouteInfos, pRouteResults);

	//	return ROUTE_RESULT_FAILED;
	//}
	if (pReqInfo == nullptr || ppResultTables == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, request info param null, pReqInfo:%p, ppResultTables:%p", pReqInfo, ppResultTables);

		return ROUTE_RESULT_FAILED;
	}

	int32_t cntPoints = pReqInfo->vtPoints.size();

	if (cntPoints <= 2 || (pReqInfo->vtPoints.size() != pReqInfo->vtIdLinks.size())) {
		LOG_TRACE(LOG_WARNING, "Failed, route request point too short, points:%d, links:%d", pReqInfo->vtPoints.size(), pReqInfo->vtIdLinks.size());

		return ROUTE_RESULT_FAILED_WRONG_PARAM;
	}


	Initialize();

	RouteResultInfo routeResult;
	vector<RouteLinkInfo> mapLocations;

	for (int ii = 0; ii < cntPoints; ii++) {
		RouteLinkInfo newInfo;

		KeyID keyLink =  pReqInfo->vtIdLinks[ii];
		SPoint ptRequest = { pReqInfo->vtPoints[ii].x, pReqInfo->vtPoints[ii].y };

		// 각 지점의 매칭 정보
		if (!SetRouteLinkInfo(ptRequest, keyLink, true, &newInfo)) {
			LOG_TRACE(LOG_ERROR, "Failed, set tabuate link info, idx:%d, map:%d, link;%d, x:%.5f, y:%.5f", ii, keyLink.tile_id, keyLink.nid, ptRequest.x, ptRequest.y);

			return ROUTE_RESULT_FAILED_SET_VIA;
		}

		mapLocations.emplace_back(newInfo);
	} // for





	int ret = MakeTabulate(mapLocations, ppResultTables);

	if (ret == ROUTE_RESULT_SUCCESS) {
		routeResult.RequestMode = pReqInfo->RequestMode;
		routeResult.RequestId = pReqInfo->RequestId;
		routeResult.RouteOption = pReqInfo->RouteOption;
		routeResult.RouteAvoid = pReqInfo->AvoidOption;
	}

	//pRouteResults->emplace_back(routeResult);

	return ret;
}
#endif // #if defined(USE_TSP_MODULE)


const int CRoutePlan::MakeRoute(IN RouteInfo* pRouteInfo, OUT RouteResultInfo* pRouteResult)
{
	stLinkInfo* pLink = nullptr;
	stNodeInfo* pNode = nullptr;

	// 종료 지점의 탐색 매칭 카운트 계산
	uint32_t cntGoal = 0; // 목적지 도달 가능한 링크 수 
	uint32_t cntGoalTouch = 0; // 목적지 도달한 링크 수
	CandidateLink checkFinishRouteDir[2];// = { 0, }; // 최대 링크 양단에서 한번씩만 허용
	checkFinishRouteDir[0].costTreavel = DBL_MAX;
	checkFinishRouteDir[1].costTreavel = DBL_MAX;

	if (pRouteInfo == nullptr || pRouteResult == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, pRouteInfo:%p, pRouteResult:%p", pRouteInfo, pRouteResult);

		return ROUTE_RESULT_FAILED;
	}


	// 종료점 링크의
#if defined(USE_TREKKING_DATA)
	pLink = m_pDataMgr->GetWLinkDataById(pRouteInfo->EndLinkInfo.LinkId);
	if (pLink == nullptr) {
		pLink = m_pDataMgr->GetLinkDataById(pRouteInfo->EndLinkInfo.LinkId);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	pLink = m_pDataMgr->GetWLinkDataById(pRouteInfo->EndLinkInfo.LinkId);
#elif defined(USE_VEHICLE_DATA)
	pLink = m_pDataMgr->GetVLinkDataById(pRouteInfo->EndLinkInfo.LinkId);
#else
	pLink = m_pDataMgr->GetLinkDataById(pRouteInfo->EndLinkInfo.LinkId);
#endif	

	if (!pLink)
	{
		LOG_TRACE(LOG_WARNING, "Failed, can't find end link, tile:%d, link:%d", pRouteInfo->EndLinkInfo.LinkId.tile_id, pRouteInfo->EndLinkInfo.LinkId.nid);

		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_READ_DATA;
		return pRouteResult->ResultCode;
	}

	// s 노드에서의 진입은 정방향
	if (pRouteInfo->EndLinkInfo.LinkDir == 0 || (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 1)) && (pRouteInfo->EndLinkInfo.LinkDir == 1))) // 양방향 or 정방향
	{
#if defined(USE_TREKKING_DATA)
		pNode = m_pDataMgr->GetWNodeDataById(pLink->snode_id);
		if (pNode == nullptr) {
			pNode = m_pDataMgr->GetNodeDataById(pLink->snode_id);
		}
#elif defined(USE_PEDESTRIAN_DATA)
		pNode = m_pDataMgr->GetWNodeDataById(pLink->snode_id);
#elif defined(USE_VEHICLE_DATA)
		pNode = m_pDataMgr->GetVNodeDataById(pLink->snode_id);
#else
		pNode = m_pDataMgr->GetNodeDataById(pLink->snode_id);
#endif

		// S 노드가 단점 아닌지 확인
		if (pNode && pNode->base.point_type != 2 && pNode->base.connnode_count >= 2) {
			cntGoal++;
		}
	}

	// e 노드에서의 진입은 역방향
	if (pRouteInfo->EndLinkInfo.LinkDir == 0 || (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 2)) && (pRouteInfo->EndLinkInfo.LinkDir == 2))) // 양방향 or 역방향
	{
#if defined(USE_TREKKING_DATA)
		pNode = m_pDataMgr->GetWNodeDataById(pLink->enode_id);
		if (pNode == nullptr) {
			pNode = m_pDataMgr->GetNodeDataById(pLink->enode_id);
		}
#elif defined(USE_PEDESTRIAN_DATA)
		pNode = m_pDataMgr->GetWNodeDataById(pLink->enode_id);
#elif defined(USE_VEHICLE_DATA)
		pNode = m_pDataMgr->GetVNodeDataById(pLink->enode_id);
#else
		pNode = m_pDataMgr->GetNodeDataById(pLink->enode_id);
#endif

		// E 노드가 단점 아닌지 확인
		if (pNode && pNode->base.point_type != 2 && pNode->base.connnode_count >= 2) {
			cntGoal++;
		}
	}



	// 시작 정보 등록
#if defined(USE_TREKKING_DATA)
	pLink = m_pDataMgr->GetWLinkDataById(pRouteInfo->StartLinkInfo.LinkId);
	if (pLink == nullptr) {
		pLink = m_pDataMgr->GetLinkDataById(pRouteInfo->StartLinkInfo.LinkId);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	pLink = m_pDataMgr->GetWLinkDataById(pRouteInfo->StartLinkInfo.LinkId);
#elif defined(USE_VEHICLE_DATA)
	pLink = m_pDataMgr->GetVLinkDataById(pRouteInfo->StartLinkInfo.LinkId);
#else
	pLink = m_pDataMgr->GetLinkDataById(pRouteInfo->StartLinkInfo.LinkId);
#endif

	if (!pLink)
	{
		LOG_TRACE(LOG_WARNING, "Failed, can't find start link, tile:%d, link:%d", pRouteInfo->StartLinkInfo.LinkId.tile_id, pRouteInfo->StartLinkInfo.LinkId.nid);

		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_READ_DATA;
		return pRouteResult->ResultCode;
	}

	KeyID candidateId;
	if (pRouteInfo->StartLinkInfo.LinkDir == 0 || (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 2)) && (pRouteInfo->StartLinkInfo.LinkDir == 2))) // 양방향 or 역방향
	{
		//candidateId.parents_id = 2;// + s노드 역방향 정보 이력 - 최초는 부모가 없으니까 진행 방향을 값으로 주자
		candidateId.parents_id = 2; // 시작 노드의 방향
		candidateId.current_id = pLink->link_id.nid;

		//KeyID curLink = candidateId;
		//curLink.dir = 2;// + s노드 역방향 정보 이력


		CandidateLink* pItem = new CandidateLink;
		pItem->candidateId = candidateId; // 후보 ID
		pItem->parentId.llid = candidateId.parents_id;	// 부모 링크 ID
		pItem->linkId = pLink->link_id;  // 링크 ID
		pItem->nodeId = pLink->snode_id;	// 노드 ID
		pItem->distReal = pRouteInfo->StartLinkInfo.LinkDistToS; // 실 거리
		pItem->timeReal = GetCost(pLink, pRouteInfo->RouteOption, pRouteInfo->StartLinkInfo.LinkDistToS, 0); // 실 주행 시간
		pItem->costTreavel = pRouteInfo->StartLinkInfo.LinkDistToS;	// 계산 비용
		pItem->costHeuristic = pRouteInfo->StartLinkInfo.LinkDistToS;	// 가중치 계산 비용
		pItem->depth = 0;	// 탐색 깊이
		pItem->visited = false; // 방문 여부
		pItem->dir = candidateId.parents_id; // 탐색방향


		// 링크 방문 목록 등록
		pRouteInfo->mRoutePass.emplace(candidateId.llid, pItem);
		pRouteInfo->pqDijkstra.emplace(pItem);
	}


	if (pRouteInfo->StartLinkInfo.LinkDir == 0 || (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 1)) && (pRouteInfo->StartLinkInfo.LinkDir == 1))) // 양방향 or 정방향
	{
		//candidateId.parents_id = 1;// + e노드 정방향 정보 이력 - 최초는 부모가 없으니까 진행 방향을 값으로 주자
		candidateId.parents_id = 1; // 시작 노드의 방향
		candidateId.current_id = pLink->link_id.nid;

		//KeyID curLink = candidateId;
		//curLink.dir = 1;// + e노드 정방향 정보 이력

		CandidateLink* pItem = new CandidateLink;
		pItem->candidateId = candidateId; // 후보 ID
		pItem->parentId.llid = candidateId.parents_id;	// 부모 링크 ID
		pItem->linkId = pLink->link_id;  // 링크 ID
		pItem->nodeId = pLink->enode_id;	// 노드 ID
		pItem->distReal = pRouteInfo->StartLinkInfo.LinkDistToE;	// 실 거리
		pItem->timeReal = GetCost(pLink, pRouteInfo->RouteOption, pRouteInfo->StartLinkInfo.LinkDistToE, 0); // 실 주행 시간
		pItem->costTreavel = pRouteInfo->StartLinkInfo.LinkDistToE;	// 계산 비용
		pItem->costHeuristic = pRouteInfo->StartLinkInfo.LinkDistToE;	// 가중치 계산 비용
		pItem->depth = 0;	// 탐색 깊이
		pItem->visited = false; // 방문 여부
		pItem->dir = candidateId.parents_id; // 탐색방향

		// 링크 방문 목록 등록
		pRouteInfo->mRoutePass.emplace(candidateId.llid, pItem);
		pRouteInfo->pqDijkstra.emplace(pItem);
	}



	// LOG_TRACE(LOG_DEBUG, "Start tree : link:%d", pLink->link_id.nid);


	CandidateLink* pCurInfo;
	static const uint32_t cntMaxExtraSearch = 100; // 목적지 추가 도달 검사 최대 허용치
	uint32_t cntExtraSearch = 0; // 목적지 도달 후 추가 도달 검사 카운트

	// 경로 탐색 성공
	LOG_TRACE(LOG_DEBUG, "Start find goal");

	while (!pRouteInfo->pqDijkstra.empty())
	{
		pCurInfo = pRouteInfo->pqDijkstra.top();
		pCurInfo->visited = true;

		// 현재 링크 트리에서 제거
		pRouteInfo->pqDijkstra.pop();

#if defined(USE_SHOW_ROUTE_SATATUS)
		//LOG_TRACE(LOG_DEBUG, "Current Link : id:%lld(real cost:%f, heuristic cost:%f, lv:%d)", curInfo.linkId, curInfo.costReal, curInfo.costHeuristic, curInfo.depth);

		// 현재까지 탐색된 정보 전달, Function Call
		if (m_fpRoutingStatus) {
			m_fpRoutingStatus(m_pHost, &pRouteInfo->mRoutePass);
		}
#endif



		if (((pCurInfo->linkId.llid == pRouteInfo->EndLinkInfo.LinkId.llid) && ((pRouteInfo->EndLinkInfo.LinkDir == 0) || (pCurInfo->dir == pRouteInfo->EndLinkInfo.LinkDir))) ||
			(cntMaxExtraSearch < cntExtraSearch))
		{
			// 목적지 도달 최대 한도 이전까지만 비교, 최대 한도치 넘으면 이전 결과로 성공
			if (cntGoal <= 1)
			{
				if (pRouteInfo->EndLinkInfo.LinkDir == 1 && pCurInfo->dir == 1) {
					checkFinishRouteDir[0].dir = 1;
					cntGoalTouch++;
				}
				else if (pRouteInfo->EndLinkInfo.LinkDir == 2 && pCurInfo->dir == 2) {
					checkFinishRouteDir[0].dir = 2;
					cntGoalTouch++;
				}
				else if (pRouteInfo->EndLinkInfo.LinkDir == 0) {
					checkFinishRouteDir[0].dir = pCurInfo->dir;
					cntGoalTouch++;

					//// s 노드 정보와 일치하면
					//if (pCurInfo->dir ->nodeId == pEndLinkSNode->node_id)
					//{
					//	//checkFinishRouteDir[0].linkId.dir = 1;
					//	
					//}
					//// e 노드 정보와 일치하면
					//else if (pCurInfo->nodeId == pEndLinkENode->node_id)
					//{
					//	//checkFinishRouteDir[0].linkId.dir = 2;
					//	checkFinishRouteDir[0].dir = 2;
					//}
				}
				else {
					continue;
				}

				if (cntGoalTouch) {
					memcpy(&checkFinishRouteDir[0], pCurInfo, sizeof(checkFinishRouteDir[0]));
				}
			}
			else if ((cntGoal >= 2) && (cntMaxExtraSearch > cntExtraSearch))
			{
				// s 노드 정보와 일치하면
				//if (pCurInfo->nodeId == pEndLinkSNode->node_id)
				if (pCurInfo->dir == 1)
				{
					if (checkFinishRouteDir[0].linkId.llid == 0) {
						//checkFinishRouteDir[0] = curInfo;
						memcpy(&checkFinishRouteDir[0], pCurInfo, sizeof(checkFinishRouteDir[0]));
						//checkFinishRouteDir[0].linkId.dir = 1;
						checkFinishRouteDir[0].dir = 1;
						cntGoalTouch++;
					}
				}
				// e 노드 정보와 일치하면
				//else if (pCurInfo->nodeId == pEndLinkENode->node_id)
				else if (pCurInfo->dir == 2)
				{
					if (checkFinishRouteDir[1].linkId.llid == 0) {
						//checkFinishRouteDir[1] = curInfo;
						memcpy(&checkFinishRouteDir[1], pCurInfo, sizeof(checkFinishRouteDir[1]));
						//checkFinishRouteDir[1].linkId.dir = 2;
						checkFinishRouteDir[1].dir = 2;
						cntGoalTouch++;
					}
				}

				// 종료점 양단의 노드로 경탐될때까지
				if (cntGoalTouch < cntGoal)
				{
					// 목적지 추가 탐색 가능하도록 현재 방문 정보 삭제
					//canKey.parents_id = curInfo.parentId.nid;
					//canKey.current_id = curInfo.linkId.nid;
					//mRoutePass.erase(canKey);

					continue;
				}
			}



			// 경로 탐색 성공
			LOG_TRACE(LOG_DEBUG, "Success find goal (%d/%d)", cntGoalTouch, cntGoal);

			if (cntGoal >= 2) {
				// 양단 모두 확인 필요
				// 탐색 요금이 작은 결과를 사용하자.
				if (checkFinishRouteDir[0].costTreavel <= checkFinishRouteDir[1].costTreavel) {
					pCurInfo = &checkFinishRouteDir[0];
				}
				else {
					pCurInfo = &checkFinishRouteDir[1];
				}
			}
			else {
				pCurInfo = &checkFinishRouteDir[0];
			}


			//char szMsg[MAX_PATH] = { 0, };
			//LOG_TRACE(LOG_DEBUG, "Reversed Path : ");
			//LOG_TRACE(LOG_DEBUG, " !! idx:%lld(mesh:%d, id:%d), real cost:%f, heuristic cost:%f, lv:%d", curInfo.linkId, pLink->link_id.tile_id, pLink->link_id.nid, curInfo.costReal, curInfo.costHeuristic, curInfo.depth);


			//LOG_TRACE(LOG_DEBUG, "max tree depth : %d", pCurInfo->depth);

			// 큐 트리 깊이 + 1 (마지막은 저장안되니까)가 전체 경로 링크 사이즈가 됨
			pRouteInfo->vtCandidateResult.reserve(pCurInfo->depth + 1);

			// 종료점 정보 최초 등록

			pRouteInfo->vtCandidateResult.emplace_back(pCurInfo);

			unordered_map<uint64_t, CandidateLink*>::const_iterator it;
			int visitStartLinkCnt = 0;
			bool finding = true;
			while (finding)
			{
				it = pRouteInfo->mRoutePass.find(pCurInfo->candidateId.llid);

				if (it == pRouteInfo->mRoutePass.end())
				{
					finding = false;
					LOG_TRACE(LOG_ERROR, "Failed, can't find start link");
				}
				else
				{
					pCurInfo = it->second;
					pRouteInfo->vtCandidateResult.emplace_back(pCurInfo);

					// 종료 지점 확인
					if (pCurInfo->linkId.llid == pRouteInfo->StartLinkInfo.LinkId.llid) {
						visitStartLinkCnt++;
						if (pRouteInfo->StartLinkInfo.LinkDir == 0 || (pRouteInfo->StartLinkInfo.LinkDir != 0 && pCurInfo->dir == pRouteInfo->StartLinkInfo.LinkDir))
						{
							finding = false;
							LOG_TRACE(LOG_DEBUG, "Success find start");
						}
					}
				}



				//				if (it == mRoutePass.end())
				//				{
				//					finding = false;
				//				}
				//				else
				//				{
				//#if defined(USE_SHOW_ROUTE_SATATUS)
				//					//LOG_TRACE(LOG_DEBUG, " <- idx:%lld(mesh:%d, id:%d), real cost:%f, heuristic cost:%f, lv:%d", it->second.linkId.llid, it->second.linkId.tile_id, it->second.linkId.nid, it->second.costReal, it->second.costHeuristic, it->second.depth);
				//#endif
				//
				//					
				//
				//					// find parents
				//					// 시작점에는 방향성을 강제 부여했으므로 링크 ID만 확인하자.
				//					//CandidateKey { it->first }.current_id
				//					/*canKey.llid = it->first;
				//					if (canKey.current_id == pRouteInfo->StartLinkId.nid)*/
				//					
				//					if (KeyID{ it->first }.current_id == pRouteInfo->StartLinkInfo.LinkId.nid)
				//					{
				//						finding = false;
				//					}
				//					else
				//					{
				//						it = mRoutePass.find(it->second->candidateId.llid);
				//					}
				//				}

				// 시작점을 유턴포함 2번이상 지날일이 없으니까
				if (finding == true && (visitStartLinkCnt >= 2 || pRouteInfo->vtCandidateResult.size() >= pRouteInfo->mRoutePass.size())) {
					finding = false;
					LOG_TRACE(LOG_WARNING, "Failed, can't find start link, finding count over the base, cnt:%d", cntExtraSearch);
				}
			} // while

			int ret = MakeRouteResult(pRouteInfo, pRouteResult);

			//LOG_TRACE(LOG_DEBUG, "max result size : %d", m_routeResult.LinkInfo.size());

			return ret;
		}
		else {
			/*AddNewLinks(&m_routeInfo, pCurInfo, pCurInfo->linkId.dir);*/
			AddNewLinks(pRouteInfo, pCurInfo, pCurInfo->dir);

			if (cntGoalTouch > 0) {
				cntExtraSearch++;
			}
		}

		// 연결 노드의 링크 셋
#if defined(USE_SHOW_ROUTE_SATATUS)
		//LOG_TRACE(LOG_DEBUG, "Add new tree : ");

		//LOG_TRACE(LOG_DEBUG, "Visited Link : ");
		//for (map<uint32_t, Blob>::iterator it = mRoute.begin(); it != mRoute.end(); it++)
		//{
		//	LOG_TRACE(LOG_DEBUG, "id:%d(%d) ", it->second.linkId, it->second.cost);
		//}
#endif
	}

	LOG_TRACE(LOG_DEBUG, "Failed, can't find route, couldn't leached end point(the point will isolated)");

	/* display
	WCHAR buff[128];
	while (!pqScore.empty())
	{
	wsprintf(buff, L"%d, %d \n", pqScore.top().id, pqScore.top().length);
	OutputDebugString(buff);
	pqScore.pop();
	}
	*/

	pRouteResult->ResultCode = ROUTE_RESULT_FAILED_EXPEND;

	return pRouteResult->ResultCode;
}


// 출발지 링크의 매칭 정보
uint32_t CheckStartDirectionMaching(IN const stLinkInfo* pLink, IN const RouteLinkInfo* pRoutLinkInfo, IN const int32_t routeOpt, OUT vector<CandidateLink>& vtCandidateInfo)
{
	KeyID candidateId;
	if (pRoutLinkInfo->LinkDir == 0 || (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 2)) && (pRoutLinkInfo->LinkDir == 2))) // 양방향 or 역방향
	{
		//candidateId.parents_id = 2;// + s노드 역방향 정보 이력 - 최초는 부모가 없으니까 진행 방향을 값으로 주자
		candidateId.parents_id = 2; // 시작 노드의 방향
		candidateId.current_id = pLink->link_id.nid;

		//KeyID curLink = candidateId;
		//curLink.dir = 2;// + s노드 역방향 정보 이력

		CandidateLink pItem;
		pItem.candidateId = candidateId; // 후보 ID
		pItem.parentId.llid = candidateId.parents_id;	// 부모 링크 ID
		pItem.linkId = pLink->link_id;  // 링크 ID
		pItem.nodeId = pLink->snode_id;	// 노드 ID
		pItem.distReal = pRoutLinkInfo->LinkDistToS; // 실 거리
		pItem.timeReal = GetCost(pLink, routeOpt, pRoutLinkInfo->LinkDistToS, 0); // 실 주행 시간
		pItem.costTreavel = pRoutLinkInfo->LinkDistToS;	// 계산 비용
		pItem.costHeuristic = pRoutLinkInfo->LinkDistToS;	// 가중치 계산 비용
		pItem.depth = 0;	// 탐색 깊이
		pItem.visited = false; // 방문 여부
		pItem.dir = candidateId.parents_id; // 탐색방향


		 // 링크 방문 목록 등록
		vtCandidateInfo.emplace_back(pItem);
	}


	if (pRoutLinkInfo->LinkDir == 0 || (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 1)) && (pRoutLinkInfo->LinkDir == 1))) // 양방향 or 역방향
	{
		//candidateId.parents_id = 1;// + e노드 정방향 정보 이력 - 최초는 부모가 없으니까 진행 방향을 값으로 주자
		candidateId.parents_id = 1; // 시작 노드의 방향
		candidateId.current_id = pLink->link_id.nid;

		//KeyID curLink = candidateId;
		//curLink.dir = 1;// + e노드 정방향 정보 이력

		CandidateLink pItem;
		pItem.candidateId = candidateId; // 후보 ID
		pItem.parentId.llid = candidateId.parents_id;	// 부모 링크 ID
		pItem.linkId = pLink->link_id;  // 링크 ID
		pItem.nodeId = pLink->enode_id;	// 노드 ID
		pItem.distReal = pRoutLinkInfo->LinkDistToE;	// 실 거리
		pItem.timeReal = GetCost(pLink, routeOpt, pRoutLinkInfo->LinkDistToE, 0); // 실 주행 시간
		pItem.costTreavel = pRoutLinkInfo->LinkDistToE;	// 계산 비용
		pItem.costHeuristic = pRoutLinkInfo->LinkDistToE;	// 가중치 계산 비용
		pItem.depth = 0;	// 탐색 깊이
		pItem.visited = false; // 방문 여부
		pItem.dir = candidateId.parents_id; // 탐색방향

		// 링크 방문 목록 등록
		vtCandidateInfo.emplace_back(pItem);
	}

	return ROUTE_RESULT_SUCCESS;
}


// 도착지 링크의 매칭 정보
uint32_t CheckEndDirectionMaching(IN const stLinkInfo* pLink, IN const stNodeInfo* pSNode, IN const stNodeInfo* pENode, IN const RouteLinkInfo* pRoutLinkInfo, IN const int32_t routeOpt, OUT vector<CandidateLink>& vtCandidateInfo)
{
	// s 노드에서의 진입은 정방향
	if (pRoutLinkInfo->LinkDir == 0 || (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 1)) && (pRoutLinkInfo->LinkDir == 1))) // 양방향 or 정방향
	{
		// S 노드가 단점 아닌지 확인
		if (pSNode && pSNode->base.point_type != 2 && pSNode->base.connnode_count >= 2) {
			// 노드에서 진입 가능한 모든 경우 확인
			for (int ii = 0; ii < pSNode->base.connnode_count; ii++) {
				// 자신은 제외
				if (pLink->link_id == pSNode->connnodes[ii]) {
					continue;
				}

				if (getPrevPassCode(pLink->link_id, pSNode->connnodes[ii], pSNode) == false) {
					continue;
				}

				KeyID candidateId;
				candidateId.parents_id = pSNode->connnodes[ii].dir; // 시작 링크의 방향
				candidateId.current_id = pLink->link_id.nid;
				
				CandidateLink pItem;
				pItem.candidateId = candidateId; // 후보 ID
				pItem.parentId.llid = candidateId.parents_id;	// 부모 링크 ID
				pItem.linkId = pLink->link_id;  // 링크 ID
				pItem.nodeId = pLink->snode_id;	// 노드 ID
				pItem.distReal = pRoutLinkInfo->LinkDistToS; // 실 거리
				pItem.timeReal = GetCost(pLink, routeOpt, pRoutLinkInfo->LinkDistToS, 0); // 실 주행 시간
				pItem.costTreavel = pRoutLinkInfo->LinkDistToS;	// 계산 비용
				pItem.costHeuristic = pRoutLinkInfo->LinkDistToS;	// 가중치 계산 비용
				pItem.depth = 0;	// 탐색 깊이
				pItem.visited = false; // 방문 여부
				pItem.dir = candidateId.parents_id; // 탐색방향
													 // 링크 방문 목록 등록
				vtCandidateInfo.emplace_back(pItem);
			} // for
		}
	}

	// e 노드에서의 진입은 역방향
	if (pRoutLinkInfo->LinkDir == 0 || (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 2)) && (pRoutLinkInfo->LinkDir == 2))) // 양방향 or 역방향
	{
		// E 노드가 단점 아닌지 확인
		if (pENode && pENode->base.point_type != 2 && pENode->base.connnode_count >= 2) {
			// 노드에서 진입 가능한 모든 경우 확인
			for (int ii = 0; ii < pENode->base.connnode_count; ii++) {
				// 자신은 제외
				if (pLink->link_id == pENode->connnodes[ii]) {
					continue;
				}

				if (getPrevPassCode(pLink->link_id, pENode->connnodes[ii], pENode) == false) {
					continue;
				}

				KeyID candidateId;
				candidateId.parents_id = pENode->connnodes[ii].dir; // 시작 링크의 방향
				candidateId.current_id = pLink->link_id.nid;

				CandidateLink pItem;
				pItem.candidateId = candidateId; // 후보 ID
				pItem.parentId.llid = candidateId.parents_id;	// 부모 링크 ID
				pItem.linkId = pLink->link_id;  // 링크 ID
				pItem.nodeId = pLink->enode_id;	// 노드 ID
				pItem.distReal = pRoutLinkInfo->LinkDistToS; // 실 거리
				pItem.timeReal = GetCost(pLink, routeOpt, pRoutLinkInfo->LinkDistToS, 0); // 실 주행 시간
				pItem.costTreavel = pRoutLinkInfo->LinkDistToS;	// 계산 비용
				pItem.costHeuristic = pRoutLinkInfo->LinkDistToS;	// 가중치 계산 비용
				pItem.depth = 0;	// 탐색 깊이
				pItem.visited = false; // 방문 여부
				pItem.dir = candidateId.parents_id; // 탐색방향
													 // 링크 방문 목록 등록
				vtCandidateInfo.emplace_back(pItem);
			} // for
		}
	}

	return ROUTE_RESULT_SUCCESS;
}


#if defined(USE_TSP_MODULE)
const int CRoutePlan::MakeTabulate(IN const vector<RouteLinkInfo>& linkInfos, OUT RouteTable** ppResultTables)
{
	uint32_t ret = ROUTE_RESULT_SUCCESS;
	stLinkInfo* pLink = nullptr;
	stNodeInfo* pSNode = nullptr;
	stNodeInfo* pENode = nullptr;

	// 종료 지점의 탐색 매칭 카운트 계산
	uint32_t cntGoal = 0; // 목적지 도달 가능한 링크 수 
	uint32_t cntGoalTouch = 0; // 목적지 도달한 링크 수
	CandidateLink checkFinishRouteDir[2];// = { 0, }; // 최대 링크 양단에서 한번씩만 허용
	checkFinishRouteDir[0].costTreavel = DBL_MAX;
	checkFinishRouteDir[1].costTreavel = DBL_MAX;

	uint32_t cntRows = linkInfos.size();
	uint32_t cntCols = cntRows;
	if (cntRows <= 2) {
		LOG_TRACE(LOG_ERROR, "Failed, table rows too short, rows : %d", cntRows);
		return ROUTE_RESULT_FAILED;
	}

	// 경탐 옵션
	int32_t routeOpt = 0;

	// 경탐 확장 시 목적지 도달 확인 용
	unordered_map<uint64_t, int32_t> checkDestination;


	// 지점 개수 만큼의 출발지 테이블 생성
	TableBaseInfo* baseTablesStart = new TableBaseInfo[cntRows];
	// 지점 개수 만큼의 목적지 테이블 생성
	TableBaseInfo* baseTablesEnd = new TableBaseInfo[cntRows];
	
	// 지점 개수 만큼의 결과 테이블(n * n) 생성
	//RouteTable** resultTables = new RouteTable*[cntRows];

	size_t tickTableStart = TICK_COUNT();

	// 기본 테이블 정보 생성
	for (int ii = 0; ii < cntRows; ii++) {
		TableBaseInfo* pBase = nullptr;

		//resultTables[ii] = new RouteTable[cntCols];


		// 시작 정보 등록
		pBase = &baseTablesStart[ii];
		pBase->routeLinkInfo = linkInfos[ii];

#if defined(USE_TREKKING_DATA)
		pLink = m_pDataMgr->GetWLinkDataById(linkInfos[ii].LinkId);
		if (pLink == nullptr) {
			pLink = m_pDataMgr->GetLinkDataById(linkInfos[ii].LinkId);
		}
#elif defined(USE_PEDESTRIAN_DATA)
		pLink = m_pDataMgr->GetWLinkDataById(linkInfos[ii].LinkId);
#elif defined(USE_VEHICLE_DATA)
		pLink = m_pDataMgr->GetVLinkDataById(linkInfos[ii].LinkId);
#else
		pLink = m_pDataMgr->GetLinkDataById(linkInfos[ii].LinkId);
#endif

		if (!pLink)
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find start link, tile:%d, link:%d", linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);
			ret = ROUTE_RESULT_FAILED_READ_DATA;
			break;
		}

		// 출발지 시작 갯수 예약
		CheckStartDirectionMaching(pLink, &linkInfos[ii], routeOpt, pBase->vtCandidateLink);

		if (pBase->vtCandidateLink.empty()) {
			LOG_TRACE(LOG_WARNING, "Failed, can't find start link matching info, tile:%d, link:%d", linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);
			ret = ROUTE_RESULT_FAILED_SET_START;
			break;
		}



		// 종료점 링크의 도착정보
		pBase = &baseTablesEnd[ii];
		pBase->routeLinkInfo = linkInfos[ii];

#if defined(USE_TREKKING_DATA)
		pLink = m_pDataMgr->GetWLinkDataById(linkInfos[ii].LinkId);
		if (pLink == nullptr) {
			pLink = m_pDataMgr->GetLinkDataById(linkInfos[ii].LinkId);
		}
#elif defined(USE_PEDESTRIAN_DATA)
		pLink = m_pDataMgr->GetWLinkDataById(linkInfos[ii].LinkId);
#elif defined(USE_VEHICLE_DATA)
		pLink = m_pDataMgr->GetVLinkDataById(linkInfos[ii].LinkId);
#else
		pLink = m_pDataMgr->GetLinkDataById(linkInfos[ii].LinkId);
#endif	
		if (!pLink)
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find end link, tile:%d, link:%d", linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);
			ret = ROUTE_RESULT_FAILED_READ_DATA;
			break;
		}

		// s 노드에서의 진입은 정방향
#if defined(USE_TREKKING_DATA)
		pSNode = m_pDataMgr->GetWNodeDataById(pLink->snode_id);
		if (pSNode == nullptr) {
			pSNode = m_pDataMgr->GetNodeDataById(pLink->snode_id);
		}
#elif defined(USE_PEDESTRIAN_DATA)
		pSNode = m_pDataMgr->GetWNodeDataById(pLink->snode_id);
#elif defined(USE_VEHICLE_DATA)
		pSNode = m_pDataMgr->GetVNodeDataById(pLink->snode_id);
#else
		pSNode = m_pDataMgr->GetNodeDataById(pLink->snode_id);
#endif

		// e 노드에서의 진입은 역방향
#if defined(USE_TREKKING_DATA)
		pENode = m_pDataMgr->GetWNodeDataById(pLink->enode_id);
		if (pENode == nullptr) {
			pENode = m_pDataMgr->GetNodeDataById(pLink->enode_id);
		}
#elif defined(USE_PEDESTRIAN_DATA)
		pENode = m_pDataMgr->GetWNodeDataById(pLink->enode_id);
#elif defined(USE_VEHICLE_DATA)
		pENode = m_pDataMgr->GetVNodeDataById(pLink->enode_id);
#else
		pENode = m_pDataMgr->GetNodeDataById(pLink->enode_id);
#endif

		// 목적지 도달 갯수 예약
		CheckEndDirectionMaching(pLink, pSNode, pENode, &linkInfos[ii], routeOpt, pBase->vtCandidateLink);

		if (pBase->vtCandidateLink.empty()) {
			LOG_TRACE(LOG_WARNING, "Failed, can't find end link matching info, tile:%d, link:%d", linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);
			ret = ROUTE_RESULT_FAILED_SET_END;
			break;
		}

		// 목적지 도달 체크 데이터 추가
		//// 방향성 체크
		//for (const auto& item : pItem->routeInfo.vtCandidateResult) {
		//	checkDestination.emplace(item->linkId.llid, ii);
		//}
		checkDestination.emplace(pLink->link_id.llid, ii);
		LOG_TRACE(LOG_DEBUG, "Set destination link info, idx:%d, cnt:%d, tile:%d, link:%d", ii, pBase->vtCandidateLink.size(), linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);
	}


	if (ret != ROUTE_RESULT_SUCCESS) {
		return ret;
	}


	// 테이블 각 행(rows)에 지점 정보(cols) 배치
#if defined(USE_MULTIPROCESS)
	volatile bool flag = false;
#endif

#pragma omp parallel for
	for (int ii = 0; ii < cntRows; ii++) 
	{
#if defined(USE_MULTIPROCESS)
		if (flag) continue;
#endif		

		// 경로 탐색 시작
		LOG_TRACE(LOG_DEBUG, "Start find goal table rows[%02d]", ii);

		TableBaseInfo* pBase = &baseTablesEnd[ii];

		// 시작 시각
		pBase->tickStart = TICK_COUNT();

		// 목적지 정보를 기본 방문지 테이블로부터 복사
		for (int jj = 0; jj < cntCols; jj++) {
			if (ii == jj) {
				ppResultTables[ii][jj].nUsable = -1; // 자신은 패스
			}
			else {
				ppResultTables[ii][jj].nUsable = 0; // 미방문
			}

			ppResultTables[ii][jj].linkInfo = baseTablesEnd[jj].routeLinkInfo;
		} // for cols
		// memcpy(resultTables[ii], baseTables, sizeof(RouteTable) * cntRows);



		// 시작 지점 정보 등록
		for (auto & item : baseTablesStart[ii].vtCandidateLink) {
			CandidateLink* pNew = new CandidateLink();
			memcpy(pNew, &item, sizeof(item));
			pBase->pqDijkstra.emplace(pNew);
		}


		CandidateLink* pCurInfo;
		static const uint32_t cntMaxExtraSearch = 100; // 목적지 추가 도달 검사 최대 허용치
		uint32_t cntExtraSearch = 0; // 목적지 도달 후 추가 도달 검사 카운트

		uint32_t cntDestination = checkDestination.size() - 1; // 자신은 제외
		uint32_t cntGoal = 0;
		uint32_t cntExtraGoal = 0;

		RouteTable* pCell = nullptr;
		unordered_map<uint64_t, int32_t>::const_iterator it;

		while (!pBase->pqDijkstra.empty())
		{
			pCurInfo = pBase->pqDijkstra.top();
			pCurInfo->visited = true;

			// 현재 링크 트리에서 제거
			pBase->pqDijkstra.pop();

#if defined(USE_SHOW_ROUTE_SATATUS)
			//LOG_TRACE(LOG_DEBUG, "Current Link : id:%lld(real cost:%f, heuristic cost:%f, lv:%d)", curInfo.linkId, curInfo.costReal, curInfo.costHeuristic, curInfo.depth);

			// 현재까지 탐색된 정보 전달, Function Call
			if (m_fpRoutingStatus) {
				m_fpRoutingStatus(m_pHost, &pRouteInfo->mRoutePass);
			}
#endif


			//if (((pCurInfo->linkId.llid == pRouteInfo->EndLinkInfo.LinkId.llid) && ((pRouteInfo->EndLinkInfo.LinkDir == 0) || (pCurInfo->dir == pRouteInfo->EndLinkInfo.LinkDir))) ||
			if ((it = checkDestination.find(pCurInfo->linkId.llid)) != checkDestination.end())
			{
				pCell = &ppResultTables[ii][it->second];

				// 자기 자신은 패스
				if (pCell->nUsable < 0) {
					;
				}
				// 같은 링크인지 확인
				else if (it->first == pCell->linkInfo.LinkId.llid)
				{					
					// 방향성은 확인하지 말자

					pCell->nUsable++;


					// 첫방문
					if (pCell->nUsable <= 1) {
						cntGoal++;
						cntExtraGoal++;

						pCell->nTotalTime = pCurInfo->timeReal;
						pCell->nTotalDist = pCurInfo->distReal;

						// 경로 탐색 성공
						LOG_TRACE(LOG_DEBUG, "Find route table item, %d/%d, table[%d][%d], time:%d, dist:%d",
							cntGoal, cntDestination, ii, it->second, pCell->nTotalTime, pCell->nTotalDist);
					}
					// 재방문
					else { // if (pCell->nUsable <= 2) {
						cntExtraGoal++;

						// 짧은 녀석 사용
						if ((pCell->nTotalTime > pCurInfo->timeReal) || (pCell->nTotalDist > pCurInfo->distReal)) {
							pCell->nTotalTime = pCurInfo->timeReal;
							pCell->nTotalDist = pCurInfo->distReal;

							// 경로 재 정리
							LOG_TRACE(LOG_DEBUG, "Change route table item, %d/%d, table[%d][%d], time:%d, dist:%d",
								cntGoal, cntDestination, ii, it->second, pCell->nTotalTime, pCell->nTotalDist);
						}
					}
				}
				else {
					// 여기로 들어오느다는건 목적지 테이블이 아예 잘못되었다는건데, 이건 있어서는 안되는 거야
					LOG_TRACE(LOG_WARNING, "Failed, destination table info not correct, table[%d][%d] should have llid:%lld, but now llid:%lld (tile:%d, id:%d)",
						ii, it->second, it->first, pCell->linkInfo.LinkId.llid, pCell->linkInfo.LinkId.tile_id, pCell->linkInfo.LinkId.nid);
					ret = ROUTE_RESULT_FAILED_SET_MEMORY;
					break;
				}
			}

			// 모든 방문지를 다 검색했으면
			if (cntDestination <= cntGoal)
			{
				// 경로 탐색 성공
				LOG_TRACE(LOG_DEBUG, "Success make table[%d], rows:%d/%d, goal:%d/%d +%d", ii, ii + 1, cntRows, cntGoal, cntDestination, cntExtraGoal);
				ret = ROUTE_RESULT_SUCCESS;
				break;
			}
			else {
				// 단순 확장
				Propagation(pBase, pCurInfo, pCurInfo->dir);
			}

			// 연결 노드의 링크 셋
#if defined(USE_SHOW_ROUTE_SATATUS)
			//LOG_TRACE(LOG_DEBUG, "Add new tree : ");

			//LOG_TRACE(LOG_DEBUG, "Visited Link : ");
			//for (map<uint32_t, Blob>::iterator it = mRoute.begin(); it != mRoute.end(); it++)
			//{
			//	LOG_TRACE(LOG_DEBUG, "id:%d(%d) ", it->second.linkId, it->second.cost);
			//}
#endif
		} // while

		// 종료 시각
		pBase->tickFinish = TICK_COUNT();


		if (cntDestination != cntGoal) {
			LOG_TRACE(LOG_DEBUG, "Failed, can't make table, rows[%02d], result:%d/%d, tick:%lld", ii, cntGoal, cntDestination, pBase->tickFinish - pBase->tickStart);
			ret = ROUTE_RESULT_FAILED_EXPEND;
#if defined(USE_MULTIPROCESS)
			flag = true;
			continue;
#else
			break;
#endif	
		}

		// release mem
		// 테이블 행이 완성되면, 행 확장에 사용된 메모리 정리
		//SAFE_DELETE(pBase);
		//if (pBase) {
		//	delete pBase;
		//	pBase = nullptr;
		//}

		
	} // for rows


	size_t tickTable = TICK_COUNT() - tickTableStart;

	
	if (ret != ROUTE_RESULT_SUCCESS) {
		LOG_TRACE(LOG_DEBUG, "Failed, can't find route, couldn't leached end point(the point will isolated)");
	}
	else {
		// print result table

		// 경로 탐색 성공
		// print table rows
		static const int width = 8;
		string strRowsValue;
		char szBuff[1024] = { 0, };

		if (1)
		{
			//======================
			// print top
			strRowsValue.clear();
			memset(szBuff, 0x00, sizeof(szBuff));
			memset(szBuff, '=', (width * (cntCols + 2)) * sizeof(char));
			strRowsValue.append(szBuff);
			LOG_TRACE(LOG_DEBUG, "%s", strRowsValue.c_str());

			// print colums index
			strRowsValue.clear();
			sprintf(szBuff, "       |");
			strRowsValue.append(szBuff);

			for (int cols = 0; cols < cntCols; cols++) {
				sprintf(szBuff, "%4d   |", cols);
				strRowsValue.append(szBuff);
			} // for

			sprintf(szBuff, "  tick |");
			strRowsValue.append(szBuff);
			LOG_TRACE(LOG_DEBUG, "%s", strRowsValue.c_str());

			// print top
			strRowsValue.clear();
			memset(szBuff, 0x00, sizeof(szBuff));
			memset(szBuff, '-', (width * (cntCols + 2)) * sizeof(char));
			strRowsValue.append(szBuff);
			LOG_TRACE(LOG_DEBUG, "%s", strRowsValue.c_str());

			//======================
			// print rows
			for (int rows = 0; rows < cntRows; rows++) {
				strRowsValue.clear();

				// print rows index
				sprintf(szBuff, "%4d   |", rows);
				strRowsValue.append(szBuff);

				// print rows data
				for (int cols = 0; cols < cntCols; cols++) {
					if (rows == cols) {
						sprintf(szBuff, "%6d |", 0);
					}
					else {
						sprintf(szBuff, "%6d |", ppResultTables[rows][cols].nTotalDist);
					}
					strRowsValue.append(szBuff);
				} // for

				// print result tick
				sprintf(szBuff, "%6d |", baseTablesEnd[rows].tickFinish - baseTablesEnd[rows].tickStart);
				strRowsValue.append(szBuff);

				LOG_TRACE(LOG_DEBUG, "%s", strRowsValue.c_str());
			} // for



			//======================
			// print bottom
			strRowsValue.clear();
			memset(szBuff, 0x00, sizeof(szBuff));
			memset(szBuff, '-', (width * (cntCols + 2)) * sizeof(char));
			strRowsValue.append(szBuff);
			LOG_TRACE(LOG_DEBUG, "%s", strRowsValue.c_str());
		} // print table

		if (1) {
			// print summary
			strRowsValue.clear();
			sprintf(szBuff, " total |");
			strRowsValue.append(szBuff);

			memset(szBuff, 0x00, sizeof(szBuff));
			memset(szBuff, ' ', (width * (cntCols - 3)) * sizeof(char));
			strRowsValue.append(szBuff);

			// process
#if defined(USE_MULTIPROCESS)
			sprintf(szBuff, "%8d cores |", omp_get_max_threads() /*omp_get_num_threads()*/);
#else
			sprintf(szBuff, "%8d cores |", PROCESS_NUM);
#endif
			strRowsValue.append(szBuff);

			// time
			sprintf(szBuff, "%11d ms |", tickTable);
			strRowsValue.append(szBuff);
			LOG_TRACE(LOG_DEBUG, "%s", strRowsValue.c_str());

			// print bottom
			strRowsValue.clear();
			memset(szBuff, 0x00, sizeof(szBuff));
			memset(szBuff, '=', (width * (cntCols + 2)) * sizeof(char));
			strRowsValue.append(szBuff);
			LOG_TRACE(LOG_DEBUG, "%s", strRowsValue.c_str());
		}
	}


	// release mem	
	if (baseTablesStart) {
		SAFE_DELETE_ARR(baseTablesStart);
	}

	if (baseTablesEnd) {
		//// 생성된 candidate 데이터의 최종 누적 위치에서 삭제 --> doroute는 pass에서 알아서 삭제, table은 pass 안쓰니까 따로 삭제하자
		//CandidateLink* pInfo;
		//for (int ii = 0; ii < cntRows; ii++) {
		//	for (; !baseTablesEnd[ii].routeInfo.pqDijkstra.empty();) {
		//		pInfo = baseTablesEnd[ii].routeInfo.pqDijkstra.top(); baseTablesEnd[ii].routeInfo.pqDijkstra.pop();
		//		SAFE_DELETE(pInfo);
		//	}
		//}

		SAFE_DELETE_ARR(baseTablesEnd);
	}

	//if (resultTables) {
	//	for (int ii = 0; ii < cntRows; ii++) {
	//		SAFE_DELETE_ARR(resultTables[ii]);
	//	}
	//	SAFE_DELETE_ARR(resultTables);
	//}

	return ret;
}
#endif // #if defined(USE_TSP_MODULE)


const int CRoutePlan::MakeRouteResult(IN RouteInfo* pRouteInfo, OUT RouteResultInfo* pRouteResult)
{
	if (pRouteInfo == nullptr || pRouteResult == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, pRouteInfo:%p, pRouteResult:%p", pRouteInfo, pRouteResult);

		return ROUTE_RESULT_FAILED;
	}
	
	pRouteResult->ResultCode = ROUTE_RESULT_SUCCESS;

	pRouteResult->RequestMode = pRouteInfo->RequestMode;
	pRouteResult->RequestId = pRouteInfo->RequestId;

	pRouteResult->RouteBox.Xmin = pRouteResult->RouteBox.Ymin = INT_MAX;
	pRouteResult->RouteBox.Xmax = pRouteResult->RouteBox.Ymax = INT_MIN;

	const size_t cntLink = pRouteInfo->vtCandidateResult.size();
	pRouteResult->LinkInfo.reserve(cntLink);

	float remainDist = 0.f;
	float remainTime = 0.f;

	// 전체
	pRouteResult->TotalLinkCount = cntLink;

	// 경로 옵션
	pRouteResult->RouteOption = pRouteInfo->RouteOption;
	pRouteResult->RouteAvoid = pRouteInfo->AvoidOption;

	// 경탐 좌표
	memcpy(&pRouteResult->StartResultLink.Coord, &pRouteInfo->StartLinkInfo.Coord, sizeof(pRouteInfo->StartLinkInfo.Coord));
	memcpy(&pRouteResult->EndResultLink.Coord, &pRouteInfo->EndLinkInfo.Coord, sizeof(pRouteInfo->EndLinkInfo.Coord));



	// 시작 끝 링크가 동일 링크이고, 주행 방향성이 동일한 경우
	if (cntLink <= 2 &&
		pRouteInfo->StartLinkInfo.LinkId.llid == pRouteInfo->EndLinkInfo.LinkId.llid &&
		pRouteInfo->StartLinkInfo.LinkDir == pRouteInfo->EndLinkInfo.LinkDir)
	{
		// 시작 링크
		CandidateLink* pCandidateStartLink = pRouteInfo->vtCandidateResult.at(cntLink - 1);

		// 시작점
		pRouteResult->StartResultLink.LinkId = pRouteInfo->StartLinkInfo.LinkId;
		memcpy(&pRouteResult->StartResultLink.Coord, &pRouteInfo->StartLinkInfo.MatchCoord, sizeof(pRouteInfo->StartLinkInfo.MatchCoord));
		pRouteResult->StartResultLink.LinkSplitIdx = pRouteInfo->StartLinkInfo.LinkSplitIdx;

		// 종료점
		pRouteResult->EndResultLink.LinkId = pRouteInfo->EndLinkInfo.LinkId;
		memcpy(&pRouteResult->EndResultLink.MatchCoord, &pRouteInfo->EndLinkInfo.MatchCoord, sizeof(pRouteInfo->EndLinkInfo.MatchCoord));
		pRouteResult->EndResultLink.LinkSplitIdx = pRouteInfo->EndLinkInfo.LinkSplitIdx;


		// 시작 vtx
		// s노드로 나간 경우, 역
		if (pRouteInfo->StartLinkInfo.LinkDir == 2)
		{
			pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToS - pRouteInfo->EndLinkInfo.LinkDistToS;
			//pRouteResult->StartResultLink.LinkVtx.push_back(pRouteInfo->StartLinkInfo.LinkVtxToS[0]);
			pRouteResult->LinkVertex.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToS[0]);
		}
		// e노드로 나간 경우, 정
		else //if (pRouteInfo->StartLinkInfo.LinkDir == 1)
		{
			pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE - pRouteInfo->EndLinkInfo.LinkDistToE;
			//pRouteResult->StartResultLink.LinkVtx.push_back(pRouteInfo->StartLinkInfo.LinkVtxToE[0]);
			pRouteResult->LinkVertex.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToE[0]);
		}


		// 중간 vtx
		if (pRouteResult->StartResultLink.LinkSplitIdx != pRouteInfo->EndLinkInfo.LinkSplitIdx) {
			for (int ii = pRouteResult->StartResultLink.LinkSplitIdx + 1; ii < pRouteInfo->EndLinkInfo.LinkSplitIdx; ii++) {
				if (pRouteInfo->StartLinkInfo.LinkDir == 2)
				{
					//pRouteResult->StartResultLink.LinkVtx.push_back(pRouteInfo->StartLinkInfo.LinkVtxToS[ii]);
					pRouteResult->LinkVertex.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToS[ii]);
				}
				// e노드로 나간 경우, 정
				else //if (pRouteInfo->StartLinkInfo.LinkDir == 1)
				{
					//pRouteResult->StartResultLink.LinkVtx.push_back(pRouteInfo->StartLinkInfo.LinkVtxToE[ii]);
					pRouteResult->LinkVertex.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToE[ii]);
				}
			} // for
		}


		// 종료 vtx
		// e노드로 들어온 경우
		if (pRouteInfo->EndLinkInfo.LinkDir == 2)
		{
			pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToE - pRouteInfo->StartLinkInfo.LinkDistToE;

			// 진행과 반대방향의 시작 vtx
			pRouteResult->LinkVertex.emplace_back(pRouteInfo->EndLinkInfo.LinkVtxToE[pRouteInfo->EndLinkInfo.LinkVtxToE.size() - 1]);
		}

		// s노드로 들어온 경우
		else //if (pRouteInfo->EndLinkInfo.LinkDir == 1)
		{
			pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToS - pRouteInfo->StartLinkInfo.LinkDistToS;

			// 진행과 반대방향의 시작 vtx
			pRouteResult->LinkVertex.emplace_back(pRouteInfo->EndLinkInfo.LinkVtxToS[pRouteInfo->EndLinkInfo.LinkVtxToS.size() - 1]);
		}


		//linkMerge(pRouteResult->LinkVertex, pRouteResult->StartResultLink.LinkVtx);


		RouteResultLinkEx currentLinkInfo = { 0, };
#if defined(USE_TREKKING_DATA)
		stLinkInfo* pLink = m_pDataMgr->GetWLinkDataById(pRouteResult->StartResultLink.LinkId);
		if (pLink == nullptr) {
			pLink = m_pDataMgr->GetLinkDataById(pRouteResult->StartResultLink.LinkId);
		}
#elif defined(USE_PEDESTRIAN_DATA)
		stLinkInfo* pLink = m_pDataMgr->GetWLinkDataById(pRouteResult->StartResultLink.LinkId);
#elif defined(USE_VEHICLE_DATA)
		stLinkInfo* pLink = m_pDataMgr->GetVLinkDataById(pRouteResult->StartResultLink.LinkId);
#else
		stLinkInfo* pLink = m_pDataMgr->GetLinkDataById(pRouteResult->StartResultLink.LinkId);
#endif


		CandidateLink* pCandidateEndLink = pRouteInfo->vtCandidateResult.at(0);

		currentLinkInfo.link_id = pRouteResult->StartResultLink.LinkId;
		currentLinkInfo.node_id.llid = pLink->enode_id.llid;
		currentLinkInfo.link_info = pLink->sub_info;
		currentLinkInfo.length = pRouteResult->StartResultLink.LinkDist;
		currentLinkInfo.time = min(1, (int)(pCandidateEndLink->timeReal * (pRouteResult->EndResultLink.LinkDist / pCandidateEndLink->distReal))); //currentLinkInfo.time = pCandidateEndLink->timeReal; // 종료 링크시간은 사용된 거리대비 시간만큼만 사용.
		currentLinkInfo.vtx_off = pRouteResult->StartResultLink.LinkSplitIdx;
		currentLinkInfo.vtx_cnt = pRouteResult->LinkVertex.size(); // pRouteInfo->EndLinkInfo.LinkSplitIdx - pRouteResult->StartResultLink.LinkSplitIdx + 1;
		//currentLinkInfo.rlength = currentLinkInfo.length;
		//currentLinkInfo.rtime = currentLinkInfo.time;
		currentLinkInfo.angle = 0;
		currentLinkInfo.dir = pCandidateEndLink->dir == 1 ? 0 : 1;


		pRouteResult->LinkInfo.emplace_back(currentLinkInfo);

		pRouteResult->TotalLinkDist += currentLinkInfo.length;
		pRouteResult->TotalLinkTime += currentLinkInfo.time;
	}
	else
	{
		//
		// 시작 링크
		CandidateLink* pCandidateStartLink = pRouteInfo->vtCandidateResult.at(cntLink - 1);

		pRouteResult->StartResultLink.LinkId = pRouteInfo->StartLinkInfo.LinkId;
		memcpy(&pRouteResult->StartResultLink.Coord, &pRouteInfo->StartLinkInfo.MatchCoord, sizeof(pRouteInfo->StartLinkInfo.MatchCoord));
		pRouteResult->StartResultLink.LinkSplitIdx = pRouteInfo->StartLinkInfo.LinkSplitIdx;
		// s노드로 나간 경우, 역
		if (pCandidateStartLink->dir == 2)
		{
			pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToS;
			pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToS.begin(), pRouteInfo->StartLinkInfo.LinkVtxToS.end());
		}
		// e노드로 나간 경우, 정
		else if (pCandidateStartLink->dir == 1)
		{
			pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE;
			pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToE.begin(), pRouteInfo->StartLinkInfo.LinkVtxToE.end());
		}
		else
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't find route start link, link:%d, dir:%d", pCandidateStartLink->linkId.nid, pCandidateStartLink->linkId.dir);
			return -1;
		}

		// 링크 진행 각도 계산용 정/역
		int prevDir = 0;
		int nextDir = 0;

		RouteResultLinkEx currentLinkInfo = { 0, };
#if defined(USE_TREKKING_DATA)
		stLinkInfo* pLink = m_pDataMgr->GetWLinkDataById(pRouteResult->StartResultLink.LinkId);
		if (pLink == nullptr) {
			pLink = m_pDataMgr->GetLinkDataById(pRouteResult->StartResultLink.LinkId);
		}
#elif defined(USE_PEDESTRIAN_DATA)
		stLinkInfo* pLink = m_pDataMgr->GetWLinkDataById(pRouteResult->StartResultLink.LinkId);
#elif defined(USE_VEHICLE_DATA)
		stLinkInfo* pLink = m_pDataMgr->GetVLinkDataById(pRouteResult->StartResultLink.LinkId);
#else
		stLinkInfo* pLink = m_pDataMgr->GetLinkDataById(pRouteResult->StartResultLink.LinkId);
#endif

		if (pLink == nullptr) {
			LOG_TRACE(LOG_ERROR, "Failed, can't find route start link(%d)", pRouteResult->StartResultLink.LinkId.nid);
			return -1;
		}
		else if (pCandidateStartLink->linkId.nid != pLink->link_id.nid) {
			LOG_TRACE(LOG_ERROR, "Failed, matched start link not same with result start, smlink:%d != srlink:%d", pCandidateStartLink->linkId.nid, pLink->link_id.nid);
			return -1;
		}

		currentLinkInfo.link_id = pCandidateStartLink->linkId;
		currentLinkInfo.node_id = pCandidateStartLink->nodeId;
		currentLinkInfo.link_info = pLink->sub_info;
		currentLinkInfo.length = pRouteResult->StartResultLink.LinkDist; // pCandidateStartLink->distReal; 
		currentLinkInfo.time = pCandidateStartLink->timeReal;
		currentLinkInfo.vtx_off += currentLinkInfo.vtx_cnt;
		currentLinkInfo.vtx_cnt = pRouteResult->StartResultLink.LinkVtx.size();
		//currentLinkInfo.rlength = remainDist;
		//currentLinkInfo.rtime = remainTime;

		// angle
		if (pCandidateStartLink->dir == 1) { // 정 to enode
											 // 주행 각도는 링크 시작값으로 주기에 최초 링크는 각도 무시.
			currentLinkInfo.angle = 0;
			currentLinkInfo.dir = 0;
			prevDir = pLink->base.enode_ang;
		}
		else { // 역 to snode
			   // 주행 각도는 링크 시작값으로 주기에 최초 링크는 각도 무시.
			currentLinkInfo.angle = 0;
			currentLinkInfo.dir = 1;
			prevDir = pLink->base.snode_ang;
		}

		pRouteResult->LinkInfo.emplace_back(currentLinkInfo);
		linkMerge(pRouteResult->LinkVertex, pRouteResult->StartResultLink.LinkVtx);

		pRouteResult->TotalLinkDist += currentLinkInfo.length;
		pRouteResult->TotalLinkTime += currentLinkInfo.time;


		// 
		// 중간 링크
		CandidateLink* pCandidateLink = nullptr;

		for (int ii = cntLink - 2; ii > 0; --ii)
		{
			pCandidateLink = pRouteInfo->vtCandidateResult.at(ii);

#if defined(USE_TREKKING_DATA)
			pLink = m_pDataMgr->GetWLinkDataById(pCandidateLink->linkId);
			if (pLink == nullptr) {
				pLink = m_pDataMgr->GetLinkDataById(pCandidateLink->linkId);
			}
#elif defined(USE_PEDESTRIAN_DATA)
			pLink = m_pDataMgr->GetWLinkDataById(pCandidateLink->linkId);
#elif defined(USE_VEHICLE_DATA)
			pLink = m_pDataMgr->GetVLinkDataById(pCandidateLink->linkId);
#else
			pLink = m_pDataMgr->GetLinkDataById(pCandidateLink->linkId);
#endif

			if (pLink == nullptr) {
				LOG_TRACE(LOG_ERROR, "Failed, can't find route mid link(%d)", ii);
				return -1;
			}

			currentLinkInfo.link_id = pCandidateLink->linkId;
			currentLinkInfo.node_id = pCandidateLink->nodeId;
			currentLinkInfo.link_info = pLink->sub_info;
			currentLinkInfo.length = pCandidateLink->distReal;
			currentLinkInfo.time = pCandidateLink->timeReal;
			currentLinkInfo.vtx_off += currentLinkInfo.vtx_cnt;
			currentLinkInfo.vtx_cnt = pLink->getVertexCount();
			//currentLinkInfo.rlength = remainDist;
			//currentLinkInfo.rtime = remainTime;

			// angle
			if (pCandidateLink->dir == 1) { // 정, to enode
				// 주행 각도는 링크 시작값으로 주기에 최초 링크는 각도 무시.
				currentLinkInfo.angle = abs(prevDir - int(pLink->base.enode_ang)) % 360;
				currentLinkInfo.dir = 0;
				prevDir = pLink->base.enode_ang;
			}
			else { // 역, to snode
				   // 주행 각도는 링크 시작값으로 주기에 최초 링크는 각도 무시.
				currentLinkInfo.angle = abs(prevDir - int(pLink->base.snode_ang)) % 360;
				currentLinkInfo.dir = 1;
				prevDir = pLink->base.snode_ang;
			}


			pRouteResult->LinkInfo.emplace_back(currentLinkInfo);
			linkMerge(pRouteResult->LinkVertex, pLink->getVertex(), pLink->getVertexCount());

			pRouteResult->TotalLinkDist += currentLinkInfo.length;
			pRouteResult->TotalLinkTime += currentLinkInfo.time;
		}


		//
		// 종료 링크
		CandidateLink* pCandidateEndLink = pRouteInfo->vtCandidateResult.at(0);

		pRouteResult->EndResultLink.LinkId = pRouteInfo->EndLinkInfo.LinkId;
		memcpy(&pRouteResult->EndResultLink.MatchCoord, &pRouteInfo->EndLinkInfo.MatchCoord, sizeof(pRouteInfo->EndLinkInfo.MatchCoord));
		pRouteResult->EndResultLink.LinkSplitIdx = pRouteInfo->EndLinkInfo.LinkSplitIdx;
		// s노드로 들어온 경우
		//if (pCandidateEndLink->linkId.dir == 1)
		if (pCandidateEndLink->dir == 1)
		{
			pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToS;
			pRouteResult->EndResultLink.LinkVtx.assign(pRouteInfo->EndLinkInfo.LinkVtxToS.begin(), pRouteInfo->EndLinkInfo.LinkVtxToS.end());
		}
		// e노드로 들어온 경우
		/*else if (pCandidateEndLink->linkId.dir == 2)*/
		else if (pCandidateEndLink->dir == 2)
		{
			pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToE;
			pRouteResult->EndResultLink.LinkVtx.assign(pRouteInfo->EndLinkInfo.LinkVtxToE.begin(), pRouteInfo->EndLinkInfo.LinkVtxToE.end());
		}
		else
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't find route end link, link:%d, dir:%d", pCandidateEndLink->linkId.nid, pCandidateEndLink->linkId.dir);
			return -1;
		}

#if defined(USE_TREKKING_DATA)
		pLink = m_pDataMgr->GetWLinkDataById(pRouteResult->EndResultLink.LinkId);
		if (pLink == nullptr) {
			pLink = m_pDataMgr->GetLinkDataById(pRouteResult->EndResultLink.LinkId);
		}
#elif defined(USE_PEDESTRIAN_DATA)
		pLink = m_pDataMgr->GetWLinkDataById(pRouteResult->EndResultLink.LinkId);
#elif defined(USE_VEHICLE_DATA)
		pLink = m_pDataMgr->GetVLinkDataById(pRouteResult->EndResultLink.LinkId);
#else
		pLink = m_pDataMgr->GetLinkDataById(pRouteResult->EndResultLink.LinkId);
#endif
		if (pLink == nullptr) {
			LOG_TRACE(LOG_ERROR, "Failed, can't find route end link(%d)", pRouteResult->EndResultLink.LinkId.nid);
			return -1;
		}
		else if (pCandidateEndLink->linkId.nid != pLink->link_id.nid) {
			LOG_TRACE(LOG_ERROR, "Failed, matched end link not same with result end, emlink:%d != erlink:%d", pCandidateEndLink->linkId.nid, pLink->link_id.nid);
			return -1;
		}

		currentLinkInfo.link_id = pCandidateEndLink->linkId;
		currentLinkInfo.node_id = pCandidateEndLink->nodeId;
		currentLinkInfo.link_info = pLink->sub_info;
		currentLinkInfo.length = pRouteResult->EndResultLink.LinkDist; // pCandidateEndLink->distReal;
		currentLinkInfo.time = min(1, (int)(pCandidateEndLink->timeReal * (pRouteResult->EndResultLink.LinkDist / pCandidateEndLink->distReal))); // currentLinkInfo.time = pCandidateEndLink->timeReal; // 종료 링크시간은 사용된 거리대비 시간만큼만 사용.
		currentLinkInfo.vtx_off += currentLinkInfo.vtx_cnt;
		currentLinkInfo.vtx_cnt = pRouteResult->EndResultLink.LinkVtx.size();
		//currentLinkInfo.rlength = remainDist;
		//currentLinkInfo.rtime = remainTime;

		

		// angle
		if (pCandidateEndLink->dir == 1) { // 정 to enode
			// 주행 각도는 링크 시작값으로 주기에 최초 링크는 각도 무시.
			currentLinkInfo.angle = abs(prevDir - int(pLink->base.enode_ang)) % 360;
			currentLinkInfo.dir = 0;
			prevDir = pLink->base.enode_ang;
		}
		else { // 역 to snode
			   // 주행 각도는 링크 시작값으로 주기에 최초 링크는 각도 무시.
			currentLinkInfo.angle = abs(prevDir - int(pLink->base.snode_ang)) % 360;
			currentLinkInfo.dir = 1;
			prevDir = pLink->base.snode_ang;
		}

		pRouteResult->LinkInfo.emplace_back(currentLinkInfo);
		linkMerge(pRouteResult->LinkVertex, pRouteResult->EndResultLink.LinkVtx);

		pRouteResult->TotalLinkDist += currentLinkInfo.length;
		pRouteResult->TotalLinkTime += currentLinkInfo.time;
	}

	// 남은 거리/시간 정보
	remainDist = pRouteResult->TotalLinkDist;
	remainTime = pRouteResult->TotalLinkTime;

	for (auto & item : pRouteResult->LinkInfo) {
		item.rlength = remainDist;
		item.rtime = remainTime;

		remainDist -= item.length;
		remainTime -= item.time;

		if (remainDist <= 0) {
			remainDist = 0;
		}

		if (remainTime <= 0) {
			remainTime = 0;
		}
	}

	// route box
	for (vector<SPoint>::const_iterator it = pRouteResult->LinkVertex.begin(); it != pRouteResult->LinkVertex.end(); it++)
	{
		if (it->x < pRouteResult->RouteBox.Xmin || pRouteResult->RouteBox.Xmin == 0) { pRouteResult->RouteBox.Xmin = it->x; }
		if (pRouteResult->RouteBox.Xmax < it->x || pRouteResult->RouteBox.Xmax == 0) { pRouteResult->RouteBox.Xmax = it->x; }
		if (it->y < pRouteResult->RouteBox.Ymin || pRouteResult->RouteBox.Ymin == 0) { pRouteResult->RouteBox.Ymin = it->y; }
		if (pRouteResult->RouteBox.Ymax < it->y || pRouteResult->RouteBox.Ymax == 0) { pRouteResult->RouteBox.Ymax = it->y; }
	}

	return 0;
}


#if defined(USE_SHOW_ROUTE_SATATUS)
void CRoutePlan::SetRouteStatusFunc(IN const void* pHost, IN void(*fpdrawRouting)(const void*, const unordered_map<uint64_t, CandidateLink*>*))
{
	m_pHost = const_cast<void*>(pHost);
	m_fpRoutingStatus = fpdrawRouting;
}
#endif