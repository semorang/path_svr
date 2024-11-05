#pragma once

#include "FileBase.h"
#include "FileComplex.h"

//class CFileManager;

struct stBuilding
{
	//uint32_t MeshID; // �޽� ��Ī ID
	string BldId; // �ǹ� ��Ī ID
	//string CpxId; // ���� ��Ī ID
	uint32_t Code : 6; // �ǹ� ���� �ڵ� -> 63
	uint32_t Height : 10; // �ǹ� ���� -> 1023
	uint32_t Num : 10; // �ǹ� �� ��ȣ �ε��� --> 1023
	uint32_t NumType : 6; // �ǹ� �� ��ȣ Ÿ�� 0~7, 0:�ε���, 1:����('101'��), 2:����('A'��), 3:�ѱ�('��'��) -> 63
	//uint32_t Name; // ��Ī
	SBox Box; 
	vector<uint16_t> vtParts; // ��Ʈ �ε���
	vector<SPoint> vtVertex; // ���ؽ�
	vector<stEntranceInfo> vtEntrance; // �Ա���

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
	unordered_map<uint64_t, stBuilding> m_mapBuilding;

private:
	uint32_t getNameType(IN char* name, OUT uint32_t& type);
	uint32_t getBuildingCode(IN const char* code);
	bool SetData_Building(int idx, stBuilding &getNode_Dbf, char* colData);
	//bool AddMeshDataByBuilding(IN const stMeshInfo * pInfo, IN const stPolygonInfo * pData);

	unordered_map<string, KeyID>m_mapStringId;
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

