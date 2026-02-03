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


//#define EXPIRY_DURATION		60 * 60 * 24 * 31 // 최대 31일간 유효함
#define EXPIRY_DURATION		60 * 60 * 24 * 365 // 최대 1년간 유효함


struct stClustValue
{
	int32_t nType = 0; // 값 타입

	double dwDist = 0.f; // 거리
	int32_t nSpot = 0; // 방문지
	int32_t nTime = 0; // 시간
	int32_t nCargo = 0; // 화물 

	stClustValue(const int type) {
		nType = type;
	}

	void init(void) {
		//nType = 0;

		dwDist = 0.f;
		nSpot = 0;
		nTime = 0;
		nCargo = 0;
	}

	int32_t getValueType(void) {
		return nType;
	}

	void setValueType(const int type) {
		nType = type;
	}

	double getCurrentValue(void) const {
		double value = 0.f;
		if (nType == CLUST_VALUE_TYPE_SPOT) {
			value = nSpot;
		} else if (nType == CLUST_VALUE_TYPE_DIST) {
			value = dwDist;
		} else if (nType == CLUST_VALUE_TYPE_TIME) {
			value = nTime;
		} else if (nType == CLUST_VALUE_TYPE_CARGO) {
			value = nCargo;
		}
		return value;
	}

	void setCurrentValue(const double value) {
		if (nType == CLUST_VALUE_TYPE_SPOT) {
			nSpot = value;
		} else if (nType == CLUST_VALUE_TYPE_DIST) {
			dwDist = value;
		} else if (nType == CLUST_VALUE_TYPE_TIME) {
			nTime = value;
		} else if (nType == CLUST_VALUE_TYPE_CARGO) {
			nCargo = value;
		}
	}

	double getValue(const int32_t type) const {
		double value = 0.f;
		if (type == CLUST_VALUE_TYPE_SPOT) {
			value = nSpot;
		} else if (type == CLUST_VALUE_TYPE_DIST) {
			value = dwDist;
		} else if (type == CLUST_VALUE_TYPE_TIME) {
			value = nTime;
		} else if (type == CLUST_VALUE_TYPE_CARGO) {
			value = nCargo;
		}
		return value;
	}

	void setValue(const int32_t type, const double value) {
		if (type == CLUST_VALUE_TYPE_SPOT) {
			nSpot = value;
		} else if (type == CLUST_VALUE_TYPE_DIST) {
			dwDist = value;
		} else if (type == CLUST_VALUE_TYPE_TIME) {
			nTime = value;
		} else if (type == CLUST_VALUE_TYPE_CARGO) {
			nCargo = value;
		}
	}

	void addValue(const int32_t type, const double value)
	{
		if (type == CLUST_VALUE_TYPE_SPOT) {
			nSpot += value;
		} else if (type == CLUST_VALUE_TYPE_DIST) {
			dwDist += value;
		} else if (type == CLUST_VALUE_TYPE_TIME) {
			nTime += value;
		} else if (type == CLUST_VALUE_TYPE_CARGO) {
			nCargo += value;
		}
	}

	void addCurrentValue(const double value)
	{
		if (nType == CLUST_VALUE_TYPE_SPOT) {
			nSpot += value;
		} else if (nType == CLUST_VALUE_TYPE_DIST) {
			dwDist += value;
		} else if (nType == CLUST_VALUE_TYPE_TIME) {
			nTime += value;
		} else if (nType == CLUST_VALUE_TYPE_CARGO) {
			nCargo += value;
		}
	}

	stClustValue& operator=(const stClustValue& rhs)
	{
		nType = rhs.nType; // 타입까지 바꾸는게 맞나????

		dwDist = rhs.dwDist;
		nSpot = rhs.nSpot;
		nTime = rhs.nTime;
		nCargo = rhs.nCargo;

		return *this;
	}

	stClustValue& operator+(const stClustValue& rhs)
	{
		dwDist += rhs.dwDist;
		nSpot += rhs.nSpot;
		nTime += rhs.nTime;
		nCargo += rhs.nCargo;

		return *this;
	}
};


class CTMSManager
{
public:
	CTMSManager();
	~CTMSManager();

protected:


private:
	uint64_t currentTimestamp;
	int32_t m_nMaxLimitCount;

protected:
	

private:
	CDataManager* m_pDataMgr;

public:
	bool Initialize(void);
	void Release(void);

	void SetDataMgr(IN CDataManager* pDataMgr);
	void SetLimitPointCount(IN const int32_t count = 0) { m_nMaxLimitCount = count; }
	int32_t GetLimitPointCount(void) const { return m_nMaxLimitCount; }

	int32_t GetBestway(IN const TspOption* pTspOpt, IN const std::vector<std::vector<stDistMatrix>>& vtDistMatrix, IN const std::vector<stWaypoint>& vtOrigin, OUT std::vector<int32_t>& vtBestWaypoints, OUT double& dist, OUT int32_t& time);
	int32_t GetCluster(IN const ClusteringOption* pClustOpt, IN const std::vector<std::vector<stDistMatrix>>& vtDistMatrix, IN const std::vector<stWaypoint>& vtOrigin, OUT std::vector<stCluster>& vtDistrict, OUT std::vector<SPoint>& vtEndPoint);
	int32_t GetGroup(IN const ClusteringOption* pClustOpt, IN const std::vector<stWaypoint>& vtOrigin, OUT std::vector<stCluster>& vtDistrict);
	int32_t GetBoundary(IN std::vector<SPoint>& vtPois, OUT std::vector<SPoint>& vtBoundary, OUT SPoint& center);

	int32_t ParsingRequestBaseOption(IN const char* szRequest, OUT BaseOption& option);
	int32_t ParsingRequestRoute(IN const char* szRequest, OUT BaseOption& baseOpt, OUT std::vector<stWaypoint>& vtOrigin);
	int32_t ParsingRequestWeightMatrix(IN const char* szRequest, OUT BaseOption& baseOpt, OUT std::vector<stWaypoint>& vtOrigin, OUT std::vector<stWaypoint>& vtDestination, OUT std::vector<std::vector<stDistMatrix>>& vtDistanceMatrix, OUT int32_t& typeDistMatrix);
	int32_t ParsingRequestWeightMatrixRoute(IN const char* szRequest, OUT BaseOption& baseOpt, OUT std::vector<stWaypoint>& vtOrigin, OUT std::vector<stWaypoint>& vtDestination, OUT std::vector<std::vector<stDistMatrix>>& vtDistanceMatrix/*, OUT int32_t& typeDistMatrix*/);
	int32_t ParsingRequestWeightMatrixRoutePathIndex(IN const char* szRequest, OUT std::string& fileName, OUT size_t& fileSize, OUT std::vector<std::vector<FileIndex>>& vtPathMatrixIndex);
	int32_t ParsingRequestWeightMatrixRoutePathData(IN const char* szRequest, OUT std::vector<WeightMatrixPath>& vtPathMatrixData);
	int32_t ParsingRequestBestway(IN const char* szRequest, OUT TspOption& tspOpt, OUT std::vector<stWaypoint>& vtOrigin, OUT std::vector<std::vector<stDistMatrix>>& vtDistMatrix);
	int32_t ParsingRequestCluster(IN const char* szRequest, OUT ClusteringOption& clustOpt, OUT std::vector<stWaypoint>& vtOrigin);
	int32_t ParsingRequestGroup(IN const char* szRequest, OUT ClusteringOption& clustOpt, OUT std::vector<stWaypoint>& vtOrigin);
	//int32_t LoadWeightMatrix(IN const char* szFileName, IN const size_t sizeFile, OUT BaseOption& option, OUT std::std::vector<Origins>& vtOrigin, OUT std::vector<std::vector<stDistMatrix>>& vtDistMatrix);
	//int32_t LoadWeightMatrixRouteLine(IN const char* szFileName, IN const size_t sizeFile, OUT vector<vector<FileIndex>>& vtPathMatrixIndex);
	int32_t SaveWeightMatrix(IN const char* szFileName, IN const BaseOption* pOption, IN const int cntItem, IN const int sizeItem, IN const uint32_t crc, IN const std::vector<stWaypoint>& vtOrigin, IN const std::vector<std::vector<stDistMatrix>>& vtDistMatrix);
	int32_t SaveWeightMatrixRouteLine(IN const char* szFileName, IN const std::vector<std::vector<stPathMatrix>>& vtPathMatrix);

private:
	int32_t Clustering(IN const ClusteringOption* pClustOpt, IN const std::vector<std::vector<stDistMatrix>>& vtDistMatrix, IN const std::vector<stWaypoint>& vtWaypoints, IN const std::vector<int32_t>& vtBestway, IN const int32_t nBonusValue,  OUT std::vector<stCluster>& vtClusters);
	int32_t DevideClusterUsingTsp(IN const ClusteringOption* pClustOpt, IN const std::vector<std::vector<stDistMatrix>>& vtDistMatrix, IN const std::vector<stWaypoint>& vtWaypoints, IN const std::vector<int32_t>& vtBestways, IN const int32_t firstIdx, IN const int32_t lastIdx, IN OUT int32_t& bonusValue, OUT stCluster& cluster, OUT std::vector<int32_t>& vtRemains);
	int32_t DevideClusterUsingLink(IN const ClusteringOption* pClustOpt, IN const std::vector<stWaypoint>& vtOrigin, OUT std::vector<stCluster>& vtDistrict);
	int32_t GetRecommendedDeviation(IN ClusteringOption* pClustOpt, IN const std::vector<std::vector<stDistMatrix>>& vtDistMatrix, IN std::vector<stWaypoint>& vtWaypoints, IN std::vector<int32_t>& vtBestway, OUT std::vector<stCluster>& vtDistrict);
	int32_t GetNewBestway(IN const TspOption* pTspOpt, IN const std::vector<std::vector<stDistMatrix>>& vtDistMatrix, IN const std::vector<stWaypoint>& vtWaypoints, OUT std::vector<stWaypoint>& vtNewWaypoints, OUT std::vector<int32_t>& vtNewBestways, OUT stClustValue& clustValue);
};

int32_t GetMatrixPathVertex(IN const stPathMatrix* pMatrix, IN CDataManager* pDataMgr, OUT std::vector<SPoint>& vtLines);
