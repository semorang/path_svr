#ifndef _H__VECTOR_H
#define _H__VECTOR_H

#include <string>

class CVector2D
{
public:
	double x;
	double y;
public:
	CVector2D();
	CVector2D(double _x, double _y);
	CVector2D(const CVector2D& rhs);
	CVector2D& operator=(const CVector2D& rhs);

public:
	CVector2D cross(const CVector2D& rhs) const;
	CVector2D normalize() const;
	double dot(const CVector2D& rhs) const;
	double lengthSq() const;

public:
	CVector2D Truncate(double max_value) const;

public:
	CVector2D& operator-=(const CVector2D& rhs);
	CVector2D& operator+=(const CVector2D& rhs);
	CVector2D& operator*=(double value);
	CVector2D& operator/=(double value);

public:
	bool operator <=(const CVector2D& rhs) const;
	bool operator >=(const CVector2D& rhs) const;
	bool operator ==(const CVector2D& rhs) const;
	bool operator !=(const CVector2D& rhs) const;

public:
	void reset() { x=0; y=0; }

public:
	std::string toString() const;

public:
	double length() const;
};

CVector2D operator+(const CVector2D& v1, const CVector2D& v2);
CVector2D operator-(const CVector2D& v1, const CVector2D& v2);
CVector2D operator*(const CVector2D& v1, double value);
CVector2D operator*(double value, const CVector2D& v1);
CVector2D operator/(const CVector2D& v1, double value);

#endif