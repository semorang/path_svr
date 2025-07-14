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


void routeProcessPrint(const int nProcess)
{
#if defined(_DEBUG)
	static int nProcessGap = 1000;
#else
	static int nProcessGap = 3000;
#endif

	if (nProcess % (nProcessGap * 4) == 0) {
		printf("\rrouting process : /");
	}
	else if (nProcess % (nProcessGap * 3) == 0) {
		printf("\rrouting process : |");
	}
	else if (nProcess % (nProcessGap * 2) == 0) {
		printf("\rrouting process : \\");
	}
	else if (nProcess % (nProcessGap * 1) == 0) {
		printf("\rrouting process : -");
	}
	//printf("\x1b[%dA", 2);	// 2줄위로 이동
	//sleep(1);				// 육안으로 변경내용 확인 위해 추가
	//printf("Hello\n");
	//printf("\x1b[%dB", 2);	// 원래위치(2줄 아래)로 이동(==\n\n 과 같음)

	//printf("\x1b[%dA\r", 1);	// 1줄위로, 해당줄의 가장왼쪽으로(\r)
}


static const int32_t MAX_FOREST_POPULAR = 10;//3072; // 숲길 인기도 최대치
static const int32_t MAX_FOREST_POPULAR_NONE = 100;//4095; // 숲길 인기도 최대치
static const int32_t MAX_LANE_COUNT = 8; // 최대 차선 수

CRoutePlan::CRoutePlan()
{
	memset(m_rpCost.base, 0x00, sizeof(m_rpCost.base));

#if defined(USE_SHOW_ROUTE_SATATUS)	
	m_pHost = nullptr;
	m_fpRoutingStatus = nullptr;
#endif

#if defined(USE_FOREST_DATA)
	// 차선 가중치
	m_rpCost.pedestrian.cost_lane_walk[0] = .0f; // 최단
	m_rpCost.pedestrian.cost_lane_walk[1] = .0f; // 추천
	m_rpCost.pedestrian.cost_lane_walk[2] = .1f; // 편한
	m_rpCost.pedestrian.cost_lane_walk[3] = .0f; // 빠른
	m_rpCost.pedestrian.cost_lane_walk[4] = .05f; // 큰길
	m_rpCost.pedestrian.cost_lane_bike[0] = .0f; // 최단
	m_rpCost.pedestrian.cost_lane_bike[1] = .05f; // 추천
	m_rpCost.pedestrian.cost_lane_bike[2] = .05f; // 편한
	m_rpCost.pedestrian.cost_lane_bike[3] = .0f; // 빠른
	m_rpCost.pedestrian.cost_lane_bike[4] = .05f; // 큰길

	// 회전 가중치
	m_rpCost.pedestrian.cost_angle_walk[0] = .0f; // 최단
	m_rpCost.pedestrian.cost_angle_walk[1] = .5f; // 추천
	m_rpCost.pedestrian.cost_angle_walk[2] = 1.f; // 편한
	m_rpCost.pedestrian.cost_angle_walk[3] = .5f; // 빠른
	m_rpCost.pedestrian.cost_angle_walk[4] = 1.f; // 큰길
	m_rpCost.pedestrian.cost_angle_bike[0] = .0f; // 최단
	m_rpCost.pedestrian.cost_angle_bike[1] = .5f; // 추천
	m_rpCost.pedestrian.cost_angle_bike[2] = 1.f; // 편한
	m_rpCost.pedestrian.cost_angle_bike[3] = .0f; // 빠른
	m_rpCost.pedestrian.cost_angle_bike[4] = .0f; // 큰길

	// 교차로 가중치
	m_rpCost.pedestrian.cost_cross_walk[0] = .0f; // 최단
	m_rpCost.pedestrian.cost_cross_walk[1] = .0f; // 추천
	m_rpCost.pedestrian.cost_cross_walk[2] = .01f; // 편한
	m_rpCost.pedestrian.cost_cross_walk[3] = .0f; // 빠른
	m_rpCost.pedestrian.cost_cross_walk[4] = .0f; // 큰길
	m_rpCost.pedestrian.cost_cross_bike[0] = .0f; // 최단
	m_rpCost.pedestrian.cost_cross_bike[1] = .0f; // 추천
	m_rpCost.pedestrian.cost_cross_bike[2] = .0f; // 편한
	m_rpCost.pedestrian.cost_cross_bike[3] = .0f; // 빠른
	m_rpCost.pedestrian.cost_cross_bike[4] = .0f; // 큰길

	//보행자도로 타입
	m_rpCost.pedestrian.cost_walk_side[0] = .0f; // 복선도로
	m_rpCost.pedestrian.cost_walk_side[1] = .0f;
	m_rpCost.pedestrian.cost_walk_side[2] = .0f;
	m_rpCost.pedestrian.cost_walk_side[3] = .0f;
	m_rpCost.pedestrian.cost_walk_side[4] = .0f;
	m_rpCost.pedestrian.cost_walk_with[0] = .0f; // 차량겸용도로
	m_rpCost.pedestrian.cost_walk_with[1] = .0f;
	m_rpCost.pedestrian.cost_walk_with[2] = .0f;
	m_rpCost.pedestrian.cost_walk_with[3] = .0f;
	m_rpCost.pedestrian.cost_walk_with[4] = .0f;
	m_rpCost.pedestrian.cost_walk_bike[0] = 1.f; // 자전거전용
	m_rpCost.pedestrian.cost_walk_bike[1] = 1.f;
	m_rpCost.pedestrian.cost_walk_bike[2] = 1.f;
	m_rpCost.pedestrian.cost_walk_bike[3] = 1.f;
	m_rpCost.pedestrian.cost_walk_bike[4] = 1.f;
	m_rpCost.pedestrian.cost_walk_only[0] = .0f; // 보행전용
	m_rpCost.pedestrian.cost_walk_only[1] = .0f;
	m_rpCost.pedestrian.cost_walk_only[2] = .0f;
	m_rpCost.pedestrian.cost_walk_only[3] = .0f;
	m_rpCost.pedestrian.cost_walk_only[4] = .0f;
	m_rpCost.pedestrian.cost_walk_line[0] = .0f; // 가상보행
	m_rpCost.pedestrian.cost_walk_line[1] = .0f;
	m_rpCost.pedestrian.cost_walk_line[2] = .0f;
	m_rpCost.pedestrian.cost_walk_line[3] = .0f;
	m_rpCost.pedestrian.cost_walk_line[4] = .0f;

	// 자전거전용
	m_rpCost.pedestrian.cost_bink_only[0] = .0f; // 짧은, 자전거전용
	m_rpCost.pedestrian.cost_bink_only[1] = .0f; // 추천
	m_rpCost.pedestrian.cost_bink_only[2] = .0f; // 편한
	m_rpCost.pedestrian.cost_bink_only[3] = .0f; // 빠른
	m_rpCost.pedestrian.cost_bink_only[4] = .0f; // 큰길
	// 자전거-보행겸용
	m_rpCost.pedestrian.cost_bike_with[0] = .0f;	// 짧은, 자전거-보행겸용
	m_rpCost.pedestrian.cost_bike_with[1] = 3.f;	// 추천
	m_rpCost.pedestrian.cost_bike_with[2] = .5f;	// 편한
	m_rpCost.pedestrian.cost_bike_with[3] = .0f;	// 빠른
	m_rpCost.pedestrian.cost_bike_with[4] = .5f;	// 큰길
	// 보행전용
	m_rpCost.pedestrian.cost_bike_walk[0] = .0f;	// 짧은, 보행전용
	m_rpCost.pedestrian.cost_bike_walk[1] = 4.f;	// 추천
	m_rpCost.pedestrian.cost_bike_walk[2] = .5f;	// 편한
	m_rpCost.pedestrian.cost_bike_walk[3] = .0f;	// 빠른
	m_rpCost.pedestrian.cost_bike_walk[4] = .5f;	// 큰길
#elif defined(USE_PEDESTRIAN_DATA)
#	if 1
	// 차선 가중치
	m_rpCost.pedestrian.cost_lane_walk[0] = .0f; // 최단
	m_rpCost.pedestrian.cost_lane_walk[1] = .05f; // 추천, ex) kvx = 0.f
	m_rpCost.pedestrian.cost_lane_walk[2] = .5f; // 편한
	m_rpCost.pedestrian.cost_lane_walk[3] = .05f; // 빠른
	m_rpCost.pedestrian.cost_lane_walk[4] = 2.f; // 큰길
	m_rpCost.pedestrian.cost_lane_bike[0] = .0f; // 최단
	m_rpCost.pedestrian.cost_lane_bike[1] = .01f; // 추천
	m_rpCost.pedestrian.cost_lane_bike[2] = .05f; // 편한
	m_rpCost.pedestrian.cost_lane_bike[3] = .0f; // 빠른
	m_rpCost.pedestrian.cost_lane_bike[4] = .05f; // 큰길

	// 교차로 가중치
	m_rpCost.pedestrian.cost_cross_walk[0] = .0f; // 최단
	m_rpCost.pedestrian.cost_cross_walk[1] = .0f; // 추천
	m_rpCost.pedestrian.cost_cross_walk[2] = .0f; // 편한
	m_rpCost.pedestrian.cost_cross_walk[3] = .0f; // 빠른
	m_rpCost.pedestrian.cost_cross_walk[4] = .0f; // 큰길
	m_rpCost.pedestrian.cost_cross_bike[0] = .0f; // 최단
	m_rpCost.pedestrian.cost_cross_bike[1] = .0f; // 추천
	m_rpCost.pedestrian.cost_cross_bike[2] = .0f; // 편한
	m_rpCost.pedestrian.cost_cross_bike[3] = .0f; // 빠른
	m_rpCost.pedestrian.cost_cross_bike[4] = .0f; // 큰길

	// 회전 가중치
	m_rpCost.pedestrian.cost_angle_walk[0] = .0f; // 최단
	m_rpCost.pedestrian.cost_angle_walk[1] = .0f; // 추천
	m_rpCost.pedestrian.cost_angle_walk[2] = .5f; // 편한
	m_rpCost.pedestrian.cost_angle_walk[3] = .1f; // 빠른
	m_rpCost.pedestrian.cost_angle_walk[4] = 2.f; // 큰길
	m_rpCost.pedestrian.cost_angle_bike[0] = .0f; // 최단
	m_rpCost.pedestrian.cost_angle_bike[1] = .5f; // 추천
	m_rpCost.pedestrian.cost_angle_bike[2] = 1.f; // 편한
	m_rpCost.pedestrian.cost_angle_bike[3] = .0f; // 빠른
	m_rpCost.pedestrian.cost_angle_bike[4] = .0f; // 큰길

	//걷기 도로타입별 가중치
	m_rpCost.pedestrian.cost_walk_side[0] = .0f; // 복선도로
	m_rpCost.pedestrian.cost_walk_side[1] = .0f;
	m_rpCost.pedestrian.cost_walk_side[2] = .0f;
	m_rpCost.pedestrian.cost_walk_side[3] = .0f;
	m_rpCost.pedestrian.cost_walk_side[4] = .0f;
	m_rpCost.pedestrian.cost_walk_with[0] = .0f; // 차량겸용도로
	m_rpCost.pedestrian.cost_walk_with[1] = .5f;
	m_rpCost.pedestrian.cost_walk_with[2] = 1.f;
	m_rpCost.pedestrian.cost_walk_with[3] = .5f;
	m_rpCost.pedestrian.cost_walk_with[4] = .5f;
	m_rpCost.pedestrian.cost_walk_bike[0] = 1.f; // 자전거전용
	m_rpCost.pedestrian.cost_walk_bike[1] = 1.f;
	m_rpCost.pedestrian.cost_walk_bike[2] = 1.f;
	m_rpCost.pedestrian.cost_walk_bike[3] = 1.f;
	m_rpCost.pedestrian.cost_walk_bike[4] = 1.f;
	m_rpCost.pedestrian.cost_walk_only[0] = .0f; // 보행전용
	m_rpCost.pedestrian.cost_walk_only[1] = .0f;
	m_rpCost.pedestrian.cost_walk_only[2] = .0f;
	m_rpCost.pedestrian.cost_walk_only[3] = .0f;
	m_rpCost.pedestrian.cost_walk_only[4] = .0f;
	m_rpCost.pedestrian.cost_walk_line[0] = .0f; // 가상보행
	m_rpCost.pedestrian.cost_walk_line[1] = .0f;
	m_rpCost.pedestrian.cost_walk_line[2] = .0f;
	m_rpCost.pedestrian.cost_walk_line[3] = .0f;
	m_rpCost.pedestrian.cost_walk_line[4] = .0f;

	// 자전거 주행 도로타입별 가중치 
	m_rpCost.pedestrian.cost_bike_only[0] = .0f; // 짧은, 자전거전용
	m_rpCost.pedestrian.cost_bike_only[1] = .0f; // 추천
	m_rpCost.pedestrian.cost_bike_only[2] = .0f; // 편한
	m_rpCost.pedestrian.cost_bike_only[3] = .0f; // 빠른
	m_rpCost.pedestrian.cost_bike_only[4] = .0f; // 큰길
	m_rpCost.pedestrian.cost_bike_with[0] = .0f;	// 짧은, 자전거-보행겸용
	m_rpCost.pedestrian.cost_bike_with[1] = .5f;	// 추천, ex) kvx = 3.f;
	m_rpCost.pedestrian.cost_bike_with[2] = .5f;	// 편한
	m_rpCost.pedestrian.cost_bike_with[3] = .0f;	// 빠른
	m_rpCost.pedestrian.cost_bike_with[4] = .5f;	// 큰길
	m_rpCost.pedestrian.cost_bike_walk[0] = .0f;	// 짧은, 보행전용
	m_rpCost.pedestrian.cost_bike_walk[1] = 1.f;	// 추천
	m_rpCost.pedestrian.cost_bike_walk[2] = .5f;	// 편한
	m_rpCost.pedestrian.cost_bike_walk[3] = .0f;	// 빠른
	m_rpCost.pedestrian.cost_bike_walk[4] = .5f;	// 큰길
#	else
	// 차선 가중치
	//m_rpCost.pedestrian.cost_lane0 = .0f; // 최단
	//m_rpCost.pedestrian.cost_lane1 = .03f; // 추천
	//m_rpCost.pedestrian.cost_lane2 = .03f; // 편한
	//m_rpCost.pedestrian.cost_lane3 = .04f; // 빠른
	//m_rpCost.pedestrian.cost_lane4 = .1f; // 큰길
	m_rpCost.pedestrian.cost_lane_walk0 = .0f; // 최단
	m_rpCost.pedestrian.cost_lane_walk1 = .03f; // 추천
	m_rpCost.pedestrian.cost_lane_walk2 = .03f; // 편한
	m_rpCost.pedestrian.cost_lane_walk3 = .04f; // 빠른
	m_rpCost.pedestrian.cost_lane_walk4 = .1f; // 큰길 

	m_rpCost.pedestrian.cost_lane_bike0 = .0f; // 최단
	m_rpCost.pedestrian.cost_lane_bike1 = .05f; // 추천
	m_rpCost.pedestrian.cost_lane_bike2 = .05f; // 편한
	m_rpCost.pedestrian.cost_lane_bike3 = .0f; // 빠른
	m_rpCost.pedestrian.cost_lane_bike4 = .05f; // 큰길
	
	// 회전 가중치
	//m_rpCost.pedestrian.cost_angle0 = .0f; // 최단
	//m_rpCost.pedestrian.cost_angle1 = 3.0f; // 추천
	//m_rpCost.pedestrian.cost_angle2 = 3.0f; // 편한
	//m_rpCost.pedestrian.cost_angle3 = 4.0; // 빠른
	//m_rpCost.pedestrian.cost_angle4 = 5.0f; // 큰길
	m_rpCost.pedestrian.cost_angle_walk0 = .0f; // 최단
	m_rpCost.pedestrian.cost_angle_walk1 = 3.0f; // 추천
	m_rpCost.pedestrian.cost_angle_walk2 = 3.0; // 편한
	m_rpCost.pedestrian.cost_angle_walk3 = 4.0; // 빠른
	m_rpCost.pedestrian.cost_angle_walk4 = 5.0; // 큰길

	m_rpCost.pedestrian.cost_angle_bike0 = .0f; // 최단
	m_rpCost.pedestrian.cost_angle_bike1 = .5f; // 추천
	m_rpCost.pedestrian.cost_angle_bike2 = 1.f; // 편한
	m_rpCost.pedestrian.cost_angle_bike3 = .0f; // 빠른
	m_rpCost.pedestrian.cost_angle_bike4 = .0f; // 큰길

	// 교차로 가중치
	m_rpCost.pedestrian.cost_cross_walk0 = .0f; // 최단
	m_rpCost.pedestrian.cost_cross_walk1 = .0f; // 추천
	m_rpCost.pedestrian.cost_cross_walk2 = .0f; // 편한
	m_rpCost.pedestrian.cost_cross_walk3 = .0f; // 빠른
	m_rpCost.pedestrian.cost_cross_walk4 = .0f; // 큰길
	m_rpCost.pedestrian.cost_cross_bike0 = .0f; // 최단
	m_rpCost.pedestrian.cost_cross_bike1 = .0f; // 추천
	m_rpCost.pedestrian.cost_cross_bike2 = .0f; // 편한
	m_rpCost.pedestrian.cost_cross_bike3 = .0f; // 빠른
	m_rpCost.pedestrian.cost_cross_bike4 = .0f; // 큰길

	//보행자도로 타입
	m_rpCost.pedestrian.cost_walk_side0 = .0f; // 복선도로
	m_rpCost.pedestrian.cost_walk_side1 = .0f;
	m_rpCost.pedestrian.cost_walk_side2 = .0f;
	m_rpCost.pedestrian.cost_walk_side3 = .0f;
	m_rpCost.pedestrian.cost_walk_side4 = .0f;
	m_rpCost.pedestrian.cost_walk_with0 = .0f; // 차량겸용도로
	m_rpCost.pedestrian.cost_walk_with1 = .0f;
	m_rpCost.pedestrian.cost_walk_with2 = .0f;
	m_rpCost.pedestrian.cost_walk_with3 = .0f;
	m_rpCost.pedestrian.cost_walk_with4 = .0f;
	m_rpCost.pedestrian.cost_walk_bike0 = 1.f; // 자전거전용
	m_rpCost.pedestrian.cost_walk_bike1 = 1.f;
	m_rpCost.pedestrian.cost_walk_bike2 = 1.f;
	m_rpCost.pedestrian.cost_walk_bike3 = 1.f;
	m_rpCost.pedestrian.cost_walk_bike4 = 1.f;
	m_rpCost.pedestrian.cost_walk_only0 = .0f; // 보행전용
	m_rpCost.pedestrian.cost_walk_only1 = .0f;
	m_rpCost.pedestrian.cost_walk_only2 = .0f;
	m_rpCost.pedestrian.cost_walk_only3 = .0f;
	m_rpCost.pedestrian.cost_walk_only4 = .0f;
	m_rpCost.pedestrian.cost_walk_line0 = .0f; // 가상보행
	m_rpCost.pedestrian.cost_walk_line1 = .0f;
	m_rpCost.pedestrian.cost_walk_line2 = .0f;
	m_rpCost.pedestrian.cost_walk_line3 = .0f;
	m_rpCost.pedestrian.cost_walk_line4 = .0f;

	// 자전거
	//m_rpCost.pedestrian.cost_bicycle00 = 1.f; // 자전거전용
	//m_rpCost.pedestrian.cost_bicycle10 = 1.f;
	//m_rpCost.pedestrian.cost_bicycle20 = 1.f;
	//m_rpCost.pedestrian.cost_bicycle30 = 1.f;
	//m_rpCost.pedestrian.cost_bicycle40 = 1.f;
	m_rpCost.pedestrian.cost_bink_only0 = 1.f; // 짧은, 자전거전용
	m_rpCost.pedestrian.cost_bink_only1 = 1.f; // 추천
	m_rpCost.pedestrian.cost_bink_only2 = 1.f; // 편한
	m_rpCost.pedestrian.cost_bink_only3 = 1.f; // 빠른
	m_rpCost.pedestrian.cost_bink_only4 = 1.f; // 큰길
	// 자전거-보행겸용
	//m_rpCost.pedestrian.cost_bicycle01 = 1.f; // 자전거-보행겸용
	//m_rpCost.pedestrian.cost_bicycle11 = 2.f;
	//m_rpCost.pedestrian.cost_bicycle21 = 2.f;
	//m_rpCost.pedestrian.cost_bicycle31 = 2.f;
	//m_rpCost.pedestrian.cost_bicycle41 = 2.f;
	m_rpCost.pedestrian.cost_bike_with0 = 1.f;	// 짧은, 자전거-보행겸용
	m_rpCost.pedestrian.cost_bike_with1 = 2.f;	// 추천
	m_rpCost.pedestrian.cost_bike_with2 = 2.f;	// 편한
	m_rpCost.pedestrian.cost_bike_with3 = 2.f;	// 빠른
	m_rpCost.pedestrian.cost_bike_with4 = 2.f;	// 큰길
	// 보행전용
	//m_rpCost.pedestrian.cost_bicycle02 = 1.f; // 보행전용
	//m_rpCost.pedestrian.cost_bicycle12 = 2.5f;
	//m_rpCost.pedestrian.cost_bicycle22 = 2.5f;
	//m_rpCost.pedestrian.cost_bicycle32 = 2.2f;
	//m_rpCost.pedestrian.cost_bicycle42 = 2.5f;
	m_rpCost.pedestrian.cost_bike_walk0 = 1.f;	// 짧은, 보행전용
	m_rpCost.pedestrian.cost_bike_walk1 = 2.5f;	// 추천
	m_rpCost.pedestrian.cost_bike_walk2 = 2.5f;	// 편한
	m_rpCost.pedestrian.cost_bike_walk3 = 2.2f;	// 빠른
	m_rpCost.pedestrian.cost_bike_walk4 = 2.5f;	// 큰길
#	endif
#else
#	if defined(USE_P2P_DATA)
	// 도로 가중치(km/h)
	m_rpCost.vehicle.cost_speed_lv0[0] = 50;
	m_rpCost.vehicle.cost_speed_lv0[1] = 50;
	m_rpCost.vehicle.cost_speed_lv0[2] = 50;
	m_rpCost.vehicle.cost_speed_lv0[3] = 50;
	m_rpCost.vehicle.cost_speed_lv0[4] = 50;

	m_rpCost.vehicle.cost_speed_lv1[0] = 50;
	m_rpCost.vehicle.cost_speed_lv1[1] = 50;
	m_rpCost.vehicle.cost_speed_lv1[2] = 50;
	m_rpCost.vehicle.cost_speed_lv1[3] = 50;
	m_rpCost.vehicle.cost_speed_lv1[4] = 50;

	m_rpCost.vehicle.cost_speed_lv2[0] = 40;
	m_rpCost.vehicle.cost_speed_lv2[1] = 40;
	m_rpCost.vehicle.cost_speed_lv2[2] = 40;
	m_rpCost.vehicle.cost_speed_lv2[3] = 40;
	m_rpCost.vehicle.cost_speed_lv2[4] = 40;

	m_rpCost.vehicle.cost_speed_lv3[0] = 30;
	m_rpCost.vehicle.cost_speed_lv3[1] = 30;
	m_rpCost.vehicle.cost_speed_lv3[2] = 30;
	m_rpCost.vehicle.cost_speed_lv3[3] = 30;
	m_rpCost.vehicle.cost_speed_lv3[4] = 30;

	m_rpCost.vehicle.cost_speed_lv4[0] = 30;
	m_rpCost.vehicle.cost_speed_lv4[1] = 30;
	m_rpCost.vehicle.cost_speed_lv4[2] = 30;
	m_rpCost.vehicle.cost_speed_lv4[3] = 30;
	m_rpCost.vehicle.cost_speed_lv4[4] = 30;

	m_rpCost.vehicle.cost_speed_lv5[0] = 25;
	m_rpCost.vehicle.cost_speed_lv5[1] = 25;
	m_rpCost.vehicle.cost_speed_lv5[2] = 25;
	m_rpCost.vehicle.cost_speed_lv5[3] = 25;
	m_rpCost.vehicle.cost_speed_lv5[4] = 25;

	m_rpCost.vehicle.cost_speed_lv6[0] = 20;
	m_rpCost.vehicle.cost_speed_lv6[1] = 20;
	m_rpCost.vehicle.cost_speed_lv6[2] = 20;
	m_rpCost.vehicle.cost_speed_lv6[3] = 20;
	m_rpCost.vehicle.cost_speed_lv6[4] = 20;

	m_rpCost.vehicle.cost_speed_lv7[0] = 10;
	m_rpCost.vehicle.cost_speed_lv7[1] = 10;
	m_rpCost.vehicle.cost_speed_lv7[2] = 10;
	m_rpCost.vehicle.cost_speed_lv7[3] = 10;
	m_rpCost.vehicle.cost_speed_lv7[4] = 10;

	m_rpCost.vehicle.cost_speed_lv8[0] = 5;
	m_rpCost.vehicle.cost_speed_lv8[1] = 5;
	m_rpCost.vehicle.cost_speed_lv8[2] = 5;
	m_rpCost.vehicle.cost_speed_lv8[3] = 5;
	m_rpCost.vehicle.cost_speed_lv8[4] = 5;

	m_rpCost.vehicle.cost_speed_lv9[0] = 5;
	m_rpCost.vehicle.cost_speed_lv9[1] = 5;
	m_rpCost.vehicle.cost_speed_lv9[2] = 5;
	m_rpCost.vehicle.cost_speed_lv9[3] = 5;
	m_rpCost.vehicle.cost_speed_lv9[4] = 5;

	// 회전 가중치(초)
	m_rpCost.vehicle.cost_time_ang0[0] = 0; // 직진
	m_rpCost.vehicle.cost_time_ang0[1] = 0;
	m_rpCost.vehicle.cost_time_ang0[2] = 0;
	m_rpCost.vehicle.cost_time_ang0[3] = 0;
	m_rpCost.vehicle.cost_time_ang0[4] = 0;

	m_rpCost.vehicle.cost_time_ang45[0] = 5; // 우측
	m_rpCost.vehicle.cost_time_ang45[1] = 5;
	m_rpCost.vehicle.cost_time_ang45[2] = 5;
	m_rpCost.vehicle.cost_time_ang45[3] = 5;
	m_rpCost.vehicle.cost_time_ang45[4] = 5;

	m_rpCost.vehicle.cost_time_ang90[0] = 50; // 우회전 
	m_rpCost.vehicle.cost_time_ang90[1] = 50;
	m_rpCost.vehicle.cost_time_ang90[2] = 50;
	m_rpCost.vehicle.cost_time_ang90[3] = 50;
	m_rpCost.vehicle.cost_time_ang90[4] = 50;

	m_rpCost.vehicle.cost_time_ang135[0] = 400; // 급우회전
	m_rpCost.vehicle.cost_time_ang135[1] = 400;
	m_rpCost.vehicle.cost_time_ang135[2] = 400;
	m_rpCost.vehicle.cost_time_ang135[3] = 400;
	m_rpCost.vehicle.cost_time_ang135[4] = 400;

	m_rpCost.vehicle.cost_time_ang180[0] = 600; // 유턴
	m_rpCost.vehicle.cost_time_ang180[1] = 600;
	m_rpCost.vehicle.cost_time_ang180[2] = 600;
	m_rpCost.vehicle.cost_time_ang180[3] = 600;
	m_rpCost.vehicle.cost_time_ang180[4] = 600;

	m_rpCost.vehicle.cost_time_ang225[0] = 500; // 급좌회전 
	m_rpCost.vehicle.cost_time_ang225[1] = 500;
	m_rpCost.vehicle.cost_time_ang225[2] = 500;
	m_rpCost.vehicle.cost_time_ang225[3] = 500;
	m_rpCost.vehicle.cost_time_ang225[4] = 500;

	m_rpCost.vehicle.cost_time_ang270[0] = 80; // 좌회전 
	m_rpCost.vehicle.cost_time_ang270[1] = 80;
	m_rpCost.vehicle.cost_time_ang270[2] = 80;
	m_rpCost.vehicle.cost_time_ang270[3] = 80;
	m_rpCost.vehicle.cost_time_ang270[4] = 80;

	m_rpCost.vehicle.cost_time_ang315[0] = 60; // 좌측 
	m_rpCost.vehicle.cost_time_ang315[1] = 60;
	m_rpCost.vehicle.cost_time_ang315[2] = 60;
	m_rpCost.vehicle.cost_time_ang315[3] = 60;
	m_rpCost.vehicle.cost_time_ang315[4] = 60;
#	else // #	if defined(USE_P2P_DATA)
	// 도로 가중치(km/h)
	m_rpCost.vehicle.cost_speed_lv0[0] = 60;
	m_rpCost.vehicle.cost_speed_lv0[1] = 60;
	m_rpCost.vehicle.cost_speed_lv0[2] = 60;
	m_rpCost.vehicle.cost_speed_lv0[3] = 60;
	m_rpCost.vehicle.cost_speed_lv0[4] = 60;

	m_rpCost.vehicle.cost_speed_lv1[0] = 50;
	m_rpCost.vehicle.cost_speed_lv1[1] = 50;
	m_rpCost.vehicle.cost_speed_lv1[2] = 50;
	m_rpCost.vehicle.cost_speed_lv1[3] = 50;
	m_rpCost.vehicle.cost_speed_lv1[4] = 50;

	m_rpCost.vehicle.cost_speed_lv2[0] = 40;
	m_rpCost.vehicle.cost_speed_lv2[1] = 40;
	m_rpCost.vehicle.cost_speed_lv2[2] = 40;
	m_rpCost.vehicle.cost_speed_lv2[3] = 40;
	m_rpCost.vehicle.cost_speed_lv2[4] = 40;

	m_rpCost.vehicle.cost_speed_lv3[0] = 30;
	m_rpCost.vehicle.cost_speed_lv3[1] = 30;
	m_rpCost.vehicle.cost_speed_lv3[2] = 30;
	m_rpCost.vehicle.cost_speed_lv3[3] = 30;
	m_rpCost.vehicle.cost_speed_lv3[4] = 30;

	m_rpCost.vehicle.cost_speed_lv4[0] = 30;
	m_rpCost.vehicle.cost_speed_lv4[1] = 30;
	m_rpCost.vehicle.cost_speed_lv4[2] = 30;
	m_rpCost.vehicle.cost_speed_lv4[3] = 30;
	m_rpCost.vehicle.cost_speed_lv4[4] = 30;

	m_rpCost.vehicle.cost_speed_lv5[0] = 25;
	m_rpCost.vehicle.cost_speed_lv5[1] = 25;
	m_rpCost.vehicle.cost_speed_lv5[2] = 25;
	m_rpCost.vehicle.cost_speed_lv5[3] = 25;
	m_rpCost.vehicle.cost_speed_lv5[4] = 25;

	m_rpCost.vehicle.cost_speed_lv6[0] = 20;
	m_rpCost.vehicle.cost_speed_lv6[1] = 20;
	m_rpCost.vehicle.cost_speed_lv6[2] = 20;
	m_rpCost.vehicle.cost_speed_lv6[3] = 20;
	m_rpCost.vehicle.cost_speed_lv6[4] = 20;

	m_rpCost.vehicle.cost_speed_lv7[0] = 10; // 15
	m_rpCost.vehicle.cost_speed_lv7[1] = 10;
	m_rpCost.vehicle.cost_speed_lv7[2] = 10;
	m_rpCost.vehicle.cost_speed_lv7[3] = 10;
	m_rpCost.vehicle.cost_speed_lv7[4] = 10;

	m_rpCost.vehicle.cost_speed_lv8[0] = 5; // 10
	m_rpCost.vehicle.cost_speed_lv8[1] = 5;
	m_rpCost.vehicle.cost_speed_lv8[2] = 5;
	m_rpCost.vehicle.cost_speed_lv8[3] = 5;
	m_rpCost.vehicle.cost_speed_lv8[4] = 5;

	m_rpCost.vehicle.cost_speed_lv9[0] = 5; // 10
	m_rpCost.vehicle.cost_speed_lv9[1] = 5;
	m_rpCost.vehicle.cost_speed_lv9[2] = 5;
	m_rpCost.vehicle.cost_speed_lv9[3] = 5;
	m_rpCost.vehicle.cost_speed_lv9[4] = 5;

	//m_rpCost.vehicle.cost_lv0 = 50;
	//m_rpCost.vehicle.cost_lv1 = 50;
	//m_rpCost.vehicle.cost_lv2 = 40;
	//m_rpCost.vehicle.cost_lv3 = 30;
	//m_rpCost.vehicle.cost_lv4 = 30;
	//m_rpCost.vehicle.cost_lv5 = 25;
	//m_rpCost.vehicle.cost_lv6 = 20;
	//m_rpCost.vehicle.cost_lv7 = 15;
	//m_rpCost.vehicle.cost_lv8 = 15;
	//m_rpCost.vehicle.cost_lv9 = 15;

	//m_rpCost.vehicle.cost_lv0 = 90;
	//m_rpCost.vehicle.cost_lv1 = 80;
	//m_rpCost.vehicle.cost_lv2 = 80;
	//m_rpCost.vehicle.cost_lv3 = 70;
	//m_rpCost.vehicle.cost_lv4 = 60;
	//m_rpCost.vehicle.cost_lv5 = 50;
	//m_rpCost.vehicle.cost_lv6 = 40;
	//m_rpCost.vehicle.cost_lv7 = 30;
	//m_rpCost.vehicle.cost_lv8 = 15;
	//m_rpCost.vehicle.cost_lv9 = 15;

	// 회전 가중치
	// for fleetue
	//m_rpCost.vehicle.cost_ang0 = 0; // 직진
	//m_rpCost.vehicle.cost_ang45 = 5; // 우측
	//m_rpCost.vehicle.cost_ang90 = 10; // 우회전 
	//m_rpCost.vehicle.cost_ang135 = 15; // 급우회전 
	//m_rpCost.vehicle.cost_ang180 = 20;//600; // 유턴
	//m_rpCost.vehicle.cost_ang225 = 30; // 급좌회전 
	//m_rpCost.vehicle.cost_ang270 = 25; // 좌회전 
	//m_rpCost.vehicle.cost_ang315 = 20; // 좌측

	//m_rpCost.vehicle.cost_ang0 = 0; // 직진
	//m_rpCost.vehicle.cost_ang45 = 5; // 우측
	//m_rpCost.vehicle.cost_ang90 = 20; // 우회전 
	//m_rpCost.vehicle.cost_ang135 = 30; // 급우회전 
	//m_rpCost.vehicle.cost_ang180 = 300;//600; // 유턴
	//m_rpCost.vehicle.cost_ang225 = 60; // 급좌회전 
	//m_rpCost.vehicle.cost_ang270 = 60; // 좌회전 
	//m_rpCost.vehicle.cost_ang315 = 60; // 좌측

	// 회전 가중치(초)
	m_rpCost.vehicle.cost_time_ang0[0] = 0; // 직진
	m_rpCost.vehicle.cost_time_ang0[1] = 0;
	m_rpCost.vehicle.cost_time_ang0[2] = 0;
	m_rpCost.vehicle.cost_time_ang0[3] = 0;
	m_rpCost.vehicle.cost_time_ang0[4] = 0;

	m_rpCost.vehicle.cost_time_ang45[0] = 5; // 우측
	m_rpCost.vehicle.cost_time_ang45[1] = 5;
	m_rpCost.vehicle.cost_time_ang45[2] = 5;
	m_rpCost.vehicle.cost_time_ang45[3] = 5;
	m_rpCost.vehicle.cost_time_ang45[4] = 5;

	m_rpCost.vehicle.cost_time_ang90[0] = 10; // 우회전 
	m_rpCost.vehicle.cost_time_ang90[1] = 20;
	m_rpCost.vehicle.cost_time_ang90[2] = 20;
	m_rpCost.vehicle.cost_time_ang90[3] = 20;
	m_rpCost.vehicle.cost_time_ang90[4] = 20;

	m_rpCost.vehicle.cost_time_ang135[0] = 15; // 급우회전
	m_rpCost.vehicle.cost_time_ang135[1] = 30;
	m_rpCost.vehicle.cost_time_ang135[2] = 30;
	m_rpCost.vehicle.cost_time_ang135[3] = 30;
	m_rpCost.vehicle.cost_time_ang135[4] = 30;

	m_rpCost.vehicle.cost_time_ang180[0] = 20; // 유턴
	m_rpCost.vehicle.cost_time_ang180[1] = 300;
	m_rpCost.vehicle.cost_time_ang180[2] = 300;
	m_rpCost.vehicle.cost_time_ang180[3] = 300;
	m_rpCost.vehicle.cost_time_ang180[4] = 300;

	m_rpCost.vehicle.cost_time_ang225[0] = 30; // 급좌회전
	m_rpCost.vehicle.cost_time_ang225[1] = 60;
	m_rpCost.vehicle.cost_time_ang225[2] = 60;
	m_rpCost.vehicle.cost_time_ang225[3] = 60;
	m_rpCost.vehicle.cost_time_ang225[4] = 60;

	m_rpCost.vehicle.cost_time_ang270[0] = 25; // 좌회전 
	m_rpCost.vehicle.cost_time_ang270[1] = 60;
	m_rpCost.vehicle.cost_time_ang270[2] = 60;
	m_rpCost.vehicle.cost_time_ang270[3] = 60;
	m_rpCost.vehicle.cost_time_ang270[4] = 60;

	m_rpCost.vehicle.cost_time_ang315[0] = 20; // 좌측 
	m_rpCost.vehicle.cost_time_ang315[1] = 60;
	m_rpCost.vehicle.cost_time_ang315[2] = 60;
	m_rpCost.vehicle.cost_time_ang315[3] = 60;
	m_rpCost.vehicle.cost_time_ang315[4] = 60;
#	endif
#endif //#if defined(USE_P2P_DATA)
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

const bool CRoutePlan::SetVisitedLink(IN RouteInfo* pRouteInfo, IN const KeyID linkId, IN const bool isReverse)
{
	bool isSet = false;
	unordered_map<uint64_t, CandidateLink*>::const_iterator it;

	if (isReverse) {
		it = pRouteInfo->mRouteReversePass.find(linkId.llid);
		if (it != pRouteInfo->mRouteReversePass.end()) {
			it->second->visited = true;
			isSet = true;
		}
	} else {
		it = pRouteInfo->mRoutePass.find(linkId.llid);
		if (it != pRouteInfo->mRoutePass.end()) {
			it->second->visited = true;
			isSet = true;
		}
	}

	return isSet;
}

const bool CRoutePlan::IsVisitedLink(IN RouteInfo* pRouteInfo, IN const KeyID linkId, IN const bool isReverse)
{
	bool visited = false;
	if (isReverse) {
		if (pRouteInfo->mRouteReversePass.find(linkId.llid) != pRouteInfo->mRouteReversePass.end()) {
			visited = true;
		}
	} else {
		if (pRouteInfo->mRoutePass.find(linkId.llid) != pRouteInfo->mRoutePass.end()) {
			visited = true;
		}
	}

	return visited;
}


const bool CRoutePlan::IsAddedLink(IN RouteInfo* pRouteInfo, IN KeyID linkId, IN const bool isReverse)
{
	bool added = false;
	if (isReverse) {
		if (pRouteInfo->mRouteReversePass.find(linkId.llid) != pRouteInfo->mRouteReversePass.end()) {
			added = true;
		}
	} else {
		if (pRouteInfo->mRoutePass.find(linkId.llid) != pRouteInfo->mRoutePass.end()) {
			added = true;
		}
	}

	return added;
}


const DataCost* CRoutePlan::GetRouteCost(void) const
{
	return &m_rpCost;
}


void CRoutePlan::SetRouteCost(IN const DataCost* pCost)
{
	if (pCost != nullptr) {
		memcpy(&m_rpCost, pCost, sizeof(DataCost));
	}
}


void CRoutePlan::SetRouteCost(IN const uint32_t type, IN const DataCost* pCost, IN const uint32_t cntCost)
{
	if (pCost != nullptr) {
		// ROUTE_TYPE_NONE = 0, // 미정의
		// ROUTE_TYPE_TREKKING, // 숲길
		// ROUTE_TYPE_PEDESTRIAN, // 보행자
		// ROUTE_TYPE_BIKE, // 자전거
		// ROUTE_TYPE_KICKBOARD, // 킥보드
		// ROUTE_TYPE_MOTOCYCLE, // 모터사이클
		// ROUTE_TYPE_VEHICLE, // 차량

		int32_t listCount = cntCost;

		//TYPE_DATA_NAME, // 명칭사전
		//TYPE_DATA_MESH, // 메쉬
		//TYPE_DATA_TREKKING, // 숲길
		//TYPE_DATA_PEDESTRIAN, // 보행자/자전거
		//TYPE_DATA_VEHICLE, // 자동차
		//TYPE_DATA_BUILDING, // 건물
		//TYPE_DATA_COMPLEX, // 단지
		//TYPE_DATA_ENTRANCE, // 입구점
		//TYPE_DATA_TRAFFIC, // 교통정보
		//TYPE_DATA_COUNT,

		switch (type) {
			case TYPE_DATA_TREKKING:
			{
				if (listCount == 0) {
					listCount = 30;
				}
			}
			break;

			case TYPE_DATA_PEDESTRIAN:
			{
				if (listCount == 0) {
					listCount = 30;
				}
			}
			break;

			case TYPE_DATA_VEHICLE:
			{
				if (listCount == 0) {
					listCount = 18;
				}
			}
			break;

			case TYPE_DATA_BUILDING:
			case TYPE_DATA_COMPLEX:
			case TYPE_DATA_ENTRANCE:
			{
				// 최적지점 함수가 datamgr에 있기에 datamgr 호출 
				if (listCount == 0) {
					listCount = 15;
				}

				if (m_pDataMgr != nullptr) {
					m_pDataMgr->SetDataCost(type, pCost);
				}
				return;
			}
			break;

			default :
			{
				if (listCount == 0) {
					listCount = 50;
				}
			}
			break;
		} //switch

		LOG_TRACE(LOG_DEBUG, "set rp type:%d", type);

		LOG_TRACE(LOG_DEBUG_LINE, "set rp cost(old):");
		for(int ii=0; ii<listCount; ii++) {
			LOG_TRACE(LOG_DEBUG_CONTINUE, "%f, ", m_rpCost.base[ii]);
		}
		LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");

		memcpy(&m_rpCost.base, &pCost->base, sizeof(float) * listCount);

		LOG_TRACE(LOG_DEBUG_LINE, "set rp cost(new):");
		for(int ii=0; ii<listCount; ii++) {
			LOG_TRACE(LOG_DEBUG_CONTINUE, "%f, ", pCost->base[ii]);
		}
		LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");
	}
}


const double CRoutePlan::GetTravelTime(IN const RequestRouteInfo* pReqInfo, IN const stLinkInfo* pLink, IN const int spd, IN const int angle, IN const double dist, IN const double time)
{
	double prevTime = 0; // 기본 시간

	if (pReqInfo == nullptr || pLink == nullptr) {
		return VAL_MAX_COST;
	}

	double baseTime = 0.f; // 기본 주행 시간
	double travelTime = 0.f; // 실 주행 시간

	double delayValue = 0;
	double avgSpd = spd;
	double length = min(dist, pLink->length);

	const uint32_t opt = pReqInfo->RouteOption;
	const uint32_t avoid = pReqInfo->AvoidOption;
	const uint32_t mobility = pReqInfo->MobilityOption;

	if (spd == SPEED_NOT_AVALABLE) {
		// 거리를 시간으로 환산	// 기본 이동 속도 = 5Km/h = 5000m/60M = 500m/6M = 500m/360s = 1.388..m/s => 1.3888/s;
		static const double avgTreSpd = 3.0; // 산행 이동 속도 = 3Km/h
		static const double avgPedSpd = 4.0;// 3.3; // 도보 이동 속도 = 4Km/h
		static const double avgBycSpd = 20.0; // 16.0; // 자전거 이동 속도 = 20Km/h
		static const double avgCarSpd = 50.0; // 차량 이동 속도 = 50Km/h

		if (pLink->base.link_type == TYPE_LINK_DATA_TREKKING) {
			if (mobility == TYPE_MOBILITY_BICYCLE) {
				avgSpd = avgBycSpd;
			} else {
				avgSpd = avgTreSpd;
			}
		} else if (pLink->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) {
			if (mobility == TYPE_MOBILITY_BICYCLE) {
				// 자전거도로 타입, 1:자전거전용, 2:보행자/차량겸용 자전거도로, 3:보행도로
				if (pLink->ped.bicycle_type == TYPE_BYC_ONLY) {
					avgSpd = avgBycSpd;// 자전거 전용 도로는 원래의 속도로 주행
				} else if (pLink->ped.bicycle_type == TYPE_BYC_WITH_CAR) {
					avgSpd = avgBycSpd * .8f; // 보행자/차량 겸용 도로는 80% 감속 주행
				} else {
					if (pLink->ped.walk_type == TYPE_WALK_ONLY) {
						avgSpd = avgBycSpd * .3f; // 보행자 전용도로는 50% 감속 주행
					} else {
						avgSpd = avgBycSpd * .7f; // 보행 도로는 70% 감속 주행
					}
				}
			} else {
				//보행자도로 타입, 1:복선도로, 2:차량겸용도로, 3:자전거전용도로, 4:보행전용도로, 5:
				if (pLink->ped.bicycle_type == TYPE_WALK_SIDE) { // 복선도로
					avgSpd = avgPedSpd;// 복선도로는 원래의 속도로 주행
				} else if (pLink->ped.bicycle_type == TYPE_WALK_WITH_CAR) { // 차량겸용도로
					avgSpd = avgPedSpd * .95f; // 보행자/차량 겸용 도로는 90%로 감속 주행
				} else if (pLink->ped.bicycle_type == TYPE_WALK_WITH_BYC) { // 자전거전용도로
					avgSpd = avgPedSpd * .9f; // 보행자/차량 겸용 도로는 90%로 감속 주행
				} else if (pLink->ped.walk_type == TYPE_WALK_ONLY) { // 보행전용도로
					avgSpd = avgPedSpd;// 보행전용도로는 원래의 속도로 주행
				} else if (pLink->ped.walk_type == TYPE_WALK_THROUGH) { // 가상보행도로
					avgSpd = avgPedSpd; // 가상보행도로는 60% 감속 주행
				} else {
					avgSpd = avgPedSpd;
				}
			}
		} else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			avgSpd = GetLinkSpeed(pLink->link_id, pLink->veh.level, pReqInfo->RouteOption);
		} else {
			avgSpd = avgPedSpd;
		}
	} else {
		avgSpd = spd;
	}

	double spdValue = avgSpd / 3.6f;

	if ((length > 0) && (spdValue > 0)) {
		travelTime = baseTime = (length / spdValue); // 기본 주행 시간

		if (pLink->base.link_type == TYPE_LINK_DATA_TREKKING) {
			travelTime = time;
		} else if (pLink->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) {
			// angle => 180:직진, 0:유턴, 90:우회전, -90:좌회전, (수정 전)
			// angle => 0:직진, 180:유턴, 90:우회전, 270:좌회전, (수정 후, 2023-10-13)
			//int ang360 = angle;
			//int ang180 = 180 - abs(180 - angle); // +-10:직진, +-170: 유턴
			//if (abs(ang180) > 10) {
			//	angCost = abs(ang180) % 180;
			//}

			// 자전거
			if (mobility == TYPE_MOBILITY_BICYCLE) {
				//if (angCost > 0 && m_rpCost.pedestrian.cost_angle_walk[opt] > 0) {
				//	secCost += angCost * m_rpCost.pedestrian.cost_angle_walk[opt];
				//}

				// +400% 가산은 걷기 속도와 유사

				// 시설물 타입별
				switch (pLink->ped.facility_type) {
				case TYPE_OBJECT_CAVE: {//strcat(szInfo, "토끼굴");
					travelTime += baseTime * 1.0; // 100% 가산
					break;
				}
				case TYPE_OBJECT_UNDERPASS: {//strcat(szInfo, "지하보도");
					travelTime += baseTime * 1.0; // 100% 가산
					break;
				}
				case TYPE_OBJECT_FOOTBRIDGE: {//strcat(szInfo, "육교");
					travelTime += baseTime * 1.0; // 100% 가산
					break;
				}
				case TYPE_OBJECT_OVERPASS: {//strcat(szInfo, "고가도로");
					travelTime += baseTime * 0.5; // 50% 가산
					break;
				}
				case TYPE_OBJECT_BRIDGE: {//strcat(szInfo, "교량");
					travelTime += baseTime * 0.5; // 50% 가산
					break;
				}
				case TYPE_OBJECT_SUBWAY: {//strcat(szInfo, "지하철역");
					travelTime += baseTime * 4.0; // 400% 가산
					break;
				}
				case TYPE_OBJECT_RAILROAD: {//strcat(szInfo, "철도");
					travelTime += baseTime * 4.0; // 400% 가산
					break;
				}
				case TYPE_OBJECT_BUSSTATION: {//strcat(szInfo, "중앙버스정류장");
					travelTime += baseTime * 4.0; // 400% 가산
					break;
				}
				case TYPE_OBJECT_UNDERGROUNDMALL: {//strcat(szInfo, "지하상가");
					travelTime += baseTime * 4.0; // 400% 가산
					break;
				}
				case TYPE_OBJECT_THROUGHBUILDING: {//strcat(szInfo, "건물관통도로");
					travelTime += baseTime * 4.0; // 400% 가산
					break;
				}
				case TYPE_OBJECT_COMPLEXPARK: {//strcat(szInfo, "단지도로_공원");
					travelTime += baseTime * 1.0; // 100% 가산
					break;
				}
				case TYPE_OBJECT_COMPLEXAPT: {//strcat(szInfo, "단지도로_주거시설");
					travelTime += baseTime * 1.0; // 100% 가산
					break;
				}
				case TYPE_OBJECT_COMPLEXTOUR: {//strcat(szInfo, "단지도로_관광지");
					travelTime += baseTime * 4.0; // 400% 가산
					break;
				}
				case TYPE_OBJECT_COMPLEXETC: {//strcat(szInfo, "단지도로_기타");
					travelTime += baseTime * 1.0; // 100% 가산
					break;
				}
				default:
				{
					break;
				}
				} // switch (pLink->ped.facility_type)


				 // 진입로 타입
				switch (pLink->ped.gate_type) {
				case TYPE_GATE_SLOP: {// 1 : 경사로
					travelTime += baseTime * 0.2; // 20% 가산
					break;
				}
				case TYPE_GATE_STAIRS: {// 2 : 계단
					travelTime += baseTime * 4.0; // 400% 가산
					break;
				}
				case TYPE_GATE_ESCALATOR: {// 3 : 에스컬레이터
					break;
				}
				case TYPE_GATE_STAIRS_ESCALATOR: {// 4 : 계단 / 에스컬레이터
					travelTime += baseTime * 0.1; // 10% 가산
					break;
				}
				case TYPE_GATE_ELEVATOR: {// 5 : 엘리베이터
					break;
				}
				case TYPE_GATE_CONNECTION: {// 6 : 단순연결로
					break;
				}
				case TYPE_GATE_CROSSWALK: {// 7 : 횡단보도
					if (pLink->length >= 10) {
						travelTime += ((pLink->length / 10) * 10); // 10m당 10초 가산
					}
					break;
				}
				case TYPE_GATE_MOVINGWALK: {// 8 : 무빙워크
					break;
				}
				case TYPE_GATE_STEPPINGSTONES: {// 9 : 징검다리
					travelTime += baseTime * 4.0; // 400% 가산
					break;
				}
				case TYPE_GATE_VIRTUAL: {// 10 : 의사횡단
					travelTime += baseTime * 4.0; // 400% 가산
					break;
				}
				default: { // 0:미정의
					break;
				}
				} // switch (pLink->ped.gate_type)
			} // 보행
			else { //(mobility == TYPE_MOBILITY_PEDESTRIAN)
				// 회전을 할수록 가중치 적용
				//if (angCost > 0 && m_rpCost.pedestrian.cost_angle_walk[opt] > 0) {
				//	secCost += angCost * m_rpCost.pedestrian.cost_angle_walk[opt];
				//}

				// 시설물 타입별 시간 가/감
				switch (pLink->ped.facility_type) {
				case TYPE_OBJECT_CAVE: {//strcat(szInfo, "토끼굴");
					break;
				}
				case TYPE_OBJECT_UNDERPASS: {//strcat(szInfo, "지하보도");
					break;
				}
				case TYPE_OBJECT_FOOTBRIDGE: {//strcat(szInfo, "육교");
					break;
				}
				case TYPE_OBJECT_OVERPASS: {//strcat(szInfo, "고가도로");
					break;
				}
				case TYPE_OBJECT_BRIDGE: {//strcat(szInfo, "교량");
					break;
				}
				case TYPE_OBJECT_SUBWAY: {//strcat(szInfo, "지하철역");
					travelTime += baseTime * 0.1; // 10% 가산
					break;
				}
				case TYPE_OBJECT_RAILROAD: {//strcat(szInfo, "철도");
					travelTime += baseTime * 0.1; // 10% 가산
					break;
				}
				case TYPE_OBJECT_BUSSTATION: {//strcat(szInfo, "중앙버스정류장");
					travelTime += baseTime * 0.1; // 10% 가산
					break;
				}
				case TYPE_OBJECT_UNDERGROUNDMALL: {//strcat(szInfo, "지하상가");
					travelTime += baseTime * 0.2; // 20% 가산
					break;
				}
				case TYPE_OBJECT_THROUGHBUILDING: {//strcat(szInfo, "건물관통도로");
					break;
				}
				case TYPE_OBJECT_COMPLEXPARK: {//strcat(szInfo, "단지도로_공원");
					break;
				}
				case TYPE_OBJECT_COMPLEXAPT: {//strcat(szInfo, "단지도로_주거시설");
					break;
				}
				case TYPE_OBJECT_COMPLEXTOUR: {//strcat(szInfo, "단지도로_관광지");
					break;
				}
				case TYPE_OBJECT_COMPLEXETC: {//strcat(szInfo, "단지도로_기타");
					break;
				}
				default:
				{
					break;
				}
				} // switch (pLink->ped.facility_type)


				// 진입로 타입
				switch (pLink->ped.gate_type) {
				case TYPE_GATE_SLOP: {// 1 : 경사로
					travelTime += baseTime * 0.1; // 10% 가산
					break;
				}
				case TYPE_GATE_STAIRS: {// 2 : 계단
					travelTime += baseTime * 0.2; // 20% 가산
					break;
				}
				case TYPE_GATE_ESCALATOR: {// 3 : 에스컬레이터
					break;
				}
				case TYPE_GATE_STAIRS_ESCALATOR: {// 4 : 계단 / 에스컬레이터
					break;
				}
				case TYPE_GATE_ELEVATOR: {// 5 : 엘리베이터
					break;
				}
				case TYPE_GATE_CONNECTION: {// 6 : 단순연결로
					break;
				}
				case TYPE_GATE_CROSSWALK: {// 7 : 횡단보도
					if (pLink->length >= 30) {
						travelTime += ((pLink->length / 30) * 5); // 30m당 10초 가산
					}
					break;
				}
				case TYPE_GATE_MOVINGWALK: {// 8 : 무빙워크
					break;
				}
				case TYPE_GATE_STEPPINGSTONES: {// 9 : 징검다리
					travelTime += baseTime * 0.5; // 50% 가산
					break;
				}
				case TYPE_GATE_VIRTUAL: {// 10 : 의사횡단
					travelTime += baseTime * 0.1; // 10% 가산
					break;
				}
				default:
				{ // 0:미정의
					break;
				}
				} // switch (pLink->ped.gate_type)
			}			
		} else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			travelTime = time; // 원래 시간 사용
		}
	}

	return travelTime;
}


const uint8_t CRoutePlan::GetLinkSpeed(IN const KeyID linkId, IN const uint8_t level, IN const uint32_t opt)
{
	uint8_t speed = SPEED_NOT_AVALABLE;

#if defined(USE_VEHICLE_DATA)
	// 경로레벨, 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상, 4:일반도로6차선이상, 
	// 5:일반도로4차선이상, 6:일반도로2차선이상, 7:일반도로1차선이상, 8:SS도로, 9:GSS도로/단지내도로/통행금지도로/비포장도로
	switch (level) {
		case 0:
			speed = m_rpCost.vehicle.cost_speed_lv0[opt]; break;// 50; break; //100
		case 1:
			speed = m_rpCost.vehicle.cost_speed_lv1[opt]; break;// 50; break; //80
		case 2:
			speed = m_rpCost.vehicle.cost_speed_lv2[opt]; break;// 40; break; //80
		case 3:
			speed = m_rpCost.vehicle.cost_speed_lv3[opt]; break;// 30; break; //70
		case 4:
			speed = m_rpCost.vehicle.cost_speed_lv4[opt]; break;// 30; break; //60
		case 5:
			speed = m_rpCost.vehicle.cost_speed_lv5[opt]; break;// 25; break; //50
		case 6:
			speed = m_rpCost.vehicle.cost_speed_lv6[opt]; break;// 20; break; //40
		case 7:
			speed = m_rpCost.vehicle.cost_speed_lv7[opt]; break;// 30; break; //60
		case 8:
			speed = m_rpCost.vehicle.cost_speed_lv8[opt]; break;// 25; break; //50
		case 9:
			speed = m_rpCost.vehicle.cost_speed_lv9[opt]; break;// 20; break; //40

		default:
			speed = m_rpCost.vehicle.cost_speed_lv9[opt]; break;// 15; break; //30
	} // switch
#endif // #if defined(USE_VEHICLE_DATA)

	return speed;
}


//const double CRoutePlan::GetCost(IN const stLinkInfo* pLink, IN const uint32_t dirTarget, IN const uint32_t opt, IN const uint32_t mobility, IN const double length, IN const uint8_t spd) // type, 0:보행자, 1:자전거 
const double CRoutePlan::GetCost(IN const RequestRouteInfo* pReqInfo, IN const stLinkInfo* pLink, IN const uint32_t dirTarget, IN const double length, IN const uint8_t spd)
{
	if (pReqInfo == nullptr || pLink == nullptr) {
		return VAL_MAX_COST;
	}

	// 거리를 시간으로 환산	// 기본 이동 속도 = 5Km/h = 5000m/60M = 500m/6M = 500m/360s = 1.388..m/s => 1.3888/s;
	static const double avgTreSpd = 3.0; // 산행 이동 속도 = 3Km/h
	static const double avgPedSpd = 4.0;// 3.3; // 도보 이동 속도 = 4Km/h
	static const double avgBycSpd = 16.0; // 16.0; // 자전거 이동 속도 = 20Km/h
	static const double avgCarSpd = 50.0; // 차량 이동 속도 = 50Km/h

	double secCost = 0;

	double delayValue = 0;
	double avgSpd = spd;
	double dist = length;

	const int32_t opt = pReqInfo->RouteOption;
	const int32_t mobility = pReqInfo->MobilityOption;

	if (spd == SPEED_NOT_AVALABLE) {
		if (pLink->base.link_type == TYPE_LINK_DATA_TREKKING) {
			if (mobility == TYPE_MOBILITY_BICYCLE) {
				avgSpd = avgBycSpd;
			} else {
				avgSpd = avgTreSpd;
			}
		} else if (pLink->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) {
			if (mobility == TYPE_MOBILITY_BICYCLE) {
#if 1
				avgSpd = avgBycSpd; // pedestrian_cost 값으로 변경
#else
				// 자전거도로 타입, 1:자전거전용, 2:보행자/차량겸용 자전거도로, 3:보행도로
				if (pLink->ped.bicycle_type == TYPE_BYC_ONLY) {
					avgSpd = avgBycSpd;// *1.2f; // 자전거 전용 도로는 원래의 속도로 주행
				}
				else if (pLink->ped.bicycle_type == TYPE_BYC_WITH_CAR) {
					avgSpd = avgBycSpd * .8f; // 보행자/차량 겸용 도로는 80% 감속 주행
				}
				else {
					if (pLink->ped.walk_type == TYPE_WALK_ONLY) {
						avgSpd = avgBycSpd * .5f; // 보행자 전용도로는 50% 감속 주행
					} else {
						avgSpd = avgBycSpd * .7f; // 보행 도로는 70% 감속 주행
					}
				}
#endif
			}
			//else if (pLink->ped.walk_type == TYPE_WALK_WITH_BYC) {
			//	avgSpd = avgPedSpd * .8f; // 통행 불가 확인, 자전거 전용도로면 조심해서 걷기에 보행 속도 절반으로 책정
			//}
			else {
				avgSpd = avgPedSpd;
			}
		} else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			avgSpd = GetLinkSpeed(pLink->link_id, pLink->veh.level, pReqInfo->RouteOption);
		} else {
			avgSpd = avgPedSpd;
		}
	}

	if ((opt == ROUTE_OPT_SHORTEST) && (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE)) {
		avgSpd = avgCarSpd;
	}
		

	double spdValue = avgSpd / 3.6f;


	if (dist <= 0) {
		dist = pLink->length;
	}

	if (dist > 0)
	{
		//secCost = round(dist / spdValue);
		secCost = (dist / spdValue);

		if (pLink->base.link_type == TYPE_LINK_DATA_TREKKING)
		{
			int nRoadType = pLink->trk.road_info;
			// 노면정보 코드, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드

#if defined(USE_FOREST_DATA)
			if (mobility == TYPE_MOBILITY_PEDESTRIAN) {
				if (dirTarget == 1) { // 정방향 
					secCost = pLink->trk.fw_tm;
				} else {
					secCost = pLink->trk.bw_tm;
				}
			}
#endif

			if (opt == ROUTE_OPT_SHORTEST) {
				;
			}
			else if (opt == ROUTE_OPT_FASTEST) {
				;
			}
			else if (nRoadType > 0 && opt == ROUTE_OPT_COMFORTABLE)
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
		else if (pLink->base.link_type == TYPE_LINK_DATA_PEDESTRIAN)
		{
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
		else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			// if (opt == ROUTE_OPT_RECOMMENDED || opt == ROUTE_OPT_COMFORTABLE || opt == ROUTE_OPT_MAINROAD || mobility == TYPE_MOBILITY_AUTONOMOUS) {
			if (opt != ROUTE_OPT_SHORTEST) {
				if (pLink->veh.car_only == 1) { // 자동차 전용도로
					secCost *= 0.85;
				} else if (pLink->veh.link_type == 2 || pLink->veh.lane_cnt >= 6) { // 분리도로는 좀 더 빠르게
					secCost *= 0.9;
				}
			}
		}
	}
	
	return secCost;
}

// 거리를 시간으로 환산
//const double CRoutePlan::GetTravelCost(IN const stLinkInfo* pLink, IN const stLinkInfo* pLinkPrev, IN const int spd, IN const double cost, IN const int angle, IN const uint32_t opt, IN const uint32_t avoid, IN const uint32_t mobility, IN const stLinkSubInfo subinfo)
const double CRoutePlan::GetTravelCost(IN const RequestRouteInfo* pReqInfo, IN const stLinkInfo* pLink, IN const stLinkInfo* pLinkPrev, IN const RouteLinkInfo* pLinkInfo, IN const int spd, IN const double cost, IN const int angle)
{
	if (pReqInfo == nullptr || pLink == nullptr) {
		return 0.f;
	}

	int angCost = 0;
	double secCost = cost;
	double waitCost = 0;

	const uint32_t opt = pReqInfo->RouteOption;
	const uint32_t avoid = pReqInfo->AvoidOption;
	const uint32_t mobility = pReqInfo->MobilityOption;

	if (pLink != nullptr && pLink->length > 0)
	{
		if (pLink->base.link_type == TYPE_LINK_DATA_TREKKING)
		{
			// 회피 구간의 진행 시간을 크게 (하루) 늘림
			int nRoadType = pLink->trk.road_info;
			// 노면정보 코드, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드

			// 등산로의 인기도등급 10 & 비법정코스 길은, 가능하면(길이 없는 경우가 아니면) 보류하자
			if ((pLink->trk.course_type <= 1) && (pLink->trk.pop_grade == 10) && (pLink->trk.legal == 0)) {
				secCost += pLink->length * MAX_FOREST_POPULAR_NONE;
			}

			if ((pLink->trk.course_type <= 1) && (m_rpCost.pedestrian.cost_forest_popular[opt] > 0)) { // 현재는 ROUTE_OPT_RECOMMENDED, ROUTE_OPT_MAINROAD 만
				if (pLink->trk.legal != 0) { // 법정코스는 코스트를 절반 낮추자
					secCost += (pLink->length / 2) * pLink->trk.pop_grade * m_rpCost.pedestrian.cost_forest_popular[opt];
				} else {
					secCost += pLink->length * pLink->trk.pop_grade * m_rpCost.pedestrian.cost_forest_popular[opt];
				}
			}

			if (m_rpCost.pedestrian.cost_forest_slop[opt] > 0) { // 현재는 ROUTE_OPT_COMFORTABLE 만
				int slop = static_cast<int32_t>(pLinkPrev->trk.slop - pLink->trk.slop);
				secCost += pLink->length * abs(slop) * m_rpCost.pedestrian.cost_forest_slop[opt];
			}

			if (opt == ROUTE_OPT_SHORTEST)
				// 짧은 길은 시간 무시
				secCost = pLink->length;

			//if (opt == ROUTE_OPT_RECOMMENDED) {
			//	if (pLink->trk.course_type <= 1) {
			//		if (pLink->trk.legal != 0) { // 법정코스는 코스트를 절반 낮추자
			//			secCost += (pLink->length / 2) * pLink->trk.pop_grade * m_rpCost.pedestrian.cost_forest_popular[opt];
			//		} else {
			//			secCost += pLink->length * pLink->trk.pop_grade * m_rpCost.pedestrian.cost_forest_popular[opt];
			//		}
			//	}
			//}
			//else if (opt == ROUTE_OPT_FASTEST) {
			//	; // 기본 시간 사용
			//}
			//else if (opt == ROUTE_OPT_COMFORTABLE) {
			//	int slop = static_cast<int32_t>(pLinkPrev->trk.slop - pLink->trk.slop);
			//	secCost += pLink->length * abs(slop) * m_rpCost.pedestrian.cost_forest_slop[opt];
			//}
			//else if (opt == ROUTE_OPT_MAINROAD) {
			//	if (pLink->trk.course_type <= 1) {
			//		if (pLink->trk.legal != 0) { // 법정코스는 코스트를 절반 낮추자
			//			secCost += (pLink->length / 2) * pLink->trk.pop_grade * m_rpCost.pedestrian.cost_forest_popular[opt];
			//		} else {
			//			secCost += pLink->length * pLink->trk.pop_grade * m_rpCost.pedestrian.cost_forest_popular[opt];
			//		}
			//	}
			//} else { //if(opt == ROUTE_OPT_SHORTEST)
			//	// 짧은 길은 시간 무시
			//	secCost = pLink->length;
			//}

			
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
					if ((avoid & ROUTE_AVOID_STAIRS) == ROUTE_AVOID_STAIRS) {
						waitCost = 3600;
					}
				}
				if ((nRoadType & ROUTE_AVOID_PAVE) == ROUTE_AVOID_PAVE) { // 2:포장길 = 10 = 2				
				}
				if ((nRoadType & ROUTE_AVOID_ALLEY) == ROUTE_AVOID_ALLEY) { // 1:오솔길 = 1
				}
			}			
		}
		else if (pLink->base.link_type == TYPE_LINK_DATA_PEDESTRIAN)
		{
			// angle => 180:직진, 0:유턴, 90:우회전, -90:좌회전, (수정 전)
			// angle => 0:직진, 180:유턴, 90:우회전, 270:좌회전, (수정 후, 2023-10-13)
			int ang360 = angle;
			int ang180 = 180 - abs(180 - angle); // +-10:직진, +-170: 유턴
			if (abs(ang180) > 10) {
				angCost = abs(ang180) % 180;
			}

			// 지점 주변에선는 지점 링크 속성과 동일한 링크는 가중치를 마구 높이지 않도록 하자
			// 그러나 양방향 탐색을 적용하면 자연스레 해소될 내용임...
			bool isNearLink = false;
			static const int maxNearDist = 100;
			static const int limitNearDist = 500;
			if ((pLinkInfo != nullptr)) {
				const int MAX_NEAR_DIST = min(limitNearDist, (maxNearDist > max(pLinkInfo->LinkDistToE + 5, pLinkInfo->LinkDistToE + 5)) ? maxNearDist : (pLinkInfo->LinkDistToE + pLinkInfo->LinkDistToE));
				// 목적지 부근 100m ~ 500m (또는 링크길이 + 5m 이내)일 때만 동일 속성인지 확인하자
				if (getRealWorldDistance(pLinkInfo->MatchCoord.x, pLinkInfo->MatchCoord.y, pLink->getVertexX(0), pLink->getVertexY(0)) <= MAX_NEAR_DIST) {
					isNearLink = true;
				}
			}

			// 자전거 처리
			if (mobility == TYPE_MOBILITY_BICYCLE) {
				// 자전거도로 타입, 1:자전거전용, 2:보행자/차량겸용 자전거도로, 3:보행도로
				if (pLink->ped.bicycle_type == TYPE_BYC_ONLY) {
					secCost += pLink->length * m_rpCost.pedestrian.cost_bike_only[opt];
				} else if (pLink->ped.bicycle_type == TYPE_BYC_WITH_CAR) {
					secCost += pLink->length * m_rpCost.pedestrian.cost_bike_with[opt];
				} else { // if (pLink->ped.bicycle_type == TYPE_BYC_WITH_WALK) {
					//secCost += pLink->length * m_rpCost.pedestrian.cost_bike_walk[opt] * (double)min(1, (MAX_LANE_COUNT - pLink->ped.lane_count * 2));
					secCost += pLink->length * ((double)max(1, MAX_LANE_COUNT - static_cast<int32_t>(pLink->ped.lane_count)) * m_rpCost.pedestrian.cost_bike_walk[opt]);
					if ((pLink->ped.gate_type != TYPE_GATE_CROSSWALK) && (pLink->ped.walk_type == TYPE_WALK_ONLY)) { // 보행 전용(횡단보도제외)은 가중치를 더 준다.
						secCost *= 2.f;
					}
				}				

				// 차선 가중치, 자전거 전용 도로는 좀더 빠르게
				if (pLink->ped.lane_count > 1) {
					secCost += pLink->length * ((double)max(0, MAX_LANE_COUNT - static_cast<int32_t>(pLink->ped.lane_count)) * m_rpCost.pedestrian.cost_lane_bike[opt]);
				}


				// 회전을 할수록 가중치 적용
				if (angCost > 0 && m_rpCost.pedestrian.cost_angle_bike[opt] > 0) {
					secCost += angCost * m_rpCost.pedestrian.cost_angle_bike[opt];
				}


				if ((pLink->ped.bicycle_type == TYPE_BYC_ONLY) || ((isNearLink) && (pLinkInfo->LinkSubInfo.ped.facility_type == pLink->ped.facility_type))) {
					secCost += 0; // 자전거 전용 속성이 함께 있는 경우가 있는 경우 가중치 무효
				} else {
					// 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
					switch (pLink->ped.facility_type) {
					case TYPE_OBJECT_CAVE:
					{//strcat(szInfo, "토끼굴");
						break;
					}
					case TYPE_OBJECT_UNDERPASS:
					{//strcat(szInfo, "지하보도");
						if ((opt == ROUTE_OPT_SHORTEST) && (pLink->ped.gate_type == TYPE_GATE_CONNECTION)) {// 6 : 단순연결로))
							;
						} else {
							secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						}
						break;
					}
					case TYPE_OBJECT_FOOTBRIDGE:
					{//strcat(szInfo, "육교");
						secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						break;
					}
					case TYPE_OBJECT_OVERPASS:
					{//strcat(szInfo, "고가도로");
						break;
					}
					case TYPE_OBJECT_BRIDGE:
					{//strcat(szInfo, "교량");
						break;
					}
					case TYPE_OBJECT_SUBWAY:
					{//strcat(szInfo, "지하철역");
						secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						break;
					}
					case TYPE_OBJECT_RAILROAD:
					{//strcat(szInfo, "철도");
						secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						break;
					}
					case TYPE_OBJECT_BUSSTATION:
					{//strcat(szInfo, "중앙버스정류장");
						secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						break;
					}
					case TYPE_OBJECT_UNDERGROUNDMALL:
					{//strcat(szInfo, "지하상가");
						secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						break;
					}
					case TYPE_OBJECT_THROUGHBUILDING:
					{//strcat(szInfo, "건물관통도로");
						secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						break;
					}
					case TYPE_OBJECT_COMPLEXPARK:
					{//strcat(szInfo, "단지도로_공원");
						if (opt != ROUTE_OPT_SHORTEST) {
							secCost *= 2.f; // 자전거는 진입 어렵게
						}
						break;
					}
					case TYPE_OBJECT_COMPLEXAPT:
					{//strcat(szInfo, "단지도로_주거시설");
						if (opt != ROUTE_OPT_SHORTEST) {
							secCost *= 10.f; // 자전거는 진입 어렵게
						}
						break;
					}
					case TYPE_OBJECT_COMPLEXTOUR:
					{
						//strcat(szInfo, "단지도로_관광지");
						if (opt != ROUTE_OPT_SHORTEST) {
#if defined(USE_FOREST_DATA)
							secCost += pLink->length * 10.f; // 자전거는 진입 어렵게
#else
							secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
#endif
						}
						break;
					}
					case TYPE_OBJECT_COMPLEXETC:
					{//strcat(szInfo, "단지도로_기타");
						if (opt != ROUTE_OPT_SHORTEST) {
							secCost *= 2.f; // 자전거는 진입 어렵게
						}
						break;
					}
					default:
					{
						break;
					}
					} // switch (pLink->ped.facility_type)
				}

				if ((pLink->ped.bicycle_type == TYPE_BYC_ONLY) || ((isNearLink) && (pLinkInfo->LinkSubInfo.ped.gate_type == pLink->ped.gate_type))) {
					secCost += 0; // 자전거 전용 속성이 함께 있는 경우가 있는 경우 가중치 무효
				} else {
					// 진입로 타입, 0:미정의, 1:경사로, 2:계단, 3:에스컬레이터, 4:계단/에스컬레이터, 5:엘리베이터, 6:단순연결로, 7:횡단보도, 8:무빙워크, 9:징검다리, 10:의사횡단
					switch (pLink->ped.gate_type) {
					case TYPE_GATE_SLOP: {// 1 : 경사로
						break;
					}
					case TYPE_GATE_STAIRS: {// 2 : 계단
						secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						break;
					}
					case TYPE_GATE_ESCALATOR: {// 3 : 에스컬레이터
						secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						break;
					}
					case TYPE_GATE_STAIRS_ESCALATOR: {// 4 : 계단 / 에스컬레이터
						secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						break;
					}
					case TYPE_GATE_ELEVATOR: {// 5 : 엘리베이터
						secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						break;
					}
					case TYPE_GATE_CONNECTION: {// 6 : 단순연결로
						break;
					}
					case TYPE_GATE_CROSSWALK: {// 7 : 횡단보도
						//waitCost = 30; // 횡단보도는 기다리는 시간이 주어 자주 건너지 않도록...
						waitCost += pLink->length * m_rpCost.pedestrian.cost_cross_bike[opt];
						break;
					}
					case TYPE_GATE_MOVINGWALK: {// 8 : 무빙워크
						secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						break;
					}
					case TYPE_GATE_STEPPINGSTONES: {// 9 : 징검다리
						secCost += pLink->length * 100.f; // 자전거는 진입 어렵게
						break;
					}
					case TYPE_GATE_VIRTUAL: {// 10 : 의사횡단
						break;
					}
					default: { // 0:미정의
						break;
					}
					} // switch (pLink->ped.gate_type)
				}
			}
			// 보행 처리
			else { //(mobility == TYPE_MOBILITY_PEDESTRIAN)
				// 차선 가중치 - 차선이 많을수록 추천
				secCost += cost * ((double)max(0, MAX_LANE_COUNT - static_cast<int32_t>(pLink->ped.lane_count)) * m_rpCost.pedestrian.cost_lane_walk[opt]);

				//보행자도로 타입, 
				if (pLink->ped.walk_type == TYPE_WALK_SIDE) { // 1:복선도로
					secCost += pLink->length * m_rpCost.pedestrian.cost_walk_side[opt];
				} else if (pLink->ped.walk_type == TYPE_WALK_WITH_CAR) { // 2:차량겸용도로
					secCost += pLink->length * m_rpCost.pedestrian.cost_walk_with[opt];
				} else if (pLink->ped.walk_type == TYPE_WALK_WITH_BYC) { // 3:자전거전용도로
					secCost += pLink->length * m_rpCost.pedestrian.cost_walk_bike[opt];
				} else if (pLink->ped.walk_type == TYPE_WALK_ONLY) { // 4:보행전용도로
					secCost += pLink->length * m_rpCost.pedestrian.cost_walk_only[opt];
				} else if (pLink->ped.walk_type == TYPE_WALK_THROUGH) { // 5:가상보행도로
					secCost += pLink->length * m_rpCost.pedestrian.cost_walk_line[opt];
				} else { //
					;
				}

				if (opt == ROUTE_OPT_RECOMMENDED) {
					;
				} else if (opt == ROUTE_OPT_COMFORTABLE) {
					;
				} else if (opt == ROUTE_OPT_FASTEST) {
					if (pLink->ped.walk_type == TYPE_WALK_SIDE) {
						if ((pLink->ped.gate_type == TYPE_GATE_CONNECTION) || (pLink->ped.gate_type == TYPE_GATE_NONE)) {
							secCost *= 0.7;
						}
					} else if (pLink->ped.walk_type == TYPE_WALK_WITH_CAR) {
						secCost *= 0.9;
					} else if (pLink->ped.walk_type == TYPE_WALK_WITH_BYC) {
						;
					} else if (pLink->ped.walk_type == TYPE_WALK_ONLY) {
						if ((pLink->ped.gate_type == TYPE_GATE_CONNECTION) || (pLink->ped.gate_type == TYPE_GATE_NONE)) {
							secCost *= 0.7;
						}
					} else if (pLink->ped.walk_type == TYPE_WALK_THROUGH) {
						;
					} else {
						;
					}
				} else if (opt == ROUTE_OPT_MAINROAD) {
					if (pLink->ped.walk_type == TYPE_WALK_SIDE) {
						if ((pLink->ped.gate_type == TYPE_GATE_CONNECTION) || (pLink->ped.gate_type == TYPE_GATE_NONE)) {
							secCost *= 0.7;
						}
					} else if (pLink->ped.walk_type == TYPE_WALK_WITH_CAR) {
						secCost *= 0.9;
					} else if (pLink->ped.walk_type == TYPE_WALK_WITH_BYC) {
						;
					} else if (pLink->ped.walk_type == TYPE_WALK_ONLY) {
						if ((pLink->ped.gate_type == TYPE_GATE_CONNECTION) || (pLink->ped.gate_type == TYPE_GATE_NONE)) {
							secCost *= 0.7;
						}
					} else if (pLink->ped.walk_type == TYPE_WALK_THROUGH) {
						;
					} else {
						;
					}
				} else { //else if (opt == ROUTE_OPT_SHORTEST) {
					;
				}
				

				// 회전을 할수록 가중치 적용
				if (angCost > 0 && m_rpCost.pedestrian.cost_angle_walk[opt] > 0) {
					secCost += angCost * m_rpCost.pedestrian.cost_angle_walk[opt];
				}
				

				// 지점 주변에선는 지점 링크 속성과 동일한 링크는 가중치를 마구 높이지 않도록 하자
				if ((isNearLink) && (pLinkInfo->LinkSubInfo.ped.facility_type == pLink->ped.facility_type)) {
					secCost += 0;
				} else {
					// 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
					switch (pLink->ped.facility_type) {
					case TYPE_OBJECT_CAVE:
					{//strcat(szInfo, "토끼굴");
						break;
					}
					case TYPE_OBJECT_UNDERPASS:
					{//strcat(szInfo, "지하보도");
						if (opt != ROUTE_OPT_SHORTEST) {
							secCost += pLink->length * 2.f; // 진입 어렵게
						}
						break;
					}
					case TYPE_OBJECT_FOOTBRIDGE:
					{//strcat(szInfo, "육교");
						if (opt != ROUTE_OPT_SHORTEST) {
							secCost += pLink->length * 1.2f; // 진입 어렵게
						}
						break;
					}
					case TYPE_OBJECT_OVERPASS:
					{//strcat(szInfo, "고가도로");
						break;
					}
					case TYPE_OBJECT_BRIDGE:
					{//strcat(szInfo, "교량");
						break;
					}
					case TYPE_OBJECT_SUBWAY:
					{//strcat(szInfo, "지하철역");
						secCost += pLink->length * 10.f; // 진입 어렵게
						break;
					}
					case TYPE_OBJECT_RAILROAD:
					{//strcat(szInfo, "철도");
						secCost += pLink->length * 10.f; // 진입 어렵게
						break;
					}
					case TYPE_OBJECT_BUSSTATION:
					{//strcat(szInfo, "중앙버스정류장");
						secCost += pLink->length * 10.f; // 진입 어렵게
						break;
					}
					case TYPE_OBJECT_UNDERGROUNDMALL:
					{//strcat(szInfo, "지하상가");
						if (opt == ROUTE_OPT_SHORTEST) {
							secCost += pLink->length * 2.0f; // 진입 어렵게
						} else {
							secCost += pLink->length * 10.f; // 진입 어렵게
						}
						break;
					}
					case TYPE_OBJECT_THROUGHBUILDING:
					{//strcat(szInfo, "건물관통도로");
						if (opt == ROUTE_OPT_SHORTEST) {
							secCost += pLink->length * 2.0f; // 진입 어렵게
						} else {
							secCost += pLink->length * 10.f; // 진입 어렵게
						}					
						break;
					}
					case TYPE_OBJECT_COMPLEXPARK:
					{//strcat(szInfo, "단지도로_공원");
						if (opt == ROUTE_OPT_SHORTEST) {
							secCost += pLink->length * .2f; // 진입 어렵게
						} else {
							secCost += pLink->length * 1.2f; // 진입 어렵게
						}
						break;
					}
					case TYPE_OBJECT_COMPLEXAPT:
					{//strcat(szInfo, "단지도로_주거시설");
						if (opt == ROUTE_OPT_SHORTEST) {
							secCost += pLink->length * .2f; // 진입 어렵게
						} else {
							secCost += pLink->length * 1.2f; // 진입 어렵게
						}
						break;
					}
					case TYPE_OBJECT_COMPLEXTOUR:
					{//strcat(szInfo, "단지도로_관광지");
						if (opt == ROUTE_OPT_SHORTEST) {
							secCost += pLink->length * .2f; // 진입 어렵게
						} else {
							secCost += pLink->length * 1.2f; // 진입 어렵게
						}
						break;
					}
					case TYPE_OBJECT_COMPLEXETC:
					{//strcat(szInfo, "단지도로_기타");
						if (opt == ROUTE_OPT_SHORTEST) {
							secCost += pLink->length * 1.2f; // 진입 어렵게
						} else {
							secCost += pLink->length * 1.2f; // 진입 어렵게
						}
						break;
					}
					default:
					{
						break;
					}
					} // switch (pLink->ped.facility_type)
				}

				// 지점 주변에선는 지점 링크 속성과 동일한 링크는 가중치를 마구 높이지 않도록 하자
				if ((isNearLink) && (pLinkInfo->LinkSubInfo.ped.gate_type == pLink->ped.gate_type)) {
					secCost += 0;
				} else {
					// 진입로 타입, 0:미정의, 1:경사로, 2:계단, 3:에스컬레이터, 4:계단/에스컬레이터, 5:엘리베이터, 6:단순연결로, 7:횡단보도, 8:무빙워크, 9:징검다리, 10:의사횡단
					switch (pLink->ped.gate_type) {
					case TYPE_GATE_SLOP: {// 1 : 경사로
						break;
					}
					case TYPE_GATE_STAIRS: {// 2 : 계단
						secCost += pLink->length * 2.f; // 진입 어렵게
						break;
					}
					case TYPE_GATE_ESCALATOR: {// 3 : 에스컬레이터
						secCost += pLink->length * 2.f; // 진입 어렵게
						break;
					}
					case TYPE_GATE_STAIRS_ESCALATOR: {// 4 : 계단 / 에스컬레이터
						secCost += pLink->length * 2.f; // 진입 어렵게
						break;
					}
					case TYPE_GATE_ELEVATOR: {// 5 : 엘리베이터
						secCost += pLink->length * 2.f; // 진입 어렵게
						break;
					}
					case TYPE_GATE_CONNECTION: {// 6 : 단순연결로
						break;
					}
					case TYPE_GATE_CROSSWALK: {// 7 : 횡단보도
						//waitCost = 30; // 횡단보도는 1min정도 기다리는 시간이 주어진다.
						// 횡단 보도를 너무 자주 건너지 않도록 가중치 적용
						if ((pLink->ped.lane_count > 1) && (m_rpCost.pedestrian.cost_cross_walk[opt] > 0)) {
							waitCost += pLink->length * m_rpCost.pedestrian.cost_cross_walk[opt];
						}
						break;
					}
					case TYPE_GATE_MOVINGWALK: {// 8 : 무빙워크
						secCost += pLink->length * 100.f; // 탐색 지양
						break;
					}
					case TYPE_GATE_STEPPINGSTONES: {// 9 : 징검다리
						secCost += pLink->length * 100.f; // 탐색 지양
						break;
					}
					case TYPE_GATE_VIRTUAL: {// 10 : 의사횡단
						break;
					}
					default: { // 0:미정의
						break;
					}
					} // switch (pLink->ped.gate_type)
				}
			}


			if ((!isNearLink) && (pReqInfo->FreeOption == 0) && (pLink->ped.walk_charge != 0)) { // 유료링크
				secCost += cost * 10.f; // 지점 근처가 아닌 유료는 가능하면 우회하자
			}


			if (avoid != 0) {
				// 지점 주변에선는 지점 링크 속성과 동일한 링크는 가중치를 마구 높이지 않도록 하자
				if (((mobility == TYPE_MOBILITY_BICYCLE) && (pLink->ped.bicycle_type == TYPE_BYC_ONLY)) || 
					((isNearLink) && (pLinkInfo->LinkSubInfo.ped.gate_type == pLink->ped.gate_type))) {
					waitCost += 0;
				} else {
					// 진입로 타입
					if (((avoid & ROUTE_AVOID_PED_SLOP) == ROUTE_AVOID_PED_SLOP) && (pLink->ped.gate_type == TYPE_GATE_SLOP)) { // 경사로
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_STAIRS) == ROUTE_AVOID_PED_STAIRS) && (pLink->ped.gate_type == TYPE_GATE_STAIRS)) { // 계단
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_ESCALATOR) == ROUTE_AVOID_PED_ESCALATOR) && (pLink->ped.gate_type == TYPE_GATE_ESCALATOR)) { // 에스컬레이터
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_STAIRS_ESCALATOR) == ROUTE_AVOID_PED_STAIRS_ESCALATOR) && (pLink->ped.gate_type == TYPE_GATE_STAIRS_ESCALATOR)) { // 계단/에스컬레이터
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_ELEVATOR) == ROUTE_AVOID_PED_ELEVATOR) && (pLink->ped.gate_type == TYPE_GATE_ELEVATOR)) { // 엘리베이터
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_CONNECTION) == ROUTE_AVOID_PED_CONNECTION) && (pLink->ped.gate_type == TYPE_GATE_CONNECTION)) { // 단순연결로
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_CROSSWALK) == ROUTE_AVOID_PED_CROSSWALK) && (pLink->ped.gate_type == TYPE_GATE_CROSSWALK)) { // 횡단보도
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_MOVINGWALK) == ROUTE_AVOID_PED_MOVINGWALK) && (pLink->ped.gate_type == TYPE_GATE_MOVINGWALK)) { // 무빙워크
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_STEPPINGSTONES) == ROUTE_AVOID_PED_STEPPINGSTONES) && (pLink->ped.gate_type == TYPE_GATE_STEPPINGSTONES)) { // 징검다리
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_VIRTUAL) == ROUTE_AVOID_PED_VIRTUAL) && (pLink->ped.gate_type == TYPE_GATE_VIRTUAL)) { // 의사횡단
						waitCost += INT_MAX;
					}
				}


				// 시설물 타입
				// 지점 주변에선는 지점 링크 속성과 동일한 링크는 가중치를 마구 높이지 않도록 하자
				if (((mobility == TYPE_MOBILITY_BICYCLE) && (pLink->ped.bicycle_type == TYPE_BYC_ONLY)) || 
					((isNearLink) && (pLinkInfo->LinkSubInfo.ped.facility_type == pLink->ped.facility_type))) {
					waitCost += 0;
				} else {
					if (((avoid & ROUTE_AVOID_PED_CAVE) == ROUTE_AVOID_PED_CAVE) && (pLink->ped.facility_type == TYPE_OBJECT_CAVE)) { // 토끼굴
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_UNDERPASS) == ROUTE_AVOID_PED_UNDERPASS) && (pLink->ped.facility_type == TYPE_OBJECT_UNDERPASS)) { // 지하보도
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_FOOTBRIDGE) == ROUTE_AVOID_PED_FOOTBRIDGE) && (pLink->ped.facility_type == TYPE_OBJECT_FOOTBRIDGE)) { // 육교
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_OVERPASS) == ROUTE_AVOID_PED_OVERPASS) && (pLink->ped.facility_type == TYPE_OBJECT_OVERPASS)) { // 고가도로
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_BRIDGE) == ROUTE_AVOID_PED_BRIDGE) && (pLink->ped.facility_type == TYPE_OBJECT_BRIDGE)) { // 교량
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_SUBWAY) == ROUTE_AVOID_PED_SUBWAY) && (pLink->ped.facility_type == TYPE_OBJECT_SUBWAY)) { // 지하철역
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_RAILROAD) == ROUTE_AVOID_PED_RAILROAD) && (pLink->ped.facility_type == TYPE_OBJECT_RAILROAD)) { // 철도
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_BUSSTATION) == ROUTE_AVOID_PED_BUSSTATION) && (pLink->ped.facility_type == TYPE_OBJECT_BUSSTATION)) { // 중앙버스정류장
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_UNDERGROUNDMALL) == ROUTE_AVOID_PED_UNDERGROUNDMALL) && (pLink->ped.facility_type == TYPE_OBJECT_UNDERGROUNDMALL)) { // 지하상가
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_THROUGHBUILDING) == ROUTE_AVOID_PED_THROUGHBUILDING) && (pLink->ped.facility_type == TYPE_OBJECT_THROUGHBUILDING)) { // 건물관통도로
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_COMPLEXPARK) == ROUTE_AVOID_PED_COMPLEXPARK) && (pLink->ped.facility_type == TYPE_OBJECT_COMPLEXPARK)) { // 단지도로_공원
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_COMPLEXAPT) == ROUTE_AVOID_PED_COMPLEXAPT) && (pLink->ped.facility_type == TYPE_OBJECT_COMPLEXAPT)) { // 단지도로_주거시설
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_COMPLEXTOUR) == ROUTE_AVOID_PED_COMPLEXTOUR) && (pLink->ped.facility_type == TYPE_OBJECT_COMPLEXTOUR)) { // 단지도로_관광지
						waitCost += INT_MAX;
					}
					if (((avoid & ROUTE_AVOID_PED_COMPLEXETC) == ROUTE_AVOID_PED_COMPLEXETC) && (pLink->ped.facility_type == TYPE_OBJECT_COMPLEXETC)) { // 단지도로_기타
						waitCost += INT_MAX;
					}
				}
			}
		}
		else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE)
		{
			//if ((opt == ROUTE_OPT_FASTEST || opt == ROUTE_OPT_COMFORTABLE || opt == ROUTE_OPT_MAINROAD || mobility == TYPE_MOBILITY_AUTONOMOUS)) {
			if (opt != ROUTE_OPT_SHORTEST) {
				// 경로레벨, 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상, 4:일반도로6차선이상, 
				// 5:일반도로4차선이상, 6:일반도로2차선이상, 7:일반도로1차선이상, 8:SS도로, 9:GSS도로/단지내도로/통행금지도로/비포장도로
#if 0 // defined(USE_P2P_DATA)
				if (pLink->veh.safe_zone != 0) { // 보호구역
					secCost *= 1000;
				} else if (pLink->veh.tunnel != 0) { // 터널
					secCost *= 1000;
				} else if (pLink->veh.under_pass != 0) { // 지하도
					secCost *= 1.5;
				} else
#endif
				if (pLink->length > 0)
				{
					double avgSpd = spd;
					if (avgSpd == SPEED_NOT_AVALABLE) {
						avgSpd = GetLinkSpeed(pLink->link_id, pLink->veh.level, pReqInfo->RouteOption);
					}

#if defined(USE_P2P_DATA)
					if (pLink->veh.link_type == 8) { // 유턴
						secCost += 1000.f; // 유턴 지점은 코스트를 올려 가능하면 유턴 않도록 하자
					}
					double spdValue = avgSpd / 3.6f;
#elif defined(USE_REAL_ROUTE_TSP)
					double spdValue = avgSpd / 1.f;
#else
					double spdValue = avgSpd / 3.6f;
#endif

					secCost += pLink->length / spdValue;
				}

				if (opt == ROUTE_OPT_COMFORTABLE) {
					// 서로다른 레벨의 도로를 통과할때 코스트 부여
					//if (pLinkPrev->veh.level != pLink->veh.level) {
					//	if (pLinkPrev->veh.level > pLink->veh.level) { // 더 높은 레벨(좋은길)로 진입
					//		secCost -= (pLinkPrev->veh.level - pLink->veh.level) * secCost;
					//	} else { // 더 낮은 레벨(나쁜길)로 진입
					//		secCost += (pLink->veh.level - pLinkPrev->veh.level) * secCost;
					//	}
					//}
					if (pLink->veh.level == 0) { 
						secCost *= 0.5;
					} else if (pLink->veh.level == 1) { 
						secCost *= 0.8; 
					} else if (pLink->veh.level == 2 || pLink->veh.level == 3) {
						secCost *= 0.9; 
					} //
				}

				// angle => 180:직진, 0:유턴, 90:우회전, -90:좌회전
				// angle => 0:직진, 180:유턴, 90:우회전, 270:좌회전, (수정 후, 2023-10-13)
				int ang360 = angle;
				int ang180 = 180 - abs(180 - angle); // +-10:직진, +-170: 유턴
				//if (/*(pLink->ped.gate_type == 0) && */(abs(angle) != 180)) {
				//	// 회전을 할수록 가중치 적용
				//	//angCost = abs(180 - abs(angle)) % 180;
				//	angCost = angle % 360;
				//	if (angCost > 180) {
				//		angCost = 180 - (angCost - 180);
				//	}
				//	else if (angCost < -180) {
				//		angCost = (180 + (angCost + 180)) * -1;
				//	}

				//	if ((-180 <= angCost && angCost <= -160)
				//		|| (160 <= angCost && angCost <= 180)) { // 직진
				//		waitCost += m_rpCost.vehicle.cost_ang0;
				//	}
				//	else if (110 <= angCost && angCost <= 160) { // 우측
				//		waitCost += m_rpCost.vehicle.cost_ang45; //5;
				//	}
				//	else if (70 <= angCost && angCost <= 110) { // 우회전
				//		waitCost += m_rpCost.vehicle.cost_ang90; //20;
				//	}
				//	else if (10 <= angCost && angCost <= 70) { // 급우회전
				//		waitCost += m_rpCost.vehicle.cost_ang135;//30;
				//	}
				//	else if (-160 <= angCost && angCost <= -110) { // 급좌회전
				//		waitCost += m_rpCost.vehicle.cost_ang225;//60;
				//	}
				//	else if (-110 <= angCost && angCost <= -70) { // 좌회전
				//		waitCost += m_rpCost.vehicle.cost_ang270;//60;
				//	}
				//	else if (-70 <= angCost && angCost <= -10) { // 좌측
				//		waitCost += m_rpCost.vehicle.cost_ang315;//60;
				//	}
				//	else { // 유턴
				//		waitCost += m_rpCost.vehicle.cost_ang180;
				//	}
				//}

				// 고속화 도로 이상은 각도 보지 말자
				if ((pLink->veh.level > 1) && (pLinkPrev->veh.level > 1)) {
					if ((abs(ang180) > 5)) {
						// 회전을 할수록 가중치 적용
						int idxMobility = 0; // default vehicle
						if (mobility == TYPE_MOBILITY_TRUCK) {
							idxMobility = 1;
						} else if (mobility == TYPE_MOBILITY_EMERGENCY) {
							idxMobility = 2;
						}

						if (abs(ang180) <= 10) { // 직진
							waitCost += m_rpCost.vehicle.cost_time_ang0[pReqInfo->RouteOption];
						} else if (ang360 < 45) { // 우측
							waitCost += m_rpCost.vehicle.cost_time_ang45[pReqInfo->RouteOption];
						} else if (ang360 < 135) { // 우회전
							waitCost += m_rpCost.vehicle.cost_time_ang90[pReqInfo->RouteOption];
						} else if (ang360 < 170) { // 급우회전
							waitCost += m_rpCost.vehicle.cost_time_ang135[pReqInfo->RouteOption];
						} else if (abs(ang180) >= 170) { // 유턴
							waitCost += m_rpCost.vehicle.cost_time_ang180[pReqInfo->RouteOption];
						} else if (ang360 < 190) { // 급좌회전
							waitCost += m_rpCost.vehicle.cost_time_ang225[pReqInfo->RouteOption];
						} else if (ang360 < 315) { // 좌회전
							waitCost += m_rpCost.vehicle.cost_time_ang270[pReqInfo->RouteOption];
						} else if (ang360 < 350) { // 좌측
							waitCost += m_rpCost.vehicle.cost_time_ang315[pReqInfo->RouteOption];
						} else { // 유턴
							waitCost += m_rpCost.vehicle.cost_time_ang180[pReqInfo->RouteOption];
						}
					}
				}
			} // if 
		}
	}

	return secCost + waitCost;
}


const double CRoutePlan::GetCourseCost(IN const RouteInfo* pRouteInfo, IN const stLinkInfo* pLink, IN const double cost)
{
	double costCourse = cost;

	if (pRouteInfo == nullptr || pLink == nullptr) {
		return costCourse;
	}

#if defined(USE_FOREST_DATA)
	// 코스 데이터 가중치 부여
	int opt = pRouteInfo->RouteOption;
	if ((pLink->base.link_type == TYPE_LINK_DATA_TREKKING) && (pLink->trk_ext.course_cnt > 0) && (!pRouteInfo->CandidateCourse.empty())) {
		set<uint32_t>* psetCourse = m_pDataMgr->GetCourseByLink(pLink->link_id.llid);
		if (psetCourse != nullptr && !psetCourse->empty()) {
			for (const auto& course : pRouteInfo->CandidateCourse) {
				if (psetCourse->find(course) != psetCourse->end()) {
					if (opt == ROUTE_OPT_RECOMMENDED) {
						costCourse *= m_rpCost.pedestrian.cost_forest_course1;
					} if (opt == ROUTE_OPT_COMFORTABLE) {
						costCourse *= m_rpCost.pedestrian.cost_forest_course2;
					} else if ((opt == ROUTE_OPT_FASTEST)) {
						costCourse *= m_rpCost.pedestrian.cost_forest_course3;
					} else if ((opt == ROUTE_OPT_MAINROAD)) {
						costCourse *= m_rpCost.pedestrian.cost_forest_course4;
					} else { // if (opt == ROUTE_OPT_SHOTEST) {
						costCourse *= m_rpCost.pedestrian.cost_forest_course0;
					}
					
					break;
				}
			} // for
		}
	}
#endif

	return costCourse;
}


double getHueristicCost(IN const double departCoordX, IN const double departCoordY, IN const double destCoordX, IN const double destCoordY, IN const double nextCoordX, IN const double nextCoordY, IN const int32_t typeLinkData, IN const int32_t routeOption, IN const double costRate)
{
	double fFactor = VAL_HEURISTIC_VEHICLE_FACTOR;
#if defined(USE_OPTIMAL_POINT_API)
#	if defined(_DEBUG)
	fFactor = 0.10f;
#	else
	fFactor = VAL_HEURISTIC_VEHICLE_FACTOR;
#	endif
#elif defined(USE_FOREST_DATA)
	if (pLinkNext->base.link_type == TYPE_LINK_DATA_TREKKING) {
		fFactor = VAL_HEURISTIC_FOREST_FACTOR;
	} else {
		fFactor = VAL_HEURISTIC_PEDESTRIAN_FACTOR;
	}
#elif defined(USE_PEDESTRIAN_DATA)
	fFactor = VAL_HEURISTIC_PEDESTRIAN_FACTOR;
#else
	if (routeOption < 0) {
		fFactor = VAL_HEURISTIC_VEHICLE_FACTOR_FOR_TABLE;
	} else {
		fFactor = VAL_HEURISTIC_VEHICLE_FACTOR;
		if (routeOption == ROUTE_OPT_SHORTEST) {
			fFactor *= 0.5;
		}
	}
#endif
	double heuristicCost = 0.f;
	double newFactor = fFactor;

	const double toDepatureStraightDist = getRealWorldDistance(departCoordX, departCoordY, nextCoordX, nextCoordY);
	const double toDestStraightDist = getRealWorldDistance(destCoordX, destCoordY, nextCoordX, nextCoordY);

#if 1
	if (costRate > 0.f) {
		//newFactor = fFactor * costRate;
	}
#else
	// 출/도착지 Nm 내는 factor 작게, 그 이외에는 크게
	if (typeLinkData == TYPE_LINK_DATA_TREKKING) {
		if (toDepatureStraightDist > VAL_HEURISTIC_FACTOR_MIN_DIST && toDestStraightDist > VAL_HEURISTIC_FACTOR_MIN_DIST) {
			newFactor = fFactor * 1.5;
		}
	} else if (typeLinkData == TYPE_LINK_DATA_PEDESTRIAN) {
		if (toDepatureStraightDist > VAL_HEURISTIC_FACTOR_MIN_DIST && toDestStraightDist > VAL_HEURISTIC_FACTOR_MIN_DIST) {
			newFactor = fFactor * 3;
		}
	} else {
		newFactor = fFactor * 2;
	}
#endif

#if defined(USE_OPTIMAL_POINT_API)
#	if defined(_DEBUG)
	newFactor = 0.10f;
	heuristicCost = toDestStraightDist * newFactor;
#	else
	heuristicCost = toDestStraightDist * VAL_HEURISTIC_VEHICLE_FACTOR;
#	endif
#else
	heuristicCost = toDestStraightDist * newFactor;
#endif
	// 링크 타입 비교를 안하면 연산에 좀더 빠르려나
	//if (pLinkNext->base.link_type == TYPE_LINK_DATA_VEHICLE) {
	//	heuristicCost = static_cast<int>(toDestStraightDist * VAL_HEURISTIC_VEHICLE_FACTOR);
	//}
	//else // if (VAL_HEURISTIC_VEHICLE_FACTOR > 0)
	//{
	//	heuristicCost = static_cast<int>(toDestStraightDist * VAL_HEURISTIC_VEHICLE_FACTOR);
	//}

	return heuristicCost;
}


double getDistanceFactor(IN const double departCoordX, IN const double departCoordY, IN const double destCoordX, IN const double destCoordY, IN const stLinkInfo* pLink, IN const int32_t routeOption)
{
	double fFactor = 0;

	if (pLink == nullptr) {
		return fFactor;
	}

	const double toDepatureStraightDist = getRealWorldDistance(departCoordX, departCoordY, pLink->getVertexX(pLink->getVertexCount() / 2), pLink->getVertexY(pLink->getVertexCount() / 2));
	const double toDestStraightDist = getRealWorldDistance(destCoordX, destCoordY, pLink->getVertexX(pLink->getVertexCount() / 2), pLink->getVertexY(pLink->getVertexCount() / 2));

	/*double distFactorLimit1st = VAL_DISTANCE_FACTOR_MIN_DIST * 10;*/
	double distFactorLimit1st = VAL_DISTANCE_FACTOR_MIN_DIST * 20; // 100 km
	double distFactorLimit2nd = VAL_DISTANCE_FACTOR_MIN_DIST; // 5 km

	if (pLink->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) {
		double distFactorLimit1st = 5000; // 3 km
		double distFactorLimit2nd = 2000; // 1 km
	} else { //if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {			 
		// 최단거리는 좀더 넓은 영역을 탐색하도록 허용하자
		if (routeOption == ROUTE_OPT_SHORTEST) {
			distFactorLimit2nd *= 2;
		}
	}

	// 출/도착지 N meter 내는 factor 작게, 그 이외에는 크게
	if ((toDepatureStraightDist > distFactorLimit1st) && (toDestStraightDist > distFactorLimit1st)) {
		if (pLink->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) {
			fFactor = 10.f; // 기본 0.2 기준 VAL_HEURISTIC_PEDESTRIAN_FACTOR
		}
		else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
#if defined(USE_TMS_API)
			if (pLink->veh.level > USE_ROUTE_TABLE_LEVEL) {
				fFactor = -1;
			} else
#endif
			// 50km 이상은 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도 까지만 
			if (pLink->veh.level > 1) {
				fFactor = pLink->veh.level;
			}
		}
	} else if ((toDepatureStraightDist > distFactorLimit2nd) && (toDestStraightDist > distFactorLimit2nd)) {
		if (pLink->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) {
			fFactor = 5.f; // 기본 0.2 기준 VAL_HEURISTIC_PEDESTRIAN_FACTOR
		} else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
#if defined(USE_TMS_API)
			if (pLink->veh.level > USE_ROUTE_TABLE_LEVEL) {
				fFactor = -1;
			} else
#endif
			// 5km 이상은 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상 까지만 
			if (pLink->veh.level > 3) {
				fFactor = pLink->veh.level;
			}
		}
	} else {
		if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
#if defined(USE_TMS_API)
			if (pLink->veh.level > USE_ROUTE_TABLE_LEVEL) {
				fFactor = pLink->veh.level;
			}
#endif
		}
	}

	return fFactor;
}


double getPropagationDistanceFactor(IN const vector<SPoint>& vtOrigins, IN const int32_t currIdx, IN const int32_t nextIdx, IN const stLinkInfo* pLink, IN const int32_t routeOption)
{
	double fFactor = 0;

	if (pLink == nullptr) {
		return fFactor;
	}

	const double toDepatureStraightDist = getRealWorldDistance(vtOrigins[currIdx].x, vtOrigins[currIdx].y, pLink->getVertexX(pLink->getVertexCount() / 2), pLink->getVertexY(pLink->getVertexCount() / 2));
	const double toDestStraightDist = getRealWorldDistance(vtOrigins[nextIdx].x, vtOrigins[nextIdx].y, pLink->getVertexX(pLink->getVertexCount() / 2), pLink->getVertexY(pLink->getVertexCount() / 2));

	static double distFactorLimit1st = VAL_DISTANCE_FACTOR_MIN_DIST * 10; // 100km
	static double distFactorLimit2nd = VAL_DISTANCE_FACTOR_MIN_DIST; // 5km
	static double distFactorLimit3rd = 1000; // 1km
	//static const double distFactorLimit1st = 5000;
	//static const double distFactorLimit2nd = 3000;
	//static const double distFactorLimit1st = 100000;
	//static const double distFactorLimit2nd = 50000;
	double dwMinDist = UINT32_MAX;
	double dwMaxDist = 0;

	if (dwMinDist > toDepatureStraightDist) {
		dwMinDist = toDepatureStraightDist;
	}
	if (dwMaxDist < toDepatureStraightDist) {
		dwMaxDist = toDepatureStraightDist;
	}

	if (dwMaxDist < toDestStraightDist) {
		dwMaxDist = toDestStraightDist;
	}

	//if (dwMaxDist > distFactorLimit1st) {
	//	LOG_TRACE(LOG_DEBUG, "max dist : %.2f", dwMaxDist);
	//}

	if (dwMinDist > distFactorLimit2nd) {
		for (int ii = 0; ii < vtOrigins.size(); ii++) {
			if (ii == currIdx || ii == nextIdx) {
				continue;
			}
			double dwDist = getRealWorldDistance(vtOrigins[ii].x, vtOrigins[ii].y, pLink->getVertexX(pLink->getVertexCount() / 2), pLink->getVertexY(pLink->getVertexCount() / 2));

			if (dwMinDist > dwDist) {
				dwMinDist = dwDist;
			}

			if (dwMinDist < distFactorLimit2nd) {
				// 최소 반경값이 한개라도 있으면 무시하자
				break;
			}
		}
	}


	// 최단거리는 좀더 넓은 영역을 탐색하도록 허용하자
	//if (routeOption == ROUTE_OPT_SHORTEST) {
	//	distFactorLimit2nd *= 2;
	//}

	// 출/도착지 N meter 내는 factor 작게, 그 이외에는 크게
	if (dwMinDist > distFactorLimit1st) { // 50km 이상은 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도 까지만 		
		if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
#if defined(USE_TMS_API)
			if (pLink->veh.level > USE_ROUTE_TABLE_LEVEL) {
				fFactor = -1;
			} else
#endif
			if (pLink->veh.level > 3) {
				fFactor = 1.5;// pLink->veh.level * 5;
				//fFactor = 4;
			} else if (pLink->veh.level > 1) {
			//if (pLink->veh.level > 2) {
				fFactor = 1.2;// pLink->veh.level;
				//fFactor = 2;
			}
		}
	} else if (dwMinDist > distFactorLimit2nd) { // 5km 이상은 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상 까지만 
		// 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상, 4:일반도로6차선이상, 5:일반도로4차선이상, 6:일반도로2차선이상, 7:일반도로1차선이상, 8:SS도로, 9:GSS도로/단지내도로/통행금지도로/비포장도로
		if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
#if defined(USE_TMS_API)
			if (pLink->veh.level > USE_ROUTE_TABLE_LEVEL) {
				fFactor = -1;
			} else
#endif
			if (pLink->veh.level > 5) {
				fFactor = 1.2;
			}
		}
	} else if (dwMinDist > distFactorLimit3rd) { // 근거리는 
		if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
#if defined(USE_TMS_API)
			if (pLink->veh.level > 7) { // 7레벨 이상 도로는 무시
				fFactor = -1;
			}
#endif
		}
	} else { // 최소 거리 이내의 도로는 모든 레벨을 다 보자
		if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
#if defined(USE_TMS_API)
			if (pLink->veh.level > 8) { // 8레벨 이상 도로는 무시
				fFactor = -1;
			} else if (pLink->veh.level > USE_ROUTE_TABLE_LEVEL) {
				fFactor = 1.5;
			}
#endif
		}
	}	

	return fFactor;
}


const stNodeInfo* CRoutePlan::GetNextNode(IN const CandidateLink* pCurInfo)
{
	const stLinkInfo* pLink = nullptr;
	const stNodeInfo* pNextNode = nullptr;

	pLink = m_pDataMgr->GetLinkDataById(pCurInfo->linkId, pCurInfo->linkDataType);

	if (pLink != nullptr) { // 노드로 진출
		KeyID nextNodeId = (pCurInfo->dir == 1) ? pLink->enode_id : pLink->snode_id;

		pNextNode = m_pDataMgr->GetNodeDataById(nextNodeId, pLink->base.link_type);
	}

	return pNextNode;
}


const stNodeInfo* CRoutePlan::GetPrevNode(IN const CandidateLink* pCurInfo)
{
	const stLinkInfo* pLink = nullptr;
	const stNodeInfo* pPrevNode = nullptr;

	pLink = m_pDataMgr->GetLinkDataById(pCurInfo->linkId, pCurInfo->linkDataType);

	if (pLink != nullptr) { // 링크로 진입
		KeyID prevNodeId = (pCurInfo->dir == 1) ? pLink->snode_id : pLink->enode_id;

		pPrevNode = m_pDataMgr->GetNodeDataById(prevNodeId, pLink->base.link_type);
	}

	return pPrevNode;
}


const bool isAvoidLink(IN const CDataManager* pDataMgr, IN const stLinkInfo* pLink, IN const int32_t nAvoid, IN const stRouteSubOption nSubOption) {
	bool isAvoid = false;
	int newAvoid = nAvoid;
#if defined(USE_FOREST_DATA)
	if (nSubOption.mnt.course_type) {
		if (nSubOption.mnt.course_type != 0 && nAvoid == 0) {
			// 코스 타입 //0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
			newAvoid = ROUTE_AVOID_HIKING | ROUTE_AVOID_TRAIL | ROUTE_AVOID_BIKE | ROUTE_AVOID_CROSS | ROUTE_AVOID_RECOMMEND | ROUTE_AVOID_MTB | ROUTE_AVOID_POPULAR;

			if (nSubOption.mnt.course_type == TYPE_TRE_HIKING) { // 1:등산
				newAvoid -= ROUTE_AVOID_HIKING;
			} else if (nSubOption.mnt.course_type == TYPE_TRE_TRAIL) { // 2:걷기 - 보행자길 사용
				; // newAvoid -= ROUTE_AVOID_TRAIL;
			} else if (nSubOption.mnt.course_type == TYPE_TRE_BIKE) { // 3:자전거 - 보행자길 사용
				; // newAvoid -= ROUTE_AVOID_BIKE;
			} else { // if (nSubOption.mnt.course_type == TYPE_TRE_CROSS) { // 4:코스
				//newAvoid -= ROUTE_AVOID_HIKING;
				newAvoid -= ROUTE_AVOID_TRAIL;
				newAvoid -= ROUTE_AVOID_BIKE;
				newAvoid -= ROUTE_AVOID_CROSS;
				newAvoid -= ROUTE_AVOID_RECOMMEND;
				newAvoid -= ROUTE_AVOID_MTB;
				newAvoid -= ROUTE_AVOID_POPULAR;
			}
			//else if (nSubOption.mnt.course_type == TYPE_TRE_RECOMMENDED) { // 5:추천코스
			//	newAvoid -= ROUTE_AVOID_RECOMMEND;
			//}
			//else if (nSubOption.mnt.course_type == TYPE_TRE_MTB) { // 6:MTB코스, 보행자길 사용
			//	newAvoid -= ROUTE_AVOID_MTB;
			//}
			//else if (nSubOption.mnt.course_type == TYPE_TRE_POPULAR) { // 7:인기코스
			//	newAvoid -= ROUTE_AVOID_POPULAR;
			//}
		}
	}


	// 숲길은 등산로를 기본으로 하고, 둘레길, 자전거길, 종주길 등은 기본 경로 탐색 시 제외하자
	if ((pLink->base.link_type == TYPE_LINK_DATA_TREKKING) && (newAvoid != 0) &&		
		((pLink->trk.course_type != 0) || (pLink->trk_ext.course_cnt > 0))) {
		// 파일에는 3비트에 순서대로, 옵션은 비트별로 중복 설정 가능하기에 비트 비교 수행
		int nType = 0;
		if (pLink->trk.course_type != 0) {
			nType = (pLink->trk.course_type <= 2) ? pLink->trk.course_type : 2 << (pLink->trk.course_type - 2);			
		} else if (pLink->trk_ext.course_type != 0) {
			nType = (pLink->trk_ext.course_type <= 2) ? pLink->trk_ext.course_type : 2 << (pLink->trk_ext.course_type - 2);
		}

		if ((newAvoid & nType) == nType) {
			isAvoid = true;

			// 중용 코스 정보도 확인
			if (pLink->trk_ext.course_cnt > 0) {
				stCourseInfo courseInfo;
				set<uint32_t>* pCourse = const_cast<CDataManager*>(pDataMgr)->GetCourseByLink(pLink->link_id.llid);
				if (pCourse != nullptr) {
					for (const auto& course : *pCourse) {
						courseInfo.course_id = course;
						// 파일에는 3비트에 순서대로, 옵션은 비트별로 중복 설정 가능하기에 비트 비교 수행
						nType = (courseInfo.course_type <= 2) ? courseInfo.course_type : 2 << (courseInfo.course_type - 2);
						// 중용코스가 회피에 속하지 않으면 포함
						if ((newAvoid & nType) != nType) {
							isAvoid = false;
							break;
						}
					} // for
				}
			}
		}
	}
#endif

	return isAvoid;
}


bool checkAvoidTurnLeft(IN const RequestRouteInfo* reqInfo, IN const CandidateLink* pCurInfo, IN const stNodeInfo* pNodeNext, IN const stLinkInfo* pLinkNext, IN const int angle)
{
	// 일반 경로(최초탐색, 정차 후 출발)일 경우(대안경로 등은 무시), 출발지 바로 다음이 좌회전이고 거리가 너무 짧으면 좌회전 회피
	// 최소 회전 반경은 첫 차로에서 50m로, 이후는 30m로 // 4차선 이상 도로만 적용 // 3개 이상 연결로

	bool ret = false;

	if (reqInfo == nullptr || pCurInfo == nullptr || pNodeNext == nullptr || pLinkNext == nullptr) {
		return ret;
	}

	// 첫번째 링크가 너무 짧아 다음 링크 까지 보는데, 이전 링크에서 직진한 경우만 보자
	if ((reqInfo->RequestMode == 0) && (pCurInfo->depth <= 0 || pCurInfo->depth <= 1) &&
		(pNodeNext->base.connnode_count >= 3) && (pLinkNext->veh.lane_cnt >= 4) && (200 <= angle && angle <= 300)) {
		static const int dist1st = 50; // 최초 차로 변경 시 최소 50m 필요
		static const int dist2nd = 30; // 이후 차로 변경 시 최소 30m 필요

		int maxDist = dist1st + (pLinkNext->veh.lane_cnt / 2 - 2) * dist2nd;
		if (pLinkNext->veh.link_type == 2) { // 분리도로일 경우
			maxDist = dist1st + (pLinkNext->veh.lane_cnt - 2) * dist2nd;
		}

		if ((maxDist > pCurInfo->distTreavel) && // 너무 짧은데
			((pCurInfo->depth == 0) || // 첫 링크 이거나
			(((pCurInfo->depth == 1) && (340 <= pCurInfo->angle || pCurInfo->angle <= 20))))) { // 이전 링크에서 직진으로 이어진 경우
			ret = true;
		}
	}

	return ret;
}


int getEntryAngle(IN const int angStart, IN const int angEnd)
{
	int retAngle = 0;

	retAngle = abs(180 - angStart + angEnd + 360) % 360;

	return retAngle;
}


const int getAngle(IN const stLinkInfo* pLink, IN const int dir, IN const bool useTail = true) // dir, 1:정, 2:역, useTail, 종료 링크 사용 여부
{
	int retAng = 0;

	if (pLink == nullptr) {
		LOG_TRACE(LOG_WARNING, "warning, get link angle param null");
		return retAng;
	}

#if defined(USE_FOREST_DATA) 
	// 구조체의 자릿수 부족으로 필요 시점에 직접 계산
	MMPoint<double> coord1;
	MMPoint<double> coord2;

	int cntLinkVtx = pLink->getVertexCount();

	// ang
	if (cntLinkVtx >= 2) {
		if (dir == 1) { // 정, to enode
						// enode ang
			if (useTail == true) {
				coord1 = { pLink->getVertexX(cntLinkVtx - 1), pLink->getVertexY(cntLinkVtx - 1) };
				coord2 = { pLink->getVertexX(cntLinkVtx - 2), pLink->getVertexY(cntLinkVtx - 2) };
			} else {
				coord1 = { pLink->getVertexX(0), pLink->getVertexY(0) };
				coord2 = { pLink->getVertexX(1), pLink->getVertexY(1) };
			}
		} else { // 역, to snode			
				 // snode ang
			if (useTail == true) {
				coord1 = { pLink->getVertexX(0), pLink->getVertexY(0) };
				coord2 = { pLink->getVertexX(1), pLink->getVertexY(1) };
			} else {
				coord1 = { pLink->getVertexX(cntLinkVtx - 1), pLink->getVertexY(cntLinkVtx - 1) };
				coord2 = { pLink->getVertexX(cntLinkVtx - 2), pLink->getVertexY(cntLinkVtx - 2) };
			}
		}
		retAng = coord1.azimuth(coord2);
	}
#else // #if defined(USE_FOREST_DATA) 
	if (dir == 1) { // 정
#if defined(USE_P2P_DATA)
		if (!useTail) {
			retAng = pLink->veh_ext.snode_ang;
		} else {
			retAng = pLink->veh_ext.enode_ang;
		}
#else
		if (!useTail) {
			retAng = pLink->base.snode_ang;
		} else {
			retAng = pLink->base.enode_ang;
		}
#endif
	} else { // 역
#if defined(USE_P2P_DATA)
		if (!useTail) {
			retAng = pLink->veh_ext.enode_ang;
		} else {
			retAng = pLink->veh_ext.snode_ang;
		}
#else
		if (!useTail) {
			retAng = pLink->base.enode_ang;
		} else {
			retAng = pLink->base.snode_ang;
		}
#endif
	}
#endif // #if defined(USE_FOREST_DATA) 

	return retAng;
}


const int getPathAngle(IN const stLinkInfo* pPrevLink, IN const stLinkInfo* pNextLink)
{
	int ang = 0;
	int dir = 0;

	// 이전 링크 각도
	if ((pPrevLink->enode_id.llid == pNextLink->enode_id.llid) || (pPrevLink->enode_id.llid == pNextLink->snode_id.llid)) { // 정, 이전 링크의 enode 연결
		if (pPrevLink->link_id.dir != 2) { // 일방(역)에 시작점이 일치하면 불가
			dir = 1; // 정 - e에서 나가는
		}
	} else { // 다음 링크의 enode 연결
		if (pPrevLink->link_id.dir != 1) { // 일방(정)에 종료점이 일치하면 불가
			dir = 2; // 역 - s에서 나가는 
		}
	}

	if (dir == 0) {
		return ang;
	}

	int angStart = getAngle(pPrevLink, dir);


	// 다음 링크 각도
	if ((pPrevLink->enode_id.llid == pNextLink->snode_id.llid) || (pPrevLink->snode_id.llid == pNextLink->snode_id.llid)) { // 정, 다음 링크의 snode 연결
		if (pNextLink->link_id.dir != 2) { // 일방(역)에 시작점이 일치하면 불가
			dir = 1; // 정 - s로 들어오는 나가는
		}
	} else { // 다음 링크의 enode 연결
		if (pNextLink->link_id.dir != 1) { // 일방(정)에 종료점이 일치하면 불가
			dir = 2; // 역 - e로 들어오는 
		}
	}

	if (dir == 0) {
		return ang;
	}

	int angEnd = getAngle(pNextLink, dir, false);

	// 각도
	ang = getEntryAngle(angStart, angEnd);

	return ang;
}


const int CRoutePlan::AddNextLinks(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo)
{
	KeyID candidateId;
	int cntLinks = 0;

	candidateId.parents_id = pCurInfo->parentLinkId.nid;
	candidateId.current_id = pCurInfo->linkId.nid;

	SetVisitedLink(pRouteInfo, candidateId);

	//if (dir == 0) // 양방향
	//{
	//	cntLinks += AddNextLinks(pRouteInfo, pCurInfo, 1); // 정으로
	//	cntLinks += AddNextLinks(pRouteInfo, pCurInfo, 2); // 역으로
	//	return cntLinks;
	//}

	int altDiff = 0; // 고도차이
	double altCost = 0.f; // 고도차 가중치
	int angDiff = 0;
	int angStart = 0;
	int angEnd = 0;
	int dirTarget = 0;
	bool isTail = false;
	int isPayedLink = 0;
	int isPayedArea = ((pRouteInfo->StartLinkInfo.Payed) || (pRouteInfo->EndLinkInfo.Payed)) ? 1 : 0;

	KeyID nextNodeId = { 0, };
	stNodeInfo* pNode = nullptr;
	stNodeInfo* pNodeNext = nullptr;
	stLinkInfo* pLink = nullptr;
	stLinkInfo* pLinkNext = nullptr;

	// 현재 링크 get
	candidateId = pCurInfo->linkId;
	//candidateId.dir = 0; // 입력시에 방향성 줬기에 원데이터에서는 방향성 빼고 검색

	pNode = m_pDataMgr->GetNodeDataById(pCurInfo->nodeId, pCurInfo->linkDataType);
	pLink = m_pDataMgr->GetLinkDataById(candidateId, pCurInfo->linkDataType);

	if (!pLink)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", candidateId.nid);
		return -1;
	}

	// 다음 노드 get
	if (pCurInfo->dir == 3) { // 통행불가
		LOG_TRACE(LOG_DEBUG, "entrance disable link, link id:%d", pLink->link_id.nid);
		return -1;
	}
#if defined(USE_P2P_DATA)
	else if ((pRouteInfo->reqInfo.RouteOption == ROUTE_OPT_MAINROAD) && (pLink->veh.hd_flag == 0)) { // HD 링크와 매칭 정보가 없으면 통행 불가
		return -1;
	}
#endif

	dirTarget = pCurInfo->dir;
	if (dirTarget == 1) { // e 정방향
		if (pLink->veh.pass_code == 6) { // 일방(역)
			return 0; // 일방 정으로 향하는 링크에는 역방향 진입이 불가
		}
		nextNodeId = pLink->enode_id;
	} else if (dirTarget == 2) { // s 역방향으로
		if (pLink->veh.pass_code == 5) { // 일방(정)
			return 0; // 일방 역으로 향하는 링크에는 정방향 진입이 불가
		}
		nextNodeId = pLink->snode_id;
	} else {
		// 다음 노드를 찾아야 함.
		if (pCurInfo->nodeId.nid == pLink->snode_id.nid) {
			nextNodeId = pLink->enode_id;
			dirTarget = 1;
		} else if (pCurInfo->nodeId.nid == pLink->enode_id.nid) {
			nextNodeId = pLink->snode_id;
			dirTarget = 2;
		} else {
			LOG_TRACE(LOG_ERROR, "Failed, can't find next node, link id:%d", pLink->link_id.nid);
			return -1;
		}
	}
	angStart = getAngle(pLink, dirTarget);

	pNodeNext = m_pDataMgr->GetNodeDataById(nextNodeId, pLink->base.link_type);

	if (!pNodeNext) {
		LOG_TRACE(LOG_ERROR, "Failed, can't find next node, next node id:%d", nextNodeId.nid);
		return -1;
	}


	// 구획변교점일경우 연결 노드 구하자
	if (pNodeNext->base.point_type == TYPE_NODE_EDGE) {
		KeyID oldKey = pNodeNext->edgenode_id;

		pNodeNext = m_pDataMgr->GetNodeDataById(oldKey, pNodeNext->base.node_type);
		if (!pNodeNext) {
			// 테스트 영역 이외의 메쉬는 일부러 제외했기에 오류 메시지 출력 하지 말자
			if (g_isUseTestMesh && m_pDataMgr->GetMeshDataById(oldKey.tile_id) == nullptr) {
				return 0;
			}

			LOG_TRACE(LOG_ERROR, "Failed, can't find prev edge node, edge node tile:%d, id:%d", oldKey.tile_id, oldKey.nid);
			return -1;
		}
	}


	double heuristicCost = 0.f;
	double costReal = 0.f;
	double costTreavel = 0.f;
	double costAdditional = 0.f;

	KeyID nextCandidate = { 0, };
	CandidateLink* pItem = nullptr;
	int32_t retPassCode = 0; // 진행코드

	// 노드에 연결된 다음 링크 정보
 	for (uint32_t ii = 0; ii < pNodeNext->base.connnode_count; ii++)
	{
		if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			if ((retPassCode = getNextPassCode(pLink->link_id, pNodeNext->connnodes[ii], pNodeNext)) == PASS_CODE_DISABLE) {
				continue;
			}

#if !defined(USE_P2P_DATA) // p2p는 단점노드 유턴처럼 어려운 길은 막자
			if ((pNodeNext->base.point_type == TYPE_NODE_END) && (pNodeNext->base.connnode_count == 1)) { // 단점 노드일 경우
				if ((pLink->veh.link_type != 2) && (pLink->veh.pass_code == 1)) { // 본선분리 아니고, 일방 아닌, 통행 가능일 때
					retPassCode = PASS_CODE_UTURN; // 유턴가능 코드로 적용
					costAdditional = 1000.; // 단점 유턴의 경우, 비용을 높게 주자
				}
			} else 
#endif
			if ((pLink->link_id.llid == pNodeNext->connnodes[ii].llid) && (retPassCode != PASS_CODE_UTURN)) { // 자기 자신은 제외
				continue;
			}
		}


		pLinkNext = m_pDataMgr->GetLinkDataById(pNodeNext->connnodes[ii], pLink->base.link_type);
		if (!pLinkNext) {
			LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", pNodeNext->connnodes[ii].llid);
			continue;
		}
#if defined(USE_VEHICLE_DATA)
		else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			// 트럭 회피 옵션
			if (m_pDataMgr->IsAvoidTruckLink(pRouteInfo->reqInfo.RouteTruckOption.option(), pLinkNext)) {
				continue;
			}

#if defined(USE_P2P_DATA)
			if ((pRouteInfo->reqInfo.RouteOption == ROUTE_OPT_MAINROAD) && (pLinkNext->veh.hd_flag != 1) && // HD 링크와 매칭 정보가 없으면 통행 불가
				((pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DEFAULT) ||
					(pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT))) {
				// 출발지와 도착지는 HD 부분 링크를 허용, 하지만 경유지는 불허
				continue;
			} else if (retPassCode == PASS_CODE_UTURN) { // 유턴 코드는 불가
				continue;
			} else if ((pLinkNext->veh.link_type == 8) && (pLinkNext->veh.level > 3)) { // 지방도 이하 레벨의 유턴 도로는 금지
				continue;
			} else if ((pRouteInfo->reqInfo.RequestMode == 100) && (pLinkNext->veh.hd_flag != 1)) { // 대안경로 요청일 경우, HD만 매칭
				continue;
			}
#endif

			// 낮은 레벨은 제외
#if defined(USE_TMS_API)
			//if (pLinkNext->veh.level > USE_ROUTE_TABLE_LEVEL) { // 일반도로 이상만 따져보자
			//	continue;
			//}
#endif

			isPayedLink = pLinkNext->veh.charge;
		}
#endif
#if defined(USE_FOREST_DATA)
		else if (pLinkNext->base.link_type == TYPE_LINK_DATA_TREKKING) {
			// 숲길은 등산로를 기본으로 하고, 둘레길, 자전거길, 종주길 등은 기본 경로 탐색 시 제외하자
			bool isAvoid = isAvoidLink(m_pDataMgr, pLinkNext, pRouteInfo->AvoidOption, pRouteInfo->RouteSubOpt);
			if (isAvoid) {
				continue;
			}
		}
#endif
#if defined(USE_PEDESTRIAN_DATA)
		else if (pLinkNext->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) {
			if ((pRouteInfo->reqInfo.MobilityOption == TYPE_MOBILITY_BICYCLE) && (pLinkNext->ped.bicycle_control == 3)) {
				continue; // 자전거 통행 금지
			}

			isPayedLink = pLinkNext->ped.walk_charge;
		}
#endif

		// 무료 적용 옵션일 경우, 출도착이 무료고 유료 링크면 패스
		if ((pRouteInfo->reqInfo.FreeOption == 1) && (isPayedArea == 0) && (isPayedLink == 1)) {
			continue;
			// costAdditional = 1000.; 또는 가중치 증가 
		}


		candidateId.parents_id = pLink->link_id.nid;
		candidateId.current_id = pLinkNext->link_id.nid;

		if (!IsVisitedLink(pRouteInfo, candidateId)) { // && !IsAddedLink(pLink->link_id)) {
			if (pNodeNext->coord.x == pLinkNext->getVertexX(0) && pNodeNext->coord.y == pLinkNext->getVertexY(0)) { // 다음 링크의 snode 연결
				if (pLinkNext->link_id.dir == 2) { // 일방(역)에 시작점이 일치하면 불가
					continue;
				}
				dirTarget = 1; // 정 - s에서 나가는
			} else { // 다음 링크의 enode 연결
				if (pLinkNext->link_id.dir == 1) { // 일방(정)에 종료점이 일치하면 불가
					continue;
				}
				dirTarget = 2; // 역 - e에서 나가는 
			}
			angEnd = getAngle(pLinkNext, dirTarget, false);
			angDiff = getEntryAngle(angStart, angEnd);

#if defined(USE_P2P_DATA)
			if (checkAvoidTurnLeft(&pRouteInfo->reqInfo, pCurInfo, pNodeNext, pLinkNext, angDiff) == true) {
				continue;
			}
#endif

			// 고도 차이
			// 현재 숲길 데이터에만 존재
			if (pLinkNext->base.link_type == TYPE_LINK_DATA_TREKKING && pNodeNext->node_id != pNode->node_id) {
				altDiff = pNodeNext->trk.z_value - pNode->trk.z_value;
				if (altDiff != 0) {
					altCost = get_road_slope(static_cast<int32_t>(pLinkNext->length), altDiff);
				}
			}


			double distFactor = getDistanceFactor(pRouteInfo->StartLinkInfo.Coord.x, pRouteInfo->StartLinkInfo.Coord.y, pRouteInfo->EndLinkInfo.Coord.x, pRouteInfo->EndLinkInfo.Coord.y, pLinkNext, pRouteInfo->reqInfo.RouteOption);

#if defined(USE_TMS_API)
			if (distFactor < 0) { // rdm에서 일정 거리 이상의 낮은 도로는 무시하자					
				continue;
			}
#endif

			// 다음 링크의 끝노드 위치를 기준으로 휴리스틱 계산하는게 맞을 듯
			int nextLinkIdx = 0;
			if (dirTarget == 1) { // 정 - s에서 나가는
				nextLinkIdx = pLinkNext->getVertexCount() - 1; // 멀리있는 노드는 e
			}

			heuristicCost = getHueristicCost(pRouteInfo->StartLinkInfo.Coord.x, pRouteInfo->StartLinkInfo.Coord.y, pRouteInfo->EndLinkInfo.Coord.x, pRouteInfo->EndLinkInfo.Coord.y, pLinkNext->getVertexX(nextLinkIdx), pLinkNext->getVertexY(nextLinkIdx), pLinkNext->base.link_type, pRouteInfo->reqInfo.RouteOption, distFactor);

			uint8_t curSpeed = SPEED_NOT_AVALABLE;
			uint8_t curSpeedType = pRouteInfo->reqInfo.RequestTraffic;
#if defined(USE_VEHICLE_DATA) // && defined(USE_TRAFFIC_DATA)
			if (pLinkNext->veh.level <= 6) {
#	if USE_TRAFFIC_LINK_ATTRIBUTE // use link speed
				if (dirTarget == DIR_POSITIVE) {
					curSpeed = pLinkNext->veh_ext.spd_p;
					curSpeedType = pLinkNext->veh_ext.spd_type_p;
				} else { // if (dirTarget == DIR_NAGATIVE) {
					curSpeed = pLinkNext->veh_ext.spd_n;
					curSpeedType = pLinkNext->veh_ext.spd_type_n;
				}
#	else // use traffic map speed
				curSpeed = m_pDataMgr->GetTrafficSpeed(pLinkNext->link_id, dirTarget, pRouteInfo->reqInfo.RequestTime, curSpeedType);
#	endif // #if USE_TRAFFIC_LINK_ATTRIBUTE
			}
#endif
			// 주변 링크를 확인하고, 최대 속도를 주변 링크 속도 이하로 조정
			//if ((curSpeed == SPEED_NOT_AVALABLE) && (pCurInfo->speed != SPEED_NOT_AVALABLE) && (pLinkNext->veh.level >= pLink->veh.level)) {
			//	if (GetLinkSpeed(pLinkNext->link_id, pLinkNext->veh.level, pRouteInfo->reqInfo.RouteOption) > pCurInfo->speed) {
			//		curSpeed = pCurInfo->speed;
			//	}
			//}

			// 트리 등록
			costReal = GetCost(&pRouteInfo->reqInfo, pLinkNext, dirTarget, 0, curSpeed);
			//costTreavel = GetTravelCost(pLinkNext, pLink, curSpeed, costReal, angDiff, pRouteInfo->RouteOption, pRouteInfo->AvoidOption, pRouteInfo->MobilityOption, isPayedArea); // 이전 링크/노드(단점 유턴) 속성에 의해 비용이 높아진 상태 반영
			costTreavel = GetTravelCost(&pRouteInfo->reqInfo, pLinkNext, pLink, &pRouteInfo->EndLinkInfo, curSpeed, costReal, angDiff);
				

#if defined(USE_FOREST_DATA)
			// 코스 데이터 가중치 부여, 코스탐색옵션 전용 && 최단 제외
			if ((pRouteInfo->RouteOption != ROUTE_OPT_SHORTEST) && (pRouteInfo->RouteSubOpt.mnt.course_type == TYPE_TRE_CROSS))	{
				costTreavel = GetCourseCost(pRouteInfo, pLinkNext, costTreavel);
			}
#endif

			nextCandidate.parents_id = pCurInfo->parentLinkId.nid;
			nextCandidate.current_id = pCurInfo->linkId.nid;

			pItem = new CandidateLink;
			pItem->candidateId = nextCandidate; // 후보 ID
			pItem->parentLinkId = pCurInfo->linkId;	// 부모 링크 ID
			pItem->linkId = pLinkNext->link_id; // 링크 ID
			pItem->nodeId = pNodeNext->node_id;	// 노드 ID
			pItem->dist = pLinkNext->length;	// 실 거리
			pItem->time = costReal; // 실 시간
			pItem->cost = costTreavel + costAdditional; // 논리적 주행 시간
			pItem->distTreavel = pCurInfo->distTreavel + pItem->time;	// 누적 거리
			pItem->timeTreavel = pCurInfo->timeTreavel + costReal; // 누적 시간
			pItem->costTreavel = pCurInfo->costTreavel + pItem->cost;	// 계산 비용
			// pItem->costHeuristic = pCurInfo->costTreavel + heuristicCost;	// 가중치 계산 비용 // old
			pItem->costHeuristic = pItem->costTreavel + heuristicCost;	// 가중치 계산 비용
			pItem->linkDataType = pCurInfo->linkDataType;
			pItem->depth = pCurInfo->depth + 1;	// 탐색 깊이
			pItem->visited = false; // 방문 여부
			pItem->dir = dirTarget; // 탐색방향
			pItem->angle = angDiff; // 진입각도
#if defined(USE_VEHICLE_DATA)
			pItem->speed = curSpeed;
			pItem->speed_type = curSpeedType;
			pItem->speed_level = pLinkNext->veh.level;
#endif
			pItem->pPrevLink = const_cast<CandidateLink*>(pCurInfo);

			// 링크 방문 목록 등록
			pRouteInfo->mRoutePass.emplace(candidateId.llid, pItem);
			pRouteInfo->pqDijkstra.emplace(pItem);

			cntLinks++;

#if defined(USE_SHOW_ROUTE_SATATUS)
			//LOG_TRACE(LOG_DEBUG, "id:%lld(real cost:%f, heuristic cost:%f, lv:%d) ", newItem.linkId.llid, newItem.costReal, newItem.costHeuristic, newItem.depth);
#endif
		}
	} // for

	return cntLinks;
}


const int CRoutePlan::AddPrevLinks(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo)
{
	KeyID candidateId;
	int cntLinks = 0;

	// 역방향은 방향 바꿔서, 2025-03-18
	candidateId.parents_id = pCurInfo->linkId.nid; // current
	candidateId.current_id = pCurInfo->parentLinkId.nid; // child
	//candidateId.parents_id = pCurInfo->parentLinkId.nid;
	//candidateId.current_id = pCurInfo->linkId.nid;

	SetVisitedLink(pRouteInfo, candidateId, true);

	//if (dir == 0) // 양방향
	//{
	//	cntLinks += AddNextLinks(pRouteInfo, pCurInfo, 1); // 정으로
	//	cntLinks += AddNextLinks(pRouteInfo, pCurInfo, 2); // 역으로
	//	return cntLinks;
	//}

	int altDiff = 0; // 고도차이
	double altCost = 0.f; // 고도차 가중치
	int angDiff = 0;
	int angStart = 0;
	int angEnd = 0;
	int dirTarget = 0;
	int isPayedLink = 0;
	int isPayedArea = ((pRouteInfo->StartLinkInfo.Payed) || (pRouteInfo->EndLinkInfo.Payed)) ? 1 : 0;

	KeyID prevNodeId = { 0, };
	stNodeInfo* pNode = nullptr;
	stNodeInfo* pNodePrev = nullptr;
	stLinkInfo* pLink = nullptr;
	stLinkInfo* pLinkPrev = nullptr;

	// 현재 링크 get
	//candidateId = pCurInfo->linkId;
	//candidateId.dir = 0; // 입력시에 방향성 줬기에 원데이터에서는 방향성 빼고 검색

	pNode = m_pDataMgr->GetNodeDataById(pCurInfo->nodeId, pCurInfo->linkDataType);
	pLink = m_pDataMgr->GetLinkDataById(pCurInfo->linkId, pCurInfo->linkDataType);

	if (!pLink)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", pCurInfo->linkId.nid);
		return -1;
	}

	// 이전 노드 get
	if (pCurInfo->dir == 3) { // 통행불가
		LOG_TRACE(LOG_DEBUG, "entrance disable link, link id:%d", pLink->link_id.nid);
		return -1;
	}
#if defined(USE_P2P_DATA)
	else if (pLink->veh.hd_flag == 0) { // HD 링크와 매칭 정보가 없으면 통행 불가
		return -1;
	}
#endif

	dirTarget = pCurInfo->dir;
	if (dirTarget == 1) { // 정 - s로 진입하는
		if (pLink->veh.pass_code == 6) { // 일방(역)
			return 0; // 일방 역으로 향하는 링크에는 정방향 진입이 불가
		}
		prevNodeId = pLink->snode_id;
	} else if (dirTarget == 2) { //역 - e로 진입하는
		if (pLink->veh.pass_code == 5) { // 일방(정)
			return 0; // 일방 정으로 향하는 링크에는 역방향 진입이 불가
		}
		prevNodeId = pLink->enode_id;
	} else {
		// 다음 노드를 찾아야 함.
		if (pCurInfo->nodeId.nid == pLink->snode_id.nid) {
			prevNodeId = pLink->enode_id;
			dirTarget = 1;
		} else if (pCurInfo->nodeId.nid == pLink->enode_id.nid) {
			prevNodeId = pLink->snode_id;
			dirTarget = 2;
		} else {
			LOG_TRACE(LOG_ERROR, "Failed, can't find next node, link id:%d", pLink->link_id.nid);
			return -1;
		}
	}
	angEnd = getAngle(pLink, dirTarget, false);

	pNodePrev = m_pDataMgr->GetNodeDataById(prevNodeId, pLink->base.link_type);

	if (!pNodePrev) {
		LOG_TRACE(LOG_ERROR, "Failed, can't find prev node, node id:%d", prevNodeId.nid);
		return -1;
	}


	// 구획변교점일경우 연결 노드 구하자
	if (pNodePrev->base.point_type == TYPE_NODE_EDGE) {
		KeyID oldKey = pNodePrev->edgenode_id;

		pNodePrev = m_pDataMgr->GetNodeDataById(oldKey, pNodePrev->base.node_type);
		if (!pNodePrev) {
			// 테스트 영역 이외의 메쉬는 일부러 제외했기에 오류 메시지 출력 하지 말자
			if (g_isUseTestMesh && m_pDataMgr->GetMeshDataById(oldKey.tile_id) == nullptr) {
				return 0;
			}

			LOG_TRACE(LOG_ERROR, "Failed, can't find prev edge node, edge node tile:%d, id:%d", oldKey.tile_id, oldKey.nid);
			return -1;
		}
	}


	double heuristicCost = 0.f;
	double costReal = 0.f;
	double costTreavel = 0.f;
	double costAdditional = 0.f;
	KeyID prevCandidate = { 0, };
	CandidateLink* pItem = nullptr;
	int32_t retPassCode = 0;

	// 노드에 연결된 이전 링크 정보
	for (uint32_t ii = 0; ii < pNodePrev->base.connnode_count; ii++)
	{
		if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			if ((retPassCode = getPrevPassCode(pLink->link_id, pNodePrev->connnodes[ii], pNodePrev)) == PASS_CODE_DISABLE) {
				continue;
			}

#if !defined(USE_P2P_DATA) // p2p는 단점노드 유턴처럼 어려운 길은 막자
			if ((pNodePrev->base.point_type == TYPE_NODE_END) && (pNodePrev->base.connnode_count == 1)) { // 단점 노드일 경우
				if ((pLink->veh.link_type != 2) && (pLink->veh.pass_code == 1)) { // 본선분리 아니고, 일방 아닌, 통행 가능일 때
					retPassCode = PASS_CODE_UTURN; // 유턴가능 코드로 적용
					costAdditional = 1000.; // 단점 유턴의 경우, 비용을 높게 주자
				}
			} else 
#endif			
			if ((pLink->link_id.llid == pNodePrev->connnodes[ii].llid) && (retPassCode != PASS_CODE_UTURN)) { // (유턴이 아닌) 자기 자신은 제외
				continue;
			}
		}


		pLinkPrev = m_pDataMgr->GetLinkDataById(pNodePrev->connnodes[ii], pLink->base.link_type);
		if (!pLinkPrev) {
			LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", pNodePrev->connnodes[ii].llid);
			continue;
		}
#if defined(USE_VEHICLE_DATA)
		else if (pLinkPrev->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			// 트럭 회피 옵션
			if (m_pDataMgr->IsAvoidTruckLink(pRouteInfo->reqInfo.RouteTruckOption.option(), pLinkPrev)) {
				continue;
			}

#if defined(USE_P2P_DATA)
			if ((pLinkPrev->veh.hd_flag != 1) && // HD 링크와 매칭 정보가 없으면 통행 불가
				((pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DEFAULT) ||
					(pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT))) {
				// 출발지와 도착지는 HD 부분 링크를 허용, 하지만 경유지는 불허
				continue;
			} else if (retPassCode == PASS_CODE_UTURN) { // 유턴 코드는 불가
				continue;
			} else if ((pLinkPrev->veh.link_type == 8) && (pLinkPrev->veh.level > 3)) { // 지방도 이하 레벨의 유턴 도로는 금지
				continue;
			}
#endif

			isPayedLink = pLinkPrev->veh.charge;
		}
#endif
#if defined(USE_FOREST_DATA)
		else if (pLinkPrev->base.link_type == TYPE_LINK_DATA_TREKKING) {
			// 숲길은 등산로를 기본으로 하고, 둘레길, 자전거길, 종주길 등은 기본 경로 탐색 시 제외하자
			bool isAvoid = isAvoidLink(m_pDataMgr, pLinkPrev, pRouteInfo->AvoidOption, pRouteInfo->RouteSubOpt);
			if (isAvoid) {
				continue;
			}
		}
#endif
#if defined(USE_PEDESTRIAN_DATA)
		else if (pLinkPrev->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) {
			if ((pRouteInfo->reqInfo.MobilityOption == TYPE_MOBILITY_BICYCLE) && (pLinkPrev->ped.bicycle_control == 3)) {
				continue; // 자전거 통행 금지
			}

			isPayedLink = pLinkPrev->ped.walk_charge;
		}
#endif

		// 무료 적용 옵션일 경우, 출도착이 무료고 유료 링크면 패스
		if ((pRouteInfo->reqInfo.FreeOption == 1) && (isPayedArea == 0) && (isPayedLink == 1)) {
			continue;
			// costAdditional = 1000.; 또는 가중치 증가 
		}


		// 역방향은 순서 바뀌어야 할 듯, 2025-03-18
		candidateId.parents_id = pLinkPrev->link_id.nid;
		candidateId.current_id = pLink->link_id.nid; 
		//candidateId.parents_id = pLink->link_id.nid;
		//candidateId.current_id = pLinkPrev->link_id.nid;

		if (!IsVisitedLink(pRouteInfo, candidateId, true)) { // && !IsAddedLink(pLink->link_id)) {
			if (pNodePrev->coord.x == pLinkPrev->getVertexX(0) && pNodePrev->coord.y == pLinkPrev->getVertexY(0)) { // 다음 링크의 snode 연결
				if (pLinkPrev->link_id.dir == 2) { // 일방(정)에 종료점이 일치하면 불가
					continue;
				}
				dirTarget = 2; // 역 - s로 들어오는
			} else { // 다음 링크의 enode 연결
				if (pLinkPrev->link_id.dir == 1) { // 일방(역)에 시작점이 일치하면 불가
					continue;
				}
				dirTarget = 1; // 정 - e로 들어오는
			}
			angStart = getAngle(pLinkPrev, dirTarget);
			angDiff = getEntryAngle(angStart, angEnd);

#if defined(USE_P2P_DATA)
			if (checkAvoidTurnLeft(&pRouteInfo->reqInfo, pCurInfo, pNodePrev, pLinkPrev, angDiff) == true) {
				continue;
			}
#endif

			// 고도 차이
			// 현재 숲길 데이터에만 존재
			if (pLinkPrev->base.link_type == TYPE_LINK_DATA_TREKKING && pNodePrev->node_id != pNode->node_id) {
				altDiff = pNode->trk.z_value - pNodePrev->trk.z_value;
				if (altDiff != 0) {
					altCost = get_road_slope(static_cast<int32_t>(pLinkPrev->length), altDiff);
				}
			}


			double distFactor = getDistanceFactor(pRouteInfo->StartLinkInfo.Coord.x, pRouteInfo->StartLinkInfo.Coord.y, pRouteInfo->EndLinkInfo.Coord.x, pRouteInfo->EndLinkInfo.Coord.y, pLinkPrev, pRouteInfo->reqInfo.RouteOption);

#if defined(USE_TMS_API)
			if (distFactor < 0) { // rdm에서 일정 거리 이상의 낮은 도로는 무시하자					
				continue;
			}
#endif

			// 이전 링크의 끝노드 위치를 기준으로 휴리스틱 계산하는게 맞을 듯, next와 다르게 방향성 맞는지 한번 확인 필요(2025-02-07)
			int nextLinkIdx = 0;
			if (dirTarget == 2) { // 역 - s로 들어오는
				nextLinkIdx = pLinkPrev->getVertexCount() - 1; // 멀리있는 노드는 e
			}

			heuristicCost = getHueristicCost(pRouteInfo->StartLinkInfo.Coord.x, pRouteInfo->StartLinkInfo.Coord.y, pRouteInfo->EndLinkInfo.Coord.x, pRouteInfo->EndLinkInfo.Coord.y, pLinkPrev->getVertexX(nextLinkIdx), pLinkPrev->getVertexY(nextLinkIdx), pLinkPrev->base.link_type, pRouteInfo->reqInfo.RouteOption, distFactor);

			uint8_t curSpeed = SPEED_NOT_AVALABLE;
			uint8_t curSpeedType = pRouteInfo->reqInfo.RequestTraffic;
#if defined(USE_VEHICLE_DATA) // && defined(USE_TRAFFIC_DATA)
			if (pLinkPrev->veh.level <= 6) {
#	if USE_TRAFFIC_LINK_ATTRIBUTE // use link speed
				if (dirTarget == DIR_POSITIVE) {
					curSpeed = pLinkPrev->veh_ext.spd_p;
					curSpeedType = pLinkPrev->veh_ext.spd_type_p;
				} else { // if (dirTarget == DIR_NAGATIVE) {
					curSpeed = pLinkPrev->veh_ext.spd_n;
					curSpeedType = pLinkPrev->veh_ext.spd_type_n;
				}
#	else // use traffic map speed
				curSpeed = m_pDataMgr->GetTrafficSpeed(pLinkPrev->link_id, dirTarget, pRouteInfo->reqInfo.RequestTime, curSpeedType);
#	endif // #if USE_TRAFFIC_LINK_ATTRIBUTE
			}
#endif
			// 주변 링크를 확인하고, 최대 속도를 주변 링크 속도 이하로 조정
			//if ((curSpeed == SPEED_NOT_AVALABLE) && (pCurInfo->speed != SPEED_NOT_AVALABLE) && (pLinkPrev->veh.level >= pLink->veh.level)) {
			//	if (GetLinkSpeed(pLinkPrev->link_id, pLinkPrev->veh.level, pRouteInfo->reqInfo.RouteOption) > pCurInfo->speed) {
			//		curSpeed = pCurInfo->speed;
			//	}
			//}

			// 트리 등록
			costReal = GetCost(&pRouteInfo->reqInfo, pLinkPrev, dirTarget, 0, curSpeed);
			//costTreavel = GetTravelCost(pLinkPrev, pLink, curSpeed, costReal, angDiff, pRouteInfo->RouteOption, pRouteInfo->AvoidOption, pRouteInfo->MobilityOption, isPayedArea); // 이전 링크/노드(단점 유턴) 속성에 의해 비용이 높아진 상태 반영
			costTreavel = GetTravelCost(&pRouteInfo->reqInfo, pLinkPrev, pLink, &pRouteInfo->StartLinkInfo, curSpeed, costReal, angDiff);


#if defined(USE_FOREST_DATA)
			// 코스 데이터 가중치 부여, 코스탐색옵션 전용 && 최단 제외
			if ((pRouteInfo->RouteOption != ROUTE_OPT_SHORTEST) && (pRouteInfo->RouteSubOpt.mnt.course_type == TYPE_TRE_CROSS)) {
				costTreavel = GetCourseCost(pRouteInfo, pLinkPrev, costTreavel);
			}
#endif

			// 역방향은 순서 바뀌어야 할 듯, 2025-03-18
			prevCandidate.parents_id = pCurInfo->linkId.nid;
			prevCandidate.current_id = pCurInfo->parentLinkId.nid;
			//nextCandidate.parents_id = pCurInfo->parentLinkId.nid;
			//nextCandidate.current_id = pCurInfo->linkId.nid;

			pItem = new CandidateLink;
			pItem->candidateId = prevCandidate; // 후보 ID
			pItem->parentLinkId = pCurInfo->linkId;	// 자식 링크 ID
			pItem->linkId = pLinkPrev->link_id; // 링크 ID
			pItem->nodeId = pNodePrev->node_id;	// 노드 ID
			pItem->dist = pLinkPrev->length;	// 실 거리
			pItem->time = costReal; // 실 시간
			pItem->cost = costTreavel + costAdditional; // 논리적 주행 시간
			pItem->distTreavel = pCurInfo->distTreavel + pItem->dist;	// 누적 거리
			pItem->timeTreavel = pCurInfo->timeTreavel + pItem->time; // 누적 시간
			pItem->costTreavel = pCurInfo->costTreavel + pItem->cost;	// 계산 비용
			// pItem->costHeuristic = pCurInfo->costTreavel + heuristicCost;	// 가중치 계산 비용 // old
			pItem->costHeuristic = pItem->costTreavel + heuristicCost;	// 가중치 계산 비용
			pItem->linkDataType = pCurInfo->linkDataType;
			pItem->depth = pCurInfo->depth + 1;	// 탐색 깊이
			pItem->visited = false; // 방문 여부
			pItem->dir = dirTarget; // 탐색방향
			pItem->angle = angDiff; // 진입각도
#if defined(USE_VEHICLE_DATA)
			pItem->speed = curSpeed;
			pItem->speed_type = curSpeedType;
			pItem->speed_level = pLinkPrev->veh.level;
#endif
			pItem->pPrevLink = const_cast<CandidateLink*>(pCurInfo);

			// 링크 방문 목록 등록
			pRouteInfo->mRouteReversePass.emplace(candidateId.llid, pItem);				
			pRouteInfo->pqDijkstra.emplace(pItem);

			cntLinks++;

#if defined(USE_SHOW_ROUTE_SATATUS)
			//LOG_TRACE(LOG_DEBUG, "id:%lld(real cost:%f, heuristic cost:%f, lv:%d) ", newItem.linkId.llid, newItem.costReal, newItem.costHeuristic, newItem.depth);
#endif
		}
	} // for

	return cntLinks;
}


const int CRoutePlan::AddPrevLinksEx(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo)
{
	KeyID candidateId;
	int cntLinks = 0;

	// 역방향은 방향 바꿔서, 2025-03-18
	candidateId.parents_id = pCurInfo->linkId.nid; // current
	candidateId.current_id = pCurInfo->parentLinkId.nid; // child
	//candidateId.parents_id = pCurInfo->parentLinkId.nid;
	//candidateId.current_id = pCurInfo->linkId.nid;

	SetVisitedLink(pRouteInfo, candidateId, true);

	//if (dir == 0) // 양방향
	//{
	//	cntLinks += AddNextLinks(pRouteInfo, pCurInfo, 1); // 정으로
	//	cntLinks += AddNextLinks(pRouteInfo, pCurInfo, 2); // 역으로
	//	return cntLinks;
	//}

	int altDiff = 0; // 고도차이
	double altCost = 0.f; // 고도차 가중치
	int angDiff = 0;
	int angStart = 0;
	int angEnd = 0;
	int dirTarget = 0;
	int isPayedLink = 0;
	int isPayedArea = ((pRouteInfo->StartLinkInfo.Payed) || (pRouteInfo->EndLinkInfo.Payed)) ? 1 : 0;

	KeyID prevNodeId = { 0, };
	stNodeInfo* pNode = nullptr;
	stNodeInfo* pNodePrev = nullptr;
	stLinkInfo* pLink = nullptr;
	stLinkInfo* pLinkPrev = nullptr;


	// 현재 링크 get
	//candidateId = pCurInfo->linkId;
	//candidateId.dir = 0; // 입력시에 방향성 줬기에 원데이터에서는 방향성 빼고 검색

	pNode = m_pDataMgr->GetNodeDataById(pCurInfo->nodeId, pCurInfo->linkDataType);
	pLink = m_pDataMgr->GetLinkDataById(pCurInfo->linkId, pCurInfo->linkDataType);

	if (!pLink) {
		LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", pCurInfo->linkId.nid);
		return -1;
	}

	// 이전 노드 get
	if (pCurInfo->dir == 3) { // 통행불가
		LOG_TRACE(LOG_DEBUG, "entrance disable link, link id:%d", pLink->link_id.nid);
		return -1;
	}
#if defined(USE_P2P_DATA)
	else if (pLink->veh.hd_flag == 0) { // HD 링크와 매칭 정보가 없으면 통행 불가
		return -1;
	}
#endif

	dirTarget = pCurInfo->dir;
	if (dirTarget == 1) { // 정 - s로 진입하는
		if (pLink->veh.pass_code == 6) { // 일방(역)
			return 0; // 일방 역으로 향하는 링크에는 정방향 진입이 불가
		}
		prevNodeId = pLink->snode_id;
	} else if (dirTarget == 2) { //역 - e로 진입하는
		if (pLink->veh.pass_code == 5) { // 일방(정)
			return 0; // 일방 정으로 향하는 링크에는 역방향 진입이 불가
		} 
		prevNodeId = pLink->enode_id;
	} else {
		// 다음 노드를 찾아야 함.
		if (pCurInfo->nodeId.nid == pLink->snode_id.nid) {
			prevNodeId = pLink->enode_id;
			dirTarget = 1;
		} else if (pCurInfo->nodeId.nid == pLink->enode_id.nid) {
			prevNodeId = pLink->snode_id;
			dirTarget = 2;
		} else {
			LOG_TRACE(LOG_ERROR, "Failed, can't find next node, link id:%d", pLink->link_id.nid);
			return -1;
		}
	}
	angEnd = getAngle(pLink, dirTarget, false);

	pNodePrev = m_pDataMgr->GetNodeDataById(prevNodeId, pLink->base.link_type);

	if (!pNodePrev) {
		LOG_TRACE(LOG_ERROR, "Failed, can't find prev node, node id:%d", prevNodeId.nid);
		return -1;
	}


	// 구획변교점일경우 연결 노드 구하자
#if 0
	// 왜 이런 구문을 넣었지????
	for (int cntLoop = 0; (pNodePrev->base.point_type == TYPE_NODE_EDGE) && (cntLoop < 10); cntLoop++) {
		KeyID oldKey = pNodePrev->edgenode_id;

		pNodePrev = m_pDataMgr->GetNodeDataById(oldKey, pNodePrev->base.node_type);

		if (!pNodePrev) {
			// 테스트 영역 이외의 메쉬는 일부러 제외했기에 오류 메시지 출력 하지 말자
			if (g_isUseTestMesh && m_pDataMgr->GetMeshDataById(oldKey.tile_id) == nullptr) {
				return 0;
			}

			LOG_TRACE(LOG_ERROR, "Failed, can't find prev edge node, edge node tile:%d, id:%d", oldKey.tile_id, oldKey.nid);
			return -1;
		} else if (pNodePrev->base.point_type == TYPE_NODE_EDGE) {
			LOG_TRACE(LOG_DEBUG, "this link is has all edge node, loop:%d, old tile:%d, old id:%d, new tile:%d, new id:%d", cntLoop, oldKey.tile_id, oldKey.nid, pNodePrev->node_id.tile_id, pNodePrev->node_id.nid);
		}
	}
#else
	if (pNodePrev->base.point_type == TYPE_NODE_EDGE) {
		KeyID oldKey = pNodePrev->edgenode_id;

		pNodePrev = m_pDataMgr->GetNodeDataById(oldKey, pNodePrev->base.node_type);
		if (!pNodePrev) {
			// 테스트 영역 이외의 메쉬는 일부러 제외했기에 오류 메시지 출력 하지 말자
			if (g_isUseTestMesh && m_pDataMgr->GetMeshDataById(oldKey.tile_id) == nullptr) {
				return 0;
			}

			LOG_TRACE(LOG_ERROR, "Failed, can't find prev edge node, edge node tile:%d, id:%d", oldKey.tile_id, oldKey.nid);
			return -1;
		}
	}
#endif


	double heuristicCost = 0.f;
	double costReal = 0.f;
	double costTreavel = 0.f;
	double costAdditional = 0.f;
	KeyID prevCandidate = { 0, };
	CandidateLink* pItem = nullptr;
	int32_t retPassCode = 0;

	// 노드에 연결된 이전 링크 정보
	for (uint32_t ii = 0; ii < pNodePrev->base.connnode_count; ii++) {
		if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			if ((retPassCode = getPrevPassCode(pLink->link_id, pNodePrev->connnodes[ii], pNodePrev)) == PASS_CODE_DISABLE) {
				continue;
			}

#if !defined(USE_P2P_DATA) // p2p는 단점노드 유턴처럼 어려운 길은 막자
			if ((pNodePrev->base.point_type == TYPE_NODE_END) && (pNodePrev->base.connnode_count == 1)) { // 단점 노드일 경우
				if ((pLink->veh.link_type != 2) && (pLink->veh.pass_code == 1)) { // 본선분리 아니고, 일방 아닌, 통행 가능일 때
					retPassCode = PASS_CODE_UTURN; // 유턴가능 코드로 적용
					costAdditional = 1000.; // 단점 유턴의 경우, 비용을 높게 주자
				}
			} else
#endif			
				if ((pLink->link_id.llid == pNodePrev->connnodes[ii].llid) && (retPassCode != PASS_CODE_UTURN)) { // (유턴이 아닌) 자기 자신은 제외
					continue;
				}
		}


		pLinkPrev = m_pDataMgr->GetLinkDataById(pNodePrev->connnodes[ii], pLink->base.link_type);
		if (!pLinkPrev) {
			LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", pNodePrev->connnodes[ii].llid);
			continue;
		}
#if defined(USE_VEHICLE_DATA)
		else if (pLinkPrev->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			// 트럭 회피 옵션
			if (m_pDataMgr->IsAvoidTruckLink(pRouteInfo->reqInfo.RouteTruckOption.option(), pLinkPrev)) {
				continue;
			}

#if defined(USE_P2P_DATA)
			if ((pLinkPrev->veh.hd_flag != 1) && // HD 링크와 매칭 정보가 없으면 통행 불가
				((pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DEFAULT) ||
					(pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT))) {
				// 출발지와 도착지는 HD 부분 링크를 허용, 하지만 경유지는 불허
				continue;
			} else if (retPassCode == PASS_CODE_UTURN) { // 유턴 코드는 불가
				continue;
			} else if ((pLinkPrev->veh.link_type == 8) && (pLinkPrev->veh.level > 3)) { // 지방도 이하 레벨의 유턴 도로는 금지
				continue;
			}
#endif

			isPayedLink = pLinkPrev->veh.charge;
		}
#endif
#if defined(USE_FOREST_DATA)
		else if (pLinkPrev->base.link_type == TYPE_LINK_DATA_TREKKING) {
			// 숲길은 등산로를 기본으로 하고, 둘레길, 자전거길, 종주길 등은 기본 경로 탐색 시 제외하자
			bool isAvoid = isAvoidLink(m_pDataMgr, pLinkPrev, pRouteInfo->AvoidOption, pRouteInfo->RouteSubOpt);
			if (isAvoid) {
				continue;
			}
		}
#endif
#if defined(USE_PEDESTRIAN_DATA)
		else if (pLinkPrev->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) {
			if ((pRouteInfo->reqInfo.MobilityOption == TYPE_MOBILITY_BICYCLE) && (pLinkPrev->ped.bicycle_control == 3)) {
				continue; // 자전거 통행 금지
			}

			isPayedLink = pLinkPrev->ped.walk_charge;
		}
#endif

		// 무료 적용 옵션일 경우, 출도착이 무료고 유료 링크면 패스
		if ((pRouteInfo->reqInfo.FreeOption == 1) && (isPayedArea == 0) && (isPayedLink == 1)) {
			continue;
			// costAdditional = 1000.; 또는 가중치 증가 
		}


		// 역방향은 순서 바뀌어야 할 듯, 2025-03-18
		candidateId.parents_id = pLinkPrev->link_id.nid;
		candidateId.current_id = pLink->link_id.nid;
		//candidateId.parents_id = pLink->link_id.nid;
		//candidateId.current_id = pLinkPrev->link_id.nid;

		if (!IsVisitedLink(pRouteInfo, candidateId, true)) { // && !IsAddedLink(pLink->link_id)) {
			if (pNodePrev->coord.x == pLinkPrev->getVertexX(0) && pNodePrev->coord.y == pLinkPrev->getVertexY(0)) { // 다음 링크의 snode 연결
				if (pLinkPrev->link_id.dir == 1) { // 일방(정)에 종료점이 일치하면 불가
					continue;
				}
				dirTarget = 2; // 역 - s로 나가는
			} else { // 다음 링크의 enode 연결
				if (pLinkPrev->link_id.dir == 2) { // 일방(역)에 시작점이 일치하면 불가
					continue;
				}
				dirTarget = 1; // 정 - e로 나가는
			}
			angStart = getAngle(pLinkPrev, dirTarget);
			angDiff = getEntryAngle(angStart, angEnd);

#if defined(USE_P2P_DATA)
			if (checkAvoidTurnLeft(&pRouteInfo->reqInfo, pCurInfo, pNodePrev, pLinkPrev, angDiff) == true) {
				continue;
			}
#endif

			// 고도 차이
			// 현재 숲길 데이터에만 존재
			if (pLinkPrev->base.link_type == TYPE_LINK_DATA_TREKKING && pNodePrev->node_id != pNode->node_id) {
				altDiff = pNode->trk.z_value - pNodePrev->trk.z_value;
				if (altDiff != 0) {
					altCost = get_road_slope(static_cast<int32_t>(pLinkPrev->length), altDiff);
				}
			}


			double distFactor = getDistanceFactor(pRouteInfo->StartLinkInfo.Coord.x, pRouteInfo->StartLinkInfo.Coord.y, pRouteInfo->EndLinkInfo.Coord.x, pRouteInfo->EndLinkInfo.Coord.y, pLinkPrev, pRouteInfo->reqInfo.RouteOption);

#if defined(USE_TMS_API)
			if (distFactor < 0) { // rdm에서 일정 거리 이상의 낮은 도로는 무시하자					
				continue;
			}
#endif

			// 이전 링크의 끝노드 위치를 기준으로 휴리스틱 계산하는게 맞을 듯, next와 다르게 방향성 맞는지 한번 확인 필요(2025-02-07)
			int nextLinkIdx = 0;
			if (dirTarget == 2) { // 역 - s로 들어오는
				nextLinkIdx = pLinkPrev->getVertexCount() - 1; // 멀리있는 노드는 e
			}

			heuristicCost = getHueristicCost(pRouteInfo->StartLinkInfo.Coord.x, pRouteInfo->StartLinkInfo.Coord.y, pRouteInfo->EndLinkInfo.Coord.x, pRouteInfo->EndLinkInfo.Coord.y, pLinkPrev->getVertexX(nextLinkIdx), pLinkPrev->getVertexY(nextLinkIdx), pLinkPrev->base.link_type, pRouteInfo->reqInfo.RouteOption, distFactor);

			uint8_t curSpeed = SPEED_NOT_AVALABLE;
			uint8_t curSpeedType = pRouteInfo->reqInfo.RequestTraffic;
#if defined(USE_VEHICLE_DATA) // && defined(USE_TRAFFIC_DATA)
			if (pLinkPrev->veh.level <= 6) {
#	if USE_TRAFFIC_LINK_ATTRIBUTE // use link speed
				if (dirTarget == DIR_POSITIVE) {
					curSpeed = pLinkPrev->veh_ext.spd_p;
					curSpeedType = pLinkPrev->veh_ext.spd_type_p;
				} else { // if (dirTarget == DIR_NAGATIVE) {
					curSpeed = pLinkPrev->veh_ext.spd_n;
					curSpeedType = pLinkPrev->veh_ext.spd_type_n;
				}
#	else // use traffic map speed
				curSpeed = m_pDataMgr->GetTrafficSpeed(pLinkPrev->link_id, dirTarget, pRouteInfo->reqInfo.RequestTime, curSpeedType);
#	endif // #if USE_TRAFFIC_LINK_ATTRIBUTE
			}
#endif
			// 주변 링크를 확인하고, 최대 속도를 주변 링크 속도 이하로 조정
			//if ((curSpeed == SPEED_NOT_AVALABLE) && (pCurInfo->speed != SPEED_NOT_AVALABLE) && (pLinkPrev->veh.level >= pLink->veh.level)) {
			//	if (GetLinkSpeed(pLinkPrev->link_id, pLinkPrev->veh.level, pRouteInfo->reqInfo.RouteOption) > pCurInfo->speed) {
			//		curSpeed = pCurInfo->speed;
			//	}
			//}

			// 트리 등록
			costReal = GetCost(&pRouteInfo->reqInfo, pLinkPrev, dirTarget, 0, curSpeed);
			//costTreavel = GetTravelCost(pLinkPrev, pLink, curSpeed, costReal, angDiff, pRouteInfo->RouteOption, pRouteInfo->AvoidOption, pRouteInfo->MobilityOption, isPayedArea); // 이전 링크/노드(단점 유턴) 속성에 의해 비용이 높아진 상태 반영
			costTreavel = GetTravelCost(&pRouteInfo->reqInfo, pLinkPrev, pLink, &pRouteInfo->StartLinkInfo, curSpeed, costReal, angDiff);


#if defined(USE_FOREST_DATA)
			// 코스 데이터 가중치 부여, 코스탐색옵션 전용 && 최단 제외
			if ((pRouteInfo->RouteOption != ROUTE_OPT_SHORTEST) && (pRouteInfo->RouteSubOpt.mnt.course_type == TYPE_TRE_CROSS)) {
				costTreavel = GetCourseCost(pRouteInfo, pLinkPrev, costTreavel);
			}
#endif

			// 역방향은 순서 바뀌어야 할 듯, 2025-03-18
			prevCandidate.parents_id = pCurInfo->linkId.nid;
			prevCandidate.current_id = pCurInfo->parentLinkId.nid;
			//nextCandidate.parents_id = pCurInfo->parentLinkId.nid;
			//nextCandidate.current_id = pCurInfo->linkId.nid;

			pItem = new CandidateLink;
			pItem->candidateId = candidateId;// prevCandidate; // 후보 ID
			pItem->parentLinkId = pCurInfo->linkId;	// 자식 링크 ID
			pItem->linkId = pLinkPrev->link_id; // 링크 ID
			pItem->nodeId = pNodePrev->node_id;	// 노드 ID
			pItem->dist = pLinkPrev->length;	// 실 거리
			pItem->time = costReal; // 실 시간
			pItem->cost = costTreavel + costAdditional; // 논리적 주행 시간
			pItem->distTreavel = pCurInfo->distTreavel + pItem->dist;	// 누적 거리
			pItem->timeTreavel = pCurInfo->timeTreavel + pItem->time; // 누적 시간
			pItem->costTreavel = pCurInfo->costTreavel + pItem->cost;	// 계산 비용
																		// pItem->costHeuristic = pCurInfo->costTreavel + heuristicCost;	// 가중치 계산 비용 // old
			pItem->costHeuristic = pItem->costTreavel + heuristicCost;	// 가중치 계산 비용
			pItem->linkDataType = pCurInfo->linkDataType;
			pItem->depth = pCurInfo->depth + 1;	// 탐색 깊이
			pItem->visited = false; // 방문 여부
			pItem->dir = dirTarget; // 탐색방향
			pItem->angle = angDiff; // 진입각도
#if defined(USE_VEHICLE_DATA)
			pItem->speed = curSpeed;
			pItem->speed_type = curSpeedType;
			pItem->speed_level = pLinkPrev->veh.level;
#endif
			pItem->pPrevLink = const_cast<CandidateLink*>(pCurInfo);

			// 링크 방문 목록 등록
			pRouteInfo->mRouteReversePass.emplace(candidateId.llid, pItem);
			pRouteInfo->pqDijkstra.emplace(pItem);

			cntLinks++;

#if defined(USE_SHOW_ROUTE_SATATUS)
			//LOG_TRACE(LOG_DEBUG, "id:%lld(real cost:%f, heuristic cost:%f, lv:%d) ", newItem.linkId.llid, newItem.costReal, newItem.costHeuristic, newItem.depth);
#endif
		}
	} // for

	return cntLinks;
}


const int CRoutePlan::AddNextCourse(IN RouteInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const set<uint64_t>* psetCourseLinks)
{
	KeyID candidateId;
	int cntLinks = 0;

	candidateId.parents_id = pCurInfo->parentLinkId.nid;
	candidateId.current_id = pCurInfo->linkId.nid;

	SetVisitedLink(pRouteInfo, candidateId);

	//if (dir == 0) // 양방향
	//{
	//	cntLinks += AddNextLinks(pRouteInfo, pCurInfo, 1); // 정으로
	//	cntLinks += AddNextLinks(pRouteInfo, pCurInfo, 2); // 역으로
	//	return cntLinks;
	//}

	int altDiff = 0; // 고도차이
	double altCost = 0.f; // 고도차 가중치
	int angDiff = 0;
	int angStart = 0;
	int angEnd = 0;
	int dirTarget = 0;
	int isPayedLink = 0;
	int isPayedArea = ((pRouteInfo->StartLinkInfo.Payed) || (pRouteInfo->EndLinkInfo.Payed)) ? 1 : 0;

	KeyID nextNodeId = { 0, };
	stNodeInfo* pNode = nullptr;
	stNodeInfo* pNodeNext = nullptr;
	stLinkInfo* pLink = nullptr;
	stLinkInfo* pLinkNext = nullptr;


	// 현재 링크 get
	candidateId = pCurInfo->linkId;
	//candidateId.dir = 0; // 입력시에 방향성 줬기에 원데이터에서는 방향성 빼고 검색

	pNode = m_pDataMgr->GetNodeDataById(pCurInfo->nodeId, pCurInfo->linkDataType);
	pLink = m_pDataMgr->GetLinkDataById(candidateId, pCurInfo->linkDataType);

	if (!pLink) {
		LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", candidateId.nid);
		return -1;
	}


	// 다음 노드 get
	if (pCurInfo->dir == 3) { // 통행불가
		LOG_TRACE(LOG_DEBUG, "entrance disable link, link id:%d", pLink->link_id.nid);
		return -1;
	}
#if defined(USE_P2P_DATA)
	else if (pLink->veh.hd_flag == 0) { // HD 링크와 매칭 정보가 없으면 통행 불가
		return -1;
	}
#endif

	dirTarget = pCurInfo->dir;
	if (dirTarget == 1) { // e 정방향
		nextNodeId = pLink->enode_id;
	} else if (dirTarget == 2) { // s 역방향으로
		nextNodeId = pLink->snode_id;
	} else {
		// 다음 노드를 찾아야 함.
		if (pCurInfo->nodeId.nid == pLink->snode_id.nid) {
			nextNodeId = pLink->enode_id;
			dirTarget = 1;
		} else if (pCurInfo->nodeId.nid == pLink->enode_id.nid) {
			nextNodeId = pLink->snode_id;
			dirTarget = 2;
		} else {
			LOG_TRACE(LOG_ERROR, "Failed, can't find next node, link id:%d", pLink->link_id.nid);
			return -1;
		}
	}
	angStart = getAngle(pLink, dirTarget);

	pNodeNext = m_pDataMgr->GetNodeDataById(nextNodeId, pLink->base.link_type);

	if (!pNodeNext) {
		LOG_TRACE(LOG_ERROR, "Failed, can't find next node, next node id:%d", nextNodeId.nid);
		return -1;
	}


	// 구획변교점일경우 연결 노드 구하자
	if (pNodeNext->base.point_type == TYPE_NODE_EDGE) {
		KeyID oldKey = pNodeNext->edgenode_id;

		pNodeNext = m_pDataMgr->GetNodeDataById(oldKey, pNodeNext->base.node_type);
		if (!pNodeNext) {
			// 테스트 영역 이외의 메쉬는 일부러 제외했기에 오류 메시지 출력 하지 말자
			if (g_isUseTestMesh && m_pDataMgr->GetMeshDataById(oldKey.tile_id) == nullptr) {
				return 0;
			}

			LOG_TRACE(LOG_ERROR, "Failed, can't find next edge node, next edge node tile:%d, id:%d", oldKey.tile_id, oldKey.nid);
			return -1;
		}
	}


	double heuristicCost = 0.f;
	double costReal = 0.f;
	double costTreavel = 0.f;
	double costAdditional = 0.f;
	KeyID nextCandidate = { 0, };
	CandidateLink* pItem = nullptr;
	int32_t retPassCode = 0;

	// 노드에 연결된 다음 링크 정보
	for (uint32_t ii = 0; ii < pNodeNext->base.connnode_count; ii++) {
		if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			if ((retPassCode = getNextPassCode(pLink->link_id, pNodeNext->connnodes[ii], pNodeNext)) == PASS_CODE_DISABLE) {
				continue;
			}

#if !defined(USE_P2P_DATA) // p2p는 단점노드 유턴처럼 어려운 길은 막자
			if ((pNodeNext->base.point_type == TYPE_NODE_END) && (pNodeNext->base.connnode_count == 1)) { // 단점 노드일 경우
				if ((pLink->veh.link_type != 2) && (pLink->veh.pass_code == 1)) { // 본선분리 아니고, 일방 아닌, 통행 가능일 때
					retPassCode = PASS_CODE_UTURN; // 유턴가능 코드로 적용
					costAdditional = 1000.; // 단점 유턴의 경우, 비용을 높게 주자
				}
			} else 
#endif				
			if ((pLink->link_id.llid == pNodeNext->connnodes[ii].llid) && (retPassCode != PASS_CODE_UTURN)) { // 자기 자신은 제외
				continue;
			}
		}


		pLinkNext = m_pDataMgr->GetLinkDataById(pNodeNext->connnodes[ii], pLink->base.link_type);
		if (!pLinkNext) {
			LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", pNodeNext->connnodes[ii].llid);
			continue;
		}
#if defined(USE_VEHICLE_DATA)
		else if (pLinkNext->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			// 트럭 회피 옵션
			if (m_pDataMgr->IsAvoidTruckLink(pRouteInfo->reqInfo.RouteTruckOption.option(), pLinkPrev)) {
				continue;
			}

#if defined(USE_P2P_DATA)
			if ((pLinkNext->veh.hd_flag != 1) && // HD 링크와 매칭 정보가 없으면 통행 불가
				((pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DEFAULT) ||
					(pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT))) {
				// 출발지와 도착지는 HD 부분 링크를 허용, 하지만 경유지는 불허
				continue;
			} else if (retPassCode == PASS_CODE_UTURN) { // 유턴 코드는 불가
				continue;
			} else if ((pLinkNext->veh.link_type == 8) && (pLinkNext->veh.level > 3)) { // 지방도 이하 레벨의 유턴 도로는 금지
				continue;
			}
#endif

			// 낮은 레벨은 제외
#if defined(USE_TMS_API)
			//if (pLinkNext->veh.level > USE_ROUTE_TABLE_LEVEL) { // 일반도로 이상만 따져보자
			//	continue;
			//}
#endif

			isPayedLink = pLinkNext->veh.charge;
		}
#endif
#if defined(USE_FOREST_DATA)
		else if (pLinkNext->base.link_type == TYPE_LINK_DATA_TREKKING) {
			// 숲길은 등산로를 기본으로 하고, 둘레길, 자전거길, 종주길 등은 기본 경로 탐색 시 제외하자
			bool isAvoid = isAvoidLink(m_pDataMgr, pLinkNext, pRouteInfo->AvoidOption, pRouteInfo->RouteSubOpt);
			if (isAvoid) {
				continue;
			}

			// 코스ID가 아니면 제외
			if (psetCourseLinks->find(pLinkNext->link_id.llid) == psetCourseLinks->end()) {
				continue;
			}
		}
#endif
#if defined(USE_PEDESTRIAN_DATA)
		else if (pLinkNext->base.link_type == TYPE_LINK_DATA_PEDESTRIAN) {
			if ((pRouteInfo->reqInfo.MobilityOption == TYPE_MOBILITY_BICYCLE) && (pLinkNext->ped.bicycle_control == 3)) {
				continue; // 자전거 통행 금지
			}

			isPayedLink = pLinkNext->ped.walk_charge;
		}
#endif

		// 무료 적용 옵션일 경우, 출도착이 무료고 유료 링크면 패스
		if ((pRouteInfo->reqInfo.FreeOption == 1) && (isPayedArea == 0) && (isPayedLink == 1)) {
			continue;
			// costAdditional = 1000.; 또는 가중치 증가 
		}


		candidateId.parents_id = pLink->link_id.nid;
		candidateId.current_id = pLinkNext->link_id.nid;

		if (!IsVisitedLink(pRouteInfo, candidateId)) { // && !IsAddedLink(pLink->link_id)) {
			if (pNodeNext->coord.x == pLinkNext->getVertexX(0) && pNodeNext->coord.y == pLinkNext->getVertexY(0)) { // 다음 링크의 snode 연결
				if (pLinkNext->link_id.dir == 2) { // 일방(역)에 시작점이 일치하면 불가
					continue;
				}
				dirTarget = 1; // 정 - s에서 나가는
			} else { // 다음 링크의 enode 연결
				if (pLinkNext->link_id.dir == 1) { // 일방(정)에 종료점이 일치하면 불가
					continue;
				}
				dirTarget = 2; // 역 - e에서 나가는
			}
			angEnd = getAngle(pLinkNext, dirTarget, false);
			angDiff = getEntryAngle(angStart, angEnd);

#if defined(USE_P2P_DATA)
			if (checkAvoidTurnLeft(&pRouteInfo->reqInfo, pCurInfo, pNodeNext, pLinkNext, angDiff) == true) {
				continue;
			}
#endif

			// 고도 차이
			// 현재 숲길 데이터에만 존재
			if (pLinkNext->base.link_type == TYPE_LINK_DATA_TREKKING && pNodeNext->node_id != pNode->node_id) {
				altDiff = pNodeNext->trk.z_value - pNode->trk.z_value;
				if (altDiff != 0) {
					altCost = get_road_slope(static_cast<int32_t>(pLinkNext->length), altDiff);
				}
			}

			// 다음 링크의 끝노드 위치를 기준으로 휴리스틱 계산하는게 맞을 듯
			int nextLinkIdx = 0;
			if (dirTarget == 1) { // 정 - s에서 나가는
				nextLinkIdx = pLinkNext->getVertexCount() - 1; // 멀리있는 노드는 e
			}
			heuristicCost = getHueristicCost(pRouteInfo->StartLinkInfo.Coord.x, pRouteInfo->StartLinkInfo.Coord.y, pRouteInfo->EndLinkInfo.Coord.x, pRouteInfo->EndLinkInfo.Coord.y, pLinkNext->getVertexX(nextLinkIdx), pLinkNext->getVertexY(nextLinkIdx), pLinkNext->base.link_type, pRouteInfo->reqInfo.RouteOption, 0);

			uint8_t curSpeed = SPEED_NOT_AVALABLE;
			uint8_t curSpeedType = pRouteInfo->reqInfo.RequestTraffic;
#if defined(USE_VEHICLE_DATA) // && defined(USE_TRAFFIC_DATA)
			if (pLinkNext->veh.level <= 6) {
#	if USE_TRAFFIC_LINK_ATTRIBUTE // use link speed
				if (dirTarget == DIR_POSITIVE) {
					curSpeed = pLinkNext->veh_ext.spd_p;
					curSpeedType = pLinkNext->veh_ext.spd_type_p;
				} else { // if (dirTarget == DIR_NAGATIVE) {
					curSpeed = pLinkNext->veh_ext.spd_n;
					curSpeedType = pLinkNext->veh_ext.spd_type_n;
				}
#	else // use traffic map speed
				curSpeed = m_pDataMgr->GetTrafficSpeed(pLinkNext->link_id, dirTarget, pRouteInfo->reqInfo.RequestTime, curSpeedType);
#	endif // #if USE_TRAFFIC_LINK_ATTRIBUTE
			}
#endif
			// 주변 링크를 확인하고, 최대 속도를 주변 링크 속도 이하로 조정
			//if ((curSpeed == SPEED_NOT_AVALABLE) && (pCurInfo->speed != SPEED_NOT_AVALABLE) && (pLinkNext->veh.level >= pLink->veh.level)) {
			//	if (GetLinkSpeed(pLinkNext->link_id, pLinkNext->veh.level, pRouteInfo->reqInfo.RouteOption) > pCurInfo->speed) {
			//		curSpeed = pCurInfo->speed;
			//	}
			//}

			// 트리 등록
			costReal = GetCost(&pRouteInfo->reqInfo, pLinkNext, dirTarget, 0, curSpeed);
			//costTreavel = GetTravelCost(pLinkNext, pLink, curSpeed, costReal, angDiff, pRouteInfo->RouteOption, pRouteInfo->AvoidOption, pRouteInfo->MobilityOption, isPayedArea); // 이전 링크/노드(단점 유턴) 속성에 의해 비용이 높아진 상태 반영
			costTreavel = GetTravelCost(&pRouteInfo->reqInfo, pLinkNext, pLink, &pRouteInfo->EndLinkInfo, curSpeed, costReal, angDiff);


			nextCandidate.parents_id = pCurInfo->parentLinkId.nid;
			nextCandidate.current_id = pCurInfo->linkId.nid;

			pItem = new CandidateLink;
			pItem->candidateId = nextCandidate; // 후보 ID
			pItem->parentLinkId = pCurInfo->linkId;	// 부모 링크 ID
			pItem->linkId = pLinkNext->link_id; // 링크 ID
			pItem->nodeId = pNodeNext->node_id;	// 노드 ID
			pItem->dist = pLinkNext->length;	// 실 거리
			pItem->time = costReal; // 실 시간
			pItem->cost = costTreavel + costAdditional; // 논리적 주행 시간
			pItem->distTreavel = pCurInfo->distTreavel + pItem->dist;	// 누적 거리
			pItem->timeTreavel = pCurInfo->timeTreavel + pItem->time; // 누적 시간
			pItem->costTreavel = pCurInfo->costTreavel + pItem->cost;	// 계산 비용
			// pItem->costHeuristic = pCurInfo->costTreavel + heuristicCost;	// 가중치 계산 비용 // old
			pItem->costHeuristic = pItem->costTreavel + heuristicCost;	// 가중치 계산 비용
			pItem->linkDataType = pCurInfo->linkDataType;
			pItem->depth = pCurInfo->depth + 1;	// 탐색 깊이
			pItem->visited = false; // 방문 여부
			pItem->dir = dirTarget; // 탐색방향
			pItem->angle = angDiff; // 진입각도

			pItem->pPrevLink = const_cast<CandidateLink*>(pCurInfo);

			// 링크 방문 목록 등록
			pRouteInfo->mRoutePass.emplace(candidateId.llid, pItem);
			pRouteInfo->pqDijkstra.emplace(pItem);

			cntLinks++;

#if defined(USE_SHOW_ROUTE_SATATUS)
			//LOG_TRACE(LOG_DEBUG, "id:%lld(real cost:%f, heuristic cost:%f, lv:%d) ", newItem.linkId.llid, newItem.costReal, newItem.costHeuristic, newItem.depth);
#endif
		}
	} // for

	return cntLinks;
}


// 단순 확장
const int CRoutePlan::Propagation(IN TableBaseInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const int dir, IN const SBox& boxExpandArea, IN const vector<SPoint>& vtOrigins, IN const int32_t currIdx, IN const int32_t nextIdx, IN const uint32_t timestamp)
{
	KeyID candidateId;
	int cntLinks = 0;

	candidateId.parents_id = pCurInfo->parentLinkId.nid;
	candidateId.current_id = pCurInfo->linkId.nid;

	unordered_map<uint64_t, CandidateLink*>::iterator it = pRouteInfo->mRoutePass.find(candidateId.llid);
	if (it != pRouteInfo->mRoutePass.end()) {
		it->second = (CandidateLink*)pCurInfo;
	}

	int altDiff = 0; // 고도차이
	double altCost = 0.f; // 고도차 가중치
	int angDiff = 0;
	int angStart = 0;
	int angEnd = 0;
	int dirTarget = 0;
	int isPayedLink = 0;	
	int isPayedArea = pRouteInfo->routeLinkInfo.Payed;

	KeyID nextNodeId = { 0, };
	stNodeInfo* pNode = nullptr;
	stNodeInfo* pNodeNext = nullptr;
	stLinkInfo* pLink = nullptr;
	stLinkInfo* pLinkNext = nullptr;


	// 현재 링크 get
	candidateId = pCurInfo->linkId;
	//candidateId.dir = 0; // 입력시에 방향성 줬기에 원데이터에서는 방향성 빼고 검색

	pNode = m_pDataMgr->GetNodeDataById(pCurInfo->nodeId, pCurInfo->linkDataType);
	pLink = m_pDataMgr->GetLinkDataById(candidateId, pCurInfo->linkDataType);

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
#endif

	dirTarget = dir;
	if (dirTarget == DIR_POSITIVE) { // e 정방향
		nextNodeId = pLink->enode_id;
	} else if (dirTarget == 2) { // s 역방향으로
		nextNodeId = pLink->snode_id;
	} else {
		// 다음 노드를 찾아야 함.
		if (pCurInfo->nodeId.nid == pLink->snode_id.nid) {
			nextNodeId = pLink->enode_id;
			dirTarget = DIR_POSITIVE; // 정 - s에서 시작
		} else if (pCurInfo->nodeId.nid == pLink->enode_id.nid) {
			nextNodeId = pLink->snode_id;
			dirTarget = DIR_NAGATIVE; // 역 - e에서 시작
		} else {
			LOG_TRACE(LOG_ERROR, "Failed, can't find next node, link id:%d", pLink->link_id.nid);
			return -1;
		}
	}
	angStart = getAngle(pLink, dirTarget);

	pNodeNext = m_pDataMgr->GetNodeDataById(nextNodeId, pLink->base.link_type);

	if (!pNodeNext) {
		LOG_TRACE(LOG_ERROR, "Failed, can't find next node, next node id:%d", nextNodeId.nid);
		return -1;
	}
	
	static const double distMargin = 5000.f / 100000.f; // 5km // 확장 범위 마진 추가
	if (!isInBox(pNodeNext->coord.x, pNodeNext->coord.y, boxExpandArea, distMargin)) {
		return -1;
	}
	
	double heuristicCost = 0.f;
	double costReal = 0.f;
	double costTreavel = 0.f;
	double costAdditional = 0.f;
	KeyID nextCandidate = { 0, };
	CandidateLink* pItem = nullptr;
	int32_t retPassCode = 0;

	// 노드에 연결된 다음 링크 정보
	for (int ii = 0; ii < pNodeNext->base.connnode_count; ii++) {
		// 구획변교점일경우 연결 노드 구하자
		if (pNodeNext->base.point_type == TYPE_NODE_EDGE) {
			KeyID oldKey = pNodeNext->edgenode_id;
			int oldType = pNodeNext->base.node_type;
#if 0
			pNodeNext = m_pDataMgr->GetNodeDataById(oldKey, oldType);
#else
			//stNodeInfo* pEdgeNode = m_pDataMgr->GetNodeDataById(oldKey, oldType);
			pNodeNext = m_pDataMgr->GetNodeDataById(oldKey, oldType);
			if (pNodeNext && pNodeNext->base.connnode_count >= 1) {
				//stLinkInfo* pConnLink = m_pDataMgr->GetLinkDataById(pEdgeNode->connnodes[0], oldType);
				pLinkNext = m_pDataMgr->GetLinkDataById(pNodeNext->connnodes[0], oldType);
				if (pLinkNext) {
					//// 반대변 노드 정보 가져오기
					//if (pLinkNext->snode_id == pEdgeNode->node_id) {
					//	pNodeNext = m_pDataMgr->GetNodeDataById(pLinkNext->enode_id, oldType);
					//} else {
					//	pNodeNext = m_pDataMgr->GetNodeDataById(pLinkNext->snode_id, oldType);
					//}
				} else {
					// 여기 들어오면 아니되오
					LOG_TRACE(LOG_ERROR, "Failed, can't find link as connected from edge node, edge node tile:%d, id:%d", oldKey.tile_id, oldKey.nid);
				}
			}
#endif

			if (!pNodeNext) {
				// 테스트 영역 이외의 메쉬는 일부러 제외했기에 오류 메시지 출력 하지 말자
				if (g_isUseTestMesh && m_pDataMgr->GetMeshDataById(oldKey.tile_id) == nullptr) {
					return 0;
				}

				LOG_TRACE(LOG_ERROR, "Failed, can't find next edge node, next edge node tile:%d, id:%d", oldKey.tile_id, oldKey.nid);
				return -1;
			}
		} else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			if ((retPassCode = getNextPassCode(pLink->link_id, pNodeNext->connnodes[ii], pNodeNext)) == PASS_CODE_DISABLE) {
				continue;
			}

#if !defined(USE_P2P_DATA) // p2p는 단점노드 유턴처럼 어려운 길은 막자
			if ((pNodeNext->base.point_type == TYPE_NODE_END) && (pNodeNext->base.connnode_count == 1)){ // 단점 노드일 경우
				if ((pLink->veh.link_type != 2) && (pLink->veh.pass_code == 1)) { // 본선분리 아니고, 일방 아닌, 통행 가능일 때
					retPassCode = PASS_CODE_UTURN; // 유턴가능 코드로 적용
					costAdditional = 1000.; // 단점 유턴의 경우, 비용을 높게 주자
				}
			} else 
#endif
			if ((pLink->link_id.llid == pNodeNext->connnodes[ii].llid) && retPassCode != PASS_CODE_UTURN) { // 자기 자신은 제외
				continue;
			}

	
			pLinkNext = m_pDataMgr->GetLinkDataById(pNodeNext->connnodes[ii], pLink->base.link_type);
			if (!pLinkNext) {
				LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", pNodeNext->connnodes[ii].llid);
				continue;
			}
			else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
				// 트럭 회피 옵션
				if (m_pDataMgr->IsAvoidTruckLink(pRouteInfo->reqInfo.RouteTruckOption.option(), pLinkNext)) {
					continue;
				}
				// 낮은 레벨은 제외
#if defined(USE_TMS_API)
			//if (pLinkNext->veh.level > USE_ROUTE_TABLE_LEVEL) { // 일반도로 이상만 따져보자
			//	continue;
			//}
#elif defined(USE_P2P_DATA)
				if (pLinkNext->veh.hd_flag != 1) { // HD 링크와 매칭 정보가 없으면 통행 불가
					continue;
				}
#endif
				isPayedLink = pLinkNext->veh.charge;
			}

			// 무료 적용 옵션일 경우, 출도착이 무료고 유료 링크면 패스
			if ((pRouteInfo->reqInfo.FreeOption == 1) && (isPayedArea == 0) && (isPayedLink == 1)) {
				continue;
				// costAdditional = 1000.; 또는 가중치 증가 
			}
		}


		candidateId.parents_id = pLink->link_id.nid;
		candidateId.current_id = pLinkNext->link_id.nid;

		// 다음 방문이 더 싼경우 싼녀석으로 갈아치우자. 2025-03-04
		static bool useReplace = true;
		static int typeCompare = 2; // 0:COST, 1:DIST, 2:TIME
		unordered_map<uint64_t, CandidateLink*>::iterator rpIt = pRouteInfo->mRoutePass.find(candidateId.llid);
		if (useReplace && (rpIt != pRouteInfo->mRoutePass.end()) && (pLinkNext->veh.link_type == 1 || pLinkNext->veh.link_type == 2)) { // 로터리 등은 제외하고, 일반 도로에서만 비교하자
			CandidateLink* pOtherInfo = rpIt->second;
			if (((typeCompare == 0) && (pCurInfo->costTreavel < pOtherInfo->costTreavel)) ||
				((typeCompare == 1) && (pCurInfo->distTreavel < pOtherInfo->distTreavel)) ||
				((typeCompare == 2) && (pCurInfo->timeTreavel < pOtherInfo->timeTreavel))) {
			//if ((pLink->link_id.nid == 1514369) && pCurInfo->costTreavel < rpIt->second->costTreavel) {
			//if (/*(pLink->link_id.nid == 1514369) && */pCurInfo->timeTreavel < pPrevInfo->timeTreavel) {
					
				// 재귀 반복되지 않는지 확인 필요
				// 키를 변경하기 위해 기존 값 삭제 후 다시 추가
				pOtherInfo->isEnable = false;
				pRouteInfo->mRoutePass.erase(candidateId.llid);
#if defined(USE_TMS_API)
				pRouteInfo->mRoutePass.emplace(pRouteInfo->cntReplaced++, pOtherInfo);
#endif
			}
		}

		// 첫 방문일 경우에만 추가
		//if (!IsVisitedLink(pRouteInfo, candidateId)) { // && !IsAddedLink(pLink->link_id)) {
		/*else */if (pRouteInfo->mRoutePass.find(candidateId.llid) == pRouteInfo->mRoutePass.end()) {
			if (pNodeNext->coord.x == pLinkNext->getVertexX(0) && pNodeNext->coord.y == pLinkNext->getVertexY(0))
			{ // 다음 링크의 snode 연결
				if (pLinkNext->link_id.dir == DIR_NAGATIVE) { // 일방(역)에 시작점이 일치하면 불가
					continue;
				}
				dirTarget = DIR_POSITIVE; // 정 - s에서 나가는
			} else { // 다음 링크의 enode 연결
				if (pLinkNext->link_id.dir == DIR_POSITIVE) { // 일방(정)에 종료점이 일치하면 불가
					continue;
				}
				dirTarget = DIR_NAGATIVE; // 역 - e에서 나가는
			}
			angEnd = getAngle(pLinkNext, dirTarget, false);
			angDiff = getEntryAngle(angStart, angEnd);

#if defined(USE_P2P_DATA)
			if (checkAvoidTurnLeft(&pRouteInfo->reqInfo, pCurInfo, pNodeNext, pLinkNext, angDiff) == true) {
				continue;
			}
#endif

			// 고도 차이
			// 현재 숲길 데이터에만 존재
			if (pLinkNext->base.link_type == TYPE_LINK_DATA_TREKKING && pNodeNext->node_id != pNode->node_id) {
				altDiff = pNodeNext->trk.z_value - pNode->trk.z_value;
				if (altDiff != 0) {
					altCost = get_road_slope(static_cast<int32_t>(pLinkNext->length), altDiff);
				}
			}


#if 0
			double distFactor = getDistanceFactor(pRouteInfo->routeLinkInfo.Coord.x, pRouteInfo->routeLinkInfo.Coord.y, vtOrigins[nextIdx].x, vtOrigins[nextIdx].y, pLinkNext, pRouteInfo->routeOption);
#else
			double distFactor = getPropagationDistanceFactor(vtOrigins, currIdx, nextIdx, pLinkNext, pRouteInfo->reqInfo.RouteOption);
#endif

#if defined(USE_TMS_API)
			if (distFactor < 0) { // rdm에서 일정 거리 이상의 낮은 도로는 무시하자
				continue;
			}
#endif

			// 다음 링크의 끝노드 위치를 기준으로 휴리스틱 계산하는게 맞을 듯
			int nextLinkIdx = 0;
			if (dirTarget == 1) { // 정 - s에서 나가는
				nextLinkIdx = pLinkNext->getVertexCount() - 1; // 멀리있는 노드는 e
			}
			heuristicCost = getHueristicCost(pRouteInfo->routeLinkInfo.Coord.x, pRouteInfo->routeLinkInfo.Coord.y, vtOrigins[nextIdx].x, vtOrigins[nextIdx].y, pLinkNext->getVertexX(nextLinkIdx), pLinkNext->getVertexY(nextLinkIdx), pLinkNext->base.link_type, -1, distFactor);

			uint8_t curSpeed = SPEED_NOT_AVALABLE;
			uint8_t curSpeedType = pRouteInfo->reqInfo.RequestTraffic;
#if defined(USE_VEHICLE_DATA) // && defined(USE_TRAFFIC_DATA)
			if (pLinkNext->veh.level <= 6) {
#	if USE_TRAFFIC_LINK_ATTRIBUTE // use link speed
				if (dirTarget == DIR_POSITIVE) {
					curSpeed = pLinkNext->veh_ext.spd_p;
					curSpeedType = pLinkNext->veh_ext.spd_type_p;
				} else { // if (dirTarget == DIR_NAGATIVE) {
					curSpeed = pLinkNext->veh_ext.spd_n;
					curSpeedType = pLinkNext->veh_ext.spd_type_n;
				}
#	else // use traffic map speed
				curSpeed = m_pDataMgr->GetTrafficSpeed(pLinkNext->link_id, dirTarget, pRouteInfo->reqInfo.RequestTime, curSpeedType);
#	endif // #if USE_TRAFFIC_LINK_ATTRIBUTE
			}
#endif
			// 주변 링크를 확인하고, 최대 속도를 주변 링크 속도 이하로 조정
			//if ((curSpeed == SPEED_NOT_AVALABLE) && (pCurInfo->speed != SPEED_NOT_AVALABLE) && (pLinkNext->veh.level >= pLink->veh.level)) {
			//	if (GetLinkSpeed(pLinkNext->link_id, pLinkNext->veh.level, pRouteInfo->reqInfo.RouteOption) > pCurInfo->speed) {
			//		curSpeed = pCurInfo->speed;
			//	}
			//}

			// 트리 등록
			costReal = GetCost(&pRouteInfo->reqInfo, pLinkNext, dirTarget, 0, curSpeed);
			//costTreavel = GetTravelCost(pLinkNext, pLink, curSpeed, costReal, angDiff, pRouteInfo->routeOption, pRouteInfo->avoidOption, pRouteInfo->mobilityOption, isPayedArea);
			costTreavel = GetTravelCost(&pRouteInfo->reqInfo, pLinkNext, pLink, &pRouteInfo->routeLinkInfo, curSpeed, costReal, angDiff);


#if defined(USE_TMS_API)
			if (distFactor > 0) { // rdm에서 일정 거리 이상의 낮은 도로는 무시하자
				costTreavel += costReal * distFactor;
			}
#endif

			nextCandidate.parents_id = pCurInfo->parentLinkId.nid;
			nextCandidate.current_id = pCurInfo->linkId.nid;

			pItem = new CandidateLink;
			pItem->candidateId = nextCandidate; // 후보 ID
			pItem->parentLinkId = pCurInfo->linkId;	// 부모 링크 ID
			pItem->linkId = pLinkNext->link_id; // 링크 ID
			pItem->nodeId = pNodeNext->node_id;	// 노드 ID
			pItem->dist = pLinkNext->length;	// 실 거리
			pItem->time = costReal; // 실 시간
			pItem->cost = costTreavel + costAdditional; // 논리적 주행 시간
			pItem->distTreavel = pCurInfo->distTreavel + pItem->dist;	// 누적 거리
			pItem->timeTreavel = pCurInfo->timeTreavel + pItem->time; // 누적 시간
			pItem->costTreavel = pCurInfo->costTreavel + pItem->cost;	// 계산 비용
			//pItem->costHeuristic = pCurInfo->costTreavel;// +heuristicCost;	// 가중치 계산 비용 // old
			pItem->costHeuristic = pItem->costTreavel + heuristicCost;	// 가중치 계산 비용
			pItem->linkDataType = pCurInfo->linkDataType;
			pItem->depth = pCurInfo->depth + 1;	// 탐색 깊이
			pItem->visited = false; // 방문 여부
			pItem->dir = dirTarget; // 탐색방향
			pItem->angle = angDiff; // 진입각도
#if defined(USE_VEHICLE_DATA)
			pItem->speed = curSpeed;
			pItem->speed_type = curSpeedType;
			pItem->speed_level = pLinkNext->veh.level;
#endif

			pItem->pPrevLink = const_cast<CandidateLink*>(pCurInfo);
			pRouteInfo->mRoutePass.emplace(candidateId.llid, pItem);
			pRouteInfo->pqDijkstra.emplace(pItem);

			cntLinks++;

#if defined(USE_SHOW_ROUTE_SATATUS)
			//LOG_TRACE(LOG_DEBUG, "id:%lld(real cost:%f, heuristic cost:%f, lv:%d) ", newItem.linkId.llid, newItem.costReal, newItem.costHeuristic, newItem.depth);
#endif
		}
	} // for

	return cntLinks;
}


// 입구점 확장
const int CRoutePlan::EdgePropagation(IN const int32_t idx, IN const bool isReverse, IN const vector<stOptimalPointInfo>& vtOptInfo, IN OUT RouteInfo* pRouteInfo, IN const vector<CandidateLink*> vtCandidateInfo/*, OUT vector<ComplexPointInfo>& vtComplexPointInfo*/)
{
	int ret = ROUTE_RESULT_FAILED;
	bool isForceBreak = false;
	// 종료 지점의 탐색 매칭 카운트 계산
	int MAX_TREE_DEPTH = 60; // 디버깅 끝나면 static으로 변경할 것, 60=서울대입구-관악산정상일때 서울대통과 안하는 정도의 값
	uint32_t cntGoal = 0; // 목적지 도달 가능한 링크 수 
	uint32_t cntGoalTouch = 0; // 목적지 도달한 링크 수
	//const uint32_t cntMaxGoalTouch = 100; // 최대 목적지 도착 검사 개수, 큰산은 1000개가 넘는 입구점이 있어 다 확인하긴 무리
	const uint32_t cntMaxGoalTouch = 30; // 최대 목적지 도착 검사 개수, 큰산은 1000(설악산14,000)개가 넘는 입구점이 있어 다 확인하긴 무리
	MultimodalPointInfo* pPointInfo = nullptr;

	if (isReverse) {
		pPointInfo = &pRouteInfo->ComplexPointReverseInfo;
	} else {
		pPointInfo = &pRouteInfo->ComplexPointInfo;
	}

	// 다익스트라 초기화
	for (; !pRouteInfo->pqDijkstra.empty(); pRouteInfo->pqDijkstra.pop());

	// 큐에 목록에 출발지로 등록
	for (auto & item : vtCandidateInfo) {
		if (isReverse) {
			pRouteInfo->mRouteReversePass.emplace(item->linkId.llid, item);
		} else {
			pRouteInfo->mRoutePass.emplace(item->linkId.llid, item);
		}
		pRouteInfo->pqDijkstra.emplace(item);
	}

	stEntryPointInfo targetEnt = { 0, };
	// 숲바운더리 입구점이 있으면, 목적 지점과 가장 가까운 입구점을 타겟으로 하고, 반드시 도달하도록 하자

	if (!vtOptInfo[idx].vtEntryPoint.empty()) {
		double minDist = INT_MAX;
		double curDist = INT_MAX;
		for (const auto& ent : vtOptInfo[idx].vtEntryPoint) {
			curDist = getRealWorldDistance(ent.x, ent.y, pRouteInfo->EndLinkInfo.Coord.x, pRouteInfo->EndLinkInfo.Coord.y);

			if (curDist < minDist) {
				minDist = curDist;
				memcpy(&targetEnt, &ent, sizeof(targetEnt));
			}
		} // for
	}


	CandidateLink* pCurInfo = nullptr;
	const stNodeInfo* pNode = nullptr;

	uint32_t cntMaxExtraSearch = 100; // 목적지 추가 도달 검사 최대 허용치
	uint32_t cntExtraSearch = 0; // 목적지 도달 후 추가 도달 검사 카운트
	uint32_t cntMntEntryPoint = 0; // 산바운더리 입구점 카운트

	// 경로 탐색
	int cntAddedLink = 0;
	int nProcess = 0;
	// debug for status

	unordered_set<uint64_t> setGoalTouchedNode;

	if (!vtOptInfo[idx].vtEntryPoint.empty()) {
		cntMntEntryPoint = vtOptInfo[idx].vtEntryPoint.size();
	}

	while (!pRouteInfo->pqDijkstra.empty()) {
		nProcess++;
		// debug for status
#if defined(_WIN32)
		routeProcessPrint(nProcess);
#endif // #if defined(_WIN32)

		// 현재 링크 트리에서 제거
		pCurInfo = pRouteInfo->pqDijkstra.top();
		pRouteInfo->pqDijkstra.pop();
		pCurInfo->visited = true;

		if (isReverse) {
			cntAddedLink = AddPrevLinks(pRouteInfo, pCurInfo);
			pNode = GetPrevNode(pCurInfo);
		} else {
			cntAddedLink = AddNextLinks(pRouteInfo, pCurInfo);
			pNode = GetNextNode(pCurInfo);
		}

		// 현재 노드가 숲길 입구점이면 입구점 등록
		if (pNode && (pNode->trk.entrance) && (pNode->edgenode_id.llid != 0)) {
			bool isAdded = false;

			if (setGoalTouchedNode.find(pNode->node_id.llid) != setGoalTouchedNode.end()) {
				// 이미 확인된 노드면
				;
			}
			else {
				setGoalTouchedNode.emplace(pNode->node_id.llid);

				// 산바운더리가 있고, 산바운더리 입구점이 있으면, 산바운더리 입구점만 등록하자
				if (cntMntEntryPoint > 0) {
					for (const auto& ent : vtOptInfo[idx].vtEntryPoint) {
						if (pNode->node_id.nid == ent.nID_1) {
							pPointInfo->vtEntryPoint.emplace_back(ent);
							isAdded = true;
							break;
						}
					} // for
				}
				else { // 전부 다 등록
					stEntryPointInfo entry;
					entry.nID_1 = pNode->node_id.llid;
					entry.nID_2 = pNode->edgenode_id.llid; //
					entry.x = pNode->coord.x;
					entry.y = pNode->coord.y;
					entry.nAttribute = TYPE_OPTIMAL_ENTRANCE_MOUNTAIN;
					pPointInfo->vtEntryPoint.emplace_back(entry);

					isAdded = true;
				}

				if (isAdded) {
					pPointInfo->vtCost.emplace_back(pCurInfo->costTreavel);
					pPointInfo->vtDist.emplace_back(pCurInfo->distTreavel);
					pPointInfo->vtTime.emplace_back(pCurInfo->timeTreavel);

					// 입구점까지의 전체 경로 저장
					pPointInfo->vtRoutePathInfo.emplace_back(pCurInfo);
					//captureResultPath(pCurInfo, vtComplexPointInfo[ii].vtRoutePathInfo);

					cntGoalTouch++;
				}
			}

			// 목표 지점에 도달 했는지 확인
			if ((cntMntEntryPoint > 0) && (targetEnt.nID_1 > 0) && (targetEnt.nID_1 == pNode->node_id.nid)) {
				isForceBreak = true;
			}
		}

		//if (((cntMntEntryPoint > 0) && (cntMntEntryPoint <= cntGoalTouch)) ||
		//	((cntMntEntryPoint <= 0) && (cntGoalTouch > 0) && (pCurInfo->depth >= MAX_TREE_DEPTH)) ||
		//	(cntMaxGoalTouch <= cntGoalTouch)) {
		//	break;
		//}
		if (isForceBreak || ((cntMntEntryPoint > 0) && (cntMntEntryPoint <= cntGoalTouch)) ||
			((cntMntEntryPoint <= 0) && (cntGoalTouch > 0) && ((pCurInfo->depth >= MAX_TREE_DEPTH) || (cntMaxGoalTouch <= cntGoalTouch))))
		{
			ret = ROUTE_RESULT_SUCCESS;
			break;
		}

		// time-out check
		if (pCurInfo->depth >= 1000) {
			LOG_TRACE(LOG_WARNING, "---------- EdgePropagation extend too far, depth: %d, queue size: %d, loop cnt: %d", pCurInfo->depth, pRouteInfo->pqDijkstra.size(), nProcess);
			break;
		}
		// 너무 많은 탐색 시도일 경우 실패 처리하자
#if defined(USE_FOREST_DATA)
		else if (pCurInfo->depth > 1000 && (pCurInfo->distTreavel > (MAX_SEARCH_DIST_FOR_COURSE * 2)))
#elif defined(USE_VEHICLE_DATA)
		else if (pCurInfo->depth > 3000 && (pCurInfo->distTreavel > (MAX_SEARCH_DIST_FOR_VEHICLE * 2)))
#else
		else if (pCurInfo->depth > 300 && (pCurInfo->distTreavel > (MAX_SEARCH_DIST_FOR_PEDESTRIAN)))
#endif
		{
			ret = ROUTE_RESULT_FAILED_EXPEND;
			break;
		}
		// error
	} // while

	return ret;
}


const int CRoutePlan::LevelPropagation(IN TableBaseInfo* pRouteInfo, IN const CandidateLink* pCurInfo, IN const int dir, IN const SBox& boxExpandArea, IN const int32_t nLimtLevel, IN OUT int32_t& enableStartLevelIgnore) // 모든 레벨 확장
{
	KeyID candidateId;
	int cntLinks = 0;

	candidateId.parents_id = pCurInfo->parentLinkId.nid;
	candidateId.current_id = pCurInfo->linkId.nid;

	// SetVisitedLink(pRouteInfo, candidateId);
	unordered_map<uint64_t, CandidateLink*>::iterator it = pRouteInfo->mRoutePass.find(candidateId.llid);
	if (it != pRouteInfo->mRoutePass.end()) {
		it->second = (CandidateLink*)pCurInfo;
	}

	int altDiff = 0; // 고도차이
	double altCost = 0.f; // 고도차 가중치
	int angDiff = 0;
	int angStart = 0;
	int angEnd = 0;
	int dirTarget = 0;
	int isPayedLink = 0;
	int isPayedArea = pRouteInfo->routeLinkInfo.Payed;

	KeyID nextNodeId = { 0, };
	stNodeInfo* pNode = nullptr;
	stNodeInfo* pNodeNext = nullptr;
	stLinkInfo* pLink = nullptr;
	stLinkInfo* pLinkNext = nullptr;


	// 현재 링크 get
	candidateId = pCurInfo->linkId;
	//candidateId.dir = 0; // 입력시에 방향성 줬기에 원데이터에서는 방향성 빼고 검색

	pNode = m_pDataMgr->GetNodeDataById(pCurInfo->nodeId, pCurInfo->linkDataType);
	pLink = m_pDataMgr->GetLinkDataById(candidateId, pCurInfo->linkDataType);

	if (!pLink) {
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
#endif

	dirTarget = dir;
	if (dirTarget == DIR_POSITIVE) { // e 정방향
		nextNodeId = pLink->enode_id;
	} else if (dirTarget == DIR_NAGATIVE) { // s 역방향으로
		nextNodeId = pLink->snode_id;
	} else {
		// 다음 노드를 찾아야 함.
		if (pCurInfo->nodeId.nid == pLink->snode_id.nid) {
			nextNodeId = pLink->enode_id;
			dirTarget = DIR_POSITIVE; // 정 - s에서 시작
		} else if (pCurInfo->nodeId.nid == pLink->enode_id.nid) {
			nextNodeId = pLink->snode_id;
			dirTarget = DIR_NAGATIVE; // 역 - e에서 시작
		} else {
			LOG_TRACE(LOG_ERROR, "Failed, can't find next node, link id:%d", pLink->link_id.nid);
			return -1;
		}
	}
	angStart = getAngle(pLink, dirTarget);

	pNodeNext = m_pDataMgr->GetNodeDataById(nextNodeId, pLink->base.link_type);

	if (!pNodeNext) {
		LOG_TRACE(LOG_ERROR, "Failed, can't find next node, next node id:%d", nextNodeId.nid);
		return -1;
	}

#if 0 //defined(_WIN32) && defined(_DEBUG)
	static const double DBG_DIST = 20000; // 30km
	if (getRealWorldDistance(pNodeNext->coord.x, pNodeNext->coord.y, vtOrigins[currIdx].x, vtOrigins[currIdx].y) > DBG_DIST) {
		return -1;
	}
#else
	static const double distMargin = 5000.f / 100000.f; // 5km // 확장 범위 마진 추가
	if (!isInBox(pNodeNext->coord.x, pNodeNext->coord.y, boxExpandArea, distMargin)) {
		return -1;
	}
#endif

	double heuristicCost = 0.f;
	double costReal = 0.f;
	double costTreavel = 0.f;
	double costAdditional = 0.f;
	KeyID nextCandidate = { 0, };
	int32_t retPassCode = 0;

	// 노드에 연결된 다음 링크 정보
	for (int ii = 0; ii < pNodeNext->base.connnode_count; ii++) {
		// 구획변교점일경우 연결 노드 구하자
		if (pNodeNext->base.point_type == TYPE_NODE_EDGE) {
			KeyID oldKey = pNodeNext->edgenode_id;
			int oldType = pNodeNext->base.node_type;
#if 0
			pNodeNext = m_pDataMgr->GetNodeDataById(oldKey, oldType);
#else
			//stNodeInfo* pEdgeNode = m_pDataMgr->GetNodeDataById(oldKey, oldType);
			pNodeNext = m_pDataMgr->GetNodeDataById(oldKey, oldType);
			if (pNodeNext && pNodeNext->base.connnode_count >= 1) {
				//stLinkInfo* pConnLink = m_pDataMgr->GetLinkDataById(pEdgeNode->connnodes[0], oldType);
				pLinkNext = m_pDataMgr->GetLinkDataById(pNodeNext->connnodes[0], oldType);
				if (pLinkNext) {
					//// 반대변 노드 정보 가져오기
					//if (pLinkNext->snode_id == pEdgeNode->node_id) {
					//	pNodeNext = m_pDataMgr->GetNodeDataById(pLinkNext->enode_id, oldType);
					//} else {
					//	pNodeNext = m_pDataMgr->GetNodeDataById(pLinkNext->snode_id, oldType);
					//}
				} else {
					// 여기 들어오면 아니되오
					LOG_TRACE(LOG_ERROR, "Failed, can't find link as connected from edge node, edge node tile:%d, id:%d", oldKey.tile_id, oldKey.nid);
				}
			}
#endif

			if (!pNodeNext) {
				// 테스트 영역 이외의 메쉬는 일부러 제외했기에 오류 메시지 출력 하지 말자
				if (g_isUseTestMesh && m_pDataMgr->GetMeshDataById(oldKey.tile_id) == nullptr) {
					return 0;
				}

				LOG_TRACE(LOG_ERROR, "Failed, can't find next edge node, next edge node tile:%d, id:%d", oldKey.tile_id, oldKey.nid);
				return -1;
			}
		} else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
			if ((retPassCode = getNextPassCode(pLink->link_id, pNodeNext->connnodes[ii], pNodeNext)) == PASS_CODE_DISABLE) {
				continue;
			}

#if !defined(USE_P2P_DATA) // p2p는 단점노드 유턴처럼 어려운 길은 막자
			if ((pNodeNext->base.point_type == TYPE_NODE_END) && (pNodeNext->base.connnode_count == 1)) { // 단점 노드일 경우
				if ((pLink->veh.link_type != 2) && (pLink->veh.pass_code == 1)) { // 본선분리 아니고, 일방 아닌, 통행 가능일 때
					retPassCode = PASS_CODE_UTURN; // 유턴가능 코드로 적용
					costAdditional = 1000.; // 단점 유턴의 경우, 비용을 높게 주자
				}
			} else
#endif
			if ((pLink->link_id.llid == pNodeNext->connnodes[ii].llid) && retPassCode != PASS_CODE_UTURN) { // 자기 자신은 제외
				continue;
			}

			pLinkNext = m_pDataMgr->GetLinkDataById(pNodeNext->connnodes[ii], pLink->base.link_type);
			if (!pLinkNext) {
				LOG_TRACE(LOG_ERROR, "Failed, can't find link, id:%d", pNodeNext->connnodes[ii].llid);
				continue;
			}

#if 1
			if (enableStartLevelIgnore) {
				static const double MARGIN_DIST = 500;// 1000; // 1km 넘어가면 낮은 레벨 버리자
				if ((pLinkNext->veh.level <= nLimtLevel) &&
					(getRealWorldDistance(pNodeNext->coord.x, pNodeNext->coord.y, pRouteInfo->routeLinkInfo.MatchCoord.x, pRouteInfo->routeLinkInfo.MatchCoord.y) > MARGIN_DIST)) {
					// 특정 거리이상에서 지정 레벨 이하에 도달하면 레벨 무시 옵션 제거하자.
					enableStartLevelIgnore = 0;
				}
			} else { // 낮은 레벨은 제외 But, 램프/연결로는 허용하자 (메인에서 2단계 까지 하위 허용)
				if ((pLinkNext->veh.level > nLimtLevel) && (pLink->veh.level > nLimtLevel)) {
#if defined(USE_VEHICLE_DATA)
					// 메인에서 2단계 까지 하위 허용, // 램프추가시 시간 2배 소요됨??? (500 기준), 2025-04-01
					if ((pLinkNext->veh.link_type != 5) && (!pCurInfo->pPrevLink || (pCurInfo->pPrevLink->speed_level > nLimtLevel))) {
						continue;
					}
#else									
					continue;
#endif
				}
			}
#else
			if (pLinkNext->veh.level > nLimtLevel) {// 낮은 레벨은 제외
				continue;
			}
#endif

			// 트럭 회피 옵션
			if (m_pDataMgr->IsAvoidTruckLink(pRouteInfo->reqInfo.RouteTruckOption.option(), pLinkNext)) {
				continue;
			}

#if defined(USE_P2P_DATA)
			if (pLinkNext->veh.hd_flag != 1) { // HD 링크와 매칭 정보가 없으면 통행 불가
				continue;
			}
#endif
			isPayedLink = pLinkNext->veh.charge;
		} 

		// 무료 적용 옵션일 경우, 출도착이 무료고 유료 링크면 패스
		if ((pRouteInfo->reqInfo.FreeOption == 1) && (isPayedArea == 0) && (isPayedLink == 1)) {
			continue;
			// costAdditional = 1000.; 또는 가중치 증가 
		}

		
		candidateId.parents_id = pLink->link_id.nid;
		candidateId.current_id = pLinkNext->link_id.nid;

//#ifdef USE_LINK_HISTORY_TABLE
//			 다음 방문이 더 싼경우 싼녀석으로 갈아치우자. 2025-03-04
//			static bool useReplace = true;
//			static int typeCompare = 2; // 0:COST, 1:DIST, 2:TIME
//			unordered_map<uint64_t, CandidateLink*>::iterator rpIt = pRouteInfo->mRoutePass.find(candidateId.llid);
//			if (useReplace && (rpIt != pRouteInfo->mRoutePass.end()) && (pLinkNext->veh.link_type == 1 || pLinkNext->veh.link_type == 2)) { // 로터리 등은 제외하고, 일반 도로에서만 비교하자
//				CandidateLink* pOtherInfo = rpIt->second;
//				if (((typeCompare == 0) && (pCurInfo->costTreavel < pOtherInfo->costTreavel)) ||
//					((typeCompare == 1) && (pCurInfo->distTreavel < pOtherInfo->distTreavel)) ||
//					((typeCompare == 2) && (pCurInfo->timeTreavel < pOtherInfo->timeTreavel))) {
//					if ((pLink->link_id.nid == 1514369) && pCurInfo->costTreavel < rpIt->second->costTreavel) {
//					if (/*(pLink->link_id.nid == 1514369) && */pCurInfo->timeTreavel < pPrevInfo->timeTreavel) {
//
//					 재귀 반복되지 않는지 확인 필요
//					 키를 변경하기 위해 기존 값 삭제 후 다시 추가
//					pOtherInfo->isEnable = false;
//					pRouteInfo->mRoutePass.erase(candidateId.llid);
//					pRouteInfo->mRoutePass.emplace(pRouteInfo->cntReplaced++, pOtherInfo);
//				}
//			}
//#endif

		// 첫 방문일 경우에만 추가
		if (pRouteInfo->mRoutePass.find(candidateId.llid) == pRouteInfo->mRoutePass.end()) {
		//if (!IsVisitedLink(pRouteInfo->, candidateId)) { // && !IsAddedLink(pLink->link_id)) {
		//else if (pRouteInfo->mRoutePass.find(candidateId.llid) == pRouteInfo->mRoutePass.end()) {
			if (pNodeNext->coord.x == pLinkNext->getVertexX(0) && pNodeNext->coord.y == pLinkNext->getVertexY(0)) { // 다음 링크의 snode 연결
				if (pLinkNext->link_id.dir == DIR_NAGATIVE) { // 일방(역)에 시작점이 일치하면 불가
					continue;
				}
				dirTarget = DIR_POSITIVE; // 정 - s에서 나가는
			} else { // 다음 링크의 enode 연결
				if (pLinkNext->link_id.dir == DIR_POSITIVE) { // 일방(정)에 종료점이 일치하면 불가
					continue;
				}
				dirTarget = DIR_NAGATIVE; // 역 - e에서 나가는
			}
			angEnd = getAngle(pLinkNext, dirTarget, false);
			angDiff = getEntryAngle(angStart, angEnd);

#if defined(USE_P2P_DATA)
			if (checkAvoidTurnLeft(&pRouteInfo->reqInfo, pCurInfo, pNodeNext, pLinkNext, angDiff) == true) {
				continue;
			}
#endif

			// 고도 차이
			// 현재 숲길 데이터에만 존재
			if (pLinkNext->base.link_type == TYPE_LINK_DATA_TREKKING && pNodeNext->node_id != pNode->node_id) {
				altDiff = pNodeNext->trk.z_value - pNode->trk.z_value;
				if (altDiff != 0) {
					altCost = get_road_slope(static_cast<int32_t>(pLinkNext->length), altDiff);
				}
			}

			// 모든 레벨 확장은 가중치 필요없음
//
//#if 0
//				double distFactor = getDistanceFactor(pRouteInfo->routeLinkInfo.Coord.x, pRouteInfo->routeLinkInfo.Coord.y, vtOrigins[nextIdx].x, vtOrigins[nextIdx].y, pLinkNext, pRouteInfo->routeOption);
//#else
//				double distFactor = getPropagationDistanceFactor(vtOrigins, currIdx, nextIdx, pLinkNext, pRouteInfo->reqInfo.RouteOption);
//#endif
//
//#if defined(USE_TMS_API)
//				if (distFactor < 0) { // rdm에서 일정 거리 이상의 낮은 도로는 무시하자
//					continue;
//				}
//#endif
//
//				// 다음 링크의 끝노드 위치를 기준으로 휴리스틱 계산하는게 맞을 듯
//				int nextLinkIdx = 0;
//				if (dirTarget == 1) { // 정 - s에서 나가는
//					nextLinkIdx = pLinkNext->getVertexCount() - 1; // 멀리있는 노드는 e
//				}
//				heuristicCost = getHueristicCost(pRouteInfo->routeLinkInfo.Coord.x, pRouteInfo->routeLinkInfo.Coord.y, vtOrigins[nextIdx].x, vtOrigins[nextIdx].y, pLinkNext->getVertexX(nextLinkIdx), pLinkNext->getVertexY(nextLinkIdx), pLinkNext->base.link_type, -1, distFactor);

			uint8_t curSpeed = SPEED_NOT_AVALABLE;
			uint8_t curSpeedType = pRouteInfo->reqInfo.RequestTraffic;
#if defined(USE_VEHICLE_DATA) // && defined(USE_TRAFFIC_DATA)
			if (pLinkNext->veh.level <= 6) {
#	if USE_TRAFFIC_LINK_ATTRIBUTE // use link speed
				if (dirTarget == DIR_POSITIVE) {
					curSpeed = pLinkNext->veh_ext.spd_p;
					curSpeedType = pLinkNext->veh_ext.spd_type_p;
				} else { // if (dirTarget == DIR_NAGATIVE) {
					curSpeed = pLinkNext->veh_ext.spd_n;
					curSpeedType = pLinkNext->veh_ext.spd_type_n;
				}
#	else // use traffic map speed
				curSpeed = m_pDataMgr->GetTrafficSpeed(pLinkNext->link_id, dirTarget, pRouteInfo->reqInfo.RequestTime, curSpeedType);
#	endif // #if USE_TRAFFIC_LINK_ATTRIBUTE
			}
#endif
			// 주변 링크를 확인하고, 최대 속도를 주변 링크 속도 이하로 조정
			//if ((curSpeed == SPEED_NOT_AVALABLE) && (pCurInfo->speed != SPEED_NOT_AVALABLE) && (pLinkNext->veh.level >= pLink->veh.level)) {
			//	if (GetLinkSpeed(pLinkNext->link_id, pLinkNext->veh.level, pRouteInfo->reqInfo.RouteOption) > pCurInfo->speed) {
			//		curSpeed = pCurInfo->speed;
			//	}
			//}

			// 트리 등록
			costReal = GetCost(&pRouteInfo->reqInfo, pLinkNext, dirTarget, 0, curSpeed);
			//costTreavel = GetTravelCost(pLinkNext, pLink, curSpeed, costReal, angDiff, pRouteInfo->routeOption, pRouteInfo->avoidOption, pRouteInfo->mobilityOption, isPayedArea);
			costTreavel = GetTravelCost(&pRouteInfo->reqInfo, pLinkNext, pLink, nullptr, curSpeed, costReal, angDiff);
//#if defined(USE_TMS_API)
//				if (distFactor > 0) { // rdm에서 일정 거리 이상의 낮은 도로는 무시하자
//					costTreavel += costReal * distFactor;
//				}
//#endif

			nextCandidate.parents_id = pCurInfo->parentLinkId.nid;
			nextCandidate.current_id = pCurInfo->linkId.nid;

			CandidateLink* pItem = new CandidateLink();
			pItem->candidateId = nextCandidate; // 후보 ID
			pItem->parentLinkId = pCurInfo->linkId;	// 부모 링크 ID
			pItem->linkId = pLinkNext->link_id; // 링크 ID
			pItem->nodeId = pNodeNext->node_id;	// 노드 ID
			pItem->dist = pLinkNext->length;	// 실 거리
			pItem->time = costReal; // 실 시간
			pItem->cost = costTreavel + costAdditional; // 논리적 주행 시간
			pItem->distTreavel = pCurInfo->distTreavel + pItem->dist;	// 누적 거리
			pItem->timeTreavel = pCurInfo->timeTreavel + pItem->time; // 누적 시간
			pItem->costTreavel = pCurInfo->costTreavel + pItem->cost;	// 계산 비용
																		//pItem->costHeuristic = pCurInfo->costTreavel;// +heuristicCost;	// 가중치 계산 비용 // old
			pItem->costHeuristic = pItem->costTreavel + heuristicCost;	// 가중치 계산 비용
			pItem->linkDataType = pCurInfo->linkDataType;
			pItem->depth = pCurInfo->depth + 1;	// 탐색 깊이
			pItem->visited = false; // 방문 여부
			pItem->dir = dirTarget; // 탐색방향
			pItem->angle = angDiff; // 진입각도
#if defined(USE_VEHICLE_DATA)
			pItem->speed = curSpeed;
			pItem->speed_type = curSpeedType;
			pItem->speed_level = pLinkNext->veh.level;
#endif

			pItem->pPrevLink = const_cast<CandidateLink*>(pCurInfo);
			pRouteInfo->mRoutePass.emplace(candidateId.llid, pItem);
			pRouteInfo->pqDijkstra.emplace(pItem);

			cntLinks++;

#if defined(USE_SHOW_ROUTE_SATATUS)
			//LOG_TRACE(LOG_DEBUG, "id:%lld(real cost:%f, heuristic cost:%f, lv:%d) ", newItem.linkId.llid, newItem.costReal, newItem.costHeuristic, newItem.depth);
#endif
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

	vtVtx.clear();

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
#if defined(USE_REAL_ROUTE_TSP)
	else if ((pLink->veh.level <= 4) || ((pLink->veh.level >= 5) && (pLink->veh.lane_cnt >= 2))) {
#else
	else if ((pLink->veh.level <= 4) || ((pLink->veh.level >= 5) && (pLink->veh.lane_cnt > 2))) {
#endif
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


const bool CRoutePlan::SetRouteLinkInfo(IN const SPoint& ptPosition, IN const KeyID keyLink, IN const int32_t linkDataType, IN const int32_t linkDir, IN const bool isStart, IN const bool isDirIgnore, OUT RouteLinkInfo* pRouteLinkInfo)
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
	pLink = m_pDataMgr->GetLinkDataById(keyLink, linkDataType);

	if (!pLink)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't find start/end link, data type:%d, tile:%d, link:%d", linkDataType, keyLink.tile_id, keyLink.nid);
		return false;
	}

	int32_t nLinkMatchVtxIdx = m_pDataMgr->GetLinkVertexDataByPoint(ptPosition.x, ptPosition.y, searchRange[MAX_SEARCH_RANGE - 1], keyLink, ptSplitLink.x, ptSplitLink.y, retDist);
	if (nLinkMatchVtxIdx < 0) {
		LOG_TRACE(LOG_ERROR, "Failed, start/end link vertex not match, x:%d, y:%d, link:%lld", ptSplitLink.x, ptSplitLink.y, keyLink.llid);
		return false;
	}

	// 시작점은 s 노드로의 진행은 역방향, // 종료점은 s 노드에서의 진입은 정방향
	pRouteLinkInfo->LinkDistToS = getLinkSplitDist(pLink, nLinkMatchVtxIdx, ptSplitLink.x, ptSplitLink.y, (isStart ? 2 : 1), (isStart ? 2 : 1), pRouteLinkInfo->LinkVtxToS);
	// 시작점은 e 노드로의 진행은 정방향, // 종료점은 e 노드에서의 진입은 역방향
	pRouteLinkInfo->LinkDistToE = getLinkSplitDist(pLink, nLinkMatchVtxIdx, ptSplitLink.x, ptSplitLink.y, (isStart ? 1 : 2), (isStart ? 2 : 1), pRouteLinkInfo->LinkVtxToE);

	pRouteLinkInfo->KeyType = TYPE_KEY_LINK;
	pRouteLinkInfo->LinkDataType = linkDataType;

	pRouteLinkInfo->LinkId = keyLink;
	pRouteLinkInfo->Coord = ptPosition;
	pRouteLinkInfo->MatchCoord = ptSplitLink;
	pRouteLinkInfo->LinkSplitIdx = nLinkMatchVtxIdx;

	pRouteLinkInfo->LinkSubInfo = pLink->sub_info;
	
	// 차량 네트워크는 방향성 따지자
	if (linkDir != 0) {
		pRouteLinkInfo->LinkDir = linkDir;
	}
	else if (isDirIgnore) {
		pRouteLinkInfo->LinkDir = 0;
	}
#if defined(USE_P2P_DATA)
	else if (pLink->veh.hd_flag == 2) { // 링크가 HD와 부분 매칭이면 역-진입 시, 탐색 실패가 나올 수 있어 양방향 탐색하도록 하자
		pRouteLinkInfo->LinkDir = linkDir;
	}
#endif
	else if (pLink->base.link_type == TYPE_LINK_DATA_VEHICLE) {
		if (pLink->link_id.dir != 0) {
			// 일방이면 좌/우 상관없이 해당 방향으로 설정
			if ((pLink->link_id.dir == 1 && pLink->veh.pass_code == 5) || (pLink->link_id.dir == 2 && pLink->veh.pass_code == 6)) {
				// 정상
			} else {
				// 오류
				LOG_TRACE(LOG_WARNING, "link direction matching failed, link id:%lld, tile:%d, nid:%d, dir:%d, pass:%d", pLink->link_id.llid, pLink->link_id.tile_id, pLink->link_id.nid, pLink->link_id.dir, pLink->veh.pass_code);
			}
			pRouteLinkInfo->LinkDir = pLink->link_id.dir;
		}
		// 우선 도착지에 대해서만,-> 출발지도 적용(2023.8.10)
		// 6레벨 이상, 일반도로, 2차선 이하, 본선비분리 도로는 양쪽에 매칭 가능하도록 하자
		else if (//!isStart && 
			(pLink->veh.level >= 6) && (pLink->veh.lane_cnt <= 2) && (pLink->veh.road_type >= 6) && (pLink->veh.link_type == 1)) {
			pRouteLinkInfo->LinkDir = 0;
		} else {
			pRouteLinkInfo->LinkDir = getRouteDirFromPosition(pLink, nLinkMatchVtxIdx, ptPosition.x, ptPosition.y);

			stNodeInfo* pNode = nullptr;

			// 출발지의 방향성으로 설정시, 진출이 불가한 (이전 링크에서 회전 불가 등) 경우 확인
			// 목적지의 방향성으로 설정시, 진입이 불가한 (이전 링크에서 회전 불가 등) 경우 확인
			if ((pRouteLinkInfo->LinkDir == 1)) {
				pNode = m_pDataMgr->GetVNodeDataById(isStart ? pLink->enode_id : pLink->snode_id);
			}
			else if ((pRouteLinkInfo->LinkDir == 2)) {
				pNode = m_pDataMgr->GetVNodeDataById(isStart ? pLink->snode_id : pLink->enode_id);
			}

			if (pNode && (pNode->base.point_type == TYPE_NODE_EDGE)) {
				// 구획 변경점은 방향성 유지
			}
#if defined(USE_SAMSUNG_HEAVY)
			else if (pNode && (pNode->base.point_type == TYPE_NODE_END)) {
				// 삼성중공업의 경우, 단점은 유턴가능 지점으로 사용.
			}
#endif
			// 우선 도착지에 대해서만,
			// 단점 노드 에서의 진입인 경우 확인 -> 오류 해소
			else if (!isStart &&
#if defined(USE_P2P_DATA) // p2p는 가능하면 실패하지 않는 경로 생성하자
			isDirIgnore &&
#endif
				(((pRouteLinkInfo->LinkDir == 1) && (m_pDataMgr->GetVNodeDataById(pLink->snode_id)->base.connnode_count <= 1)) || // 목적지가 정방향인데, S노드가 단점이면
				((pRouteLinkInfo->LinkDir == 2) && (m_pDataMgr->GetVNodeDataById(pLink->enode_id)->base.connnode_count <= 1)))) // 목적지가 역방향인데, E노드가 단점이면
			{
				// 방향성 바꾸기
				pRouteLinkInfo->LinkDir = (pRouteLinkInfo->LinkDir == 1) ? 2 : 1;
			}
			else
#if !defined(USE_P2P_DATA) // p2p는 가능하면 실패하지 않는 경로 생성하자
			if (isDirIgnore)
#endif
			{
				int32_t passCode = 0;

				if (pNode && (pNode->base.connnode_count >= 1)) {
					for (int ii = 0; ii < pNode->base.connnode_count; ii++) {
						if ((isStart && (passCode = getNextPassCode(pLink->link_id, pNode->connnodes[ii], pNode, 0, m_pDataMgr)) != PASS_CODE_DISABLE) ||
							(!isStart && (passCode = getPrevPassCode(pLink->link_id, pNode->connnodes[ii], pNode, 0, m_pDataMgr)) != PASS_CODE_DISABLE)) {
							break;
						}
					} // for

					if (passCode == PASS_CODE_DISABLE) {
						// 방향성 바꾸기
						int oldDir = pRouteLinkInfo->LinkDir;
						pRouteLinkInfo->LinkDir = (pRouteLinkInfo->LinkDir == 1) ? 2 : 1;

						LOG_TRACE(LOG_WARNING, "Current link dir not connected, change link dir, isStart:%d, tile:%d, id:%d, dir:%d->%d", isStart, pRouteLinkInfo->LinkId.tile_id, pRouteLinkInfo->LinkId.nid, oldDir, pRouteLinkInfo->LinkDir);
					}
				}
			}
		}
	} else { // 방향성 기본			
		pRouteLinkInfo->LinkDir = 0;
	}

	return true;
}


const bool CRoutePlan::GetRouteTravelSpeed(IN OUT RouteResultInfo* pResult)
{
	bool ret = false;

#if defined(USE_VEHICLE_DATA)
	if ((pResult != nullptr) && (!pResult->LinkInfo.empty())) {
		double curSpeed = SPEED_NOT_AVALABLE;
		uint8_t curSpeedType = pResult->reqInfo.RequestTraffic;

		for (auto& link : pResult->LinkInfo) {
			curSpeed = m_pDataMgr->GetTrafficSpeed(link.link_id, link.dir + 1, pResult->reqInfo.RequestTime, curSpeedType);

			if (curSpeed == SPEED_NOT_AVALABLE) {
				curSpeed = GetLinkSpeed(link.link_id, link.speed_level, pResult->reqInfo.RouteOption);
			}

			link.speed = static_cast<uint8_t>(curSpeed);
			link.speed_type = curSpeedType;
			link.time = link.length / (curSpeed / 3.6f);
		} // for
	}
#endif

	return ret;
}


const int CRoutePlan::DoRoute(IN const string& reqId, IN const SPoint ptStart, IN const SPoint ptEnd, IN const KeyID sLink, IN const KeyID eLink, IN const uint32_t routeOpt, IN const uint32_t avoidOpt, IN const uint32_t mobilityOpt, IN const stRouteSubOption subOpt, IN const bool ignoreStartDir, IN const bool ignoreEndDir, OUT RouteResultInfo* pRouteResult)
{
	RouteInfo routeInfo;

#if defined(USE_VEHICLE_DATA)
	double maxDist = MAX_SEARCH_DIST_FOR_VEHICLE;
#else
	double maxDist = MAX_SEARCH_DIST_FOR_PEDESTRIAN;
	if (mobilityOpt == TYPE_MOBILITY_BICYCLE) {
		maxDist = MAX_SEARCH_DIST_FOR_BICYCLE; // 자전거 경탐은 자전거 최대 거리로 확장
	}
#endif

	Initialize();

	routeInfo.reqInfo.RequestId = reqId;

	if (pRouteResult == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, request info param null, pRouteResults:%p", pRouteResult);

		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_WRONG_PARAM;
		return pRouteResult->ResultCode; 
	}

	double retDist = 0.f;
	if ((retDist = getRealWorldDistance(ptStart.x, ptStart.y, ptEnd.x, ptEnd.y)) > maxDist) {
		LOG_TRACE(LOG_ERROR, "Failed, max request route distance(%d) over than %d", retDist, maxDist);

		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_DIST_OVER;
		return pRouteResult->ResultCode;
	}


	// 시작점 정보
	if (!SetRouteLinkInfo(ptStart, sLink, TYPE_LINK_DATA_PEDESTRIAN, BIDIRECTION, true, ignoreStartDir, &routeInfo.StartLinkInfo)) {
		LOG_TRACE(LOG_ERROR, "Failed, set start link info");

		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_SET_START;
		return pRouteResult->ResultCode;;
	}
	routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DEPARTURE; // 출발지

	// 종료점 정보
	if (!SetRouteLinkInfo(ptEnd, eLink, TYPE_LINK_DATA_PEDESTRIAN, BIDIRECTION, false, ignoreEndDir, &routeInfo.EndLinkInfo)) {
		LOG_TRACE(LOG_ERROR, "Failed, set end link info");

		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_SET_END;
		return pRouteResult->ResultCode;;
	}
	routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DESTINATION; // 도착지


	// 경탐 옵션 
	routeInfo.reqInfo.RouteOption = routeOpt;
	// 회피 옵션
	routeInfo.reqInfo.AvoidOption = avoidOpt;
	// 이동체
	routeInfo.reqInfo.MobilityOption = mobilityOpt;
	// 추가 옵션
	routeInfo.reqInfo.RouteSubOption = subOpt;

	// 방향성 무시
	routeInfo.reqInfo.StartDirIgnore = ignoreStartDir;
	routeInfo.reqInfo.EndDirIgnore = ignoreEndDir;


	// 함수로 대체
	int ret = MakeRoute(0, &routeInfo, pRouteResult);
	if (ret == ROUTE_RESULT_FAILED_EXPEND) {
		// 시작점의 방향성을 바꿔서 한번 더 재탐색 시도
		if (routeInfo.StartLinkInfo.LinkDir != 0) {
			int oldDir = routeInfo.StartLinkInfo.LinkDir;
			routeInfo.StartLinkInfo.LinkDir = (routeInfo.StartLinkInfo.LinkDir == 1) ? 2 : 1;

			LOG_TRACE(LOG_DEBUG, "Retry, make single route, change start link dir, tile:%d, id:%d, dir:%d->%d, tick:%lld", routeInfo.StartLinkInfo.LinkId.tile_id, routeInfo.StartLinkInfo.LinkId.nid, oldDir, routeInfo.StartLinkInfo.LinkId.dir);
			ret = MakeRoute(0, &routeInfo, pRouteResult);
		}
	}

	if (ret == ROUTE_RESULT_SUCCESS) {
		ret = MakeRouteResult(&routeInfo, pRouteResult);

		if (ret != ROUTE_RESULT_SUCCESS) {
			LOG_TRACE(LOG_INFO, "Failed, req_idx:%d, can't find route, couldn't leached end point(the point will isolated)", 0);
			LOG_TRACE(LOG_INFO, "Failed, req_idx:%d, start(link:%d, coord:%.6f,%.6f), end(link:%d, coord:%.6f,%.6f)", 0,
				routeInfo.StartLinkInfo.LinkId.nid, routeInfo.StartLinkInfo.Coord.x, routeInfo.StartLinkInfo.Coord.y,
				routeInfo.EndLinkInfo.LinkId.nid, routeInfo.EndLinkInfo.Coord.x, routeInfo.EndLinkInfo.Coord.y);
			/* display
			WCHAR buff[128];
			while (!pqScore.empty())
			{
			wsprintf(buff, L"%d, %d \n", pqScore.top().id, pqScore.top().length);
			OutputDebugString(buff);
			pqScore.pop();
			}
			*/
			ret = pRouteResult->ResultCode = ROUTE_RESULT_FAILED_EXPEND;
		}
	}

	return ret;
}


const int CRoutePlan::Planning(IN const RequestRouteInfo* pReqInfo, OUT RouteResultInfo* pRouteResult)
{
	int ret = -1;

	//RouteInfo routeInfo;

	//double retDist = 0.f;

	//// 경탐 옵션 
	//routeInfo.RouteOption = pReqInfo->RouteOption;
	//// 회피 옵션
	//routeInfo.AvoidOption = pReqInfo->AvoidOption;
	//// 이동체
	//routeInfo.MobilityOption = pReqInfo->MobilityOption;
	//// 추가 옵션
	//routeInfo.RouteSubOpt = pReqInfo->RouteSubOption;

	//// 방향성 무시
	//bool isDirIgnore = false;

	//KeyID sLink = pReqInfo->vtKeyId[ii];
	//KeyID eLink = pReqInfo->vtKeyId[ii + 1];

	//SPoint ptStart = { pReqInfo->vtPoints[ii].x, pReqInfo->vtPoints[ii].y };
	//SPoint ptEnd = { pReqInfo->vtPoints[ii + 1].x, pReqInfo->vtPoints[ii + 1].y };

	//if ((retDist = getRealWorldDistance(ptStart.x, ptStart.y, ptEnd.x, ptEnd.y)) > maxDist) {
	//	LOG_TRACE(LOG_ERROR, "Failed, max request route distance(%d) over than %d", retDist, maxDist);

	//	pRouteResult->ResultCode = ROUTE_RESULT_FAILED_DIST_OVER;
	//	//return routeResult.ResultCode;
	//}

	//// 시작점 정보
	//routeInfo.StartLinkInfo = pReqInfo->vtPointsInfo[ii];
	//routeInfo.StartLinkInfo.KeyType = pReqInfo->vtKeyType[ii];

	//if (ii == 0) {
	//	routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DEPARTURE; // 출발지
	//	if (pReqInfo->StartDirIgnore) { // 출발지 무시
	//		isDirIgnore = true;
	//	}
	//} else {
	//	// 출발지 이외의 지점은 종료 링크에 속성정보가 들어가므로 링크 시작의 경유지 정보는 의미가 없음
	//	routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
	//	if (pReqInfo->WayDirIgnore) { // 경유지 무시
	//		isDirIgnore = true;
	//	}
	//}

	//// 방향성 무시
	//routeInfo.StartDirIgnore = isDirIgnore;

	//if (!SetRouteLinkInfo(ptStart, sLink, pReqInfo->vtLinkDataType[ii], true, isDirIgnore, &routeInfo.StartLinkInfo)) {
	//	LOG_TRACE(LOG_ERROR, "Failed, set start link info");

	//	if (ii == 0) {
	//		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_SET_START;
	//	} else {
	//		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_SET_VIA;
	//	}
	//}

	//// 종료점 정보
	//routeInfo.EndLinkInfo = pReqInfo->vtPointsInfo[ii];

	//routeInfo.EndLinkInfo.KeyType = pReqInfo->vtKeyType[ii];

	//if (ii + 1 == cntPoints - 1) {
	//	routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DESTINATION; // 도착지
	//	if (pReqInfo->EndDirIgnore) { // 경유지 무시
	//		isDirIgnore = true;
	//	}
	//} else {
	//	// 경유지
	//	routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
	//																	//LINK_GUIDE_TYPE_DEPARTURE_WAYPOINT, // 출발지-경유지
	//																	//LINK_GUIDE_TYPE_WAYPOINT_WAYPOINT, // 경유지-경유지
	//																	//LINK_GUIDE_TYPE_WAYPOINT_DESTINATION, // 경유지-도착지
	//	if (pReqInfo->WayDirIgnore) { // 경유지 무시
	//		isDirIgnore = true;
	//	}
	//}

	//// 방향성 무시
	//routeInfo.EndDirIgnore = isDirIgnore;

	//if (!SetRouteLinkInfo(ptEnd, eLink, pReqInfo->vtLinkDataType[ii], false, isDirIgnore, &routeInfo.EndLinkInfo)) {
	//	LOG_TRACE(LOG_ERROR, "Failed, set end link info");

	//	if (ii < cntPoints - 1) {
	//		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_SET_END;
	//	} else {
	//		pRouteResult->ResultCode = ROUTE_RESULT_FAILED_SET_VIA;
	//	}
	//	//return routeResult.ResultCode;
	//}

	//int ret = MakeRoute(ii, &routeInfo, pRouteResult);
	//if (ret == ROUTE_RESULT_FAILED_EXPEND) {
	//	// 시작점의 방향성을 바꿔서 한번 더 재탐색 시도
	//	if (routeInfo.StartLinkInfo.LinkDir != 0) {
	//		int oldDir = routeInfo.StartLinkInfo.LinkDir;
	//		routeInfo.StartLinkInfo.LinkDir = (routeInfo.StartLinkInfo.LinkDir == 1) ? 2 : 1;

	//		ret = MakeRoute(ii, &routeInfo, pRouteResult);
	//		LOG_TRACE(LOG_DEBUG, "Retry, make routes, result:%d, idx:%d, change start link dir, llid:%lld, tile:%d, id:%d, dir:%d->%d", ret, ii, routeInfo.StartLinkInfo.LinkId.llid, routeInfo.StartLinkInfo.LinkId.tile_id, routeInfo.StartLinkInfo.LinkId.nid, oldDir, routeInfo.StartLinkInfo.LinkId.dir);
	//	}
	//}

	//if (ret == ROUTE_RESULT_SUCCESS) {
	//	ret = MakeRouteResult(&routeInfo, pRouteResult);

	//	if (ret != ROUTE_RESULT_SUCCESS) {
	//		LOG_TRACE(LOG_INFO, "Failed, req_idx:%d, can't find route, couldn't leached end point(the point will isolated)", ii);
	//		LOG_TRACE(LOG_INFO, "Failed, req_idx:%d, start(link:%d, coord:%.6f,%.6f), end(link:%d, coord:%.6f,%.6f)", ii,
	//			routeInfo.StartLinkInfo.LinkId.nid, routeInfo.StartLinkInfo.Coord.x, routeInfo.StartLinkInfo.Coord.y,
	//			routeInfo.EndLinkInfo.LinkId.nid, routeInfo.EndLinkInfo.Coord.x, routeInfo.EndLinkInfo.Coord.y);
	//		/* display
	//		WCHAR buff[128];
	//		while (!pqScore.empty())
	//		{
	//		wsprintf(buff, L"%d, %d \n", pqScore.top().id, pqScore.top().length);
	//		OutputDebugString(buff);
	//		pqScore.pop();
	//		}
	//		*/
	//		ret = ROUTE_RESULT_FAILED_EXPEND;
	//	}
	//} else { // if (ret != ROUTE_RESULT_SUCCESS) {
	//	LOG_TRACE(LOG_DEBUG, "--->>>Failed, make routes, result:%d, idx:%d, change start link dir, llid:%lld, tile:%d, id:%d, ", ret, ii, routeInfo.StartLinkInfo.LinkId.llid, routeInfo.StartLinkInfo.LinkId.tile_id, routeInfo.StartLinkInfo.LinkId.nid);
	//	RouteResultLinkEx failedInfo = { 0, };
	//	// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
	//	failedInfo.guide_type = routeInfo.EndLinkInfo.LinkGuideType;
	//	pRouteResult->LinkInfo.emplace_back(failedInfo);
	//}

	//pRouteResult->ResultCode = ret;

	return ret;
}


const int CRoutePlan::DoRoutes(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults)
{
	if (pReqInfo == nullptr || pRouteInfos == nullptr || pRouteResults == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, request info param null, pReqInfo:%p, pRouteInfos:%p, pRouteResults:%p", pReqInfo, pRouteInfos, pRouteResults);

		return ROUTE_RESULT_FAILED;
	}

#if defined(USE_VEHICLE_DATA)
	double maxDist = MAX_SEARCH_DIST_FOR_VEHICLE;
#else
	double maxDist = MAX_SEARCH_DIST_FOR_PEDESTRIAN;
	if (pReqInfo->MobilityOption == TYPE_MOBILITY_BICYCLE) {
		maxDist = MAX_SEARCH_DIST_FOR_BICYCLE; // 자전거 경탐은 자전거 최대 거리로 확장
}
#endif

	int32_t cntPoints = pReqInfo->vtPoints.size();

	if (cntPoints < 2 || (cntPoints != pReqInfo->vtKeyId.size())) {
		LOG_TRACE(LOG_WARNING, "Failed, route request point too short, points:%d, links:%d", cntPoints, pReqInfo->vtKeyId.size());

		return ROUTE_RESULT_FAILED_WRONG_PARAM;
	}

	LOG_TRACE(LOG_DEBUG, "request, multi routing, points:%d", cntPoints);

	Initialize();

	pRouteInfos->resize(cntPoints - 1);
	pRouteResults->resize(cntPoints - 1);

#if defined(USE_MULTIPROCESS)
#pragma omp parallel for
#endif
	for (int ii = 0; ii < cntPoints - 1; ii++)
	{
		RouteInfo& routeInfo = pRouteInfos->at(ii);
		RouteResultInfo& routeResult = pRouteResults->at(ii);

		double retDist = 0.f;

		routeInfo.reqInfo.SetOption(pReqInfo);
		//// 경탐 옵션 
		//routeInfo.RouteOption = pReqInfo->RouteOption;
		//// 회피 옵션
		//routeInfo.AvoidOption = pReqInfo->AvoidOption;
		//// 이동체
		//routeInfo.MobilityOption = pReqInfo->MobilityOption;
		//// 추가 옵션
		//routeInfo.RouteSubOpt = pReqInfo->RouteSubOption;

		// 방향성 무시
		bool isDirIgnore = false;

		KeyID sLink = pReqInfo->vtKeyId[ii];
		KeyID eLink = pReqInfo->vtKeyId[ii + 1];

		SPoint ptStart = { pReqInfo->vtPoints[ii].x, pReqInfo->vtPoints[ii].y };
		SPoint ptEnd = { pReqInfo->vtPoints[ii + 1].x, pReqInfo->vtPoints[ii + 1].y };

		if ((retDist = getRealWorldDistance(ptStart.x, ptStart.y, ptEnd.x, ptEnd.y)) > maxDist) {
			LOG_TRACE(LOG_ERROR, "Failed, max request route distance(%d) over than %d", retDist, maxDist);

			routeResult.ResultCode = ROUTE_RESULT_FAILED_DIST_OVER;
			//return routeResult.ResultCode;
		}

		// 시작점 정보
		routeInfo.StartLinkInfo = pReqInfo->vtPointsInfo[ii];


		routeInfo.StartLinkInfo.KeyType = pReqInfo->vtKeyType[ii];

		if (ii == 0) {
			routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DEPARTURE; // 출발지
			if (pReqInfo->StartDirIgnore) { // 출발지 무시
				isDirIgnore = true;
			}
		}
		else {
			// 출발지 이외의 지점은 종료 링크에 속성정보가 들어가므로 링크 시작의 경유지 정보는 의미가 없음
			routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
			if (pReqInfo->WayDirIgnore) { // 경유지 무시
				isDirIgnore = true;
			}
		}

		// 방향성 무시
		routeInfo.reqInfo.StartDirIgnore = isDirIgnore;

		if (!SetRouteLinkInfo(ptStart, sLink, pReqInfo->vtLinkDataType[ii], pReqInfo->vtPointsInfo[ii].LinkDir, true, isDirIgnore, &routeInfo.StartLinkInfo)) {
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
		routeInfo.EndLinkInfo = pReqInfo->vtPointsInfo[ii + 1];

		routeInfo.EndLinkInfo.KeyType = pReqInfo->vtKeyType[ii + 1];

		if (ii + 1 == cntPoints - 1) {
			routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DESTINATION; // 도착지
			if (pReqInfo->EndDirIgnore) { // 경유지 무시
				isDirIgnore = true;
			}
		}
		else {
			// 경유지
			routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
			//LINK_GUIDE_TYPE_DEPARTURE_WAYPOINT, // 출발지-경유지
			//LINK_GUIDE_TYPE_WAYPOINT_WAYPOINT, // 경유지-경유지
			//LINK_GUIDE_TYPE_WAYPOINT_DESTINATION, // 경유지-도착지
			if (pReqInfo->WayDirIgnore) { // 경유지 무시
				isDirIgnore = true;
			}
		}

		// 방향성 무시
		routeInfo.reqInfo.EndDirIgnore = isDirIgnore;

		if (!SetRouteLinkInfo(ptEnd, eLink, pReqInfo->vtLinkDataType[ii], BIDIRECTION, false, isDirIgnore, &routeInfo.EndLinkInfo)) {
			LOG_TRACE(LOG_ERROR, "Failed, set end link info");

			if (ii < cntPoints - 1) {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_END;
			}
			else {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_VIA;
			}
			//return routeResult.ResultCode;
		}

		int ret = MakeRoute(ii, &routeInfo, &routeResult);
#if defined(USE_P2P_DATA)
		if ((ret == ROUTE_RESULT_FAILED_EXPEND) && (pReqInfo->RequestMode != 100)) {
#else
		if (ret == ROUTE_RESULT_FAILED_EXPEND) {
#endif
			// 시작점의 방향성을 바꿔서 한번 더 재탐색 시도
			if (routeInfo.StartLinkInfo.LinkDir != 0) {
				int oldDir = routeInfo.StartLinkInfo.LinkDir;
				routeInfo.StartLinkInfo.LinkDir = (routeInfo.StartLinkInfo.LinkDir == 1) ? 2 : 1;

				ret = MakeRoute(ii, &routeInfo, &routeResult);
				LOG_TRACE(LOG_DEBUG, "Retry, make routes, result:%d, idx:%d, change start link dir, llid:%lld, tile:%d, id:%d, dir:%d->%d", ret, ii, routeInfo.StartLinkInfo.LinkId.llid, routeInfo.StartLinkInfo.LinkId.tile_id, routeInfo.StartLinkInfo.LinkId.nid, oldDir, routeInfo.StartLinkInfo.LinkId.dir);
			}
		}

		if (ret == ROUTE_RESULT_SUCCESS) {
			ret = MakeRouteResult(&routeInfo, &routeResult);

			if (ret != ROUTE_RESULT_SUCCESS) {
				LOG_TRACE(LOG_INFO, "Failed, req_idx:%d, can't find route, couldn't leached end point(the point will isolated)", ii);
				LOG_TRACE(LOG_INFO, "Failed, req_idx:%d, start(link:%d, coord:%.6f,%.6f), end(link:%d, coord:%.6f,%.6f)", ii,
					routeInfo.StartLinkInfo.LinkId.nid, routeInfo.StartLinkInfo.Coord.x, routeInfo.StartLinkInfo.Coord.y,
					routeInfo.EndLinkInfo.LinkId.nid, routeInfo.EndLinkInfo.Coord.x, routeInfo.EndLinkInfo.Coord.y);
				/* display
				WCHAR buff[128];
				while (!pqScore.empty())
				{
				wsprintf(buff, L"%d, %d \n", pqScore.top().id, pqScore.top().length);
				OutputDebugString(buff);
				pqScore.pop();
				}
				*/
				ret = ROUTE_RESULT_FAILED_EXPEND;
			}
		}
		else { // if (ret != ROUTE_RESULT_SUCCESS) {
			LOG_TRACE(LOG_DEBUG, "--->>>Failed, make routes, result:%d, idx:%d, change start link dir, llid:%lld, tile:%d, id:%d, ", ret, ii, routeInfo.StartLinkInfo.LinkId.llid, routeInfo.StartLinkInfo.LinkId.tile_id, routeInfo.StartLinkInfo.LinkId.nid);
			RouteResultLinkEx failedInfo = { 0, };
			// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
			failedInfo.guide_type = routeInfo.EndLinkInfo.LinkGuideType;
			routeResult.LinkInfo.emplace_back(failedInfo);
		}

		routeResult.ResultCode = ret;
	} // for
	
	// 1개라도 성공이면 성공으로 리턴하자
	int rets = ROUTE_RESULT_FAILED;
	for (const auto& result : *pRouteResults) {
		if (result.ResultCode == ROUTE_RESULT_SUCCESS) {
			rets = ROUTE_RESULT_SUCCESS;
			LOG_TRACE(LOG_DEBUG, "success, multi routing, routes:%d", cntPoints - 1);
			break;
		} else {
			rets = result.ResultCode;
		}
	}
	return rets;
}


const int CRoutePlan::DoComplexRoutes(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults)
{
	if (pReqInfo == nullptr || pRouteInfos == nullptr || pRouteResults == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, request info param null, pReqInfo:%p, pRouteInfos:%p, pRouteResults:%p", pReqInfo, pRouteInfos, pRouteResults);

		return ROUTE_RESULT_FAILED;
	}

	int32_t cntPoints = pReqInfo->vtPoints.size();

	if (cntPoints < 2 || (cntPoints != pReqInfo->vtKeyId.size())) {
		LOG_TRACE(LOG_WARNING, "Failed, route request point too short, points:%d, links:%d", cntPoints, pReqInfo->vtKeyId.size());

		return ROUTE_RESULT_FAILED_WRONG_PARAM;
	}

	LOG_TRACE(LOG_DEBUG, "request, multi routing, points:%d", cntPoints);

	Initialize();

	pRouteInfos->resize(cntPoints - 1);
	pRouteResults->resize(cntPoints - 1);

#if defined(USE_MULTIPROCESS)
#pragma omp parallel for
#endif
	for (int ii = 0; ii < cntPoints - 1; ii++)
	{
		RouteInfo& routeInfo = pRouteInfos->at(ii);
		RouteResultInfo& routeResult = pRouteResults->at(ii);

		double retDist = 0.f;

		routeInfo.reqInfo.SetOption(pReqInfo);
		//// 경탐 옵션 
		//routeInfo.RouteOption = pReqInfo->RouteOption;
		//// 회피 옵션
		//routeInfo.AvoidOption = pReqInfo->AvoidOption;
		//// 이동체
		//routeInfo.MobilityOption = pReqInfo->MobilityOption;
		//// 추가 옵션
		//routeInfo.RouteSubOpt = pReqInfo->RouteSubOption;

		// 방향성 무시
		bool isDirIgnore = false;

		KeyID sLink = pReqInfo->vtKeyId[ii];
		KeyID eLink = pReqInfo->vtKeyId[ii + 1];

		SPoint ptStart = { pReqInfo->vtPoints[ii].x, pReqInfo->vtPoints[ii].y };
		SPoint ptEnd = { pReqInfo->vtPoints[ii + 1].x, pReqInfo->vtPoints[ii + 1].y };

		if (pReqInfo->vtLinkDataType[ii] != pReqInfo->vtLinkDataType[ii + 1]) {
			LOG_TRACE(LOG_DEBUG, "data type not match so, pass to this pair, idx prev:%d-next:%d, type prev:%d-next:%d", ii, ii+1, pReqInfo->vtLinkDataType[ii], pReqInfo->vtLinkDataType[ii+1]);
			continue;
		}

		double maxDist = MAX_SEARCH_DIST_FOR_FOREST;
		if (pReqInfo->MobilityOption == TYPE_MOBILITY_BICYCLE) {
			maxDist = MAX_SEARCH_DIST_FOR_BICYCLE; // 자전거 경탐은 자전거 최대 거리로 확장
		}
#if defined(USE_FOREST_DATA)
		else if (!pReqInfo->vtCourse[ii].empty()) {
			maxDist = MAX_SEARCH_DIST_FOR_COURSE; // 코스ID를 보유한 상태라면 코스 거리로 확장
		}
#endif

		if ((retDist = getRealWorldDistance(ptStart.x, ptStart.y, ptEnd.x, ptEnd.y)) > maxDist) {
			LOG_TRACE(LOG_ERROR, "Failed, max request route distance(%d) over than %d", retDist, maxDist);

			routeResult.ResultCode = ROUTE_RESULT_FAILED_DIST_OVER;
			//return routeResult.ResultCode;
		}

		//////////////////////
		// 시작점 정보
		if (ii == 0) {
			routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DEPARTURE; // 출발지
			if (pReqInfo->StartDirIgnore) { // 출발지 무시
				isDirIgnore = true;
			}
		} else {
			// 출발지 이외의 지점은 종료 링크에 속성정보가 들어가므로 링크 시작의 경유지 정보는 의미가 없음
			routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
			if (pReqInfo->WayDirIgnore) { // 경유지 무시
				isDirIgnore = true;
			}
		}

		// 방향성 무시
		routeInfo.reqInfo.StartDirIgnore = isDirIgnore;

		routeInfo.StartLinkInfo.KeyType = pReqInfo->vtKeyType[ii];
		routeInfo.StartLinkInfo.LinkDataType = pReqInfo->vtLinkDataType[ii];
		if (routeInfo.StartLinkInfo.KeyType == TYPE_KEY_NODE) { // node type
			routeInfo.StartLinkInfo.LinkId = pReqInfo->vtKeyId[ii];
			routeInfo.StartLinkInfo.Coord = pReqInfo->vtPoints[ii];
		}
		else if (!SetRouteLinkInfo(ptStart, sLink, pReqInfo->vtLinkDataType[ii], BIDIRECTION, true, isDirIgnore, &routeInfo.StartLinkInfo)) {
			LOG_TRACE(LOG_ERROR, "Failed, set start link info");

			if (ii == 0) {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_START;
			}
			else {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_VIA;
			}
			//return routeResult.ResultCode;
		}

		//////////////////////
		// 종료점 정보
		if (ii + 1 == cntPoints - 1) {
			routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DESTINATION; // 도착지
			if (pReqInfo->EndDirIgnore) { // 경유지 무시
				isDirIgnore = true;
			}
		}
		else {
			// 경유지
			routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
			//LINK_GUIDE_TYPE_DEPARTURE_WAYPOINT, // 출발지-경유지
			//LINK_GUIDE_TYPE_WAYPOINT_WAYPOINT, // 경유지-경유지
			//LINK_GUIDE_TYPE_WAYPOINT_DESTINATION, // 경유지-도착지
			if (pReqInfo->WayDirIgnore) { // 경유지 무시
				isDirIgnore = true;
			}
		}

		// 방향성 무시
		routeInfo.reqInfo.EndDirIgnore = isDirIgnore;

		routeInfo.EndLinkInfo.KeyType = pReqInfo->vtKeyType[ii + 1];
		routeInfo.EndLinkInfo.LinkDataType = pReqInfo->vtLinkDataType[ii + 1];
		if (routeInfo.EndLinkInfo.KeyType == TYPE_KEY_NODE) { // node type
			routeInfo.EndLinkInfo.LinkId = pReqInfo->vtKeyId[ii + 1];
			routeInfo.EndLinkInfo.Coord = pReqInfo->vtPoints[ii + 1];
		}
		else if (!SetRouteLinkInfo(ptEnd, eLink, pReqInfo->vtLinkDataType[ii], BIDIRECTION, false, isDirIgnore, &routeInfo.EndLinkInfo)) {
			LOG_TRACE(LOG_ERROR, "Failed, set end link info");

			if (ii < cntPoints - 1) {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_END;
			}
			else {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_VIA;
			}
			//return routeResult.ResultCode;
		}

		//////////////////////
		// make route
		int ret = MakeRoute(ii, &routeInfo, &routeResult);
		if (ret == ROUTE_RESULT_FAILED_EXPEND) {
			// 시작점의 방향성을 바꿔서 한번 더 재탐색 시도
			if (routeInfo.StartLinkInfo.LinkDir != 0) {
				int oldDir = routeInfo.StartLinkInfo.LinkDir;
				routeInfo.StartLinkInfo.LinkDir = (routeInfo.StartLinkInfo.LinkDir == 1) ? 2 : 1;

				ret = MakeRoute(ii, &routeInfo, &routeResult);
				LOG_TRACE(LOG_DEBUG, "Retry, make routes, result:%d, idx:%d, change start link dir, llid:%lld, tile:%d, id:%d, dir:%d->%d", ret, ii, routeInfo.StartLinkInfo.LinkId.llid, routeInfo.StartLinkInfo.LinkId.tile_id, routeInfo.StartLinkInfo.LinkId.nid, oldDir, routeInfo.StartLinkInfo.LinkId.dir);
			}
		}

		if (ret != ROUTE_RESULT_SUCCESS) {
			LOG_TRACE(LOG_DEBUG, "--->>>Failed, make routes, result:%d, idx:%d, change start link dir, llid:%lld, tile:%d, id:%d, ", ret, ii, routeInfo.StartLinkInfo.LinkId.llid, routeInfo.StartLinkInfo.LinkId.tile_id, routeInfo.StartLinkInfo.LinkId.nid);
			RouteResultLinkEx failedInfo = { 0, };
			// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
			failedInfo.guide_type = routeInfo.EndLinkInfo.LinkGuideType;
			routeResult.LinkInfo.emplace_back(failedInfo);
		}

		routeResult.ResultCode = ret;
	} // for

	// 1개라도 성공이면 성공으로 리턴하자
	int rets = ROUTE_RESULT_FAILED;
	for (const auto& result : *pRouteResults) {
		if (result.ResultCode == ROUTE_RESULT_SUCCESS) {
			rets = ROUTE_RESULT_SUCCESS;
			LOG_TRACE(LOG_DEBUG, "success, complex routing, routes:%d", cntPoints - 1);
			break;
		} else {
			rets = result.ResultCode;
		}
	}
	return rets;
}


const int CRoutePlan::DoComplexRoutesEx(IN const RequestRouteInfo* pReqInfo/*, IN OUT vector<ComplexPointInfo>& vtCpxRouteInfo*/, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults)
{
	if (pReqInfo == nullptr || pRouteInfos == nullptr || pRouteResults == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, request info param null, pReqInfo:%p, pRouteInfos:%p, pRouteResults:%p", pReqInfo, pRouteInfos, pRouteResults);

		return ROUTE_RESULT_FAILED;
	}

	int32_t cntPoints = pReqInfo->vtPoints.size();

	if (cntPoints < 2 || (cntPoints != pReqInfo->vtKeyId.size())) {
		LOG_TRACE(LOG_WARNING, "Failed, route request point too short, points:%d, links:%d", cntPoints, pReqInfo->vtKeyId.size());

		return ROUTE_RESULT_FAILED_WRONG_PARAM;
	}

	LOG_TRACE(LOG_DEBUG, "request, complex routing, points:%d", cntPoints);

	Initialize();

#if defined(USE_MULTIPROCESS)
#pragma omp parallel for
#endif
	for (int ii = 0; ii < cntPoints - 1; ii++)
	{
		int32_t ret = ROUTE_RESULT_FAILED;

		RouteInfo& routeInfo = pRouteInfos->at(ii);
		RouteInfo& routeInfoNext = pRouteInfos->at(ii + 1);

		RouteResultInfo& routeResult = pRouteResults->at(ii);

		double retDist = 0.f;

		routeInfo.reqInfo.SetOption(pReqInfo);
		//// 경탐 옵션 
		//routeInfo.RouteOption = pReqInfo->RouteOption;
		//// 회피 옵션
		//routeInfo.AvoidOption = pReqInfo->AvoidOption;
		//// 이동체
		//routeInfo.MobilityOption = pReqInfo->MobilityOption;
		//// 추가 옵션
		//routeInfo.RouteSubOpt = pReqInfo->RouteSubOption;

		// 방향성 무시
		bool isDirIgnore = false;

		KeyID sLink = pReqInfo->vtKeyId[ii];
		KeyID eLink = pReqInfo->vtKeyId[ii + 1];

		SPoint ptStart = { pReqInfo->vtPoints[ii].x, pReqInfo->vtPoints[ii].y };
		SPoint ptEnd = { pReqInfo->vtPoints[ii + 1].x, pReqInfo->vtPoints[ii + 1].y };

		double maxDist = MAX_SEARCH_DIST_FOR_FOREST;
		if (pReqInfo->MobilityOption == TYPE_MOBILITY_BICYCLE) {
			maxDist = MAX_SEARCH_DIST_FOR_BICYCLE; // 자전거 경탐은 자전거 최대 거리로 확장
		}
#if defined(USE_FOREST_DATA)
		else if (!pReqInfo->vtCourse[ii].empty()) {
			maxDist = MAX_SEARCH_DIST_FOR_COURSE; // 코스ID를 보유한 상태라면 코스 거리로 확장
		}
#endif

		if ((retDist = getRealWorldDistance(ptStart.x, ptStart.y, ptEnd.x, ptEnd.y)) > maxDist) {
			LOG_TRACE(LOG_ERROR, "Failed, max request route distance(%d) over than %d", retDist, maxDist);

			routeResult.ResultCode = ROUTE_RESULT_FAILED_DIST_OVER;
			continue;
		}


		// 시작점 정보
		routeInfo.StartLinkInfo = pReqInfo->vtPointsInfo[ii];

		if (ii == 0) {
			routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DEPARTURE; // 출발지
			if (pReqInfo->StartDirIgnore) { // 출발지 무시
				isDirIgnore = true;
			}
		} else {
			// 출발지 이외의 지점은 종료 링크에 속성정보가 들어가므로 링크 시작의 경유지 정보는 의미가 없음
			routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
			if (pReqInfo->WayDirIgnore) { // 경유지 무시
				isDirIgnore = true;
			}
		}

		routeInfo.reqInfo.StartDirIgnore = isDirIgnore;
		routeInfo.StartLinkInfo.KeyType = pReqInfo->vtKeyType[ii];
		routeInfo.StartLinkInfo.LinkDataType = pReqInfo->vtLinkDataType[ii];
		if (!SetRouteLinkInfo(ptStart, sLink, pReqInfo->vtLinkDataType[ii], BIDIRECTION, true, isDirIgnore, &routeInfo.StartLinkInfo)) {
			LOG_TRACE(LOG_ERROR, "Failed, set start link info, idx:%d", ii);
			if (ii == 0) {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_START;
			} else {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_VIA;
			}
			continue;
		}

		// 종료점 정보
		routeInfo.EndLinkInfo = pReqInfo->vtPointsInfo[ii + 1];

		if (ii + 1 == cntPoints - 1) {
			routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DESTINATION; // 도착지
			if (pReqInfo->EndDirIgnore) { // 경유지 무시
				isDirIgnore = true;
			}
		} else {
			// 경유지
			routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지			//LINK_GUIDE_TYPE_DEPARTURE_WAYPOINT, // 출발지-경유지
			//LINK_GUIDE_TYPE_DEPARTURE_WAYPOINT, // 출발지-경유지
			//LINK_GUIDE_TYPE_WAYPOINT_WAYPOINT, // 경유지-경유지
			//LINK_GUIDE_TYPE_WAYPOINT_DESTINATION, // 경유지-도착지
			if (pReqInfo->WayDirIgnore) { // 경유지 무시
				isDirIgnore = true;
			}
		}

		routeInfo.reqInfo.EndDirIgnore = isDirIgnore;
		routeInfo.EndLinkInfo.KeyType = pReqInfo->vtKeyType[ii + 1];
		routeInfo.EndLinkInfo.LinkDataType = pReqInfo->vtLinkDataType[ii + 1];
		if (!SetRouteLinkInfo(ptEnd, eLink, pReqInfo->vtLinkDataType[ii + 1], BIDIRECTION, false, isDirIgnore, &routeInfo.EndLinkInfo)) {
			LOG_TRACE(LOG_ERROR, "Failed, set end link info, idx:%d", ii + 1);
			if (ii < cntPoints - 1) {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_END;
			} else {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_VIA;
			}
			continue;
		}

#if defined(USE_FOREST_DATA)
		// 공유 코스 ID 확인
		if (!pReqInfo->vtCourse[ii].empty() && !pReqInfo->vtCourse[ii + 1].empty()) {
			for (const auto& course : pReqInfo->vtCourse[ii + 1]) {
				if (pReqInfo->vtCourse[ii].find(course) != pReqInfo->vtCourse[ii].end()) {
					routeInfo.CandidateCourse.emplace(course);
				}
			}
		}

		// 공유되는 코스가 없으면 모두 사용
		if (routeInfo.CandidateCourse.empty()) {
			// 시작점 코스 ID 예약
			if (!pReqInfo->vtCourse[ii].empty()) {
				routeInfo.CandidateCourse.insert(pReqInfo->vtCourse[ii].begin(), pReqInfo->vtCourse[ii].end());
			}

			// 종료점 코스 ID 예약
			if (!pReqInfo->vtCourse[ii + 1].empty()) {
				routeInfo.CandidateCourse.insert(pReqInfo->vtCourse[ii + 1].begin(), pReqInfo->vtCourse[ii + 1].end());
			}
		}
#endif

		bool isFromPed = true;

		// 숲길 시작 ->
		if (pReqInfo->vtLinkDataType[ii] != TYPE_LINK_DATA_PEDESTRIAN) {
			isFromPed = false;
		}

		// case 1 : 동종 링크 경로 탐색
		if (pReqInfo->vtLinkDataType[ii] == pReqInfo->vtLinkDataType[ii + 1])
		{
			// 보행자길->보행자길
			if (pReqInfo->vtLinkDataType[ii] == TYPE_LINK_DATA_PEDESTRIAN) {
				//////////////////////
				// make route
				ret = MakeRoute(ii, &routeInfo, &routeResult);

				if (ret == ROUTE_RESULT_SUCCESS) {
					ret = MakeRouteResultAttatchEx(&routeInfo, false, TYPE_KEY_LINK, TYPE_KEY_LINK, routeInfo.vtCandidateResult[0], routeInfo.StartLinkInfo.LinkGuideType, &routeResult);
				}
				//ret |= MakeRouteResultAttatch(&routeInfo, routeInfo.vtCandidateResult[0], true, &routeResult);
			}
			// 숲길 -> 숲길
			else if (pReqInfo->vtLinkDataType[ii] == TYPE_LINK_DATA_TREKKING) {
				// 동일 산일 경우
				// or 같은 산이라도 입구점이 동일하지 않아 다른 산으로 인식될 수 있으니 우선 탐색해보고
				////if (vtCpxRouteInfo[ii].id == vtCpxRouteInfo[ii + 1].id) { // 숲길 입구점 확장을 최대 100여개로 제한해 연결된 모든 숲을 탐색하지 않아 같은 숲이라도 id가 다를 수 있음 
				//if (vtCpxRouteInfo[ii].id == vtCpxRouteInfo[ii + 1].id)
				//{
				//	RouteInfo routeInfoTrk;
				//	routeInfoTrk.RequestMode = routeInfo.RequestMode;
				//	routeInfoTrk.RequestId = routeInfo.RequestId;
				//	routeInfoTrk.StartLinkInfo = routeInfo.StartLinkInfo;
				//	routeInfoTrk.EndLinkInfo = routeInfo.EndLinkInfo;
				//	routeInfoTrk.RouteOption = routeInfo.RouteOption;
				//	routeInfoTrk.m_routeSubOpt = routeInfo.m_routeSubOpt;
				//	routeInfoTrk.AvoidOption = routeInfo.AvoidOption;
				//	routeInfoTrk.StartDirIgnore = routeInfo.StartDirIgnore;
				//	routeInfoTrk.EndDirIgnore = routeInfo.EndDirIgnore;

				//	ret = MakeRoute(ii, &routeInfo, &routeResult);

				//	ret |= MakeRouteResultAttatchEx(&routeInfoTrk, TYPE_KEY_LINK, TYPE_KEY_LINK, routeInfoTrk.vtCandidateResult[0], routeInfoTrk.StartLinkInfo.LinkGuideType, &routeResult);
				//	//ret |= MakeRouteResultAttatch(&routeInfo, routeInfo.vtCandidateResult[0], true, &routeResult);
				//}
				//// 다른 산일 경우
				//else if (!vtCpxRouteInfo[ii].vtRoutePathInfo.empty() && !vtCpxRouteInfo[ii + 1].vtRoutePathInfo.empty()) 
				{			
					RouteInfo routeInfoTrk;

					// 동일 산일 경우
					//if ((vtCpxRouteInfo[ii].id == vtCpxRouteInfo[ii + 1].id) || (vtCpxRouteInfo[ii].id == vtCpxRouteInfo[ii + 1].id) ||
					//	((vtCpxRouteInfo[ii].id == 0 && vtCpxRouteInfo[ii + 1].id != 0) ||
					//	 (vtCpxRouteInfo[ii].id != 0 && vtCpxRouteInfo[ii + 1].id == 0)))
					if (((routeInfo.ComplexPointInfo.nGroupId != 0) && (routeInfo.ComplexPointInfo.nGroupId == routeInfoNext.ComplexPointInfo.nGroupId)) ||
						((routeInfo.reqInfo.RouteSubOption.mnt.course_type == TYPE_TRE_CROSS ||
							routeInfo.reqInfo.RouteSubOption.mnt.course_type == TYPE_TRE_RECOMMENDED) &&
						(routeInfo.ComplexPointInfo.id == 0) && (routeInfoNext.ComplexPointInfo.id == 0))) // 둘레길이면, 일반 탐색 우선 수행
					{
						routeInfoTrk.StartLinkInfo = routeInfo.StartLinkInfo;
						routeInfoTrk.EndLinkInfo = routeInfo.EndLinkInfo;

						routeInfoTrk.reqInfo.SetOption(&routeInfo.reqInfo);
						//routeInfoTrk.RequestMode = routeInfo.RequestMode;
						//routeInfoTrk.RequestId = routeInfo.RequestId;
						//routeInfoTrk.StartLinkInfo = routeInfo.StartLinkInfo;
						//routeInfoTrk.EndLinkInfo = routeInfo.EndLinkInfo;
						//routeInfoTrk.RouteOption = routeInfo.RouteOption;
						//routeInfoTrk.AvoidOption = routeInfo.AvoidOption;
						//routeInfoTrk.MobilityOption = routeInfo.MobilityOption;
						//routeInfoTrk.RouteSubOpt = routeInfo.RouteSubOpt;
						//routeInfoTrk.StartDirIgnore = routeInfo.StartDirIgnore;
						//routeInfoTrk.EndDirIgnore = routeInfo.EndDirIgnore;

#if defined(USE_FOREST_DATA)
						routeInfoTrk.CandidateCourse = routeInfo.CandidateCourse;
#endif

						ret = MakeRoute(ii, &routeInfoTrk, &routeResult);
					}

					if (ret == ROUTE_RESULT_SUCCESS) {
						ret = MakeRouteResultAttatchEx(&routeInfoTrk, false, TYPE_KEY_LINK, TYPE_KEY_LINK, routeInfoTrk.vtCandidateResult[0], routeInfoTrk.StartLinkInfo.LinkGuideType, &routeResult);
					}
					else {
						// 출발 산에서 도착 산의 입구점들을 표로 만들어 경로 탐색 수행
						// 최대 100개까지만 사용하자

						// N*N에 N*M 연산을 해야 되는 상황이라 우선, 가장 가까운 두 입구점의 보행 경로를 구해 제공하자, 2024-03-08
						int32_t idxBestPrev = -1;
						int32_t idxBestNext = -1;
						double minDist = INT64_MAX;
						double curDist = 0.f;
						for (int idxPrev = 0; idxPrev < routeInfo.ComplexPointInfo.vtEntryPoint.size(); idxPrev++) {
							for (int idxNext = 0; idxNext < routeInfoNext.ComplexPointReverseInfo.vtEntryPoint.size(); idxNext++) {
								curDist = getRealWorldDistance(routeInfo.ComplexPointInfo.vtEntryPoint[idxPrev].x, routeInfo.ComplexPointInfo.vtEntryPoint[idxPrev].y, routeInfoNext.ComplexPointReverseInfo.vtEntryPoint[idxNext].x, routeInfoNext.ComplexPointReverseInfo.vtEntryPoint[idxNext].y);

								if (curDist < minDist) {
									minDist = curDist;
									idxBestPrev = idxPrev;
									idxBestNext = idxNext;
								}
							} // for idxNext
						} // for idxPrev

						if (0 <= idxBestPrev && 0 <= idxBestNext) {
							RouteInfo routeInfoPed;
							RouteResultInfo routeResultPed;

							routeInfoPed.reqInfo.SetOption(&routeInfo.reqInfo);
							//// 경탐 옵션 
							//routeInfoPed.RouteOption = pReqInfo->RouteOption;
							//// 회피 옵션
							//routeInfoPed.AvoidOption = pReqInfo->AvoidOption;
							//// 이동체 옵션
							//routeInfoPed.MobilityOption = pReqInfo->MobilityOption;
							//// 추가 옵션
							//routeInfo.RouteSubOpt = pReqInfo->RouteSubOption;

							// 방향성 무시
							bool isDirIgnore = false;

							SPoint ptStart = { routeInfo.ComplexPointInfo.vtEntryPoint[idxBestPrev].x, routeInfo.ComplexPointInfo.vtEntryPoint[idxBestPrev].y };
							SPoint ptEnd = { routeInfoNext.ComplexPointReverseInfo.vtEntryPoint[idxBestNext].x, routeInfoNext.ComplexPointReverseInfo.vtEntryPoint[idxBestNext].y };

							if ((retDist = getRealWorldDistance(ptStart.x, ptStart.y, ptEnd.x, ptEnd.y)) > maxDist) {
								LOG_TRACE(LOG_ERROR, "Failed, max request route distance(%d) over than %d", retDist, maxDist);

								routeResult.ResultCode = ROUTE_RESULT_FAILED_DIST_OVER;
								//return routeResult.ResultCode;
							}

							// 시작점 정보
							routeInfoPed.StartLinkInfo = routeInfo.StartLinkInfo;

							routeInfoPed.StartLinkInfo.LinkId.tile_id = routeInfo.ComplexPointInfo.vtEntryPoint[idxBestPrev].nID_2 / 100000;
							routeInfoPed.StartLinkInfo.LinkId.nid = routeInfo.ComplexPointInfo.vtEntryPoint[idxBestPrev].nID_2 % 100000;
							routeInfoPed.StartLinkInfo.LinkId.dir = 0;
							routeInfoPed.StartLinkInfo.KeyType = TYPE_KEY_NODE;
							routeInfoPed.StartLinkInfo.LinkDataType = TYPE_LINK_DATA_PEDESTRIAN;

							if (ii == 0) {
								routeInfoPed.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DEPARTURE; // 출발지
								if (pReqInfo->StartDirIgnore) { // 출발지 무시
									isDirIgnore = true;
								}
							}
							else {
								routeInfoPed.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
								if (pReqInfo->WayDirIgnore) { // 경유지 무시
									isDirIgnore = true;
								}
							}

							// 방향성 무시
							routeInfoPed.reqInfo.StartDirIgnore = isDirIgnore;


							// 종료점 정보
							routeInfoPed.EndLinkInfo = routeInfo.EndLinkInfo;

							routeInfoPed.EndLinkInfo.LinkId.tile_id = routeInfoNext.ComplexPointReverseInfo.vtEntryPoint[idxBestNext].nID_2 / 100000;
							routeInfoPed.EndLinkInfo.LinkId.nid = routeInfoNext.ComplexPointReverseInfo.vtEntryPoint[idxBestNext].nID_2 % 100000;
							routeInfoPed.EndLinkInfo.LinkId.dir = 0;
							routeInfoPed.EndLinkInfo.KeyType = TYPE_KEY_NODE;
							routeInfoPed.EndLinkInfo.LinkDataType = TYPE_LINK_DATA_PEDESTRIAN;

							if (ii + 1 == cntPoints - 1) {
								routeInfoPed.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DESTINATION; // 도착지
								if (pReqInfo->EndDirIgnore) { // 경유지 무시
									isDirIgnore = true;
								}
							}
							else {
								routeInfoPed.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
								if (pReqInfo->WayDirIgnore) { // 경유지 무시
									isDirIgnore = true;
								}
							}

							// 방향성 무시
							routeInfoPed.reqInfo.EndDirIgnore = isDirIgnore;

							ret = MakeRoute(ii, &routeInfoPed, &routeResult);

							if (ret == ROUTE_RESULT_SUCCESS) {
								// 세 경로 결과를 합치자.

								// prev
								//pRouteInfos->at(ii).StartLinkInfo.KeyType = TYPE_KEY_LINK;
								//pRouteInfos->at(ii).EndLinkInfo.KeyType = TYPE_KEY_NODE;
								//ret |= MakeRouteResultAttatch(&pRouteInfos->at(ii), vtCpxRouteInfo[ii].vtRoutePathInfo[idxBestPrev], isFromPed, &routeResult);
								ret = MakeRouteResultAttatchEx(&pRouteInfos->at(ii), false, TYPE_KEY_LINK, TYPE_KEY_NODE, routeInfo.ComplexPointInfo.vtRoutePathInfo[idxBestPrev], pRouteInfos->at(ii).StartLinkInfo.LinkGuideType, &routeResult);

								// curr
								//routeInfoPed.StartLinkInfo.KeyType = TYPE_KEY_NODE;
								//routeInfoPed.EndLinkInfo.KeyType = TYPE_KEY_NODE;
								//ret |= MakeRouteResultAttatch(&routeInfoPed, routeInfoPed.vtCandidateResult[0], isFromPed, &routeResult);
								if (ret == ROUTE_RESULT_SUCCESS) {
									ret = MakeRouteResultAttatchEx(&routeInfoPed, false, TYPE_KEY_NODE, TYPE_KEY_NODE, routeInfoPed.vtCandidateResult[0], routeInfoPed.StartLinkInfo.LinkGuideType, &routeResult);
								}

								// next
								//pRouteInfos->at(ii + 1).StartLinkInfo.KeyType = TYPE_KEY_NODE;
								//pRouteInfos->at(ii + 1).EndLinkInfo.KeyType = TYPE_KEY_LINK;
								//ret |= MakeRouteResultAttatch(&pRouteInfos->at(ii + 1), vtCpxRouteInfo[ii + 1].vtRoutePathInfo[idxBestNext], isFromPed, &routeResult);
								//ret |= MakeRouteResultAttatchEx(&pRouteInfos->at(ii), true, TYPE_KEY_NODE, TYPE_KEY_LINK, routeInfoNext.ComplexPointInfo.vtRoutePathInfo[idxBestNext], pRouteInfos->at(ii).StartLinkInfo.LinkGuideType, &routeResult);
								if (ret == ROUTE_RESULT_SUCCESS) {
									ret = MakeRouteResultAttatchEx(&pRouteInfos->at(ii), true, TYPE_KEY_NODE, TYPE_KEY_LINK, routeInfoNext.ComplexPointReverseInfo.vtRoutePathInfo[idxBestNext], pRouteInfos->at(ii).StartLinkInfo.LinkGuideType, &routeResult);
								}
							}
						}
					}
				}
			}
			else {
				// 여기는 타면 안되는 곳이여
				LOG_TRACE(LOG_ERROR, "================= %s, %d =================", __FUNCTION__, __LINE__);
			}
		}
		// case 2 : 이종 링크 경로 탐색
		else if (pReqInfo->vtLinkDataType[ii] != pReqInfo->vtLinkDataType[ii + 1])
		{
			// 보행자길과 숲길에서 확인된 입구점들의 링크 확장으로 입구점 검색
			MultimodalPointInfo cpxRouteInfoPed;
			MultimodalPointInfo* pCpxRouteInfoTrk = nullptr;
			//ret = DoEntryPointRoute(&routeInfo, ii, isFromPed, vtCpxRouteInfo);
			if (isFromPed) {
				pCpxRouteInfoTrk = &routeInfoNext.ComplexPointReverseInfo;
			} else {
				pCpxRouteInfoTrk = &routeInfo.ComplexPointInfo;
			}
			ret = DoEntryPointRoute(&routeInfo, isFromPed, pCpxRouteInfoTrk, cpxRouteInfoPed);

			if (ret == ROUTE_RESULT_SUCCESS)
			{
				// 두 경로 결과의 합이 가장 좋은 케이스 선별
				double minValue = INT64_MAX;
				double curValue = INT64_MAX;
				int idxBest = -1;
				int cntResult = cpxRouteInfoPed.vtRoutePathInfo.size();
				for (int idx = 0; idx < cntResult; idx++) {
					if (pCpxRouteInfoTrk->vtRoutePathInfo[idx] == nullptr || cpxRouteInfoPed.vtRoutePathInfo[idx] == nullptr) {
						continue;
					}

					curValue = pCpxRouteInfoTrk->vtCost[idx] + cpxRouteInfoPed.vtCost[idx] * m_rpCost.pedestrian.cost_forest_base[routeInfo.reqInfo.RouteOption];
					//if (routeInfo.RouteOption == ROUTE_OPT_RECOMMENDED) {
					//	curValue = pCpxRouteInfoTrk->vtCost[idx] + cpxRouteInfoPed.vtCost[idx] * m_rpCost.pedestrian.cost_forest_base1;
					//} else if (routeInfo.RouteOption == ROUTE_OPT_COMFORTABLE) {
					//	curValue = pCpxRouteInfoTrk->vtCost[idx] + cpxRouteInfoPed.vtCost[idx] * m_rpCost.pedestrian.cost_forest_base2;
					//} else if (routeInfo.RouteOption == ROUTE_OPT_FASTEST) {
					//	curValue = pCpxRouteInfoTrk->vtTime[idx] + cpxRouteInfoPed.vtTime[idx] * m_rpCost.pedestrian.cost_forest_base3;
					//} else if (routeInfo.RouteOption == ROUTE_OPT_MAINROAD) {
					//	curValue = pCpxRouteInfoTrk->vtCost[idx] + cpxRouteInfoPed.vtCost[idx] * m_rpCost.pedestrian.cost_forest_base4;
					//} else { //if (routeInfo.RouteOption == ROUTE_OPT_SHORTEST) {
					//	curValue = pCpxRouteInfoTrk->vtDist[idx] + cpxRouteInfoPed.vtDist[idx];
					//}

					if ((curValue < minValue) && 
						(pCpxRouteInfoTrk->vtRoutePathInfo[idx] != nullptr && cpxRouteInfoPed.vtRoutePathInfo[idx] != nullptr)) {
						minValue = curValue;
						idxBest = idx;
					}

					//LOG_TRACE(LOG_DEBUG, "---- idx:%d, trk_cost: %.0f, ped_cost: %.0f, factor:%f", idx, pCpxRouteInfoTrk->vtCost[idx], cpxRouteInfoPed.vtCost[idx], m_rpCost.forest.cost_forest_pedestrian);
				} // for



				if (0 <= idxBest) {
					// 두 경로 결과를 합치자.

					routeResult.reqInfo.SetOption(&routeInfo.reqInfo);
					//// 경로 옵션
					//routeResult.RouteOption = routeInfo.RouteOption;
					//routeResult.RouteAvoid = routeInfo.AvoidOption;
					//routeResult.RouteMobility = routeInfo.MobilityOption;

					// 경탐 좌표
					routeResult.StartResultLink.Coord = routeInfo.StartLinkInfo.Coord;
					routeResult.EndResultLink.Coord = routeInfo.EndLinkInfo.Coord;

					// Data Type
					routeResult.StartResultLink.LinkDataType = routeInfo.StartLinkInfo.LinkDataType;
					routeResult.EndResultLink.LinkDataType = routeInfo.EndLinkInfo.LinkDataType;

					if (isFromPed) {
						ret = MakeRouteResultAttatchEx(&pRouteInfos->at(ii), false, TYPE_KEY_LINK, TYPE_KEY_NODE, cpxRouteInfoPed.vtRoutePathInfo[idxBest], pRouteInfos->at(ii).StartLinkInfo.LinkGuideType, &routeResult);

						if (ret == ROUTE_RESULT_SUCCESS) {
							ret = MakeRouteResultAttatchEx(&pRouteInfos->at(ii), true, TYPE_KEY_NODE, TYPE_KEY_LINK, pCpxRouteInfoTrk->vtRoutePathInfo[idxBest], pRouteInfos->at(ii).StartLinkInfo.LinkGuideType, &routeResult);
						}
					}
					else {
						ret = MakeRouteResultAttatchEx(&pRouteInfos->at(ii), false, TYPE_KEY_LINK, TYPE_KEY_NODE, pCpxRouteInfoTrk->vtRoutePathInfo[idxBest], pRouteInfos->at(ii).StartLinkInfo.LinkGuideType, &routeResult);

						if (ret == ROUTE_RESULT_SUCCESS) {
							ret = MakeRouteResultAttatchEx(&pRouteInfos->at(ii), true, TYPE_KEY_NODE, TYPE_KEY_LINK, cpxRouteInfoPed.vtRoutePathInfo[idxBest], pRouteInfos->at(ii).StartLinkInfo.LinkGuideType, &routeResult);
						}
					}
				}
			}
		}
		// default : 그외
		else 
		{
			// 여기 오면 안되지
			LOG_TRACE(LOG_ERROR, "Failed, link type matching route failed.");
		}


#if 0
		////////////////////////
		//// 시작점 정보
		//if (ii == 0) {
		//	routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DEPARTURE; // 출발지
		//	if (pReqInfo->StartDirIgnore) { // 출발지 무시
		//		isDirIgnore = true;
		//	}
		//}
		//else {
		//	routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
		//	if (pReqInfo->WayDirIgnore) { // 경유지 무시
		//		isDirIgnore = true;
		//	}
		//}

		//// 방향성 무시
		//routeInfo.StartDirIgnore = isDirIgnore;

		//routeInfo.StartLinkInfo.KeyType = pReqInfo->vtKeyType[ii];
		//routeInfo.StartLinkInfo.LinkDataType = pReqInfo->vtLinkDataType[ii];
		//if (routeInfo.StartLinkInfo.KeyType == TYPE_KEY_NODE) { // node type
		//	routeInfo.StartLinkInfo.LinkId = pReqInfo->vtKeyId[ii];
		//	routeInfo.StartLinkInfo.Coord = pReqInfo->vtPoints[ii];
		//}
		//else if (!SetRouteLinkInfo(ptStart, sLink, pReqInfo->vtLinkDataType[ii], true, isDirIgnore, &routeInfo.StartLinkInfo)) {
		//	LOG_TRACE(LOG_ERROR, "Failed, set start link info");

		//	if (ii == 0) {
		//		routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_START;
		//	}
		//	else {
		//		routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_VIA;
		//	}
		//	//return routeResult.ResultCode;
		//}

		////////////////////////
		//// 종료점 정보
		//if (ii + 1 == cntPoints - 1) {
		//	routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DESTINATION; // 도착지
		//	if (pReqInfo->EndDirIgnore) { // 경유지 무시
		//		isDirIgnore = true;
		//	}
		//}
		//else {
		//	routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
		//	if (pReqInfo->WayDirIgnore) { // 경유지 무시
		//		isDirIgnore = true;
		//	}
		//}

		//// 방향성 무시
		//routeInfo.EndDirIgnore = isDirIgnore;

		//routeInfo.EndLinkInfo.KeyType = pReqInfo->vtKeyType[ii + 1];
		//routeInfo.EndLinkInfo.LinkDataType = pReqInfo->vtLinkDataType[ii + 1];
		//if (routeInfo.EndLinkInfo.KeyType == TYPE_KEY_NODE) { // node type
		//	routeInfo.EndLinkInfo.LinkId = pReqInfo->vtKeyId[ii + 1];
		//	routeInfo.EndLinkInfo.Coord = pReqInfo->vtPoints[ii + 1];
		//}
		//else if (!SetRouteLinkInfo(ptEnd, eLink, pReqInfo->vtLinkDataType[ii], false, isDirIgnore, &routeInfo.EndLinkInfo)) {
		//	LOG_TRACE(LOG_ERROR, "Failed, set end link info");

		//	if (ii < cntPoints - 1) {
		//		routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_END;
		//	}
		//	else {
		//		routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_VIA;
		//	}
		//	//return routeResult.ResultCode;
		//}
#endif

		//////////////////////
		// make route
		//if (ret == ROUTE_RESULT_FAILED_EXPEND) {
		//	// 시작점의 방향성을 바꿔서 한번 더 재탐색 시도
		//	if (routeInfo.StartLinkInfo.LinkDir != 0) {
		//		int oldDir = routeInfo.StartLinkInfo.LinkDir;
		//		routeInfo.StartLinkInfo.LinkDir = (routeInfo.StartLinkInfo.LinkDir == 1) ? 2 : 1;

		//		ret = MakeRoute(ii, &routeInfo, &routeResult);
		//		LOG_TRACE(LOG_DEBUG, "Retry, make routes, result:%d, idx:%d, change start link dir, llid:%lld, tile:%d, id:%d, dir:%d->%d", ret, ii, routeInfo.StartLinkInfo.LinkId.llid, routeInfo.StartLinkInfo.LinkId.tile_id, routeInfo.StartLinkInfo.LinkId.nid, oldDir, routeInfo.StartLinkInfo.LinkId.dir);
		//	}
		//}

		if (ret == ROUTE_RESULT_SUCCESS) {
			routeResult.reqInfo.SetOption(&routeInfo.reqInfo);
			//routeResult.RequestMode = pReqInfo->RequestMode;
			//routeResult.RequestId = pReqInfo->RequestId;
			//routeResult.RouteOption = pReqInfo->RouteOption;
			//routeResult.RouteAvoid = pReqInfo->AvoidOption;
			//routeResult.RouteMobility = pReqInfo->MobilityOption;
		}
		else { // if (ret != ROUTE_RESULT_SUCCESS) {
			LOG_TRACE(LOG_DEBUG, "--->>>Failed, make routes, result:%d, idx:%d, change start link dir, llid:%lld, tile:%d, id:%d, ", ret, ii, routeInfo.StartLinkInfo.LinkId.llid, routeInfo.StartLinkInfo.LinkId.tile_id, routeInfo.StartLinkInfo.LinkId.nid);
			RouteResultLinkEx failedInfo = { 0, };
			// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
			failedInfo.guide_type = routeInfo.EndLinkInfo.LinkGuideType;
			routeResult.LinkInfo.emplace_back(failedInfo);
		}

		routeResult.ResultCode = ret;
	} // for

	// 1개라도 성공이면 성공으로 리턴하자
	int rets = ROUTE_RESULT_FAILED;
	for (const auto& result : *pRouteResults) {
		if (result.ResultCode == ROUTE_RESULT_SUCCESS) {
			rets = ROUTE_RESULT_SUCCESS;
			LOG_TRACE(LOG_DEBUG, "success, complex routing, routes:%d", cntPoints - 1);
			break;
		} else {
			rets = result.ResultCode;
		}
	}
	return rets;
}


const int CRoutePlan::DoCourse(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>* pRouteInfos, OUT vector<RouteResultInfo>* pRouteResults)
{
	if (pReqInfo == nullptr || pRouteInfos == nullptr || pRouteResults == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, request info param null, pReqInfo:%p, pRouteInfos:%p, pRouteResults:%p", pReqInfo, pRouteInfos, pRouteResults);

		return ROUTE_RESULT_FAILED;
	}

	const double maxDist = MAX_SEARCH_DIST_FOR_COURSE;
	int32_t cntPoints = pReqInfo->vtPoints.size();

	if (cntPoints < 2 || (cntPoints != pReqInfo->vtKeyId.size())) {
		LOG_TRACE(LOG_WARNING, "Failed, route request point too short, points:%d, links:%d", cntPoints, pReqInfo->vtKeyId.size());

		return ROUTE_RESULT_FAILED_WRONG_PARAM;
	}

	LOG_TRACE(LOG_DEBUG, "request, multi routing, points:%d", cntPoints);

	Initialize();

	pRouteInfos->resize(cntPoints - 1);
	pRouteResults->resize(cntPoints - 1);

#if defined(USE_MULTIPROCESS)
#pragma omp parallel for
#endif
	for (int ii = 0; ii < cntPoints - 1; ii++) {
		RouteInfo& routeInfo = pRouteInfos->at(ii);
		RouteResultInfo& routeResult = pRouteResults->at(ii);

		double retDist = 0.f;

		routeResult.reqInfo.SetOption(&routeInfo.reqInfo);
		//// 경탐 옵션 
		//routeInfo.RouteOption = pReqInfo->RouteOption;
		//// 회피 옵션
		//routeInfo.AvoidOption = pReqInfo->AvoidOption;
		//// 이동체
		//routeInfo.MobilityOption = pReqInfo->MobilityOption;
		//// 추가 옵션
		//routeInfo.RouteSubOpt = pReqInfo->RouteSubOption;

		// 방향성 무시
		bool isDirIgnore = false;


		KeyID sLink = pReqInfo->vtKeyId[ii];
		KeyID eLink = pReqInfo->vtKeyId[ii + 1];

		SPoint ptStart = { pReqInfo->vtPoints[ii].x, pReqInfo->vtPoints[ii].y };
		SPoint ptEnd = { pReqInfo->vtPoints[ii + 1].x, pReqInfo->vtPoints[ii + 1].y };

		if ((retDist = getRealWorldDistance(ptStart.x, ptStart.y, ptEnd.x, ptEnd.y)) > maxDist) {
			LOG_TRACE(LOG_ERROR, "Failed, max request route distance(%d) over than %d", retDist, maxDist);

			routeResult.ResultCode = ROUTE_RESULT_FAILED_DIST_OVER;
			//return routeResult.ResultCode;
		}

		// 시작점 정보
		routeInfo.StartLinkInfo.KeyType = pReqInfo->vtKeyType[ii];

		if (ii == 0) {
			routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DEPARTURE; // 출발지
			if (pReqInfo->StartDirIgnore) { // 출발지 무시
				isDirIgnore = true;
			}
		}
		else {
			// 출발지 이외의 지점은 종료 링크에 속성정보가 들어가므로 링크 시작의 경유지 정보는 의미가 없음
			routeInfo.StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
			if (pReqInfo->WayDirIgnore) { // 경유지 무시
				isDirIgnore = true;
			}
		}

		// 방향성 무시
		routeInfo.reqInfo.StartDirIgnore = isDirIgnore;

		if (!SetRouteLinkInfo(ptStart, sLink, pReqInfo->vtLinkDataType[ii], BIDIRECTION, true, isDirIgnore, &routeInfo.StartLinkInfo)) {
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
		routeInfo.EndLinkInfo.KeyType = pReqInfo->vtKeyType[ii];

		if (ii + 1 == cntPoints - 1) {
			routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DESTINATION; // 도착지
			if (pReqInfo->EndDirIgnore) { // 경유지 무시
				isDirIgnore = true;
			}
		}
		else {
			// 경유지
			routeInfo.EndLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지			//LINK_GUIDE_TYPE_DEPARTURE_WAYPOINT, // 출발지-경유지
			//LINK_GUIDE_TYPE_DEPARTURE_WAYPOINT, // 출발지-경유지
			//LINK_GUIDE_TYPE_WAYPOINT_WAYPOINT, // 경유지-경유지
			//LINK_GUIDE_TYPE_WAYPOINT_DESTINATION, // 경유지-도착지
			if (pReqInfo->WayDirIgnore) { // 경유지 무시
				isDirIgnore = true;
			}
		}

		// 방향성 무시
		routeInfo.reqInfo.EndDirIgnore = isDirIgnore;

		if (!SetRouteLinkInfo(ptEnd, eLink, pReqInfo->vtLinkDataType[ii], BIDIRECTION, false, isDirIgnore, &routeInfo.EndLinkInfo)) {
			LOG_TRACE(LOG_ERROR, "Failed, set end link info");

			if (ii < cntPoints - 1) {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_END;
			}
			else {
				routeResult.ResultCode = ROUTE_RESULT_FAILED_SET_VIA;
			}
			//return routeResult.ResultCode;
		}

		int ret = MakeCourse(ii, &routeInfo, &routeResult);
		//if (ret == ROUTE_RESULT_FAILED_EXPEND) {
		//	// 시작점의 방향성을 바꿔서 한번 더 재탐색 시도
		//	if (routeInfo.StartLinkInfo.LinkDir != 0) {
		//		int oldDir = routeInfo.StartLinkInfo.LinkDir;
		//		routeInfo.StartLinkInfo.LinkDir = (routeInfo.StartLinkInfo.LinkDir == 1) ? 2 : 1;

		//		ret = MakeCourse(ii, &routeInfo, &routeResult);
		//		LOG_TRACE(LOG_DEBUG, "Retry, make routes, result:%d, idx:%d, change start link dir, llid:%lld, tile:%d, id:%d, dir:%d->%d", ret, ii, routeInfo.StartLinkInfo.LinkId.llid, routeInfo.StartLinkInfo.LinkId.tile_id, routeInfo.StartLinkInfo.LinkId.nid, oldDir, routeInfo.StartLinkInfo.LinkId.dir);
		//	}
		//}

		if (ret == ROUTE_RESULT_SUCCESS) {
			ret = MakeRouteResult(&routeInfo, &routeResult);

			if (ret != ROUTE_RESULT_SUCCESS) {
				LOG_TRACE(LOG_INFO, "Failed, req_idx:%d, can't find route, couldn't leached end point(the point will isolated)", ii);
				LOG_TRACE(LOG_INFO, "Failed, req_idx:%d, start(link:%d, coord:%.6f,%.6f), end(link:%d, coord:%.6f,%.6f)", ii,
					routeInfo.StartLinkInfo.LinkId.nid, routeInfo.StartLinkInfo.Coord.x, routeInfo.StartLinkInfo.Coord.y,
					routeInfo.EndLinkInfo.LinkId.nid, routeInfo.EndLinkInfo.Coord.x, routeInfo.EndLinkInfo.Coord.y);
				/* display
				WCHAR buff[128];
				while (!pqScore.empty())
				{
				wsprintf(buff, L"%d, %d \n", pqScore.top().id, pqScore.top().length);
				OutputDebugString(buff);
				pqScore.pop();
				}
				*/
				ret = ROUTE_RESULT_FAILED_EXPEND;
			}
		}
		else { // if (ret != ROUTE_RESULT_SUCCESS) {
			LOG_TRACE(LOG_DEBUG, "--->>>Failed, make routes, result:%d, idx:%d, change start link dir, llid:%lld, tile:%d, id:%d, ", ret, ii, routeInfo.StartLinkInfo.LinkId.llid, routeInfo.StartLinkInfo.LinkId.tile_id, routeInfo.StartLinkInfo.LinkId.nid);
			RouteResultLinkEx failedInfo = { 0, };
			// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
			failedInfo.guide_type = routeInfo.EndLinkInfo.LinkGuideType;
			routeResult.LinkInfo.emplace_back(failedInfo);
		}

		routeResult.ResultCode = ret;
	} // for

	// 1개라도 성공이면 성공으로 리턴하자
	int rets = ROUTE_RESULT_FAILED;
	for (const auto& result : *pRouteResults) {
		if (result.ResultCode == ROUTE_RESULT_SUCCESS) {
			rets = ROUTE_RESULT_SUCCESS;
			LOG_TRACE(LOG_DEBUG, "success, multi routing, routes:%d", cntPoints - 1);
			break;
		} else {
			rets = result.ResultCode;
		}
	}
	return rets;
}


#if defined(USE_TSP_MODULE)
const int CRoutePlan::DoTabulate(IN const RequestRouteInfo* pReqInfo, OUT RouteDistMatrix& RDM)
{
	//if (pReqInfo == nullptr || pRouteInfos == nullptr || pRouteResults == nullptr) {
	//	LOG_TRACE(LOG_ERROR, "Failed, request info param null, pReqInfo:%p, pRouteInfos:%p, pRouteResults:%p", pReqInfo, pRouteInfos, pRouteResults);

	//	return ROUTE_RESULT_FAILED;
	//}
	if (pReqInfo == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, request info param null, pReqInfo:%p", pReqInfo);

		return ROUTE_RESULT_FAILED;
	}

	int32_t cntPoints = pReqInfo->vtPoints.size();

	if (cntPoints <= 2 || (cntPoints != pReqInfo->vtKeyId.size())) {
		LOG_TRACE(LOG_WARNING, "Failed, route request point too short, points:%d, links:%d", cntPoints, pReqInfo->vtKeyId.size());

		return ROUTE_RESULT_FAILED_WRONG_PARAM;
	}


	Initialize();

	RouteResultInfo routeResult;
	vector<RouteLinkInfo> mapLocations;

	for (int ii = 0; ii < cntPoints; ii++) {
		RouteLinkInfo newInfo;

		KeyID keyLink =  pReqInfo->vtKeyId[ii];
		SPoint ptRequest = { pReqInfo->vtPoints[ii].x, pReqInfo->vtPoints[ii].y };

		// 각 지점의 매칭 정보
		if (!SetRouteLinkInfo(ptRequest, keyLink, pReqInfo->vtLinkDataType[ii], BIDIRECTION, false, pReqInfo->WayDirIgnore, &newInfo)) {
			LOG_TRACE(LOG_ERROR, "Failed, set tabuate link info, idx:%d, map:%d, link;%d, x:%.5f, y:%.5f", ii, keyLink.tile_id, keyLink.nid, ptRequest.x, ptRequest.y);

			return ROUTE_RESULT_FAILED_SET_VIA;
		}

		mapLocations.emplace_back(newInfo);
	} // for

	int ret = 0;
	if (pReqInfo->RouteSubOption.rdm.expand_method == 1) {
		ret = MakeTabulateEx(pReqInfo, mapLocations, RDM);
	} else {
		ret = MakeTabulate(pReqInfo, mapLocations, RDM);
	}

	if (ret == ROUTE_RESULT_SUCCESS) {
		routeResult.reqInfo.SetOption(pReqInfo);
		//routeResult.RequestMode = pReqInfo->RequestMode;
		//routeResult.RequestId = pReqInfo->RequestId;
		//routeResult.RouteOption = pReqInfo->RouteOption;
		//routeResult.RouteAvoid = pReqInfo->AvoidOption;
		//routeResult.RouteMobility = pReqInfo->MobilityOption;
	}

	//pRouteResults->emplace_back(routeResult);

	return ret;
}
#endif // #if defined(USE_TSP_MODULE)


int32_t captureResultPath(IN const CandidateLink* pCurInfo, IN vector<CandidateLink*>& vtCandidateResult)
{
	//if (pRouteInfo->EndLinkInfo.KeyType == TYPE_KEY_LINK) {
	//	// 큐 트리 깊이 + 1 (마지막은 저장안되니까)가 전체 경로 링크 사이즈가 됨
	//	pRouteInfo->vtCandidateResult.reserve(pCurInfo->depth + 1);

	//	// 종료점 정보 최초 등록
	//	pRouteInfo->vtCandidateResult.emplace_back(pCurInfo);
	//}
	//else {
	//	// 노드는 마지막 링크가 없기에 depth가 목적지다
	//	pRouteInfo->vtCandidateResult.reserve(pCurInfo->depth);
	//}

	//unordered_map<uint64_t, CandidateLink*>::const_iterator it;
	const int maxLinkDepth = pCurInfo->depth;
	int visitStartLinkCnt = 0;
	bool isSuccess = false;
	CandidateLink* pCur = const_cast<CandidateLink*>(pCurInfo);
	//CandidateLink candidateLink;

	vtCandidateResult.reserve(maxLinkDepth + 1);

	while (!isSuccess)
	{
		vtCandidateResult.emplace_back(pCur);
		//memcpy(&candidateLink, pCur, sizeof(candidateLink));
		//vtCandidateResult.emplace_back(candidateLink);

		if ((pCur->pPrevLink == nullptr) || (pCur->parentLinkId.parents_id <= 2)) {
			isSuccess = true;
			//break;
		} // 시작 지점 확인
		else {
			visitStartLinkCnt++;
			pCur = pCur->pPrevLink;
		}
		
		if (maxLinkDepth < visitStartLinkCnt) {
			LOG_TRACE(LOG_WARNING, "Failed, max result path depth over than result, max:%d, now:%d", maxLinkDepth, visitStartLinkCnt);
			break;
		}
		//it = pRouteInfo->mRoutePass.find(pCurInfo->candidateId.llid);

		//if (it == pRouteInfo->mRoutePass.end())
		//{
		//	finding = false;
		//	LOG_TRACE(LOG_ERROR, "Failed, req_idx:%d, can't find start link", idx);
		//}
		//else
		//{
		//	pCurInfo = it->second;
		//	pRouteInfo->vtCandidateResult.emplace_back(pCurInfo);

		//	// 시작 지점 확인
		//	if (((pRouteInfo->StartLinkInfo.KeyType == TYPE_KEY_NODE) && // 시작점이 노드
		//		(pCurInfo->parentLinkId.nid == 1 || pCurInfo->parentLinkId.nid == 2) && (pCurInfo->depth == 0)) || // 최초 등록
		//		(pCurInfo->linkId.llid == pRouteInfo->StartLinkInfo.LinkId.llid)) // 시작점이 링크
		//	{
		//		visitStartLinkCnt++;
		//		if (pRouteInfo->StartLinkInfo.LinkDir == 0 || (pRouteInfo->StartLinkInfo.LinkDir != 0 && pCurInfo->dir == pRouteInfo->StartLinkInfo.LinkDir)) {
		//			finding = false;
		//			LOG_TRACE(LOG_TEST, "Success find start link, req_idx:%d", idx);
		//		}
		//	}
		//}

		//// 시작점을 유턴포함 2번이상 지날일이 없으니까
		//if (finding == true && (visitStartLinkCnt >= 2 || pRouteInfo->vtCandidateResult.size() >= pRouteInfo->mRoutePass.size())) {
		//	finding = false;
		//	LOG_TRACE(LOG_WARNING, "Failed, req_idx:%d, can't find start link, finding count over the base, cnt:%d", idx, cntExtraSearch);
		//}
	} // while

	if (isSuccess) {
		LOG_TRACE(LOG_TEST, "Success find start link, req_idx:%d", visitStartLinkCnt);
	}

	return visitStartLinkCnt;
}


const int CRoutePlan::MakeRoute(IN const int idx, IN RouteInfo* pRouteInfo, OUT RouteResultInfo* pRouteResult)
{
	const stLinkInfo* pLink = nullptr;
	const stNodeInfo* pNode = nullptr;

	// 종료 지점의 탐색 매칭 카운트 계산
	uint32_t cntGoal = 0; // 목적지 도달 가능한 링크 수 
	uint32_t cntGoalTouch = 0; // 목적지 도달한 링크 수
	CandidateLink* checkFinishRouteDir[2];// = { 0, }; // 최대 링크 양단에서 한번씩만 허용
	//checkFinishRouteDir[0].costTreavel = DBL_MAX;
	//checkFinishRouteDir[1].costTreavel = DBL_MAX;

	int ret = ROUTE_RESULT_FAILED;

	if (pRouteInfo == nullptr || pRouteResult == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, req_idx:%d, input param null, pRouteInfo:%p, pRouteResult:%p", idx, pRouteInfo, pRouteResult);
		return ret;
	}


	// 종료 정보 등록
	// 종료 지점이 노드일 경우
	if (pRouteInfo->EndLinkInfo.KeyType == TYPE_KEY_NODE) // node type
	{ 
		pNode = m_pDataMgr->GetNodeDataById(pRouteInfo->EndLinkInfo.LinkId, pRouteInfo->EndLinkInfo.LinkDataType);

		if (!pNode)
		{
			LOG_TRACE(LOG_WARNING, "Failed, req_idx:%d, can't find end node, tile:%d, node:%d", idx, pRouteInfo->EndLinkInfo.LinkId.tile_id, pRouteInfo->EndLinkInfo.LinkId.nid);

			ret = ROUTE_RESULT_FAILED_READ_DATA;
			return ret;
		}

		pRouteInfo->EndLinkInfo.MatchCoord = pNode->coord;

		if (pNode->base.node_type == TYPE_NODE_DATA_VEHICLE) { // 차량은 패스코드 확인
			for (int ii = 0; ii < pNode->base.connnode_count; ii++) {
				if (getPrevPassCode(pNode, pNode->connnodes[ii], m_pDataMgr) != PASS_CODE_DISABLE) { // 통행 가능 확인
					cntGoal++;
				}
			} // for 
		}
		else if (pNode->base.connnode_count >= 1) {
			cntGoal++;
		}
	}
	else // 종료 지점이 링크일 경우
	{ // linke type
		pLink = m_pDataMgr->GetLinkDataById(pRouteInfo->EndLinkInfo.LinkId, pRouteInfo->EndLinkInfo.LinkDataType);

		if (!pLink)
		{
			LOG_TRACE(LOG_WARNING, "Failed, req_idx:%d, can't find end link, tile:%d, link:%d", idx, pRouteInfo->EndLinkInfo.LinkId.tile_id, pRouteInfo->EndLinkInfo.LinkId.nid);

			ret = ROUTE_RESULT_FAILED_READ_DATA;
			return ret;
		}

		// s 노드에서의 진입은 정방향
		if (pRouteInfo->EndLinkInfo.LinkDir == 0 || (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 1)) && (pRouteInfo->EndLinkInfo.LinkDir == 1))) // 양방향 or 정방향
		{
			pNode = m_pDataMgr->GetNodeDataById(pLink->snode_id, pLink->base.link_type);

			// S 노드가 단점 아닌지 확인
			if ((pNode && pNode->base.point_type != TYPE_NODE_END && pNode->base.connnode_count >= 2) || 
				(pNode && pNode->base.point_type == TYPE_NODE_EDGE && pNode->base.connnode_count >= 1)) { // 구획변교점
				cntGoal++;
			}
		}

		// e 노드에서의 진입은 역방향
		if (pRouteInfo->EndLinkInfo.LinkDir == 0 || (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 2)) && (pRouteInfo->EndLinkInfo.LinkDir == 2))) // 양방향 or 역방향
		{
			pNode = m_pDataMgr->GetNodeDataById(pLink->enode_id, pLink->base.link_type);

			// E 노드가 단점 아닌지 확인
			if ((pNode && pNode->base.point_type != TYPE_NODE_END && pNode->base.connnode_count >= 2) || 
				(pNode && pNode->base.point_type == TYPE_NODE_EDGE && pNode->base.connnode_count >= 1)) { // 구획변교점
				cntGoal++;
			}
		}
	}

	// 시작 정보 등록
	// 시작 지점이 노드일 경우
	if (pRouteInfo->StartLinkInfo.KeyType == TYPE_KEY_NODE) // node type
	{ 
		pNode = m_pDataMgr->GetNodeDataById(pRouteInfo->StartLinkInfo.LinkId, pRouteInfo->StartLinkInfo.LinkDataType);

		if (!pNode)
		{
			LOG_TRACE(LOG_WARNING, "Failed, req_idx:%d, can't find start node, tile:%d, node:%d", idx, pRouteInfo->EndLinkInfo.LinkId.tile_id, pRouteInfo->EndLinkInfo.LinkId.nid);

			pRouteResult->ResultCode = ROUTE_RESULT_FAILED_READ_DATA;
			return pRouteResult->ResultCode;
		}

		pRouteInfo->EndLinkInfo.MatchCoord = pNode->coord;

		for (int ii = 0; ii < pNode->base.connnode_count; ii++) {
			if ((pNode->base.node_type != TYPE_NODE_DATA_VEHICLE) || 
				((pNode->base.node_type == TYPE_NODE_DATA_VEHICLE) && // 차량은 패스코드 확인
			     (getPrevPassCode(pNode, pNode->connnodes[ii], m_pDataMgr) != PASS_CODE_DISABLE))) // 통행 가능 확인 
			{
				pLink = m_pDataMgr->GetLinkDataById(pNode->connnodes[ii], pNode->base.node_type);

				if (!pLink)
				{
					LOG_TRACE(LOG_WARNING, "Failed, req_idx:%d, can't find start link, tile:%d, link:%d", idx, pRouteInfo->StartLinkInfo.LinkId.tile_id, pRouteInfo->StartLinkInfo.LinkId.nid);
					pRouteResult->ResultCode = ROUTE_RESULT_FAILED_READ_DATA;
					return pRouteResult->ResultCode;
				}

				KeyID candidateId;
				KeyID targetNodeId; // 진행 방향 노드 ID
				int dir = 0; // 0:양방향, 1:정방향, 2:역방향

				if (pNode->node_id == pLink->snode_id) {
					dir = 1; // 정방향
					targetNodeId = pLink->enode_id;  // 진행 방향 노드 ID					
				} else if (pNode->node_id == pLink->enode_id) {
					dir = 2; // 역방향
					targetNodeId = pLink->snode_id;  // 진행 방향 노드 ID
				} else {
					LOG_TRACE(LOG_WARNING, "Failed, can't find start node matchin link, node tile:%d, id:%d", pNode->node_id.tile_id, pNode->node_id.nid);
					pRouteResult->ResultCode = ROUTE_RESULT_FAILED_READ_DATA;
					return pRouteResult->ResultCode;
				}
				
				candidateId.parents_id = dir; // 방향성, 최초는 부모가 없으니까 진행 방향을 값으로 주자
				candidateId.current_id = pLink->link_id.nid;

				CandidateLink* pItem = new CandidateLink;
				pItem->candidateId = candidateId; // 후보 ID
				pItem->parentLinkId.llid = candidateId.parents_id; // 부모 링크 ID
				pItem->linkId = pLink->link_id; // 링크 ID
				pItem->nodeId = targetNodeId; // 노드 ID
				pItem->dist = pLink->length; // 실 거리
				pItem->time = GetCost(&pRouteInfo->reqInfo, pLink, dir, pLink->length, SPEED_NOT_AVALABLE); // 실 주행 시간
				pItem->cost = pItem->time; // 논리적 주행 시간
				pItem->distTreavel = pItem->dist; // 시작 거리
				pItem->timeTreavel = pItem->time; // 시작 시간
				pItem->costTreavel = pItem->cost; // 시작 비용
				pItem->costHeuristic = 0; // pLink->length * VAL_HEURISTIC_VEHICLE_FACTOR;	// 가중치 계산 비용
				pItem->linkDataType = pRouteInfo->StartLinkInfo.LinkDataType;
				pItem->depth = 0;	// 탐색 깊이
				pItem->visited = false; // 방문 여부
				pItem->dir = dir; // 탐색방향
				pItem->angle = 0; // 진입각도

				pItem->pPrevLink = nullptr;

				// 링크 방문 목록 등록
				pRouteInfo->mRoutePass.emplace(candidateId.llid, pItem);
				pRouteInfo->pqDijkstra.emplace(pItem);
			}
		} // for 
	}
	else // 시작 지점이 링크일 경우
	{ // linke type
		pLink = m_pDataMgr->GetLinkDataById(pRouteInfo->StartLinkInfo.LinkId, pRouteInfo->StartLinkInfo.LinkDataType);

		if (!pLink) {
			LOG_TRACE(LOG_WARNING, "Failed, req_idx:%d, can't find start link, tile:%d, link:%d", idx, pRouteInfo->StartLinkInfo.LinkId.tile_id, pRouteInfo->StartLinkInfo.LinkId.nid);

			ret = ROUTE_RESULT_FAILED_READ_DATA;
			return ret;
		}

#if 1 // 함수로 대체
		vector<CandidateLink*> vtCandidateInfo;
		CheckStartDirectionMaching(&pRouteInfo->reqInfo, pLink, &pRouteInfo->StartLinkInfo, vtCandidateInfo);

		// 링크 방문 목록 등록
		for (const auto& item : vtCandidateInfo) {
			pRouteInfo->mRoutePass.emplace(item->candidateId.llid, item);
			pRouteInfo->pqDijkstra.emplace(item);
		}
#else
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
			pItem->parentLinkId.llid = candidateId.parents_id; // 부모 링크 ID
			pItem->linkId = pLink->link_id; // 링크 ID
			pItem->nodeId = pLink->snode_id; // 진행 방향 노드 ID
			pItem->dist = pRouteInfo->StartLinkInfo.LinkDistToS; // 실 거리
			pItem->time = GetCost(pLink, candidateId.parents_id, pRouteInfo->RouteOption, pRouteInfo->StartLinkInfo.LinkDistToS, SPEED_NOT_AVALABLE); // 실 주행 시간
			pItem->cost = pItem->time; // 논리적 주행 시간
			pItem->distTreavel = pItem->dist; // 시작 거리
			pItem->timeTreavel = pItem->time; // 시작 시간
			pItem->costTreavel = pItem->cost;	// 계산 비용
			pItem->costHeuristic = 0; // pRouteInfo->StartLinkInfo.LinkDistToS * VAL_HEURISTIC_VEHICLE_FACTOR;	// 가중치 계산 비용, 첫 링크는 가중치를 최하(10%)로 설정 무조건 처음부터 확인하도록(양방향일때)
			pItem->dataType = pRouteInfo->StartLinkInfo.DataType;
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
			pItem->parentLinkId.llid = candidateId.parents_id; // 부모 링크 ID
			pItem->linkId = pLink->link_id; // 링크 ID
			pItem->nodeId = pLink->enode_id; // 진행 방향 노드 ID
			pItem->dist = pRouteInfo->StartLinkInfo.LinkDistToE;	// 실 거리
			pItem->time = GetCost(pLink, candidateId.parents_id, pRouteInfo->RouteOption, pRouteInfo->StartLinkInfo.LinkDistToE, SPEED_NOT_AVALABLE); // 실 주행 시간
			pItem->cost = pItem->time;
			pItem->distTreavel = pItem->dist; // 시작 거리
			pItem->timeTreavel = pItem->time; // 시작 시간
			pItem->costTreavel = pItem->cost;	// 계산 비용
			pItem->costHeuristic = 0; // pRouteInfo->StartLinkInfo.LinkDistToE * VAL_HEURISTIC_VEHICLE_FACTOR; // 가중치 계산 비용, 첫 링크는 가중치를 최하로 설정 무조건 처음부터 확인하도록(양방향일때)
			pItem->dataType = pRouteInfo->StartLinkInfo.DataType;
			pItem->depth = 0;	// 탐색 깊이
			pItem->visited = false; // 방문 여부
			pItem->dir = candidateId.parents_id; // 탐색방향

			pItem->pPrevLink = const_cast<CandidateLink*>(pCurInfo);

			// 링크 방문 목록 등록
			pRouteInfo->mRoutePass.emplace(candidateId.llid, pItem);
			pRouteInfo->pqDijkstra.emplace(pItem);
		}
#endif 
	}
	// LOG_TRACE(LOG_DEBUG, "Start tree : link:%d", pLink->link_id.nid);


	CandidateLink* pCurInfo;
	static const uint32_t cntMaxExtraSearch = 100; // 목적지 추가 도달 검사 최대 허용치
	uint32_t cntExtraSearch = 0; // 목적지 도달 후 추가 도달 검사 카운트

	// 경로 탐색
	LOG_TRACE(LOG_TEST, "Start finding goal, req_idx:%d", idx);

	int cntAddedLink = 0;
	int nProcess = 0;
	// debug for status
#if defined(_DEBUG)
	int nProcessGap = 1000;
#else
	int nProcessGap = 3000;
#endif

	int32_t cntStartLink = pRouteInfo->pqDijkstra.size(); // 출발지 등록 링크 수(링크일 경우 양끝으로 2개, 노드일 경우, 연결링크 수만큼)

	while (!pRouteInfo->pqDijkstra.empty())
	{
		nProcess++;

#if defined(_WIN32)
		routeProcessPrint(nProcess);
#endif // #if defined(_WIN32)

		// 현재 링크 트리에서 제거
		pCurInfo = pRouteInfo->pqDijkstra.top(); pRouteInfo->pqDijkstra.pop();
		pCurInfo->visited = true;


#if defined(USE_SHOW_ROUTE_SATATUS)
		//LOG_TRACE(LOG_DEBUG, "Current Link : id:%lld(real cost:%f, heuristic cost:%f, lv:%d)", curInfo.linkId, curInfo.costReal, curInfo.costHeuristic, curInfo.depth);

		// 현재까지 탐색된 정보 전달, Function Call
		if (m_fpRoutingStatus) {
			if (pRouteInfo->reqInfo.vtPoints.size() <= 1 || idx == 0) {// 단일 경로 또는 다중일때 첫번째 옵션만
				m_fpRoutingStatus(m_pHost, &pRouteInfo->mRoutePass);
			}
		}
#endif

		if (pRouteInfo->EndLinkInfo.KeyType == TYPE_KEY_NODE)
		//if (((pRouteInfo->EndLinkInfo.KeyType == TYPE_KEY_NODE) && // 목적 지점이 노드 타입이고, 
		//	((pRouteInfo->EndLinkInfo.LinkId.llid == pCurInfo->nodeId.llid))) || // 노드 ID 가 같거나
		//	(cntMaxExtraSearch < cntExtraSearch)) // 최대 목적지 도착 횟수를 넘어서면
		{
			//if (pRouteInfo->EndLinkInfo.LinkId.llid == pCurInfo->nodeId.llid) {// 노드 ID 가 같으면
			//	// 링크 기준이기에, 순서상 이미 추가된 링크를 기준으로 목적지 노드 다음을 가리키므로, 이전의 링크까지가 결과 링크에 포함된다.
			//	if (pCurInfo->pPrevLink != nullptr) {
			//		memcpy(&checkFinishRouteDir[cntGoalTouch], pCurInfo->pPrevLink, sizeof(checkFinishRouteDir[cntGoalTouch]));
			//	} else {
			//		// -----------??? 이건 출발링크가 목적 노드와 동일한 경우인건가?????
			//		memcpy(&checkFinishRouteDir[cntGoalTouch], pCurInfo, sizeof(checkFinishRouteDir[cntGoalTouch]));
			//	}
			//	cntGoalTouch++;
			//}



			pNode = GetNextNode(pCurInfo);

			if (pNode && (pRouteInfo->EndLinkInfo.LinkId.llid == pNode->node_id.llid)) // 노드 ID 가 같으면
			{
				// 현재 링크의 다음 노드가 목적지면, 현재 링크까지가 경로 결과에 포함된다.
				//memcpy(&checkFinishRouteDir[cntGoalTouch], pCurInfo, sizeof(checkFinishRouteDir[cntGoalTouch]));
				checkFinishRouteDir[cntGoalTouch] = pCurInfo;
				cntGoalTouch++;
			}
		}
		else if (((pCurInfo->linkId.llid == pRouteInfo->EndLinkInfo.LinkId.llid) && // 목적지 링크 이면서
			((pRouteInfo->reqInfo.StartDirIgnore || pRouteInfo->reqInfo.EndDirIgnore) || // 방향성 무시거나
			 ((((pRouteInfo->EndLinkInfo.LinkId == pRouteInfo->StartLinkInfo.LinkId) && (nProcess <= cntStartLink) && // 출도착 동일 + 링크 시작이면서, 
			    (((((pRouteInfo->EndLinkInfo.LinkDir == 0) || (pRouteInfo->EndLinkInfo.LinkDir == 1)) && pCurInfo->dir == 1) && // 진행방향(출발->도착) 일때
			      (pRouteInfo->StartLinkInfo.LinkDistToS <= pRouteInfo->EndLinkInfo.LinkDistToS)) || // E -> S
			     ((((pRouteInfo->EndLinkInfo.LinkDir == 0) || (pRouteInfo->EndLinkInfo.LinkDir == 2)) && pCurInfo->dir == 2) && // 진행방향(출발->도착) 일때
			      (pRouteInfo->StartLinkInfo.LinkDistToE <= pRouteInfo->EndLinkInfo.LinkDistToE))) // S -> E
		     ))) || 
			 ((pRouteInfo->StartLinkInfo.KeyType == TYPE_KEY_NODE) && (pRouteInfo->EndLinkInfo.LinkDir == 0) && (nProcess <= cntStartLink)
		     ) || // 시작지점이 노드고, 출발링크들과의 매칭일때
			 ((((pRouteInfo->EndLinkInfo.LinkDir == 0) || (pCurInfo->dir == pRouteInfo->EndLinkInfo.LinkDir)) && (nProcess > cntStartLink))
			 ) // || // 방향성이 없고 출발링크 를 지났거나, 시작 아니고, 동일 방향이거나
			)) || 
			(cntMaxExtraSearch < cntExtraSearch)) // 최대 목적지 도착 횟수를 넘어서면
		{
			// 목적지 도달 최대 한도 이전까지만 비교, 최대 한도치 넘으면 이전 결과로 성공
			if (cntGoal <= 1)
			{
				if (pRouteInfo->EndLinkInfo.LinkDir == 1 && pCurInfo->dir == 1) {
					//checkFinishRouteDir[0].dir = 1; // 아래에서 재설정 되는데 무슨의미???
				}
				else if (pRouteInfo->EndLinkInfo.LinkDir == 2 && pCurInfo->dir == 2) {
					//checkFinishRouteDir[0].dir = 2; // 아래에서 재설정 되는데 무슨의미???
				}
				else if ((pRouteInfo->EndLinkInfo.LinkDir == 0) || (pRouteInfo->reqInfo.StartDirIgnore || pRouteInfo->reqInfo.EndDirIgnore)) {
					//checkFinishRouteDir[0].dir = pCurInfo->dir; // 아래에서 재설정 되는데 무슨의미???

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
					// 어떤 경우일까????
					LOG_TRACE(LOG_DEBUG, "================= (cntCoal == 1) ????????????=================");
					// 체크해보자

					continue;
				}

				checkFinishRouteDir[0] = pCurInfo;
				//memcpy(&checkFinishRouteDir[0], pCurInfo, sizeof(checkFinishRouteDir[0]));
				cntGoalTouch++;
			}
			else if ((cntGoal >= 2) && (cntMaxExtraSearch > cntExtraSearch))
			{
				if (pCurInfo->dir != 0)
				{
					checkFinishRouteDir[cntGoalTouch] = pCurInfo;
					//memcpy(&checkFinishRouteDir[cntGoalTouch], pCurInfo, sizeof(checkFinishRouteDir[cntGoalTouch]));
					cntGoalTouch++;
				}
				else
				{
					// 어떤 경우일까????
					LOG_TRACE(LOG_DEBUG, "================= (cntCoal == 2) ????????????=================");
					// 체크해보자
				}
				

				// 종료점 양단의 노드로 경탐될때까지
				//if (cntGoalTouch < cntGoal)
				//{
				//	// 목적지 추가 탐색 가능하도록 현재 방문 정보 삭제
				//	//canKey.parents_id = curInfo.parentId.nid;
				//	//canKey.current_id = curInfo.linkId.nid;
				//	//mRoutePass.erase(canKey);

				//	continue;
				//}
			}
		}
		else {
			/*AddNextLinks(&m_routeInfo, pCurInfo, pCurInfo->linkId.dir);*/


			// 목적 지점이 노드 타입이고, 현재 링크의 다음 노드가 종단 노드면, 현재 노드 정보 확인
			//if ((cntAddedLink <= 0) && (pRouteInfo->EndLinkInfo.KeyType == TYPE_KEY_NODE)) { 
			//	pNode = GetNextNode(pCurInfo);

			//	if (pNode && (pRouteInfo->EndLinkInfo.LinkId.llid == pNode->node_id.llid)) // 노드 ID 가 같으면
			//	{
			//		// 현재 링크의 다음 노드가 목적지면, 현재 링크까지가 경로 결과에 포함된다.
			//		memcpy(&checkFinishRouteDir[cntGoalTouch], pCurInfo, sizeof(checkFinishRouteDir[cntGoalTouch]));
			//		cntGoalTouch++;
			//	}
			//} 
			//else 
		}

		if ((cntGoal <= cntGoalTouch) || (cntMaxExtraSearch < cntExtraSearch)) {
			break;
		} else if (cntGoalTouch > 0) {
			cntExtraSearch++;
		} 
		// 너무 많은 탐색 시도일 경우 실패 처리하자
#if defined(USE_FOREST_DATA)
		else if (pCurInfo->depth > 1000 && (pCurInfo->distTreavel > (MAX_SEARCH_DIST_FOR_COURSE * 2)))
#elif defined(USE_VEHICLE_DATA)
		else if (pCurInfo->depth > 3000 && (pCurInfo->distTreavel > (MAX_SEARCH_DIST_FOR_VEHICLE * 2)))
#else
		else if (pCurInfo->depth > 300 && (pCurInfo->distTreavel > (MAX_SEARCH_DIST_FOR_PEDESTRIAN))) 
			// 우형PM은 빠른 응답이 필요하기에 탐색 시도횟수를 줄이자
#endif
		{
			ret = ROUTE_RESULT_FAILED_EXPEND;
			break;
		}

		cntAddedLink = AddNextLinks(pRouteInfo, pCurInfo);

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


	if (cntGoalTouch) {
		// 경로 탐색 성공
		LOG_TRACE(LOG_TEST, "Success find goal, req_idx:%d, (%d/%d)", idx, cntGoalTouch, cntGoal);

		if (cntGoalTouch >= 2) {
			// 양단 모두 확인 필요
			// 탐색 요금이 작은 결과를 사용하자.
			if (checkFinishRouteDir[0]->costTreavel <= checkFinishRouteDir[1]->costTreavel) {
				pCurInfo = checkFinishRouteDir[0];
			} else {
				pCurInfo = checkFinishRouteDir[1];
			}
		}
		else {
			pCurInfo = checkFinishRouteDir[0];
		}


		//char szMsg[MAX_PATH] = { 0, };
		//LOG_TRACE(LOG_DEBUG, "Reversed Path : ");
		//LOG_TRACE(LOG_DEBUG, " !! idx:%lld(mesh:%d, id:%d), real cost:%f, heuristic cost:%f, lv:%d", curInfo.linkId, pLink->link_id.tile_id, pLink->link_id.nid, curInfo.costReal, curInfo.costHeuristic, curInfo.depth);

		// 종료부터 시작까지 경로 찾기
		//LOG_TRACE(LOG_TEST, "result path tree depth : %d", pCurInfo->depth);

		//pRouteInfo->vtCandidateResult.emplace_back(pCurInfo);
		captureResultPath(pCurInfo, pRouteInfo->vtCandidateResult);

		ret = ROUTE_RESULT_SUCCESS;
	} else {
		ret = ROUTE_RESULT_FAILED_EXPEND_ISOLATED;
	}

	// debug for status
#if defined(_WIN32)
	printf("\n");
#endif

	return ret;
}


const int CRoutePlan::MakeCourse(IN const int idx, IN RouteInfo* pRouteInfo, OUT RouteResultInfo* pRouteResult)
{
	const stLinkInfo* pLink = nullptr;
	const stNodeInfo* pNode = nullptr;

	// 종료 지점의 탐색 매칭 카운트 계산
	uint32_t cntGoal = 0; // 목적지 도달 가능한 링크 수 
	uint32_t cntGoalTouch = 0; // 목적지 도달한 링크 수
	CandidateLink* checkFinishRouteDir[2];// = { 0, }; // 최대 링크 양단에서 한번씩만 허용
										  //checkFinishRouteDir[0].costTreavel = DBL_MAX;
										  //checkFinishRouteDir[1].costTreavel = DBL_MAX;

	int ret = ROUTE_RESULT_FAILED_COURSE;

	if (pRouteInfo == nullptr || pRouteResult == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, req_idx:%d, input param null, pRouteInfo:%p, pRouteResult:%p", idx, pRouteInfo, pRouteResult);
		return ret;
	}


	// 종료 정보 등록
	// 종료 지점이 노드일 경우
	if (pRouteInfo->EndLinkInfo.KeyType == TYPE_KEY_NODE) // node type
	{
		pNode = m_pDataMgr->GetNodeDataById(pRouteInfo->EndLinkInfo.LinkId, pRouteInfo->EndLinkInfo.LinkDataType);

		if (!pNode) {
			LOG_TRACE(LOG_WARNING, "Failed, req_idx:%d, can't find end node, tile:%d, node:%d", idx, pRouteInfo->EndLinkInfo.LinkId.tile_id, pRouteInfo->EndLinkInfo.LinkId.nid);

			ret = ROUTE_RESULT_FAILED_READ_DATA;
			return ret;
		}

		pRouteInfo->EndLinkInfo.MatchCoord = pNode->coord;

		if (pNode->base.node_type == TYPE_NODE_DATA_VEHICLE) { // 차량은 패스코드 확인
			for (int ii = 0; ii < pNode->base.connnode_count; ii++) {
				if (getPrevPassCode(pNode, pNode->connnodes[ii], m_pDataMgr) != PASS_CODE_DISABLE) { // 통행 가능 확인
					cntGoal++;
				}
			} // for 
		}
		else if (pNode->base.connnode_count >= 1) {
			cntGoal++;
		}
	}
	else // 종료 지점이 링크일 경우
	{ // linke type
		pLink = m_pDataMgr->GetLinkDataById(pRouteInfo->EndLinkInfo.LinkId, pRouteInfo->EndLinkInfo.LinkDataType);

		if (!pLink) {
			LOG_TRACE(LOG_WARNING, "Failed, req_idx:%d, can't find end link, tile:%d, link:%d", idx, pRouteInfo->EndLinkInfo.LinkId.tile_id, pRouteInfo->EndLinkInfo.LinkId.nid);

			ret = ROUTE_RESULT_FAILED_READ_DATA;
			return ret;
		}

		// s 노드에서의 진입은 정방향
		if (pRouteInfo->EndLinkInfo.LinkDir == 0 || (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 1)) && (pRouteInfo->EndLinkInfo.LinkDir == 1))) // 양방향 or 정방향
		{
			pNode = m_pDataMgr->GetNodeDataById(pLink->snode_id, pLink->base.link_type);

			// S 노드가 단점 아닌지 확인
			if ((pNode && pNode->base.point_type != TYPE_NODE_END && pNode->base.connnode_count >= 2) ||
				(pNode && pNode->base.point_type == TYPE_NODE_EDGE && pNode->base.connnode_count >= 1)) { // 구획변교점
				cntGoal++;
			}
		}

		// e 노드에서의 진입은 역방향
		if (pRouteInfo->EndLinkInfo.LinkDir == 0 || (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 2)) && (pRouteInfo->EndLinkInfo.LinkDir == 2))) // 양방향 or 역방향
		{
			pNode = m_pDataMgr->GetNodeDataById(pLink->enode_id, pLink->base.link_type);

			// E 노드가 단점 아닌지 확인
			if ((pNode && pNode->base.point_type != TYPE_NODE_END && pNode->base.connnode_count >= 2) ||
				(pNode && pNode->base.point_type == TYPE_NODE_EDGE && pNode->base.connnode_count >= 1)) { // 구획변교점
				cntGoal++;
			}
		}
	}

	// 시작 정보 등록
	// 시작 지점이 노드일 경우
	if (pRouteInfo->StartLinkInfo.KeyType == TYPE_KEY_NODE) // node type
	{
		pNode = m_pDataMgr->GetNodeDataById(pRouteInfo->StartLinkInfo.LinkId, pRouteInfo->StartLinkInfo.LinkDataType);

		if (!pNode) {
			LOG_TRACE(LOG_WARNING, "Failed, req_idx:%d, can't find start node, tile:%d, node:%d", idx, pRouteInfo->EndLinkInfo.LinkId.tile_id, pRouteInfo->EndLinkInfo.LinkId.nid);

			pRouteResult->ResultCode = ROUTE_RESULT_FAILED_READ_DATA;
			return pRouteResult->ResultCode;
		}

		pRouteInfo->EndLinkInfo.MatchCoord = pNode->coord;

		for (int ii = 0; ii < pNode->base.connnode_count; ii++) {
			if ((pNode->base.node_type != TYPE_NODE_DATA_VEHICLE) ||
				((pNode->base.node_type == TYPE_NODE_DATA_VEHICLE) && // 차량은 패스코드 확인
					(getPrevPassCode(pNode, pNode->connnodes[ii], m_pDataMgr) != PASS_CODE_DISABLE))) // 통행 가능 확인 
			{
				pLink = m_pDataMgr->GetLinkDataById(pNode->connnodes[ii], pNode->base.node_type);

				if (!pLink) {
					LOG_TRACE(LOG_WARNING, "Failed, req_idx:%d, can't find start link, tile:%d, link:%d", idx, pRouteInfo->StartLinkInfo.LinkId.tile_id, pRouteInfo->StartLinkInfo.LinkId.nid);
					pRouteResult->ResultCode = ROUTE_RESULT_FAILED_READ_DATA;
					return pRouteResult->ResultCode;
				}

				KeyID candidateId;
				KeyID targetNodeId; // 진행 방향 노드 ID
				int dir = 0; // 0:양방향, 1:정방향, 2:역방향

				if (pNode->node_id == pLink->snode_id) {
					dir = 1; // 정방향
					targetNodeId = pLink->enode_id;  // 진행 방향 노드 ID					
				}
				else if (pNode->node_id == pLink->enode_id) {
					dir = 2; // 역방향
					targetNodeId = pLink->snode_id;  // 진행 방향 노드 ID
				}
				else {
					LOG_TRACE(LOG_WARNING, "Failed, can't find start node matchin link, node tile:%d, id:%d", pNode->node_id.tile_id, pNode->node_id.nid);
					pRouteResult->ResultCode = ROUTE_RESULT_FAILED_READ_DATA;
					return pRouteResult->ResultCode;
				}

				candidateId.parents_id = dir; // 방향성, 최초는 부모가 없으니까 진행 방향을 값으로 주자
				candidateId.current_id = pLink->link_id.nid;

				CandidateLink* pItem = new CandidateLink;
				pItem->candidateId = candidateId; // 후보 ID
				pItem->parentLinkId.llid = candidateId.parents_id; // 부모 링크 ID
				pItem->linkId = pLink->link_id; // 링크 ID
				pItem->nodeId = targetNodeId; // 노드 ID
				pItem->dist = pLink->length; // 실 거리
				pItem->time = GetCost(&pRouteInfo->reqInfo, pLink, dir, pLink->length, SPEED_NOT_AVALABLE); // 실 주행 시간
				pItem->cost = pItem->time; // 논리적 주행 시간
				pItem->distTreavel = pItem->dist; // 시작 거리
				pItem->timeTreavel = pItem->time; // 시작 시간
				pItem->costTreavel = pItem->cost; // 시작 비용
				pItem->costHeuristic = 0; // pLink->length * VAL_HEURISTIC_VEHICLE_FACTOR;	// 가중치 계산 비용
				pItem->linkDataType = pRouteInfo->StartLinkInfo.LinkDataType;
				pItem->depth = 0;	// 탐색 깊이
				pItem->visited = false; // 방문 여부
				pItem->dir = dir; // 탐색방향
				pItem->angle = 0; // 진입각도

				pItem->pPrevLink = nullptr;

				// 링크 방문 목록 등록
				pRouteInfo->mRoutePass.emplace(candidateId.llid, pItem);
				pRouteInfo->pqDijkstra.emplace(pItem);
			}
		} // for 
	}
	else // 시작 지점이 링크일 경우
	{ // linke type
		pLink = m_pDataMgr->GetLinkDataById(pRouteInfo->StartLinkInfo.LinkId, pRouteInfo->StartLinkInfo.LinkDataType);

		if (!pLink) {
			LOG_TRACE(LOG_WARNING, "Failed, req_idx:%d, can't find start link, tile:%d, link:%d", idx, pRouteInfo->StartLinkInfo.LinkId.tile_id, pRouteInfo->StartLinkInfo.LinkId.nid);

			ret = ROUTE_RESULT_FAILED_READ_DATA;
			return ret;
		}

#if 1 // 함수로 대체
		vector<CandidateLink*> vtCandidateInfo;
		CheckStartDirectionMaching(&pRouteInfo->reqInfo, pLink, &pRouteInfo->StartLinkInfo, vtCandidateInfo);

		// 링크 방문 목록 등록
		for (const auto& item : vtCandidateInfo) {
			pRouteInfo->mRoutePass.emplace(item->candidateId.llid, item);
			pRouteInfo->pqDijkstra.emplace(item);
		}
#else
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
			pItem->parentLinkId.llid = candidateId.parents_id; // 부모 링크 ID
			pItem->linkId = pLink->link_id; // 링크 ID
			pItem->nodeId = pLink->snode_id; // 진행 방향 노드 ID
			pItem->dist = pRouteInfo->StartLinkInfo.LinkDistToS; // 실 거리
			pItem->time = GetCost(pLink, candidateId.parents_id, pRouteInfo->RouteOption, pRouteInfo->StartLinkInfo.LinkDistToS, SPEED_NOT_AVALABLE); // 실 주행 시간
			pItem->cost = pItem->time; // 논리적 주행 시간
			pItem->distTreavel = pItem->dist; // 시작 거리
			pItem->timeTreavel = pItem->time; // 시작 시간
			pItem->costTreavel = pItem->cost;	// 계산 비용
			pItem->costHeuristic = 0; // pRouteInfo->StartLinkInfo.LinkDistToS * VAL_HEURISTIC_VEHICLE_FACTOR;	// 가중치 계산 비용, 첫 링크는 가중치를 최하(10%)로 설정 무조건 처음부터 확인하도록(양방향일때)
			pItem->dataType = pRouteInfo->StartLinkInfo.DataType;
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
			pItem->parentLinkId.llid = candidateId.parents_id; // 부모 링크 ID
			pItem->linkId = pLink->link_id; // 링크 ID
			pItem->nodeId = pLink->enode_id; // 진행 방향 노드 ID
			pItem->dist = pRouteInfo->StartLinkInfo.LinkDistToE;	// 실 거리
			pItem->time = GetCost(pLink, candidateId.parents_id, pRouteInfo->RouteOption, pRouteInfo->StartLinkInfo.LinkDistToE, SPEED_NOT_AVALABLE); // 실 주행 시간
			pItem->cost = pItem->time; // 논리적 주행 시간
			pItem->distTreavel = pItem->dist; // 시작 거리
			pItem->timeTreavel = pItem->time; // 시작 시간
			pItem->costTreavel = pItem->cost; // 계산 비용
			pItem->costHeuristic = 0; // pRouteInfo->StartLinkInfo.LinkDistToE * VAL_HEURISTIC_VEHICLE_FACTOR; // 가중치 계산 비용, 첫 링크는 가중치를 최하로 설정 무조건 처음부터 확인하도록(양방향일때)
			pItem->dataType = pRouteInfo->StartLinkInfo.DataType;
			pItem->depth = 0;	// 탐색 깊이
			pItem->visited = false; // 방문 여부
			pItem->dir = candidateId.parents_id; // 탐색방향

			pItem->pPrevLink = const_cast<CandidateLink*>(pCurInfo);

			// 링크 방문 목록 등록
			pRouteInfo->mRoutePass.emplace(candidateId.llid, pItem);
			pRouteInfo->pqDijkstra.emplace(pItem);
		}
#endif 
	}
	// LOG_TRACE(LOG_DEBUG, "Start tree : link:%d", pLink->link_id.nid);


	CandidateLink* pCurInfo;
	static const uint32_t cntMaxExtraSearch = 100; // 목적지 추가 도달 검사 최대 허용치
	uint32_t cntExtraSearch = 0; // 목적지 도달 후 추가 도달 검사 카운트

	// 경로 탐색
	LOG_TRACE(LOG_TEST, "Start finding goal, req_idx:%d", idx);

	int cntAddedLink = 0;
	int nProcess = 0;
	// debug for status
#if defined(_DEBUG)
	int nProcessGap = 1000;
#else
	int nProcessGap = 3000;
#endif

	int32_t cntStartLink = pRouteInfo->pqDijkstra.size(); // 출발지 등록 링크 수(링크일 경우 양끝으로 2개, 노드일 경우, 연결링크 수만큼)


	ret = ROUTE_RESULT_FAILED_COURSE_ID;

	// 코스 데이터 링크 확보
	uint32_t courseID = pRouteInfo->reqInfo.RouteSubOption.mnt.course_id;
	if (courseID <= 0) {
		return ret;
	}

	set<uint64_t>* psetCourse = m_pDataMgr->GetLinkByCourse(courseID);
	if (psetCourse == nullptr || psetCourse->empty()) {
		return ret;
	}

	while (!pRouteInfo->pqDijkstra.empty()) {
		nProcess++;

#if defined(_WIN32)
		routeProcessPrint(nProcess);
#endif // #if defined(_WIN32)

		// 현재 링크 트리에서 제거
		pCurInfo = pRouteInfo->pqDijkstra.top(); pRouteInfo->pqDijkstra.pop();
		pCurInfo->visited = true;



#if defined(USE_SHOW_ROUTE_SATATUS)
		//LOG_TRACE(LOG_DEBUG, "Current Link : id:%lld(real cost:%f, heuristic cost:%f, lv:%d)", curInfo.linkId, curInfo.costReal, curInfo.costHeuristic, curInfo.depth);

		// 현재까지 탐색된 정보 전달, Function Call
		if (m_fpRoutingStatus) {
			m_fpRoutingStatus(m_pHost, &pRouteInfo->mRoutePass);
		}
#endif

		if (pRouteInfo->EndLinkInfo.KeyType == TYPE_KEY_NODE)
			//if (((pRouteInfo->EndLinkInfo.KeyType == TYPE_KEY_NODE) && // 목적 지점이 노드 타입이고, 
			//	((pRouteInfo->EndLinkInfo.LinkId.llid == pCurInfo->nodeId.llid))) || // 노드 ID 가 같거나
			//	(cntMaxExtraSearch < cntExtraSearch)) // 최대 목적지 도착 횟수를 넘어서면
		{
			//if (pRouteInfo->EndLinkInfo.LinkId.llid == pCurInfo->nodeId.llid) {// 노드 ID 가 같으면
			//	// 링크 기준이기에, 순서상 이미 추가된 링크를 기준으로 목적지 노드 다음을 가리키므로, 이전의 링크까지가 결과 링크에 포함된다.
			//	if (pCurInfo->pPrevLink != nullptr) {
			//		memcpy(&checkFinishRouteDir[cntGoalTouch], pCurInfo->pPrevLink, sizeof(checkFinishRouteDir[cntGoalTouch]));
			//	} else {
			//		// -----------??? 이건 출발링크가 목적 노드와 동일한 경우인건가?????
			//		memcpy(&checkFinishRouteDir[cntGoalTouch], pCurInfo, sizeof(checkFinishRouteDir[cntGoalTouch]));
			//	}
			//	cntGoalTouch++;
			//}



			pNode = GetNextNode(pCurInfo);

			if (pNode && (pRouteInfo->EndLinkInfo.LinkId.llid == pNode->node_id.llid)) // 노드 ID 가 같으면
			{
				// 현재 링크의 다음 노드가 목적지면, 현재 링크까지가 경로 결과에 포함된다.
				//memcpy(&checkFinishRouteDir[cntGoalTouch], pCurInfo, sizeof(checkFinishRouteDir[cntGoalTouch]));
				checkFinishRouteDir[cntGoalTouch] = pCurInfo;
				cntGoalTouch++;
			}
		}
		else if (((pCurInfo->linkId.llid == pRouteInfo->EndLinkInfo.LinkId.llid) && // 목적지 링크 이면서
			((pRouteInfo->reqInfo.StartDirIgnore || pRouteInfo->reqInfo.EndDirIgnore) || // 방향성 무시거나
				((((pRouteInfo->EndLinkInfo.LinkId == pRouteInfo->StartLinkInfo.LinkId) && (nProcess <= cntStartLink) && // 출도착 동일 + 링크 시작이면서, 
					(((((pRouteInfo->EndLinkInfo.LinkDir == 0) || (pRouteInfo->EndLinkInfo.LinkDir == 1)) && pCurInfo->dir == 1) && // 진행방향(출발->도착) 일때
						(pRouteInfo->StartLinkInfo.LinkDistToS <= pRouteInfo->EndLinkInfo.LinkDistToS)) || // E -> S
						((((pRouteInfo->EndLinkInfo.LinkDir == 0) || (pRouteInfo->EndLinkInfo.LinkDir == 2)) && pCurInfo->dir == 2) && // 진행방향(출발->도착) 일때
							(pRouteInfo->StartLinkInfo.LinkDistToE <= pRouteInfo->EndLinkInfo.LinkDistToE))) // S -> E
					))) ||
				((pRouteInfo->StartLinkInfo.KeyType == TYPE_KEY_NODE) && (pRouteInfo->EndLinkInfo.LinkDir == 0) && (nProcess <= cntStartLink)
					) || // 시작지점이 노드고, 출발링크들과의 매칭일때
				((((pRouteInfo->EndLinkInfo.LinkDir == 0) || (pCurInfo->dir == pRouteInfo->EndLinkInfo.LinkDir)) && (nProcess > cntStartLink))
					) // || // 방향성이 없고 출발링크 를 지났거나, 시작 아니고, 동일 방향이거나
				)) ||
			(cntMaxExtraSearch < cntExtraSearch)) // 최대 목적지 도착 횟수를 넘어서면
		{
			// 목적지 도달 최대 한도 이전까지만 비교, 최대 한도치 넘으면 이전 결과로 성공
			if (cntGoal <= 1) {
				if (pRouteInfo->EndLinkInfo.LinkDir == 1 && pCurInfo->dir == 1) {
					//checkFinishRouteDir[0].dir = 1; // 아래에서 재설정 되는데 무슨의미???
				}
				else if (pRouteInfo->EndLinkInfo.LinkDir == 2 && pCurInfo->dir == 2) {
					//checkFinishRouteDir[0].dir = 2; // 아래에서 재설정 되는데 무슨의미???
				}
				else if ((pRouteInfo->EndLinkInfo.LinkDir == 0) || (pRouteInfo->reqInfo.StartDirIgnore || pRouteInfo->reqInfo.EndDirIgnore)) {
					//checkFinishRouteDir[0].dir = pCurInfo->dir; // 아래에서 재설정 되는데 무슨의미???

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
					// 어떤 경우일까????
					LOG_TRACE(LOG_DEBUG, "================= (cntCoal == 1) ????????????=================");
					// 체크해보자

					continue;
				}

				checkFinishRouteDir[0] = pCurInfo;
				//memcpy(&checkFinishRouteDir[0], pCurInfo, sizeof(checkFinishRouteDir[0]));
				cntGoalTouch++;
			}
			else if ((cntGoal >= 2) && (cntMaxExtraSearch > cntExtraSearch)) {
				if (pCurInfo->dir != 0) {
					checkFinishRouteDir[cntGoalTouch] = pCurInfo;
					//memcpy(&checkFinishRouteDir[cntGoalTouch], pCurInfo, sizeof(checkFinishRouteDir[cntGoalTouch]));
					cntGoalTouch++;
				}
				else {
					// 어떤 경우일까????
					LOG_TRACE(LOG_DEBUG, "================= (cntCoal == 2) ????????????=================");
					// 체크해보자
				}


				// 종료점 양단의 노드로 경탐될때까지
				//if (cntGoalTouch < cntGoal)
				//{
				//	// 목적지 추가 탐색 가능하도록 현재 방문 정보 삭제
				//	//canKey.parents_id = curInfo.parentId.nid;
				//	//canKey.current_id = curInfo.linkId.nid;
				//	//mRoutePass.erase(canKey);

				//	continue;
				//}
			}
		}
		else {
			/*AddNextLinks(&m_routeInfo, pCurInfo, pCurInfo->linkId.dir);*/


			// 목적 지점이 노드 타입이고, 현재 링크의 다음 노드가 종단 노드면, 현재 노드 정보 확인
			//if ((cntAddedLink <= 0) && (pRouteInfo->EndLinkInfo.KeyType == TYPE_KEY_NODE)) { 
			//	pNode = GetNextNode(pCurInfo);

			//	if (pNode && (pRouteInfo->EndLinkInfo.LinkId.llid == pNode->node_id.llid)) // 노드 ID 가 같으면
			//	{
			//		// 현재 링크의 다음 노드가 목적지면, 현재 링크까지가 경로 결과에 포함된다.
			//		memcpy(&checkFinishRouteDir[cntGoalTouch], pCurInfo, sizeof(checkFinishRouteDir[cntGoalTouch]));
			//		cntGoalTouch++;
			//	}
			//} 
			//else 
		}

		if ((cntGoal <= cntGoalTouch) || (cntMaxExtraSearch < cntExtraSearch)) {
			break;
		}
		else if (cntGoalTouch > 0) {
			cntExtraSearch++;
		}

		cntAddedLink = AddNextCourse(pRouteInfo, pCurInfo, psetCourse);

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


	if (cntGoalTouch) {
		// 경로 탐색 성공
		LOG_TRACE(LOG_TEST, "Success find goal, req_idx:%d, (%d/%d)", idx, cntGoalTouch, cntGoal);

		if (cntGoalTouch >= 2) {
			// 양단 모두 확인 필요
			// 탐색 요금이 작은 결과를 사용하자.
			if (checkFinishRouteDir[0]->costTreavel <= checkFinishRouteDir[1]->costTreavel) {
				pCurInfo = checkFinishRouteDir[0];
			}
			else {
				pCurInfo = checkFinishRouteDir[1];
			}
		}
		else {
			pCurInfo = checkFinishRouteDir[0];
		}


		//char szMsg[MAX_PATH] = { 0, };
		//LOG_TRACE(LOG_DEBUG, "Reversed Path : ");
		//LOG_TRACE(LOG_DEBUG, " !! idx:%lld(mesh:%d, id:%d), real cost:%f, heuristic cost:%f, lv:%d", curInfo.linkId, pLink->link_id.tile_id, pLink->link_id.nid, curInfo.costReal, curInfo.costHeuristic, curInfo.depth);

		// 종료부터 시작까지 경로 찾기
		//LOG_TRACE(LOG_TEST, "result path tree depth : %d", pCurInfo->depth);

		//pRouteInfo->vtCandidateResult.emplace_back(pCurInfo);
		captureResultPath(pCurInfo, pRouteInfo->vtCandidateResult);

		ret = ROUTE_RESULT_SUCCESS;
	} else {
		ret = ROUTE_RESULT_FAILED_EXPEND;
	}

	// debug for status
#if defined(_WIN32)
	printf("\n");
#endif

	return ret;
}


// 출발지 링크의 매칭 정보 : 링크에서 노드로 나가는 매칭
const uint32_t CRoutePlan::CheckStartDirectionMaching(IN const RequestRouteInfo* pReqInfo, IN const stLinkInfo* pLink, IN const RouteLinkInfo* pRoutLinkInfo, OUT vector<CandidateLink*>& vtCandidateInfo)
{
	KeyID candidateId;
	if (pRoutLinkInfo->LinkDir == 0 // 양방향
		|| (pLink->link_id.dir == 2 && pRoutLinkInfo->LinkDir == 2)
#if defined(USE_REAL_ROUTE_TSP)
		|| (pLink->link_id.dir == 0 && pRoutLinkInfo->LinkDir == 1 && pLink->veh.level >= 8) // 2차선 이하 에서만 반대방향 주행 허용
#endif
		|| (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 2)) && (pRoutLinkInfo->LinkDir == 2)) // 양방향 or 역방향
		) {
		//candidateId.parents_id = 2;// + s노드 역방향 정보 이력 - 최초는 부모가 없으니까 진행 방향을 값으로 주자
		candidateId.parents_id = 2; // 시작 노드의 방향
		candidateId.current_id = pLink->link_id.nid;

		//KeyID curLink = candidateId;
		//curLink.dir = 2;// + s노드 역방향 정보 이력

		uint8_t curSpeed = SPEED_NOT_AVALABLE;
		uint8_t curSpeedType = TYPE_TRAFFIC_NONE;
#if defined(USE_VEHICLE_DATA)
		if (pLink->veh.level <= 6) {
			curSpeed = m_pDataMgr->GetTrafficSpeed(pLink->link_id, candidateId.parents_id, pReqInfo->RequestTime, curSpeedType);
		}
#endif

		CandidateLink* pItem = new CandidateLink;
		pItem->candidateId = candidateId; // 후보 ID
		pItem->parentLinkId.llid = candidateId.parents_id; // 부모 링크 ID
		pItem->linkId = pLink->link_id; // 링크 ID
		pItem->nodeId = pLink->snode_id; // 노드 ID
		pItem->dist = pRoutLinkInfo->LinkDistToS; // 실 거리
		pItem->time = GetCost(pReqInfo, pLink, candidateId.parents_id, pRoutLinkInfo->LinkDistToS, curSpeed); // 실 주행 시간
		pItem->cost = pItem->time;
		pItem->distTreavel = pItem->dist; // 시작 거리
		pItem->timeTreavel = pItem->time; // 시작 시간
		pItem->costTreavel = pItem->cost; // 시작 비용
		pItem->costHeuristic = 0; // pRoutLinkInfo->LinkDistToS * VAL_HEURISTIC_VEHICLE_FACTOR;	// 가중치 계산 비용
		pItem->linkDataType = pRoutLinkInfo->LinkDataType;
		pItem->depth = 0;	// 탐색 깊이
		pItem->visited = false; // 방문 여부
		pItem->dir = candidateId.parents_id; // 탐색방향
		pItem->angle = 0; // 진입각도
#if defined(USE_VEHICLE_DATA)
		pItem->speed = curSpeed;
		pItem->speed_type = curSpeedType;
		pItem->speed_level = pLink->veh.level;
#endif

		pItem->pPrevLink = nullptr;

		// 링크 방문 목록 등록
		vtCandidateInfo.emplace_back(pItem);
	}


	if (pRoutLinkInfo->LinkDir == 0 // 양방향 
		//|| (pLink->link_id.dir == 1 && pRoutLinkInfo->LinkDir == 1)
		//|| (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 1)) && (pRoutLinkInfo->LinkDir == 1)) // 양방향 or 정방향
		//)
		|| (pLink->link_id.dir == 1 && pRoutLinkInfo->LinkDir == 1)
#if defined(USE_REAL_ROUTE_TSP)
		|| (pLink->link_id.dir == 0 && pRoutLinkInfo->LinkDir == 2 && pLink->veh.level >= 8) // 2차선 이하 에서만 반대방향 주행 허용
#endif
		|| (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 1)) && (pRoutLinkInfo->LinkDir == 1)) // 양방향 or 역방향
		) {
		//candidateId.parents_id = 1;// + e노드 정방향 정보 이력 - 최초는 부모가 없으니까 진행 방향을 값으로 주자
		candidateId.parents_id = 1; // 시작 노드의 방향
		candidateId.current_id = pLink->link_id.nid;

		//KeyID curLink = candidateId;
		//curLink.dir = 1;// + e노드 정방향 정보 이력

		uint8_t curSpeed = SPEED_NOT_AVALABLE;
		uint8_t curSpeedType = TYPE_TRAFFIC_NONE;
#if defined(USE_VEHICLE_DATA)
		if (pLink->veh.level <= 6) {
			curSpeed = m_pDataMgr->GetTrafficSpeed(pLink->link_id, candidateId.parents_id, pReqInfo->RequestTime, curSpeedType);
		}
#endif

		CandidateLink* pItem = new CandidateLink;
		pItem->candidateId = candidateId; // 후보 ID
		pItem->parentLinkId.llid = candidateId.parents_id; // 부모 링크 ID
		pItem->linkId = pLink->link_id; // 링크 ID
		pItem->nodeId = pLink->enode_id; // 노드 ID
		pItem->dist = pRoutLinkInfo->LinkDistToE; // 실 거리
		pItem->time = GetCost(pReqInfo, pLink, candidateId.parents_id, pRoutLinkInfo->LinkDistToE, curSpeed); // 실 주행 시간
		pItem->cost = pItem->time;
		pItem->distTreavel = pItem->dist; // 시작 거리
		pItem->timeTreavel = pItem->time; // 시작 시간
		pItem->costTreavel = pItem->cost; // 시작 비용
		pItem->costHeuristic = 0; // pRoutLinkInfo->LinkDistToE * VAL_HEURISTIC_VEHICLE_FACTOR;	// 가중치 계산 비용
		pItem->linkDataType = pRoutLinkInfo->LinkDataType;
		pItem->depth = 0;	// 탐색 깊이
		pItem->visited = false; // 방문 여부
		pItem->dir = candidateId.parents_id; // 탐색방향
		pItem->angle = 0; // 진입각도
#if defined(USE_VEHICLE_DATA)
		pItem->speed = curSpeed;
		pItem->speed_type = curSpeedType;
		pItem->speed_level = pLink->veh.level;
#endif

		pItem->pPrevLink = nullptr;

		// 링크 방문 목록 등록
		vtCandidateInfo.emplace_back(pItem);
	}

	return ROUTE_RESULT_SUCCESS;
}


// 도착지 링크의 매칭 정보 : 노드에서 링크로 들어오는 매칭
const uint32_t CRoutePlan::CheckEndDirectionMaching(IN const RequestRouteInfo* pReqInfo, IN const stLinkInfo* pLink, IN const RouteLinkInfo* pRoutLinkInfo, OUT vector<CandidateLink*>& vtCandidateInfo)
{
	// s 노드에서의 진입은 정방향
	if (pRoutLinkInfo->LinkDir == 0 // 양방향
#if defined(USE_REAL_ROUTE_TSP)
		|| (pLink->link_id.dir == 0 && pRoutLinkInfo->LinkDir == 2 && pLink->veh.level >= 8) // 2차선 이하 에서만 반대방향 주행 허용
#endif
		|| (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 1)) && (pRoutLinkInfo->LinkDir == 1)) // 정방향
		)
	{
		// s 노드에서의 진입은 정방향
		const stNodeInfo* pSNode = m_pDataMgr->GetNodeDataById(pLink->snode_id, pLink->base.link_type);
		bool isIsolated = true;

		// 구획변교점일 경우 연결 노드 구하자
		if (pSNode && pSNode->base.point_type == TYPE_NODE_EDGE) {
			KeyID oldKey = pSNode->edgenode_id;
			int oldType = pSNode->base.node_type;
			pSNode = nullptr;

			stNodeInfo* pEdgeNode = m_pDataMgr->GetNodeDataById(oldKey, oldType);
			stLinkInfo* pConnLink = nullptr;
			if (pEdgeNode && pEdgeNode->base.connnode_count >= 1) {
				pConnLink = m_pDataMgr->GetLinkDataById(pEdgeNode->connnodes[0], oldType);
				if (pConnLink) {
					// 반대변 노드 정보 가져오기
					if (pConnLink->snode_id == pEdgeNode->node_id) {
						pSNode = m_pDataMgr->GetNodeDataById(pConnLink->enode_id, oldType);
					} else {
						pSNode = m_pDataMgr->GetNodeDataById(pConnLink->snode_id, oldType);
					}
				}
			}

			if (pSNode && pConnLink && (pSNode->base.point_type != TYPE_NODE_END && pSNode->base.connnode_count >= 2)) {
				// 노드에서 진입 가능한 모든 경우 확인
				for (int ii = 0; ii < pSNode->base.connnode_count; ii++) {
					if (pSNode->base.point_type != TYPE_NODE_EDGE) {
						// 자신은 제외
						if (pConnLink->link_id == pSNode->connnodes[ii]) {
							continue;
						}

						if (getPrevPassCode(pConnLink->link_id, pSNode->connnodes[ii], pSNode) == PASS_CODE_DISABLE) {
							continue;
						}

						isIsolated = false;
						break;
					}
				}
			}
		}
		// S 노드가 단점 아닌지 확인
		else if (pSNode && (pSNode->base.point_type != TYPE_NODE_END && pSNode->base.connnode_count >= 2)) {
			// 노드에서 진입 가능한 모든 경우 확인
			for (int ii = 0; ii < pSNode->base.connnode_count; ii++) {
				// 자신은 제외
				if (pLink->link_id == pSNode->connnodes[ii]) {
					continue;
				}

				if (getPrevPassCode(pLink->link_id, pSNode->connnodes[ii], pSNode) == PASS_CODE_DISABLE) {
					continue;
				}

				isIsolated = false;
				break;
			}			
		}

		if (isIsolated) {
			LOG_TRACE(LOG_WARNING, "destination link matching failed, S node isolated, tile:%d, link_nid:%d, node_nid:%d", pLink->link_id.tile_id, pLink->link_id.nid, pSNode->node_id.nid);
		} else {
			KeyID candidateId;
			candidateId.parents_id = 1;// pSNode->connnodes[ii].dir; // 시작 링크의 방향
			candidateId.current_id = pLink->link_id.nid;

			uint8_t curSpeed = SPEED_NOT_AVALABLE;
			uint8_t curSpeedType = TYPE_TRAFFIC_NONE;
#if defined(USE_VEHICLE_DATA)
			if (pLink->veh.level <= 6) {
				curSpeed = m_pDataMgr->GetTrafficSpeed(pLink->link_id, candidateId.parents_id, pReqInfo->RequestTime, curSpeedType);
			}
#endif

			CandidateLink* pItem = new CandidateLink;
			pItem->candidateId = candidateId; // 후보 ID
			pItem->parentLinkId.llid = candidateId.parents_id; // 부모 링크 ID
			pItem->linkId = pLink->link_id; // 링크 ID
			pItem->nodeId = pLink->snode_id; // 노드 ID
			pItem->dist = pRoutLinkInfo->LinkDistToS; // 실 거리
			pItem->time = GetCost(pReqInfo, pLink, pRoutLinkInfo->LinkDir, pRoutLinkInfo->LinkDistToS, curSpeed); // 실 주행 시간
			pItem->cost = pItem->time;
			pItem->distTreavel = pItem->dist; // 시작 거리
			pItem->timeTreavel = pItem->time; // 시작 시간
			pItem->costTreavel = pItem->cost; // 시작 비용
			pItem->costHeuristic = pRoutLinkInfo->LinkDistToS * VAL_HEURISTIC_VEHICLE_FACTOR;	// 가중치 계산 비용
			pItem->linkDataType = pRoutLinkInfo->LinkDataType;
			pItem->depth = 0;	// 탐색 깊이
			pItem->visited = false; // 방문 여부
			pItem->dir = candidateId.parents_id; // 탐색방향
			//pItem->dir = pRoutLinkInfo->LinkDir;
			pItem->angle = 0; // 진입각도
#if defined(USE_VEHICLE_DATA)
			pItem->speed = curSpeed;
			pItem->speed_type = curSpeedType;
			pItem->speed_level = pLink->veh.level;
#endif

			pItem->pPrevLink = nullptr;

			// 링크 방문 목록 등록
			vtCandidateInfo.emplace_back(pItem);
		}
	}

	// e 노드에서의 진입은 역방향
	if (pRoutLinkInfo->LinkDir == 0 // 양방향
#if defined(USE_REAL_ROUTE_TSP)
		|| (pLink->link_id.dir == 0 && pRoutLinkInfo->LinkDir == 1 && pLink->veh.level >= 8) // 2차선 이하 에서만 반대방향 주행 허용
#endif
		|| (((pLink->link_id.dir == 0) || (pLink->link_id.dir == 2)) && (pRoutLinkInfo->LinkDir == 2)) // 역방향
		)
	{
		// e 노드에서의 진입은 역방향
		const stNodeInfo* pENode = m_pDataMgr->GetNodeDataById(pLink->enode_id, pLink->base.link_type);
		bool isIsolated = true;

		// 구획변교점일경우 연결 노드 구하자
		if (pENode && pENode->base.point_type == TYPE_NODE_EDGE) {
			KeyID oldKey = pENode->edgenode_id;
			int oldType = pENode->base.node_type;
			pENode = nullptr;
			stLinkInfo* pConnLink = const_cast<stLinkInfo*>(pLink);
			stNodeInfo* pEdgeNode = m_pDataMgr->GetNodeDataById(oldKey, oldType);
			if (pEdgeNode && pEdgeNode->base.connnode_count >= 1) {
				pConnLink = m_pDataMgr->GetLinkDataById(pEdgeNode->connnodes[0], oldType);
				if (pConnLink) {
					// 반대변 노드 정보 가져오기
					if (pConnLink->snode_id == pEdgeNode->node_id) {
						pENode = m_pDataMgr->GetNodeDataById(pConnLink->enode_id, oldType);
					} else {
						pENode = m_pDataMgr->GetNodeDataById(pConnLink->snode_id, oldType);
					}
				}
			}

			if (pENode && (pENode->base.point_type != TYPE_NODE_END && pENode->base.connnode_count >= 2)) {
				// 노드에서 진입 가능한 모든 경우 확인
				for (int ii = 0; ii < pENode->base.connnode_count; ii++) {
					if (pENode->base.point_type != TYPE_NODE_EDGE) {
						// 자신은 제외
						if (pConnLink->link_id == pENode->connnodes[ii]) {
							continue;
						}

						if (getPrevPassCode(pConnLink->link_id, pENode->connnodes[ii], pENode) == PASS_CODE_DISABLE) {
							continue;
						}

						isIsolated = false;
						break;
					}
				}
			}
		} 
		// E 노드가 단점 아닌지 확인
		else if (pENode && (pENode->base.point_type != TYPE_NODE_END && pENode->base.connnode_count >= 2)) {
			// 노드에서 진입 가능한 모든 경우 확인			
			for (int ii = 0; ii < pENode->base.connnode_count; ii++) {
				// 자신은 제외
				if (pLink->link_id == pENode->connnodes[ii]) {
					continue;
				}

				if (getPrevPassCode(pLink->link_id, pENode->connnodes[ii], pENode) == PASS_CODE_DISABLE) {
					continue;
				}

				isIsolated = false;
				break;
			}
		}

		if (isIsolated) {
			LOG_TRACE(LOG_WARNING, "destination link matching failed, S node isolated, tile:%d, link_nid:%d, node_nid:%d", pLink->link_id.tile_id, pLink->link_id.nid, pENode->node_id.nid);
		} else {
			KeyID candidateId;
			candidateId.parents_id = 2;// pENode->connnodes[ii].dir; // 시작 링크의 방향
			candidateId.current_id = pLink->link_id.nid;

			uint8_t curSpeed = SPEED_NOT_AVALABLE;
			uint8_t curSpeedType = TYPE_TRAFFIC_NONE;
#if defined(USE_VEHICLE_DATA)
			if (pLink->veh.level <= 6) {
				curSpeed = m_pDataMgr->GetTrafficSpeed(pLink->link_id, candidateId.parents_id, pReqInfo->RequestTime, curSpeedType);
			}
#endif

			CandidateLink* pItem = new CandidateLink;
			pItem->candidateId = candidateId; // 후보 ID
			pItem->parentLinkId.llid = candidateId.parents_id; // 부모 링크 ID
			pItem->linkId = pLink->link_id; // 링크 ID
			pItem->nodeId = pLink->enode_id; // 노드 ID
			pItem->dist = pRoutLinkInfo->LinkDistToS; // 실 거리
			pItem->time = GetCost(pReqInfo, pLink, pRoutLinkInfo->LinkDir, pRoutLinkInfo->LinkDistToS, curSpeed); // 실 주행 시간
			pItem->cost = pItem->time;
			pItem->distTreavel = pItem->dist; // 시작 거리
			pItem->timeTreavel = pItem->time; // 시작 시간
			pItem->costTreavel = pItem->cost; // 시작 비용
			pItem->costHeuristic = pRoutLinkInfo->LinkDistToS * VAL_HEURISTIC_VEHICLE_FACTOR;	// 가중치 계산 비용
			pItem->linkDataType = pRoutLinkInfo->LinkDataType;
			pItem->depth = 0;	// 탐색 깊이
			pItem->visited = false; // 방문 여부
			pItem->dir = candidateId.parents_id; // 탐색방향
			//pItem->dir = pRoutLinkInfo->LinkDir;
			pItem->angle = 0; // 진입각도
#if defined(USE_VEHICLE_DATA)
			pItem->speed = curSpeed;
			pItem->speed_type = curSpeedType;
			pItem->speed_level = pLink->veh.level;
#endif

			pItem->pPrevLink = nullptr;

			// 링크 방문 목록 등록
			vtCandidateInfo.emplace_back(pItem);
		}
	}

	return ROUTE_RESULT_SUCCESS;
}


const int captureTablePath(IN const CandidateLink* pCurInfo, OUT vector<stRoutePath>& vtPath)
{
	CandidateLink* pLinkNow = const_cast<CandidateLink*>(pCurInfo);
	stRoutePath path;
	const int maxLinkDepth = pLinkNow->depth + 1;
	int visitStartLinkCnt = 0;
	bool isSuccess = false;

	vtPath.clear();
	vtPath.reserve(maxLinkDepth);

	for (; !isSuccess;) {
		path.linkId = pLinkNow->linkId.llid;
		path.dir = pLinkNow->dir;
#if defined(USE_VEHICLE_DATA)
		path.speed = pLinkNow->speed;
		path.speed_level = pLinkNow->speed_level;
		path.speed_type = pLinkNow->speed_type;
#endif
		vtPath.emplace_back(path);

		if ((pLinkNow->pPrevLink == nullptr) || /*(pLinkNow->parentLinkId.parents_id <= 2) ||*/ pLinkNow->depth <= 0) {
			isSuccess = true;
			//break;
		} // 시작 지점 확인
		else {
			visitStartLinkCnt++;
			pLinkNow = pLinkNow->pPrevLink;
		}

		if (maxLinkDepth < visitStartLinkCnt) {
			LOG_TRACE(LOG_WARNING, "Failed, max table path depth over than result, max:%d, now:%d, link id:%lld, tile:%d, nid:%d, cur id:%lld, tile:%d, nid:%d", maxLinkDepth, visitStartLinkCnt, pCurInfo->linkId.llid, pCurInfo->linkId.tile_id, pCurInfo->linkId.nid, pLinkNow->linkId.llid, pLinkNow->linkId.tile_id, pLinkNow->linkId.nid);
			visitStartLinkCnt = -1;
			break;		
		}
	}

	return visitStartLinkCnt;
}


#if defined(USE_TSP_MODULE)
void printRDMresult(IN const RouteDistMatrix& RDM, IN const int time)
{
	// (cntCols < 100) { // 너무크면 로그 버퍼를 넘침, 보완해야함 (2024-08-02)) {
	// print result table

	// 경로 탐색 성공
	// print table rows
	const int cntCols = RDM.vtDistMatrix.size();
	const int cntRows = RDM.vtDistMatrix.size();

	static const int width = 8;
	string strRowsValue;
	char szBuff[1024] = { 0, };

	if (1) {
		//======================
		// print top
		// strRowsValue.clear();
		// memset(szBuff, 0x00, sizeof(szBuff));
		// memset(szBuff, '=', (width * (cntCols + 2)) * sizeof(char));
		// strRowsValue.append(szBuff);
		// LOG_TRACE(LOG_DEBUG, "%s", strRowsValue.c_str());

		// print colums index
		strRowsValue.clear();
		sprintf(szBuff, "       |");
		strRowsValue.append(szBuff);

		for (int cols = 0; cols < cntCols; cols++) {
			sprintf(szBuff, "%4d   |", cols);
			//sprintf(szBuff, "%8d   |", cols);
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
					//sprintf(szBuff, "%10.1f |", 0);
				} else {
					sprintf(szBuff, "%6d |", RDM.vtDistMatrix[rows][cols].nTotalDist);
					// sprintf(szBuff, "%10.1f |", ppResultTables[rows][cols].dbTotalCost);
				}
				strRowsValue.append(szBuff);
			} // for

			  // print result tick
			//sprintf(szBuff, "%6d |", static_cast<int32_t>(baseTablesEnd[rows].tickFinish - baseTablesEnd[rows].tickStart));
			sprintf(szBuff, "%6d |", 0);
			//sprintf(szBuff, "%10d |", baseTablesEnd[rows].tickFinish - baseTablesEnd[rows].tickStart);
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
		sprintf(szBuff, "%11d ms |", static_cast<int32_t>(time));
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


const int CRoutePlan::MakeTabulate(IN const RequestRouteInfo* pReqInfo, IN const vector<RouteLinkInfo>& linkInfos, OUT RouteDistMatrix& RDM)
{
	uint32_t ret = ROUTE_RESULT_SUCCESS;

	uint32_t cntRows = linkInfos.size();
	uint32_t cntCols = cntRows;
	if (cntRows <= 2) {
		LOG_TRACE(LOG_ERROR, "Failed, table rows too short, rows : %d", cntRows);
		return ROUTE_RESULT_FAILED;
	}

	// 지점 개수 만큼의 출발지 테이블 생성
	TableBaseInfo* baseTablesStart = new TableBaseInfo[cntRows];
	// 지점 개수 만큼의 목적지 테이블 생성
	TableBaseInfo* baseTablesEnd = new TableBaseInfo[cntRows];
	
	// 지점 개수 만큼의 결과 테이블(n * n) 생성
	RouteTable** ppResultTables = new RouteTable*[cntRows]; // create route table rows
	for (int ii = 0; ii < cntRows; ii++) {
		ppResultTables[ii] = new RouteTable[cntRows]; // create route table cols 
	}

	size_t tickTableStart = TICK_COUNT();

#if defined(USE_REAL_ROUTE_TSP)

	stLinkInfo* pLink = nullptr;
	stNodeInfo* pSNode = nullptr;
	stNodeInfo* pENode = nullptr;

	// 종료 지점의 탐색 매칭 카운트 계산
	uint32_t cntGoal = 0; // 목적지 도달 가능한 링크 수 
	uint32_t cntGoalTouch = 0; // 목적지 도달한 링크 수
	CandidateLink checkFinishRouteDir[2];// = { 0, }; // 최대 링크 양단에서 한번씩만 허용
	checkFinishRouteDir[0].costTreavel = DBL_MAX;
	checkFinishRouteDir[1].costTreavel = DBL_MAX;

	// 경탐 확장 시 목적지 도달 확인 용
	uint32_t checkDestinationCount = 0;
	unordered_map<uint64_t, vector<int32_t>> checkDestination;

	SBox boxExpandedArea;
	boxExpandedArea.Xmin = boxExpandedArea.Ymin = DBL_MAX;
	boxExpandedArea.Xmax = boxExpandedArea.Ymax = DBL_MIN;

	vector<SPoint> vtDestCoords;
	vtDestCoords.reserve(cntRows);

	// 기본 테이블 정보 생성
	for (int ii = 0; ii < cntRows; ii++) {
		TableBaseInfo* pBase = nullptr;

		//resultTables[ii] = new RouteTable[cntCols];

		// 시작 정보 등록
		pBase = &baseTablesStart[ii];
		pBase->reqInfo.SetOption(pReqInfo);
		//pBase->reqInfo.routeOption = pReqInfo->RouteOption;
		//pBase->reqInfo.avoidOption = pReqInfo->AvoidOption;
		//pBase->reqInfo.mobilityOption = pReqInfo->MobilityOption;
		//pBase->reqInfo.timeOption = pReqInfo->RequestTime;
		//pBase->reqInfo.trafficOption = pReqInfo->RequestTraffic;

		pBase->routeLinkInfo = linkInfos[ii];

		pLink = m_pDataMgr->GetLinkDataById(linkInfos[ii].LinkId, pReqInfo->vtLinkDataType[ii]);

		if (!pLink)
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find start link, tile:%d, link:%d", linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);
			ret = ROUTE_RESULT_FAILED_READ_DATA;
			break;
		}

		// 출발지 시작 갯수 예약
		CheckStartDirectionMaching(&pBase->reqInfo, pLink, &linkInfos[ii], pBase->vtCandidateLink);

		if (pBase->vtCandidateLink.empty()) {
			LOG_TRACE(LOG_WARNING, "Failed, can't find start link matching info, tile:%d, link:%d", linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);
			ret = ROUTE_RESULT_FAILED_SET_START;
			break;
		}


		// 종료점 링크의 도착정보
		pBase = &baseTablesEnd[ii];
		pBase->reqInfo.SetOption(pReqInfo);
		//pBase->routeOption = pReqInfo->RouteOption;
		//pBase->avoidOption = pReqInfo->AvoidOption;
		//pBase->mobilityOption = pReqInfo->MobilityOption;
		//pBase->timeOption = pReqInfo->RequestTime;
		//pBase->trafficOption = pReqInfo->RequestTraffic;
		pBase->routeLinkInfo = linkInfos[ii];

		pLink = m_pDataMgr->GetLinkDataById(linkInfos[ii].LinkId, pReqInfo->vtLinkDataType[ii]);

		if (!pLink)
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find end link, tile:%d, link:%d", linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);
			ret = ROUTE_RESULT_FAILED_READ_DATA;
			break;
		}

		// 목적지 도달 갯수 예약
		CheckEndDirectionMaching(&pBase->reqInfo, pLink, &linkInfos[ii], pBase->vtCandidateLink);

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

		unordered_map<uint64_t, vector<int32_t>>::iterator it = checkDestination.find(pLink->link_id.llid);
		if ( it != checkDestination.end()) { // insert
			it->second.emplace_back(ii); 
		} else { // add
			vector<int32_t> vtIdx = { ii };
			checkDestination.emplace(pLink->link_id.llid, vtIdx);
		}
		checkDestinationCount++;

		vector<stDistMatrix> vtDistMatrixRow(cntCols);
		RDM.vtDistMatrix.emplace_back(vtDistMatrixRow);
		
		vector<stPathMatrix> vtPathMatrixRow(cntCols);
		RDM.vtPathMatrix.emplace_back(vtPathMatrixRow);	

		LOG_TRACE(LOG_DEBUG, "Set destination link info, idx:%d, cnt:%d, tile:%d, link:%d", ii, pBase->vtCandidateLink.size(), linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);


		// 휴리스틱 거리 판단용 좌표 정보
		vtDestCoords.emplace_back(linkInfos[ii].MatchCoord);

		extendDataBox(boxExpandedArea, linkInfos[ii].MatchCoord.x, linkInfos[ii].MatchCoord.y);
	}


	if (ret != ROUTE_RESULT_SUCCESS) {
		return ret;
	}


#if defined(USE_TMS_API)
	std::unordered_map<uint64_t, uint8_t> umap_block;
	//m_pDataMgr->GetTrafficStaticSpeedBlock(pReqInfo->RequestTime, umap_block);
#endif

	// 테이블 각 행(rows)에 지점 정보(cols) 배치
#if defined(USE_MULTIPROCESS)
	volatile bool flag = false;
#endif

	int32_t cntFinishedRoute = 0;

#if defined(USE_MULTIPROCESS)
#pragma omp parallel for
#endif
	for (int ii = 0; ii < cntRows; ii++) 
	{
#if defined(USE_MULTIPROCESS)
		if (flag) continue;
#endif

		// 경로 탐색 시작
		time_t timeStart = LOG_TRACE(LOG_TEST, "Start finding goal table rows[%02d]", ii);

		TableBaseInfo* pStartTb = &baseTablesStart[ii];
		TableBaseInfo* pDestTb = &baseTablesEnd[ii];

		// 시작 시각
		pDestTb->tickStart = TICK_COUNT();

#if 0
		// 가장 먼 지점을 우선 탐색하기 위한 hueristirc cost 우선순위
		priority_queue<distanceCandidate, vector<distanceCandidate>, cmpFarthestDist> pqDist;
#else
		// 가장 가까운 지점을 우선 탐색하기 위한 hueristirc cost 우선순위
		priority_queue<distanceCandidate, vector<distanceCandidate>, cmpNearestDist> pqDist;
#endif

		// 목적지 정보를 기본 방문지 테이블로부터 복사
		for (int jj = 0; jj < cntCols; jj++) {
			if (ii == jj) {
				ppResultTables[ii][jj].nUsable = -1; // 자신은 패스
			} else {
				ppResultTables[ii][jj].nUsable = 0; // 미방문
				
				// 다른 지점과의 직선거리
				double dist = getRealWorldDistance(pStartTb->routeLinkInfo.Coord.x, pStartTb->routeLinkInfo.Coord.y,
					baseTablesStart[jj].routeLinkInfo.Coord.x, baseTablesStart[jj].routeLinkInfo.Coord.y);

				distanceCandidate itemDist = { jj, static_cast<int32_t>(dist) };
				pqDist.emplace(itemDist);
			}

			ppResultTables[ii][jj].linkInfo = baseTablesEnd[jj].routeLinkInfo;
		} // for cols
		// memcpy(resultTables[ii], baseTables, sizeof(RouteTable) * cntRows);


		// 시작 지점 정보 등록
		for (auto & item : pStartTb->vtCandidateLink) {
			CandidateLink* pItem = new CandidateLink();
			memcpy(pItem, item, sizeof(CandidateLink));
			pDestTb->mRoutePass.emplace(pItem->candidateId.llid, pItem);
			pDestTb->pqDijkstra.emplace(pItem);
		}

		CandidateLink* pCurInfo = nullptr;
		static const uint32_t cntMaxExtraSearch = 100; // 목적지 추가 도달 검사 최대 허용치
		uint32_t cntExtraSearch = 0; // 목적지 도달 후 추가 도달 검사 카운트

		uint32_t cntDestination = checkDestinationCount - 1; // 자신은 제외
		uint32_t cntGoal = 0;
		uint32_t cntExtraGoal = 0;
		int32_t idxPassedLink = -1; // 지나는 가지만 방향성이 안맞는 링크의 idx
		bool isFirst = true;
		bool isRetry = false;

		RouteTable* pCell = nullptr;
		stDistMatrix* pDMCell = nullptr;
		stPathMatrix* pPMCell = nullptr;

		unordered_map<uint64_t, vector<int32_t>>::const_iterator it;

		// 자신과 가장 가까운 지점을 우선 타겟팅 -> 먼 곳 우선으로 변경, 2025-02-28
		distanceCandidate curNearest = pqDist.top(); pqDist.pop();

		while (!pDestTb->pqDijkstra.empty()) {
			// 현재 링크 트리에서 제거
			pCurInfo = pDestTb->pqDijkstra.top(); pDestTb->pqDijkstra.pop();
			if ((pCurInfo == nullptr) || (pCurInfo->pPrevLink == nullptr && pCurInfo->depth > 1)) {
				continue;
			}
			pCurInfo->visited = true;

			// 부모가 사용 가능한 아이템인지 확인 후 가능한 것만 사용
			if (pCurInfo->isEnable == false) {
				continue;
			}

#if defined(USE_SHOW_ROUTE_SATATUS)
			//LOG_TRACE(LOG_DEBUG, "Current Link : id:%lld(real cost:%f, heuristic cost:%f, lv:%d)", curInfo.linkId, curInfo.costReal, curInfo.costHeuristic, curInfo.depth);

			// 현재까지 탐색된 정보 전달, Function Call
			if (m_fpRoutingStatus) {
				if (ii == 1) {// 특정 케이스 확인용
				//if (ii == 0) {
					m_fpRoutingStatus(m_pHost, &pDestTb->mRoutePass);
				}
			}
#endif

			// 목적지 목록의 아이템과 일치하면, 일치하는 목적지의 idx를 찾아서 정보 업데이트
			//if (((pCurInfo->linkId.llid == pRouteInfo->EndLinkInfo.LinkId.llid) && ((pRouteInfo->EndLinkInfo.LinkDir == 0) || (pCurInfo->dir == pRouteInfo->EndLinkInfo.LinkDir))) ||
			if ((it = checkDestination.find(pCurInfo->linkId.llid)) != checkDestination.end())
			{
				// 목적지 링크에 매칭된 모든 지점들을 확인한다.
				for (const auto & idx : it->second) {
					pCell = &ppResultTables[ii][idx];
					pDMCell = &RDM.vtDistMatrix[ii][idx];
					pPMCell = &RDM.vtPathMatrix[ii][idx];

					// 자기 자신은 패스
					if (ii == idx || pCell->nUsable < 0) {
						continue;
					}

					// 방향성이 맞지 않으면 패스
					//if ((pDestTb->routeLinkInfo.LinkDir != 0) && (pDestTb->routeLinkInfo.LinkDir != pCurInfo->dir)) {
					if ((pCell->linkInfo.LinkDir != 0) && (pCell->linkInfo.LinkDir != pCurInfo->dir)) {
						continue;
					}

					// 자신을 제외한 가장 가까운 지점인지 확인 -> 먼 곳 우선으로 변경, 2025-02-28
					if (curNearest.id == idx) {
						// 다음 가까운 지점 확인
						for (; !pqDist.empty(); ) {
							curNearest = pqDist.top(); pqDist.pop();

							// 미 확인된 지점이면 다음 타겟팅으로 선정
							if (ppResultTables[ii][curNearest.id].nUsable == 0) {
								// 2025-02-28, 다음 타겟 지정시 현재 지점을 기준으로 진행된 확장 코스트가 유지된채로 추가 확장되면
								// 출발지에서 다음 타겟으로 확장시 확장 코스트가 외곡되어 반영되어 돌아가는 경로가 발생할 수 있기에
								// 반드시 개선이 필요하다
#ifdef USE_RDM_COST_RESET
								// 다익스트라를 재정의 할까? // 2025-02-28
								priority_queue<CandidateLink*, vector<CandidateLink*>, decltype(CompareCanidate)> pqNear{ CompareCanidate };
								CandidateLink* pResetInfo = nullptr;
								while (!pDestTb->pqDijkstra.empty()) {
									pResetInfo = pDestTb->pqDijkstra.top(); pDestTb->pqDijkstra.pop();

									// 휴리스틱 재정의
									pResetInfo->costHeuristic = pResetInfo->costTreavel;

									pqNear.emplace(pResetInfo);
								}
								// 값 교환
								//pqNear.swap(pDestTb->pqDijkstra);
								//pDestTb->pqDijkstra = pqNear;
								while (!pqNear.empty()) {
									pResetInfo = pqNear.top(); pqNear.pop();

									pDestTb->pqDijkstra.emplace(pResetInfo);
								}
#endif
								break;
							}
						} // for
					}

					// 목적지와 테이블 정보가 같은 링크인지 재 확인
					if (it->first == pCell->linkInfo.LinkId.llid)
					{
						// 출발지와 도착지가 동일 링크일 경우
						// 출도착지의 방향
						if ((pStartTb->routeLinkInfo.LinkId.llid == pCell->linkInfo.LinkId.llid) &&
							((pCell->linkInfo.LinkDir != 0) && (pCell->linkInfo.LinkDir != pCurInfo->dir))) {
							continue;
						}

						if (isFirst && (pStartTb->routeLinkInfo.LinkId.llid == pCell->linkInfo.LinkId.llid)) {
							// 같은 방향인데 출발지가 도착지보다 앞에 있으면 방향이 안맞으므로 통과
							//if (pStartTb->routeLinkInfo.LinkDistToE < baseTablesEnd[idx].routeLinkInfo.LinkDistToE) {
							//	continue;
							//}
							if (pStartTb->routeLinkInfo.LinkDir != pCell->linkInfo.LinkDir) {
								continue;
							}
							else if (pCell->linkInfo.LinkDir == 1) { // 정
								if (pStartTb->routeLinkInfo.LinkDistToE < baseTablesEnd[idx].routeLinkInfo.LinkDistToE) {
									continue;
								}
							}
							else { // 역
								if (pStartTb->routeLinkInfo.LinkDistToS < baseTablesEnd[idx].routeLinkInfo.LinkDistToS) {
									continue;
								}
							}
						}
						
						// 첫방문
						if (pCell->nUsable++ <= 0) {
							cntGoal++;
							cntExtraGoal++;

							pDMCell->nTotalTime = pCurInfo->timeTreavel;
							pDMCell->nTotalDist = pCurInfo->distTreavel;
							pDMCell->dbTotalCost = pCurInfo->costTreavel;

							// 경로 탐색 성공
							//LOG_TRACE(LOG_TEST, "Find route table item, %d/%d, table[%d][%d], time:%d, dist:%d",
							//cntGoal, cntDestination, ii, idx, pCell->nTotalTime, pCell->nTotalDist);

							// 링크내 종료 포인트에서 노드까지의 거리 
							if (pCurInfo->dir == 1) {
								pDMCell->nTotalDist += baseTablesEnd[idx].routeLinkInfo.LinkDistToS;
							} else {
								pDMCell->nTotalDist += baseTablesEnd[idx].routeLinkInfo.LinkDistToE;
							}

							// path 저장
							pPMCell->startLinkInfo = pStartTb->routeLinkInfo;
							pPMCell->endLinkInfo = baseTablesEnd[idx].routeLinkInfo;

							// 상용은 아직 미정
							if (captureTablePath(pCurInfo, pPMCell->vtRoutePath) < 0) {
								LOG_TRACE(LOG_WARNING, "path capture result failed, row:%d, col:%d", ii, idx);
							}
						}
						// 재방문
						else { // if (pCell->nUsable <= 2) {
							cntExtraGoal++;

							bool needChange = false;
#if 1
							if (pReqInfo->RouteSubOption.rdm.compare_type == 1) { // COST
								if (pDMCell->dbTotalCost > pCurInfo->costTreavel) {
									needChange = true;
								}
							} else if (pReqInfo->RouteSubOption.rdm.compare_type == 2) { // DIST
								if (pDMCell->nTotalDist > pCurInfo->distTreavel) {
									needChange = true;
								}
							} else if (pReqInfo->RouteSubOption.rdm.compare_type == 3) { // TIME
								if (pDMCell->nTotalTime > pCurInfo->timeTreavel) {
									needChange = true;
								}
							} else { // DEFAULT
								if ((pDMCell->nTotalTime > pCurInfo->timeTreavel) ||
									(pDMCell->nTotalDist > pCurInfo->distTreavel) ||
									(pDMCell->dbTotalCost > pCurInfo->costTreavel)) {
									needChange = true;
								}
							}
#else
							// 짧은 녀석 사용
							if ((pDMCell->nTotalTime > pCurInfo->timeTreavel) ||
								(pDMCell->nTotalDist > pCurInfo->distTreavel) ||
								(pDMCell->dbTotalCost > pCurInfo->costTreavel)) {
								needChange = true;
							}
#endif
							if (needChange) {
								pDMCell->nTotalTime = pCurInfo->timeTreavel;
								pDMCell->nTotalDist = pCurInfo->distTreavel;
								pDMCell->dbTotalCost = pCurInfo->costTreavel;

								// 경로 탐색 성공
								//LOG_TRACE(LOG_TEST, "Find route table item, %d/%d, table[%d][%d], time:%d, dist:%d",
								//cntGoal, cntDestination, ii, idx, pCell->nTotalTime, pCell->nTotalDist);

								// 링크내 종료 포인트에서 노드까지의 거리 
								if (pCurInfo->dir == 1) {
									pDMCell->nTotalDist += baseTablesEnd[idx].routeLinkInfo.LinkDistToS;
								} else {
									pDMCell->nTotalDist += baseTablesEnd[idx].routeLinkInfo.LinkDistToE;
								}

								// path 저장
								pPMCell->startLinkInfo = pStartTb->routeLinkInfo;
								pPMCell->endLinkInfo = baseTablesEnd[idx].routeLinkInfo;

								// 상용은 아직 미정
								if (captureTablePath(pCurInfo, pPMCell->vtRoutePath) < 0) {
									LOG_TRACE(LOG_WARNING, "path capture result failed, row:%d, col:%d", ii, idx);
								}
							}
						}
					}
					else {
						// 여기로 들어오느다는건 목적지 테이블이 아예 잘못되었다는건데, 이건 있어서는 안되는 거야
						LOG_TRACE(LOG_WARNING, "Failed, destination table info not correct, table[%d][%d] should have llid:%lld, but now llid:%lld (tile:%d, id:%d)",
							ii, idx, it->first, pCell->linkInfo.LinkId.llid, pCell->linkInfo.LinkId.tile_id, pCell->linkInfo.LinkId.nid);
						ret = ROUTE_RESULT_FAILED_SET_MEMORY;
						break;
					}
				} // for
			}
			else {
				// 방향성이 틀려서 못찾는 경우
				// continue;
			}

			// 모든 방문지를 다 검색했으면
			if (cntDestination <= cntGoal)
			{
				// 경로 탐색 성공
				LOG_TRACE(LOG_DEBUG, timeStart, "Success make table[%d], cnt:%d/%d, goal:%d/%d +%d", ii, cntFinishedRoute + 1, cntRows, cntGoal, cntDestination, cntExtraGoal);
				ret = ROUTE_RESULT_SUCCESS;

				pDestTb->initPass();

				break;
			} else {
				isFirst = false;
				// 단순 확장
				Propagation(pDestTb, pCurInfo, pCurInfo->dir, boxExpandedArea, vtDestCoords, ii, curNearest.id, pReqInfo->RequestTime);
				
				if (pDestTb->pqDijkstra.empty() && (cntDestination != cntGoal) && !isRetry) {

					// 시작점이 방향성을 가지고 있으면 반대로 설정해 한번더 시도
					if (cntGoal <= 0) {
						if ((pStartTb->vtCandidateLink.size() == 1) && (pStartTb->vtCandidateLink[0]->dir != 0)) {
							isRetry = 1;

							// release mem
							pDestTb->initPass();

							// 시작 지점 정보 재등록
							// 출발지 진행 방향 바꾸기
							int oldDir = pStartTb->vtCandidateLink[0]->dir;
							pStartTb->vtCandidateLink[0]->dir = (oldDir == 1) ? 2 : 1;

							CandidateLink* pItem = new CandidateLink();
							memcpy(pItem, pStartTb->vtCandidateLink[0], sizeof(CandidateLink));
							pDestTb->mRoutePass.emplace(pItem->candidateId.llid, pItem);
							pDestTb->pqDijkstra.emplace(pItem);

							LOG_TRACE(LOG_DEBUG, "Retry, make table, rows[%02d], change start link dir, tile:%d, id:%d, dir:%d->%d", ii, pStartTb->vtCandidateLink[0]->linkId.tile_id, pStartTb->vtCandidateLink[0]->linkId.nid, oldDir, pStartTb->vtCandidateLink[0]->dir);
						}
					}
					// 도착지점이 방향성을 가지고 있으면 반대로 설정해 한번더 시도
					else {
						continue;

						if ((pStartTb->vtCandidateLink.size() == 1) && (pStartTb->vtCandidateLink[0]->dir != 0)) {
							isRetry = 2;

							// release mem
							pDestTb->initPass();

							// 시작 지점 정보 재등록
							CandidateLink* pItem = new CandidateLink();
							memcpy(pItem, pStartTb->vtCandidateLink[0], sizeof(CandidateLink));
							pDestTb->mRoutePass.emplace(pItem->candidateId.llid, pItem);
							pDestTb->pqDijkstra.emplace(pItem);

							// 도착 지점 정보 변경
							int idx = 0, oldDir = 0;
							RouteLinkInfo* pDest = nullptr;
							for (int idx = 0; idx < cntDestination; idx++) {
								if (ppResultTables[ii][idx].nUsable == false) {
									pDest = &ppResultTables[ii][idx].linkInfo;
									oldDir = pDest->LinkDir;
									pDest->LinkDir = (oldDir == 1) ? 2 : 1;

									LOG_TRACE(LOG_DEBUG, "Retry, make table, rows[%02d][%02d], change end link dir, tile:%d, id:%d, dir:%d->%d", ii, idx, pDest->LinkId.tile_id, pDest->LinkId.nid, oldDir, pDest->LinkDir);
								}
							}							
						}
					}
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
		} // while

		// 종료 시각
		pDestTb->tickFinish = TICK_COUNT();


		if (cntDestination != cntGoal) {
			if (isRetry) {
				int idx = 0;
				for (int idx = 0; idx < cntDestination; idx++) {
					if (ppResultTables[ii][idx].nUsable == false) {
						LOG_TRACE(LOG_WARNING, "Failed, can't retry make table, rows[%02d][%02d], link dir, tile:%d, id:%d, dir:%d", ii, idx, pStartTb->vtCandidateLink[0]->linkId.tile_id, pStartTb->vtCandidateLink[0]->linkId.nid, pStartTb->vtCandidateLink[0]->dir);
					}
				}
			} else {
				LOG_TRACE(LOG_DEBUG_LINE, "Failed, can't make table, rows[%02d], result:%d/%d, tick:%lld", ii, cntGoal, cntDestination, pDestTb->tickFinish - pDestTb->tickStart);
				for (int idx = 0; idx < cntDestination; idx++) {
					if (ppResultTables[ii][idx].nUsable == false) {
						LOG_TRACE(LOG_DEBUG_CONTINUE, ", %d", idx);
					}
				}
				LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");
			}

			ret = ROUTE_RESULT_FAILED_EXPEND;

#if defined(USE_MULTIPROCESS)
			//flag = true; <- 실패시 남은 결과 모두가 미처리 되고 있음, 2025-02-27
			continue;
#else
			break;
#endif	
		}

#pragma omp critical
		cntFinishedRoute++;

		// release mem
		// 테이블 행이 완성되면, 행 확장에 사용된 메모리 정리
		//SAFE_DELETE(pBase);
		//if (pBase) {
		//	delete pBase;
		//	pBase = nullptr;
		//}
	} // for rows

#else // #if defined(USE_REAL_ROUTE_TSP)
	for (int ii = 0; ii < cntRows; ii++) {
		baseTablesEnd[ii].tickStart = TICK_COUNT();

		for (int jj = 0; jj < cntCols; jj++) {
			if (ii == jj) continue;

			ppResultTables[ii][jj].nTotalDist = getRealWorldDistance(linkInfos[ii].Coord.x, linkInfos[ii].Coord.y, linkInfos[jj].Coord.x, linkInfos[jj].Coord.y);
		} // for jj

		baseTablesEnd[ii].tickFinish = TICK_COUNT();

	} // for ii
	ret = ROUTE_RESULT_SUCCESS;
#endif // #if defined(USE_REAL_ROUTE_TSP)

	size_t tickTable = TICK_COUNT() - tickTableStart;

	
	if (ret != ROUTE_RESULT_SUCCESS) {
		LOG_TRACE(LOG_DEBUG, "Failed, can't find route, couldn't leached end point(the point will isolated)");
	}
	else if (0) { 
		printRDMresult(RDM, tickTable);
	}

	LOG_TRACE(LOG_DEBUG, "Finish make table, rows:%d, tick:%d", cntRows, tickTable);


	// release mem	
	if (baseTablesStart) {
		// 생성된 candidate 데이터의 최종 누적 위치에서 삭제 --> doroute는 pass에서 알아서 삭제, table은 pass 안쓰니까 따로 삭제하자
		for (int ii = 0; ii < cntRows; ii++) {
			for (auto link : baseTablesStart[ii].vtCandidateLink) {
				SAFE_DELETE(link);
			}
		}
		SAFE_DELETE_ARR(baseTablesStart);
	}

	if (baseTablesEnd) {
		// 생성된 candidate 데이터의 최종 누적 위치에서 삭제 --> doroute는 pass에서 알아서 삭제, table은 pass 안쓰니까 따로 삭제하자
		for (int ii = 0; ii < cntRows; ii++) {
			for (auto link : baseTablesEnd[ii].vtCandidateLink) {
				SAFE_DELETE(link);
			}
		}
		//CandidateLink* pInfo;
		//for (int ii = 0; ii < cntRows; ii++) {
		//	for (; !baseTablesEnd[ii].routeInfo.pqDijkstra.empty();) {
		//		pInfo = baseTablesEnd[ii].routeInfo.pqDijkstra.top(); baseTablesEnd[ii].routeInfo.pqDijkstra.pop();
		//		SAFE_DELETE(pInfo);
		//	}
		//}
		SAFE_DELETE_ARR(baseTablesEnd);
	}

	if (ppResultTables) {
		for (int ii = 0; ii < cntRows; ii++) {
			SAFE_DELETE_ARR(ppResultTables[ii]);
		}
		SAFE_DELETE_ARR(ppResultTables);
	}

	return ret;
}


// reverse expand route from destination
const int CRoutePlan::MakeTabulateEx(IN const RequestRouteInfo* pReqInfo, IN const vector<RouteLinkInfo>& linkInfos, OUT RouteDistMatrix& RDM)
{
	uint32_t ret = ROUTE_RESULT_SUCCESS;

	uint32_t cntRows = linkInfos.size();
	uint32_t cntCols = cntRows;
#if defined(_WIN32) && defined(_DEBUG)
	if (cntRows <= 1) {
#else
	if (cntRows <= 2) {
#endif
		LOG_TRACE(LOG_ERROR, "Failed, table rows too short, rows : %d", cntRows);
		return ROUTE_RESULT_FAILED;
	}

	// 지점 개수 만큼의 출발지 테이블 생성
	TableBaseInfo* baseTablesStart = new TableBaseInfo[cntRows];
	// 지점 개수 만큼의 목적지 테이블 생성
	TableBaseInfo* baseTablesEnd = new TableBaseInfo[cntRows];
	
	// 지점 개수 만큼의 결과 테이블(n * n) 생성
	RouteTable** ppResultTables = new RouteTable*[cntRows]; // create route table rows
	for (int ii = 0; ii < cntRows; ii++) {
		ppResultTables[ii] = new RouteTable[cntRows]; // create route table cols 
	}

	size_t tickTableStart = TICK_COUNT();

#if defined(USE_REAL_ROUTE_TSP)

	stLinkInfo* pLink = nullptr;
	stNodeInfo* pSNode = nullptr;
	stNodeInfo* pENode = nullptr;

	// 종료 지점의 탐색 매칭 카운트 계산
	uint32_t cntGoal = 0; // 목적지 도달 가능한 링크 수 
	uint32_t cntGoalTouch = 0; // 목적지 도달한 링크 수
	CandidateLink checkFinishRouteDir[2];// = { 0, }; // 최대 링크 양단에서 한번씩만 허용
	checkFinishRouteDir[0].costTreavel = DBL_MAX;
	checkFinishRouteDir[1].costTreavel = DBL_MAX;

	// 경탐 확장 시 목적지 도달 확인 용
	uint32_t checkDestinationCount = 0;
	unordered_map<uint64_t, vector<int32_t>> checkDestination;

	SBox boxExpandedArea;
	boxExpandedArea.Xmin = boxExpandedArea.Ymin = DBL_MAX;
	boxExpandedArea.Xmax = boxExpandedArea.Ymax = DBL_MIN;

	vector<SPoint> vtDestCoords;
	vtDestCoords.reserve(cntRows);

	time_t startTable = LOG_TRACE(LOG_DEBUG, "Start make table");

	// 기본 테이블 정보 생성
	for (int ii = 0; ii < cntRows; ii++) {
		TableBaseInfo* pBase = nullptr;

		//resultTables[ii] = new RouteTable[cntCols];

		// 시작 정보 등록
		pBase = &baseTablesStart[ii];
		pBase->reqInfo.SetOption(pReqInfo);
		pBase->routeLinkInfo = linkInfos[ii];

		pLink = m_pDataMgr->GetLinkDataById(linkInfos[ii].LinkId, pReqInfo->vtLinkDataType[ii]);
		if (!pLink)
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find start link, tile:%d, link:%d", linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);
			ret = ROUTE_RESULT_FAILED_READ_DATA;
			break;
		}

		// 출발지 시작 갯수 예약
		CheckStartDirectionMaching(&pBase->reqInfo, pLink, &linkInfos[ii], pBase->vtCandidateLink);
		if (pBase->vtCandidateLink.empty()) {
			LOG_TRACE(LOG_WARNING, "Failed, can't find start link matching info, tile:%d, link:%d", linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);
			ret = ROUTE_RESULT_FAILED_SET_START;
			break;
		}


		// 종료점 링크의 도착정보
		pBase = &baseTablesEnd[ii];
		pBase->reqInfo.SetOption(pReqInfo);
		pBase->routeLinkInfo = linkInfos[ii];

		pLink = m_pDataMgr->GetLinkDataById(linkInfos[ii].LinkId, pReqInfo->vtLinkDataType[ii]);
		if (!pLink)
		{
			LOG_TRACE(LOG_WARNING, "Failed, can't find end link, tile:%d, link:%d", linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);
			ret = ROUTE_RESULT_FAILED_READ_DATA;
			break;
		}

		// 목적지 도달 갯수 예약
		CheckEndDirectionMaching(&pBase->reqInfo, pLink, &linkInfos[ii], pBase->vtCandidateLink);
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

		unordered_map<uint64_t, vector<int32_t>>::iterator it = checkDestination.find(pLink->link_id.llid);
		if ( it != checkDestination.end()) { // insert
			it->second.emplace_back(ii); 
		} else { // add			
			vector<int32_t> vtIdx = { ii };
			checkDestination.emplace(pLink->link_id.llid, vtIdx);
		}
		checkDestinationCount++;

		vector<stDistMatrix> vtDistMatrixRow(cntCols);
		RDM.vtDistMatrix.emplace_back(vtDistMatrixRow);
		
		vector<stPathMatrix> vtPathMatrixRow(cntCols);
		RDM.vtPathMatrix.emplace_back(vtPathMatrixRow);		

		LOG_TRACE(LOG_DEBUG, "Set destination link info, idx:%d, cnt:%d, tile:%d, link:%d", ii, pBase->vtCandidateLink.size(), linkInfos[ii].LinkId.tile_id, linkInfos[ii].LinkId.nid);


		// 휴리스틱 거리 판단용 좌표 정보
		vtDestCoords.emplace_back(linkInfos[ii].MatchCoord);

		extendDataBox(boxExpandedArea, linkInfos[ii].MatchCoord.x, linkInfos[ii].MatchCoord.y);
	}


	if (ret != ROUTE_RESULT_SUCCESS) {
		return ret;
	}


#if defined(USE_TMS_API)
	std::unordered_map<uint64_t, uint8_t> umap_block;
	//m_pDataMgr->GetTrafficStaticSpeedBlock(pReqInfo->RequestTime, umap_block);
#endif

	// 테이블 각 행(rows)에 지점 정보(cols) 배치
#if defined(USE_MULTIPROCESS)
	volatile bool flag = false;
#endif

	int32_t cntFinishedRoute = 0;

#if defined(USE_MULTIPROCESS)
#pragma omp parallel for
#endif
	for (int ii = 0; ii < cntRows; ii++)
	{
#if defined(USE_MULTIPROCESS)
		if (flag) continue;
#endif

		// 경로 탐색 시작
		time_t timeStart = LOG_TRACE(LOG_TEST, "Start finding goal table rows[%02d]", ii);

		TableBaseInfo* pStartTb = &baseTablesStart[ii];
		TableBaseInfo* pDestTb = &baseTablesEnd[ii];

		// 시작 시각
		pDestTb->tickStart = TICK_COUNT();

#if 0
		// 가장 먼 지점을 우선 탐색하기 위한 hueristirc cost 우선순위
		priority_queue<distanceCandidate, vector<distanceCandidate>, cmpFarthestDist> pqDist;
#else
		// 가장 가까운 지점을 우선 탐색하기 위한 hueristirc cost 우선순위
		priority_queue<distanceCandidate, vector<distanceCandidate>, cmpNearestDist> pqDist;
#endif

		// 목적지 정보를 기본 방문지 테이블로부터 복사
		for (int jj = 0; jj < cntCols; jj++) {
			if (ii == jj) {
				ppResultTables[ii][jj].nUsable = -1; // 자신은 패스
			} else {
				// 다른 지점과의 직선거리
				double dist = getRealWorldDistance(pStartTb->routeLinkInfo.Coord.x, pStartTb->routeLinkInfo.Coord.y,
					baseTablesStart[jj].routeLinkInfo.Coord.x, baseTablesStart[jj].routeLinkInfo.Coord.y);

				// 동일 좌표
				if (dist <= 0 || (pStartTb->routeLinkInfo.Coord == baseTablesStart[jj].routeLinkInfo.Coord)) {
					ppResultTables[ii][jj].nUsable = -1; // 자신은 패스
				} else {
					ppResultTables[ii][jj].nUsable = 0; // 미방문

					distanceCandidate itemDist = { jj, static_cast<int32_t>(dist) };
					pqDist.emplace(itemDist);
				}
			}

			ppResultTables[ii][jj].linkInfo = baseTablesEnd[jj].routeLinkInfo;
		} // for cols
		// memcpy(resultTables[ii], baseTables, sizeof(RouteTable) * cntRows);


		// 특정 레벨 전수 검사
#if defined(_DEBUG)
		const int MAX_LEVEL_LIMIT = 4; // default:3
#else
		const int MAX_LEVEL_LIMIT = 4; // default:3
#endif
								
#if 0
		RouteInfo routeInfoLevel; // 우선은 지역변수로, 이후에는 메모리 바로 제거 가능한 포인터 new 로 생성 사용하자, 2025-03-17
		// --> pStartTb을 대신 써도 되지 않을라나???, 2025-03-20
		routeInfoLevel.reqInfo.SetOption(&pStartTb->reqInfo);
		routeInfoLevel.StartLinkInfo = pStartTb->routeLinkInfo;


		// 출발지에서 확장하여 특정 레벨 접촉시 최초 접촉 링크 사용 <- 결국 이걸로 해야 됨 2025-03-19
		for (const auto& item : pStartTb->vtCandidateLink) {
			CandidateLink* pItem = new CandidateLink();
			memcpy(pItem, item, sizeof(CandidateLink));
			routeInfoLevel.mRoutePass.emplace(pItem->candidateId.llid, pItem);
			routeInfoLevel.pqDijkstra.emplace(pItem);
		}
#else
		time_t startLog = LOG_TRACE(LOG_DEBUG, "");

		// 출발지에서 확장하여 특정 레벨 접촉시 최초 접촉 링크 사용 <- 결국 이걸로 해야 됨 2025-03-19
		for (const auto& item : pStartTb->vtCandidateLink) {
			CandidateLink* pItem = new CandidateLink();
			memcpy(pItem, item, sizeof(CandidateLink));
			pStartTb->mRoutePass.emplace(pItem->candidateId.llid, pItem);
			pStartTb->pqDijkstra.emplace(pItem);
		}

		int32_t enableLimitLevel = 8;//USE_ROUTE_TABLE_LEVEL;  7->8인 경우 있음, 4612444969214927602, tile:176707, nid:1565426 -> 1565425
		while (!pStartTb->pqDijkstra.empty()) {
			CandidateLink* pCurInfo = pStartTb->pqDijkstra.top(); pStartTb->pqDijkstra.pop();
			LevelPropagation(pStartTb, pCurInfo, pCurInfo->dir, boxExpandedArea, MAX_LEVEL_LIMIT, enableLimitLevel);
		}
#endif
#if !defined(DEMO_FOR_IPO)
		LOG_TRACE(LOG_DEBUG, startLog, "Level propagation, idx:%d, level:%d, size:%d", ii, MAX_LEVEL_LIMIT, pStartTb->mRoutePass.size());
#endif
		

		// 시작 지점 정보 등록
//		for (auto & item : pStartTb->vtCandidateLink) {
//			CandidateLink* pItem = new CandidateLink();
//			memcpy(pItem, item, sizeof(CandidateLink));
//#ifdef USE_LINK_HISTORY_TABLE
//			pDestTb->mRoutePass.emplace(pItem->candidateId.llid, pItem);
//#endif
//			pDestTb->pqDijkstra.emplace(pItem);
//		}

		const uint32_t cntDestination = pqDist.size(); // 자신및 동일 좌표 제외
		uint32_t cntGoal = 0;
		CandidateLink* pCurInfo = nullptr;
		int32_t idxPassedLink = -1; // 지나는 가지만 방향성이 안맞는 링크의 idx
		bool isFirst = true;
		bool isRetry = false;

		RouteTable* pCell = nullptr;
		stDistMatrix* pDMCell = nullptr;
		stPathMatrix* pPMCell = nullptr;

		unordered_map<uint64_t, CandidateLink*>::const_iterator it;

		// 자신과 가장 가까운 지점을 우선 타겟팅 -> 먼 곳 우선으로 변경, 2025-02-28
		for (; !pqDist.empty(); ) {
			// 타겟 목적지에서 레벨 확장된 링크까지 역방향 탐색
			// 시작 지점 정보 등록
			RouteInfo routeInfoDest;
			distanceCandidate curNearest = pqDist.top(); pqDist.pop();
			int idx = curNearest.id;
			pDestTb = &baseTablesEnd[idx];
			for (auto & item : pDestTb->vtCandidateLink) {
				CandidateLink* pItem = new CandidateLink();
				memcpy(pItem, item, sizeof(CandidateLink));
				pItem->candidateId.parents_id = 0; // 현재 <-> 부모값 바꿔줌
				pItem->candidateId.current_id = item->candidateId.parents_id;

				routeInfoDest.mRouteReversePass.emplace(pItem->candidateId.llid, pItem);
				routeInfoDest.pqDijkstra.emplace(pItem);
			}
			routeInfoDest.reqInfo.SetOption(pReqInfo);
			routeInfoDest.StartLinkInfo = pStartTb->routeLinkInfo;
			routeInfoDest.EndLinkInfo = pDestTb->routeLinkInfo;

			static const int cntMaxExtraSearch = 200; // 목적지 도착 후, 추가 검사 횟수 최대 허용치
			static const int cntMaxExtraGoal = 3; // 목적지 도착 중복 최대 허용치
			uint32_t cntExtraSearch = 0; // 목적지 도달 후 추가 도달 검사 카운트
			uint32_t cntTryExtraGoal = 0;
			uint32_t cntExtraGoal = 0;
			CandidateLink selectedGoal;
			selectedGoal.distTreavel = DBL_MAX;;
			selectedGoal.timeTreavel = DBL_MAX;
			selectedGoal.costTreavel = DBL_MAX;

			//time_t timeStartCols = LOG_TRACE(LOG_TEST, "start finding goal table cols[%02d][%02d]", ii, idx);
			time_t timeStartCols = LOG_TRACE(LOG_TEST, "");


			while (!routeInfoDest.pqDijkstra.empty()) {
				// 현재 링크 트리에서 제거
				pCurInfo = routeInfoDest.pqDijkstra.top(); routeInfoDest.pqDijkstra.pop();
				if ((pCurInfo == nullptr) || (pCurInfo->pPrevLink == nullptr && pCurInfo->depth > 1)) {
					continue;
				}
				pCurInfo->visited = true;

				// 부모가 사용 가능한 아이템인지 확인 후 가능한 것만 사용
				if (pCurInfo->isEnable == false) {
					continue;
				}

#if defined(USE_SHOW_ROUTE_SATATUS)
				//LOG_TRACE(LOG_DEBUG, "Current Link : id:%lld(real cost:%f, heuristic cost:%f, lv:%d)", curInfo.linkId, curInfo.costReal, curInfo.costHeuristic, curInfo.depth);

				// 현재까지 탐색된 정보 전달, Function Call
				if (m_fpRoutingStatus) {
					//if (ii == 2) {// 강원도 출발 확장 상태
					if (ii == 0) {
						m_fpRoutingStatus(m_pHost, &routeInfoDest.mRouteReversePass);
					}
				}
#endif
				
				if (((it = pStartTb->mRoutePass.find(pCurInfo->candidateId.llid)) != pStartTb->mRoutePass.end()) ||
					((pCurInfo->linkId.llid == pStartTb->vtCandidateLink[0]->linkId.llid) &&
					((pStartTb->routeLinkInfo.LinkDir == 0) || (pStartTb->routeLinkInfo.LinkDir == pCurInfo->dir)))) { 
					// 출발지에 매칭(도착,중복) 또는
					// 목적지 목록의 아이템과 일치하면, 일치하는 목적지의 idx를 찾아서 정보 업데이트
					CandidateLink selectedGoalExtra;

					pCell = &ppResultTables[ii][idx];
					pDMCell = &RDM.vtDistMatrix[ii][idx];
					pPMCell = &RDM.vtPathMatrix[ii][idx];

					// 자기 자신은 패스
					if (ii == idx || pCell->nUsable < 0) {
						continue;
					}

					// 첫방문
					if (pCell->nUsable++ <= 0) {
						cntGoal++;
						cntTryExtraGoal++;
						cntExtraSearch++;
					}
					// 재방문
					else { // if (pCell->nUsable <= 2) {
						cntExtraGoal++;
						cntTryExtraGoal++;
						cntExtraSearch++;
					}

					// 출발지와 도착지가 동일 링크일 경우
					if ((pStartTb->routeLinkInfo.LinkId.llid == pCell->linkInfo.LinkId.llid) && (pCurInfo->depth == 0)) {
						// 같은 방향인데 출발지가 도착지보다 앞에 있으면 방향이 안맞으므로 통과
						//if (pStartTb->routeLinkInfo.LinkDistToE < pDestTb->routeLinkInfo.LinkDistToE) {
						//	continue;
						//}
						if (((pCell->linkInfo.LinkDir != 0) && (pCell->linkInfo.LinkDir != pCurInfo->dir))) {
							continue; // 출도착지의 방향
						} else if (pStartTb->routeLinkInfo.LinkDir != pCell->linkInfo.LinkDir) {
							continue;
						} else if ((pCell->linkInfo.LinkDir == 1) && pCurInfo->dir == 1) { // 정
							if (pStartTb->routeLinkInfo.LinkDistToE < pDestTb->routeLinkInfo.LinkDistToE) {
								continue;
							}
						} else if ((pCell->linkInfo.LinkDir == 2) && pCurInfo->dir == 2) { // 역
							if (pStartTb->routeLinkInfo.LinkDistToS < pDestTb->routeLinkInfo.LinkDistToS) {
								continue;
							}
						}

						double distDiff = 0;
						if (pCell->linkInfo.LinkDir == 1 && pCurInfo->dir == 1) { // 정
							distDiff = pStartTb->routeLinkInfo.LinkDistToE - pDestTb->routeLinkInfo.LinkDistToE;
						} else if (pCell->linkInfo.LinkDir == 2 && pCurInfo->dir == 2) { // 역
							distDiff = pStartTb->routeLinkInfo.LinkDistToE - pDestTb->routeLinkInfo.LinkDistToE;
						} else { // 양 
							distDiff = abs(pStartTb->routeLinkInfo.LinkDistToS - pDestTb->routeLinkInfo.LinkDistToS);

							//// S-E 위치에 따른 방향성 전환
							//if (pStartTb->routeLinkInfo.LinkDistToE > pDestTb->routeLinkInfo.LinkDistToE) {// 정
							//	pCurInfo->dir = 1;
							//} else { // 역
							//	pCurInfo->dir = 2;
							//}
						}

						int timeDiff = 0;
						int speed = 50; // 적정 속도 필요
						double spdValue = speed / 3.6f;
						if (distDiff > 0) {
							timeDiff = selectedGoal.costTreavel = (distDiff / spdValue);
						}

						if (pCurInfo->depth == 0) {
							selectedGoalExtra.distTreavel = distDiff;
							selectedGoalExtra.timeTreavel = timeDiff;
							selectedGoalExtra.costTreavel = timeDiff;
						} else {
							selectedGoalExtra.distTreavel = pCurInfo->distTreavel + distDiff;
							selectedGoalExtra.timeTreavel = pCurInfo->timeTreavel + timeDiff;
							selectedGoalExtra.costTreavel = pCurInfo->costTreavel + timeDiff;
						}
						selectedGoalExtra.linkId.llid = pCurInfo->linkId.llid;
						selectedGoalExtra.pPrevLink = pCurInfo;
					}
					//if ((pCurInfo->depth <= 2)) { // && (pStartTb->routeLinkInfo.LinkId.llid == pCell->linkInfo.LinkId.llid)) {
					else if (it != pStartTb->mRoutePass.end()) {
						//// 링크 재조립
						// 트리를 깨트리면 안되니까 정보만 저장했다가 마지막에 정리(재조립) 하자
						CandidateLink* pPrevLink = it->second;
						selectedGoalExtra.linkId.llid = pCurInfo->linkId.llid;
						selectedGoalExtra.distTreavel = pPrevLink->distTreavel + pCurInfo->distTreavel;
						selectedGoalExtra.timeTreavel = pPrevLink->timeTreavel + pCurInfo->timeTreavel;
						selectedGoalExtra.costTreavel = pPrevLink->costTreavel + pCurInfo->costTreavel;
						selectedGoalExtra.pPrevLink = pCurInfo;
					} else {
						//  출발지 도착
						selectedGoalExtra.linkId.llid = pCurInfo->linkId.llid;
						selectedGoalExtra.distTreavel = pCurInfo->distTreavel;
						selectedGoalExtra.timeTreavel = pCurInfo->timeTreavel;
						selectedGoalExtra.costTreavel = pCurInfo->costTreavel;
						selectedGoalExtra.pPrevLink = pCurInfo;

						// 출발지 위치를 빼야 함.
						double distDiff = 0;
						int timeDiff = 0;
						if (pCurInfo->dir == 1) { // 정
							distDiff = pStartTb->routeLinkInfo.LinkDistToE; // s로 나가니 남은 거리 (toE를 빼자)
						} else { // 역
							distDiff = pStartTb->routeLinkInfo.LinkDistToS; // e로 나가니 남은 거리 (toS를 빼자)
						}

						double spdValue = 50 / 3.6f;
						if (distDiff > 0) {
							timeDiff = (distDiff / spdValue);
						}

						selectedGoalExtra.distTreavel -= distDiff;
						selectedGoalExtra.timeTreavel -= timeDiff;
						selectedGoalExtra.costTreavel -= timeDiff;
					}

					bool needChange = false;
#if 1
					if (pReqInfo->RouteSubOption.rdm.compare_type == 1) { // COST
						if (selectedGoal.costTreavel > selectedGoalExtra.costTreavel) {
							needChange = true;
						}
					} else if (pReqInfo->RouteSubOption.rdm.compare_type == 2) { // DIST
						if (selectedGoal.distTreavel > selectedGoalExtra.distTreavel) {
							needChange = true;
						}
					} else if (pReqInfo->RouteSubOption.rdm.compare_type == 3) { // TIME
						if (selectedGoal.timeTreavel > selectedGoalExtra.timeTreavel) {
							needChange = true;
						}
					} else { // DEFAULT
						if ((selectedGoal.distTreavel > selectedGoalExtra.distTreavel) ||
							(selectedGoal.timeTreavel > selectedGoalExtra.timeTreavel)) {
							needChange = true;
						}
					}
#else
					if ((selectedGoal.distTreavel > selectedGoalExtra.distTreavel) ||
						(selectedGoal.timeTreavel > selectedGoalExtra.timeTreavel)) {
						//(selectedGoal.costTreavel > selectedGoalExtra.costTreavel)) { // 코스트는 비교대상에 넣지 말자
						needChange = true;
					}
#endif
					if (needChange) {
#if 0
						if (cntExtraGoal >= 1) {
							LOG_TRACE(LOG_DEBUG, "route matching change, row:%d, col:%d, ExtraGoal:%d, Retry:%d, dist(%.0f->%.0f), time(%.0f->%.0f)", ii, idx, cntTryExtraGoal, cntExtraSearch, selectedGoal.distTreavel, selectedGoalExtra.distTreavel, selectedGoal.timeTreavel, selectedGoalExtra.timeTreavel);
						}
#endif
						selectedGoal = selectedGoalExtra;
					}
				} else {
					isFirst = false;

					// 역방향 탐색
					int cntAddedLink = AddPrevLinksEx(&routeInfoDest, pCurInfo);
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
				if (((cntTryExtraGoal > 0) && (cntTryExtraGoal >= cntMaxExtraGoal)) || ((cntExtraSearch > 0) && (cntExtraSearch++ >= cntMaxExtraSearch))){
					// 역방향 탐색은 완료되면 종료하고 다음 방문지를 새로 찾는다.
#if 0 // !defined(DEMO_FOR_IPO)
					LOG_TRACE(LOG_TEST, timeStartCols, "finish table cols[%02d][%02d], %d/%d", ii, idx, cntGoal, cntCols - 1);
#endif
					break;
				}
			} // while

			if ((selectedGoal.linkId.llid != 0) && (selectedGoal.pPrevLink)) {
				// 링크 재조립
				CandidateLink* pCurInfo = selectedGoal.pPrevLink;
				CandidateLink* pPrevLink = nullptr;
				CandidateLink* pCurrLink = nullptr;
				CandidateLink* pNextLink = nullptr;
				if ((it = pStartTb->mRoutePass.find(pCurInfo->candidateId.llid)) != pStartTb->mRoutePass.end()) {
					// 상위 정방향 탐색 마지막 정보부터, 역방향 마지막 정보를 거꾸로 누적하며 갱신
					pPrevLink = it->second;
					pCurrLink = pCurInfo->pPrevLink->pPrevLink;
				} else { // <-- 이구문 타는 일이 없을텐데 이 구문 코드 적용 안되도 되는지 확인 필요, 2025-03-25, pCurrLink depth가 2이하일때 일 듯
					// 상위 정방향 탐색 마지막 정보부터, 역방향 마지막 정보를 거꾸로 누적하며 갱신
					pCurrLink = pCurInfo->pPrevLink;
					pPrevLink = pCurInfo;
					pPrevLink->pPrevLink = nullptr;
					pPrevLink->depth = 0;
				}

				for (int loop = 0; pCurrLink != nullptr; loop++) {
					// 누적 데이터 변환
					pCurrLink->distTreavel = pPrevLink->distTreavel + pCurrLink->dist;
					pCurrLink->timeTreavel = pPrevLink->timeTreavel + pCurrLink->time;
					pCurrLink->costTreavel = pPrevLink->costTreavel + pCurrLink->cost;
					pCurrLink->depth = pPrevLink->depth + 1;

					// 연결성 변환 // swap
					pNextLink = pCurrLink->pPrevLink;
					pCurrLink->pPrevLink = pPrevLink;
					pPrevLink = pCurrLink;
					pCurrLink = pNextLink;
				}
				pCurInfo = pPrevLink;

				pCell = &ppResultTables[ii][idx];
				pDMCell = &RDM.vtDistMatrix[ii][idx];
				pPMCell = &RDM.vtPathMatrix[ii][idx];

				pDMCell->nTotalTime = pCurInfo->timeTreavel;
				pDMCell->nTotalDist = pCurInfo->distTreavel;
				pDMCell->dbTotalCost = pCurInfo->costTreavel;

				// 경로 탐색 성공
				//LOG_TRACE(LOG_TEST, "Find route table item, %d/%d, table[%d][%d], time:%d, dist:%d",
				//cntGoal, cntDestination, ii, idx, pCell->nTotalTime, pCell->nTotalDist);

				// 링크내 종료 포인트에서 노드까지의 거리 
				if (pCurInfo->depth <= 0) {
					pDMCell->nTotalTime = selectedGoal.timeTreavel;
					pDMCell->nTotalDist = selectedGoal.distTreavel;
					pDMCell->dbTotalCost = selectedGoal.costTreavel;
				} else {
					if (pCurInfo->dir == 1) {
						pDMCell->nTotalDist += baseTablesEnd[idx].routeLinkInfo.LinkDistToS;
					} else {
						pDMCell->nTotalDist += baseTablesEnd[idx].routeLinkInfo.LinkDistToE;
					}
				}

				// path 저장
				pPMCell->startLinkInfo = pStartTb->routeLinkInfo;
				pPMCell->endLinkInfo = baseTablesEnd[idx].routeLinkInfo;

				// 상용은 아직 미정
				if (captureTablePath(pCurInfo, pPMCell->vtRoutePath) < 0) {
					LOG_TRACE(LOG_WARNING, "path capture result failed, row:%d, col:%d", ii, idx);
				}
			}

			// 종료 시각
			pDestTb->tickFinish = TICK_COUNT();
		} // for cols

		if (cntDestination != cntGoal) {
			if (isRetry) {
				int idx = 0;
				for (int idx = 0; idx < cntDestination; idx++) {
					if (ppResultTables[ii][idx].nUsable == false) {
						LOG_TRACE(LOG_WARNING, "Failed, can't retry make table, rows[%02d][%02d], link dir, tile:%d, id:%d, dir:%d", ii, idx, pStartTb->vtCandidateLink[0]->linkId.tile_id, pStartTb->vtCandidateLink[0]->linkId.nid, pStartTb->vtCandidateLink[0]->dir);
					}
				}
			} else {
				LOG_TRACE(LOG_DEBUG_LINE, "Failed, can't make table, rows[%02d], result:%d/%d, tick:%lld", ii, cntGoal, cntDestination, pDestTb->tickFinish - pDestTb->tickStart);
				for (int idx = 0; idx < cntDestination; idx++) {
					if (ppResultTables[ii][idx].nUsable == false) {
						LOG_TRACE(LOG_DEBUG_CONTINUE, ", %d", idx);
					}
				}
				LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");
			}

			ret = ROUTE_RESULT_FAILED_EXPEND;

#if defined(USE_MULTIPROCESS)
			//flag = true; <- 실패시 남은 결과 모두가 미처리 되고 있음, 2025-02-27
			continue;
#else
			break;
#endif	
		}

#pragma omp critical
		cntFinishedRoute++;

		// release mem
		// 테이블 행이 완성되면, 행 확장에 사용된 메모리 정리
		//SAFE_DELETE(pBase);
		//if (pBase) {
		//	delete pBase;
		//	pBase = nullptr;
		//}

		LOG_TRACE(LOG_TEST, timeStart, "finish table rows[%02d], %d/%d", ii, cntFinishedRoute, cntRows - 1);
	} // for rows

#else // #if defined(USE_REAL_ROUTE_TSP)
	for (int ii = 0; ii < cntRows; ii++) {
		baseTablesEnd[ii].tickStart = TICK_COUNT();

		for (int jj = 0; jj < cntCols; jj++) {
			if (ii == jj) continue;

			ppResultTables[ii][jj].nTotalDist = getRealWorldDistance(linkInfos[ii].Coord.x, linkInfos[ii].Coord.y, linkInfos[jj].Coord.x, linkInfos[jj].Coord.y);
		} // for jj

		baseTablesEnd[ii].tickFinish = TICK_COUNT();

	} // for ii
	ret = ROUTE_RESULT_SUCCESS;
#endif // #if defined(USE_REAL_ROUTE_TSP)

	size_t tickTable = TICK_COUNT() - tickTableStart;

	
	if (ret != ROUTE_RESULT_SUCCESS) {
		LOG_TRACE(LOG_DEBUG, "Failed, can't find route, couldn't leached end point(the point will isolated)");
	}
	else if (0) { 
		printRDMresult(RDM, tickTable);
	}

	LOG_TRACE(LOG_DEBUG, startTable, "Finish make table, rows:%d", cntRows);


	// release mem	
	if (baseTablesStart) {
		// 생성된 candidate 데이터의 최종 누적 위치에서 삭제 --> doroute는 pass에서 알아서 삭제, table은 pass 안쓰니까 따로 삭제하자
		for (int ii = 0; ii < cntRows; ii++) {
			for (auto link : baseTablesStart[ii].vtCandidateLink) {
				SAFE_DELETE(link);
			}
		}
		SAFE_DELETE_ARR(baseTablesStart);
	}

	if (baseTablesEnd) {
		// 생성된 candidate 데이터의 최종 누적 위치에서 삭제 --> doroute는 pass에서 알아서 삭제, table은 pass 안쓰니까 따로 삭제하자
		for (int ii = 0; ii < cntRows; ii++) {
			for (auto link : baseTablesEnd[ii].vtCandidateLink) {
				SAFE_DELETE(link);
			}
		}
		SAFE_DELETE_ARR(baseTablesEnd);
	}

	if (ppResultTables) {
		for (int ii = 0; ii < cntRows; ii++) {
			SAFE_DELETE_ARR(ppResultTables[ii]);
		}
		SAFE_DELETE_ARR(ppResultTables);
	}

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

	pRouteResult->reqInfo.SetOption(&pRouteInfo->reqInfo);
	//pRouteResult->RequestMode = pRouteInfo->reqInfo.RequestMode;
	//pRouteResult->RequestId = pRouteInfo->reqInfo.RequestId;
	//// 경로 옵션
	//pRouteResult->RouteOption = pRouteInfo->RouteOption;
	//pRouteResult->RouteAvoid = pRouteInfo->AvoidOption;
	//pRouteResult->RouteMobility = pRouteInfo->MobilityOption;

	pRouteResult->RouteBox.Xmin = pRouteResult->RouteBox.Ymin = INT_MAX;
	pRouteResult->RouteBox.Xmax = pRouteResult->RouteBox.Ymax = INT_MIN;

	const size_t cntLink = pRouteInfo->vtCandidateResult.size();
	pRouteResult->LinkInfo.reserve(cntLink);

	double remainDist = 0.f;
	double remainTime = 0.f;

	// 전체
	pRouteResult->TotalLinkCount = cntLink;


	// 경탐 좌표
	pRouteResult->StartResultLink.Coord = pRouteInfo->StartLinkInfo.Coord;
	pRouteResult->EndResultLink.Coord = pRouteInfo->EndLinkInfo.Coord;

	// Data Type
	pRouteResult->StartResultLink.LinkDataType = pRouteInfo->StartLinkInfo.LinkDataType;
	pRouteResult->EndResultLink.LinkDataType = pRouteInfo->EndLinkInfo.LinkDataType;


	// 시작 끝이 동일 링크이고, 주행 방향성이 동일한 경우, or // 단일 링크고, 시작 또는 끝이 노드일 경우
	if ((cntLink <= 2 && (pRouteInfo->StartLinkInfo.LinkId.llid == pRouteInfo->EndLinkInfo.LinkId.llid &&
		 pRouteInfo->StartLinkInfo.LinkDir == pRouteInfo->EndLinkInfo.LinkDir)) ||
	    (cntLink <= 1 && (pRouteInfo->StartLinkInfo.KeyType != pRouteInfo->EndLinkInfo.KeyType)))
	{
		stLinkInfo* pLink = nullptr;
		CandidateLink* pCandidateLink = nullptr;
		RouteResultLinkEx currentLinkInfo = { 0, };

		// 단일 링크고, 시작 또는 끝이 노드일 경우
		if (cntLink == 1 && (pRouteInfo->StartLinkInfo.KeyType != pRouteInfo->EndLinkInfo.KeyType))
		{
			pCandidateLink = pRouteInfo->vtCandidateResult.at(0);

			pLink = m_pDataMgr->GetLinkDataById(pCandidateLink->linkId, pRouteResult->StartResultLink.LinkDataType);

			if (pLink == nullptr) {
				LOG_TRACE(LOG_ERROR, "Failed, can't find route start link tile:%d, id:%d, ", pCandidateLink->linkId.tile_id, pCandidateLink->linkId.nid);
				return -1;
			}

			int offsetVtx = 0;
			int cntVtx = 0;
			if (pRouteInfo->StartLinkInfo.KeyType == TYPE_KEY_NODE) {
				offsetVtx = pRouteInfo->EndLinkInfo.LinkSplitIdx;
				if (pCandidateLink->dir == 1) { // 정
					pRouteResult->LinkVertex.assign(pRouteInfo->EndLinkInfo.LinkVtxToE.begin(), pRouteInfo->EndLinkInfo.LinkVtxToE.end());
				} else { // 역
					pRouteResult->LinkVertex.assign(pRouteInfo->EndLinkInfo.LinkVtxToS.begin(), pRouteInfo->EndLinkInfo.LinkVtxToS.end());
				}
			} else {
				offsetVtx = pRouteInfo->StartLinkInfo.LinkSplitIdx;
				if (pCandidateLink->dir == 1) { // 정
					pRouteResult->LinkVertex.assign(pRouteInfo->StartLinkInfo.LinkVtxToE.begin(), pRouteInfo->StartLinkInfo.LinkVtxToE.end());
				} else { // 역
					pRouteResult->LinkVertex.assign(pRouteInfo->StartLinkInfo.LinkVtxToS.begin(), pRouteInfo->StartLinkInfo.LinkVtxToS.end());
				}
			}
			cntVtx = pLink->cntVtx - offsetVtx;

			// 시작점
			pRouteResult->StartResultLink.LinkId = pCandidateLink->linkId;
			pRouteResult->StartResultLink.LinkDist = pCandidateLink->dist;
			pRouteResult->StartResultLink.MatchCoord = pRouteInfo->StartLinkInfo.MatchCoord;

			// 종료점
			pRouteResult->EndResultLink.LinkId = pCandidateLink->linkId;
			pRouteResult->EndResultLink.LinkDist = pCandidateLink->dist;
			pRouteResult->EndResultLink.MatchCoord = pRouteInfo->EndLinkInfo.MatchCoord;

			// 링크 정보
			currentLinkInfo.link_id = pCandidateLink->linkId;
			currentLinkInfo.node_id = pCandidateLink->nodeId;
			currentLinkInfo.length = pCandidateLink->dist;
			currentLinkInfo.time = pCandidateLink->time;
			currentLinkInfo.rlength = pCandidateLink->dist;
			currentLinkInfo.rtime = pCandidateLink->time;
			currentLinkInfo.vtx_off = offsetVtx;
			currentLinkInfo.vtx_cnt = cntVtx;
			currentLinkInfo.link_info = pLink->sub_info;
			currentLinkInfo.angle = 0;
			currentLinkInfo.dir = pCandidateLink-> dir == 1 ? 0 : 1;
#if defined(USE_VEHICLE_DATA)
			currentLinkInfo.speed = pCandidateLink->speed;
			currentLinkInfo.speed_type = pCandidateLink->speed_type;
			currentLinkInfo.speed_level = pCandidateLink->speed_level;
#endif
		}
		else
		{
			// 시작 링크
			pCandidateLink = pRouteInfo->vtCandidateResult.at(cntLink - 1);

			// 시작점
			pRouteResult->StartResultLink.LinkId = pRouteInfo->StartLinkInfo.LinkId;
			pRouteResult->StartResultLink.LinkDataType = pRouteInfo->StartLinkInfo.LinkDataType;
			pRouteResult->StartResultLink.MatchCoord = pRouteInfo->StartLinkInfo.MatchCoord;
			pRouteResult->StartResultLink.LinkSplitIdx = pRouteInfo->StartLinkInfo.LinkSplitIdx;

			// 종료점
			pRouteResult->EndResultLink.LinkId = pRouteInfo->EndLinkInfo.LinkId;
			pRouteResult->EndResultLink.LinkDataType = pRouteInfo->EndLinkInfo.LinkDataType;
			pRouteResult->EndResultLink.MatchCoord, pRouteInfo->EndLinkInfo.MatchCoord;
			pRouteResult->EndResultLink.LinkSplitIdx = pRouteInfo->EndLinkInfo.LinkSplitIdx;


			// 시작 vtx
			int dir = pCandidateLink->dir;
			//dir = pRouteInfo->StartLinkInfo.LinkDir;
			//if (dir == 0) { // 양방향
			//	if (pRouteInfo->StartLinkInfo.LinkDistToS >= pRouteInfo->EndLinkInfo.LinkDistToS) {
			//		dir = 2; // 역
			//	} else {
			//		dir = 1; // 정
			//	}
			//}

			// s노드로 나간 경우, 역
			if (dir == 2)
			{
				pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToS - pRouteInfo->EndLinkInfo.LinkDistToS;
				pRouteResult->LinkVertex.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToS[0]);
			}
			// e노드로 나간 경우, 정
			else
			{
				pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE - pRouteInfo->EndLinkInfo.LinkDistToE;
				pRouteResult->LinkVertex.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToE[0]);
			}
			// 양방향
			//else
			//{
			//	// 역
			//	if (pRouteInfo->StartLinkInfo.LinkDistToS >= pRouteInfo->EndLinkInfo.LinkDistToS) {
			//		pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToS - pRouteInfo->EndLinkInfo.LinkDistToS;
			//		pRouteResult->LinkVertex.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToS[0]);
			//	}
			//	// 정
			//	else {
			//		pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE - pRouteInfo->EndLinkInfo.LinkDistToE;
			//		pRouteResult->LinkVertex.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToE[0]);
			//	}
			//}


			// 중간 vtx
			if (pRouteInfo->StartLinkInfo.LinkSplitIdx != pRouteInfo->EndLinkInfo.LinkSplitIdx) {
				// s노드로 나간 경우, 역
				//if (pRouteInfo->StartLinkInfo.LinkDir == 2)
				if (dir == 2)
				{
					for (int ii = 1; ii <= pRouteInfo->StartLinkInfo.LinkSplitIdx - pRouteInfo->EndLinkInfo.LinkSplitIdx; ii++) {
						pRouteResult->LinkVertex.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToS[ii]);
					}
				}
				// e노드로 나간 경우, 정
				//else if (pRouteInfo->StartLinkInfo.LinkDir == 1)
				else 
				{
					for (int ii = 1; ii <= pRouteInfo->EndLinkInfo.LinkSplitIdx - pRouteInfo->StartLinkInfo.LinkSplitIdx; ii++) {
						pRouteResult->LinkVertex.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToE[ii]);
					}
				}
				// 양방향
				//else
				//{
				//	// 역
				//	if (pRouteInfo->StartLinkInfo.LinkSplitIdx > pRouteInfo->EndLinkInfo.LinkSplitIdx) {
				//		for (int ii = pRouteInfo->EndLinkInfo.LinkSplitIdx + 1; ii <= pRouteInfo->StartLinkInfo.LinkSplitIdx; ii++) {
				//			pRouteResult->LinkVertex.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToS[ii]);
				//		}
				//	}
				//	// 정
				//	else {
				//		for (int ii = pRouteInfo->StartLinkInfo.LinkSplitIdx + 1; ii <= pRouteInfo->EndLinkInfo.LinkSplitIdx; ii++) {
				//			pRouteResult->LinkVertex.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToE[ii]);
				//		} 
				//	}
				//}
			}


			// 종료 vtx
			//dir = pRouteInfo->EndLinkInfo.LinkDir;
			// e노드로 들어온 경우 // 역
			if (dir == 2)
			{
				// 진행과 반대방향의 시작 vtx
				pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToE - pRouteInfo->StartLinkInfo.LinkDistToE;
				pRouteResult->LinkVertex.emplace_back(pRouteInfo->EndLinkInfo.LinkVtxToE[pRouteInfo->EndLinkInfo.LinkVtxToE.size() - 1]);
			}
			// s노드로 들어온 경우 // 정
			else // if (dir == 1)
			{
				// 진행과 반대방향의 시작 vtx
				pRouteResult->EndResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE - pRouteInfo->EndLinkInfo.LinkDistToE;
				pRouteResult->LinkVertex.emplace_back(pRouteInfo->EndLinkInfo.LinkVtxToS[pRouteInfo->EndLinkInfo.LinkVtxToS.size() - 1]);
			}
			// 양방향
			//else
			//{
			//	// 역
			//	if (pRouteInfo->EndLinkInfo.LinkDistToE >= pRouteInfo->StartLinkInfo.LinkDistToE) {
			//		pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToE - pRouteInfo->StartLinkInfo.LinkDistToE;
			//		pRouteResult->LinkVertex.emplace_back(pRouteInfo->EndLinkInfo.LinkVtxToS[pRouteInfo->EndLinkInfo.LinkVtxToS.size() - 1]);
			//	}
			//	// 정
			//	else {
			//		pRouteResult->EndResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE - pRouteInfo->EndLinkInfo.LinkDistToE;
			//		pRouteResult->LinkVertex.emplace_back(pRouteInfo->EndLinkInfo.LinkVtxToE[pRouteInfo->EndLinkInfo.LinkVtxToE.size() - 1]);
			//	}
			//}

			//linkMerge(pRouteResult->LinkVertex, pRouteResult->StartResultLink.LinkVtx);

			pLink = m_pDataMgr->GetLinkDataById(pRouteResult->StartResultLink.LinkId, pRouteResult->StartResultLink.LinkDataType);

			if (pLink == nullptr) {
				LOG_TRACE(LOG_ERROR, "Failed, can't find route start link tile:%d, id:%d, ", pRouteResult->StartResultLink.LinkId.tile_id, pRouteResult->StartResultLink.LinkId.nid);
				return -1;
			}

			pCandidateLink = pRouteInfo->vtCandidateResult.at(0);

			currentLinkInfo.link_id = pRouteResult->StartResultLink.LinkId;
			currentLinkInfo.node_id.llid = pLink->enode_id.llid;
			currentLinkInfo.link_info = pLink->sub_info;
			currentLinkInfo.length = pRouteResult->StartResultLink.LinkDist;
			currentLinkInfo.time = min(1, (int)(pCandidateLink->time * (pRouteResult->StartResultLink.LinkDist / pCandidateLink->dist))); // 종료 링크시간은 사용된 거리대비 시간만큼만 사용.
			currentLinkInfo.rlength = currentLinkInfo.length;
			currentLinkInfo.rtime = currentLinkInfo.time;
			currentLinkInfo.vtx_off = pRouteResult->StartResultLink.LinkSplitIdx;
			currentLinkInfo.vtx_cnt = pRouteResult->LinkVertex.size(); // pRouteInfo->EndLinkInfo.LinkSplitIdx - pRouteResult->StartResultLink.LinkSplitIdx + 1;
			currentLinkInfo.angle = 0;
			currentLinkInfo.dir = pCandidateLink->dir == 1 ? 0 : 1;
#if defined(USE_VEHICLE_DATA)
			currentLinkInfo.speed = pCandidateLink->speed;
			currentLinkInfo.speed_type = pCandidateLink->speed_type;
			currentLinkInfo.speed_level = pCandidateLink->speed_level;
#endif
		}

		// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
		if ((pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DEPARTURE) && (pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT)) { // 출발지-경유지
			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEPARTURE_WAYPOINT; //4:SV출발지-경유지
		}
		else if ((pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DEPARTURE) && (pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DESTINATION)) { // 출발지-도착지
			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEPARTURE_DESTINATION; //5:SE출발지-도착지
		}
		else if ((pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT) && (pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT)) { // 출발지-경유지
			//currentLinkInfo.guide_type = LINK_GUIDE_TYPE_WAYPOINT_WAYPOINT; //6:VV경유지-경유지
			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_WAYPOINT; // 출발지와 동일링크가 아니면 개별 정보로 표기해도 될듯 
		}
		else if ((pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT) && (pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DESTINATION)) { // 출발지-도착지
			//currentLinkInfo.guide_type = LINK_GUIDE_TYPE_WAYPOINT_DESTINATION; //7:VE경유지-도착지
			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DESTINATION; // 출발지와 동일링크가 아니면 개별 정보로 표기해도 될듯 
		}
		else {
			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEPARTURE; //1:S출발지링크
			// currentLinkInfo.type = 3; //3:E도착지링크
		}

		// add link
		pRouteResult->LinkInfo.emplace_back(currentLinkInfo);

		pRouteResult->TotalLinkDist += currentLinkInfo.length;
		pRouteResult->TotalLinkTime += currentLinkInfo.time;
	}
	else
	{
		// 시작 링크
		stLinkInfo* pLink = nullptr;
		CandidateLink* pCandidateLink = nullptr;

		pCandidateLink = pRouteInfo->vtCandidateResult.at(cntLink - 1);

		if (pRouteInfo->StartLinkInfo.KeyType == TYPE_KEY_NODE) { // node type)
			pLink = m_pDataMgr->GetLinkDataById(pCandidateLink->linkId, pCandidateLink->linkDataType);

			if (pLink == nullptr) {
				LOG_TRACE(LOG_ERROR, "Failed, can't find route start link tile:%d, id:%d, ", pCandidateLink->linkId.tile_id, pCandidateLink->linkId.nid);
				return -1;
			}

			pRouteResult->StartResultLink.LinkId = pLink->link_id;
			pRouteResult->StartResultLink.LinkDist = pLink->length;
			if (pCandidateLink->dir == 1) { // 정
				// 마지막 점
				pRouteResult->StartResultLink.MatchCoord.x = pLink->getVertexX(pLink->cntVtx - 1);
				pRouteResult->StartResultLink.MatchCoord.y = pLink->getVertexY(pLink->cntVtx - 1);
				pRouteResult->StartResultLink.LinkSplitIdx = pLink->cntVtx - 1;
				pRouteResult->StartResultLink.LinkVtx.assign(pLink->getVertex(), pLink->getVertex() + pLink->cntVtx);
			}
			else { // 역
				pRouteResult->StartResultLink.MatchCoord.x = pLink->getVertexX(0);
				pRouteResult->StartResultLink.MatchCoord.y = pLink->getVertexY(0);
				pRouteResult->StartResultLink.LinkSplitIdx = 0;
				vector<SPoint>tmpVtx;
				tmpVtx.assign(pLink->getVertex(), pLink->getVertex() + pLink->cntVtx);
				pRouteResult->StartResultLink.LinkVtx.assign(tmpVtx.rbegin(), tmpVtx.rend()); // 역순 복사
			}
		}
		else {
			pRouteResult->StartResultLink.LinkId = pRouteInfo->StartLinkInfo.LinkId;
			pRouteResult->StartResultLink.MatchCoord = pRouteInfo->StartLinkInfo.MatchCoord;
			pRouteResult->StartResultLink.LinkSplitIdx = pRouteInfo->StartLinkInfo.LinkSplitIdx;
			// s노드로 나간 경우, 역
			if (pCandidateLink->dir == 2)
			{
				pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToS;
				pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToS.begin(), pRouteInfo->StartLinkInfo.LinkVtxToS.end());
			}
			// e노드로 나간 경우, 정
			else if (pCandidateLink->dir == 1)
			{
				pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE;
				pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToE.begin(), pRouteInfo->StartLinkInfo.LinkVtxToE.end());
			}
			else
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't find route start link, link:%d, dir:%d", pCandidateLink->linkId.nid, pCandidateLink->linkId.dir);
				return -1;
			}

			pLink = m_pDataMgr->GetLinkDataById(pRouteResult->StartResultLink.LinkId, pRouteResult->StartResultLink.LinkDataType);
		}

		// 링크 진행 각도 계산용 정/역
		int angStart = 0;
		int angEnd = 0;
		int angDiff = 0;
		int targetDir = 0;

		RouteResultLinkEx currentLinkInfo = { 0, };

		if (pLink == nullptr) {
			LOG_TRACE(LOG_ERROR, "Failed, can't find route start link(%d)", pRouteResult->StartResultLink.LinkId.nid);
			return -1;
		}
		else if (pCandidateLink->linkId.nid != pLink->link_id.nid) {
			LOG_TRACE(LOG_ERROR, "Failed, matched start link not same with result start, smlink:%d != srlink:%d", pCandidateLink->linkId.nid, pLink->link_id.nid);
			return -1;
		}

		currentLinkInfo.link_id = pCandidateLink->linkId;
		currentLinkInfo.node_id = pCandidateLink->nodeId;
		currentLinkInfo.link_info = pLink->sub_info;
		currentLinkInfo.length = pRouteResult->StartResultLink.LinkDist; // pCandidateStartLink->distReal; 
#if defined(USE_PEDESTRIAN_DATA)
		currentLinkInfo.time = GetTravelTime(&pRouteInfo->reqInfo, pLink, SPEED_NOT_AVALABLE, 0, currentLinkInfo.length, pCandidateLink->time);
#else
		currentLinkInfo.time = min(1, (int)(pCandidateLink->time * (pRouteResult->StartResultLink.LinkDist / pCandidateLink->dist)));
#endif
		currentLinkInfo.vtx_off += currentLinkInfo.vtx_cnt;
		currentLinkInfo.vtx_cnt = pRouteResult->StartResultLink.LinkVtx.size();
		//currentLinkInfo.rlength = remainDist;
		//currentLinkInfo.rtime = remainTime;

		// angle

		targetDir = pCandidateLink->dir;		

		// 주행 각도는 링크 시작값으로 주기에 최초 링크는 각도 무시.
		currentLinkInfo.angle = 0;
		angStart = getAngle(pLink, targetDir);

		if (targetDir == 1) { // 정 to enode
			currentLinkInfo.dir = 0;
		} else { // 역 to snode
			currentLinkInfo.dir = 1;
		}
#if defined(USE_VEHICLE_DATA)
		currentLinkInfo.speed = pCandidateLink->speed;
		currentLinkInfo.speed_type = pCandidateLink->speed_type;
		currentLinkInfo.speed_level = pCandidateLink->speed_level;
#endif

		// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
		if (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DEPARTURE) {
			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEPARTURE; //1:S출발지링크
		} else if (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT) {
			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEFAULT;// LINK_GUIDE_TYPE_WAYPOINT; //경유지부터의 출발은 표시하지 않는다
		} else {
			// 잘못된 링크 속성
			LOG_TRACE(LOG_WARNING, "first link guild type not correct, %d", pRouteInfo->StartLinkInfo.LinkGuideType);
			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEPARTURE; //1:S출발지링크
		}
		
		// add link
		pRouteResult->LinkInfo.emplace_back(currentLinkInfo);
		linkMerge(pRouteResult->LinkVertex, pRouteResult->StartResultLink.LinkVtx);

		pRouteResult->TotalLinkDist += currentLinkInfo.length;
		pRouteResult->TotalLinkTime += currentLinkInfo.time;



		// 
		// 중간 링크
		for (int ii = cntLink - 2; ii > 0; --ii)
		{
			pCandidateLink = pRouteInfo->vtCandidateResult.at(ii);

			pLink = m_pDataMgr->GetLinkDataById(pCandidateLink->linkId, pCandidateLink->linkDataType);

			if (pLink == nullptr) {
				LOG_TRACE(LOG_ERROR, "Failed, can't find route mid link(%d)", ii);
				return -1;
			}

			currentLinkInfo.link_id = pCandidateLink->linkId;
			currentLinkInfo.node_id = pCandidateLink->nodeId;
			currentLinkInfo.link_info = pLink->sub_info;
			currentLinkInfo.length = pCandidateLink->dist;
#if defined(USE_PEDESTRIAN_DATA)
			currentLinkInfo.time = GetTravelTime(&pRouteInfo->reqInfo, pLink, SPEED_NOT_AVALABLE, 0, currentLinkInfo.length, pCandidateLink->time);
#else
			currentLinkInfo.time = pCandidateLink->time;
#endif
			currentLinkInfo.vtx_off += currentLinkInfo.vtx_cnt;
			currentLinkInfo.vtx_cnt = pLink->getVertexCount();
			//currentLinkInfo.rlength = remainDist;
			//currentLinkInfo.rtime = remainTime;

			// angle
			targetDir = pCandidateLink->dir;
			angEnd = getAngle(pLink, targetDir, false);

			currentLinkInfo.angle = getEntryAngle(angStart, angEnd);
			angStart = getAngle(pLink, targetDir);

			if (targetDir == 1) { // 정, to enode
				currentLinkInfo.dir = 0;
			} else { // 역, to snode
				currentLinkInfo.dir = 1;
			}
#if defined(USE_VEHICLE_DATA)
			currentLinkInfo.speed = pCandidateLink->speed;
			currentLinkInfo.speed_type = pCandidateLink->speed_type;
			currentLinkInfo.speed_level = pCandidateLink->speed_level;
#endif

			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEFAULT;// 일반

			// add link
			pRouteResult->LinkInfo.emplace_back(currentLinkInfo);
			linkMerge(pRouteResult->LinkVertex, pLink->getVertex(), pLink->getVertexCount());

			pRouteResult->TotalLinkDist += currentLinkInfo.length;
			pRouteResult->TotalLinkTime += currentLinkInfo.time;
		}


		//
		// 종료 링크
		pCandidateLink = pRouteInfo->vtCandidateResult.at(0);
		
		if (pRouteInfo->EndLinkInfo.KeyType == TYPE_KEY_NODE) { // node type)
			pLink = m_pDataMgr->GetLinkDataById(pCandidateLink->linkId, pCandidateLink->linkDataType);

			if (pLink == nullptr) {
				LOG_TRACE(LOG_ERROR, "Failed, can't find route end link tile:%d, id:%d, ", pCandidateLink->linkId.tile_id, pCandidateLink->linkId.nid);
				return -1;
			}

			pRouteResult->EndResultLink.LinkId = pLink->link_id;
			pRouteResult->EndResultLink.LinkDist = pLink->length;
			if (pCandidateLink->dir == 1) { // 정
				// 마지막 점
				pRouteResult->EndResultLink.MatchCoord.x = pLink->getVertexX(pLink->cntVtx - 1);
				pRouteResult->EndResultLink.MatchCoord.y = pLink->getVertexY(pLink->cntVtx - 1);
				pRouteResult->EndResultLink.LinkSplitIdx = pLink->cntVtx - 1;
				pRouteResult->EndResultLink.LinkVtx.assign(pLink->getVertex(), pLink->getVertex() + pLink->cntVtx);
			}
			else { // 역
				pRouteResult->EndResultLink.MatchCoord.x = pLink->getVertexX(0);
				pRouteResult->EndResultLink.MatchCoord.y = pLink->getVertexY(0);
				pRouteResult->EndResultLink.LinkSplitIdx = 0;
				vector<SPoint>tmpVtx;
				tmpVtx.assign(pLink->getVertex(), pLink->getVertex() + pLink->cntVtx);
				pRouteResult->EndResultLink.LinkVtx.assign(tmpVtx.rbegin(), tmpVtx.rend()); // 역순 복사
			}
		}
		else {
			pRouteResult->EndResultLink.LinkId = pRouteInfo->EndLinkInfo.LinkId;
			pRouteResult->EndResultLink.MatchCoord = pRouteInfo->EndLinkInfo.MatchCoord;
			pRouteResult->EndResultLink.LinkSplitIdx = pRouteInfo->EndLinkInfo.LinkSplitIdx;
			// s노드로 들어온 경우
			//if (pCandidateLink->linkId.dir == 1)
			if (pCandidateLink->dir == 1)
			{
				pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToS;
				pRouteResult->EndResultLink.LinkVtx.assign(pRouteInfo->EndLinkInfo.LinkVtxToS.begin(), pRouteInfo->EndLinkInfo.LinkVtxToS.end());
			}
			// e노드로 들어온 경우
			/*else if (pCandidateLink->linkId.dir == 2)*/
			else if (pCandidateLink->dir == 2)
			{
				pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToE;
				pRouteResult->EndResultLink.LinkVtx.assign(pRouteInfo->EndLinkInfo.LinkVtxToE.begin(), pRouteInfo->EndLinkInfo.LinkVtxToE.end());
			}
			else
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't find route end link, link:%d, dir:%d", pCandidateLink->linkId.nid, pCandidateLink->linkId.dir);
				return -1;
			}

			pLink = m_pDataMgr->GetLinkDataById(pRouteResult->EndResultLink.LinkId, pRouteResult->EndResultLink.LinkDataType);
		}

		if (pLink == nullptr) {
			LOG_TRACE(LOG_ERROR, "Failed, can't find route end link(%d)", pRouteResult->EndResultLink.LinkId.nid);
			return -1;
		}
		else if (pCandidateLink->linkId.nid != pLink->link_id.nid) {
			LOG_TRACE(LOG_ERROR, "Failed, matched end link not same with result end, emlink:%d != erlink:%d", pCandidateLink->linkId.nid, pLink->link_id.nid);
			return -1;
		}

		currentLinkInfo.link_id = pCandidateLink->linkId;
		currentLinkInfo.node_id = pCandidateLink->nodeId;
		currentLinkInfo.link_info = pLink->sub_info;
		currentLinkInfo.length = pRouteResult->EndResultLink.LinkDist; // pCandidateLink->distReal;
#if defined(USE_PEDESTRIAN_DATA)
		currentLinkInfo.time = GetTravelTime(&pRouteInfo->reqInfo, pLink, SPEED_NOT_AVALABLE, 0, currentLinkInfo.length, pCandidateLink->time);
#else
		currentLinkInfo.time = min(1, (int)(pCandidateLink->time * (pRouteResult->EndResultLink.LinkDist / pCandidateLink->dist))); // 종료 링크시간은 사용된 거리대비 시간만큼만 사용.
#endif
		currentLinkInfo.vtx_off += currentLinkInfo.vtx_cnt;
		currentLinkInfo.vtx_cnt = pRouteResult->EndResultLink.LinkVtx.size();
		//currentLinkInfo.rlength = remainDist;
		//currentLinkInfo.rtime = remainTime;

		// angle
		targetDir = pCandidateLink->dir;
		angEnd = getAngle(pLink, targetDir, false);

		currentLinkInfo.angle = getEntryAngle(angStart, angEnd);
		angStart = getAngle(pLink, targetDir);

		if (pCandidateLink->dir == 1) { // 정 to enode
			currentLinkInfo.dir = 0;
		} else { // 역 to snode
			currentLinkInfo.dir = 1;
		}
#if defined(USE_VEHICLE_DATA)
		currentLinkInfo.speed = pCandidateLink->speed;
		currentLinkInfo.speed_type = pCandidateLink->speed_type;
		currentLinkInfo.speed_level = pCandidateLink->speed_level;
#endif

		// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
		if (pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT) {
			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_WAYPOINT; //2:V경유지링크
		} else if (pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DESTINATION) {
			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DESTINATION; // 3:E도착지링크
		} else {
			// 잘못된 링크 속성
			LOG_TRACE(LOG_WARNING, "first link guild type not correct, %d", pRouteInfo->EndLinkInfo.LinkGuideType);
			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DESTINATION; // 3:E도착지링크
		}

		// add link
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

	// set speed
	GetRouteTravelSpeed(pRouteResult);

	return 0;
}


//const int CRoutePlan::MakeRouteResultEx(IN const RouteInfo* pRouteInfo/*, IN const vector<ComplexPointInfo>* vtComplexPointInfo*/, IN const int32_t idx, IN const int32_t bestIdx, OUT RouteResultInfo* pRouteResult)
//{
//	if (pRouteInfo == nullptr || pRouteResult == nullptr || /*vtComplexPointInfo == nullptr ||*/ idx < 0 || bestIdx < 0) {
//		LOG_TRACE(LOG_ERROR, "Failed, input param null, pRouteInfo:%p, pRouteResult:%p, idx:%d, bestIdx:%d", pRouteInfo, pRouteResult, /*vtComplexPointInfo,*/ idx, bestIdx);
//
//		return ROUTE_RESULT_FAILED;
//	}
//
//	pRouteResult->ResultCode = ROUTE_RESULT_SUCCESS;
//
//	pRouteResult->RequestMode = pRouteInfo->RequestMode;
//	pRouteResult->RequestId = pRouteInfo->RequestId;
//
//	pRouteResult->RouteBox.Xmin = pRouteResult->RouteBox.Ymin = INT_MAX;
//	pRouteResult->RouteBox.Xmax = pRouteResult->RouteBox.Ymax = INT_MIN;
//
//	MultimodalPointInfo* pPrevCpxPointInfo = const_cast<MultimodalPointInfo*>(&pRouteInfo->at(idx).ComplexPointInfo);
//	MultimodalPointInfo* pNextCpxPointInfo = const_cast<MultimodalPointInfo*>(&pRouteInfo->at(idx + 1).ComplexPointInfo);
//
//	const size_t cntLink = pPrevCpxPointInfo->vtRoutePathInfo[bestIdx]->depth + pNextCpxPointInfo->vtRoutePathInfo[bestIdx]->depth + 2;
//	pRouteResult->LinkInfo.reserve(cntLink);
//
//	float remainDist = 0.f;
//	float remainTime = 0.f;
//
//	// 전체
//	pRouteResult->TotalLinkCount = cntLink;
//
//	// 경로 옵션
//	pRouteResult->RouteOption = pRouteInfo->RouteOption;
//	pRouteResult->RouteAvoid = pRouteInfo->AvoidOption;
//	pRouteResult->RouteMobility = pRouteInfo->MobilityOption;
//
//	// 경탐 좌표
//	pRouteResult->StartResultLink.Coord = pRouteInfo->StartLinkInfo.Coord;
//	pRouteResult->EndResultLink.Coord = pRouteInfo->EndLinkInfo.Coord;
//
//	// Data Type
//	pRouteResult->StartResultLink.LinkDataType = pRouteInfo->StartLinkInfo.LinkDataType;
//	pRouteResult->EndResultLink.LinkDataType = pRouteInfo->EndLinkInfo.LinkDataType;
//
//	stLinkInfo* pCurrLink = nullptr;
//	stLinkInfo* pPrevLink = nullptr;
//	CandidateLink* pCandidateLink = pPrevCpxPointInfo->vtRoutePathInfo[bestIdx];
//
//	// 링크 진행 각도 계산용 정/역
//	int prevDir = 0;
//	int nextDir = 0;
//	int angStart = 0;
//	int angEnd = 0;
//	int targetDir = 0;
//
//	RouteResultLinkEx currentLinkInfo = { 0, };
//
//	for (int ii = cntLink - 1; ii >= 0; --ii)
//	{
//		if (pCandidateLink == nullptr) {
//			// 첫번째 경로가 완료되면 다음 경로를 진행
//			if (0 <= ii && ii == pNextCpxPointInfo->vtRoutePathInfo[bestIdx]->depth) {
//				
//				// 다음 경로 결과 처리
//				pCandidateLink = pNextCpxPointInfo->vtRoutePathInfo[bestIdx];
//
//				// 도착지(입구점)에서 출발지까지의 누적 정보이므로 뒤집어서 재 설정
//				std::reverse(pRouteResult->LinkInfo.begin(), pRouteResult->LinkInfo.end());
//
//				// 현재 선형은 방향성(출발점 위치기준)을 따져 뒤집거나 그대로 쓰거나
//				if (pRouteInfo->StartLinkInfo.MatchCoord != pRouteResult->LinkVertex[0]) {
//					std::reverse(pRouteResult->LinkVertex.begin(), pRouteResult->LinkVertex.end());
//				}
//			}
//			else {
//				LOG_TRACE(LOG_ERROR, "Failed, can't match rusult link cnt(%d) vs depth(ii)", cntLink, ii);
//				return ROUTE_RESULT_FAILED;
//				break;
//			}
//		}
//
//		pCurrLink = m_pDataMgr->GetLinkDataById(pCandidateLink->linkId, pCandidateLink->linkDataType);
//		nextDir = pCandidateLink->dir;
//		if (pCandidateLink->pPrevLink != nullptr) {
//			pPrevLink = m_pDataMgr->GetLinkDataById(pCandidateLink->pPrevLink->linkId, pCandidateLink->pPrevLink->linkDataType);
//			prevDir = pCandidateLink->pPrevLink->dir;
//		} else {
//			pPrevLink = nullptr;
//		}
//
//		if (pCurrLink == nullptr) {
//			LOG_TRACE(LOG_ERROR, "Failed, can't find route link idx(%d), tile:%d, id:%d, type:%d", ii, pCandidateLink->linkId.tile_id, pCandidateLink->linkId.nid, pCandidateLink->linkDataType);
//			return ROUTE_RESULT_FAILED;
//		}
//
//		currentLinkInfo.link_id = pCandidateLink->linkId;
//		currentLinkInfo.node_id = pCandidateLink->nodeId;
//		currentLinkInfo.link_info = pCurrLink->sub_info;
//		currentLinkInfo.vtx_off += currentLinkInfo.vtx_cnt;
//		//currentLinkInfo.rlength = remainDist;
//		//currentLinkInfo.rtime = remainTime;
//
//		// angle
//		// 주행 각도는 링크 시작값으로 주기에 최초 링크는 각도 무시.
//		targetDir = pCandidateLink->dir;
//
//		if (pPrevLink != nullptr) {
//			angStart = getAngle(pPrevLink, pCandidateLink->pPrevLink->dir, false);
//		} else {
//			angStart = 0;
//		}
//		angEnd = getAngle(pCurrLink, targetDir, false);
//		currentLinkInfo.angle = abs(180 - angStart + angEnd + 360) % 360;
//
//		if (targetDir == 1) { // 정, to enode
//			currentLinkInfo.dir = 0;
//		} else { // 역, to snode
//			currentLinkInfo.dir = 1;
//		}
//
//		if (ii > 0 && pCandidateLink->depth == 0) { // 시작
//			pRouteResult->StartResultLink.LinkId = pCurrLink->link_id;
//			pRouteResult->StartResultLink.LinkDist = pCurrLink->length;
//
//			pRouteResult->StartResultLink.MatchCoord = pRouteInfo->StartLinkInfo.MatchCoord;
//			pRouteResult->StartResultLink.LinkSplitIdx = pRouteInfo->StartLinkInfo.LinkSplitIdx;
//
//			// 경유지는 목적지 기준으로만 생성했기에, 출발지 확인에서는 방향성을 반대로 확인해야 함.
//			// e노드로 나간 경우, 정 - 출발지인 경우
//			// s노드로 나간 경우, 역 - 경유지인 경우
//			if (((pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT) && pCandidateLink->dir == 2) ||
//				((pRouteInfo->StartLinkInfo.LinkGuideType != LINK_GUIDE_TYPE_WAYPOINT) && pCandidateLink->dir == 1))
//			{
//				pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE;
//				pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToE.begin(), pRouteInfo->StartLinkInfo.LinkVtxToE.end());
//			}
//			// s노드로 나간 경우, 역
//			// e노드로 나간 경우, 정 - 경유지인 경우
//			else //if (pCandidateLink->dir == 2)
//			{
//				pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToS;
//				pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToS.begin(), pRouteInfo->StartLinkInfo.LinkVtxToS.end());
//			}
//
//			// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
//			if (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DEPARTURE) {
//				currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEPARTURE; //1:S출발지링크
//			} else if (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT) {
//				currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEFAULT;// LINK_GUIDE_TYPE_WAYPOINT; //경유지부터의 출발은 표시하지 않는다
//			} else {
//				// 잘못된 링크 속성
//				LOG_TRACE(LOG_WARNING, "first link guild type not correct, %d", pRouteInfo->StartLinkInfo.LinkGuideType);
//				currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEPARTURE; //1:S출발지링크
//			}
//			currentLinkInfo.vtx_cnt = pRouteResult->StartResultLink.LinkVtx.size();
//
//			currentLinkInfo.length = pRouteResult->StartResultLink.LinkDist;
//			currentLinkInfo.time = min(1, (int)(pCandidateLink->timeReal * (pRouteResult->StartResultLink.LinkDist / pCandidateLink->distReal))); // pCandidateLink->timeReal;
//
//			linkMerge(pRouteResult->LinkVertex, pRouteResult->StartResultLink.LinkVtx);
//		}
//		else if (ii == 0 && pCandidateLink->depth == 0) { // 종료
//			pRouteResult->EndResultLink.MatchCoord = pRouteInfo->EndLinkInfo.MatchCoord;
//			pRouteResult->EndResultLink.LinkSplitIdx = pRouteInfo->EndLinkInfo.LinkSplitIdx;
//			// s노드로 들어온 경우
//			//if (pCandidateLink->linkId.dir == 1)
//			if (pCandidateLink->dir == 1)
//			{
//				pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToS;
//				pRouteResult->EndResultLink.LinkVtx.assign(pRouteInfo->EndLinkInfo.LinkVtxToS.begin(), pRouteInfo->EndLinkInfo.LinkVtxToS.end());
//			}
//			// e노드로 들어온 경우
//			/*else if (pCandidateLink->linkId.dir == 2)*/
//			else //if (pCandidateLink->dir == 2)
//			{
//				pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToE;
//				pRouteResult->EndResultLink.LinkVtx.assign(pRouteInfo->EndLinkInfo.LinkVtxToE.begin(), pRouteInfo->EndLinkInfo.LinkVtxToE.end());
//			}
//
//			// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
//			if (pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT) {
//				currentLinkInfo.guide_type = LINK_GUIDE_TYPE_WAYPOINT; //2:V경유지링크
//			} else if (pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DESTINATION) {
//				currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DESTINATION; // 3:E도착지링크
//			} else {
//				// 잘못된 링크 속성
//				LOG_TRACE(LOG_WARNING, "first link guild type not correct, %d", pRouteInfo->EndLinkInfo.LinkGuideType);
//				currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DESTINATION; // 3:E도착지링크
//			}
//			currentLinkInfo.vtx_cnt = pRouteResult->EndResultLink.LinkVtx.size();
//
//			currentLinkInfo.length = pRouteResult->EndResultLink.LinkDist;
//			currentLinkInfo.time = min(1, (int)(pCandidateLink->timeReal * (pRouteResult->EndResultLink.LinkDist / pCandidateLink->distReal))); // pCandidateLink->timeReal;
//
//			linkMerge(pRouteResult->LinkVertex, pRouteResult->EndResultLink.LinkVtx);
//		}
//		else { // 중간			
//			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEFAULT;// 일반
//			currentLinkInfo.vtx_cnt = pCurrLink->getVertexCount();
//
//			currentLinkInfo.length = pCandidateLink->distReal;
//			currentLinkInfo.time = pCandidateLink->timeReal;
//
//			linkMerge(pRouteResult->LinkVertex, pCurrLink->getVertex(), pCurrLink->getVertexCount());
//		}
//
//		// add link
//		pRouteResult->LinkInfo.emplace_back(currentLinkInfo);
//
//		pRouteResult->TotalLinkDist += currentLinkInfo.length;
//		pRouteResult->TotalLinkTime += currentLinkInfo.time;
//
//		// 이전 링크
//		pCandidateLink = pCandidateLink->pPrevLink;
//	} // for
//
//
//	// 남은 거리/시간 정보
//	remainDist = pRouteResult->TotalLinkDist;
//	remainTime = pRouteResult->TotalLinkTime;
//
//	for (auto & item : pRouteResult->LinkInfo) {
//		item.rlength = remainDist;
//		item.rtime = remainTime;
//
//		remainDist -= item.length;
//		remainTime -= item.time;
//
//		if (remainDist <= 0) {
//			remainDist = 0;
//		}
//
//		if (remainTime <= 0) {
//			remainTime = 0;
//		}
//	}
//
//	// route box
//	for (vector<SPoint>::const_iterator it = pRouteResult->LinkVertex.begin(); it != pRouteResult->LinkVertex.end(); it++)
//	{
//		if (it->x < pRouteResult->RouteBox.Xmin || pRouteResult->RouteBox.Xmin == 0) { pRouteResult->RouteBox.Xmin = it->x; }
//		if (pRouteResult->RouteBox.Xmax < it->x || pRouteResult->RouteBox.Xmax == 0) { pRouteResult->RouteBox.Xmax = it->x; }
//		if (it->y < pRouteResult->RouteBox.Ymin || pRouteResult->RouteBox.Ymin == 0) { pRouteResult->RouteBox.Ymin = it->y; }
//		if (pRouteResult->RouteBox.Ymax < it->y || pRouteResult->RouteBox.Ymax == 0) { pRouteResult->RouteBox.Ymax = it->y; }
//	}
//
//	return 0;
//}


//const int CRoutePlan::MakeRouteResultAttatch(IN const RouteInfo* pRouteInfo, IN const CandidateLink* pCurrResult, IN const bool isFromPed, OUT RouteResultInfo* pRouteResult)
//{
//	if (pRouteInfo == nullptr || pRouteResult == nullptr || pCurrResult == nullptr) {
//		LOG_TRACE(LOG_ERROR, "Failed, input param null, pRouteInfo:%p, pCurrResult:%p, pRouteResult:%p", pRouteInfo, pCurrResult, pRouteResult);
//
//		return ROUTE_RESULT_FAILED;
//	}
//
//	pRouteResult->ResultCode = ROUTE_RESULT_SUCCESS;
//
//	pRouteResult->RequestMode = pRouteInfo->RequestMode;
//	pRouteResult->RequestId = pRouteInfo->RequestId;
//
//	pRouteResult->RouteBox.Xmin = pRouteResult->RouteBox.Ymin = INT_MAX;
//	pRouteResult->RouteBox.Xmax = pRouteResult->RouteBox.Ymax = INT_MIN;
//
//	const size_t cntLink = pCurrResult->depth + 1;
//	pRouteResult->LinkInfo.reserve(pRouteResult->LinkInfo.size() + cntLink);
//
//	float remainDist = 0.f;
//	float remainTime = 0.f;
//
//	// 전체
//	pRouteResult->TotalLinkCount += cntLink;
//
//	// 경로 옵션
//	pRouteResult->RouteOption = pRouteInfo->RouteOption;
//	pRouteResult->RouteAvoid = pRouteInfo->AvoidOption;
//
//	// 경탐 좌표
//	pRouteResult->StartResultLink.Coord = pRouteInfo->StartLinkInfo.Coord;
//	pRouteResult->EndResultLink.Coord = pRouteInfo->EndLinkInfo.Coord;
//
//	// Data Type
//	pRouteResult->StartResultLink.LinkDataType = pRouteInfo->StartLinkInfo.LinkDataType;
//	pRouteResult->EndResultLink.LinkDataType = pRouteInfo->EndLinkInfo.LinkDataType;
//
//	stLinkInfo* pCurrLink = nullptr;
//	stLinkInfo* pPrevLink = nullptr;
//
//	// 링크 진행 각도 계산용 정/역
//	int prevDir = 0;
//	int nextDir = 0;
//	int prevAng = 0;
//	int nextAng = 0;
//
//	RouteResultLinkEx currentLinkInfo = { 0, };
//
//	currentLinkInfo.vtx_off = pRouteResult->LinkVertex.size();
//
//	// 한쪽이 노드인 경우, 노드를 향한 경로 순서가 정해지므로 노드가 시작 지점인 경우 순서를 바꿔서 진행하자
//	// 링크 순서를 무조건 시작에서 종료 순으로 정리하자
//	CandidateLink* pCandidateLink = const_cast<CandidateLink*>(pCurrResult);
//	vector<CandidateLink*> vtCandidateRouteResult(cntLink);
//	if ((pRouteInfo->StartLinkInfo.KeyType != pRouteInfo->EndLinkInfo.KeyType) && (pRouteInfo->StartLinkInfo.KeyType == TYPE_KEY_NODE)) {
//		for (int ii = 0; ii < cntLink; ii++) {
//			vtCandidateRouteResult[ii] = pCandidateLink;
//			pCandidateLink = pCandidateLink->pPrevLink;
//		}
//	}
//	else {
//		for (int ii = cntLink - 1; ii >= 0; --ii) {
//			vtCandidateRouteResult[ii] = pCandidateLink;
//			pCandidateLink = pCandidateLink->pPrevLink;
//		}
//	}
//
//	for (int ii = 0; ii < cntLink; ii++)
//	{
//		pCandidateLink = vtCandidateRouteResult[ii];
//
//		pCurrLink = m_pDataMgr->GetLinkDataById(pCandidateLink->linkId, pCandidateLink->linkDataType);
//		nextDir = pCandidateLink->dir;
//		if (pCandidateLink->pPrevLink != nullptr) {
//			pPrevLink = m_pDataMgr->GetLinkDataById(pCandidateLink->pPrevLink->linkId, pCandidateLink->pPrevLink->linkDataType);
//			prevDir = pCandidateLink->pPrevLink->dir;
//		}
//		else {
//			pPrevLink = nullptr;
//		}
//
//		if (pCurrLink == nullptr) {
//			LOG_TRACE(LOG_ERROR, "Failed, can't find route link idx(%d), tile:%d, id:%d, type:%d", ii, pCandidateLink->linkId.tile_id, pCandidateLink->linkId.nid, pCandidateLink->linkDataType);
//			return ROUTE_RESULT_FAILED;
//		}
//
//		currentLinkInfo.link_id = pCandidateLink->linkId;
//		currentLinkInfo.node_id = pCandidateLink->nodeId;
//		currentLinkInfo.link_info = pCurrLink->sub_info;
//		currentLinkInfo.length = pCandidateLink->distReal;
//		currentLinkInfo.time = pCandidateLink->timeReal;
//		currentLinkInfo.vtx_off += currentLinkInfo.vtx_cnt;
//		//currentLinkInfo.rlength = remainDist;
//		//currentLinkInfo.rtime = remainTime;
//
//		// angle
//
//		// 주행 각도는 링크 시작값으로 주기에 최초 링크는 각도 무시.
//		//currentLinkInfo.angle = abs(prevDir - tmpAng) % 360;
//		//currentLinkInfo.angle = abs(prevDir - tmpAng - 180) % 360; 
//		if (pPrevLink != nullptr) {
//			prevAng = getAngle(pPrevLink, pCandidateLink->pPrevLink->dir);
//		}
//		else {
//			prevAng = 0;
//		}
//		nextAng = getAngle(pCurrLink, pCandidateLink->dir, false);
//		currentLinkInfo.angle = abs(180 - prevAng + nextAng + 360) % 360;
//
//		if (pCandidateLink->dir == 1) { // 정, to enode
//			currentLinkInfo.dir = 0;
//		}
//		else { // 역, to snode
//			currentLinkInfo.dir = 1;
//		}
//
//		//if (ii == cntLink - 1) { // 시작, 위에서 끝->시작을 바꿨기 때문에 마지막이 시작이 됨
//		if (ii == 0) {
//			pRouteResult->StartResultLink.LinkId = pCurrLink->link_id;
//			pRouteResult->StartResultLink.LinkDist = pCurrLink->length;
//
//			if (pRouteInfo->StartLinkInfo.KeyType == TYPE_KEY_NODE) { // node type)
//				//if (pCandidateLink->dir == 1) { // 정
//				if ((isFromPed && (pCandidateLink->dir == 1)) || // 정 
//					(!isFromPed && (pCandidateLink->dir == 2) && (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT)))
//				{
//					//if (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT) {
//					//	// 경로의 최종 종착지는 최초 시작지점으로 유지되어야 하기에 순서상 항상 출발지점이 되어야 함
//					//	vector<SPoint>tmpVtx;
//					//	tmpVtx.assign(pCurrLink->getVertex(), pCurrLink->getVertex() + pCurrLink->cntVtx);
//					//	pRouteResult->StartResultLink.LinkVtx.assign(tmpVtx.rbegin(), tmpVtx.rend()); // 역순 복사
//					//	//std::reverse(pRouteResult->StartResultLink.LinkVtx.begin(), pRouteResult->StartResultLink.LinkVtx.end());
//					//}
//					//else 
//					{
//						pRouteResult->StartResultLink.LinkVtx.assign(pCurrLink->getVertex(), pCurrLink->getVertex() + pCurrLink->cntVtx);
//					}
//				}
//				else { // 역
//					//if (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT) {
//					//	pRouteResult->StartResultLink.LinkVtx.assign(pCurrLink->getVertex(), pCurrLink->getVertex() + pCurrLink->cntVtx);
//					//} else 
//					{
//						// 경로의 최종 종착지는 최초 시작지점으로 유지되어야 하기에 순서상 항상 출발지점이 되어야 함
//						vector<SPoint>tmpVtx;
//						tmpVtx.assign(pCurrLink->getVertex(), pCurrLink->getVertex() + pCurrLink->cntVtx);
//						pRouteResult->StartResultLink.LinkVtx.assign(tmpVtx.rbegin(), tmpVtx.rend()); // 역순 복사
//						//std::reverse(pRouteResult->StartResultLink.LinkVtx.begin(), pRouteResult->StartResultLink.LinkVtx.end());
//					}
//				}				
//			}
//			else {
//				pRouteResult->StartResultLink.MatchCoord = pRouteInfo->StartLinkInfo.MatchCoord;
//				pRouteResult->StartResultLink.LinkSplitIdx = pRouteInfo->StartLinkInfo.LinkSplitIdx;
//
//				// 경유지는 목적지 기준으로만 생성했기에, 출발지 확인에서는 방향성을 반대로 확인해야 함.
//				// e노드로 나간 경우, 정 - 출발지인 경우
//				// s노드로 나간 경우, 역 - 경유지인 경우
//				if ((isFromPed && (pCandidateLink->dir == 1)) || // 정 
//					(!isFromPed && (pCandidateLink->dir == 2) && (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT)))
//				{
//					pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE;
//					//if (!isFromPed && pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT) {
//					//	vector<SPoint>tmpVtx;
//					//	tmpVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToE.begin(), pRouteInfo->StartLinkInfo.LinkVtxToE.end());
//					//	pRouteResult->StartResultLink.LinkVtx.assign(tmpVtx.rbegin(), tmpVtx.rend()); // 역순 복사
//					//}
//					//else {
//						pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToE.begin(), pRouteInfo -> StartLinkInfo.LinkVtxToE.end());
//					//}
//				}
//				// s노드로 나간 경우, 역
//				// e노드로 나간 경우, 정 - 경유지인 경우
//				else //if (pCandidateLink->dir == 2)
//				{
//					pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToS;
//					//if (!isFromPed && (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT)) {
//					//	vector<SPoint>tmpVtx;
//					//	tmpVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToS.begin(), pRouteInfo->StartLinkInfo.LinkVtxToS.end());
//					//	pRouteResult->StartResultLink.LinkVtx.assign(tmpVtx.rbegin(), tmpVtx.rend()); // 역순 복사
//					//}
//					//else {
//						pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToS.begin(), pRouteInfo->StartLinkInfo.LinkVtxToS.end());
//					//}
//				}
//			}
//
//			// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
//			if (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DEPARTURE) {
//				currentLinkInfo.type = LINK_GUIDE_TYPE_DEPARTURE; //1:S출발지링크
//			}
//			else if (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT) {
//				currentLinkInfo.type = LINK_GUIDE_TYPE_DEFAULT;// LINK_GUIDE_TYPE_WAYPOINT; //경유지부터의 출발은 표시하지 않는다
//			}
//			else {
//				// 잘못된 링크 속성
//				LOG_TRACE(LOG_WARNING, "first link guild type not correct, %d", pRouteInfo->StartLinkInfo.LinkGuideType);
//				currentLinkInfo.type = LINK_GUIDE_TYPE_DEPARTURE; //1:S출발지링크
//			}
//			currentLinkInfo.vtx_cnt = pRouteResult->StartResultLink.LinkVtx.size();
//			
//			linkMerge(pRouteResult->LinkVertex, pRouteResult->StartResultLink.LinkVtx);
//		}
//		//else if (ii == 0) { // 종료, 위에서 끝->시작을 바꿨기 때문에 처음이 끝이 됨
//		else if (ii == cntLink - 1) {
//			pRouteResult->EndResultLink.LinkId = pCurrLink->link_id;
//			pRouteResult->EndResultLink.LinkDist = pCurrLink->length;
//
//			if (pRouteInfo->EndLinkInfo.KeyType == TYPE_KEY_NODE) { // node type)
//				if (pCandidateLink->dir == 1) { // 정
//					// 마지막 점
//					pRouteResult->EndResultLink.MatchCoord.x = pCurrLink->getVertexX(pCurrLink->cntVtx - 1);
//					pRouteResult->EndResultLink.MatchCoord.y = pCurrLink->getVertexY(pCurrLink->cntVtx - 1);
//					pRouteResult->EndResultLink.LinkSplitIdx = pCurrLink->cntVtx - 1;
//					pRouteResult->EndResultLink.LinkVtx.assign(pCurrLink->getVertex(), pCurrLink->getVertex() + pCurrLink->cntVtx);
//				}
//				else { // 역
//					pRouteResult->EndResultLink.MatchCoord.x = pCurrLink->getVertexX(0);
//					pRouteResult->EndResultLink.MatchCoord.y = pCurrLink->getVertexY(0);
//					pRouteResult->EndResultLink.LinkSplitIdx = 0;
//					pRouteResult->EndResultLink.LinkVtx.assign(pCurrLink->getVertex(), pCurrLink->getVertex() + pCurrLink->cntVtx);
//				}
//			}
//			else {
//				pRouteResult->EndResultLink.MatchCoord = pRouteInfo->EndLinkInfo.MatchCoord;
//				pRouteResult->EndResultLink.LinkSplitIdx = pRouteInfo->EndLinkInfo.LinkSplitIdx;
//				// s노드로 들어온 경우
//				if (pCandidateLink->dir == 1)
//				{
//					pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToS;
//					pRouteResult->EndResultLink.LinkVtx.assign(pRouteInfo->EndLinkInfo.LinkVtxToS.begin(), pRouteInfo->EndLinkInfo.LinkVtxToS.end());
//				}
//				// e노드로 들어온 경우
//				else //if (pCandidateLink->dir == 2)
//				{
//					pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToE;
//					pRouteResult->EndResultLink.LinkVtx.assign(pRouteInfo->EndLinkInfo.LinkVtxToE.begin(), pRouteInfo->EndLinkInfo.LinkVtxToE.end());
//				}
//			}
//
//			// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
//			if (pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT) {
//				currentLinkInfo.type = LINK_GUIDE_TYPE_WAYPOINT; //2:V경유지링크
//			}
//			else if (pRouteInfo->EndLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DESTINATION) {
//				currentLinkInfo.type = LINK_GUIDE_TYPE_DESTINATION; // 3:E도착지링크
//			}
//			else {
//				// 잘못된 링크 속성
//				LOG_TRACE(LOG_WARNING, "first link guild type not correct, %d", pRouteInfo->EndLinkInfo.LinkGuideType);
//				currentLinkInfo.type = LINK_GUIDE_TYPE_DESTINATION; // 3:E도착지링크
//			}
//			currentLinkInfo.vtx_cnt = pRouteResult->EndResultLink.LinkVtx.size();
//
//			linkMerge(pRouteResult->LinkVertex, pRouteResult->EndResultLink.LinkVtx);
//		}
//		else { // 중간			
//			currentLinkInfo.type = LINK_GUIDE_TYPE_DEFAULT;// 일반
//			currentLinkInfo.vtx_cnt = pCurrLink->getVertexCount();
//
//			linkMerge(pRouteResult->LinkVertex, pCurrLink->getVertex(), pCurrLink->getVertexCount());
//		}
//
//		// add link
//		pRouteResult->LinkInfo.emplace_back(currentLinkInfo);
//
//		pRouteResult->TotalLinkDist += currentLinkInfo.length;
//		pRouteResult->TotalLinkTime += currentLinkInfo.time;
//
//		// 이전 링크
//		//pCandidateLink = pCandidateLink->pPrevLink;
//	} // for
//
//
//	  // 남은 거리/시간 정보
//	remainDist = pRouteResult->TotalLinkDist;
//	remainTime = pRouteResult->TotalLinkTime;
//
//	for (auto & item : pRouteResult->LinkInfo) {
//		item.rlength = remainDist;
//		item.rtime = remainTime;
//
//		remainDist -= item.length;
//		remainTime -= item.time;
//
//		if (remainDist <= 0) {
//			remainDist = 0;
//		}
//
//		if (remainTime <= 0) {
//			remainTime = 0;
//		}
//	}
//
//	// route box
//	for (vector<SPoint>::const_iterator it = pRouteResult->LinkVertex.begin(); it != pRouteResult->LinkVertex.end(); it++)
//	{
//		if (it->x < pRouteResult->RouteBox.Xmin || pRouteResult->RouteBox.Xmin == 0) { pRouteResult->RouteBox.Xmin = it->x; }
//		if (pRouteResult->RouteBox.Xmax < it->x || pRouteResult->RouteBox.Xmax == 0) { pRouteResult->RouteBox.Xmax = it->x; }
//		if (it->y < pRouteResult->RouteBox.Ymin || pRouteResult->RouteBox.Ymin == 0) { pRouteResult->RouteBox.Ymin = it->y; }
//		if (pRouteResult->RouteBox.Ymax < it->y || pRouteResult->RouteBox.Ymax == 0) { pRouteResult->RouteBox.Ymax = it->y; }
//	}
//
//	return 0;
//}


const int CRoutePlan::MakeRouteResultAttatchEx(IN const RouteInfo* pRouteInfo, IN const bool isReverse, IN const int32_t typeStartKey, IN const int32_t typeEndKey, IN const CandidateLink* pCurrResult, IN const int32_t nGuideType, OUT RouteResultInfo* pRouteResult)
{
	if (pRouteInfo == nullptr || pRouteResult == nullptr || pCurrResult == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, pStartInfo:%p, pEndInfo:%p, pCurrResult:%p, pRouteResult:%p", pRouteInfo, pCurrResult, pRouteResult);

		return ROUTE_RESULT_FAILED;
	}
	
	int ret = ROUTE_RESULT_SUCCESS;

	const size_t cntLink = pCurrResult->depth + 1;
	pRouteResult->LinkInfo.reserve(pRouteResult->LinkInfo.size() + cntLink);

	float remainDist = 0.f;
	float remainTime = 0.f;

	//bool isReverse = false;
	//if ((typeStartKey != typeEndKey) &&
	//	((typeStartKey == TYPE_KEY_NODE) ||
	//		((typeStartKey == TYPE_KEY_LINK) && (pCurrResult->linkDataType == TYPE_LINK_DATA_TREKKING) && (nGuideType != LINK_GUIDE_TYPE_DEPARTURE)))) {
	//	isReverse = true;
	//}

	// 전체
	pRouteResult->TotalLinkCount += cntLink;

	stLinkInfo* pCurrLink = nullptr;
	stLinkInfo* pPrevLink = nullptr;

	// 링크 진행 각도 계산용 정/역
	int prevDir = 0;
	int nextDir = 0;
	int angStart = 0;
	int angEnd = 0;
	int targetDir = 0;

	RouteResultLinkEx currentLinkInfo = { 0, };

	currentLinkInfo.vtx_off = pRouteResult->LinkVertex.size();

	// 한쪽이 노드인 경우, 노드를 향한 경로 순서가 정해지므로 노드가 시작 지점인 경우 순서를 바꿔서 진행하자
	// 링크 순서를 무조건 시작에서 종료 순으로 정리하자
	CandidateLink* pCandidateLink = const_cast<CandidateLink*>(pCurrResult);
	vector<CandidateLink*> vtCandidateRouteResult(cntLink);
	if ((typeStartKey != typeEndKey) && (typeStartKey == TYPE_KEY_NODE)) {
		for (int ii = 0; ii < cntLink; ii++) {
			vtCandidateRouteResult[ii] = pCandidateLink;
			pCandidateLink = pCandidateLink->pPrevLink;
		}
	}
	else {
		for (int ii = cntLink - 1; ii >= 0; --ii) {
			vtCandidateRouteResult[ii] = pCandidateLink;
			pCandidateLink = pCandidateLink->pPrevLink;
		}
	}


	for (int ii = 0; ii < cntLink; ii++)
	{
		pCandidateLink = vtCandidateRouteResult[ii];

		pCurrLink = m_pDataMgr->GetLinkDataById(pCandidateLink->linkId, pCandidateLink->linkDataType);
		nextDir = pCandidateLink->dir;
		if (pCandidateLink->pPrevLink != nullptr) {
			pPrevLink = m_pDataMgr->GetLinkDataById(pCandidateLink->pPrevLink->linkId, pCandidateLink->pPrevLink->linkDataType);
			prevDir = pCandidateLink->pPrevLink->dir;
		}
		else {
			pPrevLink = nullptr;
		}

		if (pCurrLink == nullptr) {
			LOG_TRACE(LOG_ERROR, "Failed, can't find route link idx(%d), tile:%d, id:%d, type:%d", ii, pCandidateLink->linkId.tile_id, pCandidateLink->linkId.nid, pCandidateLink->linkDataType);
			return ROUTE_RESULT_FAILED;
		}

		currentLinkInfo.link_id = pCandidateLink->linkId;
		currentLinkInfo.node_id = pCandidateLink->nodeId;
		currentLinkInfo.link_info = pCurrLink->sub_info;
		currentLinkInfo.length = pCandidateLink->dist;
#if defined(USE_PEDESTRIAN_DATA)
		currentLinkInfo.time = GetTravelTime(&pRouteInfo->reqInfo, pCurrLink, SPEED_NOT_AVALABLE, 0, currentLinkInfo.length, pCandidateLink->time);
#else
		currentLinkInfo.time = pCandidateLink->time;
#endif
		currentLinkInfo.vtx_off += currentLinkInfo.vtx_cnt;

		// angle
		// 주행 각도는 링크 시작값으로 주기에 최초 링크는 각도 무시.
		targetDir = pCandidateLink->dir;

		if (isReverse) {
			angEnd = getAngle(pCurrLink, targetDir, false);
		}
		else {
			//if (pPrevLink != nullptr) {
			//	angStart = getAngle(pPrevLink, pCandidateLink->pPrevLink->dir);
			//}
			//else {
			//	angStart = 0;
			//}
			angEnd = getAngle(pCurrLink, targetDir, false);
		}

		currentLinkInfo.angle = getEntryAngle(angStart, angEnd);

		if (isReverse) {
			angStart = getAngle(pCurrLink, targetDir);
		}
		else {
			angStart = getAngle(pCurrLink, targetDir);
		}

		if (targetDir == 1) { // 정, to enode
			currentLinkInfo.dir = 0;
		} else { // 역, to snode
			currentLinkInfo.dir = 1;
		}

		// 단일 링크 경로일 경우
		if (cntLink == 1) {
			if (typeStartKey == TYPE_KEY_NODE || typeEndKey == TYPE_KEY_NODE) { // node type)
				// 첫번째 링크에 출발지점(보행자) 설정된 경우
				if (!isReverse) // && (pCandidateLink->linkId.llid == pRouteInfo->StartLinkInfo.LinkId.llid))
				{
					// e노드로 나간 경우, 정 - 출발지인 경우
					if (pCandidateLink->dir == 1)
					{
						pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE;
						pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToE.begin(), pRouteInfo->StartLinkInfo.LinkVtxToE.end());
					}
					// s노드로 나간 경우, 역
					else //if (pCandidateLink->dir == 2)
					{
						pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToS;
						pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToS.begin(), pRouteInfo->StartLinkInfo.LinkVtxToS.end());
					}

					// 시작점
					pRouteResult->StartResultLink.LinkId = pCurrLink->link_id;
					pRouteResult->StartResultLink.LinkDataType = pRouteInfo->StartLinkInfo.LinkDataType;
					pRouteResult->StartResultLink.MatchCoord = pRouteInfo->StartLinkInfo.MatchCoord; // pRouteResult->StartResultLink.LinkVtx.at(0);
					pRouteResult->StartResultLink.LinkSplitIdx = pRouteInfo->StartLinkInfo.LinkSplitIdx;

					// 종료점 (node)
					pRouteResult->EndResultLink.Init();
				}
				// 첫번째 링크에 출발지점(숲길) 설정된 경우
				else // if (isReverse && (pCandidateLink->linkId.llid == pRouteInfo->EndLinkInfo.LinkId.llid))
				{
					// s노드로 들어온 경우
					if (pCandidateLink->dir == 1)
					{
						pRouteResult->StartResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToS;
						pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->EndLinkInfo.LinkVtxToS.begin(), pRouteInfo->EndLinkInfo.LinkVtxToS.end());
					}
					// e노드로 들어온 경우
					else //if (pCandidateLink->dir == 2)
					{
						pRouteResult->StartResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToE;
						pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->EndLinkInfo.LinkVtxToE.begin(), pRouteInfo->EndLinkInfo.LinkVtxToE.end());
					}

					// 시작점
					pRouteResult->StartResultLink.LinkId = pCurrLink->link_id;
					pRouteResult->StartResultLink.LinkDataType = pRouteInfo->EndLinkInfo.LinkDataType;
					pRouteResult->StartResultLink.MatchCoord = pRouteInfo->EndLinkInfo.MatchCoord;
					pRouteResult->StartResultLink.LinkSplitIdx = pRouteInfo->EndLinkInfo.LinkSplitIdx;

					// 종료점 (node)
					pRouteResult->EndResultLink.Init();
				}
				
				currentLinkInfo.node_id.llid = pCurrLink->enode_id.llid;				
				currentLinkInfo.time = pCandidateLink->time;
			}
			else // if (typeStartKey == TYPE_KEY_NODE)
			{
				// 시작 vtx

				// s노드로 나간 경우, 역
				if (pCandidateLink->dir == 2)
				{
					pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToS - pRouteInfo->EndLinkInfo.LinkDistToS;
					pRouteResult->StartResultLink.LinkVtx.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToS[0]);
				}
				// e노드로 나간 경우, 정
				else
				{
					pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE - pRouteInfo->EndLinkInfo.LinkDistToE;
					pRouteResult->StartResultLink.LinkVtx.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToE[0]);
				}


				// 중간 vtx
				if (pRouteInfo->StartLinkInfo.LinkSplitIdx != pRouteInfo->EndLinkInfo.LinkSplitIdx) {
					// s노드로 나간 경우, 역
					//if (pRouteInfo->StartLinkInfo.LinkDir == 2)
					if (pCandidateLink->dir == 2)
					{
						for (int ii = 1; ii <= pRouteInfo->StartLinkInfo.LinkSplitIdx - pRouteInfo->EndLinkInfo.LinkSplitIdx; ii++) {
							pRouteResult->StartResultLink.LinkVtx.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToS[ii]);
						}
					}
					// e노드로 나간 경우, 정
					//else if (pRouteInfo->StartLinkInfo.LinkDir == 1)
					else
					{
						for (int ii = 1; ii <= pRouteInfo->EndLinkInfo.LinkSplitIdx - pRouteInfo->StartLinkInfo.LinkSplitIdx; ii++) {
							pRouteResult->StartResultLink.LinkVtx.emplace_back(pRouteInfo->StartLinkInfo.LinkVtxToE[ii]);
						}
					}
				}


				// 종료 vtx
				//dir = pRouteInfo->EndLinkInfo.LinkDir;
				// e노드로 들어온 경우 // 역
				if (pCandidateLink->dir == 2)
				{
					// 진행과 반대방향의 시작 vtx
					pRouteResult->StartResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToE - pRouteInfo->StartLinkInfo.LinkDistToE;
					pRouteResult->StartResultLink.LinkVtx.emplace_back(pRouteInfo->EndLinkInfo.LinkVtxToE[pRouteInfo->EndLinkInfo.LinkVtxToE.size() - 1]);
				}
				// s노드로 들어온 경우 // 정
				else // if (dir == 1)
				{
					// 진행과 반대방향의 시작 vtx
					pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE - pRouteInfo->EndLinkInfo.LinkDistToE;
					pRouteResult->StartResultLink.LinkVtx.emplace_back(pRouteInfo->EndLinkInfo.LinkVtxToS[pRouteInfo->EndLinkInfo.LinkVtxToS.size() - 1]);
				}

				// 시작점
				pRouteResult->StartResultLink.LinkId = pRouteInfo->StartLinkInfo.LinkId;
				pRouteResult->StartResultLink.LinkDataType = pRouteInfo->StartLinkInfo.LinkDataType;
				pRouteResult->StartResultLink.MatchCoord = pRouteInfo->StartLinkInfo.MatchCoord;
				pRouteResult->StartResultLink.LinkSplitIdx = pRouteInfo->StartLinkInfo.LinkSplitIdx;

				// 종료점
				pRouteResult->EndResultLink.Init();
				
				currentLinkInfo.node_id.llid = pCurrLink->enode_id.llid;
				currentLinkInfo.time = min(1, (int)(pCandidateLink->time * (pRouteResult->EndResultLink.LinkDist / pCandidateLink->dist)));
			}

			currentLinkInfo.link_id = pRouteResult->StartResultLink.LinkId;

			currentLinkInfo.link_info = pCurrLink->sub_info;
			currentLinkInfo.length = pRouteResult->StartResultLink.LinkDist;
			currentLinkInfo.vtx_cnt = pRouteResult->StartResultLink.LinkVtx.size();
			currentLinkInfo.angle = 0;
			currentLinkInfo.dir = pCandidateLink->dir == 1 ? 0 : 1;			

			linkMerge(pRouteResult->LinkVertex, pRouteResult->StartResultLink.LinkVtx);
		}
		//if (ii == cntLink - 1) { // 시작, 위에서 끝->시작을 바꿨기 때문에 마지막이 시작이 됨
		else if (ii == 0) {
			pRouteResult->StartResultLink.LinkId = pCurrLink->link_id;
			pRouteResult->StartResultLink.LinkDist = pCurrLink->length;

			if (typeStartKey == TYPE_KEY_NODE) { // node type)
				if (pCandidateLink->dir == 1)
				//if ((!isReverse && (pCandidateLink->dir == 1)) || // 정 
				//	(isReverse && (pCandidateLink->dir == 2)))// && (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT)))
				//if (((pCandidateLink->dir == 1) && (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DEPARTURE)) || // 정 
				//	((pCandidateLink->dir == 2) && (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT)))
				{
					pRouteResult->StartResultLink.LinkVtx.assign(pCurrLink->getVertex(), pCurrLink->getVertex() + pCurrLink->cntVtx);
					//pRouteResult->StartResultLink.LinkVtx.assign(pCurrLink->getVertex(), pCurrLink->getVertex() + pCurrLink->cntVtx);
				}
				else { // 역
					{
						// 경로의 최종 종착지는 최초 시작지점으로 유지되어야 하기에 순서상 항상 출발지점이 되어야 함
						vector<SPoint>tmpVtx;
						tmpVtx.assign(pCurrLink->getVertex(), pCurrLink->getVertex() + pCurrLink->cntVtx);
						pRouteResult->StartResultLink.LinkVtx.assign(tmpVtx.rbegin(), tmpVtx.rend()); // 역순 복사
						//std::reverse(pRouteResult->StartResultLink.LinkVtx.begin(), pRouteResult->StartResultLink.LinkVtx.end());
					}
				}

				// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
				currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEFAULT;
			}
			else {
				pRouteResult->StartResultLink.MatchCoord = pRouteInfo->StartLinkInfo.MatchCoord;
				pRouteResult->StartResultLink.LinkSplitIdx = pRouteInfo->StartLinkInfo.LinkSplitIdx;

				// 경유지는 목적지 기준으로만 생성했기에, 출발지 확인에서는 방향성을 반대로 확인해야 함.
				// e노드로 나간 경우, 정 - 출발지인 경우
				// s노드로 들어온 경우, 역 - 경유지인 경우
				if ((!isReverse && (pCandidateLink->dir == 1)) || // 정 
					(isReverse && (pCandidateLink->dir == 2))) //&& (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT)))
				//if ((!isReverse && (pCandidateLink->dir == 1) && (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_DEPARTURE)) || // 정 
				//	(isReverse && (pCandidateLink->dir == 2) && (pRouteInfo->StartLinkInfo.LinkGuideType == LINK_GUIDE_TYPE_WAYPOINT)))
				{
					pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToE;
					pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToE.begin(), pRouteInfo->StartLinkInfo.LinkVtxToE.end());
				}
				// s노드로 나간 경우, 역
				// e노드로 들어온 경우, 정 - 경유지인 경우
				else //if (pCandidateLink->dir == 2)
				{
					pRouteResult->StartResultLink.LinkDist = pRouteInfo->StartLinkInfo.LinkDistToS;
					pRouteResult->StartResultLink.LinkVtx.assign(pRouteInfo->StartLinkInfo.LinkVtxToS.begin(), pRouteInfo->StartLinkInfo.LinkVtxToS.end());
				}

				// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
				currentLinkInfo.guide_type = pRouteInfo->StartLinkInfo.LinkGuideType;
			}

			currentLinkInfo.angle = 0;
			currentLinkInfo.vtx_cnt = pRouteResult->StartResultLink.LinkVtx.size();
			linkMerge(pRouteResult->LinkVertex, pRouteResult->StartResultLink.LinkVtx);
		}
		//else if (ii == 0) { // 종료, 위에서 끝->시작을 바꿨기 때문에 처음이 끝이 됨
		else if (ii == cntLink - 1) {
			pRouteResult->EndResultLink.LinkId = pCurrLink->link_id;
			pRouteResult->EndResultLink.LinkDist = pCurrLink->length;

			if (typeEndKey == TYPE_KEY_NODE) { // node type)
				if (pCandidateLink->dir == 1) { // 정
					// 마지막 점
					pRouteResult->EndResultLink.MatchCoord.x = pCurrLink->getVertexX(pCurrLink->cntVtx - 1);
					pRouteResult->EndResultLink.MatchCoord.y = pCurrLink->getVertexY(pCurrLink->cntVtx - 1);
					pRouteResult->EndResultLink.LinkSplitIdx = pCurrLink->cntVtx - 1;
					pRouteResult->EndResultLink.LinkVtx.assign(pCurrLink->getVertex(), pCurrLink->getVertex() + pCurrLink->cntVtx);
				}
				else { // 역
					pRouteResult->EndResultLink.MatchCoord.x = pCurrLink->getVertexX(0);
					pRouteResult->EndResultLink.MatchCoord.y = pCurrLink->getVertexY(0);
					pRouteResult->EndResultLink.LinkSplitIdx = 0;
					pRouteResult->EndResultLink.LinkVtx.assign(pCurrLink->getVertex(), pCurrLink->getVertex() + pCurrLink->cntVtx);
				}

				// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
				currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEFAULT;
			}
			else {
				pRouteResult->EndResultLink.MatchCoord = pRouteInfo->EndLinkInfo.MatchCoord;
				pRouteResult->EndResultLink.LinkSplitIdx = pRouteInfo->EndLinkInfo.LinkSplitIdx;
				// s노드로 들어온 경우
				if (pCandidateLink->dir == 1) {
					pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToS;
					pRouteResult->EndResultLink.LinkVtx.assign(pRouteInfo->EndLinkInfo.LinkVtxToS.begin(), pRouteInfo->EndLinkInfo.LinkVtxToS.end());
				}
				// e노드로 들어온 경우
				else //if (pCandidateLink->dir == 2)
				{
					pRouteResult->EndResultLink.LinkDist = pRouteInfo->EndLinkInfo.LinkDistToE;
					pRouteResult->EndResultLink.LinkVtx.assign(pRouteInfo->EndLinkInfo.LinkVtxToE.begin(), pRouteInfo->EndLinkInfo.LinkVtxToE.end());
				}

				// 링크 안내 타입, 0:일반, 1:S출발지링크, 2:V경유지링크, 3:E도착지링크, 4:SV출발지-경유지, 5:SE출발지-도착지, 6:VV경유지-경유지, 7:VE경유지-도착지
				currentLinkInfo.guide_type = pRouteInfo->EndLinkInfo.LinkGuideType;
			}
			
			currentLinkInfo.vtx_cnt = pRouteResult->EndResultLink.LinkVtx.size();
			linkMerge(pRouteResult->LinkVertex, pRouteResult->EndResultLink.LinkVtx);
		}
		else { // 중간			
			currentLinkInfo.guide_type = LINK_GUIDE_TYPE_DEFAULT;// 일반
			currentLinkInfo.vtx_cnt = pCurrLink->getVertexCount();
			linkMerge(pRouteResult->LinkVertex, pCurrLink->getVertex(), pCurrLink->getVertexCount());
		}

		// add link
		pRouteResult->LinkInfo.emplace_back(currentLinkInfo);

		pRouteResult->TotalLinkDist += currentLinkInfo.length;
		pRouteResult->TotalLinkTime += currentLinkInfo.time;
	} // for


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
	pRouteResult->RouteBox.Xmin = pRouteResult->RouteBox.Ymin = INT_MAX;
	pRouteResult->RouteBox.Xmax = pRouteResult->RouteBox.Ymax = INT_MIN;

	for (vector<SPoint>::const_iterator it = pRouteResult->LinkVertex.begin(); it != pRouteResult->LinkVertex.end(); it++)
	{
		if (it->x < pRouteResult->RouteBox.Xmin || pRouteResult->RouteBox.Xmin == 0) { pRouteResult->RouteBox.Xmin = it->x; }
		if (pRouteResult->RouteBox.Xmax < it->x || pRouteResult->RouteBox.Xmax == 0) { pRouteResult->RouteBox.Xmax = it->x; }
		if (it->y < pRouteResult->RouteBox.Ymin || pRouteResult->RouteBox.Ymin == 0) { pRouteResult->RouteBox.Ymin = it->y; }
		if (pRouteResult->RouteBox.Ymax < it->y || pRouteResult->RouteBox.Ymax == 0) { pRouteResult->RouteBox.Ymax = it->y; }
	}

	// set speed
	GetRouteTravelSpeed(pRouteResult);

	return ret;
}


// 탐색 가능한 확장 후 종료 노드의 정보 획득 (입구점 등)
const int CRoutePlan::DoEdgeRoute(IN const RequestRouteInfo* pReqInfo, OUT vector<RouteInfo>& vtRouteInfos/*, OUT vector<ComplexPointInfo>& vtComplexPointInfo*/)
{
	uint32_t ret = ROUTE_RESULT_SUCCESS;

	size_t tickEdgeRouteStart = TICK_COUNT();

	if (pReqInfo == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, pReqInfo:%p", pReqInfo);
		return ROUTE_RESULT_FAILED;
	}

	const int cntPoints = pReqInfo->vtPoints.size();
	vector<stOptimalPointInfo> vtOptInfo(cntPoints);

#if 1 // 산 바운더리 오류 있어서 우선 사용 보류
	// 입구점 여부 확인 필요
	int cntEntrance = 0;
	int cntTrekking = 0;

	if (pReqInfo->RouteSubOption.mnt.course_type == TYPE_TRE_HIKING) {
#if defined(USE_MULTIPROCESS)
#pragma omp parallel for
#endif
		for (int ii = 0; ii < cntPoints; ii++) {
			stReqOptimal reqOpt = { 0 };
			stPolygonInfo* pPoly = nullptr;


			reqOpt.x = pReqInfo->vtPoints[ii].x;
			reqOpt.y = pReqInfo->vtPoints[ii].y;
			reqOpt.isExpand = false;

			if (pReqInfo->vtLinkDataType[ii] == TYPE_LINK_DATA_TREKKING) {
				cntTrekking++;

#if defined(USE_MOUNTAIN_DATA)
				reqOpt.type1st = TYPE_OPTIMAL_ENTRANCE_MOUNTAIN;
#else
				reqOpt.type1st = 0;
#endif

				// 동일 산인지 확인하는 checksum 용도 
				if (1) {
					if (m_pDataMgr->GetOptimalPointDataByPoint(reqOpt.x, reqOpt.y, &vtOptInfo[ii], reqOpt.typeAll, 0) > 0) {
						vtRouteInfos[ii].ComplexPointInfo.id = vtOptInfo[ii].id;
						vtRouteInfos[ii].ComplexPointReverseInfo.id = vtOptInfo[ii].id;
						cntEntrance++;
					}

#if defined(USE_MOUNTAIN_DATA)
					// 그룹ID 부여
					stLinkInfo* pLink = m_pDataMgr->GetLinkDataById(pReqInfo->vtKeyId[ii], TYPE_LINK_DATA_TREKKING);
					if (pLink) {
						vtRouteInfos[ii].ComplexPointInfo.nGroupId = pLink->trk_ext.group_id;
						vtRouteInfos[ii].ComplexPointReverseInfo.nGroupId = pLink->trk_ext.group_id;
					}
#endif
				} else {
					//#else
					pPoly = m_pDataMgr->GetPolygonDataByPoint(reqOpt.x, reqOpt.y, reqOpt.typeAll, 0);
					if (pPoly != nullptr && pPoly->poly_id.poly.type == TYPE_POLYGON_MOUNTAIN) {
						vtRouteInfos[ii].ComplexPointInfo.id = pPoly->poly_id.llid;
						vtRouteInfos[ii].ComplexPointReverseInfo.id = pPoly->poly_id.llid;
						cntEntrance++;
					}
				}
			}
		} // for
	}

	// 숲길인데 산바운더리 입구점이 없으면 링크 확장으로 입구점 검색
	//if (cntTrekking != cntEntrance) 
#endif // #if 0 // 일부 산 바운더리 입구점 매칭 오류 있어서 우선 사용 보류
	{
#if defined(USE_MULTIPROCESS)
#pragma omp parallel for
#endif
		for (int ii = 0; ii < cntPoints; ii++)
		{
			// 앞뒤 모두 동일 산이면, 패스
			if ((cntPoints >= 2) && (vtRouteInfos[ii].ComplexPointInfo.id != 0) &&
				(((ii == 0) && (vtRouteInfos[ii].ComplexPointInfo.id == vtRouteInfos[ii+1].ComplexPointInfo.id)) || // 처음과 다음이 같거나
				 ((ii == cntPoints - 1) && (vtRouteInfos[ii].ComplexPointInfo.id == vtRouteInfos[ii-1].ComplexPointInfo.id)) || // 끝과 이전이 같거나
				 ((ii > 0) && (ii < cntPoints - 1) && (vtRouteInfos[ii-1].ComplexPointInfo.id == vtRouteInfos[ii].ComplexPointInfo.id) && (vtRouteInfos[ii].ComplexPointInfo.id == vtRouteInfos[ii+1].ComplexPointInfo.id)))) { // 앞과 뒤가 같으면

				//LOG_TRACE(LOG_DEBUG, "-------- forest ent expand, %d", ii);
				continue;
			}
			// 현시점에서는 입구점이 숲길에만 있으니까 숲길만 확인하자
			else if ((pReqInfo->vtLinkDataType[ii] != TYPE_LINK_DATA_TREKKING) ||
				(vtRouteInfos[ii].ComplexPointInfo.nType != 0)) // 입구점 매칭 안된 경우만 재 확인
			{
				continue;
			}

			// 산바운더리가 없지만 그룹ID가 같아도 동일 산이므로, 패스
			if ((cntPoints >= 2) && (vtRouteInfos[ii].ComplexPointInfo.nGroupId != 0) &&
				(((ii == 0) && (vtRouteInfos[ii].ComplexPointInfo.nGroupId == vtRouteInfos[ii+1].ComplexPointInfo.nGroupId)) || // 처음과 다음이 같거나
					((ii == cntPoints - 1) && (vtRouteInfos[ii].ComplexPointInfo.nGroupId == vtRouteInfos[ii-1].ComplexPointInfo.nGroupId)) || // 끝과 이전이 같거나
					((ii > 0) && (ii < cntPoints - 1) && (vtRouteInfos[ii-1].ComplexPointInfo.nGroupId == vtRouteInfos[ii].ComplexPointInfo.nGroupId) && (vtRouteInfos[ii].ComplexPointInfo.nGroupId == vtRouteInfos[ii+1].ComplexPointInfo.nGroupId)))) // 앞과 뒤가 같으면
			{
				// 한쪽에만 산바운더리가 있으면 산바운더리 복사
				if ((ii < cntPoints - 1) && (vtRouteInfos[ii].ComplexPointInfo.id != vtRouteInfos[ii+1].ComplexPointInfo.id) &&
					((vtRouteInfos[ii].ComplexPointInfo.id != 0 && vtRouteInfos[ii+1].ComplexPointInfo.id == 0) ||
					 (vtRouteInfos[ii].ComplexPointInfo.id == 0 && vtRouteInfos[ii+1].ComplexPointInfo.id == 0))) {
					int idxSrc, idxDst;
					if (vtRouteInfos[ii].ComplexPointInfo.id == 0) {
						idxSrc = ii + 1;
						idxDst = ii;
					}
					else {
						idxSrc = ii;
						idxDst = ii + 1;
					}

					// 입구점 정보 복사 
					vtRouteInfos[idxDst].ComplexPointInfo.nType = vtRouteInfos[idxSrc].ComplexPointInfo.nType;
					vtRouteInfos[idxDst].ComplexPointInfo.id = vtRouteInfos[idxSrc].ComplexPointInfo.id;
					vtRouteInfos[idxDst].ComplexPointInfo.vtEntryPoint.assign(vtRouteInfos[idxSrc].ComplexPointInfo.vtEntryPoint.begin(), vtRouteInfos[idxSrc].ComplexPointInfo.vtEntryPoint.end());
				}
				continue;
			}

			RouteInfo* pRouteInfo = &vtRouteInfos[ii];

			pRouteInfo->reqInfo.SetOption(pReqInfo);
			//pRouteInfo->RequestMode = pReqInfo->RequestMode;
			//pRouteInfo->RequestId = pReqInfo->RequestId;
			//pRouteInfo->RouteOption = pReqInfo->RouteOption;
			//pRouteInfo->AvoidOption = pReqInfo->AvoidOption;
			//pRouteInfo->MobilityOption = pReqInfo->MobilityOption;
			//pRouteInfo->RouteSubOpt = pReqInfo->RouteSubOption;


			stLinkInfo* pLink = nullptr;

			pLink = m_pDataMgr->GetLinkDataById(pReqInfo->vtKeyId[ii], pReqInfo->vtLinkDataType[ii]);

			if (!pLink) {
				continue;
				/*LOG_TRACE(LOG_WARNING, "Failed, can't find start link, tile:%d, link:%d", pLinkInfo->LinkId.tile_id, pLinkInfo->LinkId.nid);
				ret = ROUTE_RESULT_FAILED_READ_DATA;
				return ret;*/
			}

			// 확장하면서 edge node 찾기
			vector<CandidateLink*> vtCandidateInfo;

			bool isReverse = true;

			pRouteInfo->ComplexPointInfo.nType = TYPE_ENT_MOUNTAIN;
			pRouteInfo->ComplexPointInfo.x = pReqInfo->vtPoints[ii].x;
			pRouteInfo->ComplexPointInfo.y = pReqInfo->vtPoints[ii].y;


			// 출발지
			if (ii == 0) {
				pRouteInfo->StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DEPARTURE; // 출발지
				if (pReqInfo->StartDirIgnore) { // 출발지 무시
					pRouteInfo->reqInfo.StartDirIgnore = true;
				}

				if (SetRouteLinkInfo(pReqInfo->vtPoints[ii], pReqInfo->vtKeyId[ii], pReqInfo->vtLinkDataType[ii], BIDIRECTION, true, pReqInfo->StartDirIgnore, &pRouteInfo->StartLinkInfo)) {
				}

				CheckStartDirectionMaching(&pRouteInfo->reqInfo, pLink, &pRouteInfo->StartLinkInfo, vtCandidateInfo);


				// 다음 지점을 도착지로
				pRouteInfo->EndLinkInfo = pReqInfo->vtPointsInfo[ii + 1];

				isReverse = false;
				EdgePropagation(ii, isReverse, vtOptInfo, pRouteInfo, vtCandidateInfo);
			}
			// 도착지
			else if (ii == cntPoints - 1) {
				pRouteInfo->StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_DESTINATION; // 도착지
				if (pReqInfo->EndDirIgnore) { // 도착지 무시
					pRouteInfo->reqInfo.EndDirIgnore = true;
				}

				if (SetRouteLinkInfo(pReqInfo->vtPoints[ii], pReqInfo->vtKeyId[ii], pReqInfo->vtLinkDataType[ii], BIDIRECTION, false, pReqInfo->EndDirIgnore, &pRouteInfo->StartLinkInfo)) {
				}

				CheckEndDirectionMaching(pReqInfo, pLink, &pRouteInfo->StartLinkInfo, vtCandidateInfo);


				// 이전 지점을 도착지로
				pRouteInfo->EndLinkInfo = pReqInfo->vtPointsInfo[ii - 1];

				// 역방향 탐색 수행
				isReverse = true;
				pRouteInfo->ComplexPointReverseInfo = pRouteInfo->ComplexPointInfo;

				EdgePropagation(ii, isReverse, vtOptInfo, pRouteInfo, vtCandidateInfo);
			}
			// 경유지
			else {
				pRouteInfo->StartLinkInfo.LinkGuideType = LINK_GUIDE_TYPE_WAYPOINT; // 경유지
				if (pReqInfo->WayDirIgnore) { // 경유지 무시
					pRouteInfo->reqInfo.StartDirIgnore = pRouteInfo->reqInfo.EndDirIgnore = true;
				}

				if (SetRouteLinkInfo(pReqInfo->vtPoints[ii], pReqInfo->vtKeyId[ii], pReqInfo->vtLinkDataType[ii], BIDIRECTION, false, pReqInfo->WayDirIgnore, &pRouteInfo->StartLinkInfo)) {
				}


				// 앞과 다른 정보일때
				if ((vtRouteInfos[ii - 1].ComplexPointInfo.id != vtRouteInfos[ii].ComplexPointInfo.id) ||
					(vtRouteInfos[ii - 1].ComplexPointInfo.nType != vtRouteInfos[ii].ComplexPointInfo.nType) ||
					(vtRouteInfos[ii - 1].ComplexPointInfo.nGroupId != vtRouteInfos[ii].ComplexPointInfo.nGroupId))
				{
					CheckEndDirectionMaching(pReqInfo, pLink, &pRouteInfo->StartLinkInfo, vtCandidateInfo);

					// 이전 지점을 도착지로
					pRouteInfo->EndLinkInfo = pReqInfo->vtPointsInfo[ii - 1];

					// 역방향 탐색 수행
					isReverse = true;
					pRouteInfo->ComplexPointReverseInfo = pRouteInfo->ComplexPointInfo;

					EdgePropagation(ii, isReverse, vtOptInfo, pRouteInfo, vtCandidateInfo);
				}

				if (!vtCandidateInfo.empty()) {
					vtCandidateInfo.clear();
					vector<CandidateLink*>().swap(vtCandidateInfo);
				}

				// 뒤와 다른 정보일때 
				if ((vtRouteInfos[ii].ComplexPointInfo.id != vtRouteInfos[ii + 1].ComplexPointInfo.id) ||
					(vtRouteInfos[ii].ComplexPointInfo.nType != vtRouteInfos[ii + 1].ComplexPointInfo.nType) ||
					(vtRouteInfos[ii].ComplexPointInfo.nGroupId != vtRouteInfos[ii + 1].ComplexPointInfo.nGroupId))
				{
					CheckStartDirectionMaching(pReqInfo, pLink, &pRouteInfo->StartLinkInfo, vtCandidateInfo);

					// 다음 지점을 도착지로
					pRouteInfo->EndLinkInfo = pReqInfo->vtPointsInfo[ii + 1];

					isReverse = false;
					EdgePropagation(ii, isReverse, vtOptInfo, pRouteInfo, vtCandidateInfo);
				}
			}
#if 1
			// using EdgePropagation function
			/*EdgePropagation(ii, isReverse, vtOptInfo, pRouteInfo, vtCandidateInfo);*/
#else
			// 종료 지점의 탐색 매칭 카운트 계산
			uint32_t cntGoal = 0; // 목적지 도달 가능한 링크 수 
			uint32_t cntGoalTouch = 0; // 목적지 도달한 링크 수
			//const uint32_t cntMaxGoalTouch = 100; // 최대 목적지 도착 검사 개수, 큰산은 1000개가 넘는 입구점이 있어 다 확인하긴 무리
			const uint32_t cntMaxGoalTouch = 30; // 최대 목적지 도착 검사 개수, 큰산은 1000개가 넘는 입구점이 있어 다 확인하긴 무리

			// 큐에 목록에 출발지로 등록
			for (auto & item : vtCandidateInfo) {
				pRouteInfo->mRoutePass.emplace(item->linkId.llid, item);
				pRouteInfo->pqDijkstra.emplace(item);				
			}

			CandidateLink* pCurInfo = nullptr;
			const stNodeInfo* pNode = nullptr;

			uint32_t cntMaxExtraSearch = 100; // 목적지 추가 도달 검사 최대 허용치
			uint32_t cntExtraSearch = 0; // 목적지 도달 후 추가 도달 검사 카운트
			uint32_t cntMntEntryPoint = 0; // 산바운더리 입구점 카운트

			int ret = ROUTE_RESULT_FAILED;

			// 경로 탐색
			int cntAddedLink = 0;
			int nProcess = 0;
			// debug for status

			unordered_set<uint64_t> setGoalTouchedNode;

			if (!vtOptInfo[ii].vtEntryPoint.empty()) {
				cntMntEntryPoint = vtOptInfo[ii].vtEntryPoint.size();
			}

			while (!pRouteInfo->pqDijkstra.empty())
			{
				nProcess++;
				// debug for status
	#if defined(_WIN32)
				routeProcessPrint(nProcess);
	#endif // #if defined(_WIN32)

				// 현재 링크 트리에서 제거
				pCurInfo = pRouteInfo->pqDijkstra.top();
				pRouteInfo->pqDijkstra.pop();
				pCurInfo->visited = true;

				if (isReverse) {
					cntAddedLink = AddPrevLinks(pRouteInfo, pCurInfo);
					pNode = GetPrevNode(pCurInfo);
				}
				else {
					cntAddedLink = AddNextLinks(pRouteInfo, pCurInfo);
					pNode = GetNextNode(pCurInfo);
				}

				// 현재 노드가 숲길 입구점이면 입구점 등록
				if (pNode && (pNode->trk.entrance) && (pNode->edgenode_id.llid != 0)) {
					bool isAdded = false;

					if (setGoalTouchedNode.find(pNode->node_id.llid) != setGoalTouchedNode.end())
					{
						// 이미 확인된 노드면
						;
					}
					else {
						setGoalTouchedNode.emplace(pNode->node_id.llid);

						// 산바운더리가 있고, 산바운더리 입구점이 있으면, 산바운더리 입구점만 등록하자
						if (cntMntEntryPoint > 0) {
							for (const auto& ent : vtOptInfo[ii].vtEntryPoint) {
								if (pNode->node_id.nid == ent.nID_1) {
									vtRouteInfos[ii].ComplexPointInfo.vtEntryPoint.emplace_back(ent);

									isAdded = true;
									break;
								}
							} // for
						}
						else { // 전부 다 등록
							stEntryPointInfo entry;
							entry.nID_1 = pNode->node_id.llid;
							entry.nID_2 = pNode->edgenode_id.llid; //
							entry.x = pNode->coord.x;
							entry.y = pNode->coord.y;
							entry.nAttribute = TYPE_OPTIMAL_ENTRANCE_MOUNTAIN;
							vtRouteInfos[ii].ComplexPointInfo.vtEntryPoint.emplace_back(entry);

							isAdded = true;
						}

						if (isAdded) {
							vtRouteInfos[ii].ComplexPointInfo.vtCost.emplace_back(pCurInfo->costTreavel);
							vtRouteInfos[ii].ComplexPointInfo.vtDist.emplace_back(pCurInfo->distTreavel);
							vtRouteInfos[ii].ComplexPointInfo.vtTime.emplace_back(pCurInfo->timeTreavel);

							// 입구점까지의 전체 경로 저장
							vtRouteInfos[ii].ComplexPointInfo.vtRoutePathInfo.emplace_back(pCurInfo);
							//captureResultPath(pCurInfo, vtComplexPointInfo[ii].vtRoutePathInfo);

							cntGoalTouch++;
						}
					}
				}

				if (((cntMntEntryPoint > 0) && (cntMntEntryPoint <= cntGoalTouch)) ||
					(cntMaxGoalTouch <= cntGoalTouch)) {
					break;
				}
			} // while
#endif

			//SAFE_DELETE(pRouteInfo);
		} // for
	}

	size_t tickEdgeRoute = TICK_COUNT() - tickEdgeRouteStart;


	if (ret != ROUTE_RESULT_SUCCESS) {
		LOG_TRACE(LOG_DEBUG, "Failed, can't find route, couldn't leached end point(the point will isolated)");
	}
	else if (0) {
	
	}

	return ret;
}


//const int CRoutePlan::DoEntryPointRoute(IN RouteInfo* pRouteInfo, IN const int32_t idx, IN const bool isFromPed, IN OUT vector<ComplexPointInfo>& vtComplexPointInfo)
//{
//	uint32_t ret = ROUTE_RESULT_FAILED;
//
//	size_t tickEdgeRouteStart = TICK_COUNT();
//
//	if (pRouteInfo == nullptr || idx < 0 || vtComplexPointInfo.empty()) {
//		LOG_TRACE(LOG_ERROR, "Failed, input param null, pRouteInfo:%p, idx:%d, vtComplexPointInfo size:%d", pRouteInfo, idx, vtComplexPointInfo.size());
//		return ROUTE_RESULT_FAILED;
//	}
//
//
//
//	// 확장하면서 entry point node 찾기
//	RouteLinkInfo* pCurrentRouteLinkInfo = nullptr;
//	vector<CandidateLink*> vtCandidateInfo;
//
//	if (isFromPed) {
//		pCurrentRouteLinkInfo = &pRouteInfo->StartLinkInfo;
//	}
//	else {
//		pCurrentRouteLinkInfo = &pRouteInfo->EndLinkInfo;
//	}
//
//	stLinkInfo* pLink = m_pDataMgr->GetLinkDataById(pCurrentRouteLinkInfo->LinkId, pCurrentRouteLinkInfo->LinkDataType);
//
//	if (!pLink)
//	{
//		LOG_TRACE(LOG_WARNING, "Failed, can't find link, tile:%d, link:%d", pCurrentRouteLinkInfo->LinkId.tile_id, pCurrentRouteLinkInfo->LinkId.nid);
//
//		return ROUTE_RESULT_FAILED_READ_DATA;
//	}
//
//	if (isFromPed) {
//		CheckStartDirectionMaching(pLink, pCurrentRouteLinkInfo, pRouteInfo->RouteOption, vtCandidateInfo);
//	}
//	else {
//		CheckEndDirectionMaching(pLink, pCurrentRouteLinkInfo, pRouteInfo->RouteOption, vtCandidateInfo);
//	}
//
//
//	// 출발지를 방문 목록에 등록
//	for (auto & item : vtCandidateInfo) {
//		pRouteInfo->mRoutePass.emplace(item->linkId.llid, item);
//		pRouteInfo->pqDijkstra.emplace(item);
//	}
//
//
//	// 종료 지점의 탐색 매칭 카운트 계산
//	uint32_t cntGoal = 0; // 목적지 도달 가능한 링크 수 
//	uint32_t cntGoalTouch = 0; // 목적지 도달한 링크 수
//	const uint32_t cntMaxGoalTouch = 100; // 최대 목적지 도착 검사 개수, 큰산은 1000개가 넘는 입구점이 있어 다 확인하긴 무리
//
//										  // 숲길 입구점에 해당하는 보행자길 목적지 입구점 노드 확인
//	unordered_map<uint64_t, int32_t> mapGoalNode;
//	unordered_map<uint64_t, int32_t>::iterator itGoalNode;
//	unordered_set<uint64_t> setGoalTouchedNode;
//
//	CandidateLink* pCurInfo = nullptr;
//	const stNodeInfo* pNode = nullptr;
//
//	static const uint32_t cntMaxExtraSearch = 100; // 목적지 추가 도달 검사 최대 허용치
//	uint32_t cntExtraSearch = 0; // 목적지 도달 후 추가 도달 검사 카운트
//
//								 // 경로 탐색
//	int cntAddedLink = 0;
//	int nProcess = 0;
//
//	ComplexPointInfo* pPedCpxPointInfo = nullptr;
//	ComplexPointInfo* pTrkCpxPointInfo = nullptr;
//
//	if (isFromPed) {
//		cntGoal = vtComplexPointInfo[idx + 1].vtEntryPoint.size();
//
//		pPedCpxPointInfo = &vtComplexPointInfo[idx];
//		pTrkCpxPointInfo = &vtComplexPointInfo[idx + 1];
//	}
//	else {
//		cntGoal = vtComplexPointInfo[idx].vtEntryPoint.size();
//
//		pPedCpxPointInfo = &vtComplexPointInfo[idx + 1];
//		pTrkCpxPointInfo = &vtComplexPointInfo[idx];
//	}
//
//	KeyID nodeId;
//	for (int32_t ii = 0; ii < cntGoal; ii++) {
//		nodeId.tile_id = pTrkCpxPointInfo->vtEntryPoint[ii].nID_2 / 100000;
//		nodeId.nid = pTrkCpxPointInfo->vtEntryPoint[ii].nID_2 % 100000;
//		nodeId.dir = 0;
//		mapGoalNode.emplace(nodeId.llid, ii);
//	}
//
//	pPedCpxPointInfo->nType = pTrkCpxPointInfo->nType;
//	pPedCpxPointInfo->x = pTrkCpxPointInfo->x;
//	pPedCpxPointInfo->y = pTrkCpxPointInfo->y;
//	pPedCpxPointInfo->id = pTrkCpxPointInfo->id;
//
//	pPedCpxPointInfo->vtEntryPoint.resize(cntGoal);
//	pPedCpxPointInfo->vtCost.resize(cntGoal);
//	pPedCpxPointInfo->vtDist.resize(cntGoal);
//	pPedCpxPointInfo->vtRoutePathInfo.resize(cntGoal);
//
//	while (!pRouteInfo->pqDijkstra.empty())
//	{
//		nProcess++;
//		// debug for status
//#if defined(_WIN32)
//		routeProcessPrint(nProcess);
//#endif // #if defined(_WIN32)
//
//		// 현재 링크 트리에서 제거
//		pCurInfo = pRouteInfo->pqDijkstra.top();
//		pRouteInfo->pqDijkstra.pop();
//		pCurInfo->visited = true;
//
//		//pNode = m_pDataMgr->GetNodeDataById(pCurInfo->nodeId, pLink->base.link_type);
//		if (isFromPed) {
//			cntAddedLink = AddNextLinks(pRouteInfo, pCurInfo);
//			pNode = GetNextNode(pCurInfo);
//		}
//		else {
//			cntAddedLink = AddPrevLinks(pRouteInfo, pCurInfo);
//			pNode = GetPrevNode(pCurInfo);
//		}
//
//		// 현재 노드가 숲길 입구점이면 입구점 등록
//		if (pNode && ((itGoalNode = mapGoalNode.find(pNode->node_id.llid)) != mapGoalNode.end())) {
//
//			if (!setGoalTouchedNode.empty() && setGoalTouchedNode.find(pNode->node_id.llid) != setGoalTouchedNode.end())
//			{
//				// 이미 확인된 노드면 패스 or 더 좋은 조건으로 스왑
//				;
//			}
//			else
//			{
//				setGoalTouchedNode.emplace(pNode->node_id.llid);
//
//				pPedCpxPointInfo->vtEntryPoint[itGoalNode->second].nID_1 = pNode->node_id.llid;
//				pPedCpxPointInfo->vtEntryPoint[itGoalNode->second].nID_2 = pNode->edgenode_id.llid; //
//				pPedCpxPointInfo->vtEntryPoint[itGoalNode->second].x = pNode->coord.x;
//				pPedCpxPointInfo->vtEntryPoint[itGoalNode->second].y = pNode->coord.y;
//				pPedCpxPointInfo->vtEntryPoint[itGoalNode->second].nAttribute = TYPE_OPTIMAL_ENTRANCE_MOUNTAIN;
//
//				pPedCpxPointInfo->vtCost[itGoalNode->second] = pCurInfo->costTreavel;
//				pPedCpxPointInfo->vtDist[itGoalNode->second] = pCurInfo->distReal;
//
//				// 입구점까지의 전체 경로 저장
//				pPedCpxPointInfo->vtRoutePathInfo[itGoalNode->second] = pCurInfo;
//				//captureResultPath(pCurInfo, complexPointInfo.vtRoutePathInfo);
//
//				cntGoalTouch++;
//			}
//		}
//
//		if ((cntGoal <= cntGoalTouch) ||
//			((cntMaxGoalTouch < cntGoal) && (cntMaxGoalTouch < cntGoalTouch))) {
//			// 경로 탐색 성공
//			ret = ROUTE_RESULT_SUCCESS;
//			LOG_TRACE(LOG_TEST, "Success find goal, (%d/%d)", cntGoalTouch, cntGoal);
//
//			break;
//		}
//	} // while
//
//	  //SAFE_DELETE(pRouteInfo);
//
//	size_t tickEdgeRoute = TICK_COUNT() - tickEdgeRouteStart;
//
//
//	if (ret != ROUTE_RESULT_SUCCESS) {
//		LOG_TRACE(LOG_DEBUG, "Failed, can't find route, couldn't leached end point(the point will isolated)");
//	}
//	else if (0) {
//
//	}
//
//	return ret;
//}


const int CRoutePlan::DoEntryPointRoute(IN RouteInfo* pRouteInfo, IN const bool isFromPed, IN const MultimodalPointInfo* pCpxPointInfoTrk, OUT MultimodalPointInfo& cpxPointInfoPed)
{
	uint32_t ret = ROUTE_RESULT_FAILED;

	size_t tickEdgeRouteStart = TICK_COUNT();

	if (pRouteInfo == nullptr || pCpxPointInfoTrk == nullptr) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, pRouteInfo:%p, pCpxPointInfoTrk:%p", pRouteInfo, pCpxPointInfoTrk);
		return ROUTE_RESULT_FAILED;
	}



	// 확장하면서 entry point node 찾기
	RouteLinkInfo* pCurrentRouteLinkInfo = nullptr;
	vector<CandidateLink*> vtCandidateInfo;

	if (isFromPed) {
		pCurrentRouteLinkInfo = &pRouteInfo->StartLinkInfo;
	}
	else {
		pCurrentRouteLinkInfo = &pRouteInfo->EndLinkInfo;
	}

	stLinkInfo* pLink = m_pDataMgr->GetLinkDataById(pCurrentRouteLinkInfo->LinkId, pCurrentRouteLinkInfo->LinkDataType);

	if (!pLink)
	{
		LOG_TRACE(LOG_WARNING, "Failed, can't find link, tile:%d, link:%d", pCurrentRouteLinkInfo->LinkId.tile_id, pCurrentRouteLinkInfo->LinkId.nid);

		return ROUTE_RESULT_FAILED_READ_DATA;
	}

	if (isFromPed) {
		CheckStartDirectionMaching(&pRouteInfo->reqInfo, pLink, pCurrentRouteLinkInfo, vtCandidateInfo);
	}
	else {
		CheckEndDirectionMaching(&pRouteInfo->reqInfo, pLink, pCurrentRouteLinkInfo, vtCandidateInfo);
	}


	// 출발지를 방문 목록에 등록
	for (auto & item : vtCandidateInfo) {
		pRouteInfo->mRoutePass.emplace(item->linkId.llid, item);
		pRouteInfo->pqDijkstra.emplace(item);
	}


	// 종료 지점의 탐색 매칭 카운트 계산
	uint32_t cntGoal = 0; // 목적지 도달 가능한 링크 수 
	uint32_t cntGoalTouch = 0; // 목적지 도달한 링크 수
	// const uint32_t cntMaxGoalTouch = 100; // 최대 목적지 도착 검사 개수, 큰산은 1000개가 넘는 입구점이 있어 다 확인하긴 무리
	const uint32_t cntMaxGoalTouch = 30; // 최대 목적지 도착 검사 개수, 큰산은 1000개가 넘는 입구점이 있어 다 확인하긴 무리

	// 숲길 입구점에 해당하는 보행자길 목적지 입구점 노드 확인
	unordered_map<uint64_t, int32_t> mapGoalNode;
	unordered_map<uint64_t, int32_t>::iterator itGoalNode;
	unordered_set<uint64_t> setGoalTouchedNode;

	CandidateLink* pCurInfo = nullptr;
	const stNodeInfo* pNode = nullptr;

	static const uint32_t cntMaxExtraSearch = 100; // 목적지 추가 도달 검사 최대 허용치
	uint32_t cntExtraSearch = 0; // 목적지 도달 후 추가 도달 검사 카운트

	// 경로 탐색
	int cntAddedLink = 0;
	int nProcess = 0;

	MultimodalPointInfo* pPedCpxPointInfo = nullptr;
	const MultimodalPointInfo* pTrkCpxPointInfo = nullptr;

	cntGoal = pCpxPointInfoTrk->vtEntryPoint.size();
	pPedCpxPointInfo = &cpxPointInfoPed;
	pTrkCpxPointInfo = pCpxPointInfoTrk;

	KeyID nodeId;
	for (int ii = 0; ii < cntGoal; ii++) {
		nodeId.tile_id = pTrkCpxPointInfo->vtEntryPoint[ii].nID_2 / 100000;
		nodeId.nid = pTrkCpxPointInfo->vtEntryPoint[ii].nID_2 % 100000;
		nodeId.dir = 0;
		mapGoalNode.emplace(nodeId.llid, ii);
	}

	pPedCpxPointInfo->nType = pTrkCpxPointInfo->nType;
	pPedCpxPointInfo->x = pTrkCpxPointInfo->x;
	pPedCpxPointInfo->y = pTrkCpxPointInfo->y;
	pPedCpxPointInfo->id = pTrkCpxPointInfo->id;
	pPedCpxPointInfo->nGroupId = pTrkCpxPointInfo->nGroupId;

	pPedCpxPointInfo->vtEntryPoint.resize(cntGoal);
	pPedCpxPointInfo->vtCost.resize(cntGoal);
	pPedCpxPointInfo->vtDist.resize(cntGoal);
	pPedCpxPointInfo->vtTime.resize(cntGoal);
	pPedCpxPointInfo->vtRoutePathInfo.resize(cntGoal);

	while (!pRouteInfo->pqDijkstra.empty())
	{
		nProcess++;
		// debug for status
#if defined(_WIN32)
		routeProcessPrint(nProcess);
#endif // #if defined(_WIN32)

		// 현재 링크 트리에서 제거
		pCurInfo = pRouteInfo->pqDijkstra.top();
		pRouteInfo->pqDijkstra.pop();
		pCurInfo->visited = true;

		//pNode = m_pDataMgr->GetNodeDataById(pCurInfo->nodeId, pLink->base.link_type);
		if (isFromPed) {
			cntAddedLink = AddNextLinks(pRouteInfo, pCurInfo);
			pNode = GetNextNode(pCurInfo);
		}
		else {
			cntAddedLink = AddPrevLinks(pRouteInfo, pCurInfo);
			pNode = GetPrevNode(pCurInfo);
		}

		// 현재 노드가 숲길 입구점이면 입구점 등록
		if (pNode && ((itGoalNode = mapGoalNode.find(pNode->node_id.llid)) != mapGoalNode.end())) {

			if (!setGoalTouchedNode.empty() && setGoalTouchedNode.find(pNode->node_id.llid) != setGoalTouchedNode.end())
			{
				// 이미 확인된 노드면 패스 or 더 좋은 조건으로 스왑
				;
			}
			else
			{
				setGoalTouchedNode.emplace(pNode->node_id.llid);

				pPedCpxPointInfo->vtEntryPoint[itGoalNode->second].nID_1 = pNode->node_id.llid;
				pPedCpxPointInfo->vtEntryPoint[itGoalNode->second].nID_2 = pNode->edgenode_id.llid; //
				pPedCpxPointInfo->vtEntryPoint[itGoalNode->second].x = pNode->coord.x;
				pPedCpxPointInfo->vtEntryPoint[itGoalNode->second].y = pNode->coord.y;
				pPedCpxPointInfo->vtEntryPoint[itGoalNode->second].nAttribute = TYPE_OPTIMAL_ENTRANCE_MOUNTAIN;

				pPedCpxPointInfo->vtCost[itGoalNode->second] = pCurInfo->costTreavel;
				pPedCpxPointInfo->vtDist[itGoalNode->second] = pCurInfo->distTreavel;
				pPedCpxPointInfo->vtTime[itGoalNode->second] = pCurInfo->timeTreavel;

				// 입구점까지의 전체 경로 저장
				pPedCpxPointInfo->vtRoutePathInfo[itGoalNode->second] = pCurInfo;
				//captureResultPath(pCurInfo, complexPointInfo.vtRoutePathInfo);

				cntGoalTouch++;
			}
		}

		// 전체 갯수의 90%만 성공해도 성공으로 치자
		if ((max(1, static_cast<int>(cntGoal * .9f)) <= cntGoalTouch) || 
			((cntMaxGoalTouch < cntGoal) && (cntMaxGoalTouch < cntGoalTouch))){
			// 경로 탐색 성공
			ret = ROUTE_RESULT_SUCCESS;
			LOG_TRACE(LOG_TEST, "Success find goal, (%d/%d)", cntGoalTouch, cntGoal);

			break;
		} 
		// 너무 많은 탐색 시도일 경우 실패 처리하자
#if defined(USE_FOREST_DATA)
		else if (pCurInfo->depth > 1000 && (pCurInfo->distTreavel > (MAX_SEARCH_DIST_FOR_COURSE * 2)))
#elif defined(USE_VEHICLE_DATA)
		else if (pCurInfo->depth > 3000 && (pCurInfo->distTreavel > (MAX_SEARCH_DIST_FOR_VEHICLE * 2)))
#else
		else if (pCurInfo->depth > 300 && (pCurInfo->distTreavel > (MAX_SEARCH_DIST_FOR_PEDESTRIAN)))
#endif
		{
			ret = ROUTE_RESULT_FAILED_EXPEND;
			break;
		}
	} // while

	//SAFE_DELETE(pRouteInfo);

	size_t tickEdgeRoute = TICK_COUNT() - tickEdgeRouteStart;


	if (ret != ROUTE_RESULT_SUCCESS) {
		LOG_TRACE(LOG_DEBUG, "Failed, can't find route, couldn't leached end point(the point will isolated)");
	}
	else if (0) {

	}

	return ret;
}


const int CRoutePlan::GetMostProbableLink(IN const RouteResultInfo* pResult, OUT vector<RouteLinkInfo>& vtMPP)
{
	// 경로선 노드의 경로선 외 링크 정보
	//const RouteResultInfo* pRouteResult = pResult;

	stLinkInfo* pLinkBefore = nullptr;
	stLinkInfo* pLinkNext = nullptr;

	if ((pResult == nullptr) || (pResult->ResultCode != ROUTE_RESULT_SUCCESS) || pResult->LinkInfo.empty()) {
		return 0;
	}

	//for (int ii = 0; ii < pRouteResult->LinkInfo.size() - 1; ii++) {
	for (int ii = 0; ii < pResult->LinkInfo.size() - 1; ii++) {
		pLinkBefore = m_pDataMgr->GetLinkDataById(pResult->LinkInfo[ii].link_id, TYPE_LINK_DATA_NONE);
		pLinkNext = m_pDataMgr->GetLinkDataById(pResult->LinkInfo[ii + 1].link_id, TYPE_LINK_DATA_NONE);

		// 정션 지점의 운행 각도
		int guideAng = pResult->LinkInfo[ii + 1].angle;
		int guideAng180 = 180 - abs(180 - guideAng); // +-10:직진, +-170: 유턴

		if (pLinkBefore == nullptr) {
			LOG_TRACE(LOG_WARNING, "failed, can't find fore link, idx:%d, id:%d", ii, pResult->LinkInfo[ii].link_id);
			// return 0;
			continue;
		} else if (pLinkNext == nullptr) {
			LOG_TRACE(LOG_WARNING, "failed, can't find next link, idx:%d, id:%d", ii + 1, pResult->LinkInfo[ii + 1].link_id);
			// return 0;
			continue;
		}

		stNodeInfo* pJctNode = nullptr;
		stLinkInfo* pJctLink = nullptr;
		int nJctDir = 0;

		// get junction node
		// <--- --->, <--- <---
		if (((pLinkBefore->getVertexX(0) == pLinkNext->getVertexX(0)) &&
			(pLinkBefore->getVertexY(0) == pLinkNext->getVertexY(0))) ||
			((pLinkBefore->getVertexX(0) == pLinkNext->getVertexX(pLinkNext->getVertexCount() - 1)) &&
				(pLinkBefore->getVertexY(0) == pLinkNext->getVertexY(pLinkNext->getVertexCount() - 1)))) {
			pJctNode = m_pDataMgr->GetNodeDataById(pLinkBefore->snode_id, TYPE_NODE_DATA_NONE);
			nJctDir = 0; // 정
		}
		// ---> <--- , ---> --->
		else if (((pLinkBefore->getVertexX(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexX(pLinkNext->getVertexCount() - 1)) &&
			(pLinkBefore->getVertexY(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexY(pLinkNext->getVertexCount() - 1))) ||
			((pLinkBefore->getVertexX(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexX(0)) &&
				(pLinkBefore->getVertexY(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexY(0)))) {
			pJctNode = m_pDataMgr->GetNodeDataById(pLinkBefore->enode_id, TYPE_NODE_DATA_NONE);
			nJctDir = 1; // 역
		}

		if (pJctNode == nullptr) {
			LOG_TRACE(LOG_WARNING, "failed, can't find junction node info, fore link:%d, next link:%d", pLinkBefore->link_id.nid, pLinkNext->link_id.nid);
			continue;
		}


		if (pJctNode->base.connnode_count >= 2) {
			for (const auto& connLinkId : pJctNode->connnodes) {
			//for (int jj = 0; jj < pJctNode->base.connnode_count; jj++) {
				// 경로에 사용된 링크는 제외
				// 2회 이상 재귀되는 경우 추가 확장되는 링크는 검사 안함 
				if ((connLinkId == pLinkBefore->link_id) || (connLinkId == pLinkNext->link_id)) {
					continue;
				}

				// 경로선 외 링크
				pJctLink = m_pDataMgr->GetLinkDataById(connLinkId, TYPE_LINK_DATA_NONE);

				// 의미 없는 링크 제외
#if defined(USE_PEDESTRIAN_DATA)
				// 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
				if ((pJctLink == nullptr) || 
					(pJctLink->ped.facility_type == 2) ||
					(pJctLink->ped.facility_type == 6) ||
					(pJctLink->ped.facility_type == 7) ||
					(pJctLink->ped.facility_type == 9) ||
					(pJctLink->ped.facility_type == 10)) {
					//LOG_TRACE(LOG_DEBUG, "---------------juntion snode slink(%d) sub continue: %lld, %.5f, %.5f", pNextLink->link_id.nid, pNextLink->link_id.llid, pNode->coord.x, pNode->coord.y);
					continue;
				}
#elif defined(USE_P2P_DATA)
				// 4차선 미만 도로는 제외
				if ((pJctLink == nullptr) || (pJctLink->veh.lane_cnt < 4) || (pJctLink->veh.hd_flag != 1))
					continue;
#endif

				RouteLinkInfo candidateLink;
				candidateLink.init();

				candidateLink.Coord = pJctNode->coord; // 좌표
				candidateLink.MatchCoord = pJctNode->coord; // 링크와 직교 접점 좌표
				candidateLink.LinkId = pJctLink->link_id; // 링크 ID
				candidateLink.LinkDataType = 0;
				candidateLink.LinkSplitIdx = 0; // 링크와 직교 접점 좌표의 링크 버텍스 idx

				if (pJctLink->snode_id == pJctNode->node_id) {
					candidateLink.LinkDir = DIR_POSITIVE;
				} else {
					candidateLink.LinkDir = DIR_NAGATIVE;
				}

				int candidateAng360 = getPathAngle(pLinkBefore, pJctLink);
				int candidateAng180 = 180 - abs(180 - candidateAng360); // +-10:직진, +-170: 유턴

				// 직진 주행이 아닌 경우, 직진 주행에 대한 대안 경로 생성
				if ((abs(guideAng180) > 30) && (candidateAng180 <= 30)) {
					vtMPP.emplace_back(candidateLink);
				}
			} // for jj
		}
	} // for ii

	return vtMPP.size();
}


#if defined(USE_SHOW_ROUTE_SATATUS)
void CRoutePlan::SetRouteStatusFunc(IN const void* pHost, IN void(*fpdrawRouting)(const void*, const unordered_map<uint64_t, CandidateLink*>*))
{
	m_pHost = const_cast<void*>(pHost);
	m_fpRoutingStatus = fpdrawRouting;
}
#endif