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
    ROUTE_RESULT_SUCCESS : 0,				//0	성공
    ROUTE_RESULT_FAILED : 1,				//1	탐색 실패	내부 오류에 의한 실패
    ROUTE_RESULT_FAILED_SAME_ROUTE : 2,		//2	스마트 재탐색 적용	기존 경로와 동일
    ROUTE_RESULT_FAILED_WRONG_PARAM : 10,	//10	잘 못된 파라미터	필수 파라미터 체크
    ROUTE_RESULT_FAILED_SET_MEMORY : 50,	//50	탐색 확장 관련 메모리 할당 오류	탐색 초기화 관련
    ROUTE_RESULT_FAILED_READ_DATA : 51,		//51	탐색 관련 데이터(지도, 옵션 등) 파일 읽기 실패	탐색 초기화 관련
    ROUTE_RESULT_FAILED_SET_START : 70,		//70	출발지가 프로젝션이 안되거나, 잘못된 출발지
    ROUTE_RESULT_FAILED_SET_VIA : 71,		//71	경유지가 프로젝션이 안되거나, 잘못된 경유지
    ROUTE_RESULT_FAILED_SET_END : 72,		//72	목적지가 프로젝션이 안되거나, 잘못된 목적지
    ROUTE_RESULT_FAILED_DIST_OVER : 90,		//90	탐색 가능 거리 초과	직선거리 15km 이내 허용
    ROUTE_RESULT_FAILED_TIME_OVER : 91,		//91	탐색 시간 초과	10초 이상
    ROUTE_RESULT_FAILED_NODE_OVER : 92,	    //92	확장 가능 Node 개수 초과
    ROUTE_RESULT_FAILED_EXPEND : 93,		//93	확장 실패
    RESULT_APPKEY_ERROR : 300,			    // 300	AppKey Error	공통	AppKey 인증 오류
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


// route avoids
const ROUTE_AVOIDS = {
    ROUTE_AVOID_NONE : 0,
	//ROUTE_AVOID_HIKING : 1, // 등산로
	//ROUTE_AVOID_TRAIL : 2, // 둘레길
	//ROUTE_AVOID_BIKE : 4, // 자전거
	//ROUTE_AVOID_CROSS : 8, // 종주길
    ROUTE_AVOID_ALLEY : 1,
    ROUTE_AVOID_PAVE : 2,
    ROUTE_AVOID_STAIRS : 4,
    ROUTE_AVOID_BRIDGE : 8,
    ROUTE_AVOID_ROCK : 16,
    ROUTE_AVOID_RIDGE : 32,
    ROUTE_AVOID_LADDER : 64,
    ROUTE_AVOID_ROPE : 128,
    ROUTE_AVOID_TATTERED : 256,
    ROUTE_AVOID_PALM : 512,
    ROUTE_AVOID_DECK : 1024,
}


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
            msg = "탐색 가능 거리 초과(직선거리 15km 이내 허용)";
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
    ROUTE_AVOIDS,

    // errors
    ERROR_CODES,
    ERROR_OPTIMAL_CODES,
    
    getErrMsg,
    getOptimalErrMsg,
    getDefaultPosition,
}
