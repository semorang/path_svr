#pragma once

#include "FileBase.h"

//class CFileManager;

struct stComplex
{
	uint32_t GID; // GID
	uint32_t MeshID; // 메시 매칭 ID
	uint32_t Code; // 단지 종별 코드
	string CpxId; // 단지 매칭 ID
	string SdSggCode; // 시도/시군구 코드
	SBox Box; 
	vector<uint16_t> vtParts; // 파트 인덱스
	vector<SPoint> vtVertex; // 버텍스
	vector<stEntranceInfo> vtEntrance; // 입구점
	vector<uint32_t> vtJoinedMesh; // 중첩 메쉬 ID
	KeyID keyCpx;

	stComplex() {
		GID = 0;
		MeshID = 0;
		Code = 0;
		CpxId = "";
		SdSggCode = "";
		memset(&Box, 0x00, sizeof(Box));

		keyCpx.llid = 0;
	}
};

//struct stComplexShare {
//	string strId; // 단지 매칭 ID 스트링
//	uint32_t cpxId; // 단지 ID
//	vector<KeyID> vtCpxParts; // 각 메시로 분할된 단지ID
//	set<KeyID> setLinks; // 단지에 포함되는 링크ID
//};

class CFileComplex : public CFileBase
{
public:
	CFileComplex();
	~CFileComplex();

protected:
	uint32_t m_nCpxIdx;
	unordered_map<uint64_t, stComplex> m_mapComplex;

private:
	uint32_t getComplexCode(IN const char* type);
	bool SetData_Complex(int idx, stComplex &getCpx_Dbf, char* colData);
	//bool AddMeshDataByComplex(IN const stMeshInfo * pInfo, IN const stPolygonInfo * pData);
	uint32_t getLinksInComplex(IN const stComplex& complex, OUT vector<KeyID>& vtLinks); // 단지내 도로 정보 처리
	
	unordered_map<string, KeyID>m_mapStringId;
	//unordered_map<uint32_t, stComplexShare>m_mapCpxShare;

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

	bool AddEntranceData(IN const KeyID keyId, IN const stEntranceInfo& entInfo);
	const stComplex* GetComplexData(IN const KeyID keyId) const;

	const KeyID AddStringId(IN const char* szStringId, IN const uint32_t meshId);
	const KeyID GetIdFromStringId(IN const char* szStringId);
	//const int32_t CFileComplex::GetCpxShare(IN const uint32_t cpxId, stComplexShare** pIds);
};

