#pragma once

#include "FileBase.h"

//class CFileManager;

struct stWalkNode
{
	uint32_t MeshID;
	uint32_t NodeID;
	uint32_t AdjEdgeMesh; // ���� �޽� ID
	uint32_t AdjEdgeNode; // ���� ��� ID
	uint32_t NodeNameIDX; // ��Ī���� �ε���
	uint32_t GateNameIDX; // ���ⱸ ��Ī,��ȣ ���� �ε���
	uint32_t FacilityNameIDX; // �ü��� �� ��Ī���� �ε���
	SPoint NodeCoord; // ��� ��ǥ
	//BYTE NodeType; // ��� Ÿ��, 1:������, 2:����, 3:������, 4:��ȹ������, 5:�Ӽ���ȭ��, 6:����ö������, 7:ö��������, 8:���ϵ�������, 9:���ϻ�������, 10:�ǹ�������
	//BYTE FacilityPhase; // �ü��� ����, 0:������, 1:����, 2:����, 3:
	//BYTE GatePhase; // ���ⱸ ����, 0:������, 1:����, 2:����, 3:������
	uint32_t ConnectNum : 4; // ���� ��� ��, MAX:15
	uint32_t NodeType : 4; // ��� Ÿ��, 1:������, 2:����, 3:������, 4:��ȹ������, 5:�Ӽ���ȭ��, 6:����ö������, 7:ö��������, 8:���ϵ�������, 9:���ϻ�������, 10:�ǹ�������
	uint32_t FacilityPhase : 2; // �ü��� ����, 0:������, 1:����, 2:����, 3:
	uint32_t GatePhase : 2; // ���ⱸ ����, 0:������, 1:����, 2:����, 3:������
	uint32_t TypeReserved : 20; // reserved
};

struct stWalkLink
{
	uint32_t MeshID;
	uint32_t LinkID;
	uint32_t FromNodeID; // from ��� ID
	uint32_t ToNodeID; // to ��� ID
	uint32_t RoadNameIDX; // ��Ī���� �ε���
	//BYTE BycicleType; // �����ŵ��� Ÿ��, 1:����������, 2:������/������� �����ŵ���, 3:���൵��
	//BYTE WalkType; //�����ڵ��� Ÿ��, 1:��������, 2:������뵵��, 3:���������뵵��, 4:�������뵵��, 5:�����൵��
	//BYTE FacilityType; // �ü��� Ÿ��, 0:������, 1:�䳢��, 2:���Ϻ���, 3:����, 4:������, 5:����, 6:����ö��, 7:ö��, 8:�߾ӹ���������, 9:���ϻ�, 10:�ǹ����뵵��, 11:��������_����, 12:��������_�ְŽü�, 13:��������_������, 14:��������_��Ÿ
	//BYTE GateType; // ���Է� Ÿ��, 0:������, 1:����, 2:���, 3:�����÷�����, 4:���/�����÷�����, 5:����������, 6:�ܼ������, 7:Ⱦ�ܺ���, 8:������ũ, 9:¡�˴ٸ�, 10:�ǻ�Ⱦ��
	//BYTE LaneCount; // ������
	//BYTE SideWalk; // �ε�(����) ����, 0:������, 1:����, 2:����
	//BYTE WalkCharge; // �����ڵ��� ���� ����, 0:����, 1:����
	//BYTE BycicleControl; // �����ŵ��� ���� �ڵ�, 0:�����, 1:������, 2:������, 3:����Ұ�
	uint32_t BycicleType : 2; // �����ŵ��� Ÿ��, 1:����������, 2:������/������� �����ŵ���, 3:���൵��
	uint32_t WalkType : 3; //�����ڵ��� Ÿ��, 1:��������, 2:������뵵��, 3:���������뵵��, 4:�������뵵��, 5:�����൵��
	uint32_t FacilityType : 4; // �ü��� Ÿ��, 0:������, 1:�䳢��, 2:���Ϻ���, 3:����, 4:������, 5:����, 6:����ö��, 7:ö��, 8:�߾ӹ���������, 9:���ϻ�, 10:�ǹ����뵵��, 11:��������_����, 12:��������_�ְŽü�, 13:��������_������, 14:��������_��Ÿ
	uint32_t GateType : 4; // ���Է� Ÿ��, 0:������, 1:����, 2:���, 3:�����÷�����, 4:���/�����÷�����, 5:����������, 6:�ܼ������, 7:Ⱦ�ܺ���, 8:������ũ, 9:¡�˴ٸ�, 10:�ǻ�Ⱦ��
	uint32_t LaneCount : 6; // ������, 63
	uint32_t SideWalk : 2; // �ε�(����) ����, 0:������, 1:����, 2:����
	uint32_t WalkCharge : 2; // �����ڵ��� ���� ����, 0:����, 1:����
	uint32_t BycicleControl : 2; // �����ŵ��� ���� �ڵ�, 0:�����, 1:������, 2:������, 3:����Ұ�
	uint32_t TypeReserved : 7; // reserved
	double LinkLen; // ��ũ ����
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

