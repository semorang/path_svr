//
//  MMPoint.hpp
//  LocationEngine
//
//  Created by LTDT on 2016. 5. 17..
//  Copyright © 2016년 thinkware. All rights reserved.
//

#ifndef MMPoint_hpp
#define MMPoint_hpp

static const double sec_coord = 360000.0;
static const double mPI = 3.1415926535;
static const double hPI = mPI/2;
static const double R = 6371000.;

#include <stdio.h>
#include <math.h>
#include <deque>
//#include "WRP_MMDefine.h"

class CoordSec {
public:
	int _lon;
	int _lat;
	CoordSec(){_lon=0;_lat=0;}
	CoordSec(int lon,int lat){_lon=lon;_lat=lat;}
};

static short rad2azm(double a){
    return (short)fmod(( (a*180.)/mPI)+270.,360.0);
}
static double azm2rad(short a){
    return fmod(a+270.,360.)*mPI/180.;
}
static double rad2deg(double a){
    return fmod((a*180.)/mPI+360.,360.);
}

static double brng2rad(double a){
	return ((mPI*3/2)-a);
}
const double toRad = 0.0174533;

static double rad_angledist(double a, double b)
{
	a= fmod(a+hPI,mPI);
	b= fmod(b+hPI,mPI);
	return (a>b?(a-b):(b-a));
}

static double rad_anglediff(double a, double b)
{
	double diff = b-a;
	while(diff>hPI)
		diff -= mPI;
	while(diff<-hPI)
		diff += mPI;
	return diff;

}
/*
static double deg2rad(double a){
    return a*mPI/180.0;
}
*/

template<class T>
class MMPoint
{
public:
    T lon;
    T lat;
    MMPoint<T> (){lon=lat = 0.0f;}
    MMPoint<T> (T Lon,T Lat){lon =Lon;lat = Lat;}
    MMPoint<T> (int Lon, int Lat ) {lon = Lon/sec_coord; lat = Lat/sec_coord; }
    MMPoint<T> operator + (const MMPoint<T> p) {return MMPoint<T>(lon+p.lon,lat+p.lat);}
    MMPoint<T> operator - (const MMPoint<T> p) {return MMPoint<T>(lon-p.lon,lat-p.lat);}
    MMPoint<T> operator - () {return MMPoint<T>(-lon,-lat);}
    MMPoint<T> operator * (const MMPoint<T> p) {return MMPoint<T>(lon*p.lon,lat*p.lat);}
    MMPoint<T> operator * (T a) {return MMPoint<T>(lon*a,lat*a);}
    MMPoint<T> operator / (const MMPoint<T> p) {return MMPoint<T>(lon/p.lon,lat/p.lat);}
    MMPoint<T> operator / (T a) {return MMPoint<T>(lon/a,lat/a);}
    MMPoint<T>& operator += (const MMPoint<T> p) {lon+=p.lon;lat+=p.lat;return*this;}
    MMPoint<T>& operator -= (const MMPoint<T> p) {lon-=p.lon;lat-=p.lat;return*this;}
    MMPoint<T>& operator *= (const MMPoint<T> p) {lon*=p.lon;lat*=p.lat;return*this;}
    MMPoint<T>& operator /= (const MMPoint<T> p) {lon/=p.lon;lat/=p.lat;return*this;}
    MMPoint<T>& operator *= (T a) {lon*=a;lat*=a;return*this;}
    MMPoint<T>& operator /= (T a) {lon/=a;lat/=a;return*this;}
    
    bool operator == (const MMPoint<T> p){return ( 0.0000001>fabs(lon-p.lon) && 0.000001 >fabs(lat-p.lat)) ;}
		bool operator != (const MMPoint<T> p){return !((*this)==p);}
    //bool operator != (const MMPoint<T> p){return(lon!=p.lon||lat!=p.lat);}
    MMPoint operator () (T Lon,T Lat){lon=Lon;lat=Lat;return *this;}
		MMPoint operator () (int Lon, int Lat ){lon = Lon/sec_coord; lat = Lat/sec_coord;return *this;}
    
    //radian
	double bearing ( MMPoint<T> p){
		static double tiny_dist = 0.000000000001;
        MMPoint<T> d = p-(*this);
        double dlon = d.lon*toRad;
        double dlat = d.lat*toRad;
        if(fabs(dlon)< tiny_dist && fabs(dlat) < tiny_dist )
            return 0.0;
        double y = sin(dlon)*cos((double)p.lat*toRad);
        double x = cos(lat*toRad) * sin (p.lat*toRad) - sin(lat*toRad)*cos(p.lat*toRad)*cos(dlon);
        return atan2(y,x);
    }

    //degree
    double azimuth (MMPoint<T> p){
        return rad2deg(bearing(p));
    }
    
    inline double dist (MMPoint<T> p){ // great circle distance
				if (p==*this)
					return 0;
        MMPoint<T> d = p - *this;
#if 0
        
        double a = sin(toRad*d.lat/2) * sin(toRad*d.lat/2) + sin(toRad*d.lon/2)*sin(toRad*d.lon/2)*cos(toRad*lat)*cos(toRad*p.lat);
        double c = 2*atan2(sqrt(a),sqrt(1-a));
        return R*c; 
#else
        double dlon = toRad*d.lon;
//        double dlat = toRad*d.lat;
        double lat1 = toRad*lat;
        double lat2 = toRad*p.lat;
        double a = cos(lat2)*sin(dlon);
        double b = cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(dlon);
        double c = sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(dlon);
        
        double t = sqrt(a*a+b*b)/c;

        return R*t;
#endif
    }
//    MMPoint dist_e(double meter){
//        double c= meter/R;
//        double _a = tan(c/2)/ (1+tan(c/2));
//        double dlon  = acos( cos(lat)/sqrt(_a) );
//        return T(dlon*2);
//    }
//    MMPoint dist_n(double meter){
//        double c = meter/R;
//        double _a = tan(c/2)/ (1+tan(c/2));
//        double dlat =sqrt (asin(_a) );
//        return dlat*2;
//    }
//    inline void moveLat(T meter_n){ lat+=dist_n(meter_n); };
//	inline void moveLon(T meter_e){ lon+=dist_e(meter_e); };
//	void move(T meter_e, T meter_n){ moveLon( meter_e); moveLat(meter_n); };
//	void move(T heading, T meter)
//	{
//		double radian = mPI/2 - heading*toRad;
//		double t = sin(radian/2);
//		double a = meter *( 1 - 2*t*t);
//		double h = a * tan( radian );
//		move (a , h);		
//	};

    MMPoint<T> left_bottom (MMPoint<T> other){
        return MMPoint<T>(fmin(lon,other.lon),fmin(lat,other.lat));
    }
    MMPoint<T> right_top (MMPoint<T> other){
        return MMPoint<T>(fmax(lon,other.lon),fmax(lat,other.lat));
    }
    
    MMPoint<T> dest_ (T dist, short azimuth){
        return dest(dist, azimuth*toRad);
    }
    
    MMPoint<T> dest (T dist, T bearing){
        double gamma = dist / R;
        double theta = bearing;
        
        double dlat = toRad*this->lat;
        double dlon = toRad*this->lon;
        
        double sin_dlat = sin(dlat); double cos_dlat = cos(dlat);
        double sin_gamma = sin(gamma); double cos_gamma = cos(gamma);
        double sin_theta = sin(theta); double cos_theta = cos(theta);
        
        double sin_dlat2 = sin_dlat*cos_gamma + cos_dlat*sin_gamma*cos_theta;
        double dlat2 = asin(sin_dlat2);
        double y = sin_theta*sin_gamma*cos_dlat;
        double x = cos_gamma - sin_dlat*sin_dlat2;
        
        double dlon2 = dlon + atan2(y,x);
        
        return MMPoint<T>(dlon2/toRad, fmod(dlat2/toRad + 540,360)-180 );
    }

	double Lon(){return lon*sec_coord;}
	double Lat(){return lat*sec_coord;}

	int lonsec(){return int(lon*sec_coord);}
	int latsec(){return int(lat*sec_coord);}

	//T lonsec(){return T(lon*sec_coord);}
	//T latsec(){return T(lat*sec_coord);}

    void coord(T &Lon, T &Lat){Lon = lon;Lat = lat;}
    void coord(int &Lon, int &Lat){Lon=lonsec();Lat = latsec(); }
		CoordSec coord(){return CoordSec(lonsec(),latsec());}
};


typedef MMPoint<double> PT;
typedef std::deque<PT> LOCLIST;
typedef LOCLIST::iterator LOC_Iter;
typedef LOCLIST::reverse_iterator LOC_RIter;

class MMRect {
    PT _bl;
    PT _tr;
    MMRect (PT bottomleft,PT topright );
    
    bool isInside(PT point){
        return (point.lon>_bl.lon && point.lat>_bl.lat && point.lon<_tr.lon && point.lat<_tr.lat);
    }
};



//////////////////////////////////////////////////////////////////////////
//
// Templete Class: Point2T
//
//////////////////////////////////////////////////////////////////////////

template<class T>
class Point2T {
public:
	T X, Y;

public:
	Point2T() { X = Y = (T)0; }
	Point2T(T x, T y) { X = x; Y = y; }

public:
	Point2T  operator +  (const Point2T& p) { return Point2T<T>(X + p.X, Y + p.Y); }
	Point2T  operator -  () { return Point2T<T>(-X, -Y); }
	Point2T  operator -  (const Point2T& p) { return Point2T<T>(X - p.X, Y - p.Y); }
	Point2T  operator *  (const Point2T& p) { return Point2T<T>(X * p.X, Y * p.Y); }
	Point2T  operator *  (T a) { return Point2T<T>(X * a, Y * a); }
	Point2T  operator /  (const Point2T& p) { return Point2T<T>(X / p.X, Y / p.Y); }
	Point2T  operator /  (T a) { return Point2T<T>(X / a, Y / a); }
	Point2T& operator += (const Point2T& p) { X += p.X; Y += p.Y; return *this; }
	Point2T& operator -= (const Point2T& p) { X -= p.X; Y -= p.Y; return *this; }
	Point2T& operator *= (const Point2T& p) { X *= p.X; Y *= p.Y; return *this; }
	Point2T& operator *= (T a) { X *= a; Y *= a; return *this; }
	Point2T& operator /= (const Point2T& p) { X /= p.X; Y /= p.Y; return *this; }
	Point2T& operator /= (T a) { X /= a; Y /= a; return *this; }
	int      operator == (const Point2T& p) { return (X == p.X && Y == p.Y) ? 1 : 0; }
	int      operator != (const Point2T& p) { return (X != p.X || Y != p.Y) ? 1 : 0; }
	T&       operator [] (int i) const { return ((T*)this)[i]; }
	void     operator () (T x, T y) { X = x; Y = y; }

public:
	void Clear() { X = Y = (T)0; }
	T    Length() { return sqrt(LengthSquared()); }
	T    LengthSquared() { return X * X + Y * Y; }
	void Set(const T& x, const T& y) { X = x; Y = y; }
	void Unit() { X = Y = (T)1; }
};

//typedef Point2T<double> DPoint2T;
//
//template<class T> T          Distance(Point2T<T> pa, const Point2T<T>& pb);
//template<class T> T          DotProduct(const Point2T<T>& va, const Point2T<T>& vb);
//template<class T> void       GetLineEquation(const Point2T<T>& p0, const Point2T<T>& p1, T& a, T& b, T& c);
//template<class T> int        Intersection(const Point2T<T>& pa0, const Point2T<T>& pa1, const Point2T<T>& pb0, const Point2T<T>& pb1, Point2T<T>& v);
//template<class T> int        LocationOfPointOnVector(const Point2T<T>& p1, const Point2T<T>& p2, const Point2T<T>& p3);
//template<class T> T          Mag(const Point2T<T>& v);
//template<class T> Point2T<T> MidWeight(const Point2T<T>& va, Point2T<T> vb, T weight = 0.5);
//template<class T> Point2T<T> Norm(const Point2T<T>& v);
//template<class T> Point2T<T> Perpendicular(const Point2T<T>& va, Point2T<T> vb, Point2T<T> vp);
//template<class T> Point2T<T> PerpendicularLeft(const Point2T<T>& v);
//template<class T> Point2T<T> PerpendicularRight(const Point2T<T>& v);
//template<class T> T          SquaredMag(const Point2T<T>& v);
//float ReverseSQRTFast(float x);    // Reverse Squre Root(1/sqrt(x)) - Used Quake3


//////////////////////////////////////////////////////////////////////////
//
// Templete Class Function: Point2T
//
//////////////////////////////////////////////////////////////////////////

template<class T>
T Distance(Point2T<T> pa, const Point2T<T>& pb)
{
	return Mag(pa - pb);
}

template<class T>
T DotProduct(const Point2T<T>& va, const Point2T<T>& vb)
{
	return (T)((double)va.X * vb.X + (double)va.Y * vb.Y);
}

template<class T>
void GetLineEquation(const Point2T<T>& p0, const Point2T<T>& p1, T& a, T& b, T& c)
{
	a = p1.Y - p0.Y;
	b = p0.X - p1.X;
	c = a * p0.X + b * p0.Y;
}

template<class T>
int Intersection(const Point2T<T>& pa0, const Point2T<T>& pa1, const Point2T<T>& pb0, const Point2T<T>& pb1, Point2T<T>& v)
{
	T a1, b1, c1;
	T a2, b2, c2;

	double d_dist_pa0_pb0 = Distance(pa0, pb0);

	if (d_dist_pa0_pb0 < 2.0) {
		v = pa0;
		return 1;
	}

	GetLineEquation(pa0, pa1, a1, b1, c1);
	GetLineEquation(pb0, pb1, a2, b2, c2);
	T under = a1*b2 - a2*b1;
	if (under == 0) return 0;
	v.X = (b2*c1 - b1*c2) / under;
	v.Y = (a1*c2 - a2*c1) / under;
	return 1;
}

template<class T>
int LocationOfPointOnVector(const Point2T<T>& p1, const Point2T<T>& p2, const Point2T<T>& p3)
{
	T dir = (p3.X - p1.X) * (p2.Y - p1.Y) - (p2.X - p1.X) * (p3.Y - p1.Y);
	return (int)dir;
}

template<class T>
T Mag(const Point2T<T>& v)
{
	return sqrt(SquaredMag(v));
}

template<class T>
Point2T<T> MidWeight(const Point2T<T>& va, Point2T<T> vb, T weight)
{
	return (vb - va) * weight + va;
}

template<class T>
#if defined(FAST_NORMALIZE)
Point2T<T> Norm(const Point2T<T>& v)
{
	return Point2T<T>(v) * (T)ReverseSQRTFast((float)SquaredMag(v));
}
#else
Point2T<T> Norm(const Point2T<T>& v)
{
	T m = Mag(v);
	if (m == 0 || m == 1) return v;
	Point2T<T> vd;
	vd.X = (T)((double)v.X / m);
	vd.Y = (T)((double)v.Y / m);
	return vd;
}
#endif // FAST_NORMALIZE

template<class T>
Point2T<T> Perpendicular(const Point2T<T>& va, Point2T<T> vb, Point2T<T> vp)
{
	Point2T<T> vab = vb - va;
	T sm = SquaredMag(vab);
	if (sm == 0.0) return vp;  // On the line
	return (vab * (DotProduct(vp - va, vab) / sm)) + va;
}

template<class T>
Point2T<T> PerpendicularLeft(const Point2T<T>& v)
{
	Point2T<T> vd;
	vd.X = -v.Y;
	vd.Y = v.X;
	return vd;
}

template<class T>
Point2T<T> PerpendicularRight(const Point2T<T>& v)
{
	Point2T<T> vd;
	vd.X = v.Y;
	vd.Y = -v.X;
	return vd;
}

template<class T>
T SquaredMag(const Point2T<T>& v)
{
	return (T)((double)v.X * v.X + (double)v.Y * v.Y);
}

//float ReverseSQRTFast(float x)
//{
//	union {
//		float f;
//		int i;
//	} tmp;
//	tmp.f = x;
//	tmp.i = 0x5f3759df - (tmp.i >> 1);
//	float y = tmp.f;
//	return y * (1.5f - 0.5f * x * y * y);
//}

#endif /* MMPoint_hpp */
