/*
 * evaluator.h
 *   created on: April 24, 2013
 * last updated: May 10, 2020
 *       author: Shujia Liu
 */

#ifndef __EVALUATOR__
#define __EVALUATOR__

#ifndef __INDI__
#include "indi.h"
#endif

#include "../tms/tms.h"

// #include <string.h>
// #include <assert.h>
// #include <vector>
// #include <string>


class TEvaluator{
public:
	TEvaluator();
	~TEvaluator();
	void setInstance(const std::vector<stWaypoint>& vtCities, const std::vector<std::vector<stDistMatrix>>& vtDistMatrix, const int compareType); // sets variables

	void doIt( TIndi& indi ); // sets indi.fEvaluationValue
	void writeTo( FILE* fp, TIndi& indi ); // prints out TSP solution
	void resultTo(std::vector<int>& array, TIndi& indi);

	bool checkValid(std::vector<int>& array, int value ); // checks if TSP solution is valid

	int fNearNumMax; // the maximum value of the number of nearby points
	std::vector<std::vector<int>> fNearCity; // NearCity[i][k] is the k points that with a shortest distance from point i
	std::vector<std::vector<int>> fEdgeDis; // EdgeDis[i][j] is the distance from city i to city j
	int Ncity; // the number of cities
	std::vector<double> x; // x[i] is the x coordinate of city i
	std::vector<double> y; // y[i] is the y coordinate of city i
	std::vector<int> Array; // the index of best solution
};

#endif
