#pragma once

#ifndef __TSP_GA__
#define __TSP_GA__

#include "../include/MapDef.h"

#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <limits>


#define MAX_INDIVIDUAL 10000 // 최대 개체 
#define MAX_CHROMOSOME 1000 //50 // 최대 염색체 수
#define MAX_GENE 1000 //50 // 최대 유전자 수

#if 1 // defined(_DEBUG)
#define SHOW_STATUS // 결과 확인용
#endif


// 개별 유전자 별 가까운 값을 바로 확인하기 위해 아이템 관리
struct tableItem
{
	uint32_t idx; // 테이블 요소 인덱스
	uint32_t value; // 값 (총거리, 총시간 등)
	double cost;
	uint32_t dist;
	uint32_t time;

	tableItem()
	{
		idx = 0;
		value = 0;
		cost = 0;
		dist = 0;
		time = 0;
	}
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

	void SetGlued(IN const int32_t idxGlue = -1);
	int32_t GetGlued(void) const;

private:
	uint32_t attribute; // 속성(거리, 시간 등)
	int32_t glued; // 고정되어 변경되지 않음, 0 미만은 고정 안됨, 0 이상은 설정된 idx로 고정됨
};


// 개체
class Individual {
public:
	Individual();
	Individual(IN const int32_t size); // 최대 유전자 수
	virtual ~Individual();

	void SetGeneSize(IN const int32_t size);
	int32_t GetGeneSize(bool isEnable = true);

	double CalculateCost(IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const int count, IN const double average);
	double GetCost(void) const;
	uint32_t GetDist(void) const;
	uint32_t GetTime(void) const;
	double GetFitness(void) const;
	double GetFValue(void) const;
	uint32_t GetChromosomeSize(void) const;
	vector<Gene>* GetChromosome(void);
	uint32_t GetMutate(void);
	int32_t GetStart(void) const;
	int32_t GetFinish(void) const;

	void SetFitness(IN const double val);
	void SetFValue(IN const double val);
	void SetParent(IN const uint32_t parent1, IN const uint32_t parent2);
	void SetMutate(IN const uint32_t state);
	void SetStartFinish(IN const int32_t nStart, IN const int32_t nFinish);

	bool Create(IN const vector<tableItemNearlest>* pNearlest, IN const int32_t start = 0, IN const int32_t finish = -1, IN const uint32_t idx = 0); // ??u ??? ????


#if defined(SHOW_STATUS)
	void Print(void);
#endif

private:
	bool Propagation(IN const vector<tableItemNearlest>* pNearlest, IN const int32_t start, IN const int32_t finish, IN const uint32_t maxLoop, OUT vector<uint32_t>& vtBestWay);

	int32_t geneSize; // 전체 유전자 수
	int32_t enableGeneSize; // 사용 가능한 유전자 수, 도착지 고정값을 뺀 수
	uint32_t value; // 값 (총거리, 총시간 등)
	double cost;
	uint32_t dist;
	uint32_t time;
	double boundary;

	double fvalue;
	double fitness; // 적합도

	vector<Gene> chromosome; // 염색체, 유전자 다발

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

	void SetOption(IN const TspOption* pOpt);

	bool SetCostTable(IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const int count);

	// 생성
	void Genesis(IN const uint32_t maxGene, IN const uint32_t maxPopulation); // 최초 생성

	// 평가
	const double Evaluation(void); // 평가 - return : top value 

	// 선택
	void Selection(OUT vector<Parents>& pairs);

	// 교차
	//bool Crossover(IN const Individual* pParentL, IN const Individual* pParentR, OUT Individual& pChild); // 교배
	bool Crossover(IN vector<Parents>& pairs);
	bool CrossBySelf(IN Individual* pSelf, IN const int32_t pos = -1, IN const int32_t size = -1, IN const int32_t off = 1);

	// 변이
	bool Mutation(IN Individual* pIndividual, IN const int32_t mutationRate);
	bool MutationReverse(IN Individual* pIndividual, IN const int32_t idxStart, IN const int32_t idxEnd, IN const int32_t state);

	void GetBest(vector<int>& result);
	const double GetBestCost(void);
	const uint32_t GetBestDist(void);
	const uint32_t GetBestTime(void);



#if defined(SHOW_STATUS)
	void Print(void);
#endif

private:
	int nMaxPopulation; // 최대 개체수
	vector<Individual> population;
	uint32_t generation; // 세대

	vector<vector<stDistMatrix>>* ppTables;
	uint32_t tableCount;

	vector<tableItemNearlest> genePriority;

	TspOption tspOption;

	double fitnessSum;
	double averageValue;
	double worstValue;
	double bestValue;
};

#endif // __TSP_GA__