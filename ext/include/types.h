#pragma once

#include <vector>
#include <string>

#include "../shp/shpio.h"

using namespace std;

// �̵�ü ����
typedef enum {
	TYPE_MOBILITY_PEDESTRIAN, // ������
	TYPE_MOBILITY_BICYCLE, // ������
	TYPE_MOBILITY_MOTORCYCLE, // �������
	TYPE_MOBILITY_VEHICLE, // �ڵ���
	TYPE_MOBILITY_AUTONOMOUS, // ��������
	TYPE_MOBILITY_COUNT,
}TYPE_MOBILITY_DATA;


// ������ ���� ����
typedef enum {
	TYPE_DATA_NONE = 0, // ������
	TYPE_DATA_NAME, // ��Ī����
	TYPE_DATA_MESH, // �޽�
	TYPE_DATA_TREKKING, // ����
	TYPE_DATA_PEDESTRIAN, // ������/������
	TYPE_DATA_VEHICLE, // �ڵ���
	TYPE_DATA_BUILDING, // �ǹ�
	TYPE_DATA_COMPLEX, // ����
	TYPE_DATA_ENTRANCE, // �Ա���
	TYPE_DATA_TRAFFIC, // ��������
	TYPE_DATA_MOUNTAIN, // ��ٿ����
	TYPE_DATA_COURSE, // �ڽ�����
	TYPE_DATA_EXTEND, // Ȯ������
	TYPE_DATA_COUNT,
}TYPE_DATA;

// ������ Ȯ���� ����
typedef enum {
	TYPE_EXEC_NONE = 0, // ������
	TYPE_EXEC_NAME, // ��Ī����
	TYPE_EXEC_MESH, // �޽�
	TYPE_EXEC_LINK, // ��ũ
	TYPE_EXEC_NODE, // ���
	TYPE_EXEC_NETWORK, // ��Ʈ��ũ
	TYPE_EXEC_POLYGON, // ������
	TYPE_EXEC_ENTRANCE, // �Ա���
	TYPE_EXEC_TRAFFIC, // ��������
	TYPE_EXEC_COURSE, // �ڽ�����
	TYPE_EXEC_INDEX, // �ε�������
	TYPE_EXEC_EXTEND, // Ȯ������
	TYPE_EXEC_COUNT,
}TYPE_EXEC_DATA;


// Ű ������ ����
typedef enum {
	TYPE_KEY_NONE = 0, // ������
	TYPE_KEY_NODE, // ���
	TYPE_KEY_LINK, // ��ũ
	TYPE_KEY_COUNT,
}TYPE_KEY;


// ��ũ ������ ���� ����
typedef enum {
	TYPE_LINK_DATA_NONE = 0, // ������
	TYPE_LINK_DATA_TREKKING, // ����
	TYPE_LINK_DATA_PEDESTRIAN, // ������/������
	TYPE_LINK_DATA_VEHICLE, // �ڵ���
	TYPE_LINK_DATA_COUNT,
}TYPE_LINK_DATA;


// ��� ������ ���� ����
typedef enum {
	TYPE_NODE_DATA_NONE = 0, // ������
	TYPE_NODE_DATA_TREKKING, // ����
	TYPE_NODE_DATA_PEDESTRIAN, // ������/������
	TYPE_NODE_DATA_VEHICLE, // �ڵ���
	TYPE_NODE_DATA_COUNT,
}TYPE_NODE_DATA;


// ���� ������
typedef enum {
	TYPE_TRE_NONE = 0, // ������(�ȱ�)
	TYPE_TRE_HIKING, // ����(���)
	TYPE_TRE_TRAIL, // �ѷ���(�ڽ�)
	TYPE_TRE_BIKE, // �����ű�(������)
	TYPE_TRE_CROSS, // ���ֱ�(�ڽ�ID)
	TYPE_TRE_RECOMMENDED, // ��õ��
	TYPE_TRE_MTB, // MTB�ڽ�
	TYPE_TRE_POPULAR, // �α��ڽ�
	TYPE_TRE_COUNT,
}TYPE_TREKKING;


// ������/������ ������
typedef enum {
	TYPE_WALK_SIDE = 1, // ��������
	TYPE_WALK_WITH_CAR, // ������뵵��
	TYPE_WALK_WITH_BYC, // ���������뵵�� -- �켱�� ���� �Ұ� �������� ��������
	TYPE_WALK_ONLY, // �������뵵��
	TYPE_WALK_THROUGH, //�����൵��
}TYPE_WALK;


typedef enum {
	TYPE_BYC_ONLY = 1, // ������ ����
	TYPE_BYC_WITH_CAR, // ������/������� �����ŵ���
	TYPE_BYC_WITH_WALK, // ���൵�� -- �켱�� ������ ��� �Ұ� �������� ��������
}TYPE_BICYCLE;


typedef enum {
	TYPE_NODE_NONE = 0, // ������
	TYPE_NODE_COROSS, // ����÷
	TYPE_NODE_END, // ����
	TYPE_NODE_DUMMY, // ������
	TYPE_NODE_EDGE, // ��ȹ������
	TYPE_NODE_ATTRIBUTE, // �Ӽ���ȭ��
	TYPE_NODE_SUBWAY, // ����ö ������
	TYPE_NODE_UNDERPASS, // ���ϵ� ������
	TYPE_NODE_UNDERGROUND_MALL, // ���ϻ� ������
	TYPE_NODE_BUILDING, // �ǹ� ������
}TYPE_NODE;


typedef enum {
    TYPE_GATE_NONE, // ������
    TYPE_GATE_SLOP, // ����
	TYPE_GATE_STAIRS, // ���
	TYPE_GATE_ESCALATOR, // �����÷�����
	TYPE_GATE_STAIRS_ESCALATOR, // ���/�����÷�����
	TYPE_GATE_ELEVATOR, // ����������
	TYPE_GATE_CONNECTION, // �ܼ������
	TYPE_GATE_CROSSWALK, // Ⱦ�ܺ���
    TYPE_GATE_MOVINGWALK, // ������ũ
    TYPE_GATE_STEPPINGSTONES, // ¡�˴ٸ�
	TYPE_GATE_VIRTUAL, // �ǻ�Ⱦ��
}TYPE_GATE;


typedef enum {
    TYPE_OBJECT_NONE, // ������
	TYPE_OBJECT_CAVE, // �䳢��
	TYPE_OBJECT_UNDERPASS, // ���Ϻ���
	TYPE_OBJECT_FOOTBRIDGE, // ����
	TYPE_OBJECT_OVERPASS, // ������
	TYPE_OBJECT_BRIDGE, // ����
	TYPE_OBJECT_SUBWAY, // ����ö��
	TYPE_OBJECT_RAILROAD, // ö��
	TYPE_OBJECT_BUSSTATION, // �߾ӹ���������
	TYPE_OBJECT_UNDERGROUNDMALL, // ���ϻ�
	TYPE_OBJECT_THROUGHBUILDING, // �ǹ����뵵��
	TYPE_OBJECT_COMPLEXPARK, // ��������_����
	TYPE_OBJECT_COMPLEXAPT, // ��������_�ְŽü�
	TYPE_OBJECT_COMPLEXTOUR, // ��������_������
	TYPE_OBJECT_COMPLEXETC, // ��������_��Ÿ
}TYPE_OBJECT;


typedef enum
{
	PASS_CODE_ENABLE,
	PASS_CODE_UTURN,
	PASS_CODE_DISABLE,
}PASS_CODE;


typedef enum {
	ROUTE_TYPE_NONE = 0, // ������
	ROUTE_TYPE_TREKKING, // ����
	ROUTE_TYPE_PEDESTRIAN, // ������
	ROUTE_TYPE_BIKE, // ������
	ROUTE_TYPE_KICKBOARD, // ű����
	ROUTE_TYPE_MOTOCYCLE, // ���ͻ���Ŭ
	ROUTE_TYPE_VEHICLE, // ����
}ROUTE_TYPE;


typedef enum {
	ROUTE_OPT_SHORTEST = 0, // �ִܰŸ�
	ROUTE_OPT_RECOMMENDED, // ��õ
	ROUTE_OPT_COMFORTABLE, // �����
	ROUTE_OPT_FASTEST, // �ּҽð�(����)
	ROUTE_OPT_MAINROAD, // ū��
	ROUTE_OPT_COUNT,
}ROUTE_OPTION;


typedef enum {
	//ROUTE_AVOID_NONE = 0,
	//ROUTE_AVOID_HIKING = 0x00000001, // ����
	//ROUTE_AVOID_TRAIL = 0x00000002, // �ѷ���
	//ROUTE_AVOID_BIKE = 0x00000004, // ������
	//ROUTE_AVOID_THEME = 0x00000008, // �׸���
	//ROUTE_AVOID_CROSS = 0x00000010, // ���ֱ�

	//0:������, 1 : ����, 2 : �ѷ���, 3 : �����ű�, 4 : �����ڽ�, 5 : ��õ�ڽ�, 6 : MTB�ڽ�, 7 : �α��ڽ�
	ROUTE_AVOID_NONE = 0,
	ROUTE_AVOID_HIKING = 0x00000001, // ����
	ROUTE_AVOID_TRAIL = 0x00000002, // �ѷ���
	ROUTE_AVOID_BIKE = 0x00000004, // ������
	ROUTE_AVOID_CROSS = 0x00000008, // �����ڽ�
	ROUTE_AVOID_RECOMMEND = 0x00000010, // ��õ�ڽ�
	ROUTE_AVOID_MTB = 0x00000020, // MTB�ڽ�
	ROUTE_AVOID_POPULAR = 0x00000040, // �α��ڽ�

	ROUTE_AVOID_ALLEY = 0x00000100,
	ROUTE_AVOID_PAVE = 0x00000200,
	ROUTE_AVOID_STAIRS = 0x00000400,
	ROUTE_AVOID_BRIDGE = 0x00000800,
	ROUTE_AVOID_ROCK = 0x00001000,
	ROUTE_AVOID_RIDGE = 0x00002000,
	ROUTE_AVOID_LADDER = 0x00004000,
	ROUTE_AVOID_ROPE = 0x00008000,
	ROUTE_AVOID_TATTERED = 0x00010000,
	ROUTE_AVOID_PALM = 0x00020000,
	ROUTE_AVOID_DECK = 0x00040000,

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
	// 0:������, 1 : ����, 2 : �ѷ���, 3 : �����ű�, 4 : �����ڽ�
	// 1:���ֱ�(1), 2:�����(2), 3:���(4), 4:����(8), 5:�ϸ�(16), 6:����(32), 7:��ٸ�(64), 8:����(128), 9:�ʴ���(256), 10:���ڼ���Ʈ(512), 11:��ũ�ε�(1024)
}ROUTE_AVOID;

typedef enum {
	ROUTE_AVOID_PED_NODE = 0, // ������
    ROUTE_AVOID_PED_SLOP = 1, // ����
	ROUTE_AVOID_PED_STAIRS = 2, // ���
	ROUTE_AVOID_PED_ESCALATOR= 4, // �����÷�����
	ROUTE_AVOID_PED_STAIRS_ESCALATOR = 8, // ���/�����÷�����
	ROUTE_AVOID_PED_ELEVATOR = 16, // ����������
	ROUTE_AVOID_PED_CONNECTION= 32, // �ܼ������
	ROUTE_AVOID_PED_CROSSWALK = 64, // Ⱦ�ܺ���
    ROUTE_AVOID_PED_MOVINGWALK = 128, // ������ũ
    ROUTE_AVOID_PED_STEPPINGSTONES = 256, // ¡�˴ٸ�
	ROUTE_AVOID_PED_VIRTUAL = 512, // �ǻ�Ⱦ��

	ROUTE_AVOID_PED_CAVE = 2048, // �䳢��
	ROUTE_AVOID_PED_UNDERPASS = 4096, // ���Ϻ���
	ROUTE_AVOID_PED_FOOTBRIDGE = 8192, // ����
	ROUTE_AVOID_PED_OVERPASS = 16384, // ������
	ROUTE_AVOID_PED_BRIDGE = 32768, // ����
	ROUTE_AVOID_PED_SUBWAY = 65536, // ����ö��
	ROUTE_AVOID_PED_RAILROAD = 131072, // ö��
	ROUTE_AVOID_PED_BUSSTATION = 262144, // �߾ӹ���������
	ROUTE_AVOID_PED_UNDERGROUNDMALL = 524288, // ���ϻ�
	ROUTE_AVOID_PED_THROUGHBUILDING = 1048576, // �ǹ����뵵��
	ROUTE_AVOID_PED_COMPLEXPARK = 2097152, // ��������_����
	ROUTE_AVOID_PED_COMPLEXAPT = 4194304, // ��������_�ְŽü�
	ROUTE_AVOID_PED_COMPLEXTOUR = 8388608, // ��������_������
	ROUTE_AVOID_PED_COMPLEXETC = 16777216, // ��������_��Ÿ
}ROUTE_PED_AVOID;


// ��ũ ���� Ÿ��
typedef enum {
	TYPE_SUBINFO_NONE = 0, // ������
	TYPE_SUBINFO_FTYPE, // �ü��� Ÿ��
	TYPE_SUBINFO_GTYPE, // ���Է� Ÿ��
}TYPE_SUBINFO;


// �ǹ� ���� �ڵ�
typedef enum {
	TYPE_BUILDING_NOT = 0,//������
	TYPE_BUILDING_APT, //����Ʈ
	TYPE_BUILDING_SCH, //�б�
	TYPE_BUILDING_OFT, //���ǽ���
	TYPE_BUILDING_B01, //�������
	TYPE_BUILDING_B02, //ġ�ȱ��
	TYPE_BUILDING_B03, //����
	TYPE_BUILDING_B04, //�������
	TYPE_BUILDING_B05, //��б��
	TYPE_BUILDING_B06, //�������
	TYPE_BUILDING_B07, //��ȭ
	TYPE_BUILDING_B08, //����
	TYPE_BUILDING_B09, //����
	TYPE_BUILDING_B10, //������
	TYPE_BUILDING_B11, //����
	TYPE_BUILDING_B12, //����
	TYPE_BUILDING_B13, //���
	TYPE_BUILDING_B14, //����ü�
	TYPE_BUILDING_B15, //�ڵ�������
	TYPE_BUILDING_B16, //����ü�
	TYPE_BUILDING_B17, //����ö�ü�
	TYPE_BUILDING_B18, //���νü�
	TYPE_BUILDING_B19, //���ð���
	TYPE_BUILDING_ETC, //���ÿܰǹ�
	TYPE_BUILDING_E2D, //���ÿܰǹ�2D
	TYPE_BUILDING_IND, //�����������ǹ�
	TYPE_BUILDING_STA, //������
	TYPE_BUILDING_APC, //�ֻ���
	TYPE_BUILDING_ROH, //��������
	TYPE_BUILDING_APL, //����Ʈ������
	TYPE_BUILDING_APS, //����Ʈ ��
	TYPE_BUILDING_MAX_CNT,
}TYPE_BUILDING;


typedef enum {
	TYPE_BLD_NAME_NOT = 0, // ��Ī ���� �ε���
	TYPE_BLD_NAME_INT, // ���� '123'
	TYPE_BLD_NAME_ENG, // ���� 'A'
	TYPE_BLD_NAME_DIC, // ��Ī ����
}TYPE_BLD_NAME;


typedef enum {
	TYPE_COMPLEX_NOT = 0, //������
	TYPE_COMPLEX_APC,	// �ֻ��մ���
	TYPE_COMPLEX_APL,	// ����Ʈ���������
	TYPE_COMPLEX_APT,	// ����Ʈ����
	TYPE_COMPLEX_B01,	// �����������
	TYPE_COMPLEX_B04,	// �����������
	TYPE_COMPLEX_B11,	// ���ǽü�����
	TYPE_COMPLEX_ETC,	// ��Ÿ����
	TYPE_COMPLEX_OFT,	// ���ǽ��ڴ���
	TYPE_COMPLEX_ROH,	// �������ô���
	TYPE_COMPLEX_SCH,	// �б�����
	TYPE_COMPLEX_MAX_CNT,
}TYPE_COMPLEX;


typedef enum {
	TYPE_POLYGON_NONE = 0, // ������
	TYPE_POLYGON_BUILDING, // ����
	TYPE_POLYGON_COMPLEX, // ����
	TYPE_POLYGON_MOUNTAIN, // ��ٿ����
}TYPE_POLYGON;


typedef enum {
	TYPE_ENT_NONE = 0, // ������
	TYPE_ENT_BUILDING, // �����Ա���
	TYPE_ENT_COMPLEX, // �����Ա���
	TYPE_ENT_MOUNTAIN = TYPE_ENT_COMPLEX, // ���� �Ա���
	TYPE_ENT_CPX_ROAD, // ����������
	TYPE_ENT_NEAR_ROAD, // �ֱ�������
}TYPE_ENTRANCE;


typedef enum {
	TYPE_OPTIMAL_ENTRANCE_NONE = 0, // ������
	TYPE_OPTIMAL_ENTRANCE_CAR, // ���� �Ա���
	TYPE_OPTIMAL_ENTRANCE_BUILDING_TAXI, // �ý� ������ ����(�ǹ�)
	TYPE_OPTIMAL_ENTRANCE_COMPLEX_TAXI, // �ý� ������ ����(�ǹ���)
	TYPE_OPTIMAL_ENTRANCE_PARCEL_CAR, // �ù� ���� ���� ����
	TYPE_OPTIMAL_ENTRANCE_PEDESTRIAN, // ������ �Ա���
	TYPE_OPTIMAL_ENTRANCE_DELIBERY_CAR, // ��� ������(����, �������)
	TYPE_OPTIMAL_ENTRANCE_DELIBERY_PED, // ��� ������(������, ����)

	TYPE_OPTIMAL_ENTRANCE_MOUNTAIN, // ���� �Ա���
	// max 15, using only 4bit
}TYPE_OPTIMAL_ENTRANCE;


typedef enum {
	TYPE_MOUNTAIN_ENTRANCE_NONE = 0, // ������

	// max 8, using only 4bit
}TYPE_MOUNTAIN_ENTRANCE;


typedef enum {
	TYPE_LINK_MATCH_NONE = 0, // ������
	TYPE_LINK_MATCH_CARSTOP, // ���� ������
	TYPE_LINK_MATCH_CARSTOP_EX, // ���� ������ + ����������(�ǹ��Ա��� ��Ȯ�ν�, �������ο� ��Ī�� �Ա����� Ȯ���ϱ� ���� ����)
	TYPE_LINK_MATCH_CARENTRANCE, // ���� ������ ����
	TYPE_LINK_MATCH_FOR_TABLE, // �湮�� ���̺�� , 2���� �̻�
	TYPE_LINK_MATCH_FOR_HD, // P2P HD ��� Ž�� ��, sd-hd ��Ī�Ǵ� ��ũ�� ����
	TYPE_LINK_MATCH_FOR_FOREST, // ���� ��� ��ũ ����, ����(�켱), �����ڱ�
}TYPE_LINK_MATCH;


#define POLYGON_DATA_ATTR_MAX		5
typedef enum {
	TYPE_POLYGON_DATA_ATTR_PART = 0,// ������ ��Ʈ �ε���
	TYPE_POLYGON_DATA_ATTR_VTX,		// ������ ���ؽ�
	TYPE_POLYGON_DATA_ATTR_ENT,		// �Ա��� ����
	TYPE_POLYGON_DATA_ATTR_LINK,	// ������ ���� ID
	TYPE_POLYGON_DATA_ATTR_MESH		// ��ø �޽�
}TYPE_POLYGON_DATA_ATTR;

/*
01	�ü��� ����
02	�����Ա� ����
*/
typedef enum
{
	TYPE_WNODEEX_FCT,
	TYPE_WNODEEX_IO,
	TYPE_WNODEEX_COUNT
}TYPE_WNODEEX;

/*
01	�߷�����	�߷� ���� ����	T(��) ����
02	��������	���� ���� ����	M(����) ����
03	����	���� ��Ī ����	��Ī
04	�ͳ�	�ͳ� ��Ī ����	��Ī
05	������	������ ��Ī ����	��Ī
06	��������	�������� ��Ī ����	��Ī
07	������������	���� ���� ���� �ð� ����	���� ���� ���� �ð� (CODE�� ����)
08	��Ӹ��	������̼� ��Ӹ�� ����	��Ī
09	��Ӹ��ΰ�	��Ӹ�� �ްԼ� �� ����	��Ӹ�� �ްԼ� �� ���� (CODE�� ����)
10	��������	���� ����	��ũ�� ���� ���� ���� ���� (CODE�� ����)
11	�������	��� ����	��ũ�� ���� ���� ��� ���� (ǥ����)
12	������������	���� ���� ���� ����	��ũ�� ���� ���� �÷� ���� ����
13	�����������	���� ���� ��� ����	��ũ�� ���� ���� ���� ������ ��� ���� (���)
14	�����Ʈ	�����Ʈ ��Ī ����	��Ī
15	Ȯ�뵵	Ȯ�뵵 ��Ī ����	Ȯ�뵵 ���� ��Ī
16	TPEG	ǥ�س�帵ũ�� ��ũ ID	ǥ�س�帵ũ LINK_ID (10�ڸ�)
17	��������	�������� ����	�������� ���� (CODE�� ����)
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
	LINK_GUIDE_TYPE_DEFAULT = 0, // �Ϲ�
	LINK_GUIDE_TYPE_DEPARTURE, // �����
	LINK_GUIDE_TYPE_WAYPOINT, // ������
	LINK_GUIDE_TYPE_DESTINATION, // ������
	LINK_GUIDE_TYPE_DEPARTURE_WAYPOINT, // �����-������
	LINK_GUIDE_TYPE_DEPARTURE_DESTINATION, // �����-������
	LINK_GUIDE_TYPE_WAYPOINT_WAYPOINT, // ������-������
	LINK_GUIDE_TYPE_WAYPOINT_DESTINATION, // ������-������
} LINK_GUIDE_TYPE;

typedef enum {
	RESULT_FAILED = -1,
	RESULT_OK = 0,
	ROUTE_RESULT_SUCCESS = RESULT_OK,					//0		����

	ROUTE_RESULT_FAILED = 1,							//1		Ž�� ����	���� ������ ���� ����
	ROUTE_RESULT_FAILED_SAME_ROUTE = 2,					//2		����Ʈ ��Ž�� ����	���� ��ο� ����

	ROUTE_RESULT_FAILED_MULTI_POS_ROUTE_ALL = 5,		//5		���� ��� Ž�� ��� ����
	ROUTE_RESULT_FAILED_MULTI_POS_ROUTE = 6,			//6		���� ��� Ž�� ����

	ROUTE_RESULT_FAILED_WRONG_PARAM = 10,				//10	�߸��� �Ķ����, �ʼ� �Ķ���� üũ
	ROUTE_RESULT_FAILED_SET_MEMORY = 50,				//50	Ž�� Ȯ�� ���� �޸� �Ҵ� ����	Ž�� �ʱ�ȭ ����
	ROUTE_RESULT_FAILED_READ_DATA = 51,					//51	Ž�� ���� ������(����, �ɼ� ��) ���� �б� ����	Ž�� �ʱ�ȭ ����
	ROUTE_RESULT_FAILED_SET_START = 70,					//70	������� ���������� �ȵǰų�, �߸��� �����
	ROUTE_RESULT_FAILED_SET_VIA = 71,					//71	�������� ���������� �ȵǰų�, �߸��� ������
	ROUTE_RESULT_FAILED_SET_END = 72,					//72	�������� ���������� �ȵǰų�, �߸��� ������
	ROUTE_RESULT_FAILED_DIST_OVER = 90,					//90	Ž�� ���� �Ÿ� �ʰ�, �����Ÿ� 15km �̳� ���
	ROUTE_RESULT_FAILED_TIME_OVER = 91,					//91	Ž�� �ð� �ʰ�	10�� �̻�
	ROUTE_RESULT_FAILED_NODE_OVER = 92,					//92	Ȯ�� ���� Node ���� �ʰ�
	ROUTE_RESULT_FAILED_EXPEND = 93,					//93	Ȯ�� ����
	ROUTE_RESULT_FAILED_EXPEND_ISOLATED = 93,			//93	Ȯ�� ����, �� ��ũ

	ROUTE_RESULT_FAILED_COURSE = 600,					//600	�ڽ� Ž�� ����
	ROUTE_RESULT_FAILED_COURSE_ID = 601,				//601	�ڽ� ID �˻� ����
	ROUTE_RESULT_FAILED_COURSE_TYPE = 602,				//602	�ڽ� TYPE �˻� ����

	TMS_RESULT_FAILED = 10000,							//10000	Ŭ�����͸� ����
	TMS_RESULT_FAILED_LOOP = 10001,						//10001 �й� ���� �Ѱ� Ƚ�� �ʰ�
	TMS_RESULT_FAILED_MATCHING_LIMIT = 10002,			//10002 ������ �й� ���������� ������ ������
	TMS_RESULT_FAILED_MATCHING_DEVIATION = 10003,		//10003 ������ ���� ���� ������ ������

	// ����Ǵ� ����
	// �ִ� �й� ���� �ʹ� �۾Ƽ� �й��� �� ����, limit_value = 1��, 1��, 1���� ��
	// �ִ� ���� ���� �ʹ� �۾ �й��� �� ����, limit_deviation = 0��, 1��, 1���� �� 
	// �׷� ���� ���� �ʹ� �۾Ƽ��й��� �� ����, limit_count = 1
	// �׷� ���� ���� �ʹ� Ŀ�� �й��� �� ����, limit_count = 100

	TMS_RESULT_FAILED_INPUT_OPTION = 10100,				//10100 ����� �ɼ� ����
	TMS_RESULT_FAILED_OPTION = 10101,					//10101 ����� �ɼ� ����
	TMS_RESULT_FAILED_LIMIT_COUNT_OVER = 10102,			//10102 ��ü ������ ���� ���� �������� ������ ���� �������� �Ѱ�ġ�� �ʰ���.

	// 	resultCode	resultMessage	���	����
	// 0		����	����
	// 100	Result Not Found	�˻� ����	��� ����
	// 101	Argument Error	����	�Ķ���� ����
	// 102	Internal Server Error	����	���� ����
	// 103	Different Map Version	�˻� ����	���� ���� �ٸ�
	// 104	StyleID already exists	�˻� ����	�̹� ������ StyleID�� ����
	// 201	Searching for Security	�˻� ����	POI ���Ƚü���
	// 202	Longitude/Latitude	�˻� ����	������
	// 203	Mobile Phone Number	�˻� ����	��ȭ��ȣ (Mobile)
	// 204	Invalid Query	�˻� ����	���� ����
	// 205	POI not in given Admin	�˻� ����	��� ���� (��������)
	// 206	POI not in given Area	�˻� ����	��� ���� (��������)
	// 207	POI not in given Category	�˻� ����	��� ���� (�з�����)
	// 208	Neighbor Search Only	�˻� ����	��� ���� (�ֺ��˻����Է�)
	// 209	Neighbor Search not Found	�˻� ����	��� ���� (�ֺ� + Ű���� �˻� ��� ����)
	// 300	AppKey Error	����	AppKey ���� ����
	// 400	Taxi Fare Info not Found	�˻� ����	�ý� ��� ���� ����
	// 401	AreaCode Convert Fail	�˻� ����	�����ڵ� ��ȯ ����
	// 501	Unknown Fail	Ž�� ����	����(unknown)
	// 502	Apply Smart Re-navigation	Ž�� ����	Smart��Ž�� ����
	// 503	Canceled navigation by user	Ž�� ����	����ڿ� ���� Ž�� ���
	// 504	Error due to Checksum	Ž�� ����	üũ�� ����
	// 517	Memory allocation failure	Ž�� ����	�޸� �Ҵ� ����
	// 518	File open failure	Ž�� ����	���� ���� ����
	// 519	File read failure	Ž�� ����	���� �б� ����
	// 520	File write failure	Ž�� ����	���� ���� ����
	// 521	Socket connection failure	Ž�� ����	���� ���� ����
	// 532	Request parameter is invalid	Ž�� ����	��û �Ķ���Ͱ� ��ȿ���� ����
	// 533	The starting point is not selected, or the wrong starting point	Ž�� ����	������� ���õ��� �ʾҰų�, �߸��� �����
	// 534	Destination is not selected or wrong destination	Ž�� ����	�������� ���õ��� �ʾҰų�, �߸��� ������
	// 535	Wrong stopover	Ž�� ����	�߸��� ������
	// 536	Link Projection failure	Ž�� ����	Link Projection ���� (��Ʈ��ũ(���θ�)�� ���� ��� ��)
	// 537	Exceeding the navigational distance (1000km, walking navigation: 20km)	Ž�� ����	Ž�� ���� �Ÿ� �ʰ�(1000km, ���� Ž�� : 20km)
	// 538	Exceeds the number of expandable nodes	Ž�� ����	Ȯ�� ���� ��� �� �ʰ�
	// 539	Expansion failure	Ž�� ����	Ȯ�� ����
	// 540	Expansion failure due to inactivity or traffic control	Ž�� ����	Ư���� �����̳� ���� ������ ���� Ȯ�� ����
	// 541	Expansion failure due to vehicle height/weight restrictions near the starting point	Ž�� ����	����� ��ó�� ���� ����/�߷� �������� Ȯ�� ����
	// 542	Expansion failed due to a part-time curfew near the departure point	Ž�� ����	����� ��ó�� �ð��� ��������� ���� Ȯ�� ����
	// 543	The destination is a physical island road, and there are no established ferry routes.	Ž�� ����	�������� ������ �������̸�, ����� �丮 �׷ΰ� ����
	// 544	The departure or destination is a logical (transportation) island, and there are more than one destination	Ž�� ����	��� �Ǵ� �������� ����(����) ���̸�, �������� 2�� �̻��� ���
	// 545	No data requested	Ž�� ����	��û�� �����Ͱ� ����
}ROUTE_RESULT;


typedef enum {
	OPTIMAL_RESULT_SUCCESS = 0,					// ����

	OPTIMAL_RESULT_FAILED = 100,				// ����
	OPTIMAL_RESULT_FAILED_WRONG_PARAM = 101,	// �Ķ���� ����
	OPTIMAL_RESULT_FAILED_SERVER_ERROR = 102,	// ���� ����
}OPTIMAL_RESULT;


typedef enum
{
	TYPE_CLUSTER_DEVIDE_BY_COUNT, // ���� �й�
	TYPE_CLUSTER_DEVIDE_BY_DIST, // �Ÿ� �й�
	TYPE_CLUSTER_DEVIDE_BY_TIME, // �ð� �й�
}TYPE_CLUSTER_DEVIDE;

typedef enum
{
	TYPE_TSP_LOCK_NONE, // ���� ���� ����
	TYPE_TSP_LOCK_START, // ����� ����
	TYPE_TSP_LOCK_END, // ������ ����
	TYPE_TSP_LOCK_START_END, // ��-������ ����
	TYPE_TSP_RECURSICVE, // ����� ȸ��
}TYPE_TSP_LOCK;

typedef enum
{
	TYPE_TSP_ALGORITHM_TSP_GA, // TSP-GA
	TYPE_TSP_ALGORITHM_COPY, // using input
	TYPE_TSP_ALGORITHM_MANHATTHAN, // manhatthan
	TYPE_TSP_ALGORITHM_TSP, // TSP
}TYPE_TSP_ALGORITHM_NOW;