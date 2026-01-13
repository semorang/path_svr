#pragma once

#include "../include/MapDef.h"
#include "../include/types.h"
#include "DataManager.h"
#include "RoutePlan.h"

typedef struct _tagRouteProbablePath {
	KeyID LinkId; // 링크 ID
	KeyID SNodeId; // s노드 ID
	KeyID ENodeId; // s노드 ID
	int NodeDir; 
	// vector<stLinkInfo*> JctLinks; // 정션 링크
	unordered_map<uint64_t, stLinkInfo*> JctLinks;

	_tagRouteProbablePath() {
		LinkId.llid = 0;
		SNodeId.llid = 0;
		ENodeId.llid = 0;
		NodeDir = 0;
	}

	~_tagRouteProbablePath() {
		if (!JctLinks.empty()) {
			for (const auto& junction : JctLinks) {
				delete ((stLinkInfo*)junction.second);
			}
			JctLinks.clear();
			unordered_map<uint64_t, stLinkInfo*>().swap(JctLinks);
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

	void GetErrorResult(IN const int32_t err_code, OUT string& strJson);
	void GetRouteResult(IN const RouteResultInfo* pResult, IN const bool isJunction, OUT string& strJson);
	void GetMultiRouteResult(IN const vector<RouteResultInfo>& vtRouteResults, IN const bool isJunction, OUT string& strJson);
	int32_t GetMapsRouteResult(IN const RouteResultInfo* pResult, OUT string& strJson);
	int32_t GetMapsMultiRouteResult(IN const vector<RouteResultInfo>& vtRouteResults, OUT string& strJson);
	void GetClusteringResult(IN const Cluster& CLUST, IN const RouteDistMatrix& RDM, IN const char* pszFile, OUT string& strJson);
	void GetGroupingResult(IN const Cluster& CLUST, OUT string& strJson);
	void GetBoundaryResult(IN const vector<SPoint>& vtBoundary, OUT string& strJson);
	void GetBestWaypointResult(IN const BestWaypoints& TSP, IN const RouteDistMatrix& RDM, IN const char* pszFile, OUT string& strJson);
	void GetWeightMatrixResult(IN const RouteDistMatrix& RDM, OUT string& strJson);
	void GetWeightMatrixRouteLineResult(IN const RouteDistMatrix& RDM, OUT string& strJson);

	int32_t GetWeightMatrixResultFile(IN const RouteDistMatrix& RDM, IN const bool includeRDM, IN const char* pszFile);
	int32_t GetWeightMatrixRouteLineResultFile(IN const RouteDistMatrix& RDM, IN const char* pszFile, IN const bool includeBinary);

		// 분기점 노드의 경로선을 제외한 나머지 링크 얻기 // like a mpp
	// length = 본경로의 최대 길이, 0: 전체, ...
	// expansion = 본 경로 외 확장할 레벨, 0: 현재 경로만, 1: 현재 경로에서 1단계만 가지치기 ...
	// branchLength = 가지치기한 경로의 최대 길이
	
	/*const size_t GetRouteProbablePath(IN const RouteResultInfo* pResult, OUT vector<RouteProbablePath*>& vtJctInfo, IN const double length = 0, IN const int32_t expansion = 1, IN const double branchLength = 100);*/
	const size_t GetRouteProbablePath(IN unordered_map<uint64_t, int>& mapVisited, IN const vector<RouteResultLinkEx>& vtLinkInfo, OUT vector<RouteProbablePath*>& vtJctInfo, IN const double length = 0, IN const int32_t expansion = 1, IN const double branchLength = 100);
	const size_t GetRouteProbablePathEx(IN const unordered_map<uint64_t, int>* pmapVisited, IN double addedlength, IN const int32_t depth, IN const double branchLength, OUT vector<RouteProbablePath*>& vtJctInfo);

	void GetOptimalPosition(IN const stReqOptimal* pReq, IN const stOptimalPointInfo* pResult, OUT string& strJson);
	void GetMultiOptimalPosition(IN const vector<stOptimalPointInfo>* pvtResult, OUT string& strJson);
private:
	CDataManager* m_pDataMgr;

	bool GetRouteResultJson(IN const RouteResultInfo* pResult, IN const time_t time, IN const bool isJunction, OUT void* pJson);
	bool GetMapsRouteResultJson(IN const RouteResultInfo* pResult, IN const time_t time, OUT void* pJson);
};

