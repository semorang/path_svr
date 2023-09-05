#ifndef _H__CatmullRom_H
#define _H__CatmullRom_H

#include "Vector2D.h"
#include <vector>
#include <cassert>

class CatmullRom
{
public:
	CatmullRom();
	virtual ~CatmullRom();
protected:
	void clear();
	void add_node(const CVector2D& node);
	void _on_way_point_added();
	CVector2D interpolate(double u, const CVector2D& P0, const CVector2D& P1, const CVector2D& P2, const CVector2D& P3);
	std::vector<CVector2D> _way_points;
	std::vector<CVector2D> _nodes;
	int _steps;
public:
	void begin(int steps) { clear(); _steps = steps; }
	void add_way_point(const CVector2D& point);
	void end(bool bClose);
	int node_count() const {  return static_cast<int>(_nodes.size()); }
	bool is_empty() { return _nodes.empty(); }
	const CVector2D& node(int i) const { return _nodes[i]; }
};

#endif