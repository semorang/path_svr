#ifndef __TSP_GA__
#define __TSP_GA__

#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <limits>

#include "../route/RoutePlan.h"


#define MAX_INDIVIDUAL 10000 // �ִ� ��ü 
#define MAX_CHROMOSOME 1000 //50 // �ִ� ����ü ��
#define MAX_GENE 1000 //50 // �ִ� ������ ��

#if 1 // defined(_DEBUG)
#define SHOW_STATUS // ��� Ȯ�ο�
#endif

// ���� ������ �� ����� ���� �ٷ� Ȯ���ϱ� ���� ������ ����
struct tableItem
{
	uint32_t idx; // ���̺� ��� �ε���
	uint32_t value; // �� (�ѰŸ�, �ѽð� ��)
	double cost;
};

// ��ü �����ں� ����� ������ ����
struct tableItemNearlest
{
	uint32_t idx; // ���̺� ��� �ε���
	vector<tableItem> nearlest; // ����� ���̺� �ε���
};


// ������
class Gene {
public:
	Gene();
	Gene(IN const uint32_t attr);
	virtual ~Gene();

	void SetAttribute(IN const uint32_t attr);
	uint32_t GetAttribute(void) const;

private:
	uint32_t attribute; // �Ӽ�(�Ÿ�, �ð� ��)
};


// ��ü
class Individual {
public:
	Individual();
	Individual(IN const uint32_t size); // �ִ� ������ ��
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

	bool Create(IN const vector<tableItemNearlest>* pNearlest, IN const uint32_t start = 0, IN const uint32_t idx = 0); // ��ü �ڵ� ����


#if defined(SHOW_STATUS)
	void Print(void);
#endif

private:
	uint32_t geneSize;
	uint32_t value; // �� (�ѰŸ�, �ѽð� ��)
	double cost;
	double boundary;

	double fvalue;
	double fitness; // ���յ�

	vector<Gene> chromosome; // ����ü

	uint32_t parent1;
	uint32_t parent2;
	uint32_t mutate; // 1:����, 2:��ü�̿�����
};


typedef struct _tagParents
{
	uint32_t idxL;
	uint32_t idxR;
	Individual parentL;
	Individual parentR;
}Parents;

// ȯ��
class Environment {
public:
	Environment();
	virtual ~Environment();

	void SetOption(TspOptions* pOpt);

	bool SetCostTable(IN const RouteTable** ppResultTables, IN const int count);

	void Genesis(IN const uint32_t maxGene, IN const uint32_t maxPopulation); // ���� ����

	const double Evaluation(void); // �� - return : top value 

	void Selection(OUT vector<Parents>& pairs); // ����

	//bool Crossover(IN const Individual* pParentL, IN const Individual* pParentR, OUT Individual& pChild); // ����
	bool Crossover(IN vector<Parents>& pairs); // ����
	bool CrossBySelf(IN Individual* pSelf, IN const int32_t pos = -1, IN const int32_t size = -1, IN const int32_t off = 1);

	bool Mutation(IN Individual* pIndividual, IN const int32_t mutationRate);

	void GetBest(vector<int>& result);



#if defined(SHOW_STATUS)
	void Print(void);
#endif

private:
	int nMaxPopulation; // �ִ� ��ü��
	vector<Individual> population;
	uint32_t generation; // ����

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