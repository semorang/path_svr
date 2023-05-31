#pragma once

#include "FileBase.h"

//class CFileManager;
#include "../route/MapTraffic.h"

struct stTraffic
{
	uint32_t Mesh; // MESH: 6磊府
	uint32_t Snode_id;	// SNODE_ID : 6磊府
	uint32_t Enode_id;	// ENODE_ID : 6磊府
	uint32_t Dir_code; // DIR_CODE(规氢内靛) : 1磊府(沥 - 1, 开 - 2)
	uint32_t KSLink_id;	// KS傅农ID : 10磊府
};



class CFileTraffic : public CFileBase
{
public:
	CFileTraffic();
	~CFileTraffic();

protected:
	uint32_t m_nEntIdx;
	vector<stTraffic> m_vtTraffic;

private:	

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
	//virtual bool LoadDataByIdx(IN const uint32_t idx);

	virtual size_t ReadBody(FILE* fp);
};

