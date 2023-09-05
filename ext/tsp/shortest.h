#ifndef __SHORTEST__
#define __SHORTEST__

#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <limits>

#include "../route/RoutePlan.h"

typedef struct _tagTravel{
	double cost;
	int32_t idx;
	uint64_t visited;
	vector<int32_t> path;

	bool operator() (_tagTravel lhs, _tagTravel rhs) {
		return lhs.cost < rhs.cost; // greater
	}
}Travel;

Travel mst_manhattan_branch_and_bound(IN const RouteTable** ppResultTables, IN const int count, int start);
pair<int, vector<int>> tsp_euclidean_branch_and_bound(IN const RouteTable** ppResultTables, IN const int count, int start);


#endif // __SHORTEST__