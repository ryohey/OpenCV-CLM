//
//  svmSearch.h
//  OpenCV-CLM
//
//  Created by ryohey on 2015/12/17.
//
//

#ifndef svmSearch_h
#define svmSearch_h

#include "structs.h"

namespace CLM {
    double* svmSearch(Si& Si, Model& pModel, cv::Mat& Image, float *QuadCoeffs, Options& Options);
}

#endif /* svmSearch_h */
