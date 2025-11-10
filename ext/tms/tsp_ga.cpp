#include "tsp_ga.h"

#include "../utils/UserLog.h"
#include <time.h>

#define USE_PARALLEL

int32_t g_valuType = TYPE_TSP_VALUE_DIST; // TYPE_TSP_VALUE_COST; // 0:cost, 1:dist, 2:time

////////////////////////////////////////////////////////////////////////////////////////////////////
// Chromosome
// Gene
Gene::Gene()
{
	attribute = 0;
	glued = -1;
}


Gene::Gene(IN const uint32_t attr)
{
	attribute = attr;
}


Gene::~Gene()
{

}


void Gene::SetAttribute(IN const uint32_t attr)
{
	attribute = attr;
}


uint32_t Gene::GetAttribute(void) const
{
	return attribute;
}


void Gene::SetGlued(IN const int32_t idxGlue)
{
	glued = idxGlue;
}


int32_t Gene::GetGlued(void) const
{
	return glued;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Individual
bool cmpFitness(Individual lhs, Individual rhs) 
{
	return lhs.GetFitness() < rhs.GetFitness(); // 올림차순 // fitness가 큰 개체 우선
}

bool cmpCost(Individual lhs, Individual rhs)
{
	return lhs.GetCost() < rhs.GetCost(); // 올림차순 // cost가 큰 개체 우선
}

bool cmpDist(Individual lhs, Individual rhs)
{
	return lhs.GetDist() < rhs.GetDist(); // 올림차순 // dist가 큰 개체 우선
}

bool cmpTime(Individual lhs, Individual rhs)
{
	return lhs.GetTime() < rhs.GetTime(); // 올림차순 // time가 큰 개체 우선
}

Individual::Individual()
{
	geneSize = 0;
	enableGeneSize = 0;

	m_start = -1;
	m_finish = -1;
}


Individual::Individual(IN const int32_t size)
{
	if (size > MAX_GENE) {
		geneSize = MAX_GENE;
	} else if (size < 0) {
		geneSize = 0;
	} else {
		geneSize = size;
	}
	chromosome.resize(geneSize);

	enableGeneSize = geneSize;

	parent1 = 0;
	parent2 = 0;
	mutate = 0;
}


Individual::~Individual()
{
	if (!chromosome.empty()) {
		chromosome.clear();
		vector<Gene>().swap(chromosome);
	}
}


void Individual::SetGeneSize(IN const int32_t size)
{
	if (size > MAX_GENE) {
		geneSize = MAX_GENE;
	} else if (size < 0) {
		geneSize = 0;
	} else {
		geneSize = size;
	}
	chromosome.resize(geneSize);

	enableGeneSize = geneSize;
}


int32_t Individual::GetGeneSize(bool isEnable)
{
	if (isEnable) {
		return enableGeneSize;
	} else {
		return geneSize;
	}
}


double Individual::CalculateCost(IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const int count, IN const double average)
{
	if (count < enableGeneSize) {
		LOG_TRACE(LOG_WARNING, "table rows not enough size compare with chromosome size, table(%d) < chromosome(%d)", count, enableGeneSize);
	}

	uint32_t prv, next;
	cost = dist = time = value = 0;

	//for (int ii = 0; ii < enableGeneSize - 1; ii++) {
	for (int ii = 0; ii < count - 1; ii++) {
		prv = chromosome[ii].GetAttribute();
		next = chromosome[ii+1].GetAttribute();

		if ((m_start >= 0) && (m_start == m_finish)) { // 출발지 원점회귀
			if (prv == vtDistMatrix.size()) {
				prv = 0;
			} else if (next == vtDistMatrix.size()) {
				next = 0;
			}
		} else if ((m_start != 0) && (m_start == m_finish)) {
			next = 0;
		}
#if defined(USE_REAL_ROUTE_TSP)
		cost += vtDistMatrix[prv][next].dbTotalCost;
		dist += vtDistMatrix[prv][next].nTotalDist;
		time += vtDistMatrix[prv][next].nTotalTime;
#else
		value += vtDistMatrix[prv][next].nTotalDist;
#endif
	} // for

//	// 원점회귀 때문에 시작지점을 다시 더해 계산한다.
//	if ((m_start >= 0) && (m_start == m_finish)) {
//		prv = chromosome[enableGeneSize - 1].GetAttribute(); // 마지막 유전자 위치
//		next = m_finish;// chromosome[0].GetAttribute(); // 마지막 유전자에서 시작 위치 계산
//#if defined(USE_REAL_ROUTE_TSP)
//		cost += ppResultTables[prv][next].dbTotalCost;
//		dist += ppResultTables[prv][next].nTotalDist;
//		time += ppResultTables[prv][next].nTotalTime;
//#else
//		value += ppResultTables[prv][next].nTotalDist;
//#endif
//	} 
//	// 도착지점이 고정일 경우 추가 계산한다.
//	else if (m_finish >= 0) {
//		prv = chromosome[enableGeneSize - 1].GetAttribute();
//		next = m_finish;
//#if defined(USE_REAL_ROUTE_TSP)
//		cost += ppResultTables[prv][next].dbTotalCost;
//		dist += ppResultTables[prv][next].nTotalDist;
//		time += ppResultTables[prv][next].nTotalTime;
//#else
//		value += ppResultTables[prv][next].nTotalDist;
//#endif
//	}

	boundary = average / cost;

	if (g_valuType == TYPE_TSP_VALUE_DIST) {
		value = dist;
	} else if (g_valuType == TYPE_TSP_VALUE_TIME) {
		value = time;
	} else {
		value = cost;
	}

	return value;
}


double Individual::GetCost(void) const
{
	return cost;
}

uint32_t Individual::GetDist(void) const
{
	return dist;
}

uint32_t Individual::GetTime(void) const
{
	return time;
}

double Individual::GetFitness(void) const
{
	return fitness;
}


double Individual::GetFValue(void) const
{
	return fvalue;
}


uint32_t Individual::GetChromosomeSize(void) const
{
	return geneSize;
}


vector<Gene>* Individual::GetChromosome(void)
{
	return &chromosome;
}


uint32_t Individual::GetMutate(void)
{
	return mutate;
}


int32_t Individual::GetStart(void) const
{
	return m_start;
}


int32_t Individual::GetFinish(void) const
{
	return m_finish;
}


void Individual::SetFitness(IN const double val)
{
	fitness = val;
}


void Individual::SetFValue(IN const double val)
{
	fvalue = val;
}


void Individual::SetParent(IN const uint32_t p1, IN const uint32_t p2)
{
	parent1 = p1;
	parent2 = p2;
}


void Individual::SetMutate(IN const uint32_t state)
{
	mutate = state;
}


void Individual::SetStartFinish(IN const int32_t nStart, IN const int32_t nEnd)
{
	m_start = nStart;
	m_finish = nEnd;

	// 출발지 원점회귀
	if ((m_start >= 0) && (m_start == m_finish)) {
		// 첫 유전자 생성
		chromosome[0].SetGlued(m_start);

		// 마지막 유전자는 이후로 컨트롤하지 말자
		//enableGeneSize--;

		// 마지막 유전자 생성
		chromosome[GetGeneSize() - 1].SetGlued(m_finish);
	}
	// 출-도착지 고정
	else if ((m_start >= 0) && (m_finish >= 0) && (m_start != m_finish)) {
		// 첫 유전자 생성
		chromosome[0].SetGlued(m_start);

		// 마지막 유전자는 이후로 컨트롤하지 말자
		//enableGeneSize--;

		// 마지막 유전자 생성
		chromosome[GetGeneSize() - 1].SetGlued(m_finish);
	}
	// 출발지 고정
	else if (m_start >= 0) {
		// 첫 유전자 생성
		chromosome[0].SetGlued(m_start);
	}
	// 도착지 고정
	else if (m_finish >= 0) {
		// 마지막 유전자는 이후로 컨트롤하지 말자
		//enableGeneSize--;

		// 마지막 유전자 생성
		chromosome[GetGeneSize() - 1].SetGlued(m_finish);
	} else {

	}
}


struct stCandidateWaypoint
{
	uint32_t idx;
	uint32_t depth;
	double x, y;
	double dist;
	double time;

	vector<uint32_t> vtBestWay;
	vector<uint32_t> vtVisited;

	stCandidateWaypoint()
	{
		idx = depth = 0;
		x = y = 0.f;
		dist = time = 0.f;
	}
};


static auto CompareWaypoint = [](const stCandidateWaypoint* lhs, const stCandidateWaypoint* rhs) {
	//if (lhs->depth < rhs->depth) {
	//	return true;
	//} else if (lhs->depth == rhs->depth) {
	//	return lhs->dist > rhs->dist;
	//} else {
	//	return false;
	//}
	if (lhs->dist > rhs->dist) {
		return true;
	} else if (lhs->dist == rhs->dist) {
		return lhs->dist > rhs->dist;
	} else {
		return false;
	}
};


bool Individual::Propagation(IN const vector<tableItemNearlest>* pNearlest, IN const int32_t start, IN const int32_t finish, IN const uint32_t maxLoop, OUT vector<uint32_t>& vtBestWay)
{
	bool ret = false;

	priority_queue<stCandidateWaypoint*, vector<stCandidateWaypoint*>, decltype(CompareWaypoint)> pqDijkstra{ CompareWaypoint };

	stCandidateWaypoint *pNewItem = nullptr;

	int idxStart = 0;
	int idxEnd = geneSize - 1;

	// 출발지 원점회귀
	if ((m_start >= 0) && (m_start == m_finish)) {
		
	}
	// 출-도착지 고정
	else if ((m_start >= 0) && (m_finish >= 0) && (m_start != m_finish)) {
		
	}
	// 출발지 고정
	else if (m_start >= 0) {
		pNewItem = new stCandidateWaypoint();
		pNewItem->idx = m_start;
		pNewItem->vtBestWay.reserve(geneSize);
		pNewItem->vtBestWay.emplace_back(m_start);
		pNewItem->vtVisited.resize(geneSize);
		pNewItem->vtVisited[m_start] = true;
		pqDijkstra.push(pNewItem);
	}
	// 도착지 고정
	else if (m_finish >= 0) {
		
	} else {

	}


	int nMaxNode = 5; //1;// 2 + maxLoop; // 처음에는 각 정점에서 확장 가능한 다음 정점을 3개로 제한

	stCandidateWaypoint* pItem = nullptr;
	stCandidateWaypoint* pSuccess = nullptr;

	double dwMinDist = DBL_MAX;
	int curRepeat = 0;
	int maxRepeat = 1000;
	int nMaxDepth = 0;
	while(!pqDijkstra.empty())
	{
		pItem = pqDijkstra.top();
		pqDijkstra.pop();

		if (nMaxDepth < pItem->depth) {
			nMaxDepth = pItem->depth;
		}

		// 종료 조건
		if (pItem->depth == idxEnd) {
			curRepeat++;
			if (dwMinDist > pItem->dist) {
				SAFE_DELETE(pSuccess);

				dwMinDist = pItem->dist;
				pSuccess = pItem;
			} else {
				SAFE_DELETE(pItem);
			}

			if (curRepeat >= maxRepeat) {
				break;
			} else {
				continue;
			}
		} else {
			
		}


		int cntAdded = 0;
		int lastIdx = 0;
		int prevIdx = pItem->idx;
		//double prevDist = 0.f;

		for (int ii = 0; ii < nMaxNode; ii++) {
			for (int jj = lastIdx + 1; jj < pNearlest->size(); jj++) {
				int nextNearIdx = pNearlest->at(prevIdx).nearlest[jj].idx;
				if (pItem->vtVisited[nextNearIdx] == false) {
					// 이전과 거리 비교 후 비슷한 거리일 경우만 분기
					//if (ii > 0 && prevDist > 0.f && lastIdx > 0) {
					//	// 10% 이하일 때만 분기
					//	if ((prevDist * 1.1) < pNearlest->at(prevIdx).nearlest[jj].dist) {
					//		break;
					//	} else {
					//		lastIdx = jj;
					//	}
					//}
					lastIdx = jj;

					// add
					pNewItem = new stCandidateWaypoint();
					pNewItem->idx = nextNearIdx;
					pNewItem->depth = pItem->depth + 1;
					pNewItem->dist = pItem->dist + pNearlest->at(pItem->idx).nearlest[lastIdx].dist;
					pNewItem->time = pItem->time + pNearlest->at(pItem->idx).nearlest[lastIdx].time;
					pNewItem->vtBestWay.assign(pItem->vtBestWay.begin(), pItem->vtBestWay.end());
					pNewItem->vtBestWay.push_back(nextNearIdx);
					pNewItem->vtVisited.assign(pItem->vtVisited.begin(), pItem->vtVisited.end());
					pNewItem->vtVisited[nextNearIdx] = true;
					pqDijkstra.push(pNewItem);

					//prevDist = pNearlest->at(pItem->idx).nearlest[lastIdx].dist;
					cntAdded++;

					break;
				}
			}
		}

		// 등록된게 없으면 삭제
		if (cntAdded <= 0) {
			SAFE_DELETE(pItem);
		}

	} // while

	if (pSuccess) {
		vtBestWay.assign(pSuccess->vtBestWay.begin(), pSuccess->vtBestWay.end());
		ret = true;
	} else {
		LOG_TRACE(LOG_DEBUG, "---------- max bestway dijkstra depth : %d", nMaxDepth);
	}

	// 큐 정리
	while (!pqDijkstra.empty()) {
		pItem = pqDijkstra.top();
		pqDijkstra.pop();

		SAFE_DELETE(pItem);
	}

	return ret;
}


bool Individual::Create(IN const vector<tableItemNearlest>* pNearlest, IN const int32_t start, IN const int32_t finish, IN const uint32_t idx)
{
	int32_t prev, curr, preVal;
	int32_t pos = start;
	vector<bool> created(geneSize, false); // 유전자 배열 생성 여부
	vector<bool> visited(geneSize, false); // 이웃 index 사용 여부
	const int32_t orderRate = 50; // 30; 우선순위 30%내에서 선택 // 1,2,3 순위를 랜덤으로 가져오도록

	m_start = start;
	m_finish = finish;

	if (geneSize <= start) {
		LOG_TRACE(LOG_WARNING, "start position value over than capacity, capacity(%d) <= start(%d)", geneSize, start);
		return false;
	}

	pos = 0;
	curr = 0;
	prev = 0;

	bool isReverse = false;
	// 2번에 1번은 거꾸로 진행해 보자
	if (idx % 2 == 0) {
		isReverse = true;
	}

	int idxStart = 0;
	int idxEnd = geneSize - 1;

	if (0) {
		const int nMaxLoopDepth = 5; // 
		bool ret = false;
		vector<uint32_t> resultBestWay(geneSize);
		for (int ii = 0; ii < nMaxLoopDepth; ii++) {
			ret = Propagation(pNearlest, start, finish, ii, resultBestWay);
			if (ret == true) {
				break;
			} else {
				ret = false;
			}
		}

		for (int ii = 0; ii < resultBestWay.size(); ii++) {
			chromosome[ii].SetAttribute(resultBestWay[ii]);

			// 출발지 원점회귀
			if ((m_start >= 0) && (m_start == m_finish)) {
				// 첫 유전자 생성
				chromosome[pos].SetGlued(m_start);

				// 마지막 유전자는 이후로 컨트롤하지 말자
				//enableGeneSize--;

				// 마지막 유전자 생성
				chromosome[pos].SetGlued(idxEnd);
			}
			// 출-도착지 고정
			else if ((m_start >= 0) && (m_finish >= 0) && (m_start != m_finish)) {
				// 첫 유전자 생성
				chromosome[pos].SetGlued(m_start);

				// 마지막 유전자는 이후로 컨트롤하지 말자
				//enableGeneSize--;

				// 마지막 유전자 생성
				chromosome[pos].SetGlued(idxEnd);
			}
			// 출발지 고정
			else if (m_start >= 0) {
				// 첫 유전자 생성
				chromosome[pos].SetGlued(m_start);
			}
			// 도착지 고정
			else if (m_finish >= 0) {
				// 마지막 유전자는 이후로 컨트롤하지 말자
				//enableGeneSize--;

				// 마지막 유전자 생성
				chromosome[pos].SetGlued(idxEnd);
			} else {

			}
		}
	} else {
		// 출발지 원점회귀
		if ((m_start >= 0) && (m_start == m_finish)) {
			// 첫 유전자 고정
			pos = m_start; // first
			curr = m_start; // first
			chromosome[pos].SetGlued(curr);
			chromosome[pos].SetAttribute(curr);
			created[pos] = visited[curr] = true;
			idxStart++;

			// 마지막 유전자는 이후로 컨트롤하지 말자
			//enableGeneSize--;

			// 마지막 고정
			pos = idxEnd; // last
			curr = idxEnd; // last
			chromosome[pos].SetGlued(m_start);
			chromosome[pos].SetAttribute(m_start);
			created[pos] = visited[curr] = true;
			idxEnd--;

			// 첫 유전자 생성
			if (isReverse) {
				// 마지막 유전자 생성
				pos = idxEnd; // 마지막 유전자부터 시작
				if (idxEnd > 1) {
					curr = (rand() % (idxEnd - 1)) + 1; // 처음과 마지막을 제외한 랜덤 위치
				}				
				chromosome[pos].SetAttribute(curr);
				created[pos] = visited[curr] = true;
				idxEnd--;
			} else {
				// 첫 유전자 생성
				pos = idxStart; // 첫 유전자부터 시작
				if (idxEnd > 1) {
					curr = (rand() % (idxEnd - 1)) + 1; // 처음과 마지막을 제외한 랜덤 위치
				}				
				chromosome[pos].SetAttribute(curr);
				created[pos] = visited[curr] = true;
				idxStart++;
			}

			//// 마지막부터 가까운 놈을 지정하자
			//if (isReverse) {
			//	// 마지막 유전자 생성
			//	pos = idxEnd; // 마지막 유전자부터 시작
			//	//if (idx % 3 == 0) {
			//	if (1) {
			//		curr = (rand() % (idxEnd - 1)) + 1; // 처음과 마지막을 제외한 랜덤 위치
			//	} else {
			//		curr = pNearlest->at(0).nearlest[0].idx;// 처음에 가장 가까운 녀석 사용하자
			//	}
			//	chromosome[pos].SetAttribute(curr);
			//	created[pos] = visited[curr] = true;
			//	idxEnd--;
			//} else {

			//}
		}
		// 출-도착지 고정
		else if ((m_start >= 0) && (m_finish >= 0) && (m_start != m_finish)) {
			// 첫 유전자 고정
			pos = m_start; // first
			curr = m_start; // first
			chromosome[pos].SetGlued(curr);
			chromosome[pos].SetAttribute(curr);
			created[pos] = visited[curr] = true;
			idxStart++;

			// 마지막 유전자는 이후로 컨트롤하지 말자
			//enableGeneSize--;

			// 마지막 유전자 고정
			pos = idxEnd; // last
			curr = idxEnd; // last
			chromosome[pos].SetGlued(curr);
			chromosome[pos].SetAttribute(curr);
			created[pos] = visited[curr] = true;
			idxEnd--;

			// 첫 유전자 생성
			if (isReverse) {
				// 마지막 유전자 생성
				pos = idxEnd; // 마지막 유전자부터 시작
				if (idxEnd > 1) {
					curr = (rand() % (idxEnd - 1)) + 1; // 처음과 마지막을 제외한 랜덤 위치
				}
				chromosome[pos].SetAttribute(curr);
				created[pos] = visited[curr] = true;
				idxEnd--;
			} else {
				// 첫 유전자 생성
				pos = idxStart; // 첫 유전자부터 시작
				if (idxEnd > 1) {
					curr = (rand() % (idxEnd - 1)) + 1; // 처음과 마지막을 제외한 랜덤 위치
				}				
				chromosome[pos].SetAttribute(curr);
				created[pos] = visited[curr] = true;
				idxStart++;
			}
		}
		// 출발지 고정
		else if (m_start >= 0) {
			// 첫 유전자 고정
			pos = m_start; // first
			curr = m_start; // first
			chromosome[pos].SetGlued(curr);
			chromosome[pos].SetAttribute(curr);
			created[pos] = visited[curr] = true;
			idxStart++;

			// 첫 유전자 생성
			if (isReverse) {
				// 마지막 유전자 생성
				pos = idxEnd; // 마지막 유전자부터 시작
				if (idxEnd > 1) {
					curr = (rand() % (idxEnd - 1)) + 1; // 처음과 마지막을 제외한 랜덤 위치
				}				
				chromosome[pos].SetAttribute(curr);
				created[pos] = visited[curr] = true;
				idxEnd--;
			}
		}
		// 도착지 고정
		else if (m_finish >= 0) {
			// 마지막 유전자는 이후로 컨트롤하지 말자
			//enableGeneSize--;

			// 마지막 유전자 고정
			pos = idxEnd; // last
			curr = idxEnd; // last
			chromosome[pos].SetGlued(curr);
			chromosome[pos].SetAttribute(curr);
			created[pos] = visited[curr] = true;
			idxEnd--;

			// 첫 유전자 생성
			if (isReverse) {
				//pos = idxEnd; // 마지막 유전자부터 시작
				//curr = chromosome[pos].GetAttribute(); // 도착지와 가까운 놈으로 지정
			} else {
				// 첫 유전자 생성
				pos = idxStart; // 첫 유전자부터 시작
				if (idxEnd > 1) {
					curr = (rand() % (idxEnd - 1)) + 1; // 처음과 마지막을 제외한 랜덤 위치
				} else {
					curr = idxEnd;
				}
				chromosome[pos].SetAttribute(curr);
				created[pos] = visited[curr] = true;
				idxStart++;
			}
		} else {
			// 첫 유전자 생성
			if (isReverse) {
				// 마지막 유전자 생성
				pos = idxEnd; // 마지막 유전자부터 시작
				if (idxEnd > 1) {
					curr = (rand() % (idxEnd - 1)); //마지막을 제외한 랜덤 위치
				}				
				chromosome[pos].SetAttribute(curr);
				created[pos] = visited[curr] = true;
				idxEnd--;
			} else {
				// 첫 유전자 생성
				pos = 0;
				if (idxEnd > 1) {
					curr = (rand() % idxEnd) + 1; // 처음을 제외한 랜덤 위치
				}				
				chromosome[pos].SetAttribute(curr);
				created[pos] = visited[curr] = true;
				idxStart++;
			}
		}



		//// 도착지 고정이면 도착지부터 거꾸로 가보자 (2번에 1번꼴로)
		//if ((m_finish >= 0) && (m_start != m_finish) && (idx % 2 == 0)) {
		//	isReverse = true;
		//	prev = m_finish;
		//} 
		//// 원점 회귀일 경우
		//else if ((m_start >= 0) && (m_start == m_finish) && (idx % 2 == 0)) {
		//	isReverse = true;
		//	prev = enableGeneSize - 1;
		//} else {
		//	prev = 0;
		//}

		for (int ii = idxStart; ii <= idxEnd; ii++) {
			//for (int ii = 1; ii < enableGeneSize; ii++, prev++) {
				//uint32_t pos = rand() % geneSize;
				//while (created[pos] == true) {
				//	pos = (pos + 1) % geneSize;
				//}
				//prev = ii - 1;
				//if (prev < 0) {
				//	prev = 0;
				//}

			preVal = chromosome[pos].GetAttribute();

			// 가까운 유전자를 미사용된 순서로 가져온다.
			uint32_t nearlest = 0;
			int order = 0; // 첫 개체는 제일 가까운 값을 사용
			if (idx != 0) {
				//if (0) {
					// 두번째 유전자는 첫번째에서 거리순 절반까지 유전자를 대입해 준다. 
					//if ((prev == 0) && (idx < (geneSize / 2 - 1))) {
				if (enableGeneSize <= 30 && idx <= 30) {
					//if ((pos == 0) && (idx < (enableGeneSize / 2 - 1))) {
					//	preVal = curr;
					//	order = idx;
					//}
					////else if (idx < (geneSize / 2 - 1)){
					//else if (enableGeneSize / 2 - 1) {
					//	;
					//}
					//else
					//{
					//	order = rand() % (enableGeneSize * orderRate / 100); // 우선순위 30%내에서 선택 // 1,2,3 순위를 랜덤으로 가져오도록
					//	//order = rand() % (geneSize * (idx % 10) * 10 / 100); // 우선순위 10%~100% 내에서 선택 // 1,2,3 순위를 랜덤으로 가져오도록
					//}
				} else {
					order = (rand() % 5); //3->5 2025-09-23 변경 // 우선순위 30%내에서 선택 // 1,2,3 순위를 랜덤으로 가져오도록
					//order = (rand() % enableGeneSize) * orderRate / 100; // 우선순위 30%내에서 선택 // 1,2,3 순위를 랜덤으로 가져오도록
					//order = rand() % (geneSize * (idx % 10) * 10 / 100); // 우선순위 10%~100% 내에서 선택 // 1,2,3 순위를 랜덤으로 가져오도록
				}
			}


			bool isOk = false;

			// find 1st nearlest
			// 현위치에서 1번째 가까운 곳을 찾고
			curr = pNearlest->at(preVal).nearlest[order].idx;
			if (visited[curr] == false) {
				isOk = true;
			} else { // 아니면 가까운 우선순위 부터 순차로 가져와보자
				for (order = 0; order < enableGeneSize; order++) {
					curr = pNearlest->at(preVal).nearlest[order].idx;
					if (visited[curr] == true) {
						continue;
					} else { //if (visited[nearlest] == false) {
						isOk = true;
						break;
					}
				} // for
			}

			// find 2nd nearlest
			// 현위치에서 2번째 가까운 곳을 찾고, 
			// 1번째의 다음이 2번째가 아닐경우, 2번째를 먼저 가보고 2번째 가까운 곳이 다시 1번째 가까운 곳으로 확인되면 2번째를 먼저 지나자
			if (1) { //isOk && (order < idxEnd)) {
				int n1stNext = 0;
				int n2ndCurr = 0;
				for (order = 1; order < enableGeneSize; order++) {
					// 1번째의 2번째
					n1stNext = pNearlest->at(curr).nearlest[order].idx;
					if ((n1stNext == curr) && (visited[n1stNext] == true)) {
						continue;
					} else {
						for (order = 1; order < enableGeneSize - 1; order++) {
							// 현위치의 2번째
							n2ndCurr = pNearlest->at(preVal).nearlest[order].idx;
							if ((n2ndCurr == curr) || (visited[n2ndCurr] == true)) {
								continue;
							} else if (n1stNext != n2ndCurr) {
								// 1번째의 다음이 2번째가 아닐경우
								int n2ndNext = 0;
								for (order = 1; order < enableGeneSize - 1; order++) {
									n2ndNext = pNearlest->at(n2ndCurr).nearlest[order].idx;
									if (visited[n2ndNext] == true) {
										continue;
									} else if (n2ndNext == curr) {
										// 2번째 가까운 곳이 1번째 가까운 곳이면 2번째 가까운 곳을 먼저 지나자
										curr = n2ndCurr;
									}
									break;
								} // for
							}
							break;
						} // for
					}
					break;
				} // for
			}


			if (isReverse) {
				pos--;
			} else {
				pos++;
			}

			if (isOk) {
				chromosome[pos].SetAttribute(curr);
				created[pos] = visited[curr] = true;
			} else {
				// 여기는 들어오면 안되는 곳이야.
				LOG_TRACE(LOG_WARNING, "can't find chromosome nearlest matching index, idx:%d", ii);
			}
		} // for
	}

	return true;
}


#if defined(SHOW_STATUS)
void Individual::Print(void)
{
	LOG_TRACE(LOG_DEBUG_CONTINUE, "p(%2d, %2d) %d, ", parent1, parent2, mutate);

	int cnt = 0;
	for (const auto & gene : chromosome) {
		LOG_TRACE(LOG_DEBUG_CONTINUE, "%2d>", gene.GetAttribute()); //→
	}
	LOG_TRACE(LOG_DEBUG_CONTINUE, "%d => %d, ==> %.5f", chromosome[0].GetAttribute(), value, boundary);
}
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// Environment
Environment::Environment()
{
	generation = 0;

	ppTables = nullptr;
	tableCount = 0;
	fitnessSum = 0.f;
	averageValue = 0.f;
	bestValue = 0.f;
	worstValue = 0.f;
}


Environment::~Environment()
{
	if (!population.empty()) {
		population.clear();
		vector<Individual>().swap(population);
	}
}


void Environment::SetOption(IN const TspOption* pOpt)
{
	if (pOpt == nullptr) {
		LOG_TRACE(LOG_WARNING, "tsp setting option null is null");
	}

	tspOption = *pOpt;

	if (tspOption.compareType != TYPE_TSP_VALUE_DIST) {
		g_valuType = tspOption.compareType;
	}
}


bool cmpNearlestGene(const tableItem& lhs, const tableItem& rhs) {
	if (g_valuType == TYPE_TSP_VALUE_DIST) {
		return lhs.dist < rhs.dist;
	} else if (g_valuType == TYPE_TSP_VALUE_TIME) {
		return lhs.time < rhs.time;
	} else {
		return lhs.cost < rhs.cost;
	}
}


bool Environment::SetCostTable(IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const int count)
{
	if (vtDistMatrix.empty()) {
		LOG_TRACE(LOG_WARNING, "input table pointer is null");
		return false;
	}
	else if (count <= 0 || count > MAX_GENE) {
		LOG_TRACE(LOG_WARNING, "input table count not valid, %d, max(%d)", count, MAX_GENE);
		return false;
	}


	ppTables = const_cast<vector<vector<stDistMatrix>>*>(&vtDistMatrix);
	tableCount = count;

	uint32_t maxGene = tableCount;

	// 출발지 고정 & 원점 회귀
	if (tspOption.endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
		// 회귀는 마지막 강제 추가
		tableCount++;
		maxGene++;
	}
	// 출발지-도착지 고정
	else if (tspOption.endpointType == TYPE_TSP_ENDPOINT_START_END) {
		//maxGene = tableCount - 1;
		maxGene = tableCount;
	}	
	// 출발지 고정
	else if (tspOption.endpointType == TYPE_TSP_ENDPOINT_START) {
		maxGene = tableCount;
	}
	// 도착지 고정
	else if (tspOption.endpointType == TYPE_TSP_ENDPOINT_END) {
		//maxGene = tableCount - 1;
		maxGene = tableCount;
	} else {
		maxGene = tableCount;
	}


	genePriority.clear();
	genePriority.reserve(maxGene - 1); // 자기는 뺀 이웃개수

	for (int ii = 0; ii < maxGene; ii++) {
		uint32_t min = UINT32_MAX;
		uint32_t max = 0;

		tableItemNearlest currentGene;
		if ((tspOption.endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) && (ii == maxGene - 1)) {
			// 회귀는 마지막 강제 추가된 염색체를 첫번째 염색체와 동일하게 적용
			currentGene.idx = 0;
		} else {
			currentGene.idx = ii;
		}

		for (int jj = 0; jj < maxGene; jj++) {
			tableItem neighborGene;
			if ((tspOption.endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) && (jj == maxGene - 1)) {
				// 회귀는 마지막 강제 추가된 염색체를 첫번째 염색체와 동일하게 적용
				currentGene.idx = 0;
			} else {
				neighborGene.idx = jj;
			}

			if (currentGene.idx != neighborGene.idx) {
#if defined(USE_REAL_ROUTE_TSP)
				if (g_valuType == TYPE_TSP_VALUE_DIST) {
					if (ppTables->at(currentGene.idx)[neighborGene.idx].nTotalDist < min) {
						min = ppTables->at(currentGene.idx)[neighborGene.idx].nTotalDist;
					}
					if (ppTables->at(currentGene.idx)[neighborGene.idx].nTotalDist > max) {
						max = ppTables->at(currentGene.idx)[neighborGene.idx].nTotalDist;
					}
				} else if (g_valuType == TYPE_TSP_VALUE_TIME) {
					if (ppTables->at(currentGene.idx)[neighborGene.idx].nTotalTime < min) {
						min = ppTables->at(currentGene.idx)[neighborGene.idx].nTotalTime;
					}
					if (ppTables->at(currentGene.idx)[neighborGene.idx].nTotalTime > max) {
						max = ppTables->at(currentGene.idx)[neighborGene.idx].nTotalTime;
					}
				} else {
					if (ppTables->at(currentGene.idx)[neighborGene.idx].dbTotalCost < min) {
						min = ppTables->at(currentGene.idx)[neighborGene.idx].dbTotalCost;
					}
					if (ppTables->at(currentGene.idx)[neighborGene.idx].dbTotalCost > max) {
						max = ppTables->at(currentGene.idx)[neighborGene.idx].dbTotalCost;
					}
				}

				neighborGene.cost = ppTables->at(currentGene.idx)[neighborGene.idx].dbTotalCost;
				neighborGene.dist = ppTables->at(currentGene.idx)[neighborGene.idx].nTotalDist;
				neighborGene.time = ppTables->at(currentGene.idx)[neighborGene.idx].nTotalTime;
#else
				if (ppTables->at(currentGene.idx)[neighborGene.idx].nTotalDist < min) {
					min = ppTables->at(currentGene.idx)[neighborGene.idx].nTotalDist;
				}

				if (ppTables->at(currentGene.idx)[neighborGene.idx].nTotalDist > max) {
					max = ppTables->at(currentGene.idx)[neighborGene.idx].nTotalDist;
				}

				neighborGene.idx = neighborGene.idx;
				neighborGene.value = ppTables->at(currentGene.idx)[neighborGene.idx].nTotalDist;
				currentGene.nearlest.emplace_back(neighborGene);
#endif
			}

			currentGene.nearlest.emplace_back(neighborGene);
		} // for jj


		// 현재 유전자와 가까운 유전자 벡터 생성
		sort(currentGene.nearlest.begin(), currentGene.nearlest.end(), cmpNearlestGene);
		genePriority.emplace_back(currentGene);

		bestValue += min;
		worstValue += max;
	} // for ii

	averageValue = static_cast<double>(bestValue + worstValue) / 2.f;

	LOG_TRACE(LOG_DEBUG, "table best: %.2f, worst: %.2f, average: %.2f", bestValue, worstValue, averageValue);

	return true;
}


void Environment::Genesis(IN const uint32_t maxGene, IN const uint32_t maxPopulation)
{
	//srand(time(NULL));
	//srand(1000 + 49 * 2);
	srand(tspOption.seed);

	nMaxPopulation = maxPopulation;

	if (nMaxPopulation > MAX_INDIVIDUAL) {
		nMaxPopulation = MAX_INDIVIDUAL;
	}

	int cntGene = max(maxGene, maxPopulation);
	for (int ii = 0; ii < cntGene; ii++) {
		Individual newIndi(maxGene);

		vector<int32_t> vtOrder;// (maxGene, -1); // 사용된 이웃 index 값, 0:자기자신, 0<x:이전위치에서의 index(가까운 정도)

		int nStart = -1;
		int nEnd = -1;

		// 출발지 고정 & 원점 회귀
		if (tspOption.endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) {
			// 회귀는 마지막 강제 추가
			newIndi.SetGeneSize(maxGene + 1);
			nStart = 0;
			nEnd = 0;
		}
		// 출발지-도착지 고정
		else if (tspOption.endpointType == TYPE_TSP_ENDPOINT_START_END) {
			nStart = 0;
			nEnd = maxGene - 1;
		}
		// 출발지 고정
		else if (tspOption.endpointType == TYPE_TSP_ENDPOINT_START) {
			nStart = 0;
			nEnd = -1;
		}
		// 도착지 고정
		else if (tspOption.endpointType == TYPE_TSP_ENDPOINT_END) {
			nStart = -1;
			nEnd = maxGene - 1;
		} 
		// 미지정
		else {
			nStart = -1;
			nEnd = -1;
		}

		newIndi.Create(&genePriority, nStart, nEnd, ii);
		population.emplace_back(newIndi);
	}

	generation++; // 세대 시작	
}


const double Environment::Evaluation(void)
{
	double min = UINT64_MAX;
	double max = 0;
	double value;

	for (auto & item : population) {
		value = item.CalculateCost(*ppTables, tableCount, averageValue);
		if (value < min) {
			min = value;
		}
		if (value > max) {
			max = value;
		}

		item.SetFValue(value);
	}

	//bestCost = min;
	//worstCost = max;
	//averageCost = static_cast<double>(min + max) / 2;


	const int32_t mulRate = 3 - 1;
	double fitness = 0.f;
	fitnessSum = 0.f;
	for (auto & item : population) {
		if (worstValue == bestValue) {
			fitness = RAND_MAX / (worstValue / mulRate) * 100;
		} else {
			fitness = RAND_MAX / ((worstValue - item.GetFValue()) + (worstValue - bestValue) / (mulRate)) * 100;
		}
		fitnessSum += fitness;

		item.SetFitness(fitness);
	} // for

	sort(population.begin(), population.end(), cmpFitness);
	value = population[0].GetFitness();

	//if (g_valuType == TYPE_TSP_VALUE_DIST) {
	//	sort(population.begin(), population.end(), cmpDist);
	//	value = population[0].GetDist();
	//} else if (g_valuType == TYPE_TSP_VALUE_DIST) {
	//	sort(population.begin(), population.end(), cmpTime);
	//	value = population[0].GetTime();
	//} else if (g_valuType == TYPE_TSP_VALUE_DIST) {
	//	sort(population.begin(), population.end(), cmpCost);
	//	value = population[0].GetCost();
	//} else {
	//	sort(population.begin(), population.end(), cmpFitness);
	//	value = population[0].GetFitness();
	//}

	return value;
}


void Environment::Selection(OUT vector<Parents>& pairs)
{
	int32_t cntParents = nMaxPopulation / 2;


	// fitness
	// f : 적합도

	//const int32_t mulRate = 3 - 1;
	//double fitness = 0.f;
	//double fitnessSum = 0.f;
	//for (auto & item : population) {
	//	fitness = RAND_MAX / ((worstCost - item.GetFValue()) + (worstCost - bestCost) / (mulRate))  * 100;
	//	fitnessSum += fitness;

	//	item.SetFitness(fitness);
	//}

	//sort(population.begin(), population.end(), cmpFitness);

	if (!pairs.empty()) {
		pairs.clear();
	}
	pairs.reserve(cntParents);

	Parents newParents;

	// elitism
	// 최상위 개체는 무조건 부모로 선택된다
	newParents.idxL = 0;
	newParents.idxR = (rand() % population.size() - 1) + 1;
	newParents.parentL = population[newParents.idxL];
	newParents.parentR = population[newParents.idxR];
	pairs.emplace_back(newParents);


	for (int pop = 0; pop < cntParents - 1; pop++) {

		// rullet wheel selection
		//
		// f = (Cw-Ci) + (Cw-Cb) / (k - 1), k > 1 , 일반적으로 k= 3~4 적용
		// Cw : 해집단 내에서 가장 나쁜 해의 비용
		// Cb : 해집단 내에서 가장 좋은 해의 비용
		// Ci : 해 i 의 비용

		array<int32_t, 2> parents = { 0, };

		for (auto & parent : parents) {
			int idx = 0;
			int fs = static_cast<uint32_t>(fitnessSum);
			if (fs <= 0) fs = 1;
			double sum = 0;
			int32_t point = rand() % fs;

			for (const auto & item : population) {
				sum += item.GetFitness();
				if (point < sum) {
					parent = idx;
					break;
				}
				idx++;
			}
		} // for


		if (parents[0] == parents[1]) {
			if (parents[1] < population.size() - 1) {
				parents[1] = parents[1] + 1;
			}
			else {
				parents[1] = 0;
			}
		}

		//
		// rullet wheel selection
		newParents.idxL = parents[0];
		newParents.idxR = parents[1];
		newParents.parentL = population[parents[0]];
		newParents.parentR = population[parents[1]];
		pairs.emplace_back(newParents);
	} // for

	// print
//	Print();
//
//	// print pairs
//	int cnt = 0;
//	LOG_TRACE(LOG_DEBUG_CONTINUE, "==================================================\n");
//	LOG_TRACE(LOG_DEBUG_CONTINUE, "pairs : %d\n", pairs.size());
//		LOG_TRACE(LOG_DEBUG_CONTINUE, "idx(%2d) : parentL(%2d) : %.5f, parentR(%2d) : %.5f \n", cnt++, pair.idxL, pair.parentL.GetFitness(), pair.idxR, pair.parentR.GetFitness());
//#if !defined(_DEBUG)
//		if (cnt > 3) break;
//#endif
//	}
//	LOG_TRACE(LOG_DEBUG_CONTINUE, "==================================================\n");
}


int32_t getFisrstVisit(IN unordered_set<uint32_t>& visited, IN const uint32_t index, IN const int32_t maxLoop)
{
	int32_t idx = -1;
	int32_t cur = index;

	// 단순 체크
	if (maxLoop < 0) {
		if (visited.find(cur) == visited.end()) {
			visited.insert(cur);
			idx = cur;
		}
	}
	else {
		unordered_set<int32_t>::const_iterator it;
		for (int32_t cnt = 0; cnt < maxLoop; cnt++) {
			if (visited.find(cur) == visited.end()) {
				visited.insert(cur);
				idx = cur;
				break;
			}

			cur++;

			// loop
			if (cur >= maxLoop) {
				cur = 0;
			}
		}
	}
	
	return idx;
}


// 서로 이웃한 유전자끼리 교차시 거리가 짧은 경우 교차시도
bool Environment::CrossBySelf(IN Individual* pSelf, IN const int32_t pos, IN const int32_t size, IN const int32_t off) {
	//offsprings.emplace_back(population[ii]);
	vector<Gene>* pGenes = pSelf->GetChromosome();
	double costBefor, costAfter;
	uint32_t distBefor, distAfter;
	uint32_t prev, next;
	int32_t start = pos;
	int32_t cnt = size;

	if (pSelf->GetMutate() == off) {
		return false;
	}

	if (pos < 0 || size <= 0) {
		start = 1; cnt = pSelf->GetChromosomeSize() - 3;
	}

	if (off > 1) {
		cnt -= (off - 1);
	}

	for (int jj = start; jj < cnt; jj++) {
		// 고정된 유전자는 변경하지 말것.
		if (pGenes->at(jj).GetGlued() >= 0) {
			continue;
		}

		costBefor = distAfter = 0;
		distBefor = distAfter = 0;

		// befor
		for (int kk = jj - 1; kk < jj + off; kk++) {
			prev = pGenes->at(kk).GetAttribute();
			next = pGenes->at(kk + 1).GetAttribute();

			if (tspOption.endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) { // 원점회귀
				if (prev == pGenes->size() - 1) {
					prev = 0;
				} else if (next == pGenes->size() - 1) {
					next = 0;
				}
			}
#if defined(USE_REAL_ROUTE_TSP)
			costBefor = ppTables->at(prev)[next].dbTotalCost;
			distBefor = ppTables->at(prev)[next].nTotalDist;
			//distBefor = ppTables[prev][next].nTotalTime;
#else
			costBefor = ppTables->at(prev)[next].dbTotalCost;
			distBefor = ppTables->at(prev)[next].nTotalDist;
#endif
		}

		// cross
		int fval = pGenes->at(jj).GetAttribute();
		int bval = pGenes->at(jj + off).GetAttribute();
		pGenes->at(jj).SetAttribute(bval);
		pGenes->at(jj + off).SetAttribute(fval);

		// after
		for (int kk = jj - 1; kk < jj + off; kk++) {
			prev = pGenes->at(kk).GetAttribute();
			next = pGenes->at(kk + 1).GetAttribute();

			if (tspOption.endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) { // 원점회귀
				if (prev == pGenes->size() - 1) {
					prev = 0;
				} else if (next == pGenes->size() - 1) {
					next = 0;
				}
			}
#if defined(USE_REAL_ROUTE_TSP)
			costAfter = ppTables->at(prev)[next].dbTotalCost;
			distAfter = ppTables->at(prev)[next].nTotalDist;
			//distAfter = ppTables[prev][next].nTotalTime;
#else
			costAfter = ppTables->at(prev)[next].dbTotalCost;
			distAfter = ppTables->at(prev)[next].nTotalDist;
#endif
		}

		// check
		//if (distBefor > distAfter) {
		if (costBefor > costAfter) {
			pSelf->SetMutate(off);
			jj += off;
		}
	} // for

	if (pSelf->GetMutate() == off) {
		return true;
	}

	return false;
}

//bool Environment::Crossover(IN const Individual* pParentL, IN const Individual* pParentR, OUT Individual& pChild)
bool Environment::Crossover(IN vector<Parents>& pairs)
{
	//if (pPairs == nullptr) {
	//	LOG_TRACE(LOG_WARNING, "input parent value is null, lhs:%p, rhs:%p", pPairs);
	//	return false;
	//}

	//LOG_TRACE(LOG_DEBUG, "input parent count : %d", pairs.size());

	// 자식들
	vector<Individual> offsprings;

	int32_t fullSize = population[0].GetGeneSize();// population[0].GetChromosomeSize();
	int32_t frontSize = fullSize / 2;
	if (fullSize > 3) {
		frontSize = fullSize % ((rand() % (fullSize - 3)) + 3); // 3~SIZE 위치 분할 
	}
	int32_t backSize = fullSize - frontSize;
	unordered_set<uint32_t> visited;

	Individual child;
	child.SetGeneSize(fullSize);
	child.SetStartFinish(population[0].GetStart(), population[0].GetFinish()); // 부모속성 전달

	visited.reserve(fullSize);
	offsprings.reserve(pairs.size() * 4 + 11); // 


	if (tspOption.endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) { // 원점회귀
		fullSize -= 1;
	}

	// elitism
	// 상위 10% 부모는 형질을 그대로 전승.
	//int32_t cntElitism = max(1, static_cast<int32_t>(population.size() / 100 * 10));
	// 최상위 부모는 형질을 그대로 전승.
	int32_t cntElitism = 10;// 10;

	for (int ii = 0; ii < cntElitism; ii++) {
		// 원형 그대로 유전
		offsprings.emplace_back(population[ii]);

		// 유전자 강제 변이
		offsprings.emplace_back(population[ii]);
		Mutation(&offsprings.back(), 30);

		if (ii % 5 == 0) {
			// 전체를 1개씩 바꿔보고
			child.GetChromosome()->assign(population[ii].GetChromosome()->begin(), population[ii].GetChromosome()->end());
			if (CrossBySelf(&child)) {
				offsprings.emplace_back(child);
			}
		}

		if (ii % 5 == 1) {
			// 전체를 2개씩 바꿔보고
			child.GetChromosome()->assign(population[ii].GetChromosome()->begin(), population[ii].GetChromosome()->end());
			if (CrossBySelf(&child, 1, fullSize - 4, 2)) {
				offsprings.emplace_back(child);
			}
		}

		if (ii % 5 == 2) {
			// 전체를 3개씩 바꿔보고
			child.GetChromosome()->assign(population[ii].GetChromosome()->begin(), population[ii].GetChromosome()->end());
			if (CrossBySelf(&child, 1, fullSize - 5, 3)) {
				offsprings.emplace_back(child);
			}
		}

		if (ii % 5 == 3) {
			// 전체를 4개씩 바꿔보고
			child.GetChromosome()->assign(population[ii].GetChromosome()->begin(), population[ii].GetChromosome()->end());
			if (CrossBySelf(&child, 1, fullSize - 6, 4)) {
				offsprings.emplace_back(child);
			}
		}

		if (ii % 5 == 4) {
			// 전체를 5개씩 바꿔보고
			child.GetChromosome()->assign(population[ii].GetChromosome()->begin(), population[ii].GetChromosome()->end());
			if (CrossBySelf(&child, 1, fullSize - 7, 5)) {
				offsprings.emplace_back(child);
			}
		}

		if (ii % 5 != 0) {
			// 1개씩 바꿔보고
			for (int jj = 1; jj < fullSize - 3; jj++) {
				child.GetChromosome()->assign(population[ii].GetChromosome()->begin(), population[ii].GetChromosome()->end());
				if (CrossBySelf(&child, jj, 3, 1)) {
					offsprings.emplace_back(child);
				}
			}

			// 2개씩 바꿔보고
			for (int jj = 1; jj < fullSize - 4; jj++) {
				child.GetChromosome()->assign(population[ii].GetChromosome()->begin(), population[ii].GetChromosome()->end());
				if (CrossBySelf(&child, jj, 4, 2)) {
					offsprings.emplace_back(child);
				}
			}

			// 3개씩 바꿔보고
			for (int jj = 1; jj < fullSize - 5; jj++) {
				child.GetChromosome()->assign(population[ii].GetChromosome()->begin(), population[ii].GetChromosome()->end());
				if (CrossBySelf(&child, jj, 5, 3)) {
					offsprings.emplace_back(child);
				}
			}

			// 4개씩 바꿔보고
			//for (int jj = 1; jj < fullSize - 6; jj++) {
			//	child.GetChromosome()->assign(population[ii].GetChromosome()->begin(), population[ii].GetChromosome()->end());
			//	if (CrossBySelf(&child, jj, 6, 4)) {
			//		offsprings.emplace_back(child);
			//	}
			//}
		}
	}

	// 유전자 강제 변이
	//for (int ii = 0; ii < cntElitism; ii++) {
	//	offsprings.emplace_back(population[ii]);

	//	Mutation(&offsprings[ii + cntElitism], 30);

	//	//child.GetChromosome()->assign(offsprings.back().GetChromosome()->begin(), offsprings.back().GetChromosome()->end());
	//	//if (CrossBySelf(&child)) {
	//	//	offsprings.emplace_back(child);
	//	//}
	//}



	// 유전자를 역순으로 배치
	int nPos;

	for (int ii = 0; ii < cntElitism; ii++) {
		// ^~1/2, 처음~가운데 교체
		offsprings.emplace_back(population[ii]);
		nPos = (rand() % (fullSize / 2) + 2);
		MutationReverse(&offsprings.back(), 0, nPos, 1012);
		
		// 1/2~$, 가운데~마지막 교체
		offsprings.emplace_back(population[ii]);
		nPos = (rand() % (fullSize / 2) + 1);
		MutationReverse(&offsprings.back(), nPos, fullSize - nPos, 1210);

		// ^~1/3, 처음~1/3 교체
		if (fullSize >= 3) {
			offsprings.emplace_back(population[ii]);
			nPos = (rand() % (fullSize / 3) + 1);
			MutationReverse(&offsprings.back(), 0, nPos, 1013);

			// 1/3~$, 1/3~마지막 교체
			offsprings.emplace_back(population[ii]);
			nPos = (rand() % (fullSize / 3) + 1);
			MutationReverse(&offsprings.back(), nPos, fullSize - 1, 1310);

			// 1/3 ~ 2/3, 1/3~2/3 교체
			offsprings.emplace_back(population[ii]);
			nPos = (rand() % (fullSize / 3) + 1);
			MutationReverse(&offsprings.back(), nPos, fullSize - nPos, 1323);

			// 2/3~, 2/3~마지막 교체
			offsprings.emplace_back(population[ii]);
			nPos = (rand() % (fullSize / 3) + 1);
			MutationReverse(&offsprings.back(), nPos, fullSize - 1, 2323);
		}
	}


	int32_t cur, next;

#ifdef USE_PARALLEL
#pragma omp parallel firstprivate(cur, next, visited, child)
#pragma omp for
	for (int ii = 0; ii < pairs.size(); ii++) {
		Parents& pair = *&pairs[ii];
#else
	for (auto & pair : pairs) {
#endif
		//frontSize = fullSize / ((rand() % (fullSize - 2)) + 2); // 2~SIZE 위치 분할 
		//backSize = fullSize - frontSize;

		//// 부모L 유전자를 역순으로 배치
		//child.GetChromosome()->assign(pair.parentL.GetChromosome()->begin(), pair.parentL.GetChromosome()->end());
		//reverse(child.GetChromosome()->begin() + 1, child.GetChromosome()->end());
		//// 자녀 등록
		//offsprings.emplace_back(child);


		//// 부모R 유전자를 역순으로 배치
		//child.GetChromosome()->assign(pair.parentR.GetChromosome()->begin(), pair.parentR.GetChromosome()->end());
		//reverse(child.GetChromosome()->begin() + 1, child.GetChromosome()->end());
		//// 자녀 등록
		//offsprings.emplace_back(child);


		// 부모 유전자를 일점 교배
		vector<Gene> *lhs, *rhs, *chd = child.GetChromosome();

		for (int loop = 0; loop < 1; loop++) {
			visited.clear();

			// 부모 등록
			child.SetParent(pair.idxL, pair.idxR);
			child.SetMutate(0);

			if (loop == 0) {
				lhs = pair.parentL.GetChromosome();
				rhs = pair.parentR.GetChromosome();
			}
			else {
				lhs = pair.parentR.GetChromosome();
				rhs = pair.parentL.GetChromosome();
			}

			// front 앞, 
			//copy(pair.parentL.GetChromosome()->begin(), pair.parentL.GetChromosome()->begin() + frontSize, child.GetChromosome()->begin());
			::copy(lhs->begin(), lhs->begin() + frontSize, chd->begin());
			for (int jj = 0; jj < frontSize; jj++) {
				cur = lhs->at(jj).GetAttribute();
				visited.emplace(cur);
			}

			// back 뒤, 
			// 중복 안되는 녀석만 우선 입력
			vector<int32_t> vtRetry;
			//copy(pair.parentL.GetChromosome()->begin() + frontSize, pair.parentL.GetChromosome()->end(), child.GetChromosome()->begin() + frontSize);
			::copy(rhs->begin() + frontSize, rhs->end(), chd->begin() + frontSize);
			for (int jj = frontSize; jj < fullSize; jj++) {
				cur = rhs->at(jj).GetAttribute();
				next = getFisrstVisit(visited, cur, -1);

				// 재시도 할 녀석들
				if (next < 0) {
					if ((tspOption.endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) && (jj == child.GetGeneSize() - 1)) { // 원점회귀
						// do notthing
					} else {
						vtRetry.emplace_back(jj);
					}
				}
				else {
					visited.emplace(cur);
				}
			}

			for (const auto & retry : vtRetry) {
				cur = rhs->at(retry).GetAttribute();
				next = getFisrstVisit(visited, cur, fullSize);

				// 위치가 변경되었다면 수정
				if (next != cur) {
					chd->at(retry).SetAttribute(next);
				} else {
					//child.GetChromosome()->at(retry).SetAttribute(cur);
				}
			}


			// 자녀 등록
#ifdef USE_PARALLEL
#pragma omp critical
#endif
			offsprings.emplace_back(child);


			// 유전자 강제 변이
			Mutation(&child, 50);


			// 자녀 등록
#ifdef USE_PARALLEL
#pragma omp critical
#endif
			offsprings.emplace_back(child);

			//child.GetChromosome()->assign(offsprings.back().GetChromosome()->begin(), offsprings.back().GetChromosome()->end());
			//if (CrossBySelf(&child)) {
			//	offsprings.emplace_back(child);
			//}
		} // for
	} // for


	// 세대 교체
	population.assign(offsprings.begin(), offsprings.end());

	generation++;

	return true;
}


bool Environment::Mutation(IN Individual* pIndividual, IN const int32_t mutationRate)
{
	if (pIndividual == nullptr || mutationRate <= 0) {
		return false;
	}

	int32_t cntMutant = pIndividual->GetGeneSize() * mutationRate / 100;;

	if (cntMutant <= 0) {
		cntMutant = 1;
	} else {
		cntMutant = rand() % cntMutant + 1;
	}

	for (int ii = 0; ii < cntMutant; ii++) {
		int32_t mutePos1 = rand() % (pIndividual->GetGeneSize() - 1) + 1;
		int32_t mutePos2 = rand() % (pIndividual->GetGeneSize() - 1) + 1;

		if (mutePos1 == mutePos2) {
			if (mutePos1 >= pIndividual->GetGeneSize() - 1) {
				// 마지막 유전자면
				mutePos1--;
			} else {
				mutePos2++;
			}
		}

		if (mutePos1 != mutePos2) {
			if (pIndividual->GetChromosome()->at(mutePos1).GetGlued() >= 0 || pIndividual->GetChromosome()->at(mutePos2).GetGlued() >= 0) {
				continue;
			}

			int32_t muteVal1 = pIndividual->GetChromosome()->at(mutePos1).GetAttribute();
			int32_t muteVal2 = pIndividual->GetChromosome()->at(mutePos2).GetAttribute();

			pIndividual->GetChromosome()->at(mutePos1).SetAttribute(muteVal2);
			pIndividual->GetChromosome()->at(mutePos2).SetAttribute(muteVal1);
		}
	}

	pIndividual->SetMutate(mutationRate);

	return true;
}


bool Environment::MutationReverse(IN Individual* pIndividual, IN const int32_t idxStart, IN const int32_t idxEnd, IN const int32_t state)
{
	int first = idxStart;
	int last = idxEnd;

	if ((first == 0) && pIndividual->GetChromosome()->at(first).GetGlued() >= 0) {
		first++;
	}
	if ((last == pIndividual->GetChromosomeSize() - 1) && pIndividual->GetChromosome()->at(last).GetGlued() >= 0) {
		last--;
	}

	reverse(pIndividual->GetChromosome()->begin() + first, pIndividual->GetChromosome()->begin() + last);
	pIndividual->SetMutate(state);

	return true;
}


void Environment::GetBest(vector<int>& result)
{
	for (int ii = 0; ii < population[0].GetChromosome()->size(); ii++) {
		// 출발지 고정 & 원점 회귀
		// 회귀는 마지막 강제 추가된 염색체 제외
		if ((tspOption.endpointType == TYPE_TSP_ENDPOINT_RECURSIVE) && (ii == tableCount - 1)) {
			break;
		}
		result.emplace_back(population[0].GetChromosome()->at(ii).GetAttribute());
	}
}


const double Environment::GetBestCost(void)
{
	double cost = population[0].GetCost();
	return cost;
}

const uint32_t Environment::GetBestDist(void)
{
	uint32_t dist = population[0].GetDist();
	return dist;
}

const uint32_t Environment::GetBestTime(void)
{
	uint32_t time = population[0].GetTime();
	return time;
}

#if defined(SHOW_STATUS)
void Environment::Print(void)
{
	int cnt = 0;
	LOG_TRACE(LOG_DEBUG_CONTINUE, "==================================================\n");
	LOG_TRACE(LOG_DEBUG_CONTINUE, "Generation : %d\n", generation);
	for (auto & indi : population) {
		LOG_TRACE(LOG_DEBUG_CONTINUE, "idx(%2d) : ", cnt++);
		indi.Print();
		LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");
#if !defined(_DEBUG)
		if (cnt > 3) break;
#endif
	}
	LOG_TRACE(LOG_DEBUG_CONTINUE, "==================================================\n");
}
#endif