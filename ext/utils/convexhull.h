#include "../include/MapDef.h"


void ConvexHull(IN vector<SPoint> &p, OUT vector<SPoint> &q);
void GetBorderOfPolygon(IN const vector<SPoint> &p, IN const double thickness, OUT vector<SPoint> &q);
void GetCatmullLine(IN const vector<SPoint> &p, IN const double nZoomDiaMeter, OUT vector<SPoint> &q);
void GetSlicedLine(IN const vector<SPoint> &p, IN const int nMeterGap, vector<SPoint> &q);