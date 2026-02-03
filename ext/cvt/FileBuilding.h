#pragma once

#include "FileBase.h"
#include "FileComplex.h"

//class CFileManager;

struct stBuilding
{
	//uint32_t MeshID; // 메시 매칭 ID
	std::string BldId; // 건물 매칭 ID
	//string CpxId; // 단지 매칭 ID
	uint32_t Code : 6; // 건물 종별 코드 -> 63
	uint32_t Height : 10; // 건물 높이 -> 1023
	uint32_t Num : 10; // 건물 동 번호 인덱스 --> 1023
	uint32_t NumType : 6; // 건물 동 번호 타입 0~7, 0:인덱스, 1:정수('101'동), 2:영어('A'동), 3:한글('가'동) -> 63
	//uint32_t Name; // 명칭
	SBox Box; 
	std::vector<uint16_t> vtParts; // 파트 인덱스
	std::vector<SPoint> vtVertex; // 버텍스
	std::vector<stEntranceInfo> vtEntrance; // 입구점

	KeyID keyBld;

	stBuilding() {
		//MeshID = 0;
		Code = 0;
		Num = 0;
		Height = 0;
		memset(&Box, 0x00, sizeof(Box));
		keyBld.llid = 0;
	}
};


class CFileBuilding : public CFileBase
{
public:
	CFileBuilding();
	~CFileBuilding();

protected:
	uint32_t m_nBldIdx;
	std::unordered_map<uint64_t, stBuilding> m_mapBuilding;

private:
	uint32_t getNameType(IN char* name, OUT uint32_t& type);
	uint32_t getBuildingCode(IN const char* code);
	bool SetData_Building(int idx, stBuilding &getNode_Dbf, char* colData);
	//bool AddMeshDataByBuilding(IN const stMeshInfo * pInfo, IN const stPolygonInfo * pData);

	std::unordered_map<std::string, KeyID>m_mapStringId;
	CFileComplex* m_pFileCpx;

public:
	virtual bool Initialize();
	virtual void Release();

	virtual bool ParseData(IN const char* fname);
	virtual bool GenServiceData();
	virtual void AddDataFeild(IN const int idx, IN const int type, IN const char* colData);
	virtual void AddDataRecord();

	virtual size_t WriteBody(FILE* fp, IN const uint32_t fileOff);
	virtual size_t ReadBody(FILE* fp);

	virtual bool LoadDataByIdx(IN const uint32_t idx);

	void SetFileComplex(CFileComplex* pCpx);
	bool AddEntranceData(IN const KeyID keyId, IN const stEntranceInfo& entInfo);

	const KeyID AddStringId(IN const char* szStringId, IN const uint32_t meshId);
	const KeyID GetIdFromStringId(IN const char* szStringId);
};

