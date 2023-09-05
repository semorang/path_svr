#ifndef __TSP_GA__
#define __TSP_GA__

#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <limits>

#include "../route/RoutePlan.h"


#define MAX_INDIVIDUAL 10000 // 최대 개체 
#define MAX_CHROMOSOME 1000 //50 // 최대 염색체 수
#define MAX_GENE 1000 //50 // 최대 유전자 수

#if 1 // defined(_DEBUG)
#define SHOW_STATUS // 결과 확인용
#endif

#define USE_FIX_TO_START // 출발지 고정
// #define USE_FIX_TO_FINISH // 도착지 모델
// #define USE_RETURN_TO_ORIGINAL // 원점회귀 모델

// 개별 유전자 별 가까운 값을 바로 확인하기 위해 아이템 관리
struct tableItem
{
	uint32_t idx; // 테이블 요소 인덱스
	uint32_t value; // 값 (총거리, 총시간 등)
	double cost;
};

// 전체 유전자별 가까운 아이템 벡터
struct tableItemNearlest
{
	uint32_t idx; // 테이블 요소 인덱스
	vector<tableItem> nearlest; // 가까운 테이블 인덱스
};


// 유전자
class Gene {
public:
	Gene();
	Gene(IN const uint32_t attr);
	virtual ~Gene();

	void SetAttribute(IN const uint32_t attr);
	uint32_t GetAttribute(void) const;

	void SetGlued(IN const bool glue);
	bool IsGlued(void) const;

private:
	uint32_t attribute; // 속성(거리, 시간 등)
	bool glued; // 고정되어 변경되지 않음
};


// 개체
class Individual {
public:
	Individual();
	Individual(IN const uint32_t size); // 최대 유전자 수
	virtual ~Individual();

	void SetGeneSize(IN const uint32_t size);
	uint32_t GetGeneSize(void);

	double CalculateCost(IN const RouteTable** ppResultTables, IN const int count, IN const double average);
	uint32_t GetValue(void) const;
	double GetCost(void) const;
	double GetFitness(void) const;
	double GetFValue(void) const;
	uint32_t GetChromosomeSize(void) const;
	vector<Gene>* GetChromosome(void);
	uint32_t GetMutate(void);

	void SetFitness(IN const double val);
	void SetFValue(IN const double val);
	void SetParent(IN const uint32_t parent1, IN const uint32_t parent2);
	void SetMutate(IN const uint32_t state);

	bool Create(IN const vector<tableItemNearlest>* pNearlest, IN const uint32_t start = 0, IN const uint32_t finish = 0, IN const uint32_t idx = 0); // 개체 자동 생성


#if defined(SHOW_STATUS)
	void Print(void);
#endif

private:
	uint32_t geneSize;
	uint32_t value; // 값 (총거리, 총시간 등)
	double cost;
	double boundary;

	double fvalue;
	double fitness; // 적합도

	vector<Gene> chromosome; // 염색체

	uint32_t parent1;
	uint32_t parent2;
	uint32_t mutate; // 1:변이, 2:자체이웃변이

	int32_t m_start;
	int32_t m_finish;
};


typedef struct _tagParents
{
	uint32_t idxL;
	uint32_t idxR;
	Individual parentL;
	Individual parentR;
}Parents;

// 환경
class Environment {
public:
	Environment();
	virtual ~Environment();

	void SetOption(TspOptions* pOpt);

	bool SetCostTable(IN const RouteTable** ppResultTables, IN const int count);

	void Genesis(IN const uint32_t maxGene, IN const uint32_t maxPopulation); // 최초 생성

	const double Evaluation(void); // 평가 - return : top value 

	void Selection(OUT vector<Parents>& pairs); // 선택

	//bool Crossover(IN const Individual* pParentL, IN const Individual* pParentR, OUT Individual& pChild); // 교배
	bool Crossover(IN vector<Parents>& pairs); // 교배
	bool CrossBySelf(IN Individual* pSelf, IN const int32_t pos = -1, IN const int32_t size = -1, IN const int32_t off = 1);

	bool Mutation(IN Individual* pIndividual, IN const int32_t mutationRate);

	void GetBest(vector<int>& result);



#if defined(SHOW_STATUS)
	void Print(void);
#endif

private:
	int nMaxPopulation; // 최대 개체수
	vector<Individual> population;
	uint32_t generation; // 세대

	const RouteTable** ppTables;
	uint32_t tableCount;

	vector<tableItemNearlest> genePriority;

	TspOptions options;

	double fitnessSum;
	double averageCost;
	double worstCost;
	double bestCost;
};

#endif // __TSP_GA__