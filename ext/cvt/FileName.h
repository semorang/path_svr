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
	std::unordered_map<std::string, uint32_t> m_mapDic;
	uint32_t m_idxDic;

public:
	virtual bool Initialize();
	virtual void Release();
	
	uint32_t AddData(char* pszName);
	
	//virtual size_t WriteBase(FILE* fp);
	virtual size_t WriteHeader(FILE* fp);
	virtual size_t WriteIndex(FILE* fp);
	virtual size_t WriteBody(FILE* fp, IN const uint32_t fileOff);


	//virtual size_t ReadBase(FILE* fp);
	virtual size_t ReadHeader(FILE* fp);
	virtual size_t ReadIndex(FILE* fp);
	virtual size_t ReadBody(FILE* fp);
};

