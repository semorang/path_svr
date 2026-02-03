#include "../include/MapDef.h"


void ConvexHull(IN std::vector<SPoint> &p, OUT std::vector<SPoint> &q);
void GetBorderOfPolygon(IN const std::vector<SPoint> &p, IN const double thickness, OUT std::vector<SPoint> &q);
void GetCatmullLine(IN const std::vector<SPoint> &p, IN const double nZoomDiaMeter, OUT std::vector<SPoint> &q);
void GetSlicedLine(IN const std::vector<SPoint> &p, IN const int nMeterGap, std::vector<SPoint> &q);