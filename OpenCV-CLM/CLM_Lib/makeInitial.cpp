////////////////////////////////////////////////////////////////
// Create initial estimation for Constrained Local Model (CLM)
//
////////////////////////////////////////////////////////////////
// Written by:
// 		Xiaoguang Yan
// Date:
//		Dec. 1st, 2010
//
////////////////////////////////////////////////////////////

#include "cv.h"
#include "makeInitial.h"

using namespace CLM;

void CLM::makeInitialShape(Model& Model, cv::Mat& Image, double x, double y, double w, double h, double rot, Si& Initial)
{
	float maxx = -1000, minx = 1000, maxy = -1000, miny = 1000;

	// Set up...
	auto& pShapes = Model.ShapeModel;
	int NumPts = pShapes.NumPtsPerSample;
	auto& pMean = pShapes.MeanShape;

    cv::Mat Homo(3, NumPts, CV_32FC1);
    

	// Calculate mean shape x, y;
	float meanx=0, meany = 0;
	
	auto psrc = pMean->data.fl;
	for (int i=0;i<NumPts;i++)
	{
		meanx += *psrc++;
		meany += *psrc++;
	}
	meanx /=NumPts;
	meany /=NumPts;

	// Copy to homogenous matrix...
	psrc = pMean->data.fl;
	int step = (int)Homo.step/sizeof(float);
	for (int i=0;i<NumPts; i++)
	{
		Homo.at<float>(i) = *(psrc) - meanx;
		Homo.at<float>(i + step) = *(psrc + 1) - meany;
		Homo.at<float>(i + step*2) = 1;

		if(Homo.at<float>(i)>maxx)
		{
			maxx = Homo.at<float>(i);
		}
		if(Homo.at<float>(i)<minx)
		{
			minx = Homo.at<float>(i);
		}

		if(Homo.at<float>(i + step) > maxy)
		{
			maxy = Homo.at<float>(i + step);
		}

		if(Homo.at<float>(i + step) < miny)
		{
			miny = Homo.at<float>(i + step);
		}

		psrc+=2;
	}

	// Calculate scale factor:
	float mwidth = (maxx - minx);
	float mheight = (maxy - miny);

	float scf = (float)w/mwidth;

	if(mheight*scf > h)
	{
		scf = (float)h/mheight;
	}
	
	// Rotation matrix:
	float centerx = (float)(x+w/2);
	float centery = (float)(y+h/2);

	float rotmdat[9];
	
    cv::Mat Rotm(3, 3, CV_32FC1, rotmdat);
	step = (int)Rotm.step/sizeof(float);
        
    Rotm.at<float>(0) = (float)cos(rot);
    Rotm.at<float>(1) = (float)sin(rot);
    Rotm.at<float>(2) = 0;
    
	Rotm.at<float>(step) = -(float)sin(rot);
	Rotm.at<float>(step + 1) = (float)cos(rot);
	Rotm.at<float>(step + 2) = 0;

	Rotm.at<float>(step*2) = 0;
	Rotm.at<float>(step*2 + 1) = 0;
	Rotm.at<float>(step*2 + 2) = 1;


	// Multiply to obtain rotation matrix:
    cv::Mat Outxy(3, NumPts, CV_32FC1);

    cv::gemm(Rotm, Homo, scf, 0, 0, Outxy, 0);

	// Assemble return value:
	
	Initial.xy = cvCreateMat(NumPts, 2, CV_32FC1);
	Initial.AlignedXY = cvCreateMat(NumPts, 2, CV_32FC1);

	step = (int)Outxy.step/sizeof(float);

	auto pdst = Initial.xy->data.fl;
	
	for (int i=0; i<NumPts;i++)
	{
		*pdst++ = Outxy.at<float>(i) + centerx;
		*pdst++ = Outxy.at<float>(i + step) + centery;
	}

	Initial.transform[0] = (float)(scf*cos(rot));
	Initial.transform[1] = (float)(scf*sin(rot));
	Initial.transform[2] = centerx;
	Initial.transform[3] = centery;
}
