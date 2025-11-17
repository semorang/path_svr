// errors
// const ROUTE_RESULT_SUCCESS = 0;				//0	성공
// const ROUTE_RESULT_FAILED = 1;				//1	탐색 실패	내부 오류에 의한 실패
// const ROUTE_RESULT_FAILED_SAME_ROUTE = 2;		//2	스마트 재탐색 적용	기존 경로와 동일
// const ROUTE_RESULT_FAILED_WRONG_PARAM = 10;	//10	잘 못된 파라미터	필수 파라미터 체크
// const ROUTE_RESULT_FAILED_SET_MEMORY = 50;	//50	탐색 확장 관련 메모리 할당 오류	탐색 초기화 관련
// const ROUTE_RESULT_FAILED_READ_DATA = 51;		//51	탐색 관련 데이터(지도, 옵션 등) 파일 읽기 실패	탐색 초기화 관련
// const ROUTE_RESULT_FAILED_SET_START = 70;		//70	출발지가 프로젝션이 안되거나, 잘못된 출발지
// const ROUTE_RESULT_FAILED_SET_VIA = 71;		//71	경유지가 프로젝션이 안되거나, 잘못된 경유지
// const ROUTE_RESULT_FAILED_SET_END = 72;		//72	목적지가 프로젝션이 안되거나, 잘못된 목적지
// const ROUTE_RESULT_FAILED_DIST_OVER = 90;		//90	탐색 가능 거리 초과	직선거리 15km 이내 허용
// const ROUTE_RESULT_FAILED_TIME_OVER = 91;		//91	탐색 시간 초과	10초 이상
// const ROUTE_RESULT_FAILED_NODE_OVER = 92;		//92	확장 가능 Node 개수 초과
// const ROUTE_RESULT_FAILED_EXPEND = 93;		//93	확장 실패
// const RESULT_APPKEY_ERROR = 300;			// 300	AppKey Error	공통	AppKey 인증 오류
const ERROR_CODES = {
    ROUTE_RESULT_SUCCESS : 0,				        //0	성공
    ROUTE_RESULT_FAILED : 1,				        //1	탐색 실패	내부 오류에 의한 실패
    ROUTE_RESULT_FAILED_SAME_ROUTE : 2,		        //2	스마트 재탐색 적용	기존 경로와 동일
    ROUTE_RESULT_FAILED_MULTI_POS_ROUTE_ALL : 5,	//5		다중 경로 탐색 모두 실패
	ROUTE_RESULT_FAILED_MULTI_POS_ROUTE : 6,		//6		다중 경로 탐색 실패
    ROUTE_RESULT_FAILED_WRONG_PARAM : 10,	        //10	잘 못된 파라미터	필수 파라미터 체크
    ROUTE_RESULT_FAILED_SET_MEMORY : 50,	        //50	탐색 확장 관련 메모리 할당 오류	탐색 초기화 관련
    ROUTE_RESULT_FAILED_READ_DATA : 51,		        //51	탐색 관련 데이터(지도, 옵션 등) 파일 읽기 실패	탐색 초기화 관련
    ROUTE_RESULT_FAILED_SET_START : 70,		        //70	출발지가 프로젝션이 안되거나, 잘못된 출발지
    ROUTE_RESULT_FAILED_SET_VIA : 71,		        //71	경유지가 프로젝션이 안되거나, 잘못된 경유지
    ROUTE_RESULT_FAILED_SET_END : 72,		        //72	목적지가 프로젝션이 안되거나, 잘못된 목적지
    ROUTE_RESULT_FAILED_DIST_OVER : 90,		        //90	탐색 가능 거리 초과	직선거리 30km 이내 허용
    ROUTE_RESULT_FAILED_TIME_OVER : 91,		        //91	탐색 시간 초과	10초 이상
    ROUTE_RESULT_FAILED_NODE_OVER : 92,	            //92	확장 가능 Node 개수 초과
    ROUTE_RESULT_FAILED_EXPEND : 93,		        //93	확장 실패
    RESULT_APPKEY_ERROR : 300,			            // 300	AppKey Error	공통	AppKey 인증 오류

    ROUTE_RESULT_FAILED_COURSE : 600,					//600	코스 탐색 실패
	ROUTE_RESULT_FAILED_COURSE_ID : 601,				//601	코스 ID 검색 실패
	ROUTE_RESULT_FAILED_COURSE_TYPE : 602,				//602	코스 TYPE 검색 실패

    TMS_RESULT_FAILED : 10000,							//10000	클러스터링 실패
	TMS_RESULT_FAILED_LOOP : 10001,						//10001 분배 재계산 한계 횟수 초과
	TMS_RESULT_FAILED_MATCHING_LIMIT : 10002,			//10002 지정된 분배 수량내에서 배정이 실패함
	TMS_RESULT_FAILED_MATCHING_DEVIATION : 10003,		//10003 지정된 편차 내의 배정이 실패함
}


const ERROR_OPTIMAL_CODES = {
    OPTIMAL_RESULT_SUCCESS : 0,				//0 성공
    OPTIMAL_RESULT_NO_RESULT : 100,			//100	결과 없음
    OPTIMAL_RESULT_WRONG_PARAM : 101,		//101	파라미터 오류
    OPTIMAL_RESULT_SERVER_ERROR : 102,      //102   서버오류
}

// route options
const ROUTE_OPTIONS = {
    ROUTE_OPT_SHORTEST : 0, // 최단거리
    ROUTE_OPT_RECOMMENDED : 1, // 추천
    ROUTE_OPT_COMFORTABLE : 2, // 편안한
    ROUTE_OPT_FASTEST : 3, // 최소시간
    ROUTE_OPT_MAINROAD : 4, // 큰길
    ROUTE_OPT_PEDESTRIAN : 5, // 보행자 전용
    ROUTE_OPT_TRAIL : 6, // 둘레길 전용
    ROUTE_OPT_BIKE : 7, // 자전거 전용
    ROUTE_OPT_AUTOMATION : 8, // 자율주행 전용
}

// mobility type
const MOBILITY_TYPE = {
    TYPE_MOBILITY_PEDESTRIAN : 0, // 보행자
	TYPE_MOBILITY_BICYCLE : 1, // 자전거
	TYPE_MOBILITY_MOTORCYCLE : 2, // 오토바이
	TYPE_MOBILITY_VEHICLE : 3, // 자동차
	TYPE_MOBILITY_AUTONOMOUS : 4, // 자율주행
	TYPE_MOBILITY_TRUCK : 5, // 트럭
	TYPE_MOBILITY_EMERGENCY : 6, // 긴급
	TYPE_MOBILITY_COUNT : 7,
}

const LINK_MATCH_TYPE = {
	TYPE_LINK_MATCH_NONE : 0, // 미지정
	TYPE_LINK_MATCH_CARSTOP : 1, // 차량 승하자
	TYPE_LINK_MATCH_CARSTOP_EX : 2, // 차량 승하자 + 단지내도로(건물입구점 재확인시, 단지도로에 매칭된 입구점을 확인하기 위해 포함)
	TYPE_LINK_MATCH_CARENTRANCE : 3, // 차량 진출입 전용
	TYPE_LINK_MATCH_FOR_TABLE : 4, // 방문지 테이블용 , 2차선 이상만
	TYPE_LINK_MATCH_FOR_HD : 5, // P2P HD 경로 탐색 용, sd-hd 매칭되는 링크만 선택
	TYPE_LINK_MATCH_FOR_FOREST : 6, // 숲길 경로 링크 선택, 숲길(우선), 보행자길
	TYPE_LINK_MATCH_FOR_BICYCLE : 7, // PM(킥보드, 자전거)
}

// route avoids
const ROUTE_AVOIDS_TRK = {
    ROUTE_AVOID_NONE : 0, // 없음
	ROUTE_AVOID_HIKING : 1, // 등산로
	ROUTE_AVOID_TRAIL : 2, // 둘레길
	ROUTE_AVOID_BIKE : 4, // 자전거
	ROUTE_AVOID_CROSS : 8, // 종주길
    ROUTE_AVOID_RECOMMEND : 16, // 추천코스
	ROUTE_AVOID_MTB : 32, // MTB코스
	ROUTE_AVOID_POPULAR : 64, // 인기코스

    ROUTE_AVOID_ALLEY : 256,
    ROUTE_AVOID_PAVE : 512,
    ROUTE_AVOID_STAIRS : 1024,
    ROUTE_AVOID_BRIDGE : 2048,
    ROUTE_AVOID_ROCK : 4096,
    ROUTE_AVOID_RIDGE : 8192,
    ROUTE_AVOID_LADDER : 16384,
    ROUTE_AVOID_ROPE : 32768,
    ROUTE_AVOID_TATTERED : 65536,
    ROUTE_AVOID_PALM : 131072,
    ROUTE_AVOID_DECK : 262144,
};

const ROUTE_AVOIDS_PED = {
	ROUTE_AVOID_PED_NODE : 0, // 미정의
    ROUTE_AVOID_PED_SLOP : 1, // 경사로
	ROUTE_AVOID_PED_STAIRS : 2, // 계단
	ROUTE_AVOID_PED_ESCALATOR: 4, // 에스컬레이터
	ROUTE_AVOID_PED_STAIRS_ESCALATOR : 8, // 계단/에스컬레이터
	ROUTE_AVOID_PED_ELEVATOR : 16, // 엘리베이터
	ROUTE_AVOID_PED_CONNECTION: 32, // 단순연결로
	ROUTE_AVOID_PED_CROSSWALK : 64, // 횡단보도
    ROUTE_AVOID_PED_MOVINGWALK : 128, // 무빙워크
    ROUTE_AVOID_PED_STEPPINGSTONES : 256, // 징검다리
	ROUTE_AVOID_PED_VIRTUAL : 512, // 의사횡단

	ROUTE_AVOID_PED_CAVE : 2048, // 토끼굴
	ROUTE_AVOID_PED_UNDERPASS : 4096, // 지하보도
	ROUTE_AVOID_PED_FOOTBRIDGE : 8192, // 육교
	ROUTE_AVOID_PED_OVERPASS : 16384, // 고가도로
	ROUTE_AVOID_PED_BRIDGE : 32768, // 교량
	ROUTE_AVOID_PED_SUBWAY : 65536, // 지하철역
	ROUTE_AVOID_PED_RAILROAD : 131072, // 철도
	ROUTE_AVOID_PED_BUSSTATION : 262144, // 중앙버스정류장
	ROUTE_AVOID_PED_UNDERGROUNDMALL : 524288, // 지하상가
	ROUTE_AVOID_PED_THROUGHBUILDING : 1048576, // 건물관통도로
	ROUTE_AVOID_PED_COMPLEXPARK : 2097152, // 단지도로_공원
	ROUTE_AVOID_PED_COMPLEXAPT : 4194304, // 단지도로_주거시설
	ROUTE_AVOID_PED_COMPLEXTOUR : 8388608, // 단지도로_관광지
	ROUTE_AVOID_PED_COMPLEXETC : 16777216, // 단지도로_기타
};

// route avoids
const ROUTE_AVOIDS_VEH = {
    ROUTE_AVOID_NONE : 0, // 없음
	ROUTE_AVOID_SHORTTURN : 1, // 짧은회전
    
    // ROUTE_AVOID_VEH_ : 2, // 
	// ROUTE_AVOID_VEH_: 4, // 
	// ROUTE_AVOID_VEH_ : 8, // 
	// ROUTE_AVOID_VEH_ : 16, // 
	// ROUTE_AVOID_VEH_: 32, // 
	// ROUTE_AVOID_VEH_ : 64, // 
    // ROUTE_AVOID_VEH_ : 128, // 
    // ROUTE_AVOID_VEH_ : 256, // 
	// ROUTE_AVOID_VEH_ : 512, // 
	// ROUTE_AVOID_VEH_ : 2048, // 
	// ROUTE_AVOID_VEH_ : 4096, // 
	// ROUTE_AVOID_VEH_ : 8192, // 
	// ROUTE_AVOID_VEH_ : 16384, // 
	// ROUTE_AVOID_VEH_ : 32768, // 
	// ROUTE_AVOID_VEH_ : 65536, // 
	// ROUTE_AVOID_VEH_ : 131072, // 
	// ROUTE_AVOID_VEH_ : 262144, // 
	// ROUTE_AVOID_VEH_ : 524288, // 
	// ROUTE_AVOID_VEH_ : 1048576, // 
	// ROUTE_AVOID_VEH_ : 2097152, // 
	// ROUTE_AVOID_VEH_ : 4194304, // 
	// ROUTE_AVOID_VEH_ : 8388608, // 
	// ROUTE_AVOID_VEH_ : 16777216, // 
};

function getErrMsg(code) {
    var msg;

    switch(code) {
        case ERROR_CODES.ROUTE_RESULT_SUCCESS:
            msg = "성공";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED:
            msg = "탐색 실패(내부 오류에 의한 실패)";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_SAME_ROUTE:
            msg = "스마트 재탐색 적용(기존 경로와 동일)";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_MULTI_POS_ROUTE_ALL:
            msg = "다중 경로 탐색 실패(모든 경로 탐색)";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_MULTI_POS_ROUTE:
            msg = "다중 경로 탐색 실패(일부 경로 탐색)";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_WRONG_PARAM:
            msg = "잘못된 파라미터(필수 파라미터 체크)";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_SET_MEMORY:
            msg = "탐색 확장 관련 메모리 할당 오류(탐색 초기화 관련)";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_READ_DATA:
            msg = "탐색 관련 데이터(지도, 옵션 등) 파일 읽기 실패(탐색 초기화 관련)";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_SET_START:
            msg = "출발지가 프로젝션이 안되거나, 잘못된 출발지";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_SET_VIA:
            msg = "경유지가 프로젝션이 안되거나, 잘못된 경유지";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_SET_END:
            msg = "목적지가 프로젝션이 안되거나, 잘못된 목적지";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_DIST_OVER:
            msg = "탐색 가능 거리 초과";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_TIME_OVER:	
            msg = "탐색 시간 초과(10초 이상)";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_NODE_OVER:	
            msg = "확장 가능 Node 개수 초과";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_EXPEND:
            msg = "확장 실패";
            break;
        case ERROR_CODES.RESULT_APPKEY_ERROR:
            msg = "AppKey 인증 오류";	// 300	AppKey Error	공통	AppKey 인증 오류
            break;

        case ERROR_CODES.ROUTE_RESULT_FAILED_COURSE:	
            msg = "코스 탐색 실패";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_COURSE_ID:
            msg = "코스 ID 검색 실패";
            break;
        case ERROR_CODES.ROUTE_RESULT_FAILED_COURSE_TYPE:
            msg = "코스 TYPE 검색 실패";
            break;

        case ERROR_CODES.TMS_RESULT_FAILED:
            msg = "클러스터링 실패";
            break;
        case ERROR_CODES.TMS_RESULT_FAILED_LOOP:
            msg = "분배 재계산 한계 횟수 초과";
            break;
        case ERROR_CODES.TMS_RESULT_FAILED_MATCHING_LIMIT:
            msg = "지정된 분배 수량내에서 배정이 실패함";
            break;
        case ERROR_CODES.TMS_RESULT_FAILED_MATCHING_DEVIATION:
            msg = "지정된 편차 내의 배정이 실패함";
            break;

        default:
            msg = "실패(알수 없는 오류)";
            break;
    }

    return msg;
}


function getDefaultPosition(target) {
    var pos;

    if (target === "kakaovx") {
        var newPos = {
            departure : "126.521857543595,33.290872116984", // 치유의숲
            destination : "126.52000827044,33.36144251951", // 한라산
            center : "126.5314590086,33.499659893321", // 제주도청
        }
        pos = newPos;
    } else if (target === "inavi") {
        var newPos = {
            departure : "127.108950065049,37.404237963557", // 성남판교행복주택
            destination : "127.111989747142,37.387052287024", // 신백현초등학교
            center : "127.110767364421,37.402196784342", // 아이나비시스템즈(판교)
        }
        pos = newPos;
    } else if (target === "p2p") {
        var newPos = {
            departure : "128.452528360378,35.694445345774", // 대구포산초등학교
            destination : "128.457019511114,35.7043620533", // 대구경북과학기술원
            center : "128.456012633768,35.691998971295", // 대구테크노폴리스지구
        }
        pos = newPos;
    }

    return pos;
}


function getOptimalErrMsg(code) {
    var msg;

    switch(code) {
        case ERROR_OPTIMAL_CODES.OPTIMAL_RESULT_SUCCESS:
            msg = "성공";
            break;
        case ERROR_OPTIMAL_CODES.OPTIMAL_RESULT_NO_RESULT:
            msg = "결과 없음";
            break;
        case ERROR_OPTIMAL_CODES.OPTIMAL_RESULT_WRONG_PARAM:
            msg = "잘못된 파라미터(필수 파라미터 체크)";
            break;
        case ERROR_OPTIMAL_CODES.OPTIMAL_RESULT_SERVER_ERROR:
            msg = "서버 오류";
            break;
        default:
            msg = "실패(알수 없는 오류)";
            break;
    }

    return msg;
}



module.exports = {
    // route options
    ROUTE_OPTIONS,

    // route avoids
    ROUTE_AVOIDS_TRK,
    ROUTE_AVOIDS_PED,
    ROUTE_AVOIDS_VEH,

    // type
    MOBILITY_TYPE,
    LINK_MATCH_TYPE,

    // errors
    ERROR_CODES,
    ERROR_OPTIMAL_CODES,
    
    getErrMsg,
    getOptimalErrMsg,
    getDefaultPosition,
}
