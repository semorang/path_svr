/* copyright <happyteam@thinkware.co.kr> */
#ifndef _S_ERRORMSG_H__
#define _S_ERRORMSG_H__

/* Cache Rel */
#define ECACHE        (1) // cache fail
#define ECACHE_RANK   (2) // cache rank fail

/**
 *  �ܺο� ������ Error Code
 */
#define MSG_SUCCESS                     (0x00)
#define MSG_SEARCH_FAILURE              (0x01)
#define MSG_USED_SMART_REROUTE          (0x02) // Smart Reroute
#define MSG_FAIL_USERSELECT             (0x03) // ����ڿ����� Ž�� ���.
#define MSG_FAIL_CHECKSUM               (0x04) // Checksum Error
#define MSG_ALLFAIL_MULTI_POS_ROUTE     (0x05) // ��� ��� ���� From Multi_Pos_Route.

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
#define MSG_SEARCH_FAILURE_LENGTH       (0x25) // Ž�� ������ �Ÿ� Over
#define MSG_EXCEED_EXP_LIMIT            (0x26) // Ȯ�� Node ���� Over
#define MSG_NO_EXPAND                   (0x27) // Ȯ�����.
#define MSG_NO_EXPAND_RTM               (0x28) // Ȯ�����(���� �Ǵ� ��������)
#define MSG_NO_EXPAND_CAR_RESTRICTION   (0x29) // Ȯ�����(���� ����)
#define MSG_NO_EXPAND_TIME_RESTRICTION  (0x2A) // Ȯ�����(�ð�����)
#define MSG_NO_FERRY                    (0x2B)
#define MSG_LOGICAL_ROAD_BLOCKING       (0x2C) // ���� ������.
#define MSG_NO_DATA                     (0x2D) // ��û�� ���� ����.

#if defined (_EXPANDFAIL_)
#define MSG_NO_EXPAND_1                 (0x30) // Ȯ�����. node�� ��� Heap Size�� Max�� ��
#define MSG_NO_EXPAND_2                 (0x31) // Ȯ�����. node�� ���� �� (Search FW)
#define MSG_NO_EXPAND_3                 (0x32) // Ȯ�����. node�� ���� �� (Search FW in Main Loop RoutePlanL)
#define MSG_NO_EXPAND_4                 (0x33) // Ȯ�����. node�� ���� �� (Search BW)
#define MSG_NO_EXPAND_5                 (0x34) // Ȯ�����. node�� ���� �� (Search BW in Main Loop RoutePlanL)
#define MSG_NO_EXPAND_6                 (0x35) // Ȯ�����. node�� ���� �� (searchOnedirection)
#define MSG_NO_EXPAND_7                 (0x36) // Ȯ�����. node�� ���� �� (searchOnedirection in Main Loop)
#define MSG_NO_EXPAND_8                 (0x37) // Ȯ�����. check_force_restriction_fw���� ������� ���Ѿ��� && Not ���⼺ Ž�� && HP_NOPASSING �϶�
#define MSG_NO_EXPAND_9                 (0x38) // Ȯ�����. check_force_restriction_fw���� ������ ���� ����� ��
#define MSG_NO_EXPAND_10                (0x39) // Ȯ�����. check_force_restriction_bw���� ������� �ڷ�, ����� ��ũ ���� ����, ������ ���� ����϶�
#define MSG_NO_EXPAND_11                (0x3A) // Ȯ�����. node�� ���� �� (Search FW in Main Loop RoutePlanU)
#define MSG_NO_EXPAND_12                (0x3B) // Ȯ�����. node�� ���� �� (Search BW in Main Loop RoutePlanU)
#endif

#define MSG_SELECT_CAR_SUCCESS          (0x40) // �������� (��������� Ž������ ������-�Ÿ����ǿ� ���� �������� ������� - ONDA)
#define MSG_SELECT_CAR_FAIL             (0x41) // �������� (��������� Ž������ ������-�Ÿ����ǿ� ���� �������� ������� - ONDA)

#define MSG_ROUTE_EXPIRATION            (0x50) // ���Ѹ��� (�ٿ��������� ����)
/**
 *  ���ο����� ����ϴ� Error Code (0x80 �̻���� ���)
 */
#define MSG_OPTION_VERSION_UPPER        (0x80) // version is upper than current file
#define MSG_OPTION_VERSION_LOWER        (0x81) // version is lower than current file

#endif
