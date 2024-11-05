#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "../utils/UserLog.h"
#include "../utils/Strings.h"
#include "../utils/convexhull.h"

#if defined(USE_CJSON)
#include "../libjson/cjson/cJSON.h"
#endif

#include "../thlib/ZLIB.H"

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


//#define USE_GN_KMEANS_ALGORITHMS // k-means 알고리즘 사용

int32_t CTMSManager::GetCluster(IN const TspOptions* pTspOpt, IN const ClusteringOptions* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<SPoint>& vtOrigin, OUT vector<stDistrict>& vtDistrict, OUT vector<SPoint>& vtPositionLock)
{
	int32_t ret = TMS_RESULT_FAILED;

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

	ret = ROUTE_RESULT_SUCCESS;

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
		waypoint.x = pt.x;
		waypoint.y = pt.y;

		vtWaypoints.emplace_back(waypoint);
	}

	// get bestway
	TspOptions tspOpt;
	vector<uint32_t> vtBestway;
	int maxSize = vtOrigin.size();

	ClusteringOptions clustOpt;
	memcpy(&clustOpt, pClustOpt, sizeof(clustOpt));

	// poi가 2개 이하인 경우
	if (maxSize <= 2) {
		return ret;
	} else if (clustOpt.limitCluster <= 0 && clustOpt.limitValue <= 0) {
		// 클러스터 제한 또는 클러스터당 POI 제한 오류
		ret = TMS_RESULT_FAILED_INPUT_OPTION;
		return ret;
	} else if ((clustOpt.limitCluster > 0) && (clustOpt.limitValue > 0) && ((maxSize / clustOpt.limitCluster) > clustOpt.limitValue)) {
		// 전체 수량에 대해 지정 구역수와 구역별 지정 배정수가 한계치를 초과함.
		ret = TMS_RESULT_FAILED_LIMIT_COUNT_OVER;
		return ret;
	} 
	
	if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_DIST) {

	} else if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_TIME) {
		// 시간균등일 경우, 입력된 시간(분)을 초 단위로 변경
		clustOpt.limitValue *= 60;
		clustOpt.limitDeviation *= 60;
	} else {
		if ((clustOpt.limitValue == 0) && (clustOpt.limitCluster != 0)) {
			// 개수 균등일때, 개수 한계가 없으면 우선, 균등 분배 
			clustOpt.limitValue = maxSize / clustOpt.limitCluster;
		}
	}

	tspOpt.userId = clustOpt.userId;;
	tspOpt.fileCache = clustOpt.fileCache;
	tspOpt.algorithm = clustOpt.algorithm;
	if (maxSize <= 30) { // 30개 이하는 자체 TSP 사용
		tspOpt.algorithm = 0;
	}
	if (clustOpt.individualSize > 0) {
		tspOpt.individualSize = clustOpt.individualSize;
	}
	if (clustOpt.loopCount > 0) {
		tspOpt.loopCount = clustOpt.loopCount;
	}
	if (clustOpt.seed > 0) {
		tspOpt.seed = clustOpt.seed;
	}
	tspOpt.positionLock = clustOpt.positionLock;
	tspOpt.geneSize = vtWaypoints.size();


	// 클러스트링을 위해 최초 TSP(원점회귀) 수행
	double totDist = 0.f;
	int32_t totTime = 0;
	ret = GetBestway(&tspOpt, vtDistMatrix, vtWaypoints, vtBestway, totDist, totTime);


#if 0 // 테스트 용
	vtBestway.clear();
	vtBestway = {
		13 , 23 , 26 , 20 , 32 , 30 , 83 , 80 , 102 , 52 , 50 , 44 , 22 , 31 , 62 , 87 , 89 , 92 , 94 , 70 , 96 , 91 , 66 , 39 , 53 , 42 , 35 , 41 , 46 , 51 , 56 , 34 , 40 , 27 , 7 , 10 , 11 , 8 , 6 , 3 , 1 , 2 , 0 , 4 , 17 , 15 , 14 , 36 , 25 , 37 , 71 , 95 , 93 , 88 , 68 , 74 , 47 , 38 , 29 , 19 , 18 , 33 , 24 , 5 , 9 , 12 , 16 , 21 , 55 , 72 , 78 , 105 , 118 , 114 , 116 , 103 , 98 , 81 , 85 , 76 , 82 , 63 , 59 , 54 , 77 , 104 , 121 , 113 , 97 , 86 , 100 , 109 , 111 , 112 , 120 , 122 , 123 , 127 , 132 , 136 , 133 , 134 , 125 , 126 , 124 , 131 , 144 , 147 , 148 , 143 , 140 , 138 , 129 , 128 , 145 , 146 , 142 , 141 , 139 , 135 , 137 , 130 , 117 , 115 , 101 , 108 , 110 , 119 , 106 , 107 , 99 , 79 , 60 , 58 , 48 , 43 , 73 , 90 , 84 , 65 , 57 , 49 , 75 , 69 , 67 , 61 , 64 , 45 , 28 , 149 };

#endif

	const int MAX_CNT_LOOP = 10000; // 무한 반복 제한
	int cntLoop = 0;
	int bonusValue = 0;
	bool isRematch = false;
	bool isComplete = false;
	double deviation = clustOpt.limitDeviation;
	double offsetValue = clustOpt.limitDeviation * .1f;

	if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_DIST) {
		// 거리 균등
		// 최소 1km ~ 최대 10km
		if (offsetValue < 1000) {
			offsetValue = 1000;
		} else if (offsetValue > 10000) {
			offsetValue = 10000;
		}

		// 거리 균등에 클러스터 개수가 결정되어 있으면, 평균을 미리 계산
		if (clustOpt.limitCluster > 0 && clustOpt.limitValue > 0) {
			int nValue = totDist;
			clustOpt.limitValue = nValue / clustOpt.limitCluster;
		}
	} 
	else if (clustOpt.divisionType == TYPE_CLUSTER_DEVIDE_BY_TIME) {
		// 시간 균등
		// 최소 5분 ~ 최대 10분
		if (offsetValue < 5 * 60) {
			offsetValue = 5 * 60;
		} else if (offsetValue > 10 * 60) {
			offsetValue = 10 * 60;
		}

		// 시간 균등에 클러스터 개수가 결정되어 있으면, 평균을 미리 계산
		if (clustOpt.limitCluster > 0 && clustOpt.limitValue > 0) {
			int nValue = totTime;
			clustOpt.limitValue = nValue / clustOpt.limitCluster;
		}
	}
	else {
		// 개수 균등
		offsetValue = 1;

		// 클러스터 개수가 결정되어 있으면, 평균 및 보너스를 미리 계산
		if (clustOpt.limitCluster > 0 && clustOpt.limitValue > 0) {
			int nValue = maxSize;
			if (clustOpt.positionLock == TYPE_TSP_LOCK_START || pClustOpt->positionLock == TYPE_TSP_LOCK_END || pClustOpt->positionLock == TYPE_TSP_RECURSICVE) {
				nValue -= 1;
			} else if (pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) {
				nValue -= 2;
			}
			clustOpt.limitValue = nValue / clustOpt.limitCluster;
			bonusValue = nValue % clustOpt.limitCluster;
		}
	}



	LOG_TRACE(LOG_DEBUG, "clustering limit value : %d, deviation: %d, deviation offset: %.2f", clustOpt.limitValue, clustOpt.limitDeviation, offsetValue);

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
				deviation = 1;
			} else {
				// 10%를 편차로 사용
				deviation = avgValue * 0.1;
			}
		}

		if (maxValue - minValue > deviation) {
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

				if (pClustOpt->positionLock == TYPE_TSP_LOCK_START || pClustOpt->positionLock == TYPE_TSP_LOCK_END || pClustOpt->positionLock == TYPE_TSP_RECURSICVE) {
					if (clustOpt.limitValue - ((maxSize - 1) % clustOpt.limitValue) > deviation) {
						remainedValue = (maxSize - 1) % clustOpt.limitValue;
						// 지점 고정은 클러스터링 개수에서 제외
					}
				} else if (pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) {
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
			}
			else {
				// 마지막 최소 값이 편차보다 작고, 최대+최소값이 최초의 한계값 보다 작으면 보너스로 분배
				if (minValue <= deviation) {
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

		LOG_TRACE(LOG_DEBUG, "clustering rematching limit value : %d", clustOpt.limitValue);
	} // for


	// TSP
	if (ret == RESULT_OK) {
		memcpy(&tspOpt, pTspOpt, sizeof(tspOpt));

		for (auto& cluster : vtDistrict) {
			double totDist = 0.f;
			int32_t totTime = 0;

			stWaypoints waypoint;
			vector<stWaypoints> vtClustWaypoints;
			for (int ii = 0; ii < cluster.vtPois.size(); ii++) {
				waypoint.nId = cluster.vtPois[ii];
				waypoint.x = cluster.vtCoord[ii].x;
				waypoint.y = cluster.vtCoord[ii].y;

				vtClustWaypoints.emplace_back(waypoint);
			} // for

			if (tspOpt.positionLock == TYPE_TSP_LOCK_START || tspOpt.positionLock == TYPE_TSP_LOCK_START_END || tspOpt.positionLock == TYPE_TSP_RECURSICVE) {
				waypoint.nId = 0;
				waypoint.x = vtOrigin[0].x;
				waypoint.y = vtOrigin[0].y;
				vtClustWaypoints.insert(vtClustWaypoints.begin(), waypoint);
			} 
			if (tspOpt.positionLock == TYPE_TSP_LOCK_END || tspOpt.positionLock == TYPE_TSP_LOCK_START_END) {
				waypoint.nId = maxSize - 1;
				waypoint.x = vtOrigin[waypoint.nId].x;
				waypoint.y = vtOrigin[waypoint.nId].y;
				vtClustWaypoints.emplace_back(waypoint);
			} 

			tspOpt.geneSize = vtClustWaypoints.size();


			if (cluster.vtPois.size() <= 2) {
				continue;
			}


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

			

			if ((ret == RESULT_OK) && 
				(totDist < cluster.dist || totTime < cluster.time)) {

				LOG_TRACE(LOG_DEBUG, "cluster result change, id:%d old_dist:%.0f, old_time:%d -> new_dist:%.0f, new_time:%d", cluster.id, cluster.dist, cluster.time, totDist, totTime);

				cluster.vtPois.clear();
				cluster.vtCoord.clear();

				for (int ii = 0; ii < vtBestway.size(); ii++) {
					if ((ii == 0) && (tspOpt.positionLock == TYPE_TSP_LOCK_START || tspOpt.positionLock == TYPE_TSP_LOCK_START_END || tspOpt.positionLock == TYPE_TSP_RECURSICVE)) {
						continue;
					}
					if ((ii == vtBestway.size() - 1) && (tspOpt.positionLock == TYPE_TSP_LOCK_END || tspOpt.positionLock == TYPE_TSP_LOCK_START_END)) {
						continue;
					}

					cluster.vtPois.emplace_back(vtClustWaypoints[vtBestway[ii]].nId);
					cluster.vtCoord.emplace_back(SPoint{ vtClustWaypoints[vtBestway[ii]].x, vtClustWaypoints[vtBestway[ii]].y });
				}

				cluster.dist = totDist;
				cluster.time = totTime;
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
			// make border
			GetBoundary(cluster.vtCoord, cluster.vtBorder, cluster.center);
		}

		// 출/도착지 고정
		if (pClustOpt->positionLock != TYPE_TSP_LOCK_NONE) {
			if (pClustOpt->positionLock == TYPE_TSP_LOCK_START || pClustOpt->positionLock == TYPE_TSP_LOCK_START_END || pClustOpt->positionLock == TYPE_TSP_RECURSICVE) {
				SPoint coord = { vtOrigin[0].x, vtOrigin[0].y };
				vtPositionLock.emplace_back(coord);
			} else {
				vtPositionLock.emplace_back(SPoint{ 0, 0 });
			}

			if (pClustOpt->positionLock == TYPE_TSP_LOCK_END || pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) {
				SPoint coord = { vtOrigin[waypoint.nId].x, vtOrigin[waypoint.nId].y };
				vtPositionLock.emplace_back(coord);
			} else {
				vtPositionLock.emplace_back(SPoint{ 0, 0 });
			}
		}
	}
#endif // #define USE_KMEANS_ALORITHM // k-means 알고리즘 사용

	return ret;
}


int32_t CTMSManager::GetBestway(IN const TspOptions* pTspOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN vector<stWaypoints>& vtOrigin, OUT vector<uint32_t>& vtBestWaypoints, OUT double& dist, OUT int32_t& time)
{
	int32_t ret = RESULT_FAILED;

#if !defined(USE_REAL_ROUTE_TSP)
	//vector<stCity> vtRawData;
#else
	//vector<stWaypoints> vtOrigin;
#endif

	LOG_TRACE(LOG_DEBUG, "start, best waypoint result, cnt:%d", vtOrigin.size());

	vector<int32_t> vtWayResult;
	vtWayResult.reserve(vtOrigin.size());

	if (pTspOpt->algorithm == TYPE_TSP_ALGORITHM_COPY) {
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
		int nStartId = 0;
		if (pTspOpt->positionLock == TYPE_TSP_LOCK_END || pTspOpt->positionLock == TYPE_TSP_LOCK_START_END) { // 목적지면 반대로 뒤집어 TSP 수행 후 다시 뒤집자
			nStartId = vtOrigin[vtDistMatrix.size() - 1].nId;
		}

		int nOffset = 0;
		for (int ii = 0; ii < vtWayResult.size(); ii++) {
			if (vtWayResult[ii] == nStartId) {
				nOffset = ii;
				break;
			}
		}

		if (nOffset > 0) {
			vector<int> vtWayReset;
			vtWayReset.reserve(vtWayResult.size());
			for (int ii = 0, off = 0; ii < vtWayResult.size(); ii++) {
				vtWayReset.emplace_back(vtWayResult[nOffset + off++]);
				if (nOffset + off > vtWayResult.size() - 1) {
					nOffset = off = 0;
				}
			} // for

			vtWayResult.assign(vtWayReset.rbegin(), vtWayReset.rend());
		}
	}
#endif // #if !defined(USE_REAL_ROUTE_TSP)
	else {
		Environment newWorld;

		// 최대 세대 
		const int32_t maxGeneration = pTspOpt->loopCount;
		// 세대당 최대 개체수
		const int32_t maxPopulation = pTspOpt->individualSize;
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

		const uint32_t MAX_SAME_RESULT = min(100, maxGene * 2); // maxGene * 10; // 500;// maxGeneration; // 같은 값으로 100회 이상 진행되면 최적값에 수렴하는것으로 판단하고 종료
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
					LOG_TRACE(LOG_DEBUG, "best result fitness value:%.3f, repeating count:%d", newWorld.GetBestDist(), repeatGeneration);
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
	uint32_t idxPrev = 0;
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

		idxPrev = item;
		idx++;
	}

	// 원점 회귀 확인
	if (pTspOpt->positionLock == TYPE_TSP_RECURSICVE) {
		totalDist += vtDistMatrix[idxPrev][idxFirst].nTotalDist;
		LOG_TRACE(LOG_DEBUG_CONTINUE, " %d => %d\n", idxFirst, totalDist);
	} else {
		LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");
	}

	vtBestWaypoints.clear();
	//vtBestWaypoints.reserve(vtOrigin->size());

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


int32_t devideCluster(IN const ClusteringOptions* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<uint32_t>& vtBestways, IN const int32_t bonusValue, IN const int32_t firstIdx, IN const int32_t lastIdx, OUT stDistrict& cluster, OUT vector<uint32_t>& vtRemains)
{
	int32_t ret = 0;

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
		int countToNext = 0;

		double distToDest = 0.f;
		double timeToDest = 0.f;
		int countToDest = 0;

		prev = vtBestways[ii];

		if (ii == 0) {
			// 공동 사용 지점은 추가 하지 않음.
			//// 출발지는 무조건 등록, // 최대 거리가 한계치 이상일때는 어떻게 할지 고민 필요
			//cluster.vtPois.emplace_back(prev);

			// 출발지 원점이 있을 경우 원점으로 부터 시작지점까지의 값을 계산
			if (((pClustOpt->positionLock == TYPE_TSP_RECURSICVE) || // 출발지 회귀
				(pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) || // 출.도착지 고정
				(pClustOpt->positionLock == TYPE_TSP_LOCK_START))) // 출발지 고정
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

			// 한개의 POI만 있을 경우
			if (ii == maxSize - 1) {
				cluster.dist += distToNext;
				cluster.time += timeToNext;
			}
		}

		// 마지막 아이템이면 여기까지
		if (ii == maxSize - 1) {
			// 도착지 원점이 있을 경우 원점으로 부터 도착지점까지의 값을 계산
			if ((pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) || // 출.도착지 고정
				(pClustOpt->positionLock == TYPE_TSP_LOCK_END)) // 도착지 고정
			{
				if (next != lastIdx) {
					distToNext += vtDistMatrix[ii][lastIdx].nTotalDist;
					timeToNext += vtDistMatrix[ii][lastIdx].nTotalTime;
					countToNext++;
				}
			}

			break;
		}


		next = vtBestways[ii + 1];

		distToNext += vtDistMatrix[prev][next].nTotalDist;
		timeToNext += vtDistMatrix[prev][next].nTotalTime;
		countToNext += 1;

		double valueToNext = 0.f;
		double valueToDest = 0.f;

		// 도착지 원점이 있을 경우 원점으로 부터 도착지점까지의 값을 계산
		if ((pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) || // 출.도착지 고정
			(pClustOpt->positionLock == TYPE_TSP_LOCK_END)) // 도착지 고정
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


		if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_DIST) {
			curValue = distToNext;
		} else 	if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_TIME) {
			curValue = timeToNext;
		} else { //if (pClustOpt->cntCluster > 0) {
			curValue = countToNext;
		}

		totValue += curValue;

		if (totValue + valueToDest <= maxValue) {
			// 공동 사용 지점은 추가 하지 않음.
			// 추가
			//cluster.vtPois.emplace_back(next);
			
			// 도착지 원점이 있을 경우
			if ((next == lastIdx) &&
				((pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) || // 출.도착지 고정
				(pClustOpt->positionLock == TYPE_TSP_LOCK_END))) // 도착지 고정
			{
				// 공동 사용 지점은 추가 하지 않음.
			} else {
				// 추가
				cluster.vtPois.emplace_back(next);
			}

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

	ret = cluster.vtPois.size();

	return ret;
}


int32_t CTMSManager::DevideClusterUsingTsp(IN const ClusteringOptions* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtWaypoints, IN const vector<uint32_t>& vtBestways, IN const int32_t firstIdx, IN const int32_t lastIdx, IN OUT int32_t& bonusValue, OUT stDistrict& cluster, OUT vector<uint32_t>& vtRemains)
{
	int32_t ret = 0;

	int maxSize = vtBestways.size();
	double maxValue = 0.f;

	// 분배 방식
	int typeDivision = pClustOpt->divisionType;

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

		if (bonusValue > 0) {
			maxValue += 1;
			bonusValue--;
		}
	}

	int32_t prev = 0;
	int32_t next = 0;

	vector<uint32_t> vtNewBestWays;
	vector<stWaypoints> vtNewBestWaypoints;

	double totValue = 0.f;
	double totDist = 0.f;
	int32_t totTime = 0;
	int32_t totCount = 0;

	double firstValue = 0.f;
	double firstDist = 0.f;
	int32_t firstTime = 0;
	int32_t firstCount = 0;


	// 1차로 최초 클러스터링된 순서에 의해 기본 분배
	for (int ii = 0; ii < maxSize; ii++) {
		prev = vtBestways[ii];

		if (ii == 0) {
			// 출발지 원점이 있을 경우 원점으로 부터 시작지점까지의 값을 계산
			if (((pClustOpt->positionLock == TYPE_TSP_RECURSICVE) || // 출발지 회귀
				(pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) || // 출.도착지 고정
				(pClustOpt->positionLock == TYPE_TSP_LOCK_START))) // 출발지 고정
			{
				firstDist += vtDistMatrix[firstIdx][prev].nTotalDist;
				firstTime += vtDistMatrix[firstIdx][prev].nTotalTime;
				firstCount++;

				//vtNewBestWays.emplace_back(ii);
				//vtNewBestWaypoints.emplace_back(vtWaypoints[ii]);
			}
		}
		
		vtNewBestWays.emplace_back(prev);
		vtNewBestWaypoints.emplace_back(vtWaypoints[prev]);

		// 마지막 아이템이면 여기까지
		if (ii == maxSize - 1) {
			// 도착지 원점이 있을 경우 원점으로 부터 도착지점까지의 값을 계산
			if ((pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) || // 출.도착지 고정
				(pClustOpt->positionLock == TYPE_TSP_LOCK_END)) // 도착지 고정
			{
				totDist += vtDistMatrix[ii][lastIdx].nTotalDist;
				totTime += vtDistMatrix[ii][lastIdx].nTotalTime;
				totCount++;

				//vtNewBestWays.emplace_back(ii);
				//vtNewBestWaypoints.emplace_back(vtWaypoints[ii]);
			}
			break;
		}


		// 다음 값을 미리 비교해 보고 현재에서 종료할지 계속할지 결정하자
		double nextValue = 0.f;
		double nextDist = 0.f;
		int32_t nextTime = 0;
		int32_t nextCount = 0;

		next = vtBestways[ii + 1];

		nextDist = vtDistMatrix[prev][next].nTotalDist;
		nextTime = vtDistMatrix[prev][next].nTotalTime;
		nextCount++;


		// 도착지 원점이 있을 경우
		double lastValue = 0.f;
		double lastDist = 0.f;
		int32_t lastTime = 0;
		int32_t lastCount = 0;
		if ((pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) || // 출.도착지 고정
			(pClustOpt->positionLock == TYPE_TSP_LOCK_END)) // 도착지 고정
		{
			lastDist = vtDistMatrix[next][lastIdx].nTotalDist;
			lastTime = vtDistMatrix[next][lastIdx].nTotalTime;
			lastCount++;
		}

		if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_DIST) {
			nextValue = nextDist;
			firstValue = firstDist;
			lastValue = lastDist;
		} else 	if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_TIME) {
			nextValue = nextTime;
			firstValue = firstTime;
			lastValue = lastTime;
		} else { //if (pClustOpt->cntCluster > 0) {
			nextValue = nextCount;
			firstValue = firstCount;
			lastValue = lastCount;
		}

		//if (totValue + firstValue + lastValue + nextValue >= maxValue) {
		if (totValue + firstValue + lastValue + nextValue > maxValue) {
			totDist = totDist + firstDist + lastDist;
			totTime = totTime + firstTime + lastTime;
			totCount = totCount + firstCount + lastCount;

			break;			
		} else {
			totCount++;
			totValue += nextValue;
		}
	} // for


	// 남은 배송지
	vtRemains.assign(vtBestways.begin() + vtNewBestWaypoints.size(), vtBestways.end());


	// 2차로 1차 분배된 값을 TSP 적용 후 상세하게 조정
	// TSP
	if (typeDivision != TYPE_CLUSTER_DEVIDE_BY_COUNT)
	{
		TspOptions tspOpt;

		// set tsp option
		tspOpt.userId = pClustOpt->userId;;
		tspOpt.fileCache = pClustOpt->fileCache;
		tspOpt.algorithm = 0; // 알고리즘 변경
		tspOpt.compareType = pClustOpt->compareType; // TYPE_TSP_VALUE_DIST; // DM 테이블 비교 기준 변경

		if (pClustOpt->individualSize > 0) {
			tspOpt.individualSize = pClustOpt->individualSize;
		}
		if (pClustOpt->loopCount > 0) {
			tspOpt.loopCount = pClustOpt->loopCount;
		}
		if (pClustOpt->seed > 0) {
			tspOpt.seed = pClustOpt->seed;
		}

		tspOpt.positionLock = pClustOpt->positionLock;
		


		// 최초 분배의 TSP를 확인하자
		int32_t prevTotCount = 0;
		int32_t prevTotTime = 0;
		double prevTotDist = 0.f;

		int32_t nextTotCount = 0;
		int32_t nextTotTime = 0;
		double nextTotDist = 0.f;
		
		vector<uint32_t> vtPrevBestways;
		vector<uint32_t> vtNextBestways;
		vector<stWaypoints> vtPrevWaypoints;
		vector<stWaypoints> vtNextWaypoints;

		vtNextBestways.assign(vtNewBestWays.begin(), vtNewBestWays.end());
		vtNextWaypoints.assign(vtNewBestWaypoints.begin(), vtNewBestWaypoints.end());

		// 출도착지점 임시로 추가하여 TSP 계산
		// 출발지 고정일 경우
		if (pClustOpt->positionLock == TYPE_TSP_LOCK_START || pClustOpt->positionLock == TYPE_TSP_LOCK_START_END || pClustOpt->positionLock == TYPE_TSP_RECURSICVE) {
			vtNextBestways.insert(vtNextBestways.begin(), firstIdx);
			stWaypoints waypoint;
			waypoint.nId = vtWaypoints[firstIdx].nId;
			waypoint.x = vtWaypoints[firstIdx].x;
			waypoint.y = vtWaypoints[firstIdx].y;
			vtNextWaypoints.insert(vtNextWaypoints.begin(), waypoint);
		}
		// 도착지 고정일 경우
		if (pClustOpt->positionLock == TYPE_TSP_LOCK_END || pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) {
			vtNextBestways.emplace_back(lastIdx);
			stWaypoints waypoint;
			waypoint.nId = vtWaypoints[lastIdx].nId;
			waypoint.x = vtWaypoints[lastIdx].x;
			waypoint.y = vtWaypoints[lastIdx].y;
			vtNextWaypoints.emplace_back(waypoint);
		} else {
			;
		}

		enum
		{
			IS_NONE = 0,
			IS_UP = 1,
			IS_DOWN = 2
		};

		int isUpDown = IS_NONE; // 0:NONE, 1:UP, 2:DOWN
		bool isReverse = false;

		for (int ii=0 ; ii < vtNextWaypoints.size(); ii++)
		{
			if (vtNewBestWays.size() < 3) {
				vtNextBestways[ii] = ii;
				if (ii > 0) {
					nextTotDist += vtDistMatrix[ii-1][ii].nTotalDist;
					nextTotTime += vtDistMatrix[ii-1][ii].nTotalTime;
				}
			} else {
				tspOpt.geneSize = vtNextWaypoints.size();

				// 클러스터당 POI WM을 새로 구성한다. 
				vector<vector<stDistMatrix>> vtNewDistMatrix;
				for (const auto& poiCols : vtNextWaypoints) {
					vector<stDistMatrix> vtRowsDM;
					for (const auto& poiRows : vtNextWaypoints) {
						vtRowsDM.emplace_back(vtDistMatrix[poiCols.nId][poiRows.nId]);
					}
					vtNewDistMatrix.emplace_back(vtRowsDM);
				}

				ret = GetBestway(&tspOpt, vtNewDistMatrix, vtNextWaypoints, vtNextBestways, nextTotDist, nextTotTime);

				double nextValue = 0.f;
				if (ret == RESULT_OK) {
					if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_DIST) {
						nextValue = nextTotDist;
					} else 	if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_TIME) {
						nextValue = nextTotTime;
					} else { //if (pClustOpt->cntCluster > 0) {
						nextValue = nextTotCount;
					}

					if (nextValue < maxValue) {
						if (isUpDown == IS_DOWN) {
							isReverse = true;
						}
						isUpDown = IS_UP;
					} else if (nextValue > maxValue) {
						if (isUpDown == IS_UP) {
							isReverse = true;
						}
						isUpDown = IS_DOWN;
					} else {
						isReverse = true;
					}
				}

				if (vtRemains.empty() || vtRemains.size() <= ii) {
					break;
				} else if (isReverse) {

					// 오버된 값이 보너스값 한계치 이내라면 보너스 값으로 보완
					if ((0 < bonusValue) && (nextValue < maxValue + bonusValue)) {
						bonusValue -= (nextValue - maxValue);
					} else {
						// 이전 값 사용
						vtNextBestways.clear();
						vtNextBestways.assign(vtPrevBestways.begin(), vtPrevBestways.end());

						vtNextWaypoints.clear();
						vtNextWaypoints.assign(vtPrevWaypoints.begin(), vtPrevWaypoints.end());

						nextTotCount = prevTotCount;
						nextTotTime = prevTotTime;
						nextTotDist = prevTotDist;
					}

					break;
				} else {
					// 이전 값 캐시
					prevTotCount = nextTotCount;
					prevTotTime = nextTotTime;
					prevTotDist = nextTotDist;

					vtPrevBestways.clear();
					vtPrevBestways.assign(vtNextBestways.begin(), vtNextBestways.end());

					vtPrevWaypoints.clear();
					vtPrevWaypoints.assign(vtNextWaypoints.begin(), vtNextWaypoints.end());

					// 값이 부족해 더 추가 필요
					if (isUpDown == IS_UP) {
						int idx = vtRemains[ii];
						stWaypoints waypoint;
						waypoint.nId = vtWaypoints[idx].nId;
						waypoint.x = vtWaypoints[idx].x;
						waypoint.y = vtWaypoints[idx].y;

						// 출발지 고정일 경우
						if (pClustOpt->positionLock == TYPE_TSP_LOCK_START || pClustOpt->positionLock == TYPE_TSP_LOCK_START_END || pClustOpt->positionLock == TYPE_TSP_RECURSICVE) {
							vtNextWaypoints.insert(vtNextWaypoints.begin() + 1, waypoint);
						}
						// 도착지 고정일 경우
						else if (pClustOpt->positionLock == TYPE_TSP_LOCK_END || pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) {
							vtNextWaypoints.insert(vtNextWaypoints.end() - 1, waypoint);
						} else {
							vtNextWaypoints.emplace_back(waypoint);
						}
					} else if (isUpDown == IS_DOWN) {
						//break;
						// 출발지 고정일 경우
						if (pClustOpt->positionLock == TYPE_TSP_LOCK_START || pClustOpt->positionLock == TYPE_TSP_LOCK_START_END || pClustOpt->positionLock == TYPE_TSP_RECURSICVE) {
							vtNextWaypoints.erase(vtNextWaypoints.begin() + 1);
						}
						// 도착지 고정일 경우
						else if (pClustOpt->positionLock == TYPE_TSP_LOCK_END || pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) {
							vtNextWaypoints.erase(vtNextWaypoints.end() - 2);
						} else {
							vtNextWaypoints.pop_back();
						}
					}
				}
			}
		} // for

		// 임시로 추가된 출도착지점 제거
		// 출발지 고정일 경우
		if (pClustOpt->positionLock == TYPE_TSP_LOCK_START || pClustOpt->positionLock == TYPE_TSP_LOCK_START_END || pClustOpt->positionLock == TYPE_TSP_RECURSICVE) {
			vtNextBestways.erase(vtNextBestways.begin());
			vtNextWaypoints.erase(vtNextWaypoints.begin());

			// 시작지가 제거되었으므로 idx offset을 하나씩 줄여줘야 함
			for (auto& way : vtNextBestways) {
				way--;
			}
		}
		// 도착지 고정일 경우
		if (pClustOpt->positionLock == TYPE_TSP_LOCK_END || pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) {
			vtNextBestways.pop_back();
			vtNextWaypoints.pop_back();
		} else {
			;
		}

		vtRemains.clear();
		if (vtNextBestways.size() < vtBestways.size()) {
			vtRemains.assign(vtBestways.begin() + vtNextBestways.size(), vtBestways.end());
		}

		vtNewBestWays.clear();
		vtNewBestWays.assign(vtNextBestways.begin(), vtNextBestways.end());

		vtNewBestWaypoints.clear();
		vtNewBestWaypoints.assign(vtNextWaypoints.begin(), vtNextWaypoints.end());

		totDist = nextTotDist;
		totTime = nextTotTime;
		totCount = nextTotCount;
	} else {

	}

	cluster.vtPois.clear();
	cluster.vtCoord.clear();
	cluster.vtTimes.clear();
	cluster.vtDistances.clear();

	int prevId = -1;
	int currId = -1;
	int offset = 0;
	for (int ii = 0; ii < vtNewBestWays.size(); ii++) {
		if (typeDivision == TYPE_CLUSTER_DEVIDE_BY_COUNT) {
			offset = ii;
		} else {
			offset = vtNewBestWays[ii];
		}
		currId = vtNewBestWaypoints[offset].nId;

		cluster.vtPois.emplace_back(vtNewBestWaypoints[offset].nId);
		cluster.vtCoord.emplace_back(SPoint{ vtNewBestWaypoints[offset].x, vtNewBestWaypoints[offset].y });

		// 시작
		if (ii == 0) {
			// 출발지 고정일 경우
			if (pClustOpt->positionLock == TYPE_TSP_LOCK_START || pClustOpt->positionLock == TYPE_TSP_LOCK_START_END || pClustOpt->positionLock == TYPE_TSP_RECURSICVE) {
				cluster.vtTimes.emplace_back(vtDistMatrix[firstIdx][currId].nTotalTime);
				cluster.vtDistances.emplace_back(vtDistMatrix[firstIdx][currId].nTotalDist);
			}
		} 
		// 종료
		else if (ii == vtNewBestWays.size() - 1) {
			cluster.vtTimes.emplace_back(vtDistMatrix[prevId][currId].nTotalTime);
			cluster.vtDistances.emplace_back(vtDistMatrix[prevId][currId].nTotalDist);

			// 도착지 고정일 경우
			if (pClustOpt->positionLock == TYPE_TSP_LOCK_END || pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) {
				cluster.vtTimes.emplace_back(vtDistMatrix[currId][lastIdx].nTotalTime);
				cluster.vtDistances.emplace_back(vtDistMatrix[currId][lastIdx].nTotalDist);
			}
		} 
		// 중간
		else {
			cluster.vtTimes.emplace_back(vtDistMatrix[prevId][currId].nTotalTime);
			cluster.vtDistances.emplace_back(vtDistMatrix[prevId][currId].nTotalDist);
		}

		prevId = vtNewBestWaypoints[offset].nId;
	} // for

	cluster.dist = totDist;
	cluster.time = totTime;

	ret = cluster.vtPois.size();

	return ret;
}


bool CTMSManager::Clustering(IN const ClusteringOptions* pClustOpt, IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const vector<stWaypoints>& vtWaypoints, IN const vector<uint32_t>& vtBestway, IN const int32_t bonusValue, OUT vector<stDistrict>& vtClusters)
{
	bool ret = false;

	vector<uint32_t> vtBestwayWork;
	int32_t nRemainedBonusValue = bonusValue;
	int32_t nFirst = vtBestway.front();
	int32_t nLast = vtBestway.back();
	bool isEmpty = false;

	// 출발지 원점이 있을 경우 원점으로 부터 시작지점까지의 값을 계산
	if (pClustOpt->positionLock == TYPE_TSP_LOCK_START_END) // 출.도착지 고정
	{
		vtBestwayWork.assign(vtBestway.begin() + 1, vtBestway.end() - 1);
	} 
	else if ((pClustOpt->positionLock == TYPE_TSP_RECURSICVE) || // 출발지 회귀
		(pClustOpt->positionLock == TYPE_TSP_LOCK_START)) // 출발지 고정
	{
		vtBestwayWork.assign(vtBestway.begin() + 1, vtBestway.end());
	} 
	else if (pClustOpt->positionLock == TYPE_TSP_LOCK_END) // 목적지 고정
	{
		vtBestwayWork.assign(vtBestway.begin(), vtBestway.end() - 1);
	} else {
		vtBestwayWork.assign(vtBestway.begin(), vtBestway.end());
	}
	
	for (int cntCluster = 0; isEmpty != true; cntCluster++) {
		//tspOpt.geneSize = vtRequest.size();
		//ret = GetBestway(&tspOpt, ppResultTables, &vtRequest, vtBestway);

		vector<uint32_t> vtRemains;
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
			ret = true;
			break;
		} else {
			//vtRequest.clear();
			vtBestwayWork.clear();
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


SPoint getCoordFromText(const char* pszCoord)
{
	SPoint ptLocation = { 0, };
	int column = 0;
	int len = 0;
	char szCoord[128] = { 0, };

	if ((len = strlen(pszCoord)) > 0) {
		memcpy(&szCoord, pszCoord, len);
		trim(szCoord);
	}

	if (strlen(szCoord) > 0) {
		char* pTok = NULL;
		char szTok[] = { " ," };

		pTok = strtok(szCoord, szTok);
		if (pTok && strlen(pTok) >= 4) {
			ptLocation.x = atof(pTok);
		}

		pTok = strtok(NULL, szTok);
		if (pTok && strlen(pTok) >= 4) {
			ptLocation.y = atof(pTok);
		}

		// 한국에서 좌표 반전 확인
		if (ptLocation.x > 0.f && ptLocation.y > 0.f) {
			if (ptLocation.x < 100.f && ptLocation.y > 100.f) {
				double tmp = ptLocation.x;
				ptLocation.x = ptLocation.y;
				ptLocation.y = tmp;
			}
		}
	}

	return ptLocation;
}


int32_t CTMSManager::GetBoundary(IN vector<SPoint>& vtCoord, OUT vector<SPoint>& vtBoundary, OUT SPoint& center)
{
	int32_t ret = -1;

	if (vtCoord.empty()) {
		return ret;
	}

	//vector<SPoint> coords;
	//vector<SPoint> border;
	//vector<SPoint> expend;
	//vector<SPoint> slice;

	// make border
	ConvexHull(vtCoord, vtBoundary);

	center = calculateCentroid(vtBoundary);

	// make closed
	if (!vtBoundary.empty() && ((vtBoundary.front().x != vtBoundary.back().x) || (vtBoundary.front().y != vtBoundary.back().y))) {
		vtBoundary.emplace_back(vtBoundary.front());
	}

	ret = 0;

#if 0
	// 면적 조사
	double area = GetPolygonArea(border);

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


uint32_t CTMSManager::GetRequestCluster(IN const char* szRequest, OUT vector<SPoint>& vtOrigin, OUT TspOptions& tspOpt, OUT ClusteringOptions& clustOpt)
{
	uint32_t crc = 0;

#if defined(USE_CJSON)
	cJSON* root = cJSON_Parse(szRequest);
	if (root != NULL) {
		cJSON* pValue = cJSON_GetObjectItem(root, "id");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			tspOpt.userId = clustOpt.userId = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			tspOpt.userId = clustOpt.userId = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(root, "cache");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			tspOpt.fileCache = clustOpt.fileCache = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			tspOpt.fileCache = clustOpt.fileCache = atoi(cJSON_GetStringValue(pValue));
		}

		cJSON* pTsp = cJSON_GetObjectItem(root, "tsp");
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
		}

		cJSON* pClust = cJSON_GetObjectItem(root, "clust");
		if (pTsp != NULL) {
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
		}
		else {
			clustOpt.seed = 10000;
			clustOpt.compareType = TYPE_TSP_VALUE_DIST;
			clustOpt.algorithm = 3;
		}

		cJSON* pOption = cJSON_GetObjectItem(root, "option");
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

			pValue = cJSON_GetObjectItem(pOption, "position_lock"); // 지점 고정
			if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
				tspOpt.positionLock = clustOpt.positionLock = cJSON_GetNumberValue(pValue);				
			} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
				tspOpt.positionLock = clustOpt.positionLock = atoi(cJSON_GetStringValue(pValue));
			}
		}

		cJSON* pOrigins = cJSON_GetObjectItem(root, "origins");
		if (pOrigins != NULL) {
			int nOrigins = cJSON_GetArraySize(pOrigins);
			if (nOrigins > 0) {
				cJSON* pCoord;
				string strCoord;
				SPoint tmpCoord;
				for (int ii = 0; ii < nOrigins; ii++) {
					pCoord = cJSON_GetArrayItem(pOrigins, ii);
					if (pCoord != NULL) {
						strCoord = cJSON_GetStringValue(pCoord);
						tmpCoord = getCoordFromText(strCoord.c_str());

						vtOrigin.emplace_back(tmpCoord);

						crc = crc32(crc, reinterpret_cast<Bytef*>(&tmpCoord.x), sizeof(tmpCoord.x));
						crc = crc32(crc, reinterpret_cast<Bytef*>(&tmpCoord.y), sizeof(tmpCoord.y));
					}
				} // for
			}
		}

		cJSON_Delete(root);
	} // cJSON
#endif // #if defined(USE_CJSON)

	return crc;
}


uint32_t CTMSManager::GetRequestBestway(IN const char* szRequest, OUT vector<SPoint>& vtOrigin, OUT TspOptions& tspOpt)
{
	uint32_t crc = 0;

#if defined(USE_CJSON)
	cJSON* root = cJSON_Parse(szRequest);
	if (root != NULL) {
		cJSON* pValue = cJSON_GetObjectItem(root, "id");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			tspOpt.userId = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			tspOpt.userId = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(root, "cache");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			tspOpt.fileCache = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			tspOpt.fileCache = atoi(cJSON_GetStringValue(pValue));
		}

		pValue = cJSON_GetObjectItem(root, "target");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			tspOpt.target = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			tspOpt.target = atoi(cJSON_GetStringValue(pValue));
		}

		cJSON* pTsp = cJSON_GetObjectItem(root, "tsp");
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
		}
				
		cJSON* pOption = cJSON_GetObjectItem(root, "option");
		if (pOption != NULL) {
			pValue = cJSON_GetObjectItem(pOption, "position_lock"); // 지점 고정
			if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
				tspOpt.positionLock = cJSON_GetNumberValue(pValue);
			} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
				tspOpt.positionLock = atoi(cJSON_GetStringValue(pValue));
			}
		}

		cJSON* pOrigins = cJSON_GetObjectItem(root, "origins");
		if (pOrigins != NULL) {
			int nOrigins = cJSON_GetArraySize(pOrigins);

			tspOpt.geneSize = nOrigins;

			if (nOrigins > 0) {
				cJSON* pCoord;
				string strCoord;
				SPoint tmpCoord;
				for (int ii = 0; ii < nOrigins; ii++) {
					pCoord = cJSON_GetArrayItem(pOrigins, ii);
					if (pCoord != NULL) {
						strCoord = cJSON_GetStringValue(pCoord);
						tmpCoord = getCoordFromText(strCoord.c_str());

						vtOrigin.emplace_back(tmpCoord);

						crc = crc32(crc, reinterpret_cast<Bytef*>(&tmpCoord.x), sizeof(tmpCoord.x));
						crc = crc32(crc, reinterpret_cast<Bytef*>(&tmpCoord.y), sizeof(tmpCoord.x));
					}
				} // for
			}
		}

		cJSON_Delete(root);
	} // cJSON
#endif // #if defined(USE_CJSON)

	return crc;
}


uint32_t CTMSManager::LoadWeightMatrix(IN const char* szFileName, IN const int cntItem, IN const int sizeItem, IN const uint32_t crc, OUT vector<vector<stDistMatrix>>& vtDistMatrix)
{
	uint32_t ret = -1;

	if (szFileName == nullptr) {
		return ret;
	}

	FILE* fp = fopen(szFileName, "rb");
	if (fp) {
		// read count
		int32_t readRows = 0;
		int32_t readDataSize = 0;
		uint32_t readCrc = 0;

		fread(&readRows, 1, sizeof(readRows), fp); // read 4byte
		fread(&readDataSize, 1, sizeof(readDataSize), fp); // read 4byte
		fread(&readCrc, 1, sizeof(readCrc), fp); // read 4byte

		if ((readRows <= 0 || readDataSize <= 0) || (readRows != cntItem) || (readDataSize != sizeItem) || readCrc != crc) {
			LOG_TRACE(LOG_WARNING, "failed, read weight matrix, [req vs res] cnt: %d vs %d, size: %d vs %d, crc: %u vs %u", readRows, cntItem, sizeItem, readDataSize, readCrc, crc, readCrc);
		} else {
			for (int ii = 0; ii < readRows; ii++) {
				vector<stDistMatrix> vtDistMatrixRow;
				for (int jj = 0; jj < readRows; jj++) {
					stDistMatrix item;

					// read matrix
					//uint32_t nTotalDist;
					fread(&item.nTotalDist, 1, sizeof(stDistMatrix::nTotalDist), fp);
					//uint32_t nTotalTime;
					fread(&item.nTotalTime, 1, sizeof(stDistMatrix::nTotalTime), fp);
					//double dbTotalCost;
					fread(&item.dbTotalCost, 1, sizeof(stDistMatrix::dbTotalCost), fp);

					vtDistMatrixRow.emplace_back(item);
				} // for
				vtDistMatrix.emplace_back(vtDistMatrixRow);
			} // for

			ret = ROUTE_RESULT_SUCCESS;
		}
		fclose(fp);
	} // fp

	return ret;
}


uint32_t CTMSManager::SaveWeightMatrix(IN const char* szFileName, IN const int cntItem, IN const int sizeItem, IN const uint32_t crc, IN const vector<vector<stDistMatrix>>& vtDistMatrix)
{
	uint32_t ret = 0;

	if (szFileName == nullptr || vtDistMatrix.empty()) {
		return 0;
	}

	const int32_t dataSize = sizeof(stDistMatrix::nTotalDist) + sizeof(stDistMatrix::nTotalTime) + sizeof(stDistMatrix::dbTotalCost);
	FILE* fp = fopen(szFileName, "wb");
	if (fp) {
		// write count
		fwrite(&cntItem, 1, sizeof(cntItem), fp);

		// write data size
		fwrite(&sizeItem, 1, sizeof(sizeItem), fp);

		// write data crc
		fwrite(&crc, 1, sizeof(crc), fp);

		// write matrix
		for (int ii = 0; ii < cntItem; ii++) {
			for (int jj = 0; jj < cntItem; jj++) {
				//uint32_t nTotalDist;
				fwrite(&vtDistMatrix[ii][jj].nTotalDist, 1, sizeof(stDistMatrix::nTotalDist), fp);
				//uint32_t nTotalTime;
				fwrite(&vtDistMatrix[ii][jj].nTotalTime, 1, sizeof(stDistMatrix::nTotalTime), fp);
				//double dbTotalCost;
				fwrite(&vtDistMatrix[ii][jj].dbTotalCost, 1, sizeof(stDistMatrix::dbTotalCost), fp);
			} // for
		} // for

		fclose(fp);
	} // fp

	return 0;
}
