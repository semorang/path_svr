/*
 * cross.h
 *   created on: April 24, 2013
 * last updated: May 10, 2020
 *       author: Shujia Liu
 */

#ifndef __CROSS__
#define __CROSS__

#ifndef __RAND__
#include "randomize.h"
#endif

#ifndef __SORT__
#include "sort.h"
#endif

#ifndef __INDI__
#include "indi.h"
#endif

#ifndef __EVALUATOR__
#include "evaluator.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

class TCross{
public:
	TCross( int N );
	~TCross();
	bool doIt( TIndi& tKid, TIndi& tPa2, int numOfKids, int flagP, int flagC[ 10 ], std::vector<std::vector<int>>& fEdgeFreq ); // EAX entry point
	void setParents( const TIndi& tPa1, const TIndi& tPa2, int flagC[ 10 ], int numOfKids ); // sets parents
	void setABcycle( const TIndi& parent1, const TIndi& parent2, int flagC[ 10 ], int numOfKids ); // sets ab cycle

	void swap(int &x, int &y);
	void formABcycle(); // saves ab cycle
	void changeSol( TIndi& tKid, int ABnum, int type ); // generates intermediate solution from ab cycle

	void makeCompleteSol( TIndi& tKid ); // the 5th step of EAX
	bool makeUnit(); // the 5-1th step of EAX
	void backToPa1( TIndi& tKid ); // rolls back p_a
	void goToBest( TIndi& tKid ); // sets tKid to the best solutions of child generation

	void incrementEdgeFreq(std::vector<std::vector<int>>& fEdgeFreq); // increates fEdgeFreq[][]
	int calAdpLoss(std::vector<std::vector<int>>& fEdgeFreq); // calculates the average road distance from fEdgeFreq[][]
	double calEntLoss(std::vector<std::vector<int>>& fEdgeFreq); // calculates the difference of edge entropy from fEdgeFreq[][]

	void setWeight( const TIndi& parent1, const TIndi& parent2 );	// Block2
	int	calCNaive();
	void searchEset( int num );
	void addAB( int num );
	void deleteAB( int num );

	int fNumOfGeneratedCh;
	TEvaluator* eval;
	int Npop;

private:
	TIndi tBestTmp;
	int fFlagImp;
	int fN;
	int r;
	int exam;
	int examFlag;
	int flagSt;
	int flagCycle;
	int prType;
	int chDis;
	int koritsuMany;
	int bunkiMany;
	int st;
	int ci;
	int pr;
	int stock;
	int stAppear;
	int fEvalType;
	int fEsetType;
	int fNumOfABcycleInESet;
	int fNumOfABcycle;
	int fPosiCurr;
	int fMaxNumOfABcycle;

	std::vector<int> koritsu;
	std::vector<int> bunki;
	std::vector<int> koriInv;
	std::vector<int> bunInv;
	std::vector<int> checkKoritsu;
	std::vector<int> fRoute;
	std::vector<int> fPermu;
	std::vector<int> fC;
	std::vector<int> fJun;
	std::vector<int> fOrd1;
	std::vector<int> fOrd2;

	std::vector<std::vector<int>> nearData;
	std::vector<std::vector<int>> fABcycle;

	// speeds up start
	int fNumOfUnit;
	int fNumOfSeg;
	int fNumOfSPL;
	int fNumOfElementInCU;
	int fNumOfSegForCenter;
	int fGainModi;
	int fNumOfModiEdge;
	int fNumOfBestModiEdge;
	int fNumOfAppliedCycle;
	int fNumOfBestAppliedCycle;

	std::vector<int> fOrder;
	std::vector<int> fInv;
	std::vector<int> fSegUnit;
	std::vector<int> fSegPosiList;
	std::vector<int> LinkAPosi;
	std::vector<int> fPosiSeg;
	std::vector<int> fNumOfElementInUnit;
	std::vector<int> fCenterUnit;
	std::vector<int> fListOfCenterUnit;
	std::vector<int> fSegForCenter;
	std::vector<int> fGainAB;
	std::vector<int> fAppliedCylce;
	std::vector<int> fBestAppliedCylce;

	std::vector<std::vector<int>> fSegment;
	std::vector<std::vector<int>> LinkBPosi;
	std::vector<std::vector<int>> fModiEdge;
	std::vector<std::vector<int>> fBestModiEdge;
	// speeds up end

	// block2
	int fNumOfUsedAB;
	int fNumC;
	int fNumE;
	int fTmax;
	int fMaxStag;
	int fNumOfABcycleInEset;
	int fDisAB;
	int fBestNumC;
	int fBestNumE;

	std::vector<int> fNumOfElementINAB;
	std::vector<int> fWeightSR;
	std::vector<int> fWeightC;
	std::vector<int> fUsedAB;
	std::vector<int> fMovedAB;
	std::vector<int> fABcycleInEset;

	std::vector<std::vector<int>> fInEffectNode;
	std::vector<std::vector<int>> fWeightRR;
};

#endif
