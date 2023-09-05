#pragma once

#include "../include/MapDef.h"
#include "../include/types.h"
#include "DataManager.h"

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args);

class CRoutePackage
{
public:
	CRoutePackage();
	virtual ~CRoutePackage();

	bool Initialize(void);
	void Release(void);

	void SetDataMgr(IN CDataManager* pDataMgr);
	void GetErrorResult(IN const int32_t err_code, IN const char* err_msg,  OUT string& strJson);
	void GetMultiRouteResult(IN const RouteResultInfo* pResult, OUT string& strJson);
	void GetMultiRouteResultForiNavi(IN const RouteResultInfo* pResult, OUT string& strResult);
	void GetClusteringResult(IN const vector<stDistrict>& vtClusters, OUT string& strJson);
	void GetBoundaryResult(IN const vector<SPoint>& vtBoundary, OUT string& strJson);

private:
	CDataManager* m_pDataMgr;
};

