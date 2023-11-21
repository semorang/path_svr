#pragma once

#include "FileBase.h"

//class CFileManager;

struct stForestNode
{
	uint32_t NodeID;
	SPoint NodeCoord; // 노드 좌표
	uint32_t ConnectNum : 4; // 접속 노드 수, MAX:15
	uint32_t NodeType : 2; // 노드 타입, 1:교차점, 2:단점, 3:속성변화점
	uint32_t ClimType : 2; // 등산기점 구분코드, 0:미정의, 1:입구, 2:봉우리
	uint32_t TypeReserved : 24; // reserved
	uint32_t ConnNode[12]; // 접속노드
	uint32_t ConnNodeAng[12]; // 접속노드각도
	uint32_t Zvalue; // 고도, 높이
	uint32_t EntNameIDX; // 입구점 명칭 인덱스
	uint32_t MntNameIDX; // 봉우리 명칭 인덱스
	uint32_t SignID; // 이정표ID
};

struct stForestLink
{
	uint32_t LinkID;
	uint32_t FromNodeID; // from 노드 ID
	uint32_t ToNodeID; // to 노드 ID
	uint64_t CourseType : 3; // 코스타입, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
	uint64_t CourseDir : 2; // 방면방향정보, 0:미정의, 1:정, 2:역
	uint64_t ConnectType: 1; // 연결로 여부, 0:기타, 1:연결로
	uint64_t Diff : 7; // 난이도(0~100,  숫자가 클수록 어려움)
	uint64_t Popular : 12; // 인기도 지수, 0-4095
	uint64_t FwTime : 12; // 정방향 이동 소요시간 (초), 1023(17분) // 최대값 확인 후 사이즈 조절 필요
	uint64_t BwTime : 12; // 역방향 이동 소요시간 (초), 1023(17분) // 최대값 확인 후 사이즈 조절 필요
	uint64_t Reserved : 15; // reserved
	uint32_t CourseNameCD; // 코스 명칭 코드
	uint32_t CourseDirNameIDX; // 진행방면정보 인덱스
	uint32_t SFRestrict; // 여름/가을 시즌 통제기간 MMDDMMDD(시작일 MMDD + 종료일 MMDD)
	uint32_t WSRestrict; // 겨울/봄 시즌 통제기간 MMDDMMDD(시작일 MMDD + 종료일 MMDD)
	int16_t Slop; // 경사도(+/- 127, 버텍스 순서기준 정방향 경사도)
	BYTE ConnCourseType[13]; // 중용코스종별, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길
	BYTE ConnCourseTypeNameIDX[13]; // 중용코스 명칭 인덱스
	BYTE RoadInfo[6]; // 노면정보, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드, 12:철구조물
	double LinkLen; // 링크 길이
	vector<SPoint> LinkVertex;
};


class CFileForest : public CFileBase
{
public:
	CFileForest();
	~CFileForest();

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

