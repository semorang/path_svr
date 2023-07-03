#include "tsp_ga.h"

#include "../utils/UserLog.h"
#include <time.h>


////////////////////////////////////////////////////////////////////////////////////////////////////
// Chromosome
// Gene
Gene::Gene()
{
	attribute = 0;
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


////////////////////////////////////////////////////////////////////////////////////////////////////
// Individual
bool cmpFitness(Individual lhs, Individual rhs) {
	return lhs.GetFitness() < rhs.GetFitness(); // 올림차순 // fitness가 큰 개체 우선
}


Individual::Individual()
{
	geneSize = 0;
}


Individual::Individual(IN const uint32_t size)
{
	if (size > MAX_GENE) {
		geneSize = MAX_GENE;
	}
	else {
		geneSize = size;
	}
	chromosome.resize(geneSize);

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


void Individual::SetGeneSize(IN const uint32_t size)
{
	if (size > MAX_GENE) {
		geneSize = MAX_GENE;
	}
	else {
		geneSize = size;
	}
	chromosome.resize(geneSize);
}


uint32_t Individual::GetGeneSize(void)
{
	return geneSize;
}


double Individual::CalculateCost(IN const RouteTable** ppResultTables, IN const int count, IN const double average)
{
	if (count < geneSize) {
		LOG_TRACE(LOG_WARNING, "table rows not enough size compare with chromosome size, table(%d) < chromosome(%d)", count, geneSize);
	}

	uint32_t prv, next;
	cost = value = 0;

	for (int ii = 0; ii < geneSize - 1; ii++) {
		prv = chromosome[ii].GetAttribute();
		next = chromosome[ii+1].GetAttribute();
#if defined(USE_REAL_ROUTE_TSP)
		cost += ppResultTables[prv][next].dbTotalCost;
		value += ppResultTables[prv][next].nTotalDist;
		//value += ppResultTables[prv][next].nTotalTime;
#else
		value += ppResultTables[prv][next].nTotalDist;
#endif
	}
	prv = chromosome[geneSize - 1].GetAttribute();
	next = chromosome[0].GetAttribute();
#if defined(USE_REAL_ROUTE_TSP)
	cost += ppResultTables[prv][next].dbTotalCost;
	value += ppResultTables[prv][next].nTotalDist;
	//value += ppResultTables[prv][next].nTotalTime;
#else
	value += ppResultTables[prv][next].nTotalDist;
#endif

	boundary = average / value;

#if defined(USE_REAL_ROUTE_TSP)
	return cost;
	//return value;
#else
	return value;
#endif
}


uint32_t Individual::GetValue(void) const
{
	return value;
}


double Individual::GetCost(void) const
{
	return cost;
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


bool Individual::Create(IN const vector<tableItemNearlest>* pNearlest, IN const uint32_t start, IN const uint32_t idx)
{
	uint32_t prev, next, preVal;
	uint32_t pos = start;
	vector<bool> created(geneSize, false); // 유전자 배열 생성 여부
	vector<bool> visited(geneSize, false); // 이웃 index 사용 여부
	const int32_t orderRate = 50; // 30; 우선순위 30%내에서 선택 // 1,2,3 순위를 랜덤으로 가져오도록

	if (geneSize <= start) {
		LOG_TRACE(LOG_WARNING, "start position value over than capacity, capacity(%d) <= start(%d)", geneSize, start);
		return false;
	}

	// 시작 위치 고정
	chromosome[0].SetAttribute(pos);
	created[pos] = visited[pos] = true;

	prev = 0;
	next = 1;

	for (; prev < geneSize - 1; prev++, next++) {
		//uint32_t pos = rand() % geneSize;
		//while (created[pos] == true) {
		//	pos = (pos + 1) % geneSize;
		//}

		preVal = chromosome[prev].GetAttribute();

		// 가까운 유전자를 미사용된 순서로 가져온다.
		uint32_t nearlest = 0;
		int order = 0; // 첫 개체는 제일 가까운 값을 사용
		//if (idx != 0) {
		if (0) {
			// 두번째 유전자는 첫번째에서 거리순 절반까지 유전자를 대입해 준다. 
			//if ((prev == 0) && (idx < (geneSize / 2 - 1))) {
			if (geneSize <= 30) {
				if ((prev == 0) && (idx < (geneSize / 2 - 1))) {
					preVal = pos;
					order = idx;
				}
				//else if (idx < (geneSize / 2 - 1)){
				else if (geneSize / 2 - 1) {
					;
				}
				else
				{
					order = rand() % (geneSize * orderRate / 100); // 우선순위 30%내에서 선택 // 1,2,3 순위를 랜덤으로 가져오도록
					//order = rand() % (geneSize * (idx % 10) * 10 / 100); // 우선순위 10%~100% 내에서 선택 // 1,2,3 순위를 랜덤으로 가져오도록
				}
			}
			else {
				order = rand() % (geneSize * orderRate / 100); // 우선순위 30%내에서 선택 // 1,2,3 순위를 랜덤으로 가져오도록
				//order = rand() % (geneSize * (idx % 10) * 10 / 100); // 우선순위 10%~100% 내에서 선택 // 1,2,3 순위를 랜덤으로 가져오도록
			}
		}


		nearlest = pNearlest->at(preVal).nearlest[order].idx;
		if (visited[nearlest] == false) {
			visited[nearlest] = true;
		}
		else { // 아니면 가까운 우선순위 부터 순차로 가져와보자
			for (order = 0; order < geneSize - 1; order++) {
				nearlest = pNearlest->at(preVal).nearlest[order].idx;
				if (visited[nearlest] == true) {
					continue;
				}
				else { //if (visited[nearlest] == false) {
					visited[nearlest] = true;
					break;
				}
			}
		}

		chromosome[next].SetAttribute(nearlest);
		created[next] = true;
	}

	return true;
}


#if defined(SHOW_STATUS)
void Individual::Print(void)
{
	LOG_TRACE(LOG_DEBUG_CONTINUE, "p(%2d, %2d) %d, ", parent1, parent2, mutate);

	int cnt = 0;
	for (const auto & gene : chromosome) {
		LOG_TRACE(LOG_DEBUG_CONTINUE, "%2d→", gene.GetAttribute());
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
	averageCost = 0.f;
	bestCost = 0.f;
	worstCost = 0.f;	
}


Environment::~Environment()
{
	if (!population.empty()) {
		population.clear();
		vector<Individual>().swap(population);
	}
}


void Environment::SetOption(TspOptions* pOpt)
{
	if (pOpt == nullptr) {
		LOG_TRACE(LOG_WARNING, "tsp setting option null is null");
	}

	memcpy(&options, pOpt, sizeof(options));
}


bool cmpNearlestGene(const tableItem& lhs, const tableItem& rhs) {
#if defined(USE_REAL_ROUTE_TSP)
	//return lhs.value < rhs.value;
	return lhs.cost < rhs.cost;
#else
	return lhs.value < rhs.value;
#endif
}

bool Environment::SetCostTable(IN const RouteTable** ppResultTables, IN const int count)
{
	if (ppResultTables == nullptr) {
		LOG_TRACE(LOG_WARNING, "input table pointer is null");
		return false;
	}
	else if (count <= 0 || count > MAX_GENE) {
		LOG_TRACE(LOG_WARNING, "input table count not valid, %d, max(%d)", count, MAX_GENE);
		return false;
	}


	ppTables = ppResultTables;
	tableCount = count;

	const uint32_t maxGene = tableCount;

	genePriority.clear();
	genePriority.reserve(maxGene - 1); // 자기는 뺀 이웃개수

	for (int ii = 0; ii < maxGene; ii++) {
		uint32_t min = UINT32_MAX;
		uint32_t max = 0;

		tableItemNearlest currentGene;
		currentGene.idx = ii;

		for (int jj = 0; jj < maxGene; jj++) {
			if (ii == jj) continue;

			tableItem neighborGene;

#if defined(USE_REAL_ROUTE_TSP)
			if (ppTables[ii][jj].dbTotalCost < min) {
				min = ppTables[ii][jj].dbTotalCost;
			}

			if (ppTables[ii][jj].dbTotalCost > max) {
				max = ppTables[ii][jj].dbTotalCost;
			}

			neighborGene.idx = jj;
			neighborGene.cost = ppTables[ii][jj].dbTotalCost;
			neighborGene.value = ppTables[ii][jj].nTotalDist;
			currentGene.nearlest.emplace_back(neighborGene);

			//if (ppTables[ii][jj].nTotalDist < min) {
			//	min = ppTables[ii][jj].nTotalDist;
			//}

			//if (ppTables[ii][jj].nTotalDist > max) {
			//	max = ppTables[ii][jj].nTotalDist;
			//}

			//neighborGene.idx = jj;
			//neighborGene.value = ppTables[ii][jj].nTotalDist;
			//currentGene.nearlest.emplace_back(neighborGene);

			//if (ppTables[ii][jj].nTotalTime < min) {
			//	min = ppTables[ii][jj].nTotalTime;
			//}

			//if (ppTables[ii][jj].nTotalTime > max) {
			//	max = ppTables[ii][jj].nTotalTime;
			//}

			//neighborGene.idx = jj;
			//neighborGene.value = ppTables[ii][jj].nTotalTime;
			//currentGene.nearlest.emplace_back(neighborGene);
#else
			if (ppTables[ii][jj].nTotalDist < min) {
				min = ppTables[ii][jj].nTotalDist;
			}

			if (ppTables[ii][jj].nTotalDist > max) {
				max = ppTables[ii][jj].nTotalDist;
			}

			neighborGene.idx = jj;
			neighborGene.value = ppTables[ii][jj].nTotalDist;
			currentGene.nearlest.emplace_back(neighborGene);
#endif
		} // for jj


		// 현재 유전자와 가까운 유전자 벡터 생성
		sort(currentGene.nearlest.begin(), currentGene.nearlest.end(), cmpNearlestGene);
		genePriority.emplace_back(currentGene);

		bestCost += min;
		worstCost += max;
	} // for ii

	averageCost = static_cast<double>(bestCost + worstCost) / 2.f;

	LOG_TRACE(LOG_DEBUG, "table best: %.2f, worst: %.2f, average: %.2f", bestCost, worstCost, averageCost);

	return true;
}


void Environment::Genesis(IN const uint32_t maxGene, IN const uint32_t maxPopulation)
{
	//srand(time(NULL));
	srand(1000 + 49 * 2);

	nMaxPopulation = maxPopulation;

	if (nMaxPopulation > MAX_INDIVIDUAL) {
		nMaxPopulation = MAX_INDIVIDUAL;
	}

	for (int ii = 0; ii < max(maxGene, maxPopulation); ii++) {
		Individual newIndi(maxGene);
		newIndi.Create(&genePriority, 0, ii);
		population.emplace_back(newIndi);
	}

	generation++; // 세대 시작	
}


const double Environment::Evaluation(void)
{
	double min = UINT64_MAX;
	double max = 0;
	double cost;

	for (auto & item : population) {
		cost = item.CalculateCost(ppTables, tableCount, averageCost);
		if (cost < min) {
			min = cost;
		}
		if (cost > max) {
			max = cost;
		}

		item.SetFValue(cost);
	}

	//bestCost = min;
	//worstCost = max;
	//averageCost = static_cast<double>(min + max) / 2;


	const int32_t mulRate = 3 - 1;
	double fitness = 0.f;
	fitnessSum = 0.f;
	for (auto & item : population) {
		if (worstCost == bestCost) {
			fitness = RAND_MAX / (worstCost / mulRate) * 100;
		}
		else {
			fitness = RAND_MAX / ((worstCost - item.GetFValue()) + (worstCost - bestCost) / (mulRate)) * 100;
		}
		fitnessSum += fitness;

		item.SetFitness(fitness);
	}

	sort(population.begin(), population.end(), cmpFitness);

	return population[0].GetFitness();
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
			double sum = 0;
			int32_t point = rand() % static_cast<uint32_t>(fitnessSum);

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
		costBefor = distAfter = 0;
		distBefor = distAfter = 0;

		// befor
		for (int kk = jj - 1; kk < jj + off; kk++) {
			prev = pGenes->at(kk).GetAttribute();
			next = pGenes->at(kk+1).GetAttribute();
#if defined(USE_REAL_ROUTE_TSP)
			costBefor = ppTables[prev][next].dbTotalCost;
			distBefor = ppTables[prev][next].nTotalDist;
			//distBefor = ppTables[prev][next].nTotalTime;
#else
			distBefor = ppTables[prev][next].nTotalDist;
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
#if defined(USE_REAL_ROUTE_TSP)
			costAfter = ppTables[prev][next].dbTotalCost;
			distAfter = ppTables[prev][next].nTotalDist;
			//distAfter = ppTables[prev][next].nTotalTime;
#else
			distAfter = ppTables[prev][next].nTotalDist;
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

	vector<Individual> offsprings;

	const int32_t fullSize = population[0].GetChromosomeSize();
	int32_t frontSize = fullSize / 2;
	if (fullSize > 3) {
		frontSize = fullSize % ((rand() % (fullSize - 3)) + 3); // 3~SIZE 위치 분할 
	}
	int32_t backSize = fullSize - frontSize;
	unordered_set<uint32_t> visited;

	Individual child;
	child.SetGeneSize(fullSize);

	visited.reserve(fullSize);
	offsprings.reserve(pairs.size() * 4 + 11); // 

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

	//유전자를 역순으로 배치
	int nPos;
	for (int ii = 0; ii < cntElitism; ii++) {

		// ~1/2
		offsprings.emplace_back(population[ii]);
		nPos = (rand() % (fullSize / 2) + 2);
		reverse(offsprings.back().GetChromosome()->begin() + 1, offsprings.back().GetChromosome()->begin() + nPos);
		offsprings.back().SetMutate(1012);

		// 1/2 ~
		offsprings.emplace_back(population[ii]);
		nPos = (rand() % (fullSize / 2) + 1);
		reverse(offsprings.back().GetChromosome()->begin() + nPos, offsprings.back().GetChromosome()->end());
		offsprings.back().SetMutate(1210);

		// 1/3 ~
		offsprings.emplace_back(population[ii]);
		nPos = (rand() % (fullSize / 3) + 1);
		reverse(offsprings.back().GetChromosome()->begin() + nPos, offsprings.back().GetChromosome()->end());
		offsprings.back().SetMutate(1310);

		// 1/3 ~ 2/3
		offsprings.emplace_back(population[ii]);
		nPos = (rand() % (fullSize / 3) + 1);
		reverse(offsprings.back().GetChromosome()->begin() + nPos, offsprings.back().GetChromosome()->end() - nPos);
		offsprings.back().SetMutate(1323);

		// 2/3 ~
		offsprings.emplace_back(population[ii]);
		nPos = (rand() % (fullSize / 3) + 1);
		reverse(offsprings.back().GetChromosome()->end() - nPos, offsprings.back().GetChromosome()->end());
		offsprings.back().SetMutate(2310);
	}



	int32_t cur, next;

	for (auto & pair : pairs) {
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
			copy(lhs->begin(), lhs->begin() + frontSize, chd->begin());
			for (int ii = 0; ii < frontSize; ii++) {
				cur = lhs->at(ii).GetAttribute();
				visited.emplace(cur);
			}

			// back 뒤, 
			// 중복 안되는 녀석만 우선 입력
			vector<int32_t> vtRetry;
			//copy(pair.parentL.GetChromosome()->begin() + frontSize, pair.parentL.GetChromosome()->end(), child.GetChromosome()->begin() + frontSize);
			copy(rhs->begin() + frontSize, rhs->end(), chd->begin() + frontSize);
			for (int ii = frontSize; ii < fullSize; ii++) {
				cur = rhs->at(ii).GetAttribute();
				next = getFisrstVisit(visited, cur, -1);

				// 재시도 할 녀석들
				if (next < 0) {
					vtRetry.emplace_back(ii);
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
				}
				else {
					//child.GetChromosome()->at(retry).SetAttribute(cur);
				}
			}

			// 자녀 등록
			offsprings.emplace_back(child);


			// 유전자 강제 변이
			Mutation(&child, 50);
			// 자녀 등록
			offsprings.emplace_back(child);

			//child.GetChromosome()->assign(offsprings.back().GetChromosome()->begin(), offsprings.back().GetChromosome()->end());
			//if (CrossBySelf(&child)) {
			//	offsprings.emplace_back(child);
			//}
		} // for
	} // for


	// 세대 교체
	population.clear();
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
			}
			else {
				mutePos2++;
			}
		}

		if (mutePos1 != mutePos2) {
			int32_t muteVal1 = pIndividual->GetChromosome()->at(mutePos1).GetAttribute();
			int32_t muteVal2 = pIndividual->GetChromosome()->at(mutePos2).GetAttribute();

			pIndividual->GetChromosome()->at(mutePos1).SetAttribute(muteVal2);
			pIndividual->GetChromosome()->at(mutePos2).SetAttribute(muteVal1);
		}
	}

	pIndividual->SetMutate(mutationRate);

	return true;
}


void Environment::GetBest(vector<int>& result)
{
	for (int ii = 0; ii < population[0].GetChromosome()->size(); ii++) {
		result.emplace_back(population[0].GetChromosome()->at(ii).GetAttribute());
	}
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