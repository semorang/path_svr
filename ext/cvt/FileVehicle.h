#pragma once

#include "FileBase.h"

//class CFileManager;

struct stConnectedAttr
{
	KeyID NodeID;
	uint16_t PassCode; // 0:통행가능(유턴제외), 1:통행가능(유턴포함), 2:통행불가
	uint16_t Angle; // 진출 각도 // 360도 방식
};

struct stTructRestriction
{
	KeyID LinkID;
	double Weight; // 중량 제한
	double Height; // 높이 제한
};

struct stVehicleNode
{
	uint32_t MeshID;
	uint32_t NodeID;
	uint32_t AdjEdgeMesh; // 인접 메쉬 ID
	uint32_t AdjEdgeNode; // 인접 노드 ID
	uint32_t CrossNameIDX; // 교차로 명칭 인덱스
	SPoint NodeCoord; // 노드 좌표
	uint32_t ConnectNum : 4; // 접속 노드 수, MAX:15
	stConnectedAttr ConnectAttr[8]; // 접속 노드 속성, MAX:8
	uint32_t NodeType : 4; // 노드 타입, 1:교차점, 2:단점, 3:더미점, 4:구획변경점, 5:속성변화점, 6:지하철진출입, 7:철도진출입, 8:지하도진출입, 9:지하상가진출입, 10:건물진출입
	uint32_t TypeReserved : 24; // reserved
};

struct stVehicleLink
{
	uint32_t MeshID;
	uint32_t LinkID;
	uint32_t SNodeID; // from 노드(snode) ID
	uint32_t ENodeID; // to 노드(enode) ID
	uint32_t RoadType; // 도로종별, 1:고속도록, 2:도시고속도로, 3:일반국도, 4:페리항로, 5:지방도, 6:일반도로, 7:소로, 8:골목길, 9:시장길
	uint32_t RoadNum; // 노선번호
	uint32_t SubRoadCount; // 중용노선 수, MAX:3
	uint32_t SubRoadType1; // 중용1종별
	uint32_t SubRoadNum1; //중용1노선번호
	uint32_t SubRoadType2; // 중용2종별
	uint32_t SubRoadNum2; //중용2노선번호
	uint32_t SubRoadType3; // 중용3종별
	uint32_t SubRoadNum3; //중용3노선번호
	uint32_t LaneCount; // 차선수, 63
	uint32_t LinkType; // 링크종별, 0:입구점, 1:본선비분리, 2:본선분리, 3:연결로, 4:교차로, 5:램프, 6:로터리, 7:휴계소(SA), 8:유턴
	uint32_t Level; // 경로레벨, 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상, 4:일반도로6차선이상, 5:일반도로4차선이상, 6:일반도로2차선이상, 7:일반도로1차선이상, 8:SS도로, 9:GSS도로/단지내도로/통행금지도로/비포장도로
	uint32_t DetailCode1; // 도로세부종별(안내관련), 0:미정의, 6:좌회전 전용링크, 7:교차로 P턴링크
	uint32_t DetailCode2; // 도로세부종별(탐색관련), 0:미정의, 1:SS도로, 2:보도블럭
	uint32_t DetailCode3; // 도로세부종별(세부속성), 0:미정의, 1:고가도로, 지하차도 옆길, 2: 비포장도로, 3: 단지내도로
	uint32_t PassCode; // 통행코드, 1:통행가능, 2:통행불가, 3:공사구간, 4:공사계획구간
	uint32_t ControlCode;// 규제코드, 1:통행가능, 2:통행불가, 4:일방통행_정, 5:일방통행_역
	uint32_t CarOnly; // 자동차전용도로, 1:있음, 2:없음
	uint32_t Charge; // 유료도로, 1:있음, 2:없음
	uint32_t SafeZone; // 보호구역, 0:없음, 1:어린이보호구역, 2:노인보호구역, 3:마을주민보호구역, 4:장애인보호구역
	uint32_t RoadNameIDX; // 도로명 인덱스
	uint32_t MaxWeight; // 중량 제한 여부, T(톤) 단위 
	uint32_t MaxHeight; // 높이 제한 여부, M(미터) 단위 
	uint32_t Bridge; // 교량 유무, 0:없음, 1:있음
	uint32_t Tunnel; // 터널 유무, 0:없음, 1:있음
	uint32_t OverPass; // 고가도로 유무, 0:없음, 1:있음
	uint32_t UnderPass; // 지하차도 유무, 0:없음, 1:있음
	uint32_t BusOnly; // 버스전용차로 유무, 0:없음, 1~19
	uint32_t ExpressMode; // 고속모드 유무, 0:없음, 1:있음
	uint32_t ExpressModeSub; // 고속모드부가 유무, 0:없음, 1:있음
	uint32_t LaneInfo; // 차선정보 유무, 0:없음, 1:있음
	uint32_t DirInfo; // 방면정보 유무, 0:없음, 1:있음
	uint32_t IndicatorLane; // 차로유도차선정보 유무, 0:없음, 1:있음
	uint32_t IndicatorDir; // 차로유도방면정보 유무, 0:없음, 1:있음
	uint32_t TollInfo; // 톨게이트 유무, 0:없음, 1:있음
	uint32_t JunctionInfo; // 확대도 유무, 0:없음, 1:있음
	uint32_t Tpeg; // TPEG 유무(YTN기준), 0:없음, 1:있음
	uint32_t EntryRestriction; // 진입제한 유무, 0:없음, 1:사람, 2:이륜, 3:화물, 4:택시, 5:입주민 외
	double LinkLen; // 링크 길이
	uint64_t TTLID_P; // TTL_ID 정방향 값
	uint64_t TTLID_N; // TTL_ID 역방향 값

#if defined(USE_P2P_DATA)
	//uint32_t Bridge; // 교량 유무, 0:없음, 1:있음
	//uint32_t OverPass; // 고가도로 유무, 0:없음, 1:있음
	uint32_t MaxSpeed; // 제한속도, 0~255
	//uint32_t MaxW; // 중량 제한 T(톤) 0~63
	//uint32_t MaxH; // 높이 제한 M(미터) 0~7
	uint32_t HdFlag; // HD 링크 매핑, 0:없읍, 1:전체, 2:부분
#endif

	vector<SPoint> LinkVertex;
};


class CFileVehicle : public CFileBase
{
public:
	CFileVehicle();
	~CFileVehicle();

public:
	virtual bool Initialize();
	virtual void Release();

	virtual bool ParseData(IN const char* fname);
	virtual bool GenServiceData();
	virtual void AddDataFeild(IN const int idx, IN const int type, IN const char* colData);
	virtual void AddDataRecord();

private:
	bool SetData_Node(int idx, stVehicleNode &getNode_Dbf, char* colData);
	bool SetData_Link(int idx, stVehicleLink &getLink_Dbf, char* colData);

private:
	unordered_map<uint64_t, stTructRestriction> m_umapTructRestriction;
};


// 8자리 통행 코드를 2비트씩 하위부터 채워넣는다
const uint16_t setPassCode(const char* szPassCode);

// 8자리 통행 코드에서 현재 인덱스의 통행 코드를 가져온다.
const uint16_t getPassCode(IN const uint16_t currentLinkPassCode, IN const int32_t idxCurrentPass);
const int32_t getNextPassCode(IN const KeyID currentLinkId, IN const KeyID nextLinkId, IN const stNodeInfo* pNode, IN const int32_t processDepth = -1, IN CDataManager* pDataMgr = nullptr);
const int32_t getPrevPassCode(IN const KeyID currentLinkId, IN const KeyID prevLinkId, IN const stNodeInfo* pNode, IN const int32_t processDepth = -1, IN CDataManager* pDataMgr = nullptr);
const int32_t getPrevPassCode(IN const stNodeInfo* pNode, IN const KeyID prevLinkId, IN CDataManager* pDataMgr);
const bool checkNextLinkIsolated(IN const stLinkInfo* pLink, CDataManager* pDataMgr);
const bool checkPrevLinkIsolated(IN const stLinkInfo* pLink, CDataManager* pDataMgr);