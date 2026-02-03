/*
 * sort.h
 *   created on: April 24, 2013
 * last updated: May 10, 2020
 *       author: Shujia Liu
 */

#ifndef __SORT__
#define __SORT__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <algorithm>
#include <vector>


void InitSort();
void swap(int &x, int &y);
void selectionSort(std::vector<int>& Arg, int l, int r);
int partition(std::vector<int>& Arg, int l, int r); // partition for quick sort
void quickSort(std::vector<int>& Arg, int l, int r);

class TSort{
public:
	TSort();
	~TSort();
	void index(std::vector<double>& Arg, int numOfArg, std::vector<int>& indexOrderd, int numOfOrd);
	void index(std::vector<int>& Arg, int numOfArg, std::vector<int>& indexOrderd, int numOfOrd);
	void indexB(std::vector<double>& Arg, int numOfArg, std::vector<int>& indexOrderd, int numOfOrd);
	void indexB(std::vector<int>& Arg, int numOfArg, std::vector<int>& indexOrderd, int numOfOrd);
	void sort(std::vector<int>& Arg, int numOfArg);
};

extern TSort* tSort;

#endif
