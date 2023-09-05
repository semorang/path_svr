//#include "stdafx.h"
#include "GnKmeansClassfy.h"
#include <math.h>

CGnKmeansClassfy::CGnKmeansClassfy() {
	m_ppWeightMatrix = nullptr;
}

CGnKmeansClassfy::~CGnKmeansClassfy() {
	m_vtInput.clear();
	m_vtResult.clear();
	m_ppWeightMatrix = nullptr;
}

double CGnKmeansClassfy::randf(double m)
{
	return m * rand() / (RAND_MAX - 1.);
}

double CGnKmeansClassfy::dist2(TFPoint* a, TFPoint* b)
{
	double x = a->x - b->x, y = a->y - b->y;
	return x*x + y*y;
}

//int CGnKmeansClassfy::nearest(TFPoint* pt, TFPoint* cent, int n_cluster, double *d2)
//{
//	int i, min_i;
//	TFPoint* c;
//	double d, min_d;
//	for (c = cent, i = 0; i < n_cluster; i++, c++) {
//		min_d = HUGE_VAL;
//		min_i = pt->group;
//		for (c = cent, i = 0; i < n_cluster; i++, c++)
//			if (min_d >(d = dist2(c, pt))) { min_d = d; min_i = i; }
//	} //for
//	if (d2) *d2 = min_d;
//	return min_i;
//}

int CGnKmeansClassfy::nearest(TFPoint* pt, TFPoint* cent, int n_cluster, double *d2)
{
	int i, min_i;
	TFPoint* c;
	double d, min_d;
	for (c = cent, i = 0; i < n_cluster; i++, c++) {
		min_d = HUGE_VAL;
		min_i = pt->group;
		for (c = cent, i = 0; i < n_cluster; i++, c++) {
			if (m_ppWeightMatrix) {
				d = m_ppWeightMatrix[c->idx][pt->idx];
			} else {
				d = dist2(c, pt);
			}

			//if (d > 3000)
			//	continue;

			if (min_d > d) { min_d = d; min_i = i; }
		} // for
	} // for
	if (d2) *d2 = min_d;
	return min_i;
}

void CGnKmeansClassfy::kpp(TFPoint* pts, int len, TFPoint* cent, int n_cent)
{
	int i, j;
	int n_cluster;
	double sum, *d = (double*)malloc(sizeof(double) * len);

	TFPoint* p, c;
	cent[0] = pts[ rand() % len ];
	for (n_cluster = 1; n_cluster < n_cent; n_cluster++) {
		sum = 0;
		for (j = 0, p = pts; j < len; j++, p++) {
			nearest(p, cent, n_cluster, d + j);
			sum += d[j];
		}
		sum = randf(sum);
		for (j = 0, p = pts; j < len; j++, p++) {
			if ((sum -= d[j]) > 0) continue;
			cent[n_cluster] = pts[j];
			break;
		}
	}
	for (j = 0, p = pts; j < len; j++, p++) p->group = nearest(p, cent, n_cluster, 0);
	free(d);
}

bool CGnKmeansClassfy::Run(int n_cluster)
{
	int i, j, min_i;
	int changed;
	TFPoint* p;
	TFPoint* c;
	TFPoint* pts = &m_vtInput[0];
	int len = m_vtInput.size();
	m_vtResult.resize(n_cluster);
	TFPoint* cent = &m_vtResult[0]; //(TFPoint*)malloc(sizeof(TFPoint) * n_cluster);


	/* assign init grouping randomly */
	//for (j = 0, p = pts; j < len; j++, p++) p->group = j % n_cluster;
	/* or call k++ init */
	kpp(pts, len, cent, n_cluster);

	do {
		/* group element for centroids are used as counters */
		for (c = cent, i = 0; i < n_cluster; i++, c++) { c->group = 0; c->x = c->y = 0; } //for
		for (j = 0, p = pts; j < len; j++, p++) {
			c = cent + p->group;
			c->group++;
			c->x += p->x; c->y += p->y;
		} //for
		for (c = cent, i = 0; i < n_cluster; i++, c++) { c->x /= c->group; c->y /= c->group; } //for
		changed = 0;
		/* find closest centroid of each TFPoint* */
		for (j = 0, p = pts; j < len; j++, p++) {
			min_i = nearest(p, cent, n_cluster, 0);
			if (min_i != p->group) {
				changed++;
				p->group = min_i;
			} //if
		} //for
	} while (changed > (len >> 10)); /* stop when 99.9% of points are good */
	for (c = cent, i = 0; i < n_cluster; i++, c++) { c->group = i; } //for
	return true;
}

void CGnKmeansClassfy::SetDataCount(int nCount)
{
	m_vtInput.reserve(nCount);
}

void CGnKmeansClassfy::ReSet(void)
{
	m_vtInput.clear();
	m_vtResult.clear();
	std::vector<TFPoint>().swap(m_vtInput);
	std::vector<TFPoint>().swap(m_vtResult);
}

void CGnKmeansClassfy::SetWeightMatrix(double** ppWeightMatrix)
{
	m_ppWeightMatrix = ppWeightMatrix;
}

void CGnKmeansClassfy::AddData(int idx, double dx, double dy)
{
	TFPoint pt;
	pt.idx = idx;
	pt.x = dx; pt.y = dy; pt.group = 0;
	m_vtInput.push_back(pt);
}

void CGnKmeansClassfy::MakeTestDummyData(int nCount)
{
	#define M_PI       3.14159265358979323846
	double radius = 10; //초기값은 10 이었음, 바꾸가면서 테스트 필요..
	double ang, r;
	TFPoint* p;
	m_vtInput.resize(nCount);
	TFPoint* pt = &m_vtInput[0];
	/* note: this is not a uniform 2-d distribution */
	for (p = pt + nCount; p-- > pt;) {
		ang = randf(2 * M_PI);
		r = randf(radius);
		p->x = r * cos(ang);
		p->y = r * sin(ang);
	} //for
}