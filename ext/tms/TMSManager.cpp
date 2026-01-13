#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "../utils/UserLog.h"
#include "../utils/Strings.h"
#include "../utils/GeoTools.h"
#include "../utils/convexhull.h"
#include "../utils/DataConvertor.h"

#include "../thlib/ZLIB.H"

#if defined(USE_CJSON)
#include "../libjson/cjson/cJSON.h"
#endif

#include "prim.h"
#include "tsp_ga.h"

#include "TMSManager.h"
#include "GnKmeansClassfy.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CTMSManager::CTMSManager()
{
	currentTimestamp = 0;
	m_nMaxLimitCount = 0;

	m_pDataMgr = nullptr;
}


CTMSManager::~CTMSManager()
{
	Release();
}


bool CTMSManager::Initialize(void)
{
	bool ret = false;

	return ret;
}


void CTMSManager::Release(void)
{

}


void CTMSManager::SetDataMgr(IN CDataManager* pDataMgr)
{
	if (pDataMgr) {
		m_pDataMgr = pDataMgr;
	}
}


int32_t getNewDistrictResult(IN const ClusteringOption* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtWaypoints, OUT stDistrict& cluster)
{
	int totTime = 0;
	int totDist = 0;
	int totCargo = 0;

	int prevId = vtWaypoints[0].nId;
	int currId = 0;

	int nId = cluster.id;
	cluster.init();
	cluster.id = nId;

	for (const auto & way : vtWaypoints) {
		currId = way.nId;

		cluster.vtPois.push_back(way.nId);
		cluster.vtCoord.push_back(SPoint{ way.x, way.y });
		cluster.vtLayoverTimes.emplace_back(way.nLayoverTime);
		cluster.vtCargoVolume.emplace_back(way.nCargoVolume);

		cluster.vtTimes.emplace_back(vtDistMatrix[prevId][currId].nTotalTime);
		cluster.vtDistances.emplace_back(vtDistMatrix[prevId][currId].nTotalDist);

		cluster.dist += vtDistMatrix[prevId][currId].nTotalDist;
		cluster.time += vtDistMatrix[prevId][currId].nTotalTime;
		cluster.time += way.nLayoverTime; // 소요 시간 추가
		cluster.cargo += way.nCargoVolume; // 화물 추가

		prevId = currId;
	}

	return cluster.vtPois.size();
}


int32_t getDistrictResult(IN const ClusteringOption* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtWaypoints, IN const int32_t firstIdx, IN const int32_t lastIdx, OUT stDistrict& cluster)
{
	int totTime = 0;
	int totDist = 0;
	int prevId = -1;
	int currId = -1;
	int offset = 0;

	int nId = cluster.id;
	cluster.init();
	cluster.id = nId;

	for (int ii = 0; ii < vtWaypoints.size(); ii++) {
		offset = ii;
		currId = vtWaypoints[offset].nId;

		cluster.vtPois.emplace_back(vtWaypoints[offset].nId);
		cluster.vtCoord.emplace_back(SPoint{ vtWaypoints[offset].x, vtWaypoints[offset].y });

		// 시작
		if (ii == 0) {
			// 출발지 고정일 경우
			if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START ||
				pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END ||
				pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
				cluster.vtTimes.emplace_back(vtDistMatrix[firstIdx][currId].nTotalTime);
				cluster.vtDistances.emplace_back(vtDistMatrix[firstIdx][currId].nTotalDist);
				totTime += vtDistMatrix[firstIdx][currId].nTotalTime;
				totDist += vtDistMatrix[firstIdx][currId].nTotalDist;
			} else if ((vtWaypoints.size() == 1) && // 1개 결과일 때 
				(pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END ||
					pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END)) {
				cluster.vtTimes.emplace_back(vtDistMatrix[currId][lastIdx].nTotalTime);
				cluster.vtDistances.emplace_back(vtDistMatrix[currId][lastIdx].nTotalDist);
				totTime += vtDistMatrix[currId][lastIdx].nTotalTime;
				totDist += vtDistMatrix[currId][lastIdx].nTotalDist;
			}
		}
		// 종료
		else if (ii == vtWaypoints.size() - 1) {
			cluster.vtTimes.emplace_back(vtDistMatrix[prevId][currId].nTotalTime);
			cluster.vtDistances.emplace_back(vtDistMatrix[prevId][currId].nTotalDist);
			totTime += vtDistMatrix[prevId][currId].nTotalTime;
			totDist += vtDistMatrix[prevId][currId].nTotalDist;

			// 도착지 고정일 경우
			if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) {
				cluster.vtTimes.emplace_back(vtDistMatrix[currId][lastIdx].nTotalTime);
				cluster.vtDistances.emplace_back(vtDistMatrix[currId][lastIdx].nTotalDist);
				totTime += vtDistMatrix[currId][lastIdx].nTotalTime;
				totDist += vtDistMatrix[currId][lastIdx].nTotalDist;
			}
		}
		// 중간
		else {
			cluster.vtTimes.emplace_back(vtDistMatrix[prevId][currId].nTotalTime);
			cluster.vtDistances.emplace_back(vtDistMatrix[prevId][currId].nTotalDist);
			totTime += vtDistMatrix[prevId][currId].nTotalTime;
			totDist += vtDistMatrix[prevId][currId].nTotalDist;
		}

		prevId = vtWaypoints[offset].nId;
	} // for

	cluster.dist = totDist;
	cluster.time = totTime;

	return cluster.vtPois.size();
}


//#define USE_GN_KMEANS_ALGORITHMS // k-means 알고리즘 사용
int32_t CTMSManager::GetCluster(IN const ClusteringOption* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<Origins>& vtOrigin, OUT vector<stDistrict>& vtDistrict, OUT vector<SPoint>& vtendpointType)
{
	int32_t ret = RESULT_OK;

#if defined(USE_GN_KMEANS_ALGORITHMS)
	double** ppWeightMatrix = nullptr;

	// 실거리 테이블이 있으면 사용, 없으면 직선거리 적용
	if (ppTables != nullptr) {
		// wm 생성
		ppWeightMatrix = new double*[cntPois];
		for (int ii = 0; ii < cntPois; ii++) {
			ppWeightMatrix[ii] = new double[cntPois];

			// copy data
			for (int jj = 0; jj < cntPois; jj++) {
				ppWeightMatrix[ii][jj] = ppResultTables[ii][jj].nTotalDist;
			}
		} // for
	}

	// k-means
	CGnKmeansClassfy kmc;
	// k-means
	CGnKmeansClassfy kmc;

	// set wm
	kmc.SetWeightMatrix(ppWeightMatrix);

	SPoint centerAll = getPolygonCenter(vtPois);

	// 최초 클러스터 데이터
	kmc.SetDataCount(cntPois);
	int ii = 0;
	for (const auto& poi : vtPois) {
		kmc.AddData(ii++, poi.x, poi.y);
	}

#if 1 // k-means 기본 분배
	kmc.Run(remainedCluster);

	// 클러스터 그룹핑
	vtCluster.reserve(remainedCluster);

	char szGroup[128] = { 0, };
	for (int ii = 0; ii < remainedCluster; ii++) {
		stDistrict tmpDistrict;

		sprintf(szGroup, "%02d", kmc.m_vtResult[ii].group);

		tmpDistrict.name = szGroup;
		tmpDistrict.center.x = kmc.m_vtResult[ii].x;
		tmpDistrict.center.y = kmc.m_vtResult[ii].y;
		tmpDistrict.pois.reserve(kmc.m_vtResult[ii].idx);

		vtCluster.emplace_back(tmpDistrict);
	} // for

	stDistrict* pDistrict = nullptr;
	for (const auto& item : kmc.m_vtInput) {
		pDistrict = &vtCluster.at(item.group);

		if (pDistrict) {
			stPoi newPoi;
			newPoi.coord.x = item.x;
			newPoi.coord.y = item.y;

			pDistrict->pois.emplace_back(newPoi);
		} else {
			LOG_TRACE(LOG_WARNING, "new cluster index not exist, idx:%d", item.group);
		}
	} // for
#else // 밀도 균등을 위해 반복 처리
	// 밀도 균등을 위해 반복 처리
	// 클러스터 수만큼 반복하여, 평균치에 가까운 녀석을 순차적으로 빼면서 반복
	const int32_t cntItemAvg = cntPois / MAX_CLUSTER_COUNT;
	const int32_t cntItemBand = cntItemAvg * 5 / 100; // 위아래 5%까지 허용

	for (; 0 < remainedCluster - 2;) {
		kmc.Run(remainedCluster);

		// 클러스터 그룹핑
		vector<stDistrict> tmpDistricts;
		tmpDistricts.reserve(kmc.m_vtResult.size());

		for (int ii = 0; ii < remainedCluster; ii++) {
			stDistrict tmpDistrict;
			tmpDistricts.emplace_back(tmpDistrict);
		} // for

		for (const auto& item : kmc.m_vtInput) {
			stDistrict* pCluster = &tmpDistricts.at(item.group);

			if (pCluster) {
				stPoi newPoi;
				newPoi.coord.x = item.x;
				newPoi.coord.y = item.y;

				pCluster->pois.emplace_back(newPoi);
				pCluster->center.x = kmc.m_vtResult[item.group].x;
				pCluster->center.y = kmc.m_vtResult[item.group].y;
			} else {
				LOG_TRACE(LOG_WARNING, "new cluster index not exist, idx:%d", item.group);
			}
		} // for

		  // 밀도 균등 클러스터 선택
		int32_t diff = INT32_MAX;
		int32_t cntAddedCluster = 0; // 등록된 클러스터 갯수
		int32_t diffNearCluster = INT32_MAX; // 평균치와 가장 가까운 클러스터 아이템 갯수
		vector<stDistrict>::iterator candidateIt = tmpDistricts.end(); // 평균치와 가장 가까운 클러스터
		char szNum[10] = { 0, };
		for (auto it = tmpDistricts.begin(); it != tmpDistricts.end();) {
			// 밀도 균등 허용치에 포함되면 사용
			diff = it->pois.size() - cntItemAvg;
			if (abs(diff) <= cntItemBand) {
				sprintf(&it->name[0], "%02d", m_tmsInfoNew.size());
				vtCluster.emplace_back(*it);

				cntAddedCluster++;

				// 삭제
				it = tmpDistricts.erase(it);
				candidateIt = tmpDistricts.end();
			} else {
				if ((cntAddedCluster <= 0) && ((0 <= diff) && (diff < diffNearCluster))) {
					// 예비 클러스터
					candidateIt = it;

					diffNearCluster = diff;
				}

				// 삭제 안하고 증가
				it++;
			}
		} // for

		  // 밀도 균등 클러스터가 없으면 예비 클러스터에서 사용
		if ((cntAddedCluster <= 0) && (candidateIt != tmpDistricts.end())) {
			// 가까운 순으로 정렬
			multimap<double, int>mapDist;
			for (int ii = 0; ii < candidateIt->pois.size(); ii++) { // diffNearCluster
				mapDist.emplace(getRealWorldDistance(candidateIt->center.x, candidateIt->center.y, candidateIt->pois[ii].coord.x, candidateIt->pois[ii].coord.y), ii);
				//mapDist.emplace(10000000.f - getRealWorldDistance(centerAll.x, centerAll.y, candidateIt->pois[ii].coord.x, candidateIt->pois[ii].coord.y), ii);
			} // for

			  // 밀도 균등 평균치 까지만 저장
			stDistrict tmpAddDistrict; // 저장될 아이템들
			stDistrict tmpRemainDistrict; // 다시 재사용될 아이템들
			sprintf(&tmpAddDistrict.name[0], "%02d", m_tmsInfoNew.size());
			tmpAddDistrict.center = candidateIt->center;

			for (const auto& dist : mapDist) {
				// 밀도 균등 평균치 까지만 저장
				if (tmpAddDistrict.pois.size() < cntItemAvg) {
					tmpAddDistrict.pois.emplace_back(candidateIt->pois[dist.second]);
				} else {
					tmpRemainDistrict.pois.emplace_back(candidateIt->pois[dist.second]);
				}
			} // for

			  // 원본 삭제
			tmpDistricts.erase(candidateIt);

			// 클러스터 추가
			vtCluster.emplace_back(tmpAddDistrict);
			cntAddedCluster++;

			// 재사용 추가
			//tmpDistricts.emplace_back(tmpRemainDistrict);
		}

		// 남은 값을 다시 넣어 클러스터링 반복 진행
		kmc.ReSet();
		int ii = 0;
		for (auto& district : tmpDistricts) {
			for (auto& item : district.pois) {
				kmc.AddData(ii++, item.coord.x, item.coord.y);
			}
		} // for

		remainedCluster -= cntAddedCluster;
	} // for

	  // 마지막 남은 아이템들을 모아 클러스터링
	if (remainedCluster != 1) {
		LOG_TRACE(LOG_WARNING, "last cluster count should remained only one, now:%d", remainedCluster);
	} else //if (0) {
		kmc.Run(remainedCluster);

	// 클러스터 그룹핑
	stDistrict tmpDistrict;
	sprintf(&tmpDistrict.name[0], "%02d", m_tmsInfoNew.size());
	tmpDistrict.center.x = kmc.m_vtResult[0].x;
	tmpDistrict.center.y = kmc.m_vtResult[0].y;

	for (const auto& item : kmc.m_vtInput) {
		stPoi newPoi;
		newPoi.coord.x = item.x;
		newPoi.coord.y = item.y;
		tmpDistrict.pois.emplace_back(newPoi);
	} // for
	vtCluster.emplace_back(tmpDistrict);
#endif

	// release
	kmc.m_vtInput.clear();
	kmc.m_vtResult.clear();


	// 배송처 권역
	for (auto& cluster : vtCluster) {
		vector<SPoint> coords;
		vector<SPoint> border;
		vector<SPoint> expend;
		vector<SPoint> slice;
		for (const auto& poi : cluster.pois) {
			coords.emplace_back(poi.coord);
		}

		// make border
		GetBoundary(coords, cluster.border);
	}

	for (int ii = 0; ii < cntPois; ii++) {
		if (ppWeightMatrix) {
			SAFE_DELETE_ARR(ppWeightMatrix[ii]);
		}
	}
	SAFE_DELETE_ARR(ppWeightMatrix);

#else // #define USE_KMEANS_ALORITHM // k-means 알고리즘 사용
	vector<stWaypoints> vtWaypoints;
	stWaypoints waypoint;
	int idx = 0;
	for (const auto& pt : vtOrigin) {
		waypoint.nId = idx++;
		waypoint.x = pt.position.x;
		waypoint.y = pt.position.y;
		waypoint.nLayoverTime = pt.layoverTime; // 지점 소요 시간
		waypoint.nCargoVolume = pt.cargoVolume; // 지점 화물 수량

		vtWaypoints.emplace_back(waypoint);
	}

	// get bestway
	TspOption tspOpt;
	ClusteringOption clustOpt;
	vector<int32_t> vtBestway;
	int maxSize = vtOrigin.size();

	clustOpt = *pClustOpt;

	// poi가 2개 이하인 경우
	if (maxSize <= 2) {
		// 클러스터 제한 또는 클러스터당 POI 제한 오류
		ret = TMS_RESULT_FAILED_INPUT_OPTION;
	} else if (clustOpt.limitCluster > 0) { // 균등 분배
		if ((maxSize < clustOpt.limitCluster) || ((clustOpt.endpointType != 0) && (maxSize - 1 < clustOpt.limitCluster))) {
			// 전체 수량에 대해 지정 구역수와 구역별 지정 배정수가 한계치를 초과함.
			ret = TMS_RESULT_FAILED_LIMIT_COUNT_OVER;
		}
	} else if (clustOpt.limitCluster <= 0) { // 최적 분배
		if (clustOpt.limitValue <= 0) {
			// 최적 분배의 경우 제한값이 필요함
			ret = TMS_RESULT_FAILED_LIMIT_VALUE_NULL;
		} else if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_COUNT) {
			if (clustOpt.limitValue <= 1) { // 너무 작은 분배, 1이하는 의미 없는 값
				ret = TMS_RESULT_FAILED_LIMIT_VALUE_TOO_SMALL;
			}
		} else if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_DIST) {
			if (clustOpt.limitValue <= 1000) { // 너무 작은 분배, 1km 이하는 의미 없는 값
				ret = TMS_RESULT_FAILED_LIMIT_VALUE_TOO_SMALL;
			}
		} else if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_TIME) {
			if (clustOpt.limitValue <= 3600) { // 너무 작은 분배, 1시간 이하는 의미 없는 값
				ret = TMS_RESULT_FAILED_LIMIT_VALUE_TOO_SMALL;
			}
		}
	}

	if (ret != 0) {
		// 1차 오류 리턴
		return ret;
	}

	if (clustOpt.limitCluster <= 0) { // 최적 분배
		if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_DIST) {

		} else if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_TIME) {
			// 시간균등일 경우, 입력된 시간(분)을 초 단위로 변경
			//clustOpt.limitValue *= 60;
			//clustOpt.limitDeviation *= 60;
		} else {
			if ((clustOpt.limitValue == 0) && (clustOpt.limitCluster != 0)) {
				// 개수 균등일때, 개수 한계가 없으면 우선, 균등 분배 
				clustOpt.limitValue = maxSize / clustOpt.limitCluster;
			}
		}
	}



	// 클러스트링을 위해 최초 TSP(원점회귀) 수행
	tspOpt.algorithm = clustOpt.algorithm;
	tspOpt.seed = clustOpt.seed;
	tspOpt.compareType = clustOpt.compareType;
	tspOpt.endpointType = clustOpt.endpointType;
	tspOpt.geneSize = vtWaypoints.size();
	
	// 기본 TSP 알고리즘은 회귀모델이기에 출/도착지 고정을 구현 못함, TSP-GA로 변경 필요
	if ((tspOpt.endpointType == TYPE_TSP_ENDPOINT_START_END) && (tspOpt.algorithm == TYPE_TSP_ALGORITHM_TSP)) {
		tspOpt.algorithm = TYPE_TSP_ALGORITHM_TSP_GA;
	}

	stClustValue totalValue(clustOpt.divisionType);
	ret = GetBestway(&tspOpt, vtDistMatrix, vtWaypoints, vtBestway, totalValue.dwDist, totalValue.nTime);

	const int MAX_CNT_LOOP = 100;// 10000; // 무한 반복 제한
	int cntLoop = 0;
	int bonusValue = 0;
	int cntRematch = 0;
	bool isComplete = false;
	double deviation = clustOpt.limitDeviation;
	double offsetValue = clustOpt.limitValue * .1f; // 최대 한계 값에서 깎아내는 수치

	if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_DIST) {
		// 거리 균등
		// 최소 1km ~ 최대 10km
		if (offsetValue < 1000) {
			offsetValue = 1000;
		} else if (offsetValue > 10000) {
			offsetValue = 10000;
		}

		// 거리 균등에 클러스터 개수가 결정되어 있으면, 평균을 미리 계산
		if (clustOpt.limitCluster > 0 /*&& clustOpt.limitValue <= 0*/) {
			int nValue = totalValue.getCurrentValue();
			clustOpt.limitValue = (nValue / clustOpt.limitCluster) * 1.2;
		}
	} else if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_TIME) {
		// 시간 균등
		// 최소 5분 ~ 최대 10분
		static const int nDeviationValue = 3 * 60;
		if (offsetValue < nDeviationValue) {
			offsetValue = nDeviationValue;
		} else if (offsetValue > 10 * 60) {
			offsetValue = 10 * 60;
		}

		// 시간 균등에 클러스터 개수가 결정되어 있으면, 평균을 미리 계산
		if (clustOpt.limitCluster > 0 /*&& clustOpt.limitValue <= 0*/) {
			int nValue = totalValue.getCurrentValue();
			clustOpt.limitValue = (nValue / clustOpt.limitCluster) * 1.2;
		}
	} else {
		// 개수 균등
		offsetValue = 1;

		// 클러스터 개수가 결정되어 있으면, 평균 및 보너스를 미리 계산
		if (clustOpt.limitCluster > 0 /*&& clustOpt.limitValue <= 0*/) {
			int nValue = maxSize;
			if (clustOpt.endpointType == TYPE_TSP_ENDPOINT_START || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
				nValue -= 1;
			} else if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) {
				nValue -= 2;
			}
			clustOpt.limitValue = nValue / clustOpt.limitCluster;
			bonusValue = nValue % clustOpt.limitCluster;
		}
	}



	LOG_TRACE(LOG_DEBUG, "clustering limit value : %d, deviation: %d, deviation offset: %.2f", clustOpt.limitValue, clustOpt.limitDeviation, offsetValue);

	double currValueGap = 0;
	int maxLimitValueOrigin = pClustOpt->limitValue;
	int maxLimitValue = 0;
	int minLimitValue = 0;
	int maxLimitDistrict = INT_MAX; // 한계 배정 수량
	int prevValueGap = INT_MAX;
	int prevLimitValue = 0;
	if (clustOpt.limitCluster > 0) { // 클러스터 고정이면
		maxLimitDistrict = clustOpt.limitCluster;
		if (clustOpt.limitValue < 0) {
			//maxLimitValue = INT_MAX;
		}
	} else {
		//maxLimitValue = clustOpt.limitValue;
	}

	for (;;) {
		vtDistrict.clear();

		ret = Clustering(&clustOpt, vtDistMatrix, vtWaypoints, vtBestway, bonusValue, vtDistrict);
		bonusValue = 0;

		int cntCluster = vtDistrict.size();
		int minPois = INT_MAX;
		int maxPois = 0;
		double avgValue = 0.f;
		double remaindValaue = 0.f;
		double totValue = 0.f;
		double minValue = DBL_MAX;
		double maxValue = 0.f;

		for (const auto & district : vtDistrict) {
			double districtValue = 0.f;
			if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_DIST) {
				districtValue = district.dist;
			} else if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_TIME) {
				districtValue = district.time;
			} else {
				districtValue = district.vtPois.size();
			}

			if (minValue > districtValue) {
				minValue = districtValue;
			}
			if (maxValue < districtValue) {
				maxValue = districtValue;
			}

			// 클러스터의 최소 방문지
			if (minPois > district.vtPois.size()) {
				minPois = district.vtPois.size();
			}
			if (maxPois < district.vtPois.size()) {
				maxPois = district.vtPois.size();
			}

			totValue += districtValue;

			LOG_TRACE(LOG_DEBUG, "clustering result, idx:%d, cnt: %d, time: %d, dist: %.2f", district.id, district.vtPois.size(), district.time, district.dist);
		} //for

		avgValue = totValue / cntCluster;
		currValueGap = maxValue - minValue;

		LOG_TRACE(LOG_DEBUG, "clustering result, poi(min:%d, max:%d), value(min:%.2f, max:%.2f), avg:%.2f, diff:%.2f", minPois, maxPois, minValue, maxValue, avgValue, currValueGap);

		int tmpLimitValue = clustOpt.limitValue; // 변경전 

		if (cntCluster < maxLimitDistrict) { // 새로운 분배 결과가 이전보다 작으면
			if (clustOpt.limitCluster > 0) { // 균등 분배
				maxLimitValue = clustOpt.limitValue; // 최대값 낮춤
			} else { // 추천 분배
				maxLimitDistrict = cntCluster; // 최대 분배 축소
				//minLimitValue = clustOpt.limitValue; // 최소값 높임
				maxLimitValue = clustOpt.limitValue; // 최대값 낮춤
			}
			clustOpt.limitValue -= offsetValue;

			// 배정 계산 결과가 최대 지정 배정치를 넘겼으면, 현재 한계값을 최소로 지정
			if (!cntRematch && minLimitValue > 0 && maxLimitValue > 0) {
				cntRematch++;
				offsetValue *= .5; // 재 분배시 가중치를 한번 낮춰주자
			}
		} else if (cntCluster > maxLimitDistrict) { // 새로운 분배 결과가 이전보다 크면
			if (clustOpt.limitCluster > 0) { // 균등 분배
				minLimitValue = clustOpt.limitValue; // 최소값 높임
			} else { // 추천 분배
				minLimitValue = clustOpt.limitValue; // 최소값 높임
			}
			clustOpt.limitValue += offsetValue;

			// 배정 계산 결과가 최대 지정 배정치를 넘겼으면, 현재 한계값을 최소로 지정
			if (!cntRematch && minLimitValue > 0 && maxLimitValue > 0) {
				cntRematch++;
				offsetValue *= .5; // 재 분배시 가중치를 한번 낮춰주자
			}
		} else { // 분배 결과가 목표와 동일하면
			if (cntRematch) {
				minLimitValue = maxLimitValue = clustOpt.limitValue; // 반복하지 말고 여기서 종료하자
			} else {
				//if (currValueGap > minValue) { // 오차가 최소보다 크면
				//	maxLimitValue = clustOpt.limitValue; // 최대값 낮추고
				//	clustOpt.limitValue -= offsetValue; // 한계값을 줄이자
				//} else { // 오차가 최소보다 작으면
				//	minLimitValue = clustOpt.limitValue; // 최소값 높이고
				//	clustOpt.limitValue += offsetValue; // 한계값을 줄이자
				//}

				maxLimitValue = clustOpt.limitValue; // 최대값 낮추고
				clustOpt.limitValue -= offsetValue; // 한계값을 줄이자
			}
		}

		if ((clustOpt.limitCluster > 0) && (cntRematch) && (minValue == 0)) {
			// 이전의 매칭된 분배가 깨졌으면 실패
			ret = TMS_RESULT_FAILED_MATCHING_LIMIT; // 지정된 분배 수량내에서 배정이 실패함
			break;
		}


		// 편차 계산
		if (clustOpt.limitDeviation == 0) { // 자동 편차
			//if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_COUNT) {
			//	if (isRematch) {
			//		deviation = avgValue * 0.3; // 30%를 편차로 사용
			//	} else {
			//		deviation = 1;// 1를 편차로 사용
			//	}				
			//} else 
			if (clustOpt.limitCluster > 0) {
				deviation = avgValue * 0.1; // 10%를 편차로 사용
			} else {
				if (cntRematch) {
					deviation = avgValue * 0.3; // 30%를 편차로 사용
				} else {
					deviation = avgValue * 0.1; // 10%를 편차로 사용
				}
			}

			if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_DIST) {
				if (deviation < 10 * 1000) { // 거리의 경우, 10km를 최소 편차로 사용하자
					deviation = 10 * 1000;
				}
			} else if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_TIME) {
				if (deviation < 30 * 60) { // 시간의 경우, 30분을 최소 편차로 사용하자
					deviation = 30 * 60;
				}
			} else { // if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_COUNT)
				if (deviation < 1) { // 시간의 경우, 30분을 최소 편차로 사용하자
					deviation = 1;
				} else {
					deviation = static_cast<int>(deviation * 1.5);
				}
			}
		}

		if ( (((minPois != maxPois) && (minPois > 1 || maxPois > 1)) && (currValueGap > deviation)) || 
			 ((cntCluster == 1) && (maxValue > maxLimitValue)) ) { // 클러스터 1개면서 최대 값을 초과해도 다시 분배 필요, add 2025-06-02
			if (minLimitValue > 0 && maxLimitValue > 0) { // 최대 클러스터는 정해졌고, 한계값 조정 필요
				//if ((minLimitValue >= maxLimitValue) && (maxValue <= maxLimitValue)) {
				if ((minLimitValue <= minValue) && (maxValue <= maxLimitValue)) {
					// 최소 한계치가 최대 한계치를 역전하거나, 최소 가중치보다 작은 차이가 생기면 종료
					isComplete = true;
					ret = RESULT_OK;
					break;
				}
			}

			// 보너스 계산, 다음 클러스터링에서 마지막 클러스터의 값이 너무 작을 경우, 이전 클러스터가 나눠 가지도록 하자
			if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_COUNT) {
				// 우선은 개수 균등에 적용
				int remainedValue = 0;

				if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
					if (clustOpt.limitValue - ((maxSize - 1) % clustOpt.limitValue) > deviation) {
						remainedValue = (maxSize - 1) % clustOpt.limitValue;
						// 지점 고정은 클러스터링 개수에서 제외
					}
				} else if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) {
					if (clustOpt.limitValue - ((maxSize - 2) % clustOpt.limitValue) > deviation) {
						// 지점 고정은 클러스터링 개수에서 제외
						remainedValue = (maxSize - 2) % clustOpt.limitValue;
					}
				} else {
					remainedValue = (maxSize) % clustOpt.limitValue;
				}

				// 이후 분배에서 편차보다 많이 남은 값은 보너스로 처리한다. 
				if (clustOpt.limitValue - remainedValue > deviation) {
					bonusValue = remainedValue;
				}
			} else {
				// 마지막 최소 값이 편차보다 작고, 최대+최소값이 최초의 한계값 보다 작으면 보너스로 분배
				if (minValue <= deviation) {
					bonusValue = minValue;
				}
			}
		} else if ((maxValue <= maxLimitValueOrigin/*maxLimitValue*/) && ((clustOpt.limitCluster <= 0) || (clustOpt.limitCluster > 0 && cntCluster == clustOpt.limitCluster) || (currValueGap <= deviation))) {
			isComplete = true;
		}


		if (isComplete) {
			if ((((clustOpt.divisionType != TYPE_CLUSTER_DEVIDE_BY_COUNT) || (pClustOpt->limitValue != 1)) && (minPois <= 1)) && (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_NONE)) {
				ret = TMS_RESULT_FAILED_MATCHING_DEVIATION;
			} else {
				ret = RESULT_OK;
			}
			break;
		} else if (clustOpt.limitValue <= 0) {
			ret = TMS_RESULT_FAILED_MATCHING_DEVIATION;
			break;
		} else {
			if (prevValueGap > currValueGap) { // 이전보다 오차가 작아짐, 갱신
				prevValueGap = currValueGap;
				prevLimitValue = tmpLimitValue;
			} else if (prevValueGap < currValueGap) { // 이전보다 오차가 커짐
				clustOpt.limitValue = prevLimitValue;
				//cntRematch++;
			} else { // 이전과 오차가 같음
				if (cntRematch >= 5) {
					ret = RESULT_OK; // 우선은 성공으로 보자
					break;
				}
				//cntRematch++;
			}
			cntRematch++;
		}

		// loop check
		if (++cntLoop > MAX_CNT_LOOP) {
			ret = TMS_RESULT_FAILED_LOOP;
			break;
		} else if (clustOpt.limitValue <= 0) {
			ret = TMS_RESULT_FAILED_MATCHING_DEVIATION;
			break;
		} else if ((clustOpt.max_distance || clustOpt.max_time || clustOpt.max_spot) && (cntLoop > 3)) {
			// 복합 제한 요청인 경우, N번 이상 재시도 하지 말자
			ret = RESULT_OK;
			break;
		}

		LOG_TRACE(LOG_DEBUG, "clustering rematching limit value : %d", clustOpt.limitValue);
	} // for


	  // TSP
	if (ret == RESULT_OK) {
		// 클러스트링된 그룹의 TSP 수행
		tspOpt = clustOpt.tspOption;
		tspOpt.endpointType = clustOpt.endpointType;
		tspOpt.geneSize = vtWaypoints.size();

		for (auto& cluster : vtDistrict) {
			double totDist = 0.f;
			int32_t totTime = 0;

			vector<stWaypoints> vtClustWaypoints;
			for (int ii = 0; ii < cluster.vtPois.size(); ii++) {
				stWaypoints waypoint;
				waypoint.nId = cluster.vtPois[ii];
				waypoint.x = cluster.vtCoord[ii].x;
				waypoint.y = cluster.vtCoord[ii].y;
				waypoint.nLayoverTime = cluster.vtLayoverTimes[ii];
				waypoint.nCargoVolume = cluster.vtCargoVolume[ii];
				vtClustWaypoints.emplace_back(waypoint);
			} // for

#if 0
			if (tspOpt.endpointType == TYPE_TSP_ENDPOINT_START || tspOpt.endpointType == TYPE_TSP_ENDPOINT_START_END || tspOpt.endpointType == TYPE_TSP_RECURSICVE) {
				waypoint.nId = 0;
				waypoint.x = vtOrigin[0].position.x;
				waypoint.y = vtOrigin[0].position.y;
				waypoint.nLayoverTime = vtOrigin[waypoint.nId].time;
				vtClustWaypoints.insert(vtClustWaypoints.begin(), waypoint);
			}
			if (tspOpt.endpointType == TYPE_TSP_ENDPOINT_END || tspOpt.endpointType == TYPE_TSP_ENDPOINT_START_END) {
				waypoint.nId = maxSize - 1;
				waypoint.x = vtOrigin[waypoint.nId].position.x;
				waypoint.y = vtOrigin[waypoint.nId].position.y;
				waypoint.nLayoverTime = vtOrigin[waypoint.nId].time;
				vtClustWaypoints.emplace_back(waypoint);
			}
#endif 

			tspOpt.geneSize = vtClustWaypoints.size();

			if (cluster.vtPois.size() <= 2) {
				continue;
			}

			// 바뀐 bestway 재설정
			vector<stWaypoints> vtNewWaypoints;
#if 0
			// 클러스터당 POI WM을 새로 구성한다. 
			vector<vector<stDistMatrix>> vtNewDistMatrix;
			for (const auto& poiCols : vtClustWaypoints) {
				vector<stDistMatrix> vtRowsDM;
				for (const auto& poiRows : vtClustWaypoints) {
					vtRowsDM.emplace_back(vtDistMatrix[poiCols.nId][poiRows.nId]);
				}
				vtNewDistMatrix.emplace_back(vtRowsDM);
			}
			ret = GetBestway(&tspOpt, vtNewDistMatrix, vtClustWaypoints, vtBestway, totDist, totTime);

			for (const auto way : vtBestway) {
				vtNewWaypoints.push_back(vtClustWaypoints[way]);
			}
#else
			if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_NONE) {
#	if 0
				vtNewWaypoints.assign(vtClustWaypoints.begin(), vtClustWaypoints.end());
				ret = RESULT_OK;
#	else
				ret = GetNewBestway(&tspOpt, vtDistMatrix, vtClustWaypoints, vtNewWaypoints, vtBestway, totalValue);
#	endif
			} else {
				ret = GetNewBestway(&tspOpt, vtDistMatrix, vtClustWaypoints, vtNewWaypoints, vtBestway, totalValue);
			}
#endif

			if ((ret == RESULT_OK) &&
				((totDist < cluster.dist) || (totTime < cluster.time))) {

				LOG_TRACE(LOG_DEBUG, "cluster result change, id:%d old_dist:%.0f, old_time:%d -> new_dist:%.0f, new_time:%d", cluster.id, cluster.dist, cluster.time, totDist, totTime);

#if 0
				int nFirst = vtWaypoints.front().nId;
				int nLast = vtWaypoints.back().nId;

				// 클러스터를 새로 구성 & 임시로 추가된 출도착지점 제거
				vector<stWaypoints> vtNewClustWaypoints;
				for (const auto& way : vtBestway) {
					// 출발지 고정일 경우
					if ((nFirst == vtClustWaypoints[way].nId) && (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END || pClustOpt->endpointType == TYPE_TSP_RECURSICVE)) {
						continue;
					}
					// 도착지 고정일 경우
					if ((nLast == vtClustWaypoints[way].nId) && (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END)) {
						continue;
					}

					vtNewClustWaypoints.push_back(vtClustWaypoints[way]);
				}

				getDistrictResult(pClustOpt, vtDistMatrix, vtNewClustWaypoints, nFirst, nLast, cluster);
#else
#	if 0
				// 바뀐 bestway 재설정
				vector<stWaypoints> vtNewWaypoints;
				for (const auto way : vtBestway) {
					vtNewWaypoints.push_back(vtClustWaypoints[way]);
				}
#	endif
				getNewDistrictResult(pClustOpt, vtDistMatrix, vtNewWaypoints, cluster);
#endif
			}
		} // for

		  // 예상 시간
		if (clustOpt.reservation > 0 && clustOpt.reservationType != 0) {
			for (auto& cluster : vtDistrict) {
				if (clustOpt.reservationType == 1) // 출발 시각
				{
					cluster.etd = clustOpt.reservation;
					cluster.eta = clustOpt.reservation + cluster.time;
				} else if (clustOpt.reservationType == 2) // 도착 시각
				{
					cluster.eta = clustOpt.reservation;
					cluster.etd = clustOpt.reservation - cluster.time;
				}
			}
		}


		//for (auto& cluster : vtDistrict) {
		//	// make border
		//	GetBoundary(cluster.vtCoord, cluster.vtBorder, cluster.center);
		//}


		// 배송처 권역
		for (auto& cluster : vtDistrict) {
#if 0
			GetBoundary(cluster.vtCoord, cluster.vtBorder, cluster.center);
#else
			// make border
			vector<SPoint> vtBorder;
			vtBorder.assign(cluster.vtCoord.begin(), cluster.vtCoord.end());

			// 출발지 고정 제외
			if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START || 
				pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END || 
				pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
				vtBorder.erase(vtBorder.begin());
			}

			// 도착지 고정 제외
			if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END || 
				pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) {
				vtBorder.erase(vtBorder.end() - 1);
			}

			GetBoundary(vtBorder, cluster.vtBorder, cluster.center);
#endif
		}

		// 출/도착지 고정
		if (pClustOpt->endpointType != TYPE_TSP_ENDPOINT_NONE) {
			if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
				SPoint coord = { vtOrigin[0].position.x, vtOrigin[0].position.y };
				vtendpointType.emplace_back(coord);
			} else {
				vtendpointType.emplace_back(SPoint{ 0, 0 });
			}

			if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) {
				SPoint coord = { vtOrigin[waypoint.nId].position.x, vtOrigin[waypoint.nId].position.y };
				vtendpointType.emplace_back(coord);
			} else {
				vtendpointType.emplace_back(SPoint{ 0, 0 });
			}
		}
	}
#endif // #define USE_KMEANS_ALORITHM // k-means 알고리즘 사용

	return ret;
}


int32_t CTMSManager::GetGroup(IN const ClusteringOption* pClustOpt, IN const vector<Origins>& vtOrigin, OUT vector<stDistrict>& vtDistrict)
{
	int32_t ret = DevideClusterUsingLink(pClustOpt, vtOrigin, vtDistrict);

	if (ret == RESULT_OK) {
		//for (auto& cluster : vtDistrict) {
		//	// make border
		//	GetBoundary(cluster.vtCoord, cluster.vtBorder, cluster.center);
		//}
		
		int prepareTime = 60 * 1; // 정차 후, 기본 배송 준비 시간(60초)
		int layoverTime = 60 * 1; // 건당 배송 소요시간(60초)
		for (auto& cluster : vtDistrict) {
			// 배송지 소요 시간			
			for (int ii = 0; ii < cluster.vtPois.size(); ii++) {
				if (ii == 0) {
					// 구역의 최초 배송지는 준비 시간 적용
					// 기본 배송 준비 시간 + 배송 건당 준비 시간(10초 * n)
					cluster.vtTimes.emplace_back(layoverTime + prepareTime + (10 * cluster.vtPois.size()));
				} else {
					cluster.vtTimes.emplace_back(layoverTime);
				}
			}

			// 배송처 권역
			// make border
			vector<SPoint> vtBorder;
			vtBorder.assign(cluster.vtCoord.begin(), cluster.vtCoord.end());

			// 출발지 고정 제외
			if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START ||
				pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END ||
				pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
				vtBorder.erase(vtBorder.begin());
			}

			// 도착지 고정 제외
			if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END ||
				pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) {
				vtBorder.erase(vtBorder.end() - 1);
			}

			SPoint tmpCenter; // 라인 중심 정보는 바꾸지 않는다.
			GetBoundary(vtBorder, cluster.vtBorder, tmpCenter);
		}
	}

	return ret;
}


int32_t CTMSManager::GetRecommendedDeviation(IN ClusteringOption* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN vector<stWaypoints>& vtWaypoints, IN vector<int32_t>& vtBestway, OUT vector<stDistrict>& vtDistrict)
{
	int32_t ret = RESULT_OK;

	LOG_TRACE(LOG_DEBUG, "calculate auto deviation value");

	// get bestway
	//TspOptions tspOpt;
	//vector<uint32_t> vtBestway;
	int maxSize = vtWaypoints.size();

	ClusteringOption clustOpt(*pClustOpt);

	const int MAX_CNT_LOOP = 10000; // 무한 반복 제한
	int cntLoop = 0;
	int bonusValue = 0;
	bool isRematch = false;
	bool isComplete = false;
	double deviationValue = 0; // clustOpt.limitDeviation;
	double offsetValue = clustOpt.limitDeviation * .1f;

	for (;;) {
		vtDistrict.clear();

		ret = Clustering(&clustOpt, vtDistMatrix, vtWaypoints, vtBestway, bonusValue, vtDistrict);
		bonusValue = 0;

		int cntCluster = vtDistrict.size();
		int minCluster = INT_MAX;
		double avgValue = 0.f;
		double remaindValaue = 0.f;
		double totValue = 0.f;
		double minValue = DBL_MAX;
		double maxValue = 0.f;

		for (const auto & district : vtDistrict) {
			double districtValue = 0.f;
			if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_DIST) {
				districtValue = district.dist;
			} else if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_TIME) {
				districtValue = district.time;
			} else {
				districtValue = district.vtPois.size();
			}

			if (minValue > districtValue) {
				minValue = districtValue;
			}
			if (maxValue < districtValue) {
				maxValue = districtValue;
			}

			// 클러스터의 최소 방문지
			if (minCluster > district.vtPois.size()) {
				minCluster = district.vtPois.size();
			}

			totValue += districtValue;

			LOG_TRACE(LOG_DEBUG, "clustering result, idx:%d, cnt: %d, time: %d, dist: %.2f", district.id, district.vtPois.size(), district.time, district.dist);
		} //for

		avgValue = totValue / cntCluster;

		LOG_TRACE(LOG_DEBUG, "clustering result, min:%.2f, max:%.2f, avg: %.2f, diff: %.2f", minValue, maxValue, avgValue, maxValue - minValue);



		if (clustOpt.limitCluster > 0) {
			// 요청 분배수량보다 자동 분배수량이 작으면
			if (cntCluster < clustOpt.limitCluster) {
				//cntCluster = clustOpt.cntCluster;
				//minValue = 0;
			} else if (cntCluster > clustOpt.limitCluster) {
				// 요청 분배수량보다 자동 분배수량이 크면
				//cntCluster = clustOpt.cntCluster;
				//minValue = 0;
			}

			if ((isRematch == true) && (minValue == 0 || clustOpt.limitCluster > 0)) {
				// 이전의 매칭된 분배가 깨졌으면 실패
				ret = TMS_RESULT_FAILED_MATCHING_LIMIT; // 지정된 분배 수량내에서 배정이 실패함
				break;
			}
		}


		// 편차 계산
		if (clustOpt.limitDeviation == 0) {
			if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_COUNT) {
				// 1를 편차로 사용
				deviationValue = 1;
			} else {
				// 10%를 편차로 사용
				//deviationValue = avgValue * 0.1;
				deviationValue = clustOpt.limitValue / 3;
			}
		}

		if (maxValue - minValue > deviationValue) {
			// 편자 조정
			if (clustOpt.limitCluster == 0) { // 자동 분배일 경우
				clustOpt.limitValue -= offsetValue;
			} else if (cntCluster > clustOpt.limitCluster) {
				clustOpt.limitValue += offsetValue;
			} else if (cntCluster < clustOpt.limitCluster) {
				clustOpt.limitValue -= offsetValue;
			} else {
				if (avgValue > clustOpt.limitValue) {
					// 범위를 0.1% 증가
					clustOpt.limitValue *= 1.001;
				} else {
					// 범위를 0.1% 감소
					clustOpt.limitValue *= 0.999;
				}

				isRematch = true;
			}

			// 보너스 계산, 다음 클러스터링에서 마지막 클러스터의 값이 너무 작을 경우, 이전 클러스터가 나눠 가지도록 하자
			if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_COUNT) {
				// 우선은 개수 균등에 적용
				int remainedValue = 0;

				if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
					if (clustOpt.limitValue - ((maxSize - 1) % clustOpt.limitValue) > deviationValue) {
						remainedValue = (maxSize - 1) % clustOpt.limitValue;
						// 지점 고정은 클러스터링 개수에서 제외
					}
				} else if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) {
					if (clustOpt.limitValue - ((maxSize - 2) % clustOpt.limitValue) > deviationValue) {
						// 지점 고정은 클러스터링 개수에서 제외
						remainedValue = (maxSize - 2) % clustOpt.limitValue;
					}
				} else {
					remainedValue = (maxSize) % clustOpt.limitValue;
				}

				// 이후 분배에서 편차보다 많이 남은 값은 보너스로 처리한다. 
				if (clustOpt.limitValue - remainedValue > deviationValue) {
					bonusValue = remainedValue;
				}
			} else {
				// 마지막 최소 값이 편차보다 작고, 최대+최소값이 최초의 한계값 보다 작으면 보너스로 분배
				if (minValue <= deviationValue) {
					bonusValue = minValue;
				}
			}
		} else {
			isComplete = true;
		}


		if (isComplete) {
			if (minCluster <= 1) {
				ret = TMS_RESULT_FAILED_MATCHING_DEVIATION;
			} else {
				ret = RESULT_OK;
			}
			break;
		} else if (clustOpt.limitValue <= 0) {
			ret = TMS_RESULT_FAILED_MATCHING_DEVIATION;
			break;
		}

		cntLoop++;

		if (cntLoop >= MAX_CNT_LOOP) {
			ret = TMS_RESULT_FAILED_LOOP;
			break;
		} else if (clustOpt.limitValue <= 0) {
			ret = TMS_RESULT_FAILED_MATCHING_DEVIATION;
			break;
		}

		LOG_TRACE(LOG_DEBUG, "calulated deviation value : %d", deviationValue);
	} // for

	return ret;
}


int32_t CTMSManager::GetNewBestway(IN const TspOption* pTspOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtWaypoints, OUT vector<stWaypoints>& vtNewWaypoints, OUT vector<int32_t>& vtNewBestways, OUT stClustValue& clustValue)
{
	int32_t ret = RESULT_OK;

	clustValue.init();

#	if 0 //defined(_DEBUG) // 디버깅 속도를 위해 TSP 안함
	vtNewWaypoints.clear();

	int32_t idxPrev = vtWaypoints[0].nId;
	int32_t idxNext = 0;

	for (const auto& way : vtWaypoints) {
		vtNewWaypoints.push_back(way);

		idxNext = way.nId;

		clustValue.dwDist += vtDistMatrix[idxPrev][idxNext].nTotalDist;
		clustValue.nTime += vtDistMatrix[idxPrev][idxNext].nTotalTime + way.nLayoverTime; // 출발~종료 모든 지점 소요 시간 추가
		clustValue.nCargo += way.nCargoVolume; // 출발~종료 모든 지점 화물 추가

		idxPrev = idxNext;
	}
#	else
	// 클러스터당 POI WM을 새로 구성한다. 
	vector<vector<stDistMatrix>> vtNewDistMatrix;
	for (const auto& poiCols : vtWaypoints) {
		vector<stDistMatrix> vtRowsDM;
		for (const auto& poiRows : vtWaypoints) {
			vtRowsDM.emplace_back(vtDistMatrix[poiCols.nId][poiRows.nId]);
		}
		vtNewDistMatrix.emplace_back(vtRowsDM);
	}

	TspOption tspOpt = *pTspOpt;
	tspOpt.geneSize = vtWaypoints.size(); // 변경된 사이즈 반영
	ret = GetBestway(&tspOpt, vtNewDistMatrix, vtWaypoints, vtNewBestways, clustValue.dwDist, clustValue.nTime);
	clustValue.nSpot = vtNewBestways.size();

	// bestway 결과로 순서가 변경되어 재 설정
	vtNewWaypoints.clear();
	for (const auto way : vtNewBestways) {
		vtNewWaypoints.push_back(vtWaypoints[way]);
	}
#	endif

	return ret;
}


#define USE_TEST_TSP_RESULT

int32_t CTMSManager::GetBestway(IN const TspOption* pTspOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtOrigin, OUT vector<int32_t>& vtBestWaypoints, OUT double& dist, OUT int32_t& time)
{
	int32_t ret = RESULT_FAILED;

#if !defined(USE_REAL_ROUTE_TSP)
	//vector<stCity> vtRawData;
#else
	//vector<stWaypoints> vtOrigin;
#endif

	LOG_TRACE(LOG_DEBUG, "start, best waypoint result, cnt:%d", vtOrigin.size());

	vector<int32_t> vtWayResult;

#if defined(USE_TEST_TSP_RESULT) // use test data
	//		ret = m_pRouteMgr.GetWeightMatrix(strRequest.c_str(), RDM, TSP.option.baseOption);
	//		//"rdm":{
	//		//	"type":"file",
	//		//		"form" : "bin",
	//		//		"size" : 3446808,
	//		//		"data" : "result_rdm_u24090948_t20250626174127.rdm"
	//		//},
	//		//"rln":{
	//		//	"type":"file",
	//		//		"form" : "bin",
	//		//		"size" : 585951156,
	//		//		"data" : "result_rdm_u24090948_t20250626174127.rln"
	//		//}

	vector<int32_t>vtlkh3_hanjin47 = { 
		1,5,10,9,8,11,6,2,3,4,
		7,36,12,21,20,14,39,41,42,22,
		40,16,15,18,17,37,23,28,31,47,
		45,46,43,44,24,25,13,26,30,19,
		29,27,38,32,35,34,33, };

	if (vtOrigin.size() == vtlkh3_hanjin47.size()) {
		LOG_TRACE(LOG_DEBUG, "using test tsp result, count:%d", vtlkh3_hanjin47.size());
		for (auto& item : vtlkh3_hanjin47) {
			item -= 1;
		}

		vtWayResult.assign(vtlkh3_hanjin47.begin(), vtlkh3_hanjin47.end());
	} else
#endif
	{
		vtWayResult.reserve(vtOrigin.size());

		if (vtOrigin.size() <= 2) {
			vtWayResult.push_back(0);
			vtWayResult.push_back(1);
		} else if (((pTspOpt->endpointType == TYPE_TSP_ENDPOINT_START) || (pTspOpt->endpointType == TYPE_TSP_ENDPOINT_START)) && vtOrigin.size() == 2) {
			vtWayResult.push_back(0);
			vtWayResult.push_back(1);
		} else if (((pTspOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) || (pTspOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE)) && vtOrigin.size() == 3) {
			vtWayResult.push_back(0);
			vtWayResult.push_back(1);
			vtWayResult.push_back(2);
		} else if (pTspOpt->algorithm == TYPE_TSP_ALGORITHM_COPY) {
			for (const auto& coord : vtOrigin) {
				vtWayResult.emplace_back(coord.nId);
			}
		} else if (pTspOpt->algorithm == TYPE_TSP_ALGORITHM_MANHATTHAN) {
			//auto result = mst_manhattan_branch_and_bound2(ppResultTables, vtOrigin->size(), 0);
			auto result = mst_manhattan_branch_and_bound(vtDistMatrix, vtOrigin.size(), 0);
			vtWayResult.assign(result.path.begin(), result.path.end());
		}
	#if 1 //!defined(USE_REAL_ROUTE_TSP)
		else if (pTspOpt->algorithm == TYPE_TSP_ALGORITHM_TSP) {
			InitURandom();
			TEnvironment env;

			env.Npop = vtOrigin.size();
			env.Nch = vtOrigin.size() / 3.2;

			LOG_TRACE(LOG_DEBUG, "init, best waypoint result");
			env.define(vtOrigin, vtDistMatrix, pTspOpt->compareType);

			LOG_TRACE(LOG_DEBUG, "building, best waypoint result");
			env.doIt();

			LOG_TRACE(LOG_DEBUG, "end, best waypoint result");
			env.printOn(0);

			env.getBest(vtWayResult);


			// 회귀모델이므로 출/도착지 순서대로 재정비 필요
			if (pTspOpt->endpointType == TYPE_TSP_ENDPOINT_END /*|| pTspOpt->endpointType == TYPE_TSP_ENDPOINT_START_END*/) { // 목적지면 반대로 뒤집어 TSP 수행 후 다시 뒤집자
	#if 1
				int nLastId = vtWayResult.size() - 1;
				int nLastIdx = 0;
				for (const auto id : vtWayResult) {
					if (nLastId == id) {
						break;
					}
					nLastIdx++;
				}


				vector<int> vtWayNewResult;
				for (int ii = 0; ii < vtWayResult.size(); ii++) {
					vtWayNewResult.push_back(vtWayResult[nLastIdx]);
					if (++nLastIdx >= vtWayResult.size()) {
						nLastIdx = 0;
					}
				}			
	#else
				int nStartId = 0;
				int nOffset = 0;
				for (int ii = 0; ii < vtWayResult.size(); ii++) {
					if (vtWayResult[ii] == nStartId) {
						nOffset = ii;
						break;
					}
				}

				if (nOffset > 0) {
					vector<int> vtWayNewResult;
					vtWayNewResult.reserve(vtWayResult.size());
					for (int ii = 0, off = 0; ii < vtWayResult.size(); ii++) {
						vtWayNewResult.emplace_back(vtWayResult[nOffset + off++]);
						if (nOffset + off > vtWayResult.size() - 1) {
							nOffset = off = 0;
						}
					} // for
				}
	#endif
				vtWayResult.assign(vtWayNewResult.rbegin(), vtWayNewResult.rend());
			}
		}
	#endif // #if !defined(USE_REAL_ROUTE_TSP)
		else {
			Environment newWorld;

			// 최대 세대 
			const int32_t maxGeneration = pTspOpt->loopCount;
			// 세대당 최대 개체수
			const int32_t maxPopulation = pTspOpt->geneSize <= 4 ? 24 : pTspOpt->individualSize; // 5개 이상부터 100번 반복 필요
			// 염색체당 최대 유전자수
			const int32_t maxGene = pTspOpt->geneSize;

			newWorld.SetOption(pTspOpt);

	#if 0//defined(USE_REAL_ROUTE_TSP)
			// 실거리 테이블의 경우, 정방향 or 역방향 결과로 통일하자 --> 2023-05-11 [ii][jj] != [jj][ii]일 경우, 순위 경쟁에서 이상한 결과를 내보낼 경우 있음, 아직 원인 못찾음
			for (int ii = 0; ii < maxGene; ii++) {
				for (int jj = 0; jj < maxGene; jj++) {
					if ((ii == jj) || (ii < jj)) break;

					const_cast<RouteTable**>(ppResultTables)[jj][ii].nTotalDist = ppResultTables[ii][jj].nTotalDist;
					/*const_cast<RouteTable**>(ppResultTables)[jj][ii].nTotalTime = ppResultTables[ii][jj].nTotalTime;*/
				} // for jj
			} // for ii
	#endif

			int cntUsableGene = maxGene;
			newWorld.SetCostTable(vtDistMatrix, cntUsableGene);

			// 신세계
			newWorld.Genesis(maxGene, maxPopulation);

#if defined(DEMO_FOR_HANJIN)
			const uint32_t MAX_SAME_RESULT = min(500, maxGene * 10);
#else
			const uint32_t MAX_SAME_RESULT = min(100, maxGene * 2); // maxGene * 10; // 500;// maxGeneration; // 같은 값으로 100회 이상 진행되면 최적값에 수렴하는것으로 판단하고 종료
#endif
			uint32_t repeatGeneration = 0;
			double topCost = DBL_MAX;
			double bestCost = DBL_MAX;

			// 반복횟수
			for (int ii = 0; ii < maxGeneration; ii++) {
				// 평가
				topCost = newWorld.Evaluation();
				//newWorld.Print();

				if (topCost < bestCost) {
					bestCost = topCost;
					repeatGeneration = 0;
				} else if (topCost == bestCost) {
					repeatGeneration++;
					if (MAX_SAME_RESULT < repeatGeneration) {
						double bestValue = 0.f;
						if (pTspOpt->compareType == TYPE_TSP_VALUE_DIST) {
							bestValue = newWorld.GetBestDist();
						} else if (pTspOpt->compareType == TYPE_TSP_VALUE_TIME) {
							bestValue = newWorld.GetBestTime();
						} else {
							bestValue = newWorld.GetBestCost();
						}

						LOG_TRACE(LOG_DEBUG, "best type(%d) result fitness value:%.3f, repeat:%d", pTspOpt->compareType, bestValue, repeatGeneration);
						break;
					}
				}

				// 부모 선택
				vector<Parents> pairs;
				newWorld.Selection(pairs);
				//newWorld.Print();

				// 자식 생성
				newWorld.Crossover(pairs);
				//newWorld.Print();
			} // for

			newWorld.GetBest(vtWayResult);
		}
	}

	if (vtOrigin.size() != vtWayResult.size()) {
		LOG_TRACE(LOG_DEBUG, "result count not match with orignal count, ori:%d != ret:%d", vtOrigin.size(), vtWayResult.size());
		return ret;
	}

	LOG_TRACE(LOG_DEBUG_LINE, "index  :", vtWayResult.size());
	for (int idx = 0; idx < vtWayResult.size(); idx++) {
		LOG_TRACE(LOG_DEBUG_CONTINUE, "%3d| ", idx);
	}
	LOG_TRACE(LOG_DEBUG_CONTINUE, " => \n", vtWayResult.size());

	double totalDist = 0;
	int32_t totalTime = 0;
	uint32_t idxFirst = 0;
	uint32_t idxPrev = vtWayResult[0];
	int32_t idx = 0;

	LOG_TRACE(LOG_DEBUG_LINE, "result :");
	for (const auto& item : vtWayResult) {
		LOG_TRACE(LOG_DEBUG_CONTINUE, "%3d>", item); //→
		if (idx >= 1) {
			totalDist += vtDistMatrix[idxPrev][item].nTotalDist;
			totalTime += vtDistMatrix[idxPrev][item].nTotalTime;
		} else {
			idxFirst = idx;
		}
		totalTime += vtOrigin[item].nLayoverTime; // 출발~종료 모든 지점 소요 시간 추가

		idxPrev = item;
		idx++;
	}

	// 원점 회귀 확인
	if (pTspOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
		totalDist += vtDistMatrix[idxPrev][idxFirst].nTotalDist;
		totalTime += vtDistMatrix[idxPrev][idxFirst].nTotalTime;
		totalTime += vtOrigin[idxPrev].nLayoverTime; // 지점 소요 시간 추가
		LOG_TRACE(LOG_DEBUG_CONTINUE, " %d => %d\n", idxFirst, totalDist);
	} else {
		LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");
	}

	vtBestWaypoints.clear();
	vtBestWaypoints.reserve(vtWayResult.size());

	int newIdx = 0;

	// 경유지 변경
	for (int curIdx = 0; curIdx <= vtWayResult.size() - 1; curIdx++) {
		newIdx = vtWayResult[curIdx];

		//if (curIdx == 0) {
		//	// 첫번째값을 출발지로 변경
		//	LOG_TRACE(LOG_DEBUG, "departure will change, idx(%d) %.5f, %.5f -> idx(%d) %.5f, %.5f",
		//		curIdx, vtOrigin->at(curIdx).x, vtOrigin->at(curIdx).y,
		//		newIdx, vtOrigin->at(newIdx).x, vtOrigin->at(newIdx).y);
		//	//SetDeparture(vtRawData[newIdx].x, vtRawData[newIdx].y);
		//} else if (curIdx == vtWayResult.size() - 2) {
		//	// 마지막값을 목적지로 변경
		//	LOG_TRACE(LOG_DEBUG, "destination will change, idx(%d) %.5f, %.5f -> idx(%d) %.5f, %.5f",
		//		curIdx, vtOrigin->at(curIdx).x, vtOrigin->at(curIdx).y,
		//		newIdx, vtOrigin->at(newIdx).x, vtOrigin->at(newIdx).y);
		//	//SetWaypoint(vtRawData[newIdx].x, vtRawData[newIdx].y);
		//} else {
		//	LOG_TRACE(LOG_DEBUG, "waypoint will change, idx(%d) %.5f, %.5f -> idx(%d) %.5f, %.5f",
		//		curIdx, vtOrigin->at(curIdx).x, vtOrigin->at(curIdx).y,
		//		newIdx, vtOrigin->at(newIdx).x, vtOrigin->at(newIdx).y);
		//	//SetDestination(vtRawData[newIdx].x, vtRawData[newIdx].y);
		//}

		vtBestWaypoints.emplace_back(newIdx);
	}

	dist = totalDist;
	time = totalTime;

	ret = RESULT_OK;

	return ret;
}


int32_t devideCluster(IN const ClusteringOption* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<int32_t>& vtBestways, IN const int32_t bonusValue, IN const int32_t firstIdx, IN const int32_t lastIdx, OUT stDistrict& cluster, OUT vector<uint32_t>& vtRemains)
{
	int32_t cnt = 0;

	int maxSize = vtBestways.size();

	double curValue = 0.f;
	double totValue = 0.f;
	double maxValue = 0.f;
	int typeDivision = 0;

	// 분배 방식
	if (pClustOpt->divisionType > 0) {
		typeDivision = pClustOpt->divisionType;
	}

	// 거리 분배
	if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_DIST) {
		maxValue = pClustOpt->limitValue; // 최대 거리
	}
	// 시간 분배
	else if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_TIME) {
		maxValue = pClustOpt->limitValue; // 최대 시간
	}
	// 지점 분배
	else { //if (pClustOpt->cntCluster > 0) {
		maxValue = pClustOpt->limitValue; // 최대 물량
	}

	if (bonusValue > 0) {
		maxValue += bonusValue;
	}

#if 0
	int32_t prev = vtBestways[0];
	int32_t next = 0;

	// 첫번째 poi
	cluster.vtPois.emplace_back(prev);

	for (int ii = 1; ii < maxSize; ii++) {
		next = vtBestways[ii];

		// 거리 확인
		double distNow = ppResultTables[prev][next].nTotalDist;
		// 시간 확인
		int32_t timeNow = ppResultTables[prev][next].nTotalTime;

		if (typeDivide == TYPE_CLUSTER_DEVIDE_BY_DIST) {
			curValue = distNow;
		} else 	if (typeDivide == TYPE_CLUSTER_DEVIDE_BY_TIME) {
			curValue = timeNow;
		} else { //if (pClustOpt->cntCluster > 0) {
			curValue++;
		}
		if (totValue + curValue <= maxValue) {
			// 추가
			cluster.vtPois.emplace_back(next);
			cluster.dist += distNow;
			cluster.time += timeNow;

			totValue += curValue;
		} else {
			vtRemains.assign(vtBestways.begin() + ii, vtBestways.end());
			break;
		}

		prev = next;
	} // for
#else 
	int32_t prev = 0;
	int32_t next = 0;

	for (int ii = 0; ii < maxSize; ii++) {
		double distToNext = 0.f;
		double timeToNext = 0.f;
		int spotToNext = 0;

		double distToDest = 0.f;
		double timeToDest = 0.f;
		int spotToDest = 0;

		prev = vtBestways[ii];

		if (ii == 0) {
			// 공동 사용 지점은 추가 하지 않음.
			//// 출발지는 무조건 등록, // 최대 거리가 한계치 이상일때는 어떻게 할지 고민 필요
			//cluster.vtPois.emplace_back(prev);

#if 0 // 출/도착지 고정도 poi로 사용
			// 출발지 원점이 있을 경우 원점으로 부터 시작지점까지의 값을 계산
			if (((pClustOpt->endpointType == TYPE_TSP_RECURSICVE) || // 출발지 회귀
				(pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) || // 출.도착지 고정
				(pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START))) // 출발지 고정
			{
				if (prev != firstIdx) { // 출발지와 시작이 다른 경우,  출발지까지 거리 합산
					distToNext += vtDistMatrix[firstIdx][prev].nTotalDist;
					timeToNext += vtDistMatrix[firstIdx][prev].nTotalTime;
					countToNext++;
				}
				// 공동 사용 지점은 추가 하지 않음.
			} else {
				// 출발지는 등록, // 최대 거리가 한계치 이상일때는 어떻게 할지 고민 필요
				cluster.vtPois.emplace_back(prev);
			}
#else
			cluster.vtPois.emplace_back(prev);
#endif

			// 한개의 POI만 있을 경우
			if (ii == maxSize - 1) {
				cluster.dist += distToNext;
				cluster.time += timeToNext;
			}
		}

		// 마지막 아이템이면 여기까지
		if (ii == maxSize - 1) {
#if 0 // 출/도착지 고정도 poi로 사용
			// 도착지 원점이 있을 경우 원점으로 부터 도착지점까지의 값을 계산
			if ((pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) || // 출.도착지 고정
				(pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END)) // 도착지 고정
			{
				if (next != lastIdx) {
					distToNext += vtDistMatrix[ii][lastIdx].nTotalDist;
					timeToNext += vtDistMatrix[ii][lastIdx].nTotalTime;
					countToNext++;
				}
			}
#endif

			break;
		}


		next = vtBestways[ii + 1];

		distToNext += vtDistMatrix[prev][next].nTotalDist;
		timeToNext += vtDistMatrix[prev][next].nTotalTime;
		spotToNext += 1;

		double valueToNext = 0.f;
		double valueToDest = 0.f;

#if 0 // 출/도착지 고정도 poi로 사용
		// 도착지 원점이 있을 경우 원점으로 부터 도착지점까지의 값을 계산
		if ((pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) || // 출.도착지 고정
			(pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END)) // 도착지 고정
		{
			if (next != lastIdx) {
				distToDest = vtDistMatrix[next][lastIdx].nTotalDist;
				timeToDest = vtDistMatrix[next][lastIdx].nTotalTime;
				countToDest++;

				if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_DIST) {
					valueToNext = distToNext;
					valueToDest = distToDest;
				} else 	if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_TIME) {
					valueToNext = timeToNext;
					valueToDest = timeToDest;
				} else { //if (pClustOpt->cntCluster > 0) {
					valueToNext = countToNext;
					valueToDest = countToDest;
				}
			}
		}
#endif

		if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_DIST) {
			curValue = distToNext;
		} else 	if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_TIME) {
			curValue = timeToNext;
		} else { //if (pClustOpt->cntCluster > 0) {
			curValue = spotToNext;
		}

		totValue += curValue;

		if (totValue + valueToDest <= maxValue) {
			// 공동 사용 지점은 추가 하지 않음.
			// 추가
			//cluster.vtPois.emplace_back(next);

#if 0 // 출/도착지 고정도 poi로 사용
			// 도착지 원점이 있을 경우
			if ((next == lastIdx) &&
				((pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) || // 출.도착지 고정
					(pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END))) // 도착지 고정
			{
				// 공동 사용 지점은 추가 하지 않음.
			} else {
				// 추가
				cluster.vtPois.emplace_back(next);
			}
#else
			cluster.vtPois.emplace_back(next);
#endif

			cluster.dist += distToNext;
			cluster.time += static_cast<int32_t>(timeToNext);
		} else {
			cluster.dist += distToDest;
			cluster.time += static_cast<int32_t>(timeToDest);

			vtRemains.assign(vtBestways.begin() + ii + 1, vtBestways.end());
			break;
		}

		prev = next;
	} // for
#endif

	cnt = cluster.vtPois.size();

	return cnt;
}


enum
{
	DIRECTION_NONE = 0,
	DIRECTION_INCREASE = 1,
	DIRECTION_DECREASE = 2
};


int getNextGrothMode(const int currDirection, const int currValue, const int nextValue)
{
	int retMode = currDirection;

	if (retMode == DIRECTION_INCREASE) { // 증가 중인데, 감소해야 되면 중지
		if (currValue >= nextValue) {
			retMode = DIRECTION_NONE;
		}
	} else if (retMode == DIRECTION_DECREASE) { // 감소 중인데, 증가해야 되면 중지
		if (currValue <= nextValue) {
			retMode = DIRECTION_NONE;
		}
	} else { // if (currDirection == IS_NONE) {
		if (currValue < nextValue) {
			retMode = DIRECTION_INCREASE;
		} else if (currValue > nextValue) {
			retMode = DIRECTION_DECREASE;
		} else {
			retMode = DIRECTION_NONE;
		}
	}

	return retMode;
}


int32_t CTMSManager::DevideClusterUsingTsp(IN const ClusteringOption* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtWaypoints, IN const vector<int32_t>& vtBestways, IN const int32_t firstIdx, IN const int32_t lastIdx, IN OUT int32_t& bonusValue, OUT stDistrict& cluster, OUT vector<int32_t>& vtRemains)
{
	int32_t cnt = 0;

	int maxSize = vtBestways.size();
	double maxValue = 0.f;

	// 분배 방식
	const int typeDivision = pClustOpt->divisionType;

	// 거리 분배
	if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_DIST) {
		maxValue = pClustOpt->limitValue; // 최대 거리
	}
	// 시간 분배
	else if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_TIME) {
		maxValue = pClustOpt->limitValue; // 최대 시간
	}
	// 지점 분배
	else { //if (pClustOpt->cntCluster > 0) {
		maxValue = pClustOpt->limitValue; // 최대 물량

		if ((bonusValue > 0) && (maxSize >= maxValue)) {
			int nBonus = min(bonusValue, static_cast<int>(bonusValue / static_cast<int>(maxSize / maxValue)));
			if (nBonus > 0) {
				maxValue += nBonus;
				bonusValue -= nBonus;
			}
		}
	}

	int32_t prev = 0;
	int32_t next = 0;

	vector<uint32_t> vtNewBestWays;
	vector<stWaypoints> vtNewBestWaypoints;

	stClustValue totalValue(typeDivision);
	stClustValue firstValue(typeDivision);

	// 1차) 최초 클러스터링된 순서에 의해 기본 분배
	for (int ii = 0; ii < maxSize; ii++) {
		prev = vtBestways[ii];
		
		//vtNewBestWays.emplace_back(prev);
		//vtNewBestWaypoints.emplace_back(vtWaypoints[prev]);

		// 출발지 원점이 있을 경우 원점으로 부터 시작지점까지의 값을 계산
		if ((ii == 0) && (((pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) || // 출발지 회귀
			(pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) || // 출.도착지 고정
			(pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START)))) // 출발지 고정
		{
			firstValue.addValue(CLUST_VALUE_TYPE_SPOT, 1);
			firstValue.addValue(CLUST_VALUE_TYPE_DIST, vtDistMatrix[firstIdx][prev].nTotalDist);
			firstValue.addValue(CLUST_VALUE_TYPE_TIME, vtDistMatrix[firstIdx][prev].nTotalTime + vtWaypoints[firstIdx].nLayoverTime);
			firstValue.addValue(CLUST_VALUE_TYPE_CARGO, vtWaypoints[firstIdx].nCargoVolume);
		}

		if (ii == maxSize - 1) {
			//if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_NONE) {
			//	// 고정값이 없이 마지막 아이템이면 무조건 추가 (고정값 없이 단독으로 존재하는건 의미가 없음)
			//	//break;
			//} else if ((pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) || // 출.도착지 고정
			//	(pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START)) {// 출발지 고정
			//} else if ((pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) || // 출.도착지 고정
			//	(pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END)) { // 도착지 고정 // 도착지 원점이 있을 경우 원점으로 부터 도착지점까지의 값을 계산
			//	// ****** 삭제 예정 -- 마지막인데 굳이 이렇게 계산할 필요가 있을까? // 2025-08-25
			//	totalValue.addValue(CLUST_VALUE_TYPE_SPOT, 1);
			//	totalValue.addValue(CLUST_VALUE_TYPE_DIST, vtDistMatrix[ii][lastIdx].nTotalDist);
			//	totalValue.addValue(CLUST_VALUE_TYPE_TIME, vtDistMatrix[ii][lastIdx].nTotalTime + vtWaypoints[ii].nLayoverTime);
			//	totalValue.addValue(CLUST_VALUE_TYPE_CARGO, vtWaypoints[ii].nCargoVolume);

			//	next = lastIdx;
			//} else {
			//	//break;
			//}

			//vtNewBestWays.emplace_back(prev);
			//vtNewBestWaypoints.emplace_back(vtWaypoints[prev]);

			//break;
			next = lastIdx;
		} else {
			next = vtBestways[ii + 1];
		}

		// 다음 값을 미리 비교해 보고 현재에서 종료할지 계속할지 결정하자
		stClustValue nextValue(typeDivision);

		nextValue.addValue(CLUST_VALUE_TYPE_SPOT, 1);
		nextValue.addValue(CLUST_VALUE_TYPE_DIST, vtDistMatrix[prev][next].nTotalDist);
		nextValue.addValue(CLUST_VALUE_TYPE_TIME, vtDistMatrix[prev][next].nTotalTime + vtWaypoints[prev].nLayoverTime);
		nextValue.addValue(CLUST_VALUE_TYPE_CARGO, vtWaypoints[prev].nCargoVolume);

		// 도착지 원점이 있을 경우
		stClustValue lastValue(typeDivision);

		if ((pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) || // 출.도착지 고정
			(pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END)) {// 도착지 고정
			lastValue.addValue(CLUST_VALUE_TYPE_SPOT, 1);
			lastValue.addValue(CLUST_VALUE_TYPE_DIST, vtDistMatrix[next][lastIdx].nTotalDist);
			lastValue.addValue(CLUST_VALUE_TYPE_TIME, vtDistMatrix[next][lastIdx].nTotalTime + vtWaypoints[lastIdx].nLayoverTime);
			lastValue.addValue(CLUST_VALUE_TYPE_CARGO, vtWaypoints[lastIdx].nCargoVolume);
		}
		
		if (totalValue.getCurrentValue() + firstValue.getCurrentValue() + lastValue.getCurrentValue() + nextValue.getCurrentValue() > maxValue) {
			totalValue = totalValue + firstValue + lastValue;

			if (ii == 0 || vtNewBestWays.empty()) {
				vtNewBestWays.emplace_back(prev);
				vtNewBestWaypoints.emplace_back(vtWaypoints[prev]);
			}
			break;
		} else {
			totalValue = totalValue + nextValue;
		}

		vtNewBestWays.emplace_back(prev);
		vtNewBestWaypoints.emplace_back(vtWaypoints[prev]);

		prev = next;
	} // for


	// 남은 배송지
	vtRemains.assign(vtBestways.begin() + vtNewBestWaypoints.size(), vtBestways.end());


	// 2차) 1차 분배된 값을 TSP 적용 후 상세하게 조정
	// TSP
	TspOption tspOpt = pClustOpt->tspOption;
	tspOpt.endpointType = pClustOpt->endpointType;

	// 최초 분배의 TSP를 확인하자
	stClustValue prevTotalValue(typeDivision);
	stClustValue nextTotalValue(typeDivision);

	vector<int32_t> vtPrevBestways;
	vector<int32_t> vtNextBestways;
	vector<stWaypoints> vtPrevWaypoints;
	vector<stWaypoints> vtNextWaypoints;

	vtNextBestways.assign(vtNewBestWays.begin(), vtNewBestWays.end());
	vtNextWaypoints.assign(vtNewBestWaypoints.begin(), vtNewBestWaypoints.end());

	// 출도착지점 임시로 추가하여 TSP 계산
	// 출발지 고정일 경우, 출발지 추가
	if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
		vtNextBestways.insert(vtNextBestways.begin(), firstIdx);
		stWaypoints waypoint;
		waypoint.nId = vtWaypoints[firstIdx].nId;
		waypoint.x = vtWaypoints[firstIdx].x;
		waypoint.y = vtWaypoints[firstIdx].y;
		waypoint.nLayoverTime = vtWaypoints[firstIdx].nLayoverTime;
		waypoint.nCargoVolume = vtWaypoints[firstIdx].nCargoVolume;
		vtNextWaypoints.insert(vtNextWaypoints.begin(), waypoint);
	}
	// 도착지 고정일 경우, 도착지 추가
	if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) {
		vtNextBestways.emplace_back(lastIdx);
		stWaypoints waypoint;
		waypoint.nId = vtWaypoints[lastIdx].nId;
		waypoint.x = vtWaypoints[lastIdx].x;
		waypoint.y = vtWaypoints[lastIdx].y;
		waypoint.nLayoverTime = vtWaypoints[lastIdx].nLayoverTime;
		waypoint.nCargoVolume = vtWaypoints[lastIdx].nCargoVolume;
		vtNextWaypoints.emplace_back(waypoint);
	} else {
		;
	}

	int ret = RESULT_FAILED;
	int nUpDownState = DIRECTION_NONE;
	bool isStop = false;

	int maxLoop = vtDistMatrix.size(); // 임의로 최대 전체 방문지 개수로 설정
	int idxIncrease = 0; // 추가 시, 남은 배열에서 가져오는 인덱스
	int idxDecrease = 0; // 삭제 시, 남은 배열에서 가져오는 인덱스
	for (int now = 0; now < maxLoop; now++) {
		if (vtNextWaypoints.size() <= 1) {
			nextTotalValue.setValue(CLUST_VALUE_TYPE_TIME, vtNextWaypoints[0].nLayoverTime); // 출발지 유예 시간
			nextTotalValue.setValue(CLUST_VALUE_TYPE_CARGO, vtNextWaypoints[0].nCargoVolume); // 출발지 화물 수량
			ret = RESULT_OK;
		} else if (vtNextWaypoints.size() <= 2) { // 2개 이하의 poi는 tsp 돌리지 말고 바로 거리 구해야 함.
			int prev = vtNextBestways[0];
			int curr = vtNextBestways[1];
			
			nextTotalValue.setValue(CLUST_VALUE_TYPE_DIST, vtDistMatrix[prev][curr].nTotalDist);
			nextTotalValue.setValue(CLUST_VALUE_TYPE_TIME, vtDistMatrix[prev][curr].nTotalTime + vtNextWaypoints[0].nLayoverTime + vtNextWaypoints[1].nLayoverTime); // 출/도착지 유예 시간
			nextTotalValue.setValue(CLUST_VALUE_TYPE_CARGO, vtNextWaypoints[0].nCargoVolume + vtNextWaypoints[1].nCargoVolume); // 출/도착지 화물 수량

			// GetBestway 통과 이후와 같이 남은 녀석들의 순서를 맞춰준다.
			vtNextBestways[0] = 0;
			vtNextBestways[1] = 1;

			ret = RESULT_OK;
		} else {
			if ((typeDivision == TYPE_CLUSTER_DEVIDE_BY_COUNT) &&
				((pClustOpt->max_distance <= 0) && (pClustOpt->max_time <= 0) && (pClustOpt->max_spot <= 0))) {
				// 개수 분배는 굳이 TSP를 새로 돌리지 말자(속도향상) 
				ret = RESULT_OK;
			} else {
#if 0
				// 클러스터당 POI WM을 새로 구성한다. 
				vector<vector<stDistMatrix>> vtNewDistMatrix;
				for (const auto& poiCols : vtNextWaypoints) {
					vector<stDistMatrix> vtRowsDM;
					for (const auto& poiRows : vtNextWaypoints) {
						vtRowsDM.emplace_back(vtDistMatrix[poiCols.nId][poiRows.nId]);
					}
					vtNewDistMatrix.emplace_back(vtRowsDM);
				}

				tspOpt.geneSize = vtNextWaypoints.size(); // 변경된 사이즈 반영
				ret = GetBestway(&tspOpt, vtNewDistMatrix, vtNextWaypoints, vtNextBestways, nextTotDist, nextTotTime);

				//// bestway 결과로 순서가 변경되어 재 설정
				vtNewBestWaypoints.clear();
				for (const auto way : vtNextBestways) {
					vtNewBestWaypoints.push_back(vtNextWaypoints[way]);
				}
#else 
				ret = GetNewBestway(&tspOpt, vtDistMatrix, vtNextWaypoints, vtNewBestWaypoints, vtNextBestways, nextTotalValue);
#endif
			}
		}

		if (ret != RESULT_OK) {
			break;
		}

		// 출발지/도착지/출도착지 고정 또는 회귀면서 1개
		// 그 외 남은게 2개 면
		// 종료
		//if ( ((pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END || pClustOpt->endpointType == TYPE_TSP_RECURSICVE || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) && (vtRemains.size() == 1)) ||
		//	((pClustOpt->endpointType == TYPE_TSP_ENDPOINT_NONE && vtRemains.size() == 2)) ) { // ()
		//	break;
		//}

		double dwNextValue = 0.f;
		if (ret == RESULT_OK) {
			dwNextValue = nextTotalValue.getCurrentValue();

			nUpDownState = getNextGrothMode(nUpDownState, dwNextValue, maxValue);
					
			// 최대 거리/시간이 설정되었으면 최대값 넘지 않도록.
			if (((typeDivision != TYPE_CLUSTER_DEVIDE_BY_DIST) && (0 < pClustOpt->max_distance) && (pClustOpt->max_distance < nextTotalValue.getValue(CLUST_VALUE_TYPE_DIST))) ||
				((typeDivision != TYPE_CLUSTER_DEVIDE_BY_TIME) && (0 < pClustOpt->max_time) && (pClustOpt->max_time < nextTotalValue.getValue(CLUST_VALUE_TYPE_TIME))) ||
				((typeDivision != TYPE_CLUSTER_DEVIDE_BY_COUNT) && (0 < pClustOpt->max_spot) && (pClustOpt->max_spot < nextTotalValue.getValue(CLUST_VALUE_TYPE_SPOT)))) {
				nUpDownState = DIRECTION_DECREASE;
			} 
				
			if ((typeDivision == TYPE_CLUSTER_DEVIDE_BY_COUNT) &&
				((pClustOpt->max_distance <= 0) && (pClustOpt->max_time <= 0) && (pClustOpt->max_spot <= 0))) {
				isStop = true;
				idxDecrease++; // 개수 분배는 바로 끝낸다.
			} else if (nUpDownState == DIRECTION_NONE) {
				isStop = true;
			} else if ((nUpDownState == DIRECTION_INCREASE) && (vtRemains.empty())) {
				isStop = true;
				idxDecrease++;
			} else if ((nUpDownState == DIRECTION_DECREASE) && (nextTotalValue.getValue(CLUST_VALUE_TYPE_SPOT) <= 3) && ((pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) || (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE))) {
				isStop = true;
				idxDecrease++;
			} else if ((nUpDownState == DIRECTION_DECREASE) && (nextTotalValue.getValue(CLUST_VALUE_TYPE_SPOT) <= 2)) {
				isStop = true;
				idxDecrease++;
			}
		}

		// 남은게 없는데 증가해야 할 경우, --> 이 구문은 굳이 필요 없을꺼 같은데, 2025-06-11
		//if (((pClustOpt->max_distance <= 0) || ((0 < pClustOpt->max_distance) && (nextTotDist <= pClustOpt->max_distance))) &&
		//	((pClustOpt->max_time <= 0) || ((0 < pClustOpt->max_time) && (nextTotTime <= pClustOpt->max_time))) &&
		//	((pClustOpt->max_count <= 0) || ((0 < pClustOpt->max_count) && (nextTotCount <= pClustOpt->max_count)))) {
		//	break;
		//} else if (isStop) {
		if (isStop) {
			if (typeDivision != TYPE_CLUSTER_DEVIDE_BY_COUNT) {
				// 오버된 값이 보너스값 한계치 이내라면 보너스 값으로 보완
				if ((0 < bonusValue) && (dwNextValue < maxValue + bonusValue)) {
					bonusValue -= (dwNextValue - maxValue);
				} else if (idxDecrease) { // 감소중 중지된 경우, 이전값 사용하지 않음.
					break;
				} else if (vtPrevBestways.size()) { // 이전 값이 있으면, 이전 값 복원
					// 이전 값 사용
					vtNextBestways.assign(vtPrevBestways.begin(), vtPrevBestways.end());
					vtNextWaypoints.assign(vtPrevWaypoints.begin(), vtPrevWaypoints.end());

					nextTotalValue = prevTotalValue;
				}
			}

			if ((typeDivision == TYPE_CLUSTER_DEVIDE_BY_COUNT) &&
				((pClustOpt->max_distance <= 0) && (pClustOpt->max_time <= 0) && (pClustOpt->max_spot <= 0))) {
				// 개수 분배시 수행하지 않은 TSP를 이제 한번 수행하자
#if 0
				// 클러스터당 POI WM을 새로 구성한다. 
				vector<vector<stDistMatrix>> vtNewDistMatrix;
				for (const auto& poiCols : vtNextWaypoints) {
					vector<stDistMatrix> vtRowsDM;
					for (const auto& poiRows : vtNextWaypoints) {
						vtRowsDM.emplace_back(vtDistMatrix[poiCols.nId][poiRows.nId]);
					}
					vtNewDistMatrix.emplace_back(vtRowsDM);
				}

				tspOpt.geneSize = vtNextWaypoints.size(); // 변경된 사이즈 반영
				ret = GetBestway(&tspOpt, vtNewDistMatrix, vtNextWaypoints, vtNextBestways, nextTotDist, nextTotTime);

				// bestway 결과로 순서가 변경되어 재 설정
				vtNewBestWaypoints.clear();
				for (const auto way : vtNextBestways) {
					vtNewBestWaypoints.push_back(vtNextWaypoints[way]);
				}
#else 
				if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_NONE) {
#	if 0
					vtNewBestWaypoints.assign(vtNextWaypoints.begin(), vtNextWaypoints.end());
					ret = RESULT_OK;
#	else
					ret = GetNewBestway(&tspOpt, vtDistMatrix, vtNextWaypoints, vtNewBestWaypoints, vtNextBestways, nextTotalValue);
#	endif
				} else {
					ret = GetNewBestway(&tspOpt, vtDistMatrix, vtNextWaypoints, vtNewBestWaypoints, vtNextBestways, nextTotalValue);
				}				
#endif
			}
			break;
		}

		// 이전 값 캐시
		prevTotalValue = nextTotalValue;

		vtPrevBestways.assign(vtNextBestways.begin(), vtNextBestways.end());
		vtPrevWaypoints.assign(vtNextWaypoints.begin(), vtNextWaypoints.end());

		// 값이 부족해 더 추가 필요
		if (nUpDownState == DIRECTION_INCREASE) {
			if (vtRemains.size() <= idxIncrease) {
				break; // 더이상 추가 할 수 없음
			}

			int idx = vtRemains[idxIncrease++];
			stWaypoints waypoint;
			waypoint.nId = vtWaypoints[idx].nId;
			waypoint.x = vtWaypoints[idx].x;
			waypoint.y = vtWaypoints[idx].y;
			waypoint.nLayoverTime = vtWaypoints[idx].nLayoverTime;
			waypoint.nCargoVolume = vtWaypoints[idx].nCargoVolume;

			// 출발지 고정일 경우, 고정값 빼고 추가
			if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
				vtNextBestways.emplace_back(idx);  // 2025.06.04
				vtNextWaypoints.emplace_back(waypoint);
			}
			// 도착지 고정일 경우, 고정값 빼고 추가
			else if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) {
				vtNextBestways.insert(vtNextBestways.end() - 1, idx); // 2025.06.04
				vtNextWaypoints.insert(vtNextWaypoints.end() - 1, waypoint);
			} else {
				vtNextBestways.emplace_back(idx); // 2025.06.04
				vtNextWaypoints.emplace_back(waypoint);
			}
		} else if (nUpDownState == DIRECTION_DECREASE) {
			if (vtNextBestways.empty()) {
				break; // 더이상 뺄 수 없음
			}

			idxDecrease++;

			// 출발지 고정일 경우, 고정값 빼고 제거
			if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
				if (vtNextWaypoints.size() <= 2) { // 1 -> 2 2025-06-11
					break; // 더이상 뺄 수 없음
				} else {
					vtNextBestways.pop_back(); // 2025.06.04
					vtNextWaypoints.pop_back();
				}
			} 
			// 도착지 고정일 경우, 고정값 빼고 제거
			else if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END) {
				if (vtNextWaypoints.size() <= 2) { // 1 -> 2 2025-06-11
					break; // 더이상 뺄 수 없음
				} else {
					vtNextBestways.erase(vtNextBestways.end() - 2); // 2025.06.04
					vtNextWaypoints.erase(vtNextWaypoints.end() - 2);
				}
			} 
			// 출-도착지 고정일 경우, 고정값 빼고 제거
			else if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) {
				if (vtNextWaypoints.size() <= 3) { // 2 -> 3 2025-06-11
					break; // 더이상 뺄 수 없음
				} else {
					vtNextBestways.erase(vtNextBestways.end() - 2); // 2025.06.04
					vtNextWaypoints.erase(vtNextWaypoints.end() - 2);
				}
			} else if (vtNextWaypoints.size() <= 1) { // 2025-06-11
				break; // 더이상 뺄 수 없음
			} else {
				vtNextBestways.pop_back(); // 2025.06.04
				vtNextWaypoints.pop_back();
			}
		}
	} // for

	vtNewBestWays.assign(vtNextBestways.begin(), vtNextBestways.end());
	vtNewBestWaypoints.assign(vtNextWaypoints.begin(), vtNextWaypoints.end());
	//// bestway 결과로 순서가 변경되어 재 설정
	//vtNewBestWaypoints.clear();
	//for (const auto way : vtNextBestways) {
	//	vtNewBestWaypoints.push_back(vtNextWaypoints[way]);
	//}

	// 임시로 추가된 출도착지점 제거
	// 출발지 고정일 경우, 출발지 빼기
	if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
		vtNextBestways.erase(vtNextBestways.begin());
		vtNextWaypoints.erase(vtNextWaypoints.begin());
	}
	// 도착지 고정일 경우, 도착지 빼기
	if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END || pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) {
		vtNextBestways.pop_back();
		vtNextWaypoints.pop_back();
	} else {
		;
	}

	vtRemains.clear();
	if (vtNextBestways.size() < vtBestways.size()) {
		vtRemains.assign(vtBestways.begin() + vtNextBestways.size(), vtBestways.end());
	}

	//vtNewBestWays.assign(vtNextBestways.begin(), vtNextBestways.end());
	//vtNewBestWaypoints.assign(vtNextWaypoints.begin(), vtNextWaypoints.end());

	totalValue = nextTotalValue;

#if 0
	ret = getDistrictResult(pClustOpt, vtDistMatrix, vtNewBestWaypoints, firstIdx, lastIdx, cluster);
#else
	vector<stWaypoints> vtNewClust;
	vtNewClust.assign(vtNewBestWaypoints.begin(), vtNewBestWaypoints.end());

	//// 출발지 고정
	//if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START ||
	//	pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END ||
	//	pClustOpt->endpointType == TYPE_TSP_RECURSICVE) {
	//	vtNewClust.insert(vtNewClust.begin(), vtWaypoints[firstIdx]);
	//}

	//// 도착지 고정일 경우
	//if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END ||
	//	pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) {
	//	vtNewClust.emplace_back(vtWaypoints[lastIdx]);
	//}

	cnt = getNewDistrictResult(pClustOpt, vtDistMatrix, vtNewClust, cluster);
#endif

	return cnt;
}


int32_t CTMSManager::Clustering(IN const ClusteringOption* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtWaypoints, IN const vector<int32_t>& vtBestway, IN const int32_t bonusValue, OUT vector<stDistrict>& vtClusters)
{
	int32_t ret = RESULT_FAILED;

	vector<int32_t> vtBestwayWork;
	int32_t nRemainedBonusValue = bonusValue;
	int32_t nFirst = vtBestway.front();
	int32_t nLast = vtBestway.back();
	bool isEmpty = false;

	// 고정값이 있을 경우 고정값을 빼고 구성 <-- 고정값 따로 빼지 않고 수행해도 큰 수정없이 코드는 더 간단해 지지 않으려나? 시간나면 검토해라 // 2025-06-12
	if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START_END) { // 출.도착지 고정
		vtBestwayWork.assign(vtBestway.begin() + 1, vtBestway.end() - 1);
	} else if ((pClustOpt->endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) || // 출발지 회귀
		(pClustOpt->endpointType == TYPE_TSP_ENDPOINT_START)) { // 출발지 고정
		vtBestwayWork.assign(vtBestway.begin() + 1, vtBestway.end());
	} else if (pClustOpt->endpointType == TYPE_TSP_ENDPOINT_END) { // 목적지 고정
		vtBestwayWork.assign(vtBestway.begin(), vtBestway.end() - 1);
	} else {
		vtBestwayWork.assign(vtBestway.begin(), vtBestway.end());
	}

	for (int cntCluster = 0; isEmpty != true; cntCluster++) {
		//tspOpt.geneSize = vtRequest.size();
		//ret = GetBestway(&tspOpt, ppResultTables, &vtRequest, vtBestway);

		vector<int32_t> vtRemains;
		stDistrict cluster;
		//int32_t bonus = (nRemainedBonusValue-- > 0) ? 1 : 0;
#if 0
		int cntPoi = devideCluster(pClustOpt, vtDistMatrix, vtBestwayWork, bonus, nFirst, nLast, cluster, vtRemains);
#else
		int cntPoi = DevideClusterUsingTsp(pClustOpt, vtDistMatrix, vtWaypoints, vtBestwayWork, nFirst, nLast, nRemainedBonusValue, cluster, vtRemains);
#endif	
		if (cntPoi > 0) {
			cluster.id = cntCluster;
			//for (const auto& poi : cluster.vtPois) {
			//	cluster.vtCoord.emplace_back(SPoint{ vtWaypoints[poi].x, vtWaypoints[poi].y });
			//}
			vtClusters.emplace_back(cluster);
		}

		isEmpty = vtRemains.empty();

		if (isEmpty) {
			ret = RESULT_OK;
			break;
		} else {
			//vtRequest.clear();
			vtBestwayWork.assign(vtRemains.begin(), vtRemains.end());
			//for (const auto& idx : vtRemains) {
			//	//vtRequest.emplace_back(vtOrigin->at(idx));
			//	//vtBestway.emplace_back(vtOrigin->at(idx).nId);
			//	vtBestway.emplace_back(idx);
			//}
		}
	} // for

	return ret;
}


int32_t CTMSManager::DevideClusterUsingLink(IN const ClusteringOption* pClustOpt, IN const vector<Origins>& vtOrigin, OUT vector<stDistrict>& vtDistrict)
{
	int32_t ret = RESULT_OK;

	int maxSize = vtOrigin.size();

	typedef struct _tagstGroupItem
	{
		// id, link nid 사용
		int32_t idxOrigin;
		KeyID linkId;
		SPoint ptOrigin;
		int32_t groupId;
		SPoint ptCenter;
		SPoint ptOptimal;
		int32_t leftSide; // 0:right, 1:left
	}stGroupItem;
	vector<stGroupItem> vtItems;

	unordered_map<uint64_t, int32_t> umapRightItems;
	unordered_map<uint64_t, int32_t> umapLeftItems;
	unordered_map<uint64_t, int32_t>::iterator itItems;

	int distId = 0;

	// 최적지점 위치 확인
	for (int ii = 0; ii < vtOrigin.size(); ii++) {
		stLinkInfo* pLink = nullptr;
		stEntryPointInfo entInfo;
		stGroupItem item = {};
		item.idxOrigin = ii;
		item.ptOrigin = item.ptOptimal = vtOrigin[ii].position;

		//if (!vtLinkKey.empty() && (vtLinkKey.size() < ii) && (vtLinkKey[ii].llid != 0)) {
		//	pLink = m_pDataMgr->GetVLinkDataById(item.linkId);
		//}

		// 가까운 링크 찾기
		if (pLink == nullptr) {
			const int maxDist = 3000; // 3km
			pLink = m_pDataMgr->GetNearRoadByPoint(item.ptOptimal.x, item.ptOptimal.y, maxDist, TYPE_LINK_MATCH_CARSTOP_EX, TYPE_LINK_DATA_VEHICLE, -1, entInfo);

			if (pLink == nullptr) {
				LOG_TRACE(LOG_WARNING, "near link info not exist, idx:%d, x:%.5f, y:%.5f, dist:%d", ii, vtOrigin[ii].position.x, vtOrigin[ii].position.y, maxDist);
				continue;
			}		
		}

		item.linkId = pLink->link_id;

		// link vertex 구하고 중심 계산
		double dwOffsetMeter = 1.5;//0.3; // 30 cm 좀 크게 떨어뜨렸을때 경탐시 의도한 링크가 아닌 근처 다른 링크에 매칭될 수 있어 주의 필요

		bool isLeft = entInfo.linkLeft;

		// pass_code(규제코드), 1:통행가능, 2:통행불가, 3:공사구간, 4:공사계획구간, 5:일방통행_정, 6:일방통행_역
		// 2차선 이하(8레벨 이하) 비분리 도로는 도로 우측				
		if ((pLink->veh.level >= 5) && ((pLink->veh.lane_cnt <= 2) && (pLink->veh.link_type != 2))) {
			// 무조건 정방향 우측에 두자
			// 2차선 이하(5레벨 이하) 추가 // 2025-12-22
			if (pLink->veh.lane_cnt >= 2) {
				dwOffsetMeter = 3.0;
			}
			item.leftSide = 0;
		} else if (pLink->veh.pass_code == 5) {
			// 일방 정방향이면 무조건 도로 우측으로 매칭
			if (isLeft) {
				dwOffsetMeter *= -1;
			}
			item.leftSide = 0;
		} else if (pLink->veh.pass_code == 6) {
			// 일방 역방향이면 무조건 도로 좌측으로 매칭
			if (!isLeft) {
				dwOffsetMeter *= -1;
			}
			item.leftSide = 1;
		} else {
			if (isLeft) {
				dwOffsetMeter *= -1;
				item.leftSide = 1;
			} else {
				item.leftSide = 0;
			}
		}

		SPoint ptCenter;
		SPoint ptOffset;

		if (getPolylineCenterWithOffset(pLink->getVertex(), pLink->getVertexCount(), dwOffsetMeter, ptCenter, ptOffset) == true) {
			item.ptCenter = ptOffset;
		} else if (getPolylineCenter(pLink->getVertex(), pLink->getVertexCount(), ptCenter) == true) {
			item.ptCenter = ptCenter;
		}


		// find and insert
		unordered_map<uint64_t, int32_t>* pMap;
		if (item.leftSide) {
			pMap = &umapLeftItems;
		} else {
			pMap = &umapRightItems;
		}

		if ((itItems = pMap->find(item.linkId.llid)) != pMap->end()) {
			item.groupId = itItems->second;
		} else { // or create
			item.groupId = distId++;
			pMap->emplace(item.linkId.llid, item.groupId);
		}

		vtItems.emplace_back(item);
	} // for

	vtDistrict.clear();
	vtDistrict.resize(umapLeftItems.size() + umapRightItems.size());

	for (const auto& item : vtItems) {
		int idx = item.groupId;
		
		if (vtDistrict[idx].vtPois.empty()) {
			// 첫번째
			vtDistrict[idx].id = item.groupId;
			vtDistrict[idx].center = item.ptCenter;
		}

		vtDistrict[idx].vtPois.emplace_back(item.idxOrigin);
		vtDistrict[idx].vtCoord.emplace_back(item.ptOptimal);
	}

	LOG_TRACE(LOG_DEBUG, "total groups:%d, origins:%d", vtDistrict.size(), vtItems.size());

	return ret;
}


SPoint calculateCentroid(const vector<SPoint>& points)
{
	double cx = 0, cy = 0, area = 0;
	int n = points.size();

	if (n <= 0) {
		;
	} else if (n == 1) {
		cx = points[0].x;
		cy = points[0].y;
	} else if (n == 2) {
		cx = (points[0].x + points[1].x) / 2;
		cy = (points[0].y + points[1].y) / 2;
	} else {
		for (int i = 0; i < n; i++) {
			int j = (i + 1) % n;  // Next vertex index
			double x0 = points[i].x, y0 = points[i].y;
			double x1 = points[j].x, y1 = points[j].y;

			double cross_product = x0 * y1 - x1 * y0;
			cx += (x0 + x1) * cross_product;
			cy += (y0 + y1) * cross_product;
			area += cross_product;
		}

		area *= 0.5;
		if (area != 0) {
			cx /= (6 * area);
			cy /= (6 * area);
		}
	}

	return { cx, cy };
}


int32_t CTMSManager::GetBoundary(IN vector<SPoint>& vtCoord, OUT vector<SPoint>& vtBoundary, OUT SPoint& center)
{
	int32_t ret = RESULT_OK;

	if (vtCoord.empty()) {
		return ret;
	}

	//vector<SPoint> coords;
	//vector<SPoint> border;
	//vector<SPoint> expend;
	//vector<SPoint> slice;

	// make border
	ConvexHull(vtCoord, vtBoundary);

	// make closed
	if (!vtBoundary.empty() && ((vtBoundary.front().x != vtBoundary.back().x) || (vtBoundary.front().y != vtBoundary.back().y))) {
		vtBoundary.emplace_back(vtBoundary.front());
	}

	center = calculateCentroid(vtBoundary);


#if 0
	// 면적 조사
	double area = getPolygonArea(border);

	if (area <= 1) {
		vtBoundary.assign(border.begin(), border.end());
	} else {
		// 2023.7.26
		// catmullline의 2DVector포함 빌드시 node 동작 안하는 이슈 발생

		// 임시로 직선 바운더리 사용
		// cluster.border = border;


		// expend border
		GetBorderOfPolygon(border, -0.002, expend);

		// 외곽선 일정거리로 나누기
		GetSlicedLine(expend, 1000, slice);

		// 외곽선 부드럽게
		GetCatmullLine(slice, 0, vtBoundary);
	}
#endif

	return ret;
}


int32_t readWeightMatrix(IN const BYTE* pszBinary, IN const size_t sizeData, OUT BaseOption& option, OUT vector<Origins>& vtOrigin, OUT vector<vector<stDistMatrix>>& vtDistMatrix)
{
	int32_t ret = RESULT_FAILED;

	if (pszBinary == nullptr || sizeData <= 0) {
		return ret;
	}

	size_t readOffset = 0;

	// read base & check
	FileBase base;
	if (readOffset >= sizeData) {
		LOG_TRACE(LOG_ERROR, "rdm base offset is over than size, %d, %d", readOffset, sizeData);
		return ret;
	}
	memcpy(&base, &pszBinary[readOffset], sizeof(base));
	readOffset += sizeof(base);
	if (strcmp(base.szType, "RDM") != 0) {
		LOG_TRACE(LOG_ERROR, "rdm file type not match with RDM, type:%s", base.szType);
		return ret;
	}

	// read header & check
	FileHeaderRDM header;
	if (readOffset >= sizeData) {
		LOG_TRACE(LOG_ERROR, "rdm header offset is over than size, %d, %d", readOffset, sizeData);
		return ret;
	}
	memcpy(&header, &pszBinary[readOffset], sizeof(header));
	readOffset += sizeof(header);

	//if (header.crcData != crc) {
	//	LOG_TRACE(LOG_ERROR, "rdm crc not match with %d vs %d", header.crcData, crc);
	//	return ret;
	//}

	//if (header.cntItem != cntItem) {
	//	LOG_TRACE(LOG_ERROR, "rdm item count not match with %d vs %d", header.cntItem, cntItem);
	//	return ret;
	//}

	// read option
	if (readOffset >= sizeData) {
		LOG_TRACE(LOG_ERROR, "rdm option offset is over than size, %d, %d", readOffset, sizeData);
		return ret;
	}
	memcpy(&option, &pszBinary[readOffset], sizeof(option));
	readOffset += sizeof(option);

	// read origin
	for (int ii = 0; ii < header.cntItem; ii++) {
		Origins origin;
		if (readOffset >= sizeData) {
			LOG_TRACE(LOG_ERROR, "rdm origin offset is over than size, %d, %d", readOffset, sizeData);
			return ret;
		}
		memcpy(&origin.position, &pszBinary[readOffset], sizeof(origin));
		readOffset += sizeof(origin);
		vtOrigin.emplace_back(origin);
	}
	
	// read body
	for (int ii = 0; ii < header.cntItem; ii++) {
		vector<stDistMatrix> vtDistMatrixRow;
		for (int jj = 0; jj < header.cntItem; jj++) {
			stDistMatrix item;
			// read matrix
			if (readOffset >= sizeData) {
				LOG_TRACE(LOG_ERROR, "rdm matrix offset is over than size, %d, %d", readOffset, sizeData);
				return ret;
			}
			memcpy(&item, &pszBinary[readOffset], sizeof(item)); // read 4byte
			readOffset += sizeof(item);

			vtDistMatrixRow.emplace_back(item);
		} // for
		vtDistMatrix.emplace_back(vtDistMatrixRow);
	} // for

	if ((readOffset <= 0 || vtDistMatrix.empty()) || (vtOrigin.size() != vtDistMatrix.size())) {
		LOG_TRACE(LOG_ERROR, "failed, read weight matrix count, origin:%d vs matrix:%d", vtOrigin.size(), vtDistMatrix.size());
	} else {
		ret = RESULT_OK;
	}

	return ret;
}


int32_t readWeightMatrixRouteLine(IN const BYTE* pszBinary, IN const size_t sizeData, OUT vector<vector<FileIndex>>& vtPathMatrixIndex)
{
	int32_t ret = RESULT_FAILED;

	if (pszBinary == nullptr || sizeData <= 0) {
		return ret;
	}

	size_t readOffset = 0;

	// base
	FileBase base;
	if (readOffset >= sizeData) {
		LOG_TRACE(LOG_ERROR, "rdm line base offset is over than size, %d, %d", readOffset, sizeData);
		return ret;
	}
	memcpy(&base, &pszBinary[readOffset], sizeof(base));
	readOffset += sizeof(base);

	if (strcmp(base.szType, "RLN") != 0) {
		LOG_TRACE(LOG_ERROR, "rdm file type not match with RLN, type:%s", base.szType);
		return ret;
	}

	// header
	FileHeader header;
	if (readOffset >= sizeData) {
		LOG_TRACE(LOG_ERROR, "rdm line header offset is over than size, %d, %d", readOffset, sizeData);
		return ret;
	}
	memcpy(&header, &pszBinary[readOffset], sizeof(header));
	readOffset += sizeof(header);

	int nCount = sqrt(header.cntIndex);

	// index
	for (int ii = 0; ii < nCount; ii++) {
		vector<FileIndex> vtCol;
		for (int jj = 0; jj < nCount; jj++) {
			FileIndex index;
			if (readOffset >= sizeData) {
				LOG_TRACE(LOG_ERROR, "rdm line index offset is over than size, %d, %d", readOffset, sizeData);
				return ret;
			}
			memcpy(&index, &pszBinary[readOffset], sizeof(index));
			readOffset += sizeof(index);

			vtCol.emplace_back(index);
		}
		vtPathMatrixIndex.emplace_back(vtCol);
	}

	// body -- 폴리라인 사이즈가 크기도 클뿐더러, 필요한 녀석만 실시간으로 빼오는 방식을 사용해야 하기에 여기서는 읽지 않는다, 2025-06-20
	// for route line info table
	//for (int row = 0; row < nCount; row++) {
	//	vector<stPathMatrix > vtPathMatrixCol;
	//	for (int col = 0; col < nCount; col++) {
	//		int cnt = vtRow[row][col].szBody / sizeof(SPoint);
	//		vector<SPoint> vtLines(cnt);
	//		stPathMatrix pathMatrix;

	//		fseek(fp, vtRow[row][col].offBody, SEEK_SET);
	//		fread(&vtLines, 1, vtRow[row][col].szBody, fp);

	//		vtPathMatrixCol.emplace_back(pathMatrix);
	//	} // for
	//	vtPathMatrix.emplace_back(vtPathMatrixCol);
	//} // for

	ret = RESULT_OK;

	return ret;
}


int32_t getRequestBaseOption(IN const cJSON* pJson, IN const uint32_t nCrc, OUT BaseOption& option)
{
	int32_t crc = nCrc;

	cJSON* pValue = cJSON_GetObjectItem(pJson, "id");
	if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
		sprintf(option.userId, "%lld", static_cast<int64_t>(cJSON_GetNumberValue(pValue)));
	} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
		strcpy(option.userId, cJSON_GetStringValue(pValue));
	}
	if (strlen(option.userId) <= 0) {
		strcpy(option.userId, "tester_0123456789");
	}
	crc = crc32(crc, reinterpret_cast<Bytef*>(&option.userId), sizeof(option.userId));

	//pValue = cJSON_GetObjectItem(pJson, "uri");
	//if ((pValue != NULL) && cJSON_IsString(pValue)) {
	//	option.uri = cJSON_GetStringValue(pValue);
	//}

	pValue = cJSON_GetObjectItem(pJson, "option");
	if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
		option.option = cJSON_GetNumberValue(pValue);
	} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
		option.option = atoi(cJSON_GetStringValue(pValue));
	}
	crc = crc32(crc, reinterpret_cast<Bytef*>(&option.option), sizeof(option.option));

	pValue = cJSON_GetObjectItem(pJson, "avoid");
	if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
		option.avoid = cJSON_GetNumberValue(pValue);
	} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
		option.avoid = atoi(cJSON_GetStringValue(pValue));
	}
	crc = crc32(crc, reinterpret_cast<Bytef*>(&option.traffic), sizeof(option.avoid));

	pValue = cJSON_GetObjectItem(pJson, "traffic");
	if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
		option.traffic = cJSON_GetNumberValue(pValue);
	} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
		option.traffic = atoi(cJSON_GetStringValue(pValue));
	}
	crc = crc32(crc, reinterpret_cast<Bytef*>(&option.traffic), sizeof(option.traffic));

	pValue = cJSON_GetObjectItem(pJson, "time");
	if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
		option.timestamp = cJSON_GetNumberValue(pValue);
	} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
		option.timestamp = atoi(cJSON_GetStringValue(pValue));
	}
	if (option.timestamp <= 0) {
		option.timestamp = time(NULL);
	}
	crc = crc32(crc, reinterpret_cast<Bytef*>(&option.timestamp), sizeof(option.timestamp));

	pValue = cJSON_GetObjectItem(pJson, "free");
	if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
		option.free = cJSON_GetNumberValue(pValue);
	} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
		option.free = atoi(cJSON_GetStringValue(pValue));
	}
	if (option.free <= 0) {
		option.free = 0;
	}
	crc = crc32(crc, reinterpret_cast<Bytef*>(&option.free), sizeof(option.free));

	pValue = cJSON_GetObjectItem(pJson, "distance_type");
	if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
		option.distance_type = cJSON_GetNumberValue(pValue);
	} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
		option.distance_type = atoi(cJSON_GetStringValue(pValue));
	}
	crc = crc32(crc, reinterpret_cast<Bytef*>(&option.distance_type), sizeof(option.distance_type));

	pValue = cJSON_GetObjectItem(pJson, "compare_type");
	if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
		option.compare_type = cJSON_GetNumberValue(pValue);
	} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
		option.compare_type = atoi(cJSON_GetStringValue(pValue));
	}
	crc = crc32(crc, reinterpret_cast<Bytef*>(&option.compare_type), sizeof(option.compare_type));

	pValue = cJSON_GetObjectItem(pJson, "expand_method");
	if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
		option.expand_method = cJSON_GetNumberValue(pValue);
	} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
		option.expand_method = atoi(cJSON_GetStringValue(pValue));
	}
	crc = crc32(crc, reinterpret_cast<Bytef*>(&option.expand_method), sizeof(option.expand_method));

	pValue = cJSON_GetObjectItem(pJson, "cache");
	if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
		option.fileCache = cJSON_GetNumberValue(pValue);
	} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
		option.fileCache = atoi(cJSON_GetStringValue(pValue));
	}

	pValue = cJSON_GetObjectItem(pJson, "binary");
	if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
		option.binary = cJSON_GetNumberValue(pValue);
	} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
		option.binary = atoi(cJSON_GetStringValue(pValue));
	}
	crc = crc32(crc, reinterpret_cast<Bytef*>(&option.binary), sizeof(option.binary));

	pValue = cJSON_GetObjectItem(pJson, "mode");
	if ((pValue != NULL) && cJSON_IsString(pValue)) {
		strcpy(option.route_mode, cJSON_GetStringValue(pValue));
	}

	pValue = cJSON_GetObjectItem(pJson, "route_cost");
	if ((pValue != NULL) && cJSON_IsString(pValue)) {
		strcpy(option.route_cost, cJSON_GetStringValue(pValue));
	}

	pValue = cJSON_GetObjectItem(pJson, "mobility");
	if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
		option.mobility = cJSON_GetNumberValue(pValue);
	} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
		if ((strcmp("motor", cJSON_GetStringValue(pValue)) == 0) ||
			(strcmp("car", cJSON_GetStringValue(pValue)) == 0) ||
			(strcmp("vehicle", cJSON_GetStringValue(pValue)) == 0)) { // 일반 자동차
			option.mobility = TYPE_MOBILITY_VEHICLE;
		} else if ((strcmp("truck", cJSON_GetStringValue(pValue)) == 0) ||
			(strcmp("bus", cJSON_GetStringValue(pValue)) == 0)) { // 트럭
			option.mobility = TYPE_MOBILITY_TRUCK;
		} else if ((strcmp("emergency", cJSON_GetStringValue(pValue)) == 0) ||
			(strcmp("fire", cJSON_GetStringValue(pValue)) == 0) ||
			(strcmp("police", cJSON_GetStringValue(pValue)) == 0) ||
			(strcmp("embulance", cJSON_GetStringValue(pValue)) == 0)) { // 긴급
			option.mobility = TYPE_MOBILITY_EMERGENCY;
		}
	}
	if (option.mobility >= TYPE_MOBILITY_COUNT) {
		LOG_TRACE(LOG_ERROR, "input mobility type not defined, type:%d", option.mobility);
		option.mobility = TYPE_MOBILITY_VEHICLE;
	}
	crc = crc32(crc, reinterpret_cast<Bytef*>(&option.mobility), sizeof(option.mobility));

	pValue = cJSON_GetObjectItem(pJson, "truck_option");
	if (pValue != nullptr) {
		cJSON* pTruck = cJSON_GetObjectItem(pValue, "height");
		if ((pTruck != NULL) && cJSON_IsNumber(pTruck)) {
			option.truck.height = cJSON_GetNumberValue(pTruck);
		} else if ((pTruck != NULL) && cJSON_IsString(pTruck)) {
			option.truck.height = atoi(cJSON_GetStringValue(pTruck));
		}

		pTruck = cJSON_GetObjectItem(pValue, "weight");
		if ((pTruck != NULL) && cJSON_IsNumber(pTruck)) {
			option.truck.weight = cJSON_GetNumberValue(pTruck);
		} else if ((pTruck != NULL) && cJSON_IsString(pTruck)) {
			option.truck.weight = atoi(cJSON_GetStringValue(pTruck));
		}

		pTruck = cJSON_GetObjectItem(pValue, "length");
		if ((pTruck != NULL) && cJSON_IsNumber(pTruck)) {
			option.truck.length = cJSON_GetNumberValue(pTruck);
		} else if ((pTruck != NULL) && cJSON_IsString(pTruck)) {
			option.truck.length = atoi(cJSON_GetStringValue(pTruck));
		}

		pTruck = cJSON_GetObjectItem(pValue, "width");
		if ((pTruck != NULL) && cJSON_IsNumber(pTruck)) {
			option.truck.width = cJSON_GetNumberValue(pTruck);
		} else if ((pTruck != NULL) && cJSON_IsString(pTruck)) {
			option.truck.width = atoi(cJSON_GetStringValue(pTruck));
		}

		pTruck = cJSON_GetObjectItem(pValue, "hazardous");
		if ((pTruck != NULL) && cJSON_IsNumber(pTruck)) {
			option.truck.hazardous = cJSON_GetNumberValue(pTruck);
		} else if ((pTruck != NULL) && cJSON_IsString(pTruck)) {
			option.truck.hazardous = atoi(cJSON_GetStringValue(pTruck));
		}
	}

	return crc;
}


int32_t getRequestOrigin(IN const cJSON* pJson, IN const uint32_t nCrc, OUT vector<Origins>& vtOrigin, OUT vector<Origins>& vtDestination)
{
	int32_t crc = nCrc;

	// origins
	cJSON* pValue;
	cJSON* pOrigins = cJSON_GetObjectItem(pJson, "origins");
	if (pOrigins != NULL) {
		int nOrigins = cJSON_GetArraySize(pOrigins);
		if (nOrigins > 0) {
			cJSON* pCoord;
			string strCoord;
			Origins origin;
			for (int ii = 0; ii < nOrigins; ii++) {
				pCoord = cJSON_GetArrayItem(pOrigins, ii);
				if (pCoord != NULL) {
					if (cJSON_IsArray(pCoord) && cJSON_GetArraySize(pCoord) == 2) {
						pValue = cJSON_GetArrayItem(pCoord, 0);
						origin.position.x = cJSON_GetNumberValue(pValue);
						pValue = cJSON_GetArrayItem(pCoord, 1);
						origin.position.y = cJSON_GetNumberValue(pValue);
					} else if (cJSON_IsString(pCoord)) { // string
						strCoord = cJSON_GetStringValue(pCoord);
						origin.position = getCoordFromText(strCoord.c_str());
					} else {
						LOG_TRACE(LOG_ERROR, "input origin data issue, idx:%d", ii);
						continue;
					}
					vtOrigin.emplace_back(origin);

					crc = crc32(crc, reinterpret_cast<Bytef*>(&origin.position.x), sizeof(origin.position.x));
					crc = crc32(crc, reinterpret_cast<Bytef*>(&origin.position.y), sizeof(origin.position.y));
				}
			} // for
		}
	}

	// destinations
	cJSON* pDestinations = cJSON_GetObjectItem(pJson, "destinations");
	if (pDestinations != NULL) {
		int nDestinations = cJSON_GetArraySize(pDestinations);
		if (nDestinations > 0) {
			cJSON* pCoord;
			string strCoord;
			Origins destination;
			for (int ii = 0; ii < nDestinations; ii++) {
				pCoord = cJSON_GetArrayItem(pDestinations, ii);
				if (pCoord != NULL) {
					if (cJSON_IsArray(pCoord) && cJSON_GetArraySize(pCoord) == 2) {
						pValue = cJSON_GetArrayItem(pCoord, 0);
						destination.position.x = cJSON_GetNumberValue(pValue);
						pValue = cJSON_GetArrayItem(pCoord, 1);
						destination.position.y = cJSON_GetNumberValue(pValue);
					} else if (cJSON_IsString(pCoord)) { // string
						strCoord = cJSON_GetStringValue(pCoord);
						destination.position = getCoordFromText(strCoord.c_str());
					} else {
						LOG_TRACE(LOG_ERROR, "input destination data issue, idx:%d", ii);
						continue;
					}
					vtDestination.emplace_back(destination);

					crc = crc32(crc, reinterpret_cast<Bytef*>(&destination.position.x), sizeof(destination.position.x));
					crc = crc32(crc, reinterpret_cast<Bytef*>(&destination.position.y), sizeof(destination.position.y));
				}
			} // for
		}
	}

	// 방문지 체류시간
	cJSON* pLayoutTimes = cJSON_GetObjectItem(pJson, "layover_times");
	if (pLayoutTimes != NULL) {
		int nTimes = cJSON_GetArraySize(pLayoutTimes);
		nTimes = min(nTimes, static_cast<int>(vtOrigin.size())); // 방문지보다 적거나 같아야 함
		if (nTimes > 0) {
			cJSON* pTime;
			string strTime;
			int nTime;
			for (int ii = 0; ii < nTimes; ii++) {
				pTime = cJSON_GetArrayItem(pLayoutTimes, ii);
				if (pTime != NULL) {
					if (cJSON_IsNumber(pTime)) { // number
						nTime = cJSON_GetNumberValue(pTime);
					} else if (cJSON_IsString(pTime)) { // string
						nTime = atoi(cJSON_GetStringValue(pTime));
					} else {
						LOG_TRACE(LOG_ERROR, "input time data issue, idx:%d", ii);
						continue;
					}
					vtOrigin.at(ii).layoverTime = nTime * 60; // min to sec
				}
			} // for
		}
	}

	// 방문지 화물
	cJSON* pCargoVolume = cJSON_GetObjectItem(pJson, "cargo_volume");
	if (pCargoVolume != NULL) {
		int nCargo = cJSON_GetArraySize(pCargoVolume);
		nCargo = min(nCargo, static_cast<int>(vtOrigin.size())); // 방문지보다 적거나 같아야 함
		if (nCargo > 0) {
			cJSON* pCargo;
			string strCargo;
			int nValue;
			for (int ii = 0; ii < nCargo; ii++) {
				pCargo = cJSON_GetArrayItem(pCargoVolume, ii);
				if (pCargo != NULL) {
					if (cJSON_IsNumber(pCargo)) { // number
						nValue = cJSON_GetNumberValue(pCargo);
					} else if (cJSON_IsString(pCargo)) { // string
						nValue = atoi(cJSON_GetStringValue(pCargo));
					} else {
						LOG_TRACE(LOG_ERROR, "input cargo data issue, idx:%d", ii);
						continue;
					}
					vtOrigin.at(ii).cargoVolume = nValue;
				}
			} // for
		}
	}

	return crc;
}


int32_t getRequestAdditional(IN const cJSON* pJson, IN const uint32_t nCrc, OUT vector<Origins>& vtOrigin)
{
	int32_t crc = nCrc;

	// origins
	int32_t nMaxSize = vtOrigin.size();
	int32_t nValue;
	cJSON* pValue;
	cJSON* pAdditional = cJSON_GetObjectItem(pJson, "additional");
	if (pAdditional != NULL) {
		pValue = cJSON_GetObjectItem(pAdditional, "count");
		int nSize = cJSON_GetArraySize(pValue);
		int nCount = min(nSize, nMaxSize);
		for (int ii = 0; ii < nCount; ii++) {
			cJSON* pItem = cJSON_GetArrayItem(pValue, ii);
			if (pItem != NULL) {
				if (cJSON_IsNumber(pItem)) {
					nValue = cJSON_GetNumberValue(pItem);
				} else if (cJSON_IsString(pItem)) { // string
					nValue = atoi(cJSON_GetStringValue(pItem));
				} else {
					LOG_TRACE(LOG_ERROR, "input additional data issue, idx:%d", ii);
					continue;
				}
				vtOrigin[ii].count = nValue;
			}
		} // for

		pValue = cJSON_GetObjectItem(pAdditional, "weight");
		nSize = cJSON_GetArraySize(pValue);
		nCount = min(nSize, nMaxSize);
		for (int ii = 0; ii < nCount; ii++) {
			cJSON* pItem = cJSON_GetArrayItem(pValue, ii);
			if (pItem != NULL) {
				if (cJSON_IsNumber(pItem)) {
					nValue = cJSON_GetNumberValue(pItem);
				} else if (cJSON_IsString(pItem)) { // string
					nValue = atoi(cJSON_GetStringValue(pItem));
				} else {
					LOG_TRACE(LOG_ERROR, "input additional data issue, idx:%d", ii);
					continue;
				}
				vtOrigin[ii].weight = nValue;
			}
		} // for

		pValue = cJSON_GetObjectItem(pAdditional, "size");
		nSize = cJSON_GetArraySize(pValue);
		nCount = min(nSize, nMaxSize);
		for (int ii = 0; ii < nCount; ii++) {
			cJSON* pItem = cJSON_GetArrayItem(pValue, ii);
			if (pItem != NULL) {
				if (cJSON_IsNumber(pItem)) {
					nValue = cJSON_GetNumberValue(pItem);
				} else if (cJSON_IsString(pItem)) { // string
					nValue = atoi(cJSON_GetStringValue(pItem));
				} else {
					LOG_TRACE(LOG_ERROR, "input additional data issue, idx:%d", ii);
					continue;
				}
				vtOrigin[ii].size = nValue;
			}
		} // for
	}

	return crc;
}

#if 1
int32_t getRequestWeightMatrixInfo(IN const cJSON* pJson, IN const char* pszType, IN const char* pszPath, OUT FILE_INFO_RDM& fileInfo)
//IN const cJSON* pJson, IN const char* pszDataPath, OUT BaseOption& option, OUT vector<Origins>& vtOrigin, OUT vector<vector<stDistMatrix>>& vtDistMatrix, OUT int32_t& typeDistMatrix)
{
	int32_t ret = RESULT_FAILED;

	char szBuff[MAX_PATH] = { 0, };
	cJSON* pValue;
	cJSON* pRaw = cJSON_GetObjectItem(pJson, pszType);
	if (pRaw != NULL) {
		pValue = cJSON_GetObjectItem(pRaw, "type");
		if ((pValue != NULL) && cJSON_IsString(pValue)) {
			strcpy(szBuff, cJSON_GetStringValue(pValue));
			strlower(szBuff);
			fileInfo.type = szBuff;
		}
		pValue = cJSON_GetObjectItem(pRaw, "format");
		if ((pValue != NULL) && cJSON_IsString(pValue)) {
			strcpy(szBuff, cJSON_GetStringValue(pValue));
			strlower(szBuff);
			fileInfo.format = szBuff;
		}
		pValue = cJSON_GetObjectItem(pRaw, "size");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			fileInfo.size = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			fileInfo.size = atoi(cJSON_GetStringValue(pValue));
		}
		pValue = cJSON_GetObjectItem(pRaw, "name");
		if ((pValue != NULL) && cJSON_IsString(pValue)) {
			fileInfo.name = cJSON_GetStringValue(pValue);
		}
		pValue = cJSON_GetObjectItem(pRaw, "data");
		if ((pValue != NULL) && cJSON_IsString(pValue)) {
			fileInfo.data = cJSON_GetStringValue(pValue);
		}
	}

	if (fileInfo.size > 0) {		
		BYTE* pszBinary = nullptr;
		size_t sizeRaw = 0;

		if ((fileInfo.type.compare("base64") == 0) && !fileInfo.data.empty()) {
			// base to binary
			sizeRaw = fileInfo.data.size();
			if (sizeRaw > 0) {
				pszBinary = new BYTE[sizeRaw];
				sizeRaw = base64toBinary(fileInfo.data.c_str(), fileInfo.data.size(), pszBinary);
			}
		} else if ((fileInfo.type.compare("file") == 0) && !fileInfo.name.empty()) {
			// loading from file
			string strFilePath = pszPath + fileInfo.name;
			FILE* fp = fopen(strFilePath.c_str(), "rb");
			if (fp) {
				sizeRaw = fileInfo.size;

				size_t sizeRead = 0;
				fseek(fp, 0, SEEK_END);
				size_t sizeFile = ftell(fp);
				fseek(fp, 0, SEEK_SET);

				if (sizeFile > 0 && (sizeFile == fileInfo.size)) {
					pszBinary = new BYTE[sizeFile];
					for (; sizeRead - sizeFile > 0;) {
						sizeRead += fread(pszBinary, 1, sizeFile - sizeRead, fp);
					}
				} else {
					LOG_TRACE(LOG_WARNING, "getRequestWeightMatrixInfo file size not same with req size, %d vs %d", sizeFile, fileInfo.size);
					return ret;
				}
				fclose(fp);
			} // fp
		} else {
			LOG_TRACE(LOG_WARNING, "undefined file type, %s", fileInfo.type.c_str());
			return ret;
		}

		
		if (fileInfo.format.compare("zip") == 0) {
			// unzip
			size_t sizeUncomp = fileInfo.size * 1.2;
			Bytef* pszUncompress = new Bytef[sizeUncomp];
			int retUnzip = uncompress((Bytef*)pszUncompress, (uLongf*)&sizeUncomp, (Bytef*)pszBinary, sizeRaw);
			if (retUnzip == Z_OK) {
				if (fileInfo.size != sizeUncomp) {
					LOG_TRACE(LOG_WARNING, "received binary data size not match with uncompressed data size, recv:%d vs uncomp:%d", fileInfo.size, sizeUncomp);
				}
			} else {
				LOG_TRACE(LOG_WARNING, "failed, uncompress rdm data, recv:%d vs uncomp:%d", fileInfo.size, sizeUncomp);
			}

			if (pszBinary && (sizeUncomp > 0)) {
				fileInfo.data.clear();
				fileInfo.data.assign(reinterpret_cast<const char*>(pszBinary), sizeUncomp);
			}
		} else if (fileInfo.format.compare("bin") == 0) {
			// copy
			fileInfo.data.clear();
			fileInfo.data.assign(reinterpret_cast<const char*>(pszBinary), fileInfo.size);
		} else {
			LOG_TRACE(LOG_WARNING, "undefined file type, %s", fileInfo.type.c_str());
		}

		SAFE_DELETE_ARR(pszBinary);

		ret = RESULT_OK;
	}

	return ret;
}
#else
uint32_t getRequestDistanceMatrix(IN const cJSON* pJson, IN const char* pszDataPath, OUT BaseOption& option, OUT vector<Origins>& vtOrigin, OUT vector<vector<stDistMatrix>>& vtDistMatrix, OUT int32_t& typeDistMatrix)
{
	char szType[32] = { 0, };
	char szForm[32] = { 0, };
	uint32_t sizeData = 0;
	string strData;

	cJSON* pValue;
	cJSON* pRaw = cJSON_GetObjectItem(pJson, "rdm");
	if (pRaw != NULL) {
		pValue = cJSON_GetObjectItem(pRaw, "type");
		if ((pValue != NULL) && cJSON_IsString(pValue)) {
			strcpy(szType, cJSON_GetStringValue(pValue));
			strlower(szType);
		}
		pValue = cJSON_GetObjectItem(pRaw, "form");
		if ((pValue != NULL) && cJSON_IsString(pValue)) {
			strcpy(szForm, cJSON_GetStringValue(pValue));
			strlower(szForm);
		}
		pValue = cJSON_GetObjectItem(pRaw, "size");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			sizeData = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			sizeData = atoi(cJSON_GetStringValue(pValue));
		}
		pValue = cJSON_GetObjectItem(pRaw, "data");
		if ((pValue != NULL) && cJSON_IsString(pValue)) {
			strData = cJSON_GetStringValue(pValue);
		}
	}

	if ((sizeData > 0) && !strData.empty()) {
		BYTE* pszBinary = nullptr;
		size_t sizeBin = 0;
		// base to binary

		if (sizeData > 0) {
			if (strcmp(szType, "base64") == 0) {
				sizeBin = strData.size();
				if (sizeBin > 0) {
					pszBinary = new BYTE[sizeBin];
					sizeBin = base64toBinary(strData.c_str(), strData.size(), pszBinary);
				}
			} else if (strcmp(szType, "file") == 0) {
				if (!strData.empty()) {
					string strFilePath = pszDataPath;
					strFilePath.append("/usr/rdm/");
					strFilePath.append(strData);

					FILE* fp = fopen(strFilePath.c_str(), "rb");
					if (fp) {
						fseek(fp, 0, SEEK_END);
						sizeBin = ftell(fp);
						fseek(fp, 0, SEEK_SET);
						if (sizeBin == sizeData) {
							pszBinary = new BYTE[sizeBin];
							size_t readed = 0;
							while (readed < sizeData) {
								readed += fread(&pszBinary[readed], 1, sizeData - readed, fp);
							}
							fclose(fp);
						}
					}
				}
			}

			if (strcmp(szForm, "zip") == 0) {
				int sizeUncomp = sizeData * 1.2;
				Bytef* pszUncompress = new Bytef[sizeUncomp];
				int retUnzip = uncompress((Bytef*)pszUncompress, (uLongf*)&sizeUncomp, (Bytef*)pszBinary, sizeBin);
				if (retUnzip == Z_OK) {
					if (sizeData != sizeUncomp) {
						LOG_TRACE(LOG_WARNING, "received binary data size not match with uncompressed data size, recv:%d vs uncomp:%d", sizeData, sizeUncomp);
					}
				} else {
					LOG_TRACE(LOG_WARNING, "failed, uncompress rdm data, recv:%d vs uncomp:%d", sizeData, sizeUncomp);
				}

				if (pszBinary) {
					SAFE_DELETE_ARR(pszBinary);
				}
				pszBinary = pszUncompress;
			} else if (strcmp(szForm, "bin") == 0) {
				/*int sizeUncomp = sizeData * 1.2;
				Bytef* pszUncompress = new Bytef[sizeUncomp];
				int retUnzip = uncompress((Bytef*)pszUncompress, (uLongf*)&sizeUncomp, (Bytef*)pszBinary, sizeBin);
				if (retUnzip == Z_OK) {
					if (sizeData != sizeUncomp) {
						LOG_TRACE(LOG_WARNING, "received binary data size not match with uncompressed data size, recv:%d vs uncomp:%d", sizeData, sizeUncomp);
					}
				} else {
					LOG_TRACE(LOG_WARNING, "failed, uncompress rdm data, recv:%d vs uncomp:%d", sizeData, sizeUncomp);
				}

				if (pszBinary) {
					SAFE_DELETE_ARR(pszBinary);
				}
				pszBinary = pszUncompress;*/
			}

			const size_t sizeItem = sizeof(stDistMatrix); // sizeof(stDistMatrix::nTotalDist) + sizeof(stDistMatrix::nTotalTime) + sizeof(stDistMatrix::dbTotalCost);
			vtOrigin.clear();
			if (RESULT_OK == readWeightMatrix(pszBinary, sizeBin, option, vtOrigin, vtDistMatrix)) {
				if (strcmp(szType, "file") == 0) {
					typeDistMatrix = 1; // 서버 파일 데이터
				} else if (strcmp(szType, "base64") == 0) {
					typeDistMatrix = 2; // 사용자 데이터
				} else {
					typeDistMatrix = 0; // 엔진 생성 데이터
				}
			}
		}

		if (pszBinary) {
			SAFE_DELETE_ARR(pszBinary);
		}
	}

	return vtDistMatrix.size();
}
#endif


int32_t getRequestTspOption(const cJSON* pJson, const uint32_t nCrc, OUT TspOption& tspOpt)
{
	int32_t crc = nCrc;

	// tsp
	cJSON* pValue;
	cJSON* pTsp = cJSON_GetObjectItem(pJson, "tsp");
	if (pTsp != NULL) {
		pValue = cJSON_GetObjectItem(pTsp, "seed");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			tspOpt.seed = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			tspOpt.seed = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pTsp, "compare_type");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			tspOpt.compareType = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			tspOpt.compareType = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pTsp, "algorithm");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			tspOpt.algorithm = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			tspOpt.algorithm = atoi(cJSON_GetStringValue(pValue));
		}
	} else {
		tspOpt.seed = 10000;
		tspOpt.compareType = TYPE_TSP_VALUE_DIST;
		tspOpt.algorithm = TYPE_TSP_ALGORITHM_TSP_GA;
	}


	// option
	cJSON* pOption = cJSON_GetObjectItem(pJson, "option");
	if (pOption != NULL) {
		pValue = cJSON_GetObjectItem(pOption, "endpoint_type"); // 지점 고정
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			tspOpt.endpointType = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			tspOpt.endpointType = atoi(cJSON_GetStringValue(pValue));
		} else {
			pValue = cJSON_GetObjectItem(pOption, "position_lock"); // endpoint_type변경, 기존 버전 안정화 후 삭제 예정
			if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
				tspOpt.endpointType = cJSON_GetNumberValue(pValue);
			} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
				tspOpt.endpointType = atoi(cJSON_GetStringValue(pValue));
			}
		}
	}

	return crc;
}


int32_t getRequestCluster(const cJSON* pJson, const uint32_t nCrc, OUT ClusteringOption& clustOpt)
{
	int32_t crc = nCrc;

	// cluster
	cJSON* pValue;
	cJSON* pClust = cJSON_GetObjectItem(pJson, "clust");
	if (pClust != NULL) {
		pValue = cJSON_GetObjectItem(pClust, "seed");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.seed = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.seed = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pClust, "compare_type");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.compareType = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.compareType = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pClust, "algorithm");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.algorithm = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.algorithm = atoi(cJSON_GetStringValue(pValue));
		}
	} else {
		clustOpt.seed = 10006;
		clustOpt.compareType = TYPE_TSP_VALUE_DIST;
		clustOpt.algorithm = TYPE_TSP_ALGORITHM_TSP;
	}


	// option
	cJSON* pOption = cJSON_GetObjectItem(pJson, "option");
	if (pOption != NULL) {
		pValue = cJSON_GetObjectItem(pOption, "division_type"); // 분배 방식
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.divisionType = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.divisionType = atoi(cJSON_GetStringValue(pValue));
		}

		cJSON* pValue = cJSON_GetObjectItem(pOption, "limit_cluster"); // 최대 분배 가능 클러스터
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.limitCluster = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.limitCluster = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pOption, "limit_value"); // 차량당 최대 운행 가능 거리
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.limitValue = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.limitValue = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pOption, "limit_deviation"); // 차량당 최대 운행 정보 편차
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.limitDeviation = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.limitDeviation = atoi(cJSON_GetStringValue(pValue));
		}

		if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_TIME) {
			// 시간균등일 경우, 입력된 시간(분)을 초 단위로 변경
			clustOpt.limitValue *= 60;
			clustOpt.limitDeviation *= 60;
		}

		pValue = cJSON_GetObjectItem(pOption, "max_spot"); // 차량당 최대 운송 가능 지점
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.max_spot = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.max_spot = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pOption, "max_distance"); // 차량당 최대 운행 가능 거리
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.max_distance = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.max_distance = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pOption, "max_time"); // 차량당 최대 운행 가능 시간
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.max_time = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.max_time = atoi(cJSON_GetStringValue(pValue));
		}
		clustOpt.max_time *= 60; // 입력된 시간(분)을 초 단위로 변경

		pValue = cJSON_GetObjectItem(pOption, "max_cargo"); // 차량당 최대 화물 수량
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.max_cargo = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.max_cargo = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pOption, "reservation"); // 예약 시간
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.reservation = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.reservation = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pOption, "reservation_type"); // 예약 시간 타입
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.reservationType = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.reservationType = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pOption, "endpoint_type"); // 지점 고정
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.endpointType = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.endpointType = atoi(cJSON_GetStringValue(pValue));
		} else {
			pValue = cJSON_GetObjectItem(pOption, "position_lock"); // endpoint_type변경, 기존 버전 안정화 후 삭제 예정
			if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
				clustOpt.endpointType = clustOpt.endpointType = cJSON_GetNumberValue(pValue);
			} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
				clustOpt.endpointType = clustOpt.endpointType = atoi(cJSON_GetStringValue(pValue));
			}
		}

		pValue = cJSON_GetObjectItem(pOption, "additional_type"); // 추가 지점 속성
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.additionalType = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.additionalType = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pOption, "additional_limit"); // 추가 지점 속성 한도
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.additionalLimit = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.additionalLimit = atoi(cJSON_GetStringValue(pValue));
		}
	}

	return crc;
}


int32_t getRequestGroupOption(const cJSON* pJson, const uint32_t nCrc, OUT ClusteringOption& clustOpt)
{
	int32_t crc = nCrc;

	// cluster
	cJSON* pValue;
	cJSON* pClust = cJSON_GetObjectItem(pJson, "clust");
	if (pClust != NULL) {
		pValue = cJSON_GetObjectItem(pClust, "seed");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.seed = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.seed = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pClust, "compare_type");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.compareType = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.compareType = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pClust, "algorithm");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.algorithm = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.algorithm = atoi(cJSON_GetStringValue(pValue));
		}
	} else {
		clustOpt.seed = 10006;
		clustOpt.compareType = TYPE_TSP_VALUE_DIST;
		clustOpt.algorithm = TYPE_TSP_ALGORITHM_TSP;
	}


	// option
	cJSON* pOption = cJSON_GetObjectItem(pJson, "option");
	if (pOption != NULL) {
		pValue = cJSON_GetObjectItem(pOption, "division_type"); // 분배 방식
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.bylink.divisionType = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.bylink.divisionType = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(pClust, "limit_length");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			clustOpt.bylink.limitLength = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			clustOpt.bylink.limitLength = atoi(cJSON_GetStringValue(pValue));
		}
	}

	return crc;
}


int32_t CTMSManager::ParsingRequestBaseOption(IN const char* szRequest, OUT BaseOption& option)
{
	int32_t crc = 0;

#if defined(USE_CJSON)
	cJSON* root = cJSON_Parse(szRequest);
	if (root != NULL) {
		crc = getRequestBaseOption(root, crc, option);

		cJSON_Delete(root);
	}	
#endif

	return crc;
}


int32_t CTMSManager::ParsingRequestRoute(IN const char* szRequest, OUT BaseOption& baseOpt, OUT vector<Origins>& vtOrigin)
{
	int32_t ret = RESULT_FAILED;

	int32_t crc = 0;

#if defined(USE_CJSON)
	cJSON* root = cJSON_Parse(szRequest);
	if (root != NULL) {
		crc = getRequestBaseOption(root, crc, baseOpt);

		vector<Origins> vtDestination;
		crc = getRequestOrigin(root, crc, vtOrigin, vtDestination);

		crc = getRequestAdditional(root, crc, vtOrigin);

		cJSON_Delete(root);
	}
#endif

	ret = RESULT_OK;

	return ret;
}


int32_t CTMSManager::ParsingRequestWeightMatrix(IN const char* szRequest, OUT BaseOption& option, OUT vector<Origins>& vtOrigin, OUT vector<Origins>& vtDestination, OUT vector<vector<stDistMatrix>>& vtDistanceMatrix, OUT int32_t& typeDistMatrix)
{
	int32_t ret = RESULT_FAILED;

#if defined(USE_CJSON)
	cJSON* root = cJSON_Parse(szRequest);
	if (root != NULL) {
		// check origin
		int crc = getRequestBaseOption(root, 0, option);
		crc = getRequestOrigin(root, crc, vtOrigin, vtDestination);

		if (!vtOrigin.empty()) {
			ret = RESULT_OK;
		}

		cJSON_Delete(root);
	} // cJSON
#endif // #if defined(USE_CJSON)

	return ret;
}


int32_t CTMSManager::ParsingRequestWeightMatrixRoute(IN const char* szRequest, OUT BaseOption& option, OUT vector<Origins>& vtOrigin, OUT vector<Origins>& vtDestination, OUT vector<vector<stDistMatrix>>& vtDistanceMatrix/*, OUT int32_t& typeDistMatrix*/)
{
	int32_t ret = RESULT_FAILED;

#if defined(USE_CJSON)
	cJSON* root = cJSON_Parse(szRequest);
	if (root != NULL) {
		// check rdm field
		FILE_INFO_RDM fileInfo;
		string strPath = m_pDataMgr->GetDataPath() + string("/usr/rdm/");
		if (getRequestWeightMatrixInfo(root, "rdm", strPath.c_str(), fileInfo) == RESULT_OK) {
			ret = readWeightMatrix((BYTE*)fileInfo.data.c_str(), fileInfo.size, option, vtOrigin, vtDistanceMatrix);
		}

		cJSON_Delete(root);
	} // cJSON
#endif // #if defined(USE_CJSON)

	return ret;
}


int32_t CTMSManager::ParsingRequestWeightMatrixRouteLine(IN const char* szRequest, OUT string& fileName, OUT size_t& fileSize, OUT vector<vector<FileIndex>>& vtPathMatrixIndex)
{
	int32_t ret = RESULT_FAILED;

#if defined(USE_CJSON)
	cJSON* root = cJSON_Parse(szRequest);
	if (root != NULL) {
		// check rdm field
		FILE_INFO_RDM fileInfo;
		string strPath = m_pDataMgr->GetDataPath() + string("/usr/rdm/");
		if (getRequestWeightMatrixInfo(root, "rln", strPath.c_str(), fileInfo) == RESULT_OK) {
			ret = readWeightMatrixRouteLine((BYTE*)fileInfo.data.c_str(), fileInfo.size, vtPathMatrixIndex);

			if (ret == RESULT_OK) {
				fileName = fileInfo.name;
				fileSize = fileInfo.size;
			}
		}

		cJSON_Delete(root);
	} // cJSON
#endif // #if defined(USE_CJSON)

	return ret;
}


int32_t CTMSManager::ParsingRequestBestway(IN const char* szRequest, OUT TspOption& tspOpt, OUT vector<Origins>& vtOrigin)
{
	int32_t crc = 0;

#if defined(USE_CJSON)
	cJSON* root = cJSON_Parse(szRequest);
	if (root != NULL) {
		if (vtOrigin.empty()) {
			// base
			crc = getRequestBaseOption(root, crc, tspOpt.baseOption);

			// origins
			vector<Origins> vtDestination;
			crc = getRequestOrigin(root, crc, vtOrigin, vtDestination);
		}

		// tsp
		tspOpt.geneSize = vtOrigin.size();
		crc = getRequestTspOption(root, crc, tspOpt);

		cJSON_Delete(root);
	} // cJSON
#endif // #if defined(USE_CJSON)

	return crc;
}


int32_t CTMSManager::ParsingRequestCluster(IN const char* szRequest, OUT ClusteringOption& clustOpt, OUT vector<Origins>& vtOrigin)
{
	int32_t crc = 0;

#if defined(USE_CJSON)
	cJSON* root = cJSON_Parse(szRequest);
	if (root != NULL) {
		if (vtOrigin.empty()) {
			// base
			crc = getRequestBaseOption(root, crc, clustOpt.tspOption.baseOption);

			// origins
			vector<Origins> vtDestination;
			crc = getRequestOrigin(root, crc, vtOrigin, vtDestination);
		}

		// tsp
		crc = getRequestTspOption(root, crc, clustOpt.tspOption);

		// cluster
		crc = getRequestCluster(root, crc, clustOpt);

		// additional
		if (clustOpt.additionalType != 0) {
			crc = getRequestAdditional(root, crc, vtOrigin);
		}

		cJSON_Delete(root);
	} // cJSON
#endif // #if defined(USE_CJSON)

	return crc;
}


int32_t CTMSManager::ParsingRequestGroup(IN const char* szRequest, OUT ClusteringOption& clustOpt, OUT vector<Origins>& vtOrigin)
{
	int32_t crc = 0;

#if defined(USE_CJSON)
	cJSON* root = cJSON_Parse(szRequest);
	if (root != NULL) {
		// base
		crc = getRequestBaseOption(root, crc, clustOpt.tspOption.baseOption);

		if (vtOrigin.empty()) {
			// origins
			vector<Origins> vtDestination;
			crc = getRequestOrigin(root, crc, vtOrigin, vtDestination);
		}

		// tsp
		crc = getRequestTspOption(root, crc, clustOpt.tspOption);

		// group
		crc = getRequestGroupOption(root, crc, clustOpt);

		cJSON_Delete(root);
	} // cJSON
#endif // #if defined(USE_CJSON)

	return crc;
}


//int32_t CTMSManager::LoadWeightMatrix(IN const char* szFileName, IN const size_t sizeFile, OUT BaseOption& option, OUT vector<Origins>& vtOrigin, OUT vector<vector<stDistMatrix>>& vtDistMatrix)
//{
//	int32_t ret = -1;
//
//	if (szFileName == nullptr) {
//		return ret;
//	}
//
//	FILE* fp = fopen(szFileName, "rb");
//	if (fp) {
//		size_t sizeRead = 0;
//		size_t sizeFile = fseek(fp, 0, SEEK_END);
//		fseek(fp, 0, SEEK_SET);
//
//		BYTE* pszBinary = new BYTE[sizeFile];
//		for (; sizeRead - sizeFile > 0;) {
//			sizeRead += fread(pszBinary, 1, sizeFile - sizeRead, fp);
//		}
//		fclose(fp);
//
//		if (sizeRead == sizeFile) {
//			ret = readWeightMatrix(pszBinary, sizeFile, option, vtOrigin, vtDistMatrix);
//		}
//
//		if (pszBinary) {
//			SAFE_DELETE_ARR(pszBinary);
//		}
//
//		fclose(fp);
//	} // fp
//
//	return ret;
//}
//
//
//int32_t CTMSManager::LoadWeightMatrixRouteLine(IN const char* szFileName, IN const size_t sizeFile, OUT vector<vector<FileIndex>>& vtPathMatrixIndex)
//{
//	int32_t ret = RESULT_FAILED;
//
//	if (szFileName == nullptr || strlen(szFileName) <=0 || sizeFile == 0) {
//		return ret;
//	}
//
//	FILE* fp = fopen(szFileName, "rb");
//	if (fp) {
//		size_t sizeRead = 0;
//		fseek(fp, 0, SEEK_END);
//		size_t szFile = ftell(fp);
//		fseek(fp, 0, SEEK_SET);
//
//		if (szFile == sizeFile) {			
//			ret = readWeightMatrixRouteLine(fp, sizeFile, vtPathMatrixIndex);
//		}
//
//		fclose(fp);
//	} // fp
//
//	return ret;
//}


int32_t CTMSManager::SaveWeightMatrix(IN const char* szFileName, IN const BaseOption* pOption, IN const int cntItem, IN const int sizeItem, IN const uint32_t crc, IN const vector<Origins>& vtOrigin, IN const vector<vector<stDistMatrix>>& vtDistMatrix)
{
	int32_t ret = RESULT_FAILED;

	if ((szFileName == nullptr || strlen(szFileName) <= 0) || vtDistMatrix.empty()) {
		return ret;
	}

	checkDirectory(szFileName);

	FILE* fp = fopen(szFileName, "wb");
	if (!fp) {
		return ret;
	}

	int nCount = vtDistMatrix.size();

	size_t sizeFileOffset = 0;

	// base
	FileBase base;
	strcpy(base.szType, "RDM");
	base.version[0] = 0;
	base.version[1] = 0;
	base.version[2] = 1;
	base.version[3] = 0;
	base.szHeader = sizeof(FileHeaderRDM);
	sizeFileOffset += fwrite(&base, 1, sizeof(base), fp);

	// rdm header
	FileHeaderRDM header;
	header.crcData = crc;
	header.cntItem = cntItem;
	header.offOption = sizeof(FileBase) + sizeof(FileHeaderRDM);
	header.offOrigin = header.offOption + sizeof(BaseOption);
	header.offBody = header.offOrigin + (sizeof(Origins) * cntItem);
	sizeFileOffset += fwrite(&header, 1, sizeof(header), fp);

	// rdm option
	sizeFileOffset += fwrite(pOption, 1, sizeof(BaseOption), fp);

	// rdm origin
	sizeFileOffset += fwrite(&vtOrigin.front(), 1, sizeof(Origins) * cntItem, fp);

	// rdm body
	// for route line info table
	// write matrix
	for (int ii = 0; ii < cntItem; ii++) {
		for (int jj = 0; jj < cntItem; jj++) {
			sizeFileOffset += fwrite(&vtDistMatrix[ii][jj], 1, sizeItem, fp);
		} // for
	} // for

	fclose(fp);

	ret = RESULT_OK;

	return ret;
}


int32_t CTMSManager::SaveWeightMatrixRouteLine(IN const char* szFileName, IN const vector<vector<stPathMatrix>>& vtPathMatrix)
{
	int32_t ret = RESULT_FAILED;

	if (szFileName == nullptr || strlen(szFileName) <= 0) {
		return ret;
	}

	checkDirectory(szFileName);

	FILE* fp = fopen(szFileName, "wb");
	if (!fp) {
		return ret;
	}

	int nCount = vtPathMatrix.size();

	size_t sizeFileOffset = 0;

	// base
	FileBase base;
	strcpy(base.szType, "RLN");
	base.version[0] = 0;
	base.version[1] = 0;
	base.version[2] = 1;
	base.version[3] = 0;
	base.szHeader = sizeof(FileHeader);
	sizeFileOffset += fwrite(&base, 1, sizeof(base), fp);

	// header
	size_t sizeIndexData = nCount * sizeof(FileIndex);
	FileHeader header;
	header.rtMap;
	header.cntIndex = nCount * nCount;
	header.offIndex = sizeof(FileBase) + sizeof(FileHeader);
	header.offBody = header.offIndex + sizeIndexData;
	sizeFileOffset += fwrite(&header, 1, sizeof(header), fp);

	// index
	vector<vector<FileIndex>> vtRow;
	for (int ii = 0; ii < nCount; ii++) {
		vector<FileIndex> vtCol;
		for (int jj = 0; jj < nCount; jj++) {
			FileIndex index;
			index.rtData.Xmin = INT_MAX;
			index.rtData.Ymin = INT_MAX;
			index.rtData.Xmax = INT_MIN;
			index.rtData.Ymax = INT_MIN;
			vtCol.emplace_back(index);
			sizeFileOffset += fwrite(&index, 1, sizeof(index), fp);
		}
		vtRow.emplace_back(vtCol);
	}

	// body
	// for route line info table
	for (int row = 0; row < nCount; row++) {
		for (int col = 0; col < nCount; col++) {
			vector<SPoint> vtLines;
			const stPathMatrix* pMatrix = &vtPathMatrix[row][col];
			if (!pMatrix->vtRoutePath.empty()) {
				if (GetMatrixPathVertex(pMatrix, m_pDataMgr, vtLines) < 0) {
					LOG_TRACE(LOG_DEBUG, "matrix path merge missing, row:%d, col:%d", row, col);
				}
			}

			// write
			size_t should = vtLines.size() * sizeof(SPoint);
			for (size_t written = 0; written < should; ) {
				written += fwrite(&vtLines.front(), 1, should - written, fp);
			}

			vtRow[row][col].idxTile = row;
			vtRow[row][col].idTile = col;
			vtRow[row][col].szBody = should;
			vtRow[row][col].offBody = sizeFileOffset;
			if (should > 0) {
				extendDataBox(vtRow[row][col].rtData, &vtLines.front(), vtLines.size());
			}

			sizeFileOffset += should;
		} // for
	} // for

	  // re-write index
	fseek(fp, header.offIndex, SEEK_SET);

	for (int ii = 0; ii < nCount; ii++) {
		for (int jj = 0; jj < nCount; jj++) {
			fwrite(&vtRow[ii][jj], 1, sizeof(FileIndex), fp);
		}
	}

	fclose(fp);

	ret = RESULT_OK;

	return ret;
}


int32_t GetMatrixPathVertex(IN const stPathMatrix* pMatrix, IN CDataManager* pDataMgr, OUT vector<SPoint>& vtLines)
{
	int32_t ret = RESULT_FAILED;

	if (pMatrix == nullptr || pDataMgr == nullptr) {
		return ret;
	}

	if (!pMatrix->vtRoutePath.empty()) {
		stLinkInfo* pLink;
		KeyID keyLink;

		vtLines.clear();
		vtLines.reserve(pMatrix->vtRoutePath.size() * 3); // 예상 사이즈

		// 최초 정보로 도착 지점 사용
		//vtLines.push_back(pMatrix->endLinkInfo.MatchCoord);

		for (int idx = 0; idx < pMatrix->vtRoutePath.size(); idx++) {
			const stRoutePath* pPath = &pMatrix->vtRoutePath[idx];
			keyLink.llid = pPath->linkId;
			pLink = pDataMgr->GetVLinkDataById(keyLink);

			if (pMatrix->vtRoutePath.size() == 1) { // 동일 링크 결과일 경우
				// 도착지
				if (pPath->dir == 1) { // 정
					vtLines.emplace_back(pMatrix->endLinkInfo.LinkVtxToS[0]);
				} else {
					vtLines.emplace_back(pMatrix->endLinkInfo.LinkVtxToE[0]);
				}

				// 중간
				if (pMatrix->startLinkInfo.LinkSplitIdx != pMatrix->endLinkInfo.LinkSplitIdx) {
					if (pMatrix->startLinkInfo.LinkSplitIdx < pMatrix->endLinkInfo.LinkSplitIdx) { // 정
						for (int vtx = pMatrix->endLinkInfo.LinkSplitIdx; vtx >= pMatrix->startLinkInfo.LinkSplitIdx + 1; vtx--) {
							vtLines.emplace_back(pLink->getVertex()[vtx]);
						}
					} else { // 역
						for (int vtx = pMatrix->endLinkInfo.LinkSplitIdx + 1; vtx <= pMatrix->startLinkInfo.LinkSplitIdx; vtx++) {
							vtLines.emplace_back(pLink->getVertex()[vtx]);
						}
					}
				}

				// 출발지
				if (pPath->dir == 1) { // 정
					vtLines.emplace_back(pMatrix->startLinkInfo.LinkVtxToE[0]);
				} else {
					vtLines.emplace_back(pMatrix->startLinkInfo.LinkVtxToS[0]);					
				}				
			} else {
				// 출도착지 링크는 매칭된 vtx만 사용
				if ((idx == pMatrix->vtRoutePath.size() - 1) && (pPath->linkId == pMatrix->startLinkInfo.LinkId.llid)) {// 출발지
					if (pPath->dir == 1) { // 정
						ret = linkMerge(vtLines, pMatrix->startLinkInfo.LinkVtxToE, false);
					} else { // 역
						ret = linkMerge(vtLines, pMatrix->startLinkInfo.LinkVtxToS, false);
					}
				} else if ((idx == 0) && (pPath->linkId == pMatrix->endLinkInfo.LinkId.llid)) { // 도착지
					if (pPath->dir == 1) { // 정
						ret = linkMerge(vtLines, pMatrix->endLinkInfo.LinkVtxToS, false);
					} else { // 역
						ret = linkMerge(vtLines, pMatrix->endLinkInfo.LinkVtxToE, false);
					}
				} else {
					ret = linkMerge(vtLines, pLink->getVertex(), pLink->getVertexCount(), false);
				}
			}

			if (ret < 0) {
				LOG_TRACE(LOG_DEBUG, "link merge missing, path idx:%d, link id:%llu, tile:%d, nid:%d", idx, pLink->link_id.llid, pLink->link_id.tile_id, pLink->link_id.nid);
			}
		}

		// 뒤집기 도-출 -> 출-도
		reverse(vtLines.begin(), vtLines.end());
	}

	return ret;
}

