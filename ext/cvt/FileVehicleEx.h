#pragma once

#include "FileBase.h"

struct stVehicleEx
{
	KeyID link_id;

	int32_t nMesh;
	int32_t nSnodeId;
	int32_t nEnodeId;
	int32_t nDirCode; // 정역
	int32_t nAttribute;

	double dWeight;
	double dHeight;

	stVehicleEx() {
		nMesh = 0;
		nSnodeId = 0;
		nEnodeId = 0;
		nDirCode = 0;
		nAttribute = 0;
		dWeight = 0.f;
		dHeight = 0.f;
	}
};


class CFileVehicleEx : public CFileBase
{
public:
	CFileVehicleEx();
	~CFileVehicleEx();

protected:
	std::vector<stVehicleEx> m_vtVehicleEx;

private:

public:
	virtual bool Initialize();
	virtual void Release();

	virtual bool ParseData(IN const char* fname);
	virtual bool GenServiceData();

	virtual size_t WriteBody(FILE* fp, IN const uint32_t fileOff);
	virtual size_t ReadBody(FILE* fp);

	virtual bool LoadDataByIdx(IN const uint32_t idx);
};

