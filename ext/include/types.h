#pragma once

#include <stdint.h>
#include <string>

#include "../shp/shpio.h"

using namespace std;

// 이동체 구분
typedef enum {
	TYPE_MOBILITY_PEDESTRIAN, // 보행자
	TYPE_MOBILITY_BICYCLE, // 자전거
	TYPE_MOBILITY_MOTORCYCLE, // 오토바이
	TYPE_MOBILITY_VEHICLE, // 자동차
	TYPE_MOBILITY_AUTONOMOUS, // 자율주행
	TYPE_MOBILITY_TRUCK, // 트럭
	TYPE_MOBILITY_EMERGENCY, // 긴급
	TYPE_MOBILITY_COUNT,
}TYPE_MOBILITY_DATA;


// 데이터 파일 구분
typedef enum {
	TYPE_DATA_NONE = 0, // 미정의
	TYPE_DATA_NAME, // 명칭사전
	TYPE_DATA_MESH, // 메쉬
	TYPE_DATA_TREKKING, // 숲길
	TYPE_DATA_PEDESTRIAN, // 보행자/자전거
	TYPE_DATA_VEHICLE, // 자동차
	TYPE_DATA_BUILDING, // 건물
	TYPE_DATA_COMPLEX, // 단지
	TYPE_DATA_ENTRANCE, // 입구점
	TYPE_DATA_TRAFFIC, // 교통정보
	TYPE_DATA_MOUNTAIN, // 산바운더리
	TYPE_DATA_COURSE, // 코스정보
	TYPE_DATA_EXTEND, // 확장정보
	TYPE_DATA_COUNT,
}TYPE_DATA;

// 데이터 확장자 구분
typedef enum {
	TYPE_EXEC_NONE = 0, // 미정의
	TYPE_EXEC_NAME, // 명칭사전
	TYPE_EXEC_MESH, // 메쉬
	TYPE_EXEC_LINK, // 링크
	TYPE_EXEC_NODE, // 노드
	TYPE_EXEC_NETWORK, // 네트워크
	TYPE_EXEC_POLYGON, // 폴리곤
	TYPE_EXEC_ENTRANCE, // 입구점
	TYPE_EXEC_TRAFFIC, // 교통정보
	TYPE_EXEC_COURSE, // 코스정보
	TYPE_EXEC_INDEX, // 인덱스정보
	TYPE_EXEC_EXTEND, // 확장정보
	TYPE_EXEC_COUNT,
}TYPE_EXEC_DATA;


// 키 데이터 구분
typedef enum {
	TYPE_KEY_NONE = 0, // 미정의
	TYPE_KEY_NODE, // 노드
	TYPE_KEY_LINK, // 링크
	TYPE_KEY_COUNT,
}TYPE_KEY;


// 링크 데이터 파일 구분
typedef enum {
	TYPE_LINK_DATA_NONE = 0, // 미정의
	TYPE_LINK_DATA_TREKKING, // 숲길
	TYPE_LINK_DATA_PEDESTRIAN, // 보행자/자전거
	TYPE_LINK_DATA_VEHICLE, // 자동차
	TYPE_LINK_DATA_COUNT,
}TYPE_LINK_DATA;


// 노드 데이터 파일 구분
typedef enum {
	TYPE_NODE_DATA_NONE = 0, // 미정의
	TYPE_NODE_DATA_TREKKING, // 숲길
	TYPE_NODE_DATA_PEDESTRIAN, // 보행자/자전거
	TYPE_NODE_DATA_VEHICLE, // 자동차
	TYPE_NODE_DATA_COUNT,
}TYPE_NODE_DATA;


// 숲길 데이터
typedef enum {
	TYPE_TRE_NONE = 0, // 미지정(걷기)
	TYPE_TRE_HIKING, // 등산로(등산)
	TYPE_TRE_TRAIL, // 둘레길(코스)
	TYPE_TRE_BIKE, // 자전거길(자전거)
	TYPE_TRE_CROSS, // 종주길(코스ID)
	TYPE_TRE_RECOMMENDED, // 추천길
	TYPE_TRE_MTB, // MTB코스
	TYPE_TRE_POPULAR, // 인기코스
	TYPE_TRE_COUNT,
}TYPE_TREKKING;


// 보행자/자전거 데이터
typedef enum {
	TYPE_WALK_SIDE = 1, // 복선도로
	TYPE_WALK_WITH_CAR, // 차량겸용도로
	TYPE_WALK_WITH_BYC, // 자전거전용도로 -- 우선은 보행 불가 지역으로 이해하자
	TYPE_WALK_ONLY, // 보행전용도로
	TYPE_WALK_THROUGH, //가상보행도로
}TYPE_WALK;


typedef enum {
	TYPE_BYC_ONLY = 1, // 자전거 전용
	TYPE_BYC_WITH_CAR, // 보행자/차량겸용 자전거도로
	TYPE_BYC_WITH_WALK, // 보행도로 -- 우선은 자전거 통과 불가 지역으로 이해하자
}TYPE_BICYCLE;


typedef enum {
	TYPE_NODE_NONE = 0, // 미정의
	TYPE_NODE_COROSS, // 교차첨
	TYPE_NODE_END, // 단점
	TYPE_NODE_DUMMY, // 더미점
	TYPE_NODE_EDGE, // 구획변교점
	TYPE_NODE_ATTRIBUTE, // 속성변화점
	TYPE_NODE_SUBWAY, // 지하철 진출입
	TYPE_NODE_UNDERPASS, // 지하도 진출입
	TYPE_NODE_UNDERGROUND_MALL, // 지하상가 진출입
	TYPE_NODE_BUILDING, // 건물 진출입
}TYPE_NODE;


typedef enum {
    TYPE_GATE_NONE, // 미정의
    TYPE_GATE_SLOP, // 경사로
	TYPE_GATE_STAIRS, // 계단
	TYPE_GATE_ESCALATOR, // 에스컬레이터
	TYPE_GATE_STAIRS_ESCALATOR, // 계단/에스컬레이터
	TYPE_GATE_ELEVATOR, // 엘리베이터
	TYPE_GATE_CONNECTION, // 단순연결로
	TYPE_GATE_CROSSWALK, // 횡단보도
    TYPE_GATE_MOVINGWALK, // 무빙워크
    TYPE_GATE_STEPPINGSTONES, // 징검다리
	TYPE_GATE_VIRTUAL, // 의사횡단
	TYPE_GATE_COUNT,
}TYPE_GATE;


typedef enum {
    TYPE_OBJECT_NONE, // 미정의
	TYPE_OBJECT_CAVE, // 토끼굴
	TYPE_OBJECT_UNDERPASS, // 지하보도
	TYPE_OBJECT_FOOTBRIDGE, // 육교
	TYPE_OBJECT_OVERPASS, // 고가도로
	TYPE_OBJECT_BRIDGE, // 교량
	TYPE_OBJECT_SUBWAY, // 지하철역
	TYPE_OBJECT_RAILROAD, // 철도
	TYPE_OBJECT_BUSSTATION, // 중앙버스정류장
	TYPE_OBJECT_UNDERGROUNDMALL, // 지하상가
	TYPE_OBJECT_THROUGHBUILDING, // 건물관통도로
	TYPE_OBJECT_COMPLEXPARK, // 단지도로_공원
	TYPE_OBJECT_COMPLEXAPT, // 단지도로_주거시설
	TYPE_OBJECT_COMPLEXTOUR, // 단지도로_관광지
	TYPE_OBJECT_COMPLEXETC, // 단지도로_기타
	TYPE_OBJECT_COUNT,
}TYPE_OBJECT;


typedef enum
{
	PASS_CODE_ENABLE,
	PASS_CODE_UTURN,
	PASS_CODE_DISABLE,
}PASS_CODE;


typedef enum {
	ROUTE_TYPE_NONE = 0, // 미정의
	ROUTE_TYPE_TREKKING, // 숲길
	ROUTE_TYPE_PEDESTRIAN, // 보행자
	ROUTE_TYPE_BIKE, // 자전거
	ROUTE_TYPE_KICKBOARD, // 킥보드
	ROUTE_TYPE_MOTOCYCLE, // 모터사이클
	ROUTE_TYPE_VEHICLE, // 차량
}ROUTE_TYPE;


typedef enum {
	ROUTE_OPT_SHORTEST = 0, // 최단거리
	ROUTE_OPT_RECOMMENDED, // 추천
	ROUTE_OPT_COMFORTABLE, // 편안한
	ROUTE_OPT_FASTEST, // 최소시간(빠른)
	ROUTE_OPT_MAINROAD, // 큰길
	ROUTE_OPT_COUNT,
}ROUTE_OPTION;


typedef enum {
	//ROUTE_AVOID_NONE = 0,
	//ROUTE_AVOID_HIKING = 0x00000001, // 등산로
	//ROUTE_AVOID_TRAIL = 0x00000002, // 둘레길
	//ROUTE_AVOID_BIKE = 0x00000004, // 자전거
	//ROUTE_AVOID_THEME = 0x00000008, // 테마길
	//ROUTE_AVOID_CROSS = 0x00000010, // 종주길

	//0:미정의, 1 : 등산로, 2 : 둘레길, 3 : 자전거길, 4 : 종주코스, 5 : 추천코스, 6 : MTB코스, 7 : 인기코스
	ROUTE_AVOID_NONE		= 0,
	ROUTE_AVOID_HIKING		= 0x00000001, // 등산로
	ROUTE_AVOID_TRAIL		= 0x00000002, // 둘레길
	ROUTE_AVOID_BIKE		= 0x00000004, // 자전거
	ROUTE_AVOID_CROSS		= 0x00000008, // 종주코스
	ROUTE_AVOID_RECOMMEND	= 0x00000010, // 추천코스
	ROUTE_AVOID_MTB			= 0x00000020, // MTB코스
	ROUTE_AVOID_POPULAR		= 0x00000040, // 인기코스

	ROUTE_AVOID_ALLEY		= 0x00000100,
	ROUTE_AVOID_PAVE		= 0x00000200,
	ROUTE_AVOID_STAIRS		= 0x00000400,
	ROUTE_AVOID_BRIDGE		= 0x00000800,
	ROUTE_AVOID_ROCK		= 0x00001000,
	ROUTE_AVOID_RIDGE		= 0x00002000,
	ROUTE_AVOID_LADDER		= 0x00004000,
	ROUTE_AVOID_ROPE		= 0x00008000,
	ROUTE_AVOID_TATTERED	= 0x00010000,
	ROUTE_AVOID_PALM		= 0x00020000,
	ROUTE_AVOID_DECK		= 0x00040000,

	//ROUTE_AVOID_ALLEY = 1,
	//ROUTE_AVOID_PAVE = 2,
	//ROUTE_AVOID_STAIRS = 4,
	//ROUTE_AVOID_BRIDGE = 8,
	//ROUTE_AVOID_ROCK = 16,
	//ROUTE_AVOID_RIDGE = 32,
	//ROUTE_AVOID_LADDER = 64,
	//ROUTE_AVOID_ROPE = 128,
	//ROUTE_AVOID_TATTERED = 256,
	//ROUTE_AVOID_PALM = 512,
	//ROUTE_AVOID_DECK = 1024,
	// 0:미정의, 1 : 등산로, 2 : 둘레길, 3 : 자전거길, 4 : 종주코스
	// 1:오솔길(1), 2:포장길(2), 3:계단(4), 4:교량(8), 5:암릉(16), 6:릿지(32), 7:사다리(64), 8:밧줄(128), 9:너덜길(256), 10:야자수매트(512), 11:데크로드(1024)
}ROUTE_AVOID;

typedef enum {
	ROUTE_AVOID_PED_NONE				= 0, // 미정의					// 0x0000 0000	// 0000 0000 0000 0000 0000 0000 0000 0000

    ROUTE_AVOID_PED_SLOP				= 1, // 경사로					// 0x0000 0001	// 0000 0000 0000 0000 0000 0000 0000 0001
	ROUTE_AVOID_PED_STAIRS				= 2, // 계단						// 0x0000 0002	// 0000 0000 0000 0000 0000 0000 0000 0010
	ROUTE_AVOID_PED_ESCALATOR			= 4, // 에스컬레이터				// 0x0000 0004	// 0000 0000 0000 0000 0000 0000 0000 0100
	ROUTE_AVOID_PED_STAIRS_ESCALATOR	= 8, // 계단/에스컬레이터			// 0x0000 0008	// 0000 0000 0000 0000 0000 0000 0000 1000

	ROUTE_AVOID_PED_ELEVATOR			= 16, // 엘리베이터				// 0x0000 0010	// 0000 0000 0000 0000 0000 0000 0001 0000
	ROUTE_AVOID_PED_CONNECTION			= 32, // 단순연결로				// 0x0000 0020	// 0000 0000 0000 0000 0000 0000 0010 0000
	ROUTE_AVOID_PED_CROSSWALK			= 64, // 횡단보도					// 0x0000 0040	// 0000 0000 0000 0000 0000 0000 0100 0000
    ROUTE_AVOID_PED_MOVINGWALK			= 128, // 무빙워크				// 0x0000 0080	// 0000 0000 0000 0000 0000 0000 1000 0000

    ROUTE_AVOID_PED_STEPPINGSTONES		= 256, // 징검다리				// 0x0000 0100	// 0000 0000 0000 0000 0000 0001 0000 0000
	ROUTE_AVOID_PED_VIRTUAL				= 512, // 의사횡단				// 0x0000 0200	// 0000 0000 0000 0000 0000 0010 0000 0000
	ROUTE_AVOID_PED_ANY					= 1024, // 실수로 건너뜀			// 0x0000 0400	// 0000 0000 0000 0000 0000 0100 0000 0000
	ROUTE_AVOID_PED_CAVE				= 2048, // 토끼굴					// 0x0000 0800	// 0000 0000 0000 0000 0000 1000 0000 0000

	ROUTE_AVOID_PED_UNDERPASS			= 4096, // 지하보도				// 0x0000 1000	// 0000 0000 0000 0000 0001 0000 0000 0000
	ROUTE_AVOID_PED_FOOTBRIDGE			= 8192, // 육교					// 0x0000 2000	// 0000 0000 0000 0000 0010 0000 0000 0000
	ROUTE_AVOID_PED_OVERPASS			= 16384, // 고가도로				// 0x0000 4000	// 0000 0000 0000 0000 0100 0000 0000 0000
	ROUTE_AVOID_PED_BRIDGE				= 32768, // 교량					// 0x0000 8000	// 0000 0000 0000 0000 1000 0000 0000 0000

	ROUTE_AVOID_PED_SUBWAY				= 65536, // 지하철역				// 0x0001 0000	// 0000 0000 0000 0001 0000 0000 0000 0000
	ROUTE_AVOID_PED_RAILROAD			= 131072, // 철도				// 0x0002 0000	// 0000 0000 0000 0010 0000 0000 0000 0000
	ROUTE_AVOID_PED_BUSSTATION			= 262144, // 중앙버스정류장		// 0x0004 0000	// 0000 0000 0000 0100 0000 0000 0000 0000
	ROUTE_AVOID_PED_UNDERGROUNDMALL		= 524288, // 지하상가				// 0x0008 0000	// 0000 0000 0000 1000 0000 0000 0000 0000

	ROUTE_AVOID_PED_THROUGHBUILDING		= 1048576, // 건물관통도로			// 0x0010 0000	// 0000 0000 0001 0000 0000 0000 0000 0000
	ROUTE_AVOID_PED_COMPLEXPARK			= 2097152, // 단지도로_공원		// 0x0020 0000	// 0000 0000 0010 0000 0000 0000 0000 0000
	ROUTE_AVOID_PED_COMPLEXAPT			= 4194304, // 단지도로_주거시설	// 0x0040 0000	// 0000 0000 0100 0000 0000 0000 0000 0000
	ROUTE_AVOID_PED_COMPLEXTOUR			= 8388608, // 단지도로_관광지		// 0x0080 0000	// 0000 0000 1000 0000 0000 0000 0000 0000

	ROUTE_AVOID_PED_COMPLEXETC			= 16777216, // 단지도로_기타		// 0x0100 0000	// 0000 0001 0000 0000 0000 0000 0000 0000
}ROUTE_PED_AVOID;


typedef enum {
	ROUTE_AVOID_VEH_NONE = 0,		// 미정의						// 0x0000 0000	// 0000 0000 0000 0000 0000 0000 0000 0000

	ROUTE_AVOID_SHORTTURN = 1,		// 짧은회전					// 0x0000 0001	// 0000 0000 0000 0000 0000 0000 0000 0001
}ROUTE_VEH_AVOID;


// 링크 서브 타입
typedef enum {
	TYPE_SUBINFO_NONE = 0, // 미지정
	TYPE_SUBINFO_FTYPE, // 시설물 타입
	TYPE_SUBINFO_GTYPE, // 진입로 타입
}TYPE_SUBINFO;


// 건물 종별 코드
typedef enum {
	TYPE_BUILDING_NOT = 0,//미지정
	TYPE_BUILDING_APT, //아파트
	TYPE_BUILDING_SCH, //학교
	TYPE_BUILDING_OFT, //오피스텔
	TYPE_BUILDING_B01, //공공기관
	TYPE_BUILDING_B02, //치안기관
	TYPE_BUILDING_B03, //공원
	TYPE_BUILDING_B04, //교육기관
	TYPE_BUILDING_B05, //언론기관
	TYPE_BUILDING_B06, //금융기관
	TYPE_BUILDING_B07, //문화
	TYPE_BUILDING_B08, //관광
	TYPE_BUILDING_B09, //레져
	TYPE_BUILDING_B10, //음식점
	TYPE_BUILDING_B11, //편의
	TYPE_BUILDING_B12, //복지
	TYPE_BUILDING_B13, //기업
	TYPE_BUILDING_B14, //농공시설
	TYPE_BUILDING_B15, //자동차관련
	TYPE_BUILDING_B16, //교통시설
	TYPE_BUILDING_B17, //지하철시설
	TYPE_BUILDING_B18, //도로시설
	TYPE_BUILDING_B19, //주택관련
	TYPE_BUILDING_ETC, //주택외건물
	TYPE_BUILDING_E2D, //주택외건물2D
	TYPE_BUILDING_IND, //공업단지내건물
	TYPE_BUILDING_STA, //기차역
	TYPE_BUILDING_APC, //주상복합
	TYPE_BUILDING_ROH, //연립주택
	TYPE_BUILDING_APL, //아파트형공장
	TYPE_BUILDING_APS, //아파트 상가
	TYPE_BUILDING_MAX_CNT,
}TYPE_BUILDING;


typedef enum {
	TYPE_BLD_NAME_NOT = 0, // 명칭 사전 인덱스
	TYPE_BLD_NAME_INT, // 정수 '123'
	TYPE_BLD_NAME_ENG, // 영문 'A'
	TYPE_BLD_NAME_DIC, // 명칭 사전
}TYPE_BLD_NAME;


typedef enum {
	TYPE_COMPLEX_NOT = 0, //미지정
	TYPE_COMPLEX_APC,	// 주상복합단지
	TYPE_COMPLEX_APL,	// 아파트형공장단지
	TYPE_COMPLEX_APT,	// 아파트단지
	TYPE_COMPLEX_B01,	// 공공기관단지
	TYPE_COMPLEX_B04,	// 교육기관단지
	TYPE_COMPLEX_B11,	// 편의시설단지
	TYPE_COMPLEX_ETC,	// 기타단지
	TYPE_COMPLEX_OFT,	// 오피스텔단지
	TYPE_COMPLEX_ROH,	// 연립주택단지
	TYPE_COMPLEX_SCH,	// 학교단지
	TYPE_COMPLEX_MAX_CNT,
}TYPE_COMPLEX;


typedef enum {
	TYPE_POLYGON_NONE = 0, // 미지정
	TYPE_POLYGON_BUILDING, // 빌딩
	TYPE_POLYGON_COMPLEX, // 단지
	TYPE_POLYGON_MOUNTAIN, // 산바운더리
}TYPE_POLYGON;


typedef enum {
	TYPE_ENT_NONE = 0, // 미지정
	TYPE_ENT_BUILDING, // 빌딩입구점
	TYPE_ENT_COMPLEX, // 단지입구점
	TYPE_ENT_MOUNTAIN = TYPE_ENT_COMPLEX, // 숲길 입구점
	TYPE_ENT_CPX_ROAD, // 단지내도로
	TYPE_ENT_NEAR_ROAD, // 최근접도로
	TYPE_ENT_SUBWAY, // 지하철입구점
}TYPE_ENTRANCE;


typedef enum {
	TYPE_OPTIMAL_ENTRANCE_NONE = 0, // 미지정
	TYPE_OPTIMAL_ENTRANCE_CAR, // 차량 입구점
	TYPE_OPTIMAL_ENTRANCE_BUILDING_TAXI, // 택시 승하차 지점(건물)
	TYPE_OPTIMAL_ENTRANCE_COMPLEX_TAXI, // 택시 승하차 지점(건물군)
	TYPE_OPTIMAL_ENTRANCE_PARCEL_CAR, // 택배 차량 하차 거점
	TYPE_OPTIMAL_ENTRANCE_PEDESTRIAN, // 보행자 입구점
	TYPE_OPTIMAL_ENTRANCE_DELIBERY_CAR, // 배달 하차점(차량, 오토바이)
	TYPE_OPTIMAL_ENTRANCE_DELIBERY_PED, // 배달 하차점(자전거, 도보)

	TYPE_OPTIMAL_ENTRANCE_MOUNTAIN, // 숲길 입구점
	TYPE_OPTIMAL_ENTRANCE_SUBWAY, // 지하철 입구점
	// max 15, using only 4bit
}TYPE_OPTIMAL_ENTRANCE;


typedef enum {
	TYPE_MOUNTAIN_ENTRANCE_NONE = 0, // 미지정

	// max 8, using only 4bit
}TYPE_MOUNTAIN_ENTRANCE;


typedef enum
{
	TYPE_ROAD_LEVEL_GROUND = 0, // 지표
	TYPE_ROAD_LEVEL_SUBSUFACE, // 지하
	TYPE_ROAD_LEVEL_ELEVATED, // 지상
}TYPE_ROAD_LEVEL;


typedef enum {
	TYPE_LINK_MATCH_NONE = 0, // 미지정
	TYPE_LINK_MATCH_CARSTOP, // 차량 승하자
	TYPE_LINK_MATCH_CARSTOP_EX, // 차량 승하자 + 단지내도로(건물입구점 재확인시, 단지도로에 매칭된 입구점을 확인하기 위해 포함)
	TYPE_LINK_MATCH_CARENTRANCE, // 차량 진출입 전용
	TYPE_LINK_MATCH_FOR_TABLE, // 방문지 테이블용 , 2차선 이상만
	TYPE_LINK_MATCH_FOR_HD, // P2P HD 경로 탐색 용, sd-hd 매칭되는 링크만 선택
	TYPE_LINK_MATCH_FOR_FOREST, // 숲길 경로 링크 선택, 숲길(우선), 보행자길
	TYPE_LINK_MATCH_FOR_BICYCLE, // PM(킥보드, 자전거)
	TYPE_LINK_MATCH_FOR_MOTORCYCLE, // 2륜차
}TYPE_LINK_MATCH;


#define POLYGON_DATA_ATTR_MAX		5
typedef enum {
	TYPE_POLYGON_DATA_ATTR_PART = 0,// 폴리곤 파트 인덱스
	TYPE_POLYGON_DATA_ATTR_VTX,		// 폴리곤 버텍스
	TYPE_POLYGON_DATA_ATTR_ENT,		// 입구점 정보
	TYPE_POLYGON_DATA_ATTR_LINK,	// 단지내 도로 ID
	TYPE_POLYGON_DATA_ATTR_MESH		// 중첩 메쉬
}TYPE_POLYGON_DATA_ATTR;

/*
01	시설물 정보
02	진출입구 정보
*/
typedef enum
{
	TYPE_WNODEEX_FCT,
	TYPE_WNODEEX_IO,
	TYPE_WNODEEX_COUNT
}TYPE_WNODEEX;

/*
01	중량제한	중량 제한 정보	T(톤) 단위
02	높이제한	높이 제한 정보	M(미터) 단위
03	교량	교량 명칭 정보	명칭
04	터널	터널 명칭 정보	명칭
05	고가도로	고가도로 명칭 정보	명칭
06	지하차도	지하차도 명칭 정보	명칭
07	버스전용차로	버스 전용 차로 시간 정보	버스 전용 차로 시간 (CODE로 구분)
08	고속모드	내비게이션 고속모드 정보	명칭
09	고속모드부가	고속모드 휴게소 상세 정보	고속모드 휴게소 상세 정보 (CODE로 구분)
10	차선정보	차선 정보	링크의 진입 기준 차선 정보 (CODE로 구분)
11	방면정보	방면 정보	링크의 진출 기준 방면 정보 (표지판)
12	차로유도차선	차로 유도 차선 정보	링크의 진입 기준 컬러 차선 정보
13	차로유도방면	차로 유도 방면 정보	링크의 진출 기준 차로 유도선 방면 정보 (노면)
14	톨게이트	톨게이트 명칭 정보	명칭
15	확대도	확대도 명칭 정보	확대도 파일 명칭
16	TPEG	표준노드링크의 링크 ID	표준노드링크 LINK_ID (10자리)
17	진입제한	진입제한 정보	진입제한 정보 (CODE로 구분)
*/
typedef enum
{
	TYPE_LINKEX_WEIGHT,
	TYPE_LINKEX_HEIGHT,
	TYPE_LINKEX_COUNT
}TYPE_LINKEX;


typedef enum {
	ROUTE_TARGET_DEFAULT = 0, // default
	ROUTE_TARGET_INAVI, // for inavi
	ROUTE_TARGET_KAKAOVX, // for kakaovx
} ROUTE_TARGET;

typedef enum {
	LINK_GUIDE_TYPE_DEFAULT = 0, // 일반
	LINK_GUIDE_TYPE_DEPARTURE, // 출발지
	LINK_GUIDE_TYPE_WAYPOINT, // 경유지
	LINK_GUIDE_TYPE_DESTINATION, // 도착지
	LINK_GUIDE_TYPE_DEPARTURE_WAYPOINT, // 출발지-경유지
	LINK_GUIDE_TYPE_DEPARTURE_DESTINATION, // 출발지-도착지
	LINK_GUIDE_TYPE_WAYPOINT_WAYPOINT, // 경유지-경유지
	LINK_GUIDE_TYPE_WAYPOINT_DESTINATION, // 경유지-도착지
} LINK_GUIDE_TYPE;

typedef enum {
	RESULT_FAILED = -1,
	RESULT_OK = 0,
	ROUTE_RESULT_SUCCESS = RESULT_OK,					//0		성공

	ROUTE_RESULT_FAILED = 1,							//1		탐색 실패	내부 오류에 의한 실패
	ROUTE_RESULT_FAILED_SAME_ROUTE = 2,					//2		스마트 재탐색 적용	기존 경로와 동일

	ROUTE_RESULT_FAILED_MULTI_POS_ROUTE_ALL = 5,		//5		다중 경로 탐색 모두 실패
	ROUTE_RESULT_FAILED_MULTI_POS_ROUTE = 6,			//6		다중 경로 탐색 실패

	ROUTE_RESULT_FAILED_WRONG_PARAM = 10,				//10	잘못된 파라미터, 필수 파라미터 체크
	ROUTE_RESULT_FAILED_SET_MEMORY = 50,				//50	탐색 확장 관련 메모리 할당 오류	탐색 초기화 관련
	ROUTE_RESULT_FAILED_READ_DATA = 51,					//51	탐색 관련 데이터(지도, 옵션 등) 파일 읽기 실패	탐색 초기화 관련
	ROUTE_RESULT_FAILED_SET_START = 70,					//70	출발지가 프로젝션이 안되거나, 잘못된 출발지
	ROUTE_RESULT_FAILED_SET_VIA = 71,					//71	경유지가 프로젝션이 안되거나, 잘못된 경유지
	ROUTE_RESULT_FAILED_SET_END = 72,					//72	목적지가 프로젝션이 안되거나, 잘못된 목적지
	ROUTE_RESULT_FAILED_DIST_OVER = 90,					//90	탐색 가능 거리 초과, 직선거리 15km 이내 허용
	ROUTE_RESULT_FAILED_TIME_OVER = 91,					//91	탐색 시간 초과	10초 이상
	ROUTE_RESULT_FAILED_NODE_OVER = 92,					//92	확장 가능 Node 개수 초과
	ROUTE_RESULT_FAILED_EXPEND = 93,					//93	확장 실패
	ROUTE_RESULT_FAILED_EXPEND_ISOLATED = 93,			//93	확장 실패, 고립 링크

	ROUTE_RESULT_FAILED_COURSE = 600,					//600	코스 탐색 실패
	ROUTE_RESULT_FAILED_COURSE_ID = 601,				//601	코스 ID 검색 실패
	ROUTE_RESULT_FAILED_COURSE_TYPE = 602,				//602	코스 TYPE 검색 실패

	TMS_RESULT_FAILED = 10000,							//10000	클러스터링 실패
	TMS_RESULT_FAILED_LOOP = 10001,						//10001 분배 재계산 한계 횟수 초과
	TMS_RESULT_FAILED_MATCHING_LIMIT = 10002,			//10002 지정된 분배 수량내에서 배정이 실패함
	TMS_RESULT_FAILED_MATCHING_DEVIATION = 10003,		//10003 지정된 편차 내의 배정이 실패함

	// 예상되는 오류
	// 최대 분배 값이 너무 작아서 분배할 수 없음, limit_value = 1개, 1초, 1미터 등
	// 최대 편차 값이 너무 작어서 분배할 수 없음, limit_deviation = 0개, 1초, 1미터 등 
	// 그룹 지정 값이 너무 작아서분배할 수 없음, limit_count = 1
	// 그룹 지정 값이 너무 커서 분배할 수 없음, limit_count = 100

	TMS_RESULT_FAILED_INPUT_OPTION = 10100,				//10100 사용자 옵션 오류
	TMS_RESULT_FAILED_OPTION = 10101,					//10101 사용자 옵션 오류
	TMS_RESULT_FAILED_LIMIT_COUNT_OVER = 10102,			//10102 전체 수량에 대해 지정 구역수와 구역별 지정 배정수가 한계치를 초과함.
	TMS_RESULT_FAILED_LIMIT_VALUE_NULL = 10103,			//10103 최적 분배의 경우 제한값 적용이 필요함
	TMS_RESULT_FAILED_LIMIT_VALUE_TOO_SMALL = 10104,	//10104 제한값이 너무 작게 설정됨 (배송지 균등에 1개 등)


	TMS_RESULT_FAILED_EXCEEDS_COUNT = 11000,			//11000 제한 개수 초과 오류
	// 	resultCode	resultMessage	비고	설명
	// 0		공통	성공
	// 100	Result Not Found	검색 전용	결과 없음
	// 101	Argument Error	공통	파라미터 오류
	// 102	Internal Server Error	공통	서버 오류
	// 103	Different Map Version	검색 전용	지도 버전 다름
	// 104	StyleID already exists	검색 전용	이미 동일한 StyleID가 있음
	// 201	Searching for Security	검색 전용	POI 보안시설물
	// 202	Longitude/Latitude	검색 전용	경위도
	// 203	Mobile Phone Number	검색 전용	전화번호 (Mobile)
	// 204	Invalid Query	검색 전용	서버 오류
	// 205	POI not in given Admin	검색 전용	결과 없음 (지역설정)
	// 206	POI not in given Area	검색 전용	결과 없음 (영역설정)
	// 207	POI not in given Category	검색 전용	결과 없음 (분류설정)
	// 208	Neighbor Search Only	검색 전용	결과 없음 (주변검색만입력)
	// 209	Neighbor Search not Found	검색 전용	결과 없음 (주변 + 키워드 검색 결과 없음)
	// 300	AppKey Error	공통	AppKey 인증 오류
	// 400	Taxi Fare Info not Found	검색 전용	택시 요금 정보 없음
	// 401	AreaCode Convert Fail	검색 전용	지역코드 변환 실패
	// 501	Unknown Fail	탐색 전용	실패(unknown)
	// 502	Apply Smart Re-navigation	탐색 전용	Smart재탐색 적용
	// 503	Canceled navigation by user	탐색 전용	사용자에 의한 탐색 취소
	// 504	Error due to Checksum	탐색 전용	체크섬 오류
	// 517	Memory allocation failure	탐색 전용	메모리 할당 실패
	// 518	File open failure	탐색 전용	파일 열기 실패
	// 519	File read failure	탐색 전용	파일 읽기 실패
	// 520	File write failure	탐색 전용	파일 쓰기 실패
	// 521	Socket connection failure	탐색 전용	소켓 연결 실패
	// 532	Request parameter is invalid	탐색 전용	요청 파라미터가 유효하지 않음
	// 533	The starting point is not selected, or the wrong starting point	탐색 전용	출발지가 선택되지 않았거나, 잘못된 출발지
	// 534	Destination is not selected or wrong destination	탐색 전용	목적지가 선택되지 않았거나, 잘못된 목적지
	// 535	Wrong stopover	탐색 전용	잘못된 경유지
	// 536	Link Projection failure	탐색 전용	Link Projection 실패 (네트워크(도로망)이 없는 경우 등)
	// 537	Exceeding the navigational distance (1000km, walking navigation: 20km)	탐색 전용	탐색 가능 거리 초과(1000km, 도보 탐색 : 20km)
	// 538	Exceeds the number of expandable nodes	탐색 전용	확장 가능 노드 수 초과
	// 539	Expansion failure	탐색 전용	확장 실패
	// 540	Expansion failure due to inactivity or traffic control	탐색 전용	특별한 사정이나 교통 통제로 인한 확장 실패
	// 541	Expansion failure due to vehicle height/weight restrictions near the starting point	탐색 전용	출발지 근처의 차량 높이/중량 제한으로 확장 실패
	// 542	Expansion failed due to a part-time curfew near the departure point	탐색 전용	출발지 근처의 시간제 통행금지로 인해 확장 실패
	// 543	The destination is a physical island road, and there are no established ferry routes.	탐색 전용	목적지가 물리적 섬도로이며, 구축된 페리 항로가 없음
	// 544	The departure or destination is a logical (transportation) island, and there are more than one destination	탐색 전용	출발 또는 목적지가 논리적(교통) 섬이며, 목적지가 2개 이상인 경우
	// 545	No data requested	탐색 전용	요청한 데이터가 없음
}ROUTE_RESULT;


typedef enum {
	OPTIMAL_RESULT_SUCCESS = 0,					// 성공

	OPTIMAL_RESULT_FAILED = 100,				// 실패
	OPTIMAL_RESULT_FAILED_WRONG_PARAM = 101,	// 파라미터 오류
	OPTIMAL_RESULT_FAILED_SERVER_ERROR = 102,	// 서버 오류
}OPTIMAL_RESULT;


typedef enum
{
	TYPE_CLUSTER_DEVIDE_BY_COUNT, // 개수 분배
	TYPE_CLUSTER_DEVIDE_BY_DIST, // 거리 분배
	TYPE_CLUSTER_DEVIDE_BY_TIME, // 시간 분배
	TYPE_CLUSTER_DEVIDE_BY_LINK, // 링크 단위 분배
}TYPE_CLUSTER_DEVIDE;

typedef enum
{
	TYPE_TSP_ENDPOINT_NONE, // 지점 고정 없음
	TYPE_TSP_ENDPOINT_START, // 출발지 고정
	TYPE_TSP_ENDPOINT_END, // 도착지 고정
	TYPE_TSP_ENDPOINT_START_END, // 출-도착지 고정
	TYPE_TSP_ENDPOINT_RECURSIVE, // 출발지 회귀
}TYPE_TSP_ENDPOINT;

typedef enum
{
	TYPE_TSP_ALGORITHM_TSP_GA, // TSP-GA
	TYPE_TSP_ALGORITHM_COPY, // using input
	TYPE_TSP_ALGORITHM_MANHATTHAN, // manhatthan
	TYPE_TSP_ALGORITHM_TSP, // TSP
	TYPE_TSP_ALGORITHM_GOOGLEOR, // google-or
	TYPE_TSP_ALGORITHM_LKH, // lkh3
}TYPE_TSP_ALGORITHM_NOW;