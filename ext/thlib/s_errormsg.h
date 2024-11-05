/* copyright <happyteam@thinkware.co.kr> */
#ifndef _S_ERRORMSG_H__
#define _S_ERRORMSG_H__

/* Cache Rel */
#define ECACHE        (1) // cache fail
#define ECACHE_RANK   (2) // cache rank fail

/**
 *  외부에 공개된 Error Code
 */
#define MSG_SUCCESS                     (0x00)
#define MSG_SEARCH_FAILURE              (0x01)
#define MSG_USED_SMART_REROUTE          (0x02) // Smart Reroute
#define MSG_FAIL_USERSELECT             (0x03) // 사용자에의한 탐색 취소.
#define MSG_FAIL_CHECKSUM               (0x04) // Checksum Error
#define MSG_ALLFAIL_MULTI_POS_ROUTE     (0x05) // 모든 경로 실패 From Multi_Pos_Route.

#define MSG_FAIL_MEMORY_ALLOC           (0x11) // Memory Allocation Fail
#define MSG_FAIL_FILE_OPEN              (0x12)
#define MSG_FAIL_FILE_READ              (0x13)
#define MSG_FAIL_FILE_WRITE             (0x14)
#define MSG_FAIL_SOCKET_CONNECTION      (0x15) // Socket Connection Fail

#define MSG_WRONG_PRAMETER              (0x20) // Routing Parameter Error
#define MSG_FAIL_START_POINT            (0x21) // Invalid Point(Start)
#define MSG_FAIL_END_POINT              (0x22) // Invalid Point(End)
#define MSG_FAIL_VIA_POINT              (0x23) // Invalid Point(Via)
#define MSG_PROJECTION_FAIL             (0x24) // Link Projection
#define MSG_SEARCH_FAILURE_LENGTH       (0x25) // 탐색 가능한 거리 Over
#define MSG_EXCEED_EXP_LIMIT            (0x26) // 확장 Node 개수 Over
#define MSG_NO_EXPAND                   (0x27) // 확장실패.
#define MSG_NO_EXPAND_RTM               (0x28) // 확장실패(유고 또는 교통통제)
#define MSG_NO_EXPAND_CAR_RESTRICTION   (0x29) // 확장실패(차량 제원)
#define MSG_NO_EXPAND_TIME_RESTRICTION  (0x2A) // 확장실패(시간제한)
#define MSG_NO_FERRY                    (0x2B)
#define MSG_LOGICAL_ROAD_BLOCKING       (0x2C) // 논리적 섬도로.
#define MSG_NO_DATA                     (0x2D) // 요청한 정보 없음.

#if defined (_EXPANDFAIL_)
#define MSG_NO_EXPAND_1                 (0x30) // 확장실패. node를 담는 Heap Size가 Max일 때
#define MSG_NO_EXPAND_2                 (0x31) // 확장실패. node가 없을 때 (Search FW)
#define MSG_NO_EXPAND_3                 (0x32) // 확장실패. node가 없을 때 (Search FW in Main Loop RoutePlanL)
#define MSG_NO_EXPAND_4                 (0x33) // 확장실패. node가 없을 때 (Search BW)
#define MSG_NO_EXPAND_5                 (0x34) // 확장실패. node가 없을 때 (Search BW in Main Loop RoutePlanL)
#define MSG_NO_EXPAND_6                 (0x35) // 확장실패. node가 없을 때 (searchOnedirection)
#define MSG_NO_EXPAND_7                 (0x36) // 확장실패. node가 없을 때 (searchOnedirection in Main Loop)
#define MSG_NO_EXPAND_8                 (0x37) // 확장실패. check_force_restriction_fw에서 진행방향 제한없음 && Not 방향성 탐색 && HP_NOPASSING 일때
#define MSG_NO_EXPAND_9                 (0x38) // 확장실패. check_force_restriction_fw에서 진출입 제한 통과일 때
#define MSG_NO_EXPAND_10                (0x39) // 확장실패. check_force_restriction_bw에서 진향방향 뒤로, 양방향 링크 단점 유턴, 진출입 제한 통과일때
#define MSG_NO_EXPAND_11                (0x3A) // 확장실패. node가 없을 때 (Search FW in Main Loop RoutePlanU)
#define MSG_NO_EXPAND_12                (0x3B) // 확장실패. node가 없을 때 (Search BW in Main Loop RoutePlanU)
#endif

#define MSG_SELECT_CAR_SUCCESS          (0x40) // 배차성공 (다중출발지 탐색에서 교차로-거리조건에 의해 배차포함 대상차량 - ONDA)
#define MSG_SELECT_CAR_FAIL             (0x41) // 배차실패 (다중출발지 탐색에서 교차로-거리조건에 의해 배차제외 대상차량 - ONDA)

#define MSG_ROUTE_EXPIRATION            (0x50) // 기한만료 (다울지오인포 제공)
/**
 *  내부에서만 사용하는 Error Code (0x80 이상부터 사용)
 */
#define MSG_OPTION_VERSION_UPPER        (0x80) // version is upper than current file
#define MSG_OPTION_VERSION_LOWER        (0x81) // version is lower than current file

#endif
