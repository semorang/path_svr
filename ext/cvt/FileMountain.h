#pragma once

#include "FileBase.h"

//class CFileManager;

// 숲길 네트워크 구분을 위한 산 폴리곤 및 입구점 데이터 관리

struct stMntBoundary
{
	uint32_t ID; // ID
	uint32_t MeshID; // 메시 매칭 ID
	uint32_t FNameIdx;// 산바운더리명칭 인덱스
	std::string Category; // 국립,도
	uint32_t Code; // 산코드
	uint32_t BeajiBal; // 배지 발
	std::string MName; // 산명칭
	uint32_t Task; // task
	SBox Box; 
	std::vector<uint16_t> vtParts; // 파트 인덱스
	std::vector<SPoint> vtVertex; // 버텍스
	std::vector<stEntranceInfo> vtEntrance; // 입구점
	std::vector<uint32_t> vtJoinedMesh; // 중첩 메쉬 ID
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
	uint64_t F_NODE_ID; // 숲길 노드 ID
	uint64_t W_NODE_ID; // 보행자 노드 ID
	uint32_t ENT_TYP; // 입구점 타입, 1: 교차점등산로 종점의 입구, 2: 등산로와 도보가 만나는 입구, 3: 등산로제외 링크와 도보가 만나는 입구
	std::string ENT_NAME; // 입구점 명칭
	uint32_t MNT_CD; // 산코드
	uint32_t DP; // DP
	uint32_t G_ID; // 그룹ID
	KeyID keyMnt;
	double x;		// XPOS // 입구점 X좌표
	double y;		// YPOS // 입구점 Y좌표

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
	std::unordered_map<uint64_t, stMntBoundary> m_mapBoundary;
	std::unordered_map<uint64_t, stMntEntrance> m_mapEntrance;
	std::unordered_map<uint32_t, std::unordered_set<uint32_t>> m_mapMntGidMcd; // g_id에 속하는 mnt_cd 관리

private:
	bool SetData_Boundary(int idx, stMntBoundary &getMnt_Dbf, char* colData);
	bool SetData_Entrance(int idx, stMntEntrance &getMnt_Dbf, char* colData);
	
	std::unordered_map<std::string, KeyID>m_mapStringId;
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

