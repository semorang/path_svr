#pragma once

#include "FileBase.h"

//class CFileManager;

struct stWalkMesh
{
	uint32_t MeshID;
	vector<uint32_t> vtNeighbors;
	SBox meshBox;
};


class CFileMesh : public CFileBase
{
public:
	CFileMesh();
	~CFileMesh();

private:
	bool AddIndexData(IN const FileIndex& pData);

public:
	virtual bool Initialize();
	virtual void Release();

	virtual bool ParseData(IN const char* fname);
	virtual bool GenServiceData();
	virtual void AddDataFeild(IN const int idx, IN const int type, IN const char* colData);
	virtual void AddDataRecord();

	virtual size_t WriteIndex(FILE* fp);
	virtual size_t WriteBody(FILE* fp, IN const uint32_t fileOff);

	virtual size_t ReadIndex(FILE* fp);
	virtual size_t ReadBody(FILE* fp);

};



//#if !defined(USE_P2P_DATA) && !defined(USE_SAMSUNG_HEAVY)// && 1 // defined(_DEBUG)
extern bool g_isUseTestMesh;
extern unordered_set<int32_t> g_arrTestMesh;
//#endif