//
//  utils.h
//  OpenCV-CLM
//
//  Created by ryohey on 2015/12/17.
//
//

#ifndef utils_h
#define utils_h

#include "structs.h"

namespace CLM {
    int alignData(float *pdat, float *pbase, int numPts, float *pout, float *tform);
    int alignDataInverse(float *pdat, float *tform, int numPts, float *pout);
    int copySi(Si& dst, Si& src);
    int dumpSi(Si& si);
    int startTimer(int id);
    long stopTimer(int id);
}

#endif /* utils_h */
