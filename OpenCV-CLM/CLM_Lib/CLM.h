/////////////////////////////////////////////////////////
// Public header file for Constrained Local Model (CLM)
// OpenCV implementation.
/////////////////////////////////////////////////////////
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

#ifndef _CLM_OPENCV_H_
#define _CLM_OPENCV_H_

#include "cv.h"
#include "highgui.h"


#define MAX_NUM_THREADS 		8
#define CURRENT_NUM_THREADS	    2

////////////////////////
// Data structures
////////////////////////
typedef struct _clm_shape_model_
{
	int NumPtsPerSample;
	int NumEvalues;
	
	CvMat* Evectors;
	CvMat* Evalues;

	CvMat* MeanShape;

	// Partial calculation results:
	CvMat* pI_EEtMat;
	CvMat* pBMat;

	// Temporary working data:
	CvMat *p_2HMat;
	CvMat *p_FMat;
	CvMat *p2alphaWMat;
	CvMat *p2alphaWtBMat;
	CvMat *pBBaseMat;
	
	// Reserved.
	int 	NumTrainSamples;
} CLM_SHAPE_MODEL;

typedef struct _clm_patch_model_
{
	int NumPatches;
	int PatchSize[2];
	
	//CvMat* weights;
	CvMat* WeightMats[100];
} CLM_PATCH_MODEL;

typedef struct _clm_svm_workspace_
{
	float *pAdat;		// Data buffer for A in SVMSearch.
	float *prLDat;	// Data buffer for rL in SVMSearch.
	float *pxcoordDat;	// Data buffer for xcoord in SVMSearch.
	float *pycoordDat;	// Data buffer for ycoord in SVMSearch.

	float *pnrDat;		// Data buffer for nr in SVMSearch
} CLM_SVM_WORKSPACE;


typedef struct _clm_model_
{
	CLM_SHAPE_MODEL 		ShapeModel;
	CLM_PATCH_MODEL 	PatchModel;

	// Temporary working data:
	CLM_SVM_WORKSPACE SvmWorkSpace[MAX_NUM_THREADS];
	
} CLM_MODEL;

typedef struct _clm_si_
{
	CvMat* xy;
	CvMat* AlignedXY;	// x, y aligned to mean shape.
	float transform[4]; //transform matrix: [c s dx; s -c dy] stored in [c s dx dy].
} CLM_SI;

typedef struct _clm_options_
{
	int FaceSize[2];
	int verbose;
	int SearchRegion[2];	// Search region size.

	int NumInterations;

	float MaxIterError;		// Iteration error before stop.
	
	int PatchSize[2];

} CLM_OPTIONS;

////////////////
// Functions:
////////////////

extern int CLM_LoadModel(const char *filename, CLM_MODEL& pModel);

extern int CLM_MakeInitialShape(CLM_MODEL& Model, cv::Mat& Image, double x, double y, double w, double h, double rot, CLM_SI& Initial);
extern int CLM_Search(CLM_MODEL& Model, cv::Mat& Image, CLM_SI& Initial, CLM_SI& Final, CLM_OPTIONS& Options);


extern int CLM_CopySi(CLM_SI& dst, CLM_SI& src);
extern int CLM_DumpSi(CLM_SI& si);

extern int CLM_StartTimer(int id);
extern long CLM_StopTimer(int id);


#define CLM_OPTM_ERROR_WEIGHT	(1.5e-2)


#endif //_CLM_OPENCV_H_

