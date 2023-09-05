#include "Vector2D.h"
#include <cmath>
#if 0
#include <sstream>
#endif

CVector2D::CVector2D()
: x(0)
, y(0)
{

}

CVector2D::CVector2D(double _x, double _y)
: x(_x)
, y(_y)
{
}

double CVector2D::length() const
{
	return sqrt(x*x + y* y);
}

bool CVector2D::operator==(const CVector2D& rhs) const
{
	if(this->x== rhs.x && this->y == rhs.y)
	{
		return true;
	}
	return false;
}

bool CVector2D::operator != (const CVector2D& rhs) const
{
	if(this->x== rhs.x && this->y == rhs.y)
	{
		return false;
	}
	return true;
}

bool CVector2D::operator<=(const CVector2D& rhs) const
{
	if(this->x <= rhs.x && this->y <= rhs.y)
	{
		return true;
	}
	return false;
}

bool CVector2D::operator>=(const CVector2D& rhs) const
{
	if(this->x >= rhs.x && this->y >= rhs.y)
	{
		return true;
	}
	return false;
}

CVector2D CVector2D::cross(const CVector2D& rhs) const
{
	double vx=this->y - rhs.y;
	double vy= - (this->x - rhs.x);
	return CVector2D(vx, vy);
}

double CVector2D::dot(const CVector2D& rhs) const
{
	return this->x * rhs.x + this->y * rhs.y;
}

double CVector2D::lengthSq() const
{
	return this->x * this->x + this->y * this->y;
}

CVector2D CVector2D::Truncate(double max_value) const
{
	CVector2D v(x, y);
	double len=this->length();
	if(len == 0)
	{
		return v;
	}

	if(len > max_value)
	{
		v.x = x * max_value / len;
		v.y = y * max_value / len;
	}

	return v;
}

CVector2D CVector2D::normalize() const
{
	double vl = this->length();
	if(vl == 0)
	{
		return CVector2D();
	}

	double vx = this->x / vl;
	double vy = this->y / vl;

	return CVector2D(vx, vy);
}

CVector2D::CVector2D(const CVector2D& rhs)
{
	this->x=rhs.x;
	this->y=rhs.y;
}

CVector2D& CVector2D::operator=(const CVector2D& rhs)
{
	this->x=rhs.x;
	this->y=rhs.y;

	return *this;
}

CVector2D& CVector2D::operator+=(const CVector2D& rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;

	return *this;
}

CVector2D& CVector2D::operator-=(const CVector2D& rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;

	return *this;
}

CVector2D& CVector2D::operator*=(double value)
{
	this->x *= value;
	this->y *= value;

	return *this;
}

CVector2D& CVector2D::operator/=(double value)
{
	this->x /= value;
	this->y /= value;

	return *this;
}

std::string CVector2D::toString() const
{
#if 1
	std::string stdOut;
	sprintf((char*)stdOut.c_str(), "[%.6f, %.6f]", this->x, this->y);
	return stdOut;
#else
	std::ostringstream oss;
	oss << "[" << x << ", " << y << ";"; //, " << z << "]";
	return oss.str();
#endif
}

CVector2D operator+(const CVector2D& v1, const CVector2D& v2)
{
	CVector2D v=v1;
	v+=v2;
	return v;
}

CVector2D operator-(const CVector2D& v1, const CVector2D& v2)
{
	CVector2D v=v1;
	v-=v2;
	return v;
}

CVector2D operator*(const CVector2D& v1, double value)
{
	CVector2D v=v1;
	v*=value;
	return v;
}

CVector2D operator/(const CVector2D& v1, double value)
{
	CVector2D v=v1;
	v/=value;
	return v;
}

CVector2D operator*(double value, const CVector2D& v1)
{
	CVector2D v=v1;
	v*=value;
	return v;
}