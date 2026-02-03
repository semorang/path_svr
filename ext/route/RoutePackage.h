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
	std::unordered_map<uint64_t, stLinkInfo*> JctLinks;

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
			std::unordered_map<uint64_t, stLinkInfo*>().swap(JctLinks);
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

	void GetErrorResult(IN const int32_t err_code, OUT std::string& strJson);
	void GetRouteResult(IN const RouteResultInfo* pResult, IN const bool isJunction, OUT std::string& strJson);
	void GetMultiRouteResult(IN const std::vector<RouteResultInfo>& vtRouteResults, IN const bool isJunction, OUT std::string& strJson);
	int32_t GetMapsRouteResult(IN const RouteResultInfo* pResult, OUT std::string& strJson);
	int32_t GetMapsMultiRouteResult(IN const std::vector<RouteResultInfo>& vtRouteResults, OUT std::string& strJson);
	void GetClusteringResult(IN const Cluster& CLUST, IN const RouteDistMatrix& RDM, OUT std::string& strJson);
	void GetGroupingResult(IN const Cluster& CLUST, OUT std::string& strJson);
	void GetBoundaryResult(IN const std::vector<SPoint>& vtBoundary, OUT std::string& strJson);
	void GetBestWaypointResult(IN const BestWaypoints& TSP, IN const RouteDistMatrix& RDM, IN const char* pszFile, OUT std::string& strJson);
	void GetWeightMatrixResult(IN const RouteDistMatrix& RDM, OUT std::string& strJson);
	void GetWeightMatrixPathResult(IN const std::vector<WeightMatrixPath>& vtRoutes, OUT std::string& strJson);

	// RDM 데이터를 직접 파일에 json 포맷으로 저장 (대용량 결과에서 cJSON 등 느려지는 현상 대체할 목적)
	int32_t SaveWeightMatrixResultFile(IN const RouteDistMatrix& RDM, IN const bool artifact);
	int32_t SaveWeightMatrixPathResultFile(IN const std::vector<WeightMatrixPath>& vtRoutes, IN const bool artifact, IN const time_t tmCreated, IN const char* pszUser);

		// 분기점 노드의 경로선을 제외한 나머지 링크 얻기 // like a mpp
	// length = 본경로의 최대 길이, 0: 전체, ...
	// expansion = 본 경로 외 확장할 레벨, 0: 현재 경로만, 1: 현재 경로에서 1단계만 가지치기 ...
	// branchLength = 가지치기한 경로의 최대 길이
	
	/*const size_t GetRouteProbablePath(IN const RouteResultInfo* pResult, OUT vector<RouteProbablePath*>& vtJctInfo, IN const double length = 0, IN const int32_t expansion = 1, IN const double branchLength = 100);*/
	const size_t GetRouteProbablePath(IN std::unordered_map<uint64_t, int>& mapVisited, IN const std::vector<RouteResultLinkEx>& vtLinkInfo, OUT std::vector<RouteProbablePath*>& vtJctInfo, IN const double length = 0, IN const int32_t expansion = 1, IN const double branchLength = 100);
	const size_t GetRouteProbablePathEx(IN const std::unordered_map<uint64_t, int>* pmapVisited, IN double addedlength, IN const int32_t depth, IN const double branchLength, OUT std::vector<RouteProbablePath*>& vtJctInfo);

	void GetOptimalPosition(IN const stReqOptimal* pReq, IN const stOptimalPointInfo* pResult, OUT std::string& strJson);
	void GetMultiOptimalPosition(IN const std::vector<stOptimalPointInfo>* pvtResult, OUT std::string& strJson);
private:
	CDataManager* m_pDataMgr;

	bool GetRouteResultJson(IN const RouteResultInfo* pResult, IN const time_t time, IN const bool isJunction, OUT void* pJson);
	bool GetMapsRouteResultJson(IN const RouteResultInfo* pResult, IN const time_t time, OUT void* pJson);
};

