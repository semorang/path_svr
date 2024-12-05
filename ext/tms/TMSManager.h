#pragma once

#include "../include/MapDef.h"
#include "../route/DataManager.h"
#include "../route/RoutePlan.h"
#if 1 //!defined(USE_REAL_ROUTE_TSP)
#include "../tsp/environment.h"
#endif

#if defined(__USE_TEMPLETE)
#include "MapBase.h"
#endif

#pragma pack (push, 1)

#pragma pack (pop)



class CTMSManager
{
public:
	CTMSManager();
	~CTMSManager();

protected:


private:
	uint64_t currentTimestamp;

protected:
	

private:
	CDataManager* m_pDataMgr;

public:
	bool Initialize(void);
	void Release(void);

	void SetDataMgr(IN CDataManager* pDataMgr);

	int32_t GetBestway(IN const TspOptions* pTspOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN vector<stWaypoints>& vtOrigin, OUT vector<uint32_t>& vtBestWaypoints, OUT double& dist, OUT int32_t& time);
	int32_t GetCluster(IN const TspOptions* pTspOpt, IN const ClusteringOptions* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<SPoint>& vtOrigin, OUT vector<stDistrict>& vtDistrict, OUT vector<SPoint>& vtPositionLock);
	int32_t GetBoundary(IN vector<SPoint>& vtPois, OUT vector<SPoint>& vtBoundary, OUT SPoint& center);

	uint32_t GetRequestCluster(IN const char* szRequest, OUT vector<SPoint>& vtOrigin, OUT TspOptions& tspOpt, OUT ClusteringOptions& clustOpt);
	uint32_t GetRequestBestway(IN const char* szRequest, OUT vector<SPoint>& vtOrigin, OUT TspOptions& tspOpt);
	uint32_t LoadWeightMatrix(IN const char* szFileName, IN const int cntItem, IN const int sizeItem, IN const uint32_t crc, OUT RequestRouteInfo& reqInfo, OUT vector<vector<stDistMatrix>>& vtDistMatrix);
	uint32_t SaveWeightMatrix(IN const char* szFileName, IN const RequestRouteInfo* pReqInfo, IN const int cntItem, IN const int sizeItem, IN const uint32_t crc, IN const vector<vector<stDistMatrix>>& vtDistMatrix);

private:
	bool Clustering(IN const ClusteringOptions* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtWaypoints, IN const vector<uint32_t>& vtBestway, IN const int32_t nBonusValue,  OUT vector<stDistrict>& vtClusters);

	int32_t DevideClusterUsingTsp(IN const ClusteringOptions* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtWaypoints, IN const vector<uint32_t>& vtBestways, IN const int32_t firstIdx, IN const int32_t lastIdx, IN OUT int32_t& bonusValue, OUT stDistrict& cluster, OUT vector<uint32_t>& vtRemains);
};

