#pragma once

#include "FileBase.h"

class CFileExtend : public CFileBase
{
public:
	CFileExtend();
	~CFileExtend();

protected:

private:

public:
	virtual bool Initialize();
	virtual void Release();

	virtual size_t WriteBody(FILE* fp, IN const uint32_t fileOff);
	virtual size_t ReadBody(FILE* fp);

	virtual bool LoadDataByIdx(IN const uint32_t idx);
};

