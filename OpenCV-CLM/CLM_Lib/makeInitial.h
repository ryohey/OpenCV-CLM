//
//  makeInitial.h
//  OpenCV-CLM
//
//  Created by ryohey on 2015/12/17.
//
//

#ifndef makeInitial_h
#define makeInitial_h

#include "structs.h"

namespace CLM {
    void makeInitialShape(Model& Model, cv::Mat& Image, double x, double y, double w, double h, double rot, Si& Initial);
}

#endif /* makeInitial_h */
