#pragma once

#include "FileBase.h"

//class CFileManager;

struct stEntrance
{
	int32_t MatchType; // 매칭 폴리곤 타입 0:미지정, 1:건물, 2:단지
	uint32_t Id;	// 건물/단지 Id
	KeyID MatchId;	// 매칭되는 건물/단지 ID
	//string CpxStr;	// DJ_TFID // 건물에 매칭되는 단지ID
	//KeyID CpxId;	// DJ_TFID // 건물에 매칭되는 단지ID
	// BUL_MAN_NO	// 도로명주소 데이터 건물 일련번호
	uint32_t CpxId; // 단지 ID
	uint32_t SdSggCode;	// 시도, 시군구 코드
	uint32_t EntCode;	// 건물 입구점 코드 // 코드 설명
	//	1: 차량 입구점, 2: 택시 승하차 지점(건물), 3: 택시 승하차 지점(건물군), 4: 택배 차량 하차 거점, 5: 보행자 입구점, 	6: 배달 하차점(차량, 오토바이), 7: 배달 하차점(자전거, 도보)
	double x;		// XPOS // 입구점 X좌표
	double y;		// YPOS // 입구점 Y좌표

	stEntrance() {
		MatchType = TYPE_ENT_NONE;
		Id = NULL_VALUE;
		MatchId = { NULL_VALUE, };
		//CpxId = { NULL_VALUE, };
		EntCode = 0;
		x = 0.f;
		y = 0.f;
	}
};



class CFileEntrance : public CFileBase
{
public:
	CFileEntrance();
	~CFileEntrance();

protected:
	uint32_t m_nEntIdx;
	vector<stEntrance> m_vtEntrance;

private:	
	int32_t m_nEntType; // 입구점 타입, 0:건물 입구점, 1:단지 입구점

	map<string, uint32_t>m_mapStringId;

	CFileComplex* m_pFileCpx;
	CFileBuilding* m_pFileBld;

public:
	virtual bool Initialize();
	virtual void Release();

	virtual bool ParseData(IN const char* fname);
	virtual bool GenServiceData();
	virtual void AddDataFeild(IN const int idx, IN const int type, IN const char* colData);
	virtual void AddDataRecord();

	//virtual bool OpenFile(IN const char* szFilePath);
	virtual bool SaveData(IN const char* szFilePath);

	//virtual size_t WriteHeader(FILE* fp, FileHeader* pHeader);
	//virtual size_t WriteIndex(FILE* fp, vector<FileIndex>* pvtFileIndex);
	virtual size_t WriteBody(FILE* fp, IN const uint32_t fileOff);

	virtual bool LoadData(IN const char* fname);
	virtual bool LoadDataByIdx(IN const uint32_t idx);

	virtual size_t ReadBody(FILE* fp);

	void SetFileComplex(CFileComplex* pCpx);
	void SetFileBuilding(CFileBuilding* pBld);
};

