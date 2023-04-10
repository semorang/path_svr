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

	virtual bool SaveData(IN const char* szFilePath);

	//virtual size_t WriteBase(FILE* fp);
	//virtual size_t WriteHeader(FILE* fp);
	virtual size_t WriteIndex(FILE* fp);
	virtual size_t WriteBody(FILE* fp, IN const uint32_t fileOff);

	///////////////////////////////////////////////////////////////////////////
	virtual bool LoadData(IN const char* szFilePath);

	//virtual size_t ReadBase(FILE* fp);
	//virtual size_t ReadHeader(FILE* fp);
	virtual size_t ReadIndex(FILE* fp);
	virtual size_t ReadBody(FILE* fp);

};

