#pragma once

#include "FileBase.h"

//class CFileManager;

struct stForestNode
{
	uint32_t MeshID;
	uint32_t NodeID;
	uint32_t AdjEdgeMesh; // ���� �޽� ID
	uint32_t AdjEdgeNode; // ���� ��� ID
	SPoint NodeCoord; // ��� ��ǥ
	uint32_t ConnectNum : 4; // ���� ��� ��, MAX:15
	uint32_t NodeType : 3; // ��� Ÿ��, 1:������, 2:����, 3:������, 4:��ȹ������, 5:�Ӽ���ȭ��
	uint32_t TypeReserved : 29; // reserved
	uint32_t ConnNode[8]; // ���ӳ��
	uint32_t ConnNodeAng[8]; // ���ӳ�尢��
	uint32_t Zvalue; // ��, ����
};

struct stForestLink
{
	uint32_t MeshID;
	uint32_t LinkID;
	uint32_t FromNodeID; // from ��� ID
	uint32_t ToNodeID; // to ��� ID
	uint32_t MntNameIDX; // ��� ��Ī �ε���
	uint32_t CourseType : 3; // �ڽ�Ÿ��, 0:������, 1:����, 2:�ѷ���, 3:�����ű�, 4:�����ڽ�
	uint32_t CourseDir : 2; // ����������, 0:������, 1:��, 2:��
	uint32_t ConnectType: 1; // ����� ����, 0:�����X, 1:�����O
	uint32_t Diff : 6; // ���� 0~50, (���ڰ� Ŭ���� �����)
	uint32_t FwTime : 8; // ������ �̵� �ҿ�ð� (��), 0-240
	uint32_t BwTime : 8; // ������ �̵� �ҿ�ð� (��), 0-240
	uint32_t Popular : 4; // �α⵵ ����, 0-10
	//uint32_t TypeReserved : 0; // reserved
	uint32_t CourseNameCD; // �ڽ� ��Ī �ڵ�
	uint32_t CourseDirNameIDX;// ���������� ��Ī �ε���
	uint32_t SFRestrict; // ����/���� ���� �����Ⱓ MMDDMMDD(������ MMDD + ������ MMDD)
	uint32_t WSRestrict; // �ܿ�/�� ���� �����Ⱓ MMDDMMDD(������ MMDD + ������ MMDD)
	BYTE ConnCourseType[8]; // �߿��ڽ�����, 0:������, 1:����, 2:�ѷ���, 3:�����ű�, 4:�����ڽ�
	BYTE ConnCourseTypeNameIDX[8]; // �߿��ڽ� ��Ī �ε���
	BYTE RoadInfo[6]; // ������� �ڵ�, 0:��Ÿ, 1:���ֱ�, 2:�����, 3:���, 4:����, 5:�ϸ�, 6:����, 7;��ٸ�, 8:����, 9:�ʴ���, 10:���ڼ���Ʈ, 11:��ũ�ε�
	BYTE VegType[2]; // �Ļ�����
	BYTE VegRate[2]; // �Ļ�����
	double LinkLen; // ��ũ ����
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

