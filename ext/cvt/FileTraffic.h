#pragma once

#include "FileBase.h"

//class CFileManager;
#include "../route/MapTraffic.h"


struct stTrafficTTL
{
	uint64_t LinkId; // LINK_ID : 6磊府
	uint64_t TTL_id_p;	// TTL_ID 沥 : 14磊府
	uint64_t TT_id_n; // TTL_ID 开 : 14磊府
};


class CFileTraffic : public CFileBase
{
public:
	CFileTraffic();
	~CFileTraffic();

protected:

private:	

public:
	virtual bool Initialize();
	virtual void Release();

	virtual bool ParseData(IN const char* fname);
	virtual bool GenServiceData();

	virtual bool LoadData(IN const char* szFilePath);

	virtual size_t WriteBody(FILE* fp, IN const uint32_t fileOff);
	virtual size_t ReadBody(FILE* fp);
};

