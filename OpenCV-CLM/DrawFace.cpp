////////////////////////////////////////////////////////////////
// Draw talking face shape.
//
////////////////////////////////////////////////////////////////
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


#include "stdafx.h"

#include "cv.h"
#include "highgui.h"

#include "CLM.h"

static int orFace[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 24, 23, 22, 21, 0};
static int orEbrowL[] = {24, 23, 22, 21, 26, 25, 24};
static int orEbrowR[] = {18, 19, 20, 15, 16, 17, 18};
static int orEyeL[] = {27, 30, 29, 31, 27, 31, 29, 28, 27};
static int orEyeR[] = {34, 35, 32, 36, 34, 36, 32, 33, 34};
static int orNose[] = {37, 38, 39, 40, 46, 41, 47, 42, 43, 44, 45, 37};
static int orMouth[] = {48, 59, 58, 57, 56, 55, 54, 53, 52, 50, 49, 48, 60, 61, 62, 63, 64, 65, 48};


void drawConnected(cv::Mat& image, float *pdat, int step, int *order, int cnt);

void drawFaceShape(cv::Mat& image, CvMat *xy)
{
	
	// Draw face:
	auto pdat =xy->data.fl;
	int step = xy->step/sizeof(float);
	
	drawConnected(image, pdat, step, orFace, sizeof(orFace)/sizeof(int));
	drawConnected(image, pdat, step, orEbrowL, sizeof(orEbrowL)/sizeof(int));
	drawConnected(image, pdat, step, orEbrowR, sizeof(orEbrowR)/sizeof(int));
	drawConnected(image, pdat, step, orEyeL, sizeof(orEyeL)/sizeof(int));
	drawConnected(image, pdat, step, orEyeR, sizeof(orEyeR)/sizeof(int));
	drawConnected(image, pdat, step, orNose, sizeof(orNose)/sizeof(int));
	drawConnected(image, pdat, step, orMouth, sizeof(orMouth)/sizeof(int));
}


void drawConnected(cv::Mat& image, float *pdat, int step, int *order, int cnt)
{
	int i;

	double x0, x1, y0, y1;
	
	for(i=0;i<cnt-1; i++)
	{
		x0 = pdat[order[i]*2];
		y0 = pdat[order[i]*2 + 1];

		x1 = pdat[order[i+1]*2];
		y1 = pdat[order[i+1]*2+ 1];

        cv::line(image, cvPoint((int)x0, (int)y0), cvPoint((int)x1, (int)y1), CV_RGB(255, 0, 0), 1, 8);
        cv::circle(image, cvPoint((int)x0, (int)y0), 1, CV_RGB(0, 255, 0), 2, 8, 0);
        cv::circle(image, cvPoint((int)x1, (int)y1), 1, CV_RGB(0, 255, 0), 2, 8, 0);
	}
}

