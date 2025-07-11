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

// 데이터 정보 헤더
typedef struct _tagFileHeaderRDM
{
	uint32_t crcData;	// crc 값
	uint32_t cntItem;	// N x N matrix의 N 갯수
	uint32_t offOption;	// 옵션 시작 offset
	uint32_t offOrigin;	// 좌표 시작 offset
	uint32_t offBody;	// 데이터 시작 offset
}FileHeaderRDM;


typedef enum {
	CLUST_VALUE_TYPE_SPOT = TYPE_CLUSTER_DEVIDE_BY_COUNT, // 방문지
	CLUST_VALUE_TYPE_DIST = TYPE_CLUSTER_DEVIDE_BY_DIST, // 거리
	CLUST_VALUE_TYPE_TIME = TYPE_CLUSTER_DEVIDE_BY_TIME, // 시간
	CLUST_VALUE_TYPE_CARGO, // 화물
}CLUST_VALUE_TYPE;


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

protected:
	

private:
	CDataManager* m_pDataMgr;

public:
	bool Initialize(void);
	void Release(void);

	void SetDataMgr(IN CDataManager* pDataMgr);

	int32_t GetBestway(IN const TspOption* pTspOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtOrigin, OUT vector<int32_t>& vtBestWaypoints, OUT double& dist, OUT int32_t& time);
	int32_t GetCluster(IN const ClusteringOption* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<Origins>& vtOrigin, OUT vector<stDistrict>& vtDistrict, OUT vector<SPoint>& vtPositionLock);
	int32_t GetBoundary(IN vector<SPoint>& vtPois, OUT vector<SPoint>& vtBoundary, OUT SPoint& center);

	uint32_t ParsingRequestWeightMatrix(IN const char* szRequest, OUT BaseOption& baseOpt, OUT vector<Origins>& vtOrigin, OUT vector<vector<stDistMatrix>>& vtDistanceMatrix, OUT int32_t& typeDistMatrix);
	uint32_t ParsingRequestWeightMatrixRouteLine(IN const char* szRequest, OUT string& strFilePath, OUT size_t& sizeFile, OUT vector<vector<FileIndex>>& vtPathMatrixIndex);
	uint32_t ParsingRequestBestway(IN const char* szRequest, OUT TspOption& tspOpt, OUT vector<Origins>& vtOrigin);
	uint32_t ParsingRequestCluster(IN const char* szRequest, OUT ClusteringOption& clustOpt, OUT vector<Origins>& vtOrigin);
	uint32_t LoadWeightMatrix(IN const char* szFileName, IN const int cntItem, IN const int sizeItem, IN const uint32_t crc, OUT BaseOption& option, OUT vector<Origins>& vtOrigin, OUT vector<vector<stDistMatrix>>& vtDistMatrix);
	uint32_t LoadWeightMatrixRouteLine(IN const char* szFileName, IN const int sizeFile, OUT vector<vector<FileIndex>>& vtPathMatrixIndex);
	uint32_t SaveWeightMatrix(IN const char* szFileName, IN const BaseOption* pOption, IN const int cntItem, IN const int sizeItem, IN const uint32_t crc, IN const vector<Origins>& vtOrigin, IN const vector<vector<stDistMatrix>>& vtDistMatrix);
	uint32_t SaveWeightMatrixRouteLine(IN const char* szFileName, IN const vector<vector<stPathMatrix>>& vtPathMatrix);

private:
	uint32_t Clustering(IN const ClusteringOption* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtWaypoints, IN const vector<int32_t>& vtBestway, IN const int32_t nBonusValue,  OUT vector<stDistrict>& vtClusters);

	int32_t DevideClusterUsingTsp(IN const ClusteringOption* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtWaypoints, IN const vector<int32_t>& vtBestways, IN const int32_t firstIdx, IN const int32_t lastIdx, IN OUT int32_t& bonusValue, OUT stDistrict& cluster, OUT vector<int32_t>& vtRemains);

	int32_t GetRecommendedDeviation(IN ClusteringOption* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN vector<stWaypoints>& vtWaypoints, IN vector<int32_t>& vtBestway, OUT vector<stDistrict>& vtDistrict);

	int32_t GetNewBestway(IN const TspOption* pTspOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtWaypoints, OUT vector<stWaypoints>& vtNewWaypoints, OUT vector<int32_t>& vtNewBestways, OUT stClustValue& clustValue);
};

int32_t GetMatrixPathVertex(IN const stPathMatrix* pMatrix, IN CDataManager* pDataMgr, OUT vector<SPoint>& vtLines);