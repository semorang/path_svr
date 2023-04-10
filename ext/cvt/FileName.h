#pragma once

#include "FileBase.h"

//class CFileManager;


class CFileName : public CFileBase
{
public:
	CFileName();
	~CFileName();

protected:


private:
	unordered_map<string, uint32_t> m_mapDic;
	uint32_t m_idxDic;

public:
	virtual bool Initialize();
	virtual void Release();
	
	uint32_t AddData(char* pszName);

	//virtual bool OpenFile(IN const char* szFilePath);
	virtual bool SaveData(IN const char* szFilePath);
	
	//virtual size_t WriteBase(FILE* fp);
	virtual size_t WriteHeader(FILE* fp);
	virtual size_t WriteIndex(FILE* fp);
	virtual size_t WriteBody(FILE* fp, IN const uint32_t fileOff);

	virtual bool LoadData(IN const char* fname);

	//virtual size_t ReadBase(FILE* fp);
	virtual size_t ReadHeader(FILE* fp);
	virtual size_t ReadIndex(FILE* fp);
	virtual size_t ReadBody(FILE* fp);

};

