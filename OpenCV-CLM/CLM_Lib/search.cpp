/////////////////////////////////////////////////////////
// Implementation of CLM search algorithm.
// 
/////////////////////////////////////////////////////////
// 
// Written by: 
//		Xiaoguang Yan
// Email:
//		xiaoguang.yan@gmail.com
// Homepage:
//		http://sites.google.com/site/xgyanhome/
// Version:
//		0.9
// Date:
//		June, 2011.
////////////////////////////////////////////////////////

#include "cv.h"
#include "search.h"
#include "svmSearch.h"
#include "optimize.h"
#include "utils.h"

#define NUM_ITER		10		// Default number of iterations.

using namespace CLM;

namespace CLM {
    void dumpResponse(CvMat * r);
}

void CLM::search(Model& Model, cv::Mat& Image, Si& Initial, Si& Final, Options& Options)
{
	// Prepare:
	auto Inter = Initial;
	Inter.xy = cvCloneMat(Initial.xy);
	Inter.AlignedXY = cvCloneMat(Initial.AlignedXY);

	int nIterations = Options.NumInterations;
	nIterations = nIterations?nIterations:NUM_ITER;
	
	// Do search:
	float coeffs[8*100];
	
	for(int iter = 0; iter < nIterations; iter++)
	{
		svmSearch(Inter, Model, Image, coeffs, Options);

		optimize(Model, Inter, coeffs, Options);
	}
	//dumpSi(&Inter);
	
	// Assemble return value:
	copySi(Final, Inter);

	// Clean up: ... should make Si a class...
	cvReleaseMat(&Inter.xy);
	cvReleaseMat(&Inter.AlignedXY);
}

