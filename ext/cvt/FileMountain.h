#pragma once

#include "FileBase.h"

//class CFileManager;

struct stMntBoundary
{
	uint32_t ID; // ID
	uint32_t MeshID; // �޽� ��Ī ID
	uint32_t FNameIdx;// ��ٿ������Ī �ε���
	string Category; // ����,��
	uint32_t Code; // ���ڵ�
	uint32_t BeajiBal; // ���� ��
	string MName; // ���Ī
	uint32_t Task; // task
	SBox Box; 
	vector<uint16_t> vtParts; // ��Ʈ �ε���
	vector<SPoint> vtVertex; // ���ؽ�
	vector<stEntranceInfo> vtEntrance; // �Ա���
	vector<uint32_t> vtJoinedMesh; // ��ø �޽� ID
	KeyID keyMnt;

	stMntBoundary() {
		ID = 0;
		MeshID = 0;
		Code = 0;
		BeajiBal = 0;
		Task = 0;
		memset(&Box, 0x00, sizeof(Box));

		keyMnt.llid = 0;
	}
};


struct stMntEntrance
{
	uint32_t ENT_ID; // ID
	uint64_t F_NODE_ID; // ���� ��� ID
	uint64_t W_NODE_ID; // ������ ��� ID
	uint32_t ENT_TYP; // �Ա��� Ÿ��, 1: ���������� ������ �Ա�, 2: ���ο� ������ ������ �Ա�, 3: �������� ��ũ�� ������ ������ �Ա�
	string ENT_NAME; // �Ա��� ��Ī
	uint32_t MNT_CD; // ���ڵ�
	uint32_t DP; // DP
	uint32_t G_ID; // �׷�ID
	KeyID keyMnt;
	double x;		// XPOS // �Ա��� X��ǥ
	double y;		// YPOS // �Ա��� Y��ǥ

	stMntEntrance() {
		ENT_ID = 0;
		F_NODE_ID = 0;
		W_NODE_ID = 0;
		ENT_TYP = 0;
		ENT_NAME = "";
		MNT_CD = 0;
		keyMnt.llid = 0;
		x = 0.f;
		y = 0.f;
	}
};


class CFileMountain : public CFileBase
{
public:
	CFileMountain();
	~CFileMountain();

protected:
	uint32_t m_nMntIdx;
	unordered_map<uint64_t, stMntBoundary> m_mapBoundary;
	unordered_map<uint64_t, stMntEntrance> m_mapEntrance;
	unordered_map<uint32_t, unordered_set<uint32_t>> m_mapMntGidMcd; // g_id�� ���ϴ� mnt_cd ����

private:
	bool SetData_Boundary(int idx, stMntBoundary &getMnt_Dbf, char* colData);
	bool SetData_Entrance(int idx, stMntEntrance &getMnt_Dbf, char* colData);
	
	unordered_map<string, KeyID>m_mapStringId;
	//unordered_map<uint32_t, stMntBoundaryShare>m_mapCpxShare;

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

	bool AddEntranceData(IN const KeyID keyId, IN const stEntranceInfo& entInfo);
	const stMntBoundary* GetMountainData(IN const KeyID keyId) const;
};

