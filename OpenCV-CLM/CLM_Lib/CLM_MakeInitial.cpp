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

#include "stdafx.h"

#include "cv.h"
#include "highgui.h"

#include "clm.h"
#include "clm_priv.h"

int CLM_MakeInitialShape(CLM_MODEL *Model, IplImage *Image, double x, double y, double w, double h, double rot, CLM_SI* Initial)
{
	CLM_PATCH_MODEL *pTemplates;
	CLM_SHAPE_MODEL *pShapes;
	CvMat *Homo = 0, *pMean = 0, *Outxy = 0;
	int NumPts = 0;
	int step;

	float *psrc, *pdst;
	float maxx = -1000, minx = 1000, maxy = -1000, miny = 1000;

	// Set up...
	pTemplates = &Model->PatchModel;
	pShapes = &Model->ShapeModel;
	NumPts = pShapes->NumPtsPerSample;

	pMean = pShapes->MeanShape;

	Homo = cvCreateMat(3, NumPts, CV_32FC1);


	// Calculate mean shape x, y;
	float meanx=0, meany = 0;
	
	psrc = pMean->data.fl;
	for (int i=0;i<NumPts;i++)
	{
		meanx += *psrc++;
		meany += *psrc++;
	}
	meanx /=NumPts;
	meany /=NumPts;

	// Copy to homogenous matrix...
	pdst = Homo->data.fl;
	psrc = pMean->data.fl;
	step = Homo->step/sizeof(float);	
	for (int i=0;i<NumPts; i++)
	{
		*pdst = *(psrc) - meanx;
		*(pdst + step) = *(psrc + 1) - meany;
		*(pdst + step*2) = 1;

		if(*pdst>maxx)
		{
			maxx = *pdst;
		}
		if(*pdst<minx)
		{
			minx = *pdst;
		}

		if(*(pdst + step) > maxy)
		{
			maxy = *(pdst + step);
		}

		if(*(pdst + step) < miny)
		{
			miny = *(pdst + step);
		}

		psrc+=2;
		pdst++;
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
	
	CvMat Rotm = cvMat(3, 3, CV_32FC1, rotmdat);
	pdst = Rotm.data.fl;
	step = Rotm.step/sizeof(float);
	
	pdst[0] = (float)cos(rot);
	pdst[1] = (float)sin(rot);
	pdst[2] = 0;

	pdst[step] = -(float)sin(rot);
	pdst[step + 1] = (float)cos(rot);
	pdst[step + 2] = 0;

	pdst[step*2] = 0;
	pdst[step*2 + 1] = 0;
	pdst[step*2 + 2] = 1;


	// Multiply to obtain rotation matrix:
	Outxy = cvCreateMat(3, NumPts, CV_32FC1);

	cvGEMM(&Rotm, Homo, scf, 0, 0, Outxy, 0);


	// Assemble return value:
	
	Initial->xy = cvCreateMat(NumPts, 2, CV_32FC1);
	Initial->AlignedXY = cvCreateMat(NumPts, 2, CV_32FC1);

	psrc = Outxy->data.fl;

	step = Outxy->step/sizeof(float);

	pdst = Initial->xy->data.fl;
	
	for (int i=0; i<NumPts;i++)
	{
		*pdst++ = *psrc + centerx;
		*pdst++ = *(psrc+step) + centery;

		psrc++;
	}

	Initial->transform[0] = (float)(scf*cos(rot));
	Initial->transform[1] = (float)(scf*sin(rot));
	Initial->transform[2] = centerx;
	Initial->transform[3] = centery;

	//Initial->center[0] = centerx;
	//Initial->center[1] = centery;
	//Initial->rotation = rot;
	//Initial->scale = scf;

	cvReleaseMat(&Homo);
	cvReleaseMat(&Outxy);
	
	
	return 0;
	
}

