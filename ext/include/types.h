#pragma once

#include <vector>
#include <string>

#include "../shp/shpio.h"

using namespace std;

typedef struct _tagPoi {
	string addr;
	string name;
	SPoint coord;
} stPoi;

typedef struct _tagstDistrict {
	string name;
	SPoint center;
	vector<stPoi> pois;
	vector<SPoint> border;
} stDistrict;