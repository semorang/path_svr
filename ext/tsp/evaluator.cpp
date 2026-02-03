/*
 * evaluator.cpp
 *   created on: April 24, 2013
 * last updated: May 10, 2020
 *       author: Shujia Liu
 */

#ifndef __EVALUATOR__
#include "evaluator.h"
#endif

using namespace std;

// #include <math.h>
// #include <iostream>

TEvaluator::TEvaluator() {
	Ncity = 0;
	fNearNumMax = 50;
}

TEvaluator::~TEvaluator() {}

void TEvaluator::setInstance(const vector<stWaypoint>& vtCities, const vector<vector<stDistMatrix>>& vtDistMatrix, const int compareType) {
	Ncity = vtCities.size();
	fNearNumMax = Ncity - 1;
	x.resize(Ncity);
	y.resize(Ncity);
	vector<int> checkedN(Ncity);
	for (int i = 0; i < Ncity; ++i) {
		x[i] = vtCities[i].position.x * 1000;
		y[i] = vtCities[i].position.y * 1000;
	} //for

	fEdgeDis.clear();
	for (int i = 0; i < Ncity; i++) {
		vector<int> row(Ncity);
		fEdgeDis.push_back(row);
	}

	fNearCity.clear();
	for (int i = 0; i < Ncity; i++) {
		vector<int> row(fNearNumMax + 1);
		fNearCity.push_back(row);
	}


#if 1 // using provide table
	// std::cout << fEdgeDis.size() << " " << fEdgeDis[0].size() << std::endl;
	printf("%d %d\n", fEdgeDis.size(), fEdgeDis[0].size());
	for (int i = 0; i < Ncity; ++i) {
		for (int j = 0; j < Ncity; ++j) {
			if (!vtDistMatrix.empty()/* && ppResultTables[i] && ppResultTables[i][j]*/) {
				if (compareType == TYPE_TSP_VALUE_DIST) {
					if (i < j) 
						fEdgeDis[i][j] = vtDistMatrix[i][j].nTotalDist;
					else 
						fEdgeDis[i][j] = vtDistMatrix[j][i].nTotalDist;
				} else if (compareType == TYPE_TSP_VALUE_TIME) {
					if (i < j) 
						fEdgeDis[i][j] = vtDistMatrix[i][j].nTotalTime;
					else 
						fEdgeDis[i][j] = vtDistMatrix[j][i].nTotalTime;
				} else {
					if (i < j) 
						fEdgeDis[i][j] = vtDistMatrix[i][j].dbTotalCost;
					else 
						fEdgeDis[i][j] = vtDistMatrix[j][i].dbTotalCost;
				}
			}
			else {
				fEdgeDis[i][j] = (int)(sqrt((x[i] - x[j])*(x[i] - x[j]) + (y[i] - y[j])*(y[i] - y[j])) + 0.5);
			}
		} // for j
	} // for i
#else
	if (strcmp(type, "EUC_2D") == 0) {
		std::cout << fEdgeDis.size() << " " << fEdgeDis[0].size() << std::endl;
		for (int i = 0; i < Ncity; ++i)
			for (int j = 0; j < Ncity; ++j) {
				fEdgeDis[i][j] = (int)(sqrt((x[i] - x[j])*(x[i] - x[j]) + (y[i] - y[j])*(y[i] - y[j])) + 0.5);
			}
	}
	else if (strcmp(type, "ATT") == 0) {
		for (int i = 0; i < Ncity; ++i) {
			for (int j = 0; j < Ncity; ++j) {
				double r = (sqrt(((x[i] - x[j])*(x[i] - x[j]) + (y[i] - y[j])*(y[i] - y[j])) / 10.0));
				int t = (int)r;
				if ((double)t < r) {
					fEdgeDis[i][j] = t + 1;
				}
				else {
					fEdgeDis[i][j] = t;
				}
			}
		}
	}
	else if (strcmp(type, "CEIL_2D") == 0) {
		for (int i = 0; i < Ncity; ++i) {
			for (int j = 0; j < Ncity; ++j) {
				fEdgeDis[i][j] = (int)ceil(sqrt((x[i] - x[j])*(x[i] - x[j]) + (y[i] - y[j])*(y[i] - y[j])));
			}
		}
	}
	else {
		printf("EDGE_WEIGHT_TYPE is not supported\n");
		exit(1);
	}
#endif 

	int ci, j1, j2, j3;
	int cityNum = 0;
	int minDis;
	for (ci = 0; ci < Ncity; ++ci) {
		for (j3 = 0; j3 < Ncity; ++j3) checkedN[j3] = 0;
		checkedN[ci] = 1;
		fNearCity[ci][0] = ci;
		for (j1 = 1; j1 <= fNearNumMax; ++j1) {
			minDis = 100000000;
			for (j2 = 0; j2 < Ncity; ++j2) {
				if (fEdgeDis[ci][j2] <= minDis && checkedN[j2] == 0) {
					cityNum = j2;
					minDis = fEdgeDis[ci][j2];
				}
			}
			fNearCity[ci][j1] = cityNum;
			checkedN[cityNum] = 1;
		}
	}
}

void TEvaluator::doIt( TIndi& indi ) {
	int d = 0;
	for( int i = 0; i < Ncity; ++i ) d += fEdgeDis[ i ][ indi.fLink[i][0] ] + fEdgeDis[ i ][ indi.fLink[i][1] ];
	indi.fEvaluationValue = d/2;
}

void TEvaluator::resultTo(vector<int>& array, TIndi& indi) {
	array.resize(Ncity);
	int curr = 0, st = 0, count = 0, pre = -1, next;
	while (1) {
		array[count++] = curr; // +1;
		if (count >= Ncity) {
			printf("Invalid\n");
			return;
		}
		if (indi.fLink[curr][0] == pre) next = indi.fLink[curr][1];
		else next = indi.fLink[curr][0];

		pre = curr;
		curr = next;
		if (curr == st) break;
	}
	if (this->checkValid(array, indi.fEvaluationValue) == false) {
	//	printf("Individual is invalid \n");
	}
	//fprintf(fp, "%d %d\n", indi.fN, indi.fEvaluationValue);
	//for (int i = 0; i < indi.fN; ++i)
	//	fprintf(fp, "%d ", Array[i]);
	//fprintf(fp, "\n");
}

void TEvaluator::writeTo( FILE* fp, TIndi& indi ){
	Array.resize(Ncity);
	int curr=0, st=0, count=0, pre=-1, next;
	while( 1 ){
		Array[count++] = curr; // +1;
		if( count > Ncity ){
			printf( "Invalid\n" );
			return;
		}
		if( indi.fLink[ curr ][ 0 ] == pre ) next = indi.fLink[ curr ][ 1 ];
		else next = indi.fLink[ curr ][ 0 ];

		pre = curr;
		curr = next;
		if( curr == st ) break;
	}
	if( this->checkValid( Array, indi.fEvaluationValue ) == false )
		printf( "Individual is invalid \n" );

	fprintf( fp, "%d %d\n", indi.fN, indi.fEvaluationValue );
	for( int i = 0; i < indi.fN; ++i )
		fprintf( fp, "%d ", Array[ i ] );
	fprintf( fp, "\n" );
}

bool TEvaluator::checkValid(vector<int>& array, int value) {
	int *check=new int[Ncity];
	for( int i = 0; i < Ncity; ++i ) check[ i ] = 0;
	for( int i = 0; i < Ncity; ++i ) ++check[ array[ i ]-1 ];
	for( int i = 0; i < Ncity; ++i )
		if (check[i] != 1) {
			//printf("if (check[%d] != 1) - ", i);
			return false;
		}
	int distance = 0;
	for( int i = 0; i < Ncity-1; ++i )
		distance += fEdgeDis[ array[ i ]-1 ][ array[ i+1 ]-1 ];

	distance += fEdgeDis[ array[ Ncity-1 ]-1 ][ array[ 0 ]-1 ];

	delete [] check;
	if (distance != value) {
		//printf("if (distance(%d) != value(%d)) - ", distance, value);
		return false;
	}
	return true;
}

