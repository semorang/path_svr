#pragma once

//from : https://rosettacode.org/wiki/K-means%2B%2B_clustering

#ifndef _GN_KMEANS_CLASSFY_HEADERS_H_
#define _GN_KMEANS_CLASSFY_HEADERS_H_

#include <vector>
#include <algorithm>

typedef struct {
	int idx;
	double x, y; 
	int group; 
} TFPoint;

class CGnKmeansClassfy
{
public:
	CGnKmeansClassfy();
	virtual ~CGnKmeansClassfy();
	void MakeTestDummyData(int nCount);
	void SetDataCount(int nCount);
	void ReSet(void);
	void SetWeightMatrix(double** ppWeightMatrix);
	void AddData(int idx, double dx, double dy);
	bool Run(int n_cluster); //result => m_vtResult
	std::vector<TFPoint> m_vtInput;
	std::vector<TFPoint> m_vtResult;
private:
	double randf(double m);
	inline double dist2(TFPoint* a, TFPoint* b);
	inline int nearest(TFPoint* pt, TFPoint* cent, int n_cluster, double *d2);
	void kpp(TFPoint* pts, int len, TFPoint* cent, int n_cent);
	double** m_ppWeightMatrix;
};


#endif

/*사용법
#include "GnKmeansClassfy.h"
#include <math.h>
RECT MakeRect(int x, int y, int nn)
{
	RECT rct = {x-nn, y-nn, x+nn, y+nn};
	return rct;
}
void CTest_Kmeans_ClassificationDlg::OnBnClickedButton1()
{
	RECT rect;
	GetClientRect(&rect);

	int nPointCount = 10000;
	int nGroupCount = 100;
	int nScreenWidth = rect.right;
	int nScreenHeight = rect.bottom;

	CGnKmeansClassfy kmc;
	//1. Test포인트 생성
	kmc.MakeTestDummyData(nPointCount);
	//2. 결과 생성
	kmc.Run(nGroupCount);
	TFPoint* g_v = &kmc.m_vtInput[0];
	TFPoint* g_c = &kmc.m_vtResult[0];

	HDC dc = ::GetDC(m_hWnd);
	HBRUSH pBrsh = (HBRUSH)::SelectObject(dc, CreateSolidBrush(RGB(0,0,0)));
	Rectangle(dc, rect.left, rect.top, rect.right, rect.bottom);
	::DeleteObject(::SelectObject(dc, pBrsh));

	//3. 결과 출력..
	int i, j;
	TFPoint* p, *c;
	double min_x, max_x, min_y, max_y, scale, cx, cy;
	unsigned char *colors = (unsigned char*)malloc(sizeof(unsigned char) * nGroupCount * 3);

	for ( i = 0; i < nGroupCount; i++) {
		colors[3*i + 0] = 255 * (3 * (i + 1) % 11)/11.;
		colors[3*i + 1] = 255 * (7 * i % 11)/11.;
		colors[3*i + 2] = 255 * (9 * i % 11)/11.;
	}

	max_x = max_y = -(min_x = min_y = HUGE_VAL);
	for (j = 0, p = g_v; j < nPointCount; j++, p++) {
		if (max_x < p->x) max_x = p->x;
		if (min_x > p->x) min_x = p->x;
		if (max_y < p->y) max_y = p->y;
		if (min_y > p->y) min_y = p->y;
	}
	scale = nScreenWidth / (max_x - min_x);
	if (scale > nScreenHeight / (max_y - min_y)) scale = nScreenHeight / (max_y - min_y);
	cx = (max_x + min_x) / 2;
	cy = (max_y + min_y) / 2;

	//printf("%%!PS-Adobe-3.0\n%%%%BoundingBox: -5 -5 %d %d\n", nScreenWidth + 10, nScreenHeight + 10);
	pBrsh = (HBRUSH)::SelectObject(dc, CreateSolidBrush(RGB(255,255,255)));
	Rectangle(dc, rect.left, rect.top, rect.right, rect.bottom);
	::DeleteObject(::SelectObject(dc, pBrsh));

	// 	printf( "/l {rlineto} def /m {rmoveto} def\n"
	// 		"/c { .25 sub exch .25 sub exch .5 0 360 arc fill } def\n"
	// 		"/s { moveto -2 0 m 2 2 l 2 -2 l -2 -2 l closepath "
	// 		"	gsave 1 setgray fill grestore gsave 3 setlinewidth"
	// 		" 1 setgray stroke grestore 0 setgray stroke }def\n"
	// 		);
	for (c = g_c, i = 0; i < nGroupCount; i++, c++) {
		//printf("%g %g %g setrgbcolor\n", colors[3*i], colors[3*i + 1], colors[3*i + 2]);
		pBrsh = (HBRUSH)::SelectObject(dc, CreateSolidBrush(RGB(colors[3*i], colors[3*i + 1], colors[3*i + 2])));
		for (j = 0, p = g_v; j < nPointCount; j++, p++) {
			if (p->group != i) continue;
			//printf("%.3f %.3f c\n", (p->x - cx) * scale + nScreenWidth / 2, (p->y - cy) * scale + nScreenHeight / 2);
			rect = MakeRect((p->x - cx) * scale + nScreenWidth / 2, (p->y - cy) * scale + nScreenHeight / 2, 3);
			Ellipse(dc, rect.left, rect.top, rect.right, rect.bottom);
		}
		::DeleteObject(::SelectObject(dc, pBrsh));

		printf("\n0 setgray %g %g s\n", (c->x - cx) * scale + nScreenWidth / 2, (c->y - cy) * scale + nScreenHeight / 2);
		pBrsh = (HBRUSH)::SelectObject(dc, CreateSolidBrush(RGB(255, 0, 0)));
		rect = MakeRect((c->x - cx) * scale + nScreenWidth / 2, (c->y - cy) * scale + nScreenHeight / 2, 5);
		//Ellipse(dc, rect.left, rect.top, rect.right, rect.bottom);
		::DeleteObject(::SelectObject(dc, pBrsh));
	}
	//printf("\n%%%%EOF");
	free(colors);
	kmc.m_vtInput.clear();
	kmc.m_vtResult.clear();

	::ReleaseDC(m_hWnd,dc);
}
*/