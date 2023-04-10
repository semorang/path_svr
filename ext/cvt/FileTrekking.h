#pragma once

#include "FileBase.h"

//class CFileManager;

struct stForestNode
{
	uint32_t MeshID;
	uint32_t NodeID;
	uint32_t AdjEdgeMesh; // 인접 메쉬 ID
	uint32_t AdjEdgeNode; // 인접 노드 ID
	SPoint NodeCoord; // 노드 좌표
	uint32_t ConnectNum : 4; // 접속 노드 수, MAX:15
	uint32_t NodeType : 3; // 노드 타입, 1:교차점, 2:단점, 3:더미점, 4:구획변경점, 5:속성변화점
	uint32_t TypeReserved : 29; // reserved
	uint32_t ConnNode[8]; // 접속노드
	uint32_t ConnNodeAng[8]; // 접속노드각도
	uint32_t Zvalue; // 고도, 높이
};

struct stForestLink
{
	uint32_t MeshID;
	uint32_t LinkID;
	uint32_t FromNodeID; // from 노드 ID
	uint32_t ToNodeID; // to 노드 ID
	uint32_t MntNameIDX; // 산봉 명칭 인덱스
	uint32_t CourseType : 3; // 코스타입, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스
	uint32_t CourseDir : 2; // 방면방향정보, 0:미정의, 1:정, 2:역
	uint32_t ConnectType: 1; // 연결로 여부, 0:연결로X, 1:연결로O
	uint32_t Diff : 6; // 난도 0~50, (숫자가 클수록 어려움)
	uint32_t FwTime : 8; // 정방향 이동 소요시간 (분), 0-240
	uint32_t BwTime : 8; // 역방향 이동 소요시간 (분), 0-240
	uint32_t Popular : 4; // 인기도 지수, 0-10
	//uint32_t TypeReserved : 0; // reserved
	uint32_t CourseNameCD; // 코스 명칭 코드
	uint32_t CourseDirNameIDX;// 진행방면정보 명칭 인덱스
	uint32_t SFRestrict; // 여름/가을 시즌 통제기간 MMDDMMDD(시작일 MMDD + 종료일 MMDD)
	uint32_t WSRestrict; // 겨울/봄 시즌 통제기간 MMDDMMDD(시작일 MMDD + 종료일 MMDD)
	BYTE ConnCourseType[8]; // 중용코스종별, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스
	BYTE ConnCourseTypeNameIDX[8]; // 중용코스 명칭 인덱스
	BYTE RoadInfo[6]; // 노면정보 코드, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드
	BYTE VegType[2]; // 식생정보
	BYTE VegRate[2]; // 식생비율
	double LinkLen; // 링크 길이
	vector<SPoint> LinkVertex;
};


class CFileTrekking : public CFileBase
{
public:
	CFileTrekking();
	~CFileTrekking();

private:
	bool SetData_Node(int idx, stForestNode &getNode_Dbf, char* colData);
	bool SetData_Link(int idx, stForestLink &getLink_Dbf, char* colData);

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

