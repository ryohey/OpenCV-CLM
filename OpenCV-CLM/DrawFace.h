//
//  DrawFace.h
//  OpenCV-CLM
//
//  Created by ryohey on 2015/12/17.
//
//

#ifndef DrawFace_h
#define DrawFace_h

#include "cv.h"

void drawFaceShape(cv::Mat& image, CvMat *xy);
void drawConnected(cv::Mat& image, float *pdat, int step, int *order, int cnt);

#endif /* DrawFace_h */
