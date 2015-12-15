////////////////////////////////////////////////////////////
// Private header file for Constrained Local Model (CLM)
// OpenCV implementation.
////////////////////////////////////////////////////////////
// Written by:
// 		Xiaoguang Yan of Changchun University
// Date:
//		Dec. 1st, 2010
//
////////////////////////////////////////////////////////////

#ifndef _CLM_PRIV_H_
#define _CLM_PRIV_H_

#include "cv.h"
#include "highgui.h"

#include "clm.h"
////////////////////////
// Data structures
////////////////////////


////////////////
// Functions:
////////////////
double* CLM_SvmSearch(CLM_SI& Si, CLM_MODEL& pModel, cv::Mat& Image, float *QuadCoeffs, CLM_OPTIONS& Options);
float CLM_Optimize(CLM_MODEL& Model, CLM_SI& Si, float *coeffs, CLM_OPTIONS& Options);


int CLM_align_data(float *pdat, float *pbase, int numPts, float *pout, float *tform);
int CLM_align_data_inverse(float *pdat, float *tform, int numPts, float *pout);






#endif _CLM_PRIV_H_

