//////////////////////////////////////////////////////////////////////
//
// GnType.h
// Simple Ver by GiJoe, 2020.07.27
//
//////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _GIJOE_PUBLIC_TYPE_H_
#define _GIJOE_PUBLIC_TYPE_H_


#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#define _PI_DIV_180 0.017453292519943295
#define _180_DIV_PI 57.295779513082323
#define _DBL_PI 6.2831853071795862

struct PointN {
	int x, y;

	bool empty() const {
		return ((this->x == 0) && (this->y == 0));
	}

	bool operator== (const PointN& rhs) const {
		return ((this->x == rhs.x) && (this->y == rhs.y));
	}

	bool operator!= (const PointN& rhs) const {
		return ((this->x != rhs.x) || (this->y != rhs.y));
	}
};

struct PointF {
	double x, y;

	bool empty() const {
		return ((this->x == 0.f) && (this->y == 0.f));
	}

	bool operator== (const PointN& rhs) const {
		return ((this->x == rhs.x) && (this->y == rhs.y));
	}

	bool operator!= (const PointN& rhs) const {
		return ((this->x != rhs.x) || (this->y != rhs.y));
	}
};

typedef struct {
	int left, top, right, bottom;
} RectN;

typedef struct {
	double left, top, right, bottom;
} RectF;

typedef struct {
	PointF lt;
	PointF rt;
	PointF lb;
	PointF rb;
} BoundF;

//
//#ifndef WIN32
//	#ifndef TRUE
//		typedef int BOOL;
//		#define TRUE 1
//		#define FALSE 0
//        typedef unsigned char BYTE, *LPBYTE;
//        typedef unsigned short SHORT, *LPWSTR;
//		typedef int INT;
//		typedef int LONG;
//		typedef double DOUBLE;
//        typedef unsigned int DWORD;
//        typedef DWORD COLORREF;
//	#endif //if !defined "TRUE"
//#endif //if !defined "WIN32"


#endif