#ifndef CHROMATIC
#define CHROMATIC

#include <iostream>
#include <cstdlib>

#pragma comment(lib, "cxcore210d.lib")
#pragma comment(lib, "cv210d.lib")
#include <opencv/cv.h>

class ChromaticPoint
{
	private:
		cv::Point leftPt;
		double dist;

	public:
		ChromaticPoint(cv::Point _pt = cv::Point(-1., -1.), double _dist = -1.);

		cv::Point getPt();
		void setPt(cv::Point);
		double getDist();
		void setDist(double);
};

#endif