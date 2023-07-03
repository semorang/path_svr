#ifndef __SHORTEST__
#define __SHORTEST__

#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <limits>

#include "../route/RoutePlan.h"


// Minimum Spaning Tree
// using prim algorithm, not kruskal

typedef struct _tagTravel{
	double cost;
	int32_t idx;
	uint64_t visited;
	vector<int32_t> path;

	bool operator() (_tagTravel lhs, _tagTravel rhs) {
		return lhs.cost < rhs.cost; // greater
	}

	void init() {
		cost = 0.f;
		idx = 0;
		visited = 0;
		path.clear();
		vector<int32_t>().swap(path);
	}
}Travel;

Travel mst_manhattan_branch_and_bound2(IN const RouteTable** ppResultTables, IN const int count, int start);
Travel mst_manhattan_branch_and_bound(IN const RouteTable** ppResultTables, IN const int count, int start);
pair<int, vector<int>> tsp_euclidean_branch_and_bound(IN const RouteTable** ppResultTables, IN const int count, int start);


#endif // __SHORTEST__