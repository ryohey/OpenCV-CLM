//
//  optimize.h
//  OpenCV-CLM
//
//  Created by ryohey on 2015/12/17.
//
//

#ifndef optimize_h
#define optimize_h

#include "structs.h"

namespace CLM {
    float optimize(Model& Model, Si& Si, float *coeffs, Options& Options);
}

#endif /* optimize_h */
