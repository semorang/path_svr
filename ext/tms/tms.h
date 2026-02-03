#pragma once

#include "../include/types.h"

#include <vector>
#include <cstring>

#define USE_REAL_ROUTE_TSP //실제 경로 기반 TSP
#define USE_REAL_ROUTE_TSP_COST // 실제 경로 코스트 기준


typedef enum
{
	TYPE_TSP_VALUE_COST,
	TYPE_TSP_VALUE_DIST,
	TYPE_TSP_VALUE_TIME
}TYPE_TSP_VALUE;


typedef struct _tagWaypoint
{
	int32_t id;
	SPoint position; // 좌표(WGS84)

	int32_t layoverTime; // 지점 소요 시간
	int32_t cargoVolume; // 지점 화물 수량

	// 각 지점별 추가 할당 리소스 정보
	int32_t weight; // 추가무게(kg)
	int32_t count; // 추가개수
	int32_t size; // 추가사이즈(cm, 가로+세로+높이)

	_tagWaypoint() {
		id = -1;
		position.x = position.y = 0.f;

		layoverTime = 0;
		cargoVolume = 0;

		weight = 0;
		count = 0;
		size = 0;
	}
}stWaypoint;


typedef struct _tagstCluster
{
	int32_t id; // 클러스터 ID
	double dist; // 예상 거리
	int32_t time; // 예상 시간
	int32_t cargo; // 화물 수량
	uint32_t etd; // 출발 예상 시각
	uint32_t eta; // 도착 예상 시각
	SPoint center; // 클러스터 중심 좌표

	std::vector<int32_t> vtPois;
	std::vector<SPoint> vtCoord;
	std::vector<SPoint> vtBorder;
	std::vector<int32_t> vtTimes;
	std::vector<int32_t> vtLayoverTimes;
	std::vector<int32_t> vtCargoVolume;
	std::vector<int32_t> vtDistances;

	_tagstCluster()
	{
		id = 0;
		dist = 0.f;
		time = 0.f;
		cargo = 0;
		etd = 0;
		eta = 0;
	}

	void init()
	{
		id = 0;
		dist = 0.f;
		time = 0.f;
		cargo = 0;
		etd = 0;
		eta = 0;
		center = { 0, 0 };

		vtPois.clear();
		vtCoord.clear();
		vtBorder.clear();
		vtTimes.clear();
		vtLayoverTimes.clear();
		vtCargoVolume.clear();
		vtDistances.clear();
	}
}stCluster;


typedef struct _tagstDistMatrix{
	int		nTotalDist;
	int		nTotalTime;
	double	dbTotalCost;

	_tagstDistMatrix() {
		nTotalDist = 0;
		nTotalTime = 0;
		dbTotalCost = 0.f;
	}
}stDistMatrix;


struct TruckOption
{
	int32_t height = 0; // 트럭 높이(cm), 제한
	int32_t weight = 0; // 트럭 중량(kg), 제한
	int32_t length = 0; // 트럭 길이(cm), 제한
	int32_t width = 0; // 트럭 너비(cm), 제한
	int32_t hazardous = 0; // 위험 수하물, 0://일발, 1:위험물, 상수원보호구역진입 제한

	bool isEnableTruckOption() const
	{
		return (height || weight || length || width || hazardous) ? true : false;
	}
};


typedef struct _tagBaseOption
{
	//string userId; // 대입할때 주소복사 되지 않도록 주의할것
	//string uri; // 데이터 경로, 기본적으로 timestamp 값을 준다.
	char userId[128];
	char route_mode[128]; // 탐색 모드, "driving", "tsp", "clustering" ...
	char route_cost[128]; // 탐색 확장 코스트 파일
	int32_t option; // 탐색 옵션,
	int32_t avoid; // 회피 옵션,
	int32_t traffic; // 교통 정보, 0:미사용, 1:실시간(REAL), 2:통계(STATIC), 3:실시간-통계(REAL-STATIC)
	int32_t free; // 무료 적용, 0:미사용, 1:무료
	int32_t timestamp;
	int32_t fileCache; // not use
	int32_t artifact; // 0:NONE, 1:RDM, 2:RPM, 3:RDM&RPM"
	int32_t mobility;
	int32_t distance_type; // RDM 생성 타입, 0:미사용(직선거리) 1:경로
	int32_t compare_type; // RDM 비교 타입, 0:미사용(기본) 1:코스트, 2:거리, 3:시간
	int32_t expand_method; // RDM 확장 방식, 0:기본, 1:레벨 단계화	

	TruckOption truck; // 트럭옵션

	_tagBaseOption()
	{
		//userId = "";
		//uri = "";
		memset(userId, 0x00, sizeof(userId));
		memset(route_mode, 0x00, sizeof(route_mode));
		memset(route_cost, 0x00, sizeof(route_cost));
		option = 0;
		avoid = 0;
		timestamp = 0;
		traffic = 0;
		free = 0;
		fileCache = 0; // not use
		artifact = 0; // 0:NONE, 1 : RDM, 2 : RPM, 3 : RDM&RPM",
		mobility = TYPE_MOBILITY_VEHICLE;
		distance_type = 1; // "0:NONE, 1:ROUTE",
		compare_type = 0; // "0:NONE, 1:COST, 2:DIST, 3:TIME",
		expand_method = 1; // "0:NONE, 1:LEVEL_PROPAGATION", // 2025-08-20 기본 레벨확장방식 사용
	}

	_tagBaseOption& operator=(const _tagBaseOption& rhs)
	{
		strcpy(this->userId, rhs.userId);
		strcpy(this->route_mode, rhs.route_mode);
		strcpy(this->route_cost, rhs.route_cost);
		//this->userId = rhs.userId;
		//this->uri = rhs.uri;
		this->option = rhs.option;
		this->avoid = rhs.avoid;
		this->traffic = rhs.traffic;
		this->timestamp = rhs.timestamp;
		this->free = rhs.free;
		this->fileCache = rhs.fileCache;
		this->artifact = rhs.artifact;
		this->mobility = rhs.mobility;

		this->distance_type = rhs.distance_type;
		this->compare_type = rhs.compare_type;
		this->expand_method = rhs.expand_method;

		memcpy(&this->truck, &rhs.truck, sizeof(this->truck));

		return *this;
	}
}BaseOption;


typedef struct _tagTspOption
{
	BaseOption baseOption;

	int32_t algorithm;
	int32_t geneSize;
	int32_t individualSize;
	int32_t loopCount;
	int32_t seed; // 랜덤 seed
	int32_t compareType;
	int32_t endpointType; // 지점 고정, 0:출발지 회귀, 1:출발지 고정, 2:도착지 고정, 3:출발지-도착지 고정, 4:출-도착지 미지정(랜덤)

	_tagTspOption()
	{
		algorithm = TYPE_TSP_ALGORITHM_GOOGLEOR;
		geneSize = 0;
		individualSize = 100;
		loopCount = 1000;
		seed = 10000;// 1000 + 49 * 2;
		compareType = TYPE_TSP_VALUE_DIST;
		endpointType = TYPE_TSP_ENDPOINT_RECURSIVE;
	}

	_tagTspOption& operator=(const _tagTspOption& rhs)
	{
		this->baseOption = rhs.baseOption;

		this->algorithm = rhs.algorithm;
		this->geneSize = rhs.geneSize;
		this->individualSize = rhs.individualSize;
		this->loopCount = rhs.loopCount;
		this->seed = rhs.seed;
		this->compareType = rhs.compareType;
		this->endpointType = rhs.endpointType;

		return *this;
	}
}TspOption;


typedef struct _tagClusteringOption
{
	TspOption tspOption;

	int32_t algorithm;
	int32_t seed; // 랜덤 seed
	int32_t compareType;

	union
	{
		int32_t optionValues[20];
		struct
		{
			int32_t divisionType; 		// 분배 타입, 0:갯수균등, 1:거리균등, 2:시간균등, 3:물량균등, 4:링크단위
			int32_t limitCluster;		// 최대 배차(클러스터링) 차량 수.
			int32_t limitValue;			// 차량당 최대 배송지수, 최대 거리(미터), 최대 시간(초)분으로 입력받아 초로 변환.
			int32_t limitDeviation; 	// 차량당 최대 운행 정보 편차
			int32_t max_spot; 			// 차량당 최대 운송 가능 개수
			int32_t max_distance; 		// 차량당 최대 운행 가능 거리
			int32_t max_time; 			// 차량당 최대 운행 가능 시간
			int32_t max_cargo; 			// 차량당 최대 적재 가능 화물
			int32_t reservation; 		// 예약 시각
			int32_t reservationType; 	// 예약 타입, 0:미사용, 1:출발시간, 2:도착시간
			int32_t endpointType; 		// 지점 고정, 0:미사용, 1:출발지 고정, 2:도착지 고정, 3:출발지-도착지 고정, 4:출발지 회귀
			int32_t additionalType; 	// 추가 배당 타입, "0:미사용, 1:갯수, 2:무게(g), 3:사이즈(cm)",
			int32_t additionalLimit; 	// 추가 배당 최대 한도
			int32_t reserved[7];
		}opt; // default
		struct
		{
			int32_t divisionType; 		// 분배 타입, 0:갯수균등, 1:거리균등, 2:시간균등, 3:물량균등, 4:링크단위
			int32_t limitLength; 		// 링크 분배 길이, 0:전체사용, 100~N:최소 Nm => 최소 거리는 전체길이 / ((전체길이/Nm) + 1)
			int32_t reserved[18];
		}bylink;
	};


	_tagClusteringOption()
	{
		algorithm = TYPE_TSP_ALGORITHM_GOOGLEOR;
		seed = 10006;// 1000 + 49 * 2;
		compareType = TYPE_TSP_VALUE_DIST;

		memset(optionValues, 0x00, sizeof(optionValues));
	}

	_tagClusteringOption& operator=(const _tagClusteringOption& rhs)
	{
		this->tspOption = rhs.tspOption;

		this->algorithm = rhs.algorithm;
		this->seed = rhs.seed;
		this->compareType = rhs.compareType;

		memcpy(this->optionValues, rhs.optionValues, sizeof(this->optionValues));

		return *this;
	}
}ClusteringOption;


#pragma pack (push, 1)
 // 데이터 정보 헤더
 typedef struct _tagFileHeaderRDM
 {
 	int32_t crcData;	// crc 값.
 	int32_t cntItem;	// N x N matrix의 N 갯수
 	int32_t offOption;	// 옵션 시작 offset
 	int32_t offOrigin;	// 좌표 시작 offset
 	int32_t offBody;	// 데이터 시작 offset
 }FileHeaderRDM;


typedef struct _tagFileInfoRDM
{
	std::string type;	// data type : file, base64 ...
	std::string form;	// file format : bin, zip, txt ...
	std::string data;	// data : name, rwa data ...
	size_t size;		// data size
	time_t created;		// data time
}FileInfoRDM;


typedef struct _tagWeightMatrixPath
{
	int32_t startIndex;
	int32_t endIndex;
	std::vector<SPoint> path;
}WeightMatrixPath;
#pragma pack (pop)