//////////////////////////////////////////////////////////////////////
//
// UniDib32.h
// Simple Ver by GiJoe, 2019.09.01
//
//////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _GIJOE_UNIDIB32_H_
#define _GIJOE_UNIDIB32_H_

/*
//#ifndef _WINDOWS
#ifndef TRUE
typedef int BOOL;
#define TRUE 1
#define FALSE 0
typedef unsigned char BYTE, * LPBYTE;
typedef unsigned short SHORT, WORD, * LPWSTR;
typedef unsigned int DWORD;
typedef int INT;
typedef int LONG;
typedef float FLOAT;
typedef double DOUBLE;
typedef DWORD COLORREF;
typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} 	RECT;

typedef struct
{
	int x;
	int y;
} 	POINT;

#define LOBYTE(w)           ((unsigned char)(((unsigned int)(w)) & 0xff))
#define GetRValue(rgb)      (LOBYTE(rgb))
#define GetGValue(rgb)      (LOBYTE(((unsigned short)(rgb)) >> 8))
#define GetBValue(rgb)      (LOBYTE((rgb)>>16))
#endif //if !defined "TRUE"
//#endif //if !defined "_WINDOWS"
*/


typedef struct
{
	int x;
	int y;
} iPOINT;

typedef struct
{
	double x;
	double y;
} dPOINT;

#ifndef TRUE
typedef unsigned long DWORD;
typedef int BOOL;
#define TRUE 1
#define FALSE 0
#endif

#if !defined(_CSIZE__)
#define _CSIZE__
typedef struct
{
	int cx;
	int cy;
} 	CSIZE;
#endif

#ifndef RGBA
#define RGBA(r,g,b,a) ((DWORD)(((unsigned char)(b)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned short)(unsigned char)(r))<<16)|(((unsigned short)(unsigned char)(a))<<24)))
//#define GetRValue(rgb)      ((BYTE)(((unsigned long)(rgb)) & 0xff))
//#define GetGValue(rgb)      ((BYTE)(((unsigned long)(((WORD)(rgb)) >> 8)) & 0xff))   
//#define GetBValue(rgb)      ((BYTE)(((unsigned long)(((WORD)(rgb)) >> 16)) & 0xff))   
#endif


#include <math.h>

class CUniDib32
{
public:
	CUniDib32();
	CUniDib32(int Width, int Height, void* pImageBuffer = NULL);
	virtual ~CUniDib32();
	void Create(int Width, int Height, void* pImageBuffer = NULL);
	void Destroy();

	void Init(int R, int G, int B);
	void FillRect(int x, int y, int w, int h, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA=255); //No SVG
	void ThickLine(float ax, float ay, float bx, float by, float fDiaWidth, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA);
	void ThickLine(int ax, int ay, int bx, int by, float fDiaWidth, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA);
	void ThickPolyLine(iPOINT* pPts, int nCount, float fDiaWidth, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA, BOOL bClose = FALSE);
	void ThickPolyDashLine(iPOINT* pPts, int nCount, float fDiaWidth, int dash_len, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA, BOOL bClose = FALSE);
	void ThickDashLine(int x0, int y0, int x1, int y1, float fDiaWidth, int dash_len, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA, BOOL bClose = FALSE);

	void FillPolygon(iPOINT poly[], int npoly, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA);
	void FillEllipse(int xc, int yc, float r, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA);
	void DrawArrowOnLine(int x0, int y0, int x1, int y1, int head_length, int head_width, unsigned char byR, unsigned char byG, unsigned char byB, unsigned char byA);

	__inline float capsuleSDF(float px, float py, float ax, float ay, float bx, float by, float r);

	//__inline void alphablend(int x, int y, float alpha, unsigned char r, unsigned char g, unsigned char b, unsigned char a=255);
	void BltFromAlpha(int dsc_x, int dsc_y, unsigned char* pBitsSrc, int src_cx, int src_cy, unsigned char R=120, unsigned char G=120, unsigned char B=120);
	void BltFromEx(int dsc_x, int dsc_y, unsigned char* pBitsSrc, int src_cx, int src_cy, float fscale);
	void BltFrom(int dsc_x, int dsc_y, unsigned char* pBitsSrc, int src_cx, int src_cy, BOOL bUseAlpha = TRUE);
	void BltFrom(int dsc_x, int dsc_y, unsigned char* pBitsSrc, int src_cx, int src_x1, int src_y1, int src_x2, int src_y2, BOOL bUseAlpha=TRUE);

	void InverseY();
	//	BOOL Display( HDC hdc, int dsc_x, int dsc_y, int dsc_cx, int dsc_cy, int src_x, int src_y, int src_cx, int src_cy );
public:
	int m_nWidth;
	int m_nHeight;
	BOOL m_bUseExtBuffer;
	DWORD* m_pBits;
	float m_preBuffer_nodeX[8192];
};

#endif // !defined(_GIJOE_UNIDIB32_H_)