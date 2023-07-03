#pragma once

#include "MapDef.h"
#include "DataManager.h"

class CRoutePackage
{
public:
	CRoutePackage();
	virtual ~CRoutePackage();

	bool Initialize(void);
	void Release(void);

	void SetDataMgr(IN CDataManager* pDataMgr);
	void GetMultiRouteResultForiNavi(IN const RouteResultInfo* pResult, OUT string& strResult);

private:
	CDataManager* m_pDataMgr;
};

