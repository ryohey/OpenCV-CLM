//
//  search.h
//  OpenCV-CLM
//
//  Created by ryohey on 2015/12/17.
//
//

#ifndef search_h
#define search_h

#include "structs.h"

namespace CLM {
    void search(Model& Model, cv::Mat& Image, Si& Initial, Si& Final, Options& Options);
}

#endif /* search_h */
