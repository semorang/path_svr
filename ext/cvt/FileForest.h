#pragma once

#include "FileBase.h"

#include "../route/MapCourse.h"

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

	uint32_t CourseType : 3; // 코스타입, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길, 4:종주코스, 5:추천코스, 6:MTB코스, 7:인기코스
	uint32_t CourseDir : 2; // 방면방향정보, 0:미정의, 1:정, 2:역
	uint32_t Popular : 12; // 인기도 지수, 0-4095
	uint32_t LeGalEx : 1; // 법정탐방로 여부, 0:비법정, 1:법정
	uint32_t AreaType : 3; // 0:기타, 1:국립공원, 2:도립공원, 3:군립공원, 4:시립공원
	uint32_t ManaOrg : 3; // 0:기타, 1:국립공원공단, 2:행정안전부, 3:문화체육관광부, 4:산림청, 5:지자체, 6:체육회, 7:비영리단체
	uint32_t SigSK : 2; // 0:미확인, 1:통신가능, 2:통신불가
	uint32_t SigKT : 2; // 0:미확인, 1:통신가능, 2:통신불가
	uint32_t SigLG : 2; // 0:미확인, 1:통신가능, 2:통신불가
	uint32_t Dp : 1; // 탐색제외구간 0, 1
	uint32_t Reserved : 1; // reserved
	//32

	uint32_t ConnectType : 1; // 연결로 여부, 0:기타, 1:연결로
	uint32_t Diff : 7; // 난이도(0~100,  숫자가 클수록 어려움)
	uint32_t FwTime : 12; // 정방향 이동 소요시간 (초), 1023(17분) // 최대값 확인 후 사이즈 조절 필요
	uint32_t BwTime : 12; // 역방향 이동 소요시간 (초), 1023(17분) // 최대값 확인 후 사이즈 조절 필요
	//32

	uint32_t CourseCount; // 코스 갯수
	uint32_t CourseCD; // 코스 코드
	uint32_t CourseDirNameIDX; // 진행방면정보 인덱스
	uint32_t MntCD; // 산 코드
	uint32_t Gid; // 그룹ID
	uint32_t Grade; // 등급

	uint32_t Storunst : 2; // 입목존재코드, 0:비산림, 1:입목지, 2:무립목지
	uint32_t FrorCD : 2; // 임종코드, 0:무립목지/비산림, 1:인공림, 2:천연림
	uint32_t FrtpCD : 3; // 임상코드, 0:무립목지/비산림, 1:침엽수림, 2:활엽수림, 3:혼효림, 4:죽림
	uint32_t KoftrGrCD : 7; // 수종그룹코드, 10:기타침엽수,11:소나무,12:잣나무,13:낙엽송,14:리기다소나무,15:곰솔,16:잔나무,17:편백나무,18:삼나무,19:가분비나무,
		// 20 비자나무,21:은행나무,30:기타활엽수,31:상수리나무,32:신갈나무,33:굴참나무,34:기타 참나무류,35:오리나무,36:고로쇠나무,37:자작나무,38:박달나무,39:밤나무,
		// 40:물푸레나무,41:서어나무,42:때죽나무,43:호두나무,44:백합나무,45:포플러,46:벚나무,47:느티나무,48:층층나무,49:아까시나무,
		// 60:기타상록활엽수,61:가시나무,62:구실잣밤나무,63:녹나무,64:굴거리나무,65:황칠나무,66:사스레피나무,67:후박나무,68:새덕이,
		// 77:침활혼효림,78:죽림,81:미립목지,82:제지,91:주거지,92:초지,93:경작지,94:수체,95:과수원,99:기타
	uint32_t DmclsCD : 2; // 경급코드, 0:치수, 1:소경목, 2:중경목, 3:대경목
	uint32_t AgclsCD : 4; // 영급코드, 0:미정의, 1:1영급~9:9영급
	uint32_t DnstCD : 2; // 밀도코드, 0:소(A), 1:중(B), 2:밀(C)
	uint32_t SFRestrictCause : 3; // 여름/가을 시즌 통제사유, 0:기타, 1:산불방지, 2:휴식년, 3:정비, 4:사유지, 5:위험지역
	uint32_t WSRestrictCause : 3; // 겨울/봄 시즌 통제사유, 0:기타, 1:산불방지, 2:휴식년, 3:정비, 4:사유지, 5:위험지역
	uint32_t Theme : 4; // 0:기타, 1:계곡, 2:강, 3:바다, 4:도시, 5:산, 6:숲, 7:섬, 8:공원
	// 32

	uint32_t WSRestrict; // 겨울/봄 시즌 통제기간 MMDDMMDD(시작일 MMDD + 종료일 MMDD)
	uint32_t SFRestrict; // 여름/가을 시즌 통제기간 MMDDMMDD(시작일 MMDD + 종료일 MMDD)
	uint32_t Mesh; // Mesh
	int16_t Slop; // 경사도(+/- 127, 버텍스 순서기준 정방향 경사도)
	uint8_t ConnCourseType[18]; // 중용코스종별, 0:미정의, 1:등산로, 2:둘레길, 3:자전거길
	uint32_t ConnCourseCD[18]; // 중용코스 코드
	uint8_t RoadInfo[6]; // 노면정보, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드, 12:철구조물
	double LinkLen; // 링크 길이
	std::vector<SPoint> LinkVertex;
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
	
	virtual bool SaveData(IN const char* szFilePath);
	virtual bool LoadData(IN const char* szFilePath);

	bool SaveCourseData(IN const char* szFilePath);
	size_t WriteCourseHeader(FILE* fp);
	size_t WriteCourseIndex(FILE* fp);
	size_t WriteCourseBody(FILE* fp, IN const uint32_t fileOff);

	virtual bool LoadCourseData(IN const char* szFilePath);
	virtual size_t ReadCourseHeader(FILE* fp);
	virtual size_t ReadCourseIndex(FILE* fp);
	virtual size_t ReadCourseBody(FILE* fp);

private:
	char m_szCourseDataPath[MAX_PATH];

	FileHeader m_fileCourseHeader;
	std::vector<FileIndex> m_vtCourseIndex;

	MapCourse m_mapCourse;
};

