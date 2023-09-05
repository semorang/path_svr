#pragma once

#include "FileBase.h"

//class CFileManager;

struct stConnectedAttr
{
	KeyID NodeID;
	uint16_t PassCode; // 0:���డ��(��������), 1:���డ��(��������), 2:����Ұ�
	uint16_t Angle; // ���� ���� // 360�� ���
};

struct stVehicleNode
{
	uint32_t MeshID;
	uint32_t NodeID;
	uint32_t AdjEdgeMesh; // ���� �޽� ID
	uint32_t AdjEdgeNode; // ���� ��� ID
	uint32_t CrossNameIDX; // ������ ��Ī �ε���
	SPoint NodeCoord; // ��� ��ǥ
	uint32_t ConnectNum : 4; // ���� ��� ��, MAX:15
	stConnectedAttr ConnectAttr[8]; // ���� ��� �Ӽ�, MAX:8
	uint32_t NodeType : 4; // ��� Ÿ��, 1:������, 2:����, 3:������, 4:��ȹ������, 5:�Ӽ���ȭ��, 6:����ö������, 7:ö��������, 8:���ϵ�������, 9:���ϻ�������, 10:�ǹ�������
	uint32_t TypeReserved : 24; // reserved
};

struct stVehicleLink
{
	uint32_t MeshID;
	uint32_t LinkID;
	uint32_t FromNodeID; // from ��� ID
	uint32_t ToNodeID; // to ��� ID
	uint32_t RoadNameIDX; // ���θ� �ε���
	uint32_t RoadNum; // �뼱��ȣ
	uint32_t SubRoadNum1; //�߿�1�뼱��ȣ
	uint32_t SubRoadNum2; //�߿�2�뼱��ȣ
	uint32_t SubRoadNum3; //�߿�3�뼱��ȣ

	uint32_t RoadType : 4; // ��������, 1:��ӵ���, 2:���ð�ӵ���, 3:�Ϲݱ���, 4:�丮�׷�, 5:���浵, 6:�Ϲݵ���, 7:�ҷ�, 8:����, 9:�����
	uint32_t LaneCount : 6; // ������, 63
	uint32_t LinkType : 4; // ��ũ����, 0:�Ա���, 1:������и�, 2:�����и�, 3:�����, 4:������, 5:����, 6:���͸�, 7:�ް��(SA), 8:����
	uint32_t Level : 4; // ��η���, 0:��ӵ���, 1:���ð�ӵ���, �ڵ������� ����/���浵, 2:����, 3:���浵/�Ϲݵ���8�����̻�, 4:�Ϲݵ���6�����̻�, 5:�Ϲݵ���4�����̻�, 6:�Ϲݵ���2�����̻�, 7:�Ϲݵ���1�����̻�, 8:SS����, 9:GSS����/����������/�����������/�����嵵��
	uint32_t PassCode : 3; // �����ڵ�, 1:���డ��, 2:����Ұ�, 3:���籸��, 4:�����ȹ����
	uint32_t ControlCode : 3;// �����ڵ�, 1:���డ��, 2:����Ұ�, 4:�Ϲ�����_��, 5:�Ϲ�����_��
	uint32_t CarOnly : 2; // �ڵ������뵵��, 1:����, 2:����
	uint32_t Charge : 2; //���ᵵ��, 1:����, 2:����
	uint32_t SafeZone : 2; //��̺�ȣ����, ���κ�ȣ����, 0:����, 1:��̺�ȣ����, 2:���κ�ȣ����
	uint32_t Tunnel : 1; // �ͳ� ����, 0:����, 1:����
	uint32_t UnderPass : 1; // �������� ����, 0:����, 1:����
	// 32

	uint32_t SubRoadCount : 2; // �߿�뼱 ��, MAX:3
	uint32_t SubRoadType1 : 4; //�߿�1����
	uint32_t SubRoadType2 : 4; //�߿�2����
	uint32_t SubRoadType3 : 4; //�߿�3����
	uint32_t DetailCode1 : 3; // ���μ�������(�ȳ�����), 0:������, 6:��ȸ�� ���븵ũ, 7:������ P�ϸ�ũ
	uint32_t DetailCode2 : 2; // ���μ�������(Ž������), 0:������, 1:SS����, 2:������
	uint32_t DetailCode3 : 3; // ���μ�������(���μӼ�), 0:������, 1:������, �������� ����, 2: �����嵵��, 3: ����������
	uint32_t TypeReserved : 10; // reserved
	// 32
	
#if defined(USE_P2P_DATA)
	uint32_t Bridge : 1; // ���� ����, 0:����, 1:����
	uint32_t OverPass : 1; // ������ ����, 0:����, 1:����
	uint32_t MaxSpeed : 8; // ���Ѽӵ�, 0~255
	uint32_t MaxW : 6; // �߷� ���� T(��) 0~63
	uint32_t MaxH : 3; // ���� ���� M(����) 0~7
	uint32_t HdFlag : 2; // HD ��ũ ����, 0:����, 1:��ü, 2:�κ�
	uint32_t HdReserved : 11; // reserved
	// 32
#endif

	double LinkLen; // ��ũ ����
	vector<SPoint> LinkVertex;
};


class CFileVehicle : public CFileBase
{
public:
	CFileVehicle();
	~CFileVehicle();

private:
	bool SetData_Node(int idx, stVehicleNode &getNode_Dbf, char* colData);
	bool SetData_Link(int idx, stVehicleLink &getLink_Dbf, char* colData);

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


// 8�ڸ� ���� �ڵ带 2��Ʈ�� �������� ä���ִ´�
const uint16_t setPassCode(const char* szPassCode);

// 8�ڸ� ���� �ڵ忡�� ���� �ε����� ���� �ڵ带 �����´�.
const uint16_t getPassCode(IN const uint16_t currentLinkPassCode, IN const int32_t idxCurrentPass);
const bool getNextPassCode(IN const KeyID currentLinkId, IN const KeyID nextLinkId, IN const stNodeInfo* pNode, IN const int32_t processDepth = -1, IN CDataManager* pDataMgr = nullptr);
const bool getPrevPassCode(IN const KeyID currentLinkId, IN const KeyID prevLinkId, IN const stNodeInfo* pNode, IN const int32_t processDepth = -1, IN CDataManager* pDataMgr = nullptr);