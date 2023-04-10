#include "../stdafx.h"

/// Dib32.cpp
// Dib.cpp: interface for the CUniDib32 class
// Improved Simple Ver by GiJoe, 2019.09.01
//
//////////////////////////////////////////////////////////////////////
#include "UniDib32.h"
#include <string.h>
#include <stdlib.h>
#include <vector>

#ifndef _WINDOWS
#include <iconv.h>
#else
#include <windows.h>
#include <assert.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUniDib32::CUniDib32() {
	m_pBits = NULL;
	m_nHeight = m_nWidth = 0;
	m_bUseExtBuffer = FALSE;
}

CUniDib32::CUniDib32(int Width, int Height, void* pImageBuffer) {
	Create(Width, Height, pImageBuffer);
}

CUniDib32::~CUniDib32() {
	Destroy();
}

void CUniDib32::Destroy() {
	if (!m_bUseExtBuffer) {
		if (m_pBits) free(m_pBits);
		m_pBits = NULL;
	}
	m_bUseExtBuffer = FALSE;
	m_nHeight = m_nWidth = 0;
}

void CUniDib32::Create(int Width, int Height, void* pImageBuffer) {
	m_nWidth = Width;
	m_nHeight = Height;
	if (pImageBuffer) {
		m_pBits = (DWORD*)pImageBuffer; m_bUseExtBuffer = TRUE;
	}
	else {
		if (m_pBits)
			free(m_pBits);
		m_pBits = (DWORD*)malloc(m_nWidth * m_nHeight * sizeof(DWORD)); m_bUseExtBuffer = FALSE;
	}
}

void CUniDib32::Init(int R, int G, int B)
{
	DWORD Color = RGBA(B, G, R, 255);
	int Size = m_nWidth * m_nHeight;

	for (int i = 0; i < Size; i++)
		m_pBits[i] = Color;
}

void CUniDib32::FillRect(int xSt, int ySt, int wdt, int hgt, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA)
{
	if (wdt <= 0 || hgt <= 0) return;
	if (xSt + wdt <= 0 || ySt + hgt <= 0) return;
	if (m_nWidth <= xSt || m_nHeight <= ySt) return;

	if (xSt < 0 && 0 < xSt + wdt) { wdt += xSt; xSt = 0; } //왼쪽이 걸친경우
	if (ySt < 0 && 0 < ySt + hgt) { hgt += ySt; ySt = 0; } //윗쪽이 걸친경우

	if (xSt < m_nWidth && m_nWidth < xSt + wdt) { wdt = m_nWidth - xSt; } //오른쪽이 걸친경우
	if (ySt < m_nHeight && m_nHeight < ySt + hgt) { hgt = m_nHeight - ySt; } //아래쪽이 걸친경우

	// Do Fill
	if (byA < 254) {
		// Prepare Buffer Address
		DWORD* dst1 = m_pBits + (ySt * m_nWidth) + xSt;
		while (hgt--) {
			unsigned char* dst = (unsigned char*)dst1;
			for (int i = 0; i < wdt; i++, dst+=4) {
				//dst[i] = Color;
				dst[0] = (BYTE)(((byB - dst[0]) * byA + (dst[0] << 8)) >> 8); //B
				dst[1] = (BYTE)(((byG - dst[1]) * byA + (dst[1] << 8)) >> 8); //G
				dst[2] = (BYTE)(((byR - dst[2]) * byA + (dst[2] << 8)) >> 8); //R //A
			} //for
			dst1 += m_nWidth;
		} //while
	}
	else {
		// Prepare Buffer Address
		DWORD* dst = m_pBits + (ySt * m_nWidth) + xSt;
		DWORD Color = RGBA(byB, byG, byR, 255);

		while (hgt--) {
			for (int i = 0; i < wdt; i++) dst[i] = Color;
			dst += m_nWidth;
		}
	}


}
/*
void CUniDib32::FillRect ( int x1, int y1, int x2, int y2, int R, int G, int B )
{
	if (x2 < 0 || y2 < 0 || m_nWidth < x1 || m_nHeight < y1) return;

	if (x1 < 0 && 0 <= x2) x1 = 0;
	if (x1 <= m_nWidth && m_nWidth <= x2) x2 = m_nWidth-1;
	if (y1 < 0 && 0 <= y2) y1 = 0;
	if (y1 <= m_nHeight && m_nHeight <= y2) y2 = m_nHeight-1;

	if (0 <= x1 && x2 < m_nWidth && 0 <= y1 && y2 < m_nHeight) {
		DWORD Color=RGBA( B, G, R, 255 );
		for (int y = y1; y < y2; y++) {
			DWORD *dst=m_pBits+(y*m_nWidth);
			for (int x = x1; x < x2; x++)
				*(dst + x)=Color;
		}//for y
	} //if
return;


	// Clip Rect
	int px=(x1>=0) ? x1 : 0;
	int py=(y1>=0) ? y1 : 0;
	int dx=(x2<m_nWidth) ? (x2-x1+1) : (m_nWidth-x1);
	int dy=(y2<m_nHeight) ? (y2-y1+1) : (m_nHeight-y1);
	dx=(x1>=0) ? dx : dx + x1;
	dy=(y1>=0) ? dy : dy + y1;

	// If Nothing to Fill return
	if ( (dx<=0) || (dy<=0) )
		return;

	// Prepare Buffer Address
	DWORD *dst=m_pBits+(py*m_nWidth)+px;
	DWORD Color=RGBA( B, G, R, 255 );

	// Do Fill
	while ( dy-- ) {
		for ( int i=0; i<dx; i++ ) dst[i]=Color;
		dst+=m_nWidth;
	}
}
*/

#if !defined(MIN)
#define MIN(A,B) ((A) < (B) ? (A) : (B))
#endif

#if !defined(MAX)
#define MAX(A,B) ((A) > (B) ? (A) : (B))
#endif

float CUniDib32::capsuleSDF(float x, float y, float ax, float ay, float bx, float by, float r) {
	float vx = x - ax, vy = y - ay, ux = bx - ax, uy = by - ay;
	float t = MAX(MIN((vx * ux + vy * uy) / (ux * ux + uy * uy), 1.0f), 0.0f);
	float dx = vx - ux * t, dy = vy - uy * t;
	return sqrtf(dx * dx + dy * dy) - r;
}

//void CUniDib32::alphablend(int x, int y, float alpha, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
//	if (alpha < 0.1f) return; //10%투명은 버리기??
//	unsigned char* p = ((unsigned char*)m_pBits) + (y * m_nWidth + x) * 4;
//	if (a == 255) { //??? Solid ????.
//		*p = (unsigned char)(*p * (1 - alpha) + b * alpha); ++p; //Blue
//		*p = (unsigned char)(*p * (1 - alpha) + g * alpha); ++p; //Green
//		*p = (unsigned char)(*p * (1 - alpha) + r * alpha); ++p; //Red
//		*p = a;
//	} else { //unsigned char a???? ????????? ?????? ???ε? ??????? ??.
//		*p = (unsigned char)((((unsigned char)(*p * (1 - alpha) + b * alpha) - *p)*a + (*p << 8)) >> 8);	 ++p;//Blue
//		*p = (unsigned char)((((unsigned char)(*p * (1 - alpha) + g * alpha) - *p)*a + (*p << 8)) >> 8);	 ++p;//Green
//		*p = (unsigned char)((((unsigned char)(*p * (1 - alpha) + r * alpha) - *p)*a + (*p << 8)) >> 8);	 ++p;//Red
//		*p = a;
//	}
//}

void CUniDib32::ThickLine(int ax, int ay, int bx, int by, float fDiaWidth, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA) {
	ThickLine((float)ax, (float)ay, (float)bx, (float)by, fDiaWidth, byR, byG, byB, byA);
}

void CUniDib32::ThickLine(float ax, float ay, float bx, float by, float fDiaWidth, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA) {
	int x0 = (MIN(ax, bx) - fDiaWidth);
	int x1 = (MAX(ax, bx) + fDiaWidth);
	int y0 = (MIN(ay, by) - fDiaWidth);
	int y1 = (MAX(ay, by) + fDiaWidth);

	if (x1 < 0 || y1 < 0 || m_nWidth <= x0 || m_nHeight <= y0) return;

	if (x0 < 0 && 0 <= x1) x0 = 0;
	if (y0 < 0 && 0 <= y1) y0 = 0;

	if (x0 <= m_nWidth && m_nWidth <= x1) x1 = m_nWidth - 1;
	if (y0 <= m_nHeight && m_nHeight <= y1) y1 = m_nHeight - 1;

#if 0 //old code
	for (int y = y0; y <= y1; y++)
		for (int x = x0; x <= x1; x++)
			alphablend(x, y, MIN(0.5f - capsuleSDF(x, y, ax, ay, bx, by, fDiaWidth), 1.0f), byR, byG, byB, byA);
#else
	float alpha = 0.f;
	unsigned char* p;
	if (byA > 250) {
		for (int y = y0; y <= y1; y++) {
			for (int x = x0; x <= x1; x++) {
				p = ((unsigned char*)m_pBits) + ((y * m_nWidth + x) << 2); //* 4;
				alpha = MIN(0.5f - capsuleSDF(x, y, ax, ay, bx, by, fDiaWidth), 1.0f);
				if (alpha < 0.1f) continue;
				*p = (unsigned char)(*p * (1. - alpha) + byB * alpha); ++p; //Blue
				*p = (unsigned char)(*p * (1. - alpha) + byG * alpha); ++p; //Green
				*p = (unsigned char)(*p * (1. - alpha) + byR * alpha); ++p; //Red
				*p = byA;
			} //for
		} //for
	}
	else {
		for (int y = y0; y <= y1; y++) {
			for (int x = x0; x <= x1; x++) {
				p = ((unsigned char*)m_pBits) + ((y * m_nWidth + x) << 2); //* 4;
				alpha = MIN(0.5f - capsuleSDF(x, y, ax, ay, bx, by, fDiaWidth), 1.0f);
				if (alpha < 0.1f) continue;
				*p = (unsigned char)((((unsigned char)(*p * (1. - alpha) + byB * alpha) - *p) * byA + (*p << 8)) >> 8);	 ++p;//Blue
				*p = (unsigned char)((((unsigned char)(*p * (1. - alpha) + byG * alpha) - *p) * byA + (*p << 8)) >> 8);	 ++p;//Green
				*p = (unsigned char)((((unsigned char)(*p * (1. - alpha) + byR * alpha) - *p) * byA + (*p << 8)) >> 8);	 ++p;//Red
				*p = byA;
			} //for
		} //for
	}
#endif
}

void CUniDib32::ThickPolyLine(iPOINT* pPts, int nCount, float fDiaWidth, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA, BOOL bClose) {
	for (int i = 1; i < nCount; ++i)
		ThickLine(pPts[i - 1].x, pPts[i - 1].y, pPts[i].x, pPts[i].y, fDiaWidth, byR, byG, byB, byA);

	if (bClose)
		if (pPts[0].x != pPts[nCount - 1].x || pPts[0].y != pPts[nCount - 1].y)
			ThickLine(pPts[0].x, pPts[0].y, pPts[nCount - 1].x, pPts[nCount - 1].y, fDiaWidth, byR, byG, byB, byA);
}

int comp(const void* a, const void* b) { return *(float*)a - *(float*)b; }
void CUniDib32::FillPolygon(iPOINT poly[], int npoly, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA)
{
	if (npoly < 3 || byA <= 10) return;
	if (poly[0].x == poly[npoly - 1].x && poly[0].y == poly[npoly - 1].y && npoly == 3) return;

	int i, x, y, mnx, mny, mxx, mxy, st_X, ed_X;
	float* nodeX, * pnodeX;
	int nodes;
	unsigned char* pBits;
	BYTE aa;
	iPOINT* ppoly, * ppoly2;

	//for extending ---------------------------
	int st_Y, ed_Y;
	float* nodeY, * pnodeY;
	//-----------------------------------------

	ppoly = &poly[0];
	mnx = mxx = poly->x;
	mny = mxy = poly->y;

	for (i = npoly; i--; ++ppoly) {
		if (mnx > ppoly->x) mnx = ppoly->x;
		if (mxx < ppoly->x) mxx = ppoly->x;
		if (mny > ppoly->y) mny = ppoly->y;
		if (mxy < ppoly->y) mxy = ppoly->y;
	}

	//ASSERT(npoly <= 4096);

	//Y축에 대한 계산(Strandard)
	nodeX = &m_preBuffer_nodeX[0]; //(int*)malloc(sizeof(int)* npoly); // new int[npoly]; //never exceeds npoly;
	for (y = MAX(mny, 0); y <= mxy; y++) {
		//if (y < 0) continue;
		if (m_nHeight <= y) break;

		memset(nodeX, 0x00, sizeof(float) * 2 * npoly);
		// find intersection nodes;
		nodes = 0;
		pnodeX = &nodeX[0];
		ppoly = &poly[0];
		ppoly2 = &poly[npoly - 1];

		for (i = npoly; i--; ppoly2 = ppoly++) {
			if ((ppoly->y < y && ppoly2->y >= y) || (ppoly2->y < y && ppoly->y >= y)) //수평인 에지는 그리지 않는다(부등식을 자세히 보라)
			{
				*pnodeX = (float)(ppoly->x + (double)((y - ppoly->y) * (ppoly2->x - ppoly->x)) / (double)(ppoly2->y - ppoly->y)); //no need round to integer for Alias
				++pnodeX;
				++nodes;
			} //if
		} //for
		// sort nodes (ascending order);
		qsort(nodeX, nodes, sizeof(float), comp);

		// fill the pixels between node pairs.
		pnodeX = &nodeX[0];
		for (i = nodes; i--; pnodeX += 2)
		{
			if (*(pnodeX) >= mxx) break;
			if (*(pnodeX + 1) > mnx)
			{
				if (*(pnodeX) < 0) *(pnodeX) = 0;
				if (*(pnodeX + 1) >= mxx) *(pnodeX + 1) = mxx;

				st_X = (*(pnodeX));
				ed_X = (*(pnodeX + 1));

				if (*(pnodeX)-st_X > 0.00) {
					aa = 255 - (255. * (*(pnodeX)-st_X));
					if (byA < 250) aa = floor(aa * (float)(byA / 255.) + 0.5);
					if (st_X < m_nWidth) {
						unsigned char* pBits = (unsigned char*)m_pBits + (((y * m_nWidth) + st_X) << 2);
						*pBits = (unsigned char)(((byB - *pBits) * aa + (*pBits << 8)) >> 8); pBits++;
						*pBits = (unsigned char)(((byG - *pBits) * aa + (*pBits << 8)) >> 8); pBits++;
						*pBits = (unsigned char)(((byR - *pBits) * aa + (*pBits << 8)) >> 8); pBits++;
						*pBits = aa;
					} //if
					st_X += 1;
				} //if

				if (*(pnodeX + 1) - ed_X > 0.00) {
					aa = (255. * (*(pnodeX + 1) - ed_X));
					if (byA < 250) aa = floor(aa * (float)(byA / 255.) + 0.5);
					if ((ed_X + 1) < m_nWidth) {
						unsigned char* pBits = (unsigned char*)m_pBits + (((y * m_nWidth) + (ed_X + 1)) << 2);
						*pBits = (unsigned char)(((byB - *pBits) * aa + (*pBits << 8)) >> 8); pBits++;
						*pBits = (unsigned char)(((byG - *pBits) * aa + (*pBits << 8)) >> 8); pBits++;
						*pBits = (unsigned char)(((byR - *pBits) * aa + (*pBits << 8)) >> 8); pBits++;
						*pBits = aa;
					} //if
				} //if

				if (0 < st_X || 0 < ed_X) {
					for (x = st_X; x <= ed_X; x++) {
						if (x < m_nWidth) {
							unsigned char* pBits = (unsigned char*)m_pBits + (((y * m_nWidth) + x) << 2);
							*pBits = (unsigned char)(((byB - *pBits) * byA + (*pBits << 8)) >> 8); pBits++;
							*pBits = (unsigned char)(((byG - *pBits) * byA + (*pBits << 8)) >> 8); pBits++;
							*pBits = (unsigned char)(((byR - *pBits) * byA + (*pBits << 8)) >> 8); pBits++;
							*pBits = byA;
						}
						else
							break;
					} //for
				} //if
			} //if
		} //for 
	} //for
	//free(nodeX);

	//X축에 대한 계산(확장)
	nodeY = &m_preBuffer_nodeX[0]; //(int*)malloc(sizeof(int)* npoly); // new int[npoly]; //never exceeds npoly;
	for (x = mnx; x <= mxx; x++) {
		if (x < 0) continue;
		if (m_nWidth <= x) break;

		memset(nodeY, 0x00, sizeof(float) * 2 * npoly);
		// find intersection nodes;
		nodes = 0;
		pnodeY = &nodeY[0];
		ppoly = &poly[0];
		ppoly2 = &poly[npoly - 1];

		for (i = npoly; i--; ppoly2 = ppoly++) {
			if ((ppoly->x < x && ppoly2->x >= x) || (ppoly2->x < x && ppoly->x >= x)) //수평인 에지는 그리지 않는다(부등식을 자세히 보라)
			{
				*pnodeY = (float)(ppoly->y + (double)((x - ppoly->x) * (ppoly2->y - ppoly->y)) / (double)(ppoly2->x - ppoly->x)); //no need round to integer for Alias
				++pnodeY;
				++nodes;
			} //if
		} //for
		// sort nodes (ascending order);
		qsort(nodeY, nodes, sizeof(float), comp);

		// fill the pixels between node pairs.
		pnodeY = &nodeY[0];
		for (i = nodes; i--; pnodeY += 2)
		{
			if (!i) break;
			if (*(pnodeY) >= mxy) break;
			if (*(pnodeY + 1) > mny)
			{
				if (*(pnodeY) < 0) *(pnodeY) = 0;
				if (*(pnodeY + 1) >= mxy) *(pnodeY + 1) = mxy;

				st_Y = (*(pnodeY));
				ed_Y = (*(pnodeY + 1));

				if (*(pnodeY)-st_Y > 0.00) {
					aa = 255 - (255. * (*(pnodeY)-st_Y));
					if (byA < 250) aa = floor(aa * (float)(byA / 255.) + 0.5);
					if (st_Y < m_nHeight) {
						unsigned char* pBits = (unsigned char*)m_pBits + (((st_Y * m_nWidth) + x) << 2);
						*pBits = (unsigned char)(((byB - *pBits) * aa + (*pBits << 8)) >> 8); pBits++;
						*pBits = (unsigned char)(((byG - *pBits) * aa + (*pBits << 8)) >> 8); pBits++;
						*pBits = (unsigned char)(((byR - *pBits) * aa + (*pBits << 8)) >> 8); pBits++;
						*pBits = aa;
					} //if
				} //if

				if (*(pnodeY + 1) - ed_Y > 0.00) {
					aa = (255. * (*(pnodeY + 1) - ed_Y));
					if (byA < 250) aa = floor(aa * (float)(byA / 255.) + 0.5);
					if ((ed_Y + 1) < m_nHeight) {
						unsigned char* pBits = (unsigned char*)m_pBits + ((((ed_Y + 1) * m_nWidth) + x) << 2);
						*pBits = (unsigned char)(((byB - *pBits) * aa + (*pBits << 8)) >> 8); pBits++;
						*pBits = (unsigned char)(((byG - *pBits) * aa + (*pBits << 8)) >> 8); pBits++;
						*pBits = (unsigned char)(((byR - *pBits) * aa + (*pBits << 8)) >> 8); pBits++;
						*pBits = aa;
					} //if
				} //if

				//Y 축 계산시 0번째 안그려지는 이슈 패치를 위해 그림(2020/04/29)
				if (mny == st_Y && 0 <= st_Y && st_Y < m_nHeight) {
					unsigned char* pBits = (unsigned char*)m_pBits + (((st_Y * m_nWidth) + x) << 2);
					*pBits = (unsigned char)(((byB - *pBits) * byA + (*pBits << 8)) >> 8); pBits++;
					*pBits = (unsigned char)(((byG - *pBits) * byA + (*pBits << 8)) >> 8); pBits++;
					*pBits = (unsigned char)(((byR - *pBits) * byA + (*pBits << 8)) >> 8); pBits++;
					*pBits = byA;
				}
			} //if
		} //for 
	} //for
}

void CUniDib32::FillEllipse(int xc, int yc, float r, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA)
{
	int x0 = (xc - r);
	int x1 = (xc + r);
	int y0 = (yc - r);
	int y1 = (yc + r);

	if (x1 < 0 || y1 < 0 || m_nWidth < x0 || m_nHeight < y0) return;

	if (x0 < 0 && 0 <= x1) x0 = 0;
	if (x0 <= m_nWidth && m_nWidth <= x1) x1 = m_nWidth - 1;
	if (y0 < 0 && 0 <= y1) y0 = 0;
	if (y0 <= m_nHeight && m_nHeight <= y1) y1 = m_nHeight - 1;

	if (0 <= x0 && x1 <= m_nWidth && 0 <= y0 && y1 <= m_nHeight) {
#if 0 //old code
		for (int y = y0; y <= y1; y++)
			for (int x = x0; x <= x1; x++)
				alphablend(x, y, MIN(0.5f - capsuleSDF(x, y, xc, yc, xc, yc, r), 1.0f), byR, byG, byB, byA);
#else
		float alpha = 0.f;
		unsigned char* p;
		if (byA > 250) {
			for (int y = y0; y <= y1; y++) {
				for (int x = x0; x <= x1; x++) {
					p = ((unsigned char*)m_pBits) + ((y * m_nWidth + x) << 2); //* 4;
					alpha = MIN(0.5f - capsuleSDF(x, y, xc, yc, xc, yc, r), 1.0f);
					if (alpha < 0.1f) continue;
					*p = (unsigned char)(*p * (1. - alpha) + byB * alpha); ++p; //Blue
					*p = (unsigned char)(*p * (1. - alpha) + byG * alpha); ++p; //Green
					*p = (unsigned char)(*p * (1. - alpha) + byR * alpha); ++p; //Red
					*p = byA;
				} //for
			} //for
		}
		else {
			for (int y = y0; y <= y1; y++) {
				for (int x = x0; x <= x1; x++) {
					p = ((unsigned char*)m_pBits) + ((y * m_nWidth + x) << 2); //* 4;
					alpha = MIN(0.5f - capsuleSDF(x, y, xc, yc, xc, yc, r), 1.0f);
					if (alpha < 0.1f) continue;
					*p = (unsigned char)((((unsigned char)(*p * (1. - alpha) + byB * alpha) - *p) * byA + (*p << 8)) >> 8);	 ++p;//Blue
					*p = (unsigned char)((((unsigned char)(*p * (1. - alpha) + byG * alpha) - *p) * byA + (*p << 8)) >> 8);	 ++p;//Green
					*p = (unsigned char)((((unsigned char)(*p * (1. - alpha) + byR * alpha) - *p) * byA + (*p << 8)) >> 8);	 ++p;//Red
					*p = byA;
				} //for
			} //for
		}
#endif
	} //if
}

void CUniDib32::InverseY()
{
	DWORD* dst;
	DWORD* src;
	DWORD* tmp = (DWORD*)malloc(m_nWidth * m_nHeight * 4);
	int nHalfH = m_nHeight / 2;
	for (int y = 0; y < nHalfH; ++y) {
		src = (DWORD*)m_pBits + (y * m_nWidth);
		dst = (DWORD*)m_pBits + (((m_nHeight - 1) - y) * m_nWidth);
		memcpy(&tmp[0], &dst[0], m_nWidth * 4);
		memcpy(&dst[0], &src[0], m_nWidth * 4);
		memcpy(&src[0], &tmp[0], m_nWidth * 4);
	} //for
	free(tmp);
}

//------------------------------------------------------------------------------------------------------------------
void CUniDib32::BltFromAlpha(int dsc_x, int dsc_y, unsigned char* pBitsSrc, int src_cx, int src_cy, unsigned char R, unsigned char G, unsigned char B)
{
	BYTE* dst;
	BYTE* src;
	for (int y = 0; y < src_cy; ++y) {
		if (dsc_y + y < 0) continue;
		if (m_nHeight <= dsc_y + y) break;
		src = &pBitsSrc[y * src_cx];
		dst = (BYTE*)m_pBits + ((dsc_y + y) * m_nWidth + dsc_x) * 4;
		for (int x = 0; x < src_cx; ++x, src ++, dst += 4) {
			if (m_nWidth <= dsc_x + x) break;
			if (*src < 5) continue;
			if (dsc_x + x < 0) continue;

			dst[0] = (BYTE)(((B - dst[0]) * (*src) + (dst[0] << 8)) >> 8); //B
			dst[1] = (BYTE)(((G - dst[1]) * (*src) + (dst[1] << 8)) >> 8); //G
			dst[2] = (BYTE)(((R - dst[2]) * (*src) + (dst[2] << 8)) >> 8); //R //A

		} //for
	} //for
}

void CUniDib32::BltFromEx(int dsc_x, int dsc_y, unsigned char* pBitsSrc, int src_cx, int src_cy, float fscale) {
	#define _lerp(s,e,t) ((s)+(float)((e)-(s))*(t))
	#define blerp(c00, c10, c01, c11, tx, ty) (_lerp(_lerp(c00, c10, tx), _lerp(c01, c11, tx), ty))
	int newWidth = (int)floor(src_cx * fscale); //scalex;
	int newHeight = (int)floor(src_cy * fscale); //scaley;
	int x, y, i, gxi, gyi;
	float gx, gy;
	unsigned int result;
	unsigned char* pBuf = NULL;
	unsigned char *c00, *c10, *c01, *c11;

	for (x = 0, y = 0; y < newHeight; x++) {
		if (x > newWidth) { x = 0; y++; }
		if (m_nHeight <= dsc_y + y) break;
		if (dsc_y + y < 0) continue;
		if (m_nWidth <= dsc_x + x) continue;
		if (dsc_x + x < 0) continue;

		gx = x / (float)(newWidth) * (src_cx - 1);
		gy = y / (float)(newHeight) * (src_cy - 1);
		gxi = (int)gx;
		gyi = (int)gy;
		result = 0;
		c00 = pBitsSrc + ((gyi * src_cx + gxi) << 2);
		c10 = pBitsSrc + ((gyi * src_cx + (gxi + 1)) << 2);
		c01 = pBitsSrc + (((gyi + 1) * src_cx + gxi) << 2);
		c11 = pBitsSrc + (((gyi + 1) * src_cx + (gxi + 1)) << 2);

		for (i = 0; i < 4; i++)
			result |= (unsigned char)blerp(c00[i], c10[i], c01[i], c11[i], gx - gxi, gy - gyi) << (8 * i); // gxi - gx, gyi - gy); // this is shady

		pBuf = (unsigned char*)(m_pBits + ((y + dsc_y) * m_nWidth + (x + dsc_x)));
		memcpy((unsigned char*)pBuf, (unsigned char*)&result, 4);
	}
}

void CUniDib32::BltFrom(int dsc_x, int dsc_y, unsigned char* pBitsSrc, int src_cx, int src_cy, BOOL bUseAlpha)
{
	BYTE* dst;
	BYTE* src;
	for (int y = 0; y < src_cy; ++y) {
		if (dsc_y + y < 0) continue;
		if (m_nHeight <= dsc_y + y) break;
		src = (BYTE*)pBitsSrc + (y * src_cx * 4);
		dst = (BYTE*)m_pBits + ((dsc_y + y) * m_nWidth + dsc_x) * 4;
		for (int x = 0; x < src_cx; ++x, src += 4, dst += 4) {
			if (m_nWidth <= dsc_x + x) break;
			if (!src[3]) continue;
			if (dsc_x + x < 0) continue;
			if (bUseAlpha) {
				dst[0]=(BYTE)(((src[0]-dst[0])*src[3]+(dst[0]<<8))>>8); //B
				dst[1]=(BYTE)(((src[1]-dst[1])*src[3]+(dst[1]<<8))>>8); //G
				dst[2]=(BYTE)(((src[2]-dst[2])*src[3]+(dst[2]<<8))>>8); //R //A
			} else {
				memcpy(&dst[0], &src[0], 4);
			}
		} //for
	} //for
}

void CUniDib32::BltFrom(int dsc_x, int dsc_y, unsigned char* pBitsSrc, int src_cx, int src_x1, int src_y1, int src_x2, int src_y2, BOOL bUseAlpha)
{
	BYTE* dst;
	BYTE* src;
	int yInc = 0;
	for (int y = src_y1; y < src_y2; ++y, ++yInc) {
		if (dsc_y + yInc < 0) continue;
		if (m_nHeight <= dsc_y + yInc) break;
		src = (BYTE*)pBitsSrc + (y * src_cx + src_x1) * 4;
		dst = (BYTE*)m_pBits + ((dsc_y + yInc) * m_nWidth + dsc_x) * 4;
		int xInc = 0;
		for (int x = src_x1; x < src_x2; ++x, ++xInc, src += 4, dst += 4) {
			if (m_nWidth <= dsc_x + xInc) break;
			if (dsc_x + xInc < 0) continue;
			if (bUseAlpha) {
				dst[0] = (BYTE)(((src[0] - dst[0]) * src[3] + (dst[0] << 8)) >> 8); //B
				dst[1] = (BYTE)(((src[1] - dst[1]) * src[3] + (dst[1] << 8)) >> 8); //G
				dst[2] = (BYTE)(((src[2] - dst[2]) * src[3] + (dst[2] << 8)) >> 8); //R //A
			} else {
				memcpy(&dst[0], &src[0], 4);
			}
		}
	}
}

// How to draw railway
// 	for (int i = 1; i < nCount; ++i) {
// 		ThickLine(pPts[i-1].x, pPts[i-1].y, pPts[i].x, pPts[i].y, fDiaWidth * 1.4, 0, 0, 0, byA);
// 		ThickDashLine(pPts[i-1].x, pPts[i-1].y, pPts[i].x, pPts[i].y, fDiaWidth, 4, byR, byG, byB, byA);
// 	}

void CUniDib32::ThickPolyDashLine(iPOINT* pPts, int nCount, float fDiaWidth, int dash_len, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA, BOOL bClose)
{
	float len_cur = 0.0;
	float dRemain, len_sum = 0.0;
	float dx, x1, x0 = pPts[0].x;
	float dy, y1, y0 = pPts[0].y;
	float cur_pos = 0;

	struct fPOINT { float x, y; };
	fPOINT ptR;
	std::vector<fPOINT> vtResult;
	vtResult.reserve(1000);

	//FillEllipse(x0, y0, 6, 0, 0, 255, 60);

	for (int i = 1; i < nCount; ++i) {
		ptR.x = x0; ptR.y = y0;
		vtResult.push_back(ptR);
		x1 = pPts[i].x; y1 = pPts[i].y;
		dx = x1 - x0; dy = y1 - y0;
		float len_cur = sqrt(dx * dx + dy * dy);
		len_sum += len_cur;
		while (cur_pos < len_sum) {
			dRemain = len_sum - cur_pos;
			ptR.x = (x1 - (dx * dRemain) / len_cur);
			ptR.y = (y1 - (dy * dRemain) / len_cur);
			vtResult.push_back(ptR);
			cur_pos += dash_len;
		}
		x0 = x1; y0 = y1;
	} //for
	ptR.x = x1 = pPts[nCount - 1].x; ptR.y = y1 = pPts[nCount - 1].y;
	vtResult.push_back(ptR);
	//FillEllipse(ptR.x, ptR.y, 6, 255, 0, 0, 60);

	int n = 0;
	std::vector<fPOINT>::iterator pre = vtResult.begin();
	std::vector<fPOINT>::iterator cur;
	for (cur = pre + 1; cur != vtResult.end(); ++cur, ++pre) {
		if (n % 2)
			ThickLine(pre->x, pre->y, cur->x, cur->y, fDiaWidth, byR, byG, byB, byA);
		++n;
	}
}

void CUniDib32::ThickDashLine(int x0, int y0, int x1, int y1, float fDiaWidth, int dash_len, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA, BOOL bClose)
{
	float dRemain, len_sum = 0.0;
	float dx;
	float dy;
	float cur_pos = 0;
	struct fPOINT { float x, y; };

	fPOINT ptR;
	std::vector<fPOINT> vtResult;
	vtResult.reserve(1000);

	//FillEllipse(x0, y0, 6, 0, 0, 255, 60);

	ptR.x = x0; ptR.y = y0;
	vtResult.push_back(ptR);
	dx = x1 - x0; dy = y1 - y0;
	float len_cur = sqrt(dx * dx + dy * dy);
	len_sum += len_cur;
	while (cur_pos < len_sum) {
		dRemain = len_sum - cur_pos;
		ptR.x = (x1 - (dx * dRemain) / len_cur);
		ptR.y = (y1 - (dy * dRemain) / len_cur);
		vtResult.push_back(ptR);
		cur_pos += dash_len;
	}
	x0 = x1; y0 = y1;

	vtResult.push_back(ptR);
	//FillEllipse(ptR.x, ptR.y, 6, 255, 0, 0, 60);

	int n = 0;
	std::vector<fPOINT>::iterator pre = vtResult.begin();
	std::vector<fPOINT>::iterator cur;
	for (cur = pre + 1; cur != vtResult.end(); ++cur, ++pre) {
		//FillEllipse(cur->x, cur->y, 6, 0, 0, 255, 60);
		if (n % 2)
			ThickLine(pre->x, pre->y, cur->x, cur->y, fDiaWidth, byR, byG, byB, byA);
		++n;
	}
}

constexpr int fRound(float x) { return static_cast<int>(x + 0.5f); }

// Draws a line from p0 to p1 with an arrowhead at p1.  Arrowhead is outlined
// with the current pen and filled with the current brush.
void CUniDib32::DrawArrowOnLine(int x0, int y0, int x1, int y1, int head_length, int head_width, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA) {
	const float dx = static_cast<float>(x1 - x0);
	const float dy = static_cast<float>(y1 - y0);
	const auto length = sqrt(dx * dx + dy * dy);
	if (length < 1) return;

	// ux,uy is a unit vector parallel to the line.
	const auto ux = dx / length;
	const auto uy = dy / length;

	// vx,vy is a unit vector perpendicular to ux,uy
	const auto vx = -uy;
	const auto vy = ux;

	const auto half_width = 0.5f * head_width;

	const POINT arrow[3] = {
		POINT{x1, y1},
		POINT{ fRound(x1 - head_length * ux + half_width * vx),
				fRound(y1 - head_length * uy + half_width * vy) },
		POINT{ fRound(x1 - head_length * ux - half_width * vx),
				fRound(y1 - head_length * uy - half_width * vy) }
	};
	//::Polygon(hdc, arrow, 3);
	//FillPolygon((iPOINT*)arrow, 3, byR, byG, byB, byA);
	ThickPolyLine((iPOINT*)arrow, 3, 1.0, byR, byG, byB, byA, true);
}