#pragma once

#include "FileBase.h"

//class CFileManager;

struct stWalkNode
{
	uint32_t MeshID;
	uint32_t NodeID;
	uint32_t AdjEdgeMesh; // 인접 메쉬 ID
	uint32_t AdjEdgeNode; // 인접 노드 ID
	uint32_t NodeNameIDX; // 명칭사전 인덱스
	uint32_t GateNameIDX; // 입출구 명칭,번호 사전 인덱스
	uint32_t FacilityNameIDX; // 시설물 상세 명칭사전 인덱스
	SPoint NodeCoord; // 노드 좌표
	//BYTE NodeType; // 노드 타입, 1:교차점, 2:단점, 3:더미점, 4:구획변경점, 5:속성변화점, 6:지하철진출입, 7:철도진출입, 8:지하도진출입, 9:지하상가진출입, 10:건물진출입
	//BYTE FacilityPhase; // 시설물 위상, 0:미정의, 1:지하, 2:지상, 3:
	//BYTE GatePhase; // 입출구 위상, 0:미정의, 1:지하, 2:지상, 3:지상위
	uint32_t ConnectNum : 4; // 접속 노드 수, MAX:15
	uint32_t NodeType : 4; // 노드 타입, 1:교차점, 2:단점, 3:더미점, 4:구획변경점, 5:속성변화점, 6:지하철진출입, 7:철도진출입, 8:지하도진출입, 9:지하상가진출입, 10:건물진출입
	uint32_t FacilityPhase : 2; // 시설물 위상, 0:미정의, 1:지하, 2:지상, 3:
	uint32_t GatePhase : 2; // 입출구 위상, 0:미정의, 1:지하, 2:지상, 3:지상위
	uint32_t TypeReserved : 20; // reserved
};

struct stWalkLink
{
	uint32_t MeshID;
	uint32_t LinkID;
	uint32_t FromNodeID; // from 노드 ID
	uint32_t ToNodeID; // to 노드 ID
	uint32_t RoadNameIDX; // 명칭사전 인덱스
	//BYTE BycicleType; // 자전거도로 타입, 1:자전거전용, 2:보행자/차량겸용 자전거도로, 3:보행도로
	//BYTE WalkType; //보행자도로 타입, 1:복선도록, 2:차량겸용도로, 3:자전거전용도로, 4:보행전용도로, 5:가상보행도로
	//BYTE FacilityType; // 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
	//BYTE GateType; // 진입로 타입, 0:미정의, 1:경사로, 2:계단, 3:에스컬레이터, 4:계단/에스컬레이터, 5:엘리베이터, 6:단순연결로, 7:횡단보도, 8:무빙워크, 9:징검다리, 10:의사횡단
	//BYTE LaneCount; // 차선수
	//BYTE SideWalk; // 인도(보도) 여부, 0:미조사, 1:있음, 2:없음
	//BYTE WalkCharge; // 보행자도로 유료 여부, 0:무료, 1:유료
	//BYTE BycicleControl; // 자전거도로 규제 코드, 0:양방향, 1:정방향, 2:역방향, 3:통행불가
	uint32_t BycicleType : 2; // 자전거도로 타입, 1:자전거전용, 2:보행자/차량겸용 자전거도로, 3:보행도로
	uint32_t WalkType : 3; //보행자도로 타입, 1:복선도록, 2:차량겸용도로, 3:자전거전용도로, 4:보행전용도로, 5:가상보행도로
	uint32_t FacilityType : 4; // 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
	uint32_t GateType : 4; // 진입로 타입, 0:미정의, 1:경사로, 2:계단, 3:에스컬레이터, 4:계단/에스컬레이터, 5:엘리베이터, 6:단순연결로, 7:횡단보도, 8:무빙워크, 9:징검다리, 10:의사횡단
	uint32_t LaneCount : 6; // 차선수, 63
	uint32_t SideWalk : 2; // 인도(보도) 여부, 0:미조사, 1:있음, 2:없음
	uint32_t WalkCharge : 2; // 보행자도로 유료 여부, 0:무료, 1:유료
	uint32_t BycicleControl : 2; // 자전거도로 규제 코드, 0:양방향, 1:정방향, 2:역방향, 3:통행불가
	uint32_t TypeReserved : 7; // reserved
	double LinkLen; // 링크 길이
	vector<SPoint> LinkVertex;
};


class CFilePedestrian : public CFileBase
{
public:
	CFilePedestrian();
	~CFilePedestrian();

private:
	bool SetData_Node(int idx, stWalkNode &getNode_Dbf, char* colData);
	bool SetData_Link(int idx, stWalkLink &getLink_Dbf, char* colData);

public:
	virtual bool Initialize();
	virtual void Release();

	virtual bool ParseData(IN const char* fname);
	virtual bool GenServiceData();
	virtual void AddDataFeild(IN const int idx, IN const int type, IN const char* colData);
	virtual void AddDataRecord();

	virtual bool OpenFile(IN const char* szFilePath);
	virtual bool SaveData(IN const char* szFilePath);
	virtual bool LoadData(IN const char* szFilePath);

};

