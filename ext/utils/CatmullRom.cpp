#include "CatmullRom.h"

using namespace std;

CatmullRom::CatmullRom()
: _steps(100)
{

}

CatmullRom::~CatmullRom()
{
	clear();
}

void CatmullRom::clear()
{
	_nodes.clear();
	_way_points.clear();
}


CVector2D CatmullRom::interpolate(double u, const CVector2D& P0, const CVector2D& P1, const CVector2D& P2, const CVector2D& P3)
{
	CVector2D point;
	point=u*u*u*((-1) * P0 + 3 * P1 - 3 * P2 + P3) / 2;
	point+=u*u*(2*P0 - 5 * P1+ 4 * P2 - P3) / 2;
	point+=u*((-1) * P0 + P2) / 2;
	point+=P1;

	return point;
}


void CatmullRom::_on_way_point_added()
{
	if(_way_points.size() < 4) return;

	int new_control_point_index=_way_points.size() - 1;
	int pt=new_control_point_index - 2;
	for(int i=0; i<=_steps; i++) {
		double u=(double)i / (double)_steps;
		add_node(interpolate(u, _way_points[pt-1], _way_points[pt], _way_points[pt+1], _way_points[pt+2]));
	}
}

void CatmullRom::add_way_point(const CVector2D& point)
{
	_way_points.push_back(point);
	_on_way_point_added();
}

void CatmullRom::add_node(const CVector2D& node)
{
	_nodes.push_back(node);
}

void CatmullRom::end(bool bClose)
{
	if (!bClose) return;
	if (_way_points.empty()) return;
	int nLast = _way_points.size() - 1;
	if (_way_points[0].x != _way_points[nLast].x || _way_points[0].y != _way_points[nLast].y) //is closed polygon
		add_way_point(_way_points[0]);
	add_way_point(_way_points[1]);
	add_way_point(_way_points[2]);

	std::vector<CVector2D>::iterator itr = _nodes.begin()+1;
	std::vector<CVector2D>::iterator itr2;
	std::vector<CVector2D> vtTemp;
	vtTemp.reserve(_nodes.size());
	int i=0;
	vtTemp.push_back( _nodes[0] );
	while (itr != _nodes.end()) {
		itr2 = itr - 1;
		if (itr->x != itr2->x && itr->y != itr2->y) {
			vtTemp.push_back( *itr );
		} //if
		++itr;
	} //while
	_nodes.swap(vtTemp);
}
