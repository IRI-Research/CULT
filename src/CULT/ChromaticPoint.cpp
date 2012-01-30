#include <iostream>
#include <cstdlib>
#include "ChromaticPoint.h"

using namespace cv;
using namespace std;

ChromaticPoint::ChromaticPoint(Point _pt, double _dist)
	:leftPt(_pt), dist(_dist){}

Point ChromaticPoint::getPt()
{
	return leftPt;
}
void ChromaticPoint::setPt(Point pt)
{
	leftPt = pt;
}
double ChromaticPoint::getDist()
{
	return dist;
}
void ChromaticPoint::setDist(double _dist)
{
	dist = _dist;
}