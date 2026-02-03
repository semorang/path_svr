#ifndef __SHORTEST__
#define __SHORTEST__

#include "../include/MapDef.h"

#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <limits>


// Minimum Spaning Tree
// using prim algorithm, not kruskal

typedef struct _tagTravel{
	double cost;
	int32_t idx;
	uint64_t visited;
	std::vector<int32_t> path;

	bool operator() (_tagTravel lhs, _tagTravel rhs) {
		return lhs.cost < rhs.cost; // greater
	}

	void init() {
		cost = 0.f;
		idx = 0;
		visited = 0;
		path.clear();
		std::vector<int32_t>().swap(path);
	}
}Travel;

Travel mst_manhattan_branch_and_bound2(IN const std::vector<std::vector<stDistMatrix>>& vtDistMatrix, IN const int count, int start);
Travel mst_manhattan_branch_and_bound(IN const std::vector<std::vector<stDistMatrix>>& vtDistMatrix, IN const int count, int start);
std::pair<int, std::vector<int>> tsp_euclidean_branch_and_bound(IN const std::vector<std::vector<stDistMatrix>>& vtDistMatrix, IN const int count, int start);

#endif // __SHORTEST__