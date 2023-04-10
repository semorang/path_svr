const ROUTE_RESULT_SUCCESS = 0;				//0	성공
const ROUTE_RESULT_FAILED = 1;				//1	탐색 실패	내부 오류에 의한 실패
const ROUTE_RESULT_FAILED_SAME_ROUTE = 2;		//2	스마트 재탐색 적용	기존 경로와 동일
const ROUTE_RESULT_FAILED_WRONG_PARAM = 10;	//10	잘 못된 파라미터	필수 파라미터 체크
const ROUTE_RESULT_FAILED_SET_MEMORY = 50;	//50	탐색 확장 관련 메모리 할당 오류	탐색 초기화 관련
const ROUTE_RESULT_FAILED_READ_DATA = 51;		//51	탐색 관련 데이터(지도, 옵션 등) 파일 읽기 실패	탐색 초기화 관련
const ROUTE_RESULT_FAILED_SET_START = 70;		//70	출발지가 프로젝션이 안되거나, 잘못된 출발지
const ROUTE_RESULT_FAILED_SET_VIA = 71;		//71	경유지가 프로젝션이 안되거나, 잘못된 경유지
const ROUTE_RESULT_FAILED_SET_END = 72;		//72	목적지가 프로젝션이 안되거나, 잘못된 목적지
const ROUTE_RESULT_FAILED_DIST_OVER = 90;		//90	탐색 가능 거리 초과	직선거리 15km 이내 허용
const ROUTE_RESULT_FAILED_TIME_OVER = 91;		//91	탐색 시간 초과	10초 이상
const ROUTE_RESULT_FAILED_NODE_OVER = 92;		//92	확장 가능 Node 개수 초과
const ROUTE_RESULT_FAILED_EXPEND = 93;		//93	확장 실패

function getMsg(code) {
    var msg;

    switch(code) {
        case ROUTE_RESULT_SUCCESS:
            msg = "성공";
            break;
        case ROUTE_RESULT_FAILED:
            msg = "탐색 실패(내부 오류에 의한 실패)";
            break;
        case ROUTE_RESULT_FAILED_SAME_ROUTE:
            msg = "스마트 재탐색 적용(기존 경로와 동일)";
        case ROUTE_RESULT_FAILED_WRONG_PARAM:
            msg = "잘못된 파라미터(필수 파라미터 체크)";
            break;
        case ROUTE_RESULT_FAILED_SET_MEMORY:
            msg = "탐색 확장 관련 메모리 할당 오류(탐색 초기화 관련)";
            break;
        case ROUTE_RESULT_FAILED_READ_DATA:
            msg = "탐색 관련 데이터(지도, 옵션 등) 파일 읽기 실패(탐색 초기화 관련)";
            break;
        case ROUTE_RESULT_FAILED_SET_START:
            msg = "출발지가 프로젝션이 안되거나, 잘못된 출발지";
            break;
        case ROUTE_RESULT_FAILED_SET_VIA:
            msg = "경유지가 프로젝션이 안되거나, 잘못된 경유지";
            break;
        case ROUTE_RESULT_FAILED_SET_END:
            msg = "목적지가 프로젝션이 안되거나, 잘못된 목적지";
            break;
        case ROUTE_RESULT_FAILED_DIST_OVER:
            msg = "탐색 가능 거리 초과(직선거리 15km 이내 허용)";
            break;
        case ROUTE_RESULT_FAILED_TIME_OVER:	
            msg = "탐색 시간 초과(10초 이상)";
            break;
        case ROUTE_RESULT_FAILED_NODE_OVER:	
            msg = "확장 가능 Node 개수 초과";
            break;
        case ROUTE_RESULT_FAILED_EXPEND:
            msg = "확장 실패";
            break;
        default:
            msg = "실패(알수 없는 오류)";
            break;
    }

    return msg;
}


module.exports = {
    ROUTE_RESULT_SUCCESS, ROUTE_RESULT_FAILED, ROUTE_RESULT_FAILED_SAME_ROUTE, ROUTE_RESULT_FAILED_WRONG_PARAM, ROUTE_RESULT_FAILED_SET_MEMORY,
    ROUTE_RESULT_FAILED_READ_DATA, ROUTE_RESULT_FAILED_SET_START, ROUTE_RESULT_FAILED_SET_VIA, ROUTE_RESULT_FAILED_SET_END, ROUTE_RESULT_FAILED_DIST_OVER, 
    ROUTE_RESULT_FAILED_TIME_OVER, ROUTE_RESULT_FAILED_NODE_OVER, ROUTE_RESULT_FAILED_EXPEND,
    getMsg
}

// export {
//     getMsg
// }