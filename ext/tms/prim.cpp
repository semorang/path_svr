#include "prim.h"

#include "../utils/UserLog.h"

const double INF = DBL_MAX;

using namespace std;

//double euclidean_distance(pair<double, double> p1, pair<double, double> p2) {
//	printf("%f, %f, %f, %f\n", p1.first, p1.second, p2.first, p2.second);
//	double dx = p1.first - p2.first;
//	double dy = p1.second - p2.second;
//	return sqrt(dx * dx + dy * dy);
//}


// 낮은 자리부터 방문 플래그를 채운다.
// 1번째는 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000001
// 4번째는 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00001000
// 2,3번째는 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000110

const uint64_t g_mask = 0xFFFFFFFFFFFFFFFF;

uint64_t setVisited(IN uint64_t flag, IN const uint32_t idx) {
	uint64_t ret = flag;

	if (idx < 64) {
		uint64_t mask = (uint64_t)0x1 << idx;

		ret = flag | mask;
	}

	return ret;
}

bool getVisited(IN const uint64_t flag, IN const uint32_t idx) {
	bool ret = false;

	if (idx < 64) {
		uint64_t mask = (uint64_t)0x1 << idx;

		ret = (flag & mask) == mask ? true : false;
	}

	return ret;
}


//static auto CompareCanidate = [](const CandidateLink* lhs, const CandidateLink* rhs) {
//	if (lhs->costHeuristic > rhs->costHeuristic) {
//		return true;
//	}
//	else if (lhs->costHeuristic == rhs->costHeuristic) {
//		return lhs->depth > rhs->depth;
//	}
//	else {
//		return false;
//	}
//};
struct cmpTravel {
	bool operator() (Travel lhs, Travel rhs) {
		return lhs.cost > rhs.cost;
	}
};



Travel mst_manhattan_branch_and_bound(IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const int count, int start)
{
	Travel best_path;

	if (!vtDistMatrix.empty() && count > 0 && start >= 0) {
		priority_queue<Travel, vector<Travel>, cmpTravel> pq;
		const double best = INF;

		// set start
		Travel travel;
		travel.cost = 0.f;
		travel.idx = start;
		travel.visited = setVisited(0, start);
		travel.path.emplace_back(start);

		pq.push(travel);

		LOG_TRACE(LOG_DEBUG, "start, find mst-prim path, cnt:%d", count);

		while (!pq.empty()) {
			auto top = pq.top(); pq.pop();
			auto cost = top.cost;
			auto cur = top.idx;
			auto visited = top.visited;
			auto path = &top.path;

			// clean queue
			while (!pq.empty()) {
				pq.pop();
			}

			LOG_TRACE(LOG_DEBUG_LINE, "result[%d] : %d[%d]", path->size(), 0, (*path)[0]);
			for (int pp = 1; pp < path->size(); pp++) {
				LOG_TRACE(LOG_DEBUG_CONTINUE, "-%d[%d]", pp, (*path)[pp]);
			}
			LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");

			if (cost >= best) { continue; }

			// check finish
			if (path->size() == count) {
				
				// set best travel
				best_path.cost = cost;
				best_path.idx = cur;
				best_path.visited = visited;
				best_path.path.assign(path->begin(), path->end());

				LOG_TRACE(LOG_DEBUG, "success, find mst-prim path, cnt:%d", best_path.path.size());
				LOG_TRACE(LOG_DEBUG_LINE, "path result : %d", best_path.path[0]);
				for (int pp = 1; pp < count; pp++) {
					LOG_TRACE(LOG_DEBUG_CONTINUE, "-%d", best_path.path[pp]);
				}
				LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");

				break;
			}


			// prapagation
			for (int i = 0; i < count; ++i) {
				if (getVisited(visited, i) == false) {

					// set new travel
					Travel new_travel;
#if defined(USE_REAL_ROUTE_TSP_COST)
					new_travel.cost = cost + vtDistMatrix[cur][i].dbTotalCost;
#else
					new_travel.cost = cost + vtDistMatrix[cur][i].nTotalDist;
#endif
					new_travel.idx = i;
					new_travel.visited = setVisited(visited, i);
					new_travel.path.assign(path->begin(), path->end());
					new_travel.path.emplace_back(i);
					
					pq.push( new_travel );
				}
			} // for
		}
	}

	return best_path;
}


Travel tsp_manhattan_branch_and_bound(IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const int count, int start)
{
	Travel best_path;

	if (!vtDistMatrix.empty() && count > 0 && start >= 0) {
		priority_queue<Travel, vector<Travel>, cmpTravel> pq;
		const double best = INF;

		// set start
		Travel travel;
		travel.cost = 0.f;
		travel.idx = start;
		travel.visited = setVisited(0, start);
		travel.path.emplace_back(start);

		pq.push(travel);

		while (!pq.empty()) {
			auto top = pq.top(); pq.pop();
			auto cost = top.cost;
			auto cur = top.idx;
			auto visited = top.visited;
			auto path = &top.path;
			//LOG_TRACE(LOG_DEBUG, "cost:%f, cur:%d, pathSize:%d [ ", cost, cur, path->size()); for (int k = 0; k < path->size(); ++k) LOG_TRACE(LOG_DEBUG, "%d-", path->at(k)); printf(" ]\n");

			if (cost >= best) { continue; }

			// check finish
			if (path->size() == count) {

				// set best travel
				best_path.cost = cost;
				best_path.idx = cur;
				best_path.visited = visited;
				best_path.path.assign(path->begin(), path->end());
				break;
			}

			// prapagation
			for (int i = 0; i < count; ++i) {
				if (getVisited(visited, i) == false) {

					// set new travel
					Travel new_travel;
#if defined(USE_REAL_ROUTE_TSP_COST)
					new_travel.cost = cost + vtDistMatrix[cur][i].dbTotalCost;
#else
					new_travel.cost = cost + vtDistMatrix[cur][i].nTotalDist;
#endif
					new_travel.idx = i;
					new_travel.visited = setVisited(visited, i);
					new_travel.path.assign(path->begin(), path->end());
					new_travel.path.emplace_back(i);

					pq.push(new_travel);
				}
			} // for
		}
	}

	return best_path;
}


//pair<int, vector<int>> tsp_euclidean_branch_and_bound(const vector<pair<double, double>>& coords, int start) {
pair<int, vector<int>> tsp_euclidean_branch_and_bound(IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const int count, int start) 
{
	vector<int> visited(count, 0);
	priority_queue<pair<double, pair<int, vector<int>>>, vector<pair<double, pair<int, vector<int>>>>, greater<pair<double, pair<int, vector<int>>>>> pq;
	vector<int> path = { start };
	pq.push({ 0, { start, path } });
	double best = INF;
	vector<int> best_path;

	if (!vtDistMatrix.empty() && count > 0 && start >= 0) {
		while (!pq.empty()) {
			auto top = pq.top(); pq.pop();
			auto cost = top.first;
			auto state = top.second;
			auto cur = state.first;
			auto path = state.second;
			LOG_TRACE(LOG_DEBUG, "cost:%f, cur:%d, pathSize:%d [ ", cost, cur, path.size()); for (int k = 0; k < path.size(); ++k) LOG_TRACE(LOG_DEBUG, "%d-", path[k]); printf(" ]\n");
			visited[cur] = 1;
			if (cost >= best) { continue; }
			if (path.size() == count) {
				//best = cost + euclidean_distance(coords[cur], coords[start]);
#if defined(USE_REAL_ROUTE_TSP_COST)
				best = cost + vtDistMatrix[cur][start].dbTotalCost;
#else
				best = cost + vtDistMatrix[cur][start].nTotalDist;
#endif				
				best_path = path;
				continue;
			}
			for (int i = 0; i < count; ++i) {
				if (visited[i] == 0) {
					vector<int> new_path = path;
					new_path.push_back(i);
					//pq.push({ cost + euclidean_distance(coords[cur], coords[i]),{ i, new_path } });
#if defined(USE_REAL_ROUTE_TSP_COST)
					pq.push({ cost + vtDistMatrix[cur][i].dbTotalCost,{ i, new_path } });
#else
					pq.push({ cost + vtDistMatrix[cur][i].nTotalDist,{ i, new_path } });
#endif					
				}
			}
		}
	}
	
	return{ best, best_path };
}


//void test {
//	vector<pair<double, double>> coords = {
//		{ 3.0, 3.0 },
//		{ 1.0, 1.0 },
//		{ 20.0, 20.0 },
//		{ 4.0, 4.0 },
//		{ 15.0, 15.0 },
//	};
//	int start = 0;
//	auto result = tsp_euclidean_branch_and_bound(coords, start);
//	double shortest_path = result.first;
//	vector<int> path = result.second;
//	printf("Shortest path starting from %d is: %f\n", start, shortest_path);
//	printf("Visiting order: ");
//	for (auto city : path) {
//		printf("%d, ", city);
//	}
//	printf("\n");
//}




////int completed[99] = { 0, };
double cost = 0.f;
Travel best_path;

int least(IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const int count, int c) {
	int i, nc = 999;
	uint32_t kmin = 0;
	uint32_t min = INT_MAX;

	for (i = 0; i < count; i++) {
#if defined(USE_REAL_ROUTE_TSP_COST)
		if ((vtDistMatrix[c][i].dbTotalCost != 0) && (getVisited(best_path.visited, i) == false)) {
			if (vtDistMatrix[c][i].dbTotalCost + vtDistMatrix[i][c].dbTotalCost < min) {
				min = vtDistMatrix[i][0].dbTotalCost + vtDistMatrix[c][i].dbTotalCost;
				kmin = vtDistMatrix[c][i].dbTotalCost;
				nc = i;
			}
		}
#else
		if ((vtDistMatrix[c][i].nTotalDist != 0) && (getVisited(best_path.visited, i) == false)) {
			if (vtDistMatrix[c][i].nTotalDist + vtDistMatrix[i][c].nTotalDist < min) {
				min = vtDistMatrix[i][0].nTotalDist + vtDistMatrix[c][i].nTotalDist;
				kmin = vtDistMatrix[c][i].nTotalDist;
				nc = i;
			}
		}
#endif		
	} // for

	if (min != 999)
		best_path.cost += kmin;

	return nc;
}

void mincost(IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const int count, int start) {
	int i, ncity;
	best_path.visited = setVisited(best_path.visited, start);

	//Travel new_travel;
	//best_path.cost = cost;
	//best_path.idx = start;
	best_path.path.emplace_back(start);
	
	LOG_TRACE(LOG_DEBUG_CONTINUE, "%d → ", start);
	ncity = least(vtDistMatrix, count, start);

	if (ncity == 999) {
		ncity = 0;
#if defined(USE_REAL_ROUTE_TSP_COST)
		best_path.cost += vtDistMatrix[start][ncity].dbTotalCost;
#else
		best_path.cost += vtDistMatrix[start][ncity].nTotalDist;
#endif

		LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");
		return;
	}

	mincost(vtDistMatrix, count, ncity);
}

Travel mst_manhattan_branch_and_bound2(IN const vector<vector<stDistMatrix>>& vtDistMatrix, IN const int count, int start) {
	//Travel new_travel;
	//best_path.cost = cost;
	//best_path.idx = start;

	best_path.init();

	LOG_TRACE(LOG_DEBUG_LINE, "best path : ");

	mincost(vtDistMatrix, count, start);

	return best_path;
}
