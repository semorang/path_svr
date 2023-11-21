#pragma once

#include "../include/MapDef.h"
#include "../include/types.h"
#include "DataManager.h"


typedef struct _tagRouteProbablePath {
	KeyID LinkId; // 링크 ID
	KeyID NodeId; // 노드 ID
	vector<stLinkInfo*> JctLinks; // 정션 링크

	_tagRouteProbablePath() {
		LinkId.llid = 0;
		NodeId.llid = 0;
	}

	~_tagRouteProbablePath() {
		if (!JctLinks.empty()) {
			for (const auto& junction : JctLinks) {
				delete ((stLinkInfo*)junction);
			}
			JctLinks.clear();
			vector<stLinkInfo*>().swap(JctLinks);
		}
	}
}RouteProbablePath;



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
	void GetMultiRouteResult(IN const RouteResultInfo* pResult, IN const int target, OUT string& strJson);
	void GetMultiRouteResultForiNavi(IN const RouteResultInfo* pResult, OUT string& strResult);
	void GetClusteringResult(IN const vector<stDistrict>& vtClusters, OUT string& strJson);
	void GetBoundaryResult(IN const vector<SPoint>& vtBoundary, OUT string& strJson);
		// 분기점 노드의 경로선을 제외한 나머지 링크 얻기 // like a mpp
	// length = 본경로의 최대 길이, 0: 전체, ...
	// expansion = 본 경로 외 확장할 레벨, 0: 현재 경로만, 1: 현재 경로에서 1단계만 가지치기 ...
	// branchLength = 가지치기한 경로의 최대 길이
	
	/*const size_t GetRouteProbablePath(IN const RouteResultInfo* pResult, OUT vector<RouteProbablePath*>& vtJctInfo, IN const double length = 0, IN const int32_t expansion = 1, IN const double branchLength = 100);*/
	const size_t GetRouteProbablePath(IN const vector<RouteResultLinkEx>& vtLinkInfo, OUT vector<RouteProbablePath*>& vtJctInfo, IN const double length = 0, IN const int32_t expansion = 1, IN const double branchLength = 100);

	void GetOptimalPosition(IN const stReqOptimal* pReq, IN const stOptimalPointInfo* pResult, OUT string& strJson);
private:
	CDataManager* m_pDataMgr;
};

