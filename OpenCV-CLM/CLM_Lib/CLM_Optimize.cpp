////////////////////////////////////////////////////////////////
// Implementation of Constrained Local Model (CLM) optimization
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

#include "math.h"

#include "cv.h"
#include "highgui.h"

#include "clm.h"
#include "clm_priv.h"

extern FILE * fpout;


#include "QuadProg_136.hh"


static void DumpMat(CvMat * r);
static void DumpVec(float *vec, int len);
static void DumpCI(double *ci, int len);

extern DWORD CountsPerSec;
static LARGE_INTEGER L1;
static LARGE_INTEGER L2;

float CLM_Optimize(CLM_MODEL* Model, CLM_SI* Si, float *coeffs, CLM_OPTIONS * Options)
{
	// Step 0: Prepare...
	int NumX = Model->PatchModel.NumPatches*2;
	int i;  

	//////////////////////////////////////////////////
	// Step 1: Prepare -2H and -F in R:(0.5*x_t*2H*x+F*x)
	//////////////////////////////////////////////////
	float *p_2Hdat = Model->ShapeModel.p_2HMat->data.fl;
	float *p_Fdat = Model->ShapeModel.p_FMat->data.fl;

	for(i=0;i<NumX/2;i++)
	{
		//DumpVec(coeffs, 8);
		p_2Hdat[i*2] = -coeffs[3]*2;
		p_2Hdat[i*2+1] = -coeffs[1];
		p_2Hdat+=NumX;
		p_2Hdat[i*2] = -coeffs[1];
		p_2Hdat[i*2+1] = -coeffs[0]*2;
		*p_Fdat++=-coeffs[4];
		*p_Fdat++=-coeffs[2];

		p_2Hdat+=NumX;
		coeffs+=8;
	}


	//////////////////////////////////////////////////////////////////
	// Step 2: Prepare norm2((x+basexy) - Evec*(Evec'*(x+basexy))));
	//////////////////////////////////////////////////////////////////
	// 2.1, basexy:
	float newxy[256];
	float w0 = (float)(Options->SearchRegion[0] + 1)/2;

	CvMat *pMeanShape = Model->ShapeModel.MeanShape;
	float *pMeanxy = pMeanShape->data.fl;
	float *pxy0 = Si->xy->data.fl;
	
	float *alignedxy = Si->AlignedXY->data.fl;
	float *tform = Si->transform;
	
	float basexy[256];

	for(i=0;i<NumX;i++)
	{
		basexy[i] = alignedxy[i]-pMeanxy[i]-w0;
	}

	CvMat BasexyMat = cvMat(NumX, 1, CV_32FC1, basexy);


	// 2.2, norm2((x+basexy)-Evec*Evec'*(x+basexy)) = x'*W*x + 2*basexy'*W*x + basexy'*W*basexy;
	//			where W = (I-E*E')'*(I-E*E')
	cvGEMM(Model->ShapeModel.p2alphaWMat, &BasexyMat, 1, 0, 0, Model->ShapeModel.p2alphaWtBMat, CV_GEMM_A_T);


	///////////////////////////
	// Step 3: Prepare G, g0:
	///////////////////////////
	float *psrc1 = Model->ShapeModel.p_2HMat->data.fl;
	float *psrc2 = Model->ShapeModel.p2alphaWMat->data.fl;
	
	static double G[200][MATRIX_DIM_440];
	
	// Here removed memset, to improve performance.
	// I am assuming G was initialized to all 0s',
	// And at every loop, I only modify the elements
	// I need to.
	//memset(G, 0, sizeof(G));
	
	for(i=0;i<NumX;i++)
	{
		for(int j=0;j<NumX;j++)
		{
			G[i][j] = (*psrc1++) + (*psrc2++);
		}
	}


	//DumpMat(p_2HMat);
	//DumpMat(p2alphaWMat);



	psrc1 = Model->ShapeModel.p_FMat->data.fl;
	psrc2 = Model->ShapeModel.p2alphaWtBMat->data.fl;

	double g0[200];

	for(i=0;i<NumX;i++)
		g0[i] = (*psrc1++) + (*psrc2++);

	////////////////////////////////////
	// Step 4: Prepare constraint matrices:
	//		-x + ub > 0;
	//		 x - lb > 0;
	//		-B'x - B'*basexy + sub > 0;
	//		B'x + B'*basexy + sub > 0;
	////////////////////////////////////
	// 4.1, Calculate BMat'*basexy
	cvGEMM(Model->ShapeModel.pBMat, &BasexyMat, 1, 0, 0, Model->ShapeModel.pBBaseMat, CV_GEMM_A_T);
	
	// 4.2, Create CI:
	//
	//	 Have to use static here because the array is too large...
	//
	static double CI[200][MATRIX_DIM_440];

	// Again, here removed memset, to improve performance.
	// I am assuming CI was initialized to all 0s',
	// And at every loop, I only modify the elements
	// I need to.
	//memset(CI, 0, sizeof(CI));

	float *pBMatDat = Model->ShapeModel.pBMat->data.fl;
	
	for(i=0;i<NumX;i++)
	{
		// Lower and upper bound.
		CI[i][i] = -1;
		CI[i][NumX+i] = 1;

		// Shape constraint: -3<bj/sqrt(lambdaj)<3
		for(int k=0;k<Model->ShapeModel.NumEvalues;k++)
		{
			CI[i][NumX*2+k] = -(*pBMatDat);
			CI[i][NumX*2+Model->ShapeModel.NumEvalues+k] = *pBMatDat++;
		}
	
	}

	//DumpCI(CI[0], NumX*2+Model->ShapeModel.NumEvalues*2);

	// 4.3, Create ci0:
	double ci0[MATRIX_DIM_440];

	for(i=0;i<NumX;i++)
	{
		// Lower and upper bound.
		ci0[i] = 2*w0;
		ci0[NumX+i] = 0;
	}

	float *pBBaseMatDat = Model->ShapeModel.pBBaseMat->data.fl;
	
	for(i=0;i<Model->ShapeModel.NumEvalues;i++)
	{
		// Shape constraint:
		ci0[2*NumX+i] = 3 - *pBBaseMatDat;
		ci0[2*NumX+Model->ShapeModel.NumEvalues+i] = 3 + *pBBaseMatDat++;
	}
	
	//////////////////////////////
	// Step 5, Do quadprog:
	//////////////////////////////
	double xout[200];
	double err = solve_quadprog_136(G, g0, NumX, 0, 0, 0, CI, ci0, (2*NumX + 2*Model->ShapeModel.NumEvalues), xout);
	
	//////////////////////////
	// Step 6, Reconstruct x:
	//////////////////////////
	for(i=0;i<NumX;i++)
	{
		newxy[i] =(float) (xout[i] - w0 + alignedxy[i]);
	}
	
	///////////////////////////////////////////////
	// Step 7, align back to image coordinate.
	///////////////////////////////////////////////
	CLM_align_data_inverse(newxy, tform, Model->ShapeModel.NumPtsPerSample, Si->xy->data.fl);


	return 0.0f;
}





//////////////////////////////
// Below are debug functions.
//////////////////////////////
extern FILE *fopout;
static void DumpMat(CvMat * r)
{
	float *pdat = r->data.fl;

	for(int i=0;i<r->rows;i++)
	{
		for(int j=0;j<r->cols;j++)
			fprintf(fopout, "%f ", *pdat++);
		
		fprintf(fopout, "\n");
	}

	fflush(fopout);

}

extern FILE *fcoeffs;
static void DumpVec(float *vec, int len)
{
	for(int i=0;i<len;i++)
		fprintf(fcoeffs, "%f ", *vec++);

	fprintf(fcoeffs, "\n");
	fflush(fcoeffs);
}

extern FILE *fci;
static void DumpCI(double *ci, int len)
{
	for(int i=0;i<136;i++)
	{
		for(int j=0;j<len;j++)
			fprintf(fci, "%f ", ci[j]);

		fprintf(fci, "\n");

		ci+=MATRIX_DIM_440;
	}

	fflush(fci);

}

