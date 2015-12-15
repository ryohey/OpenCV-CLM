/////////////////////////////////////////////////////////
// Load Constrained Local Model (CLM) model file from
// XML document, generated using Matlab.
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

#include "stdafx.h"

#include "cv.h"
#include "highgui.h"

#include "clm.h"
#include "clm_priv.h"

#include "tinyxml.h"

static int ReadVecFromString(const char *str, float *f, int count);
static int ReadMatFromString(const char *str, CvMat *Mat);


int CLM_LoadModel(const char *filename, CLM_MODEL& pModel)
{
	
	TiXmlDocument doc(filename);

	bool loadok = doc.LoadFile();
	if(!loadok)
	{
		return -1;
	}

	TiXmlHandle docHandle( &doc );
	
	auto Element = docHandle.FirstChild("root").FirstChild("ShapeModel").FirstChild("NumEvalues").ToElement();
	auto str = Element->GetText();
	pModel.ShapeModel.NumEvalues = atoi(str);

	Element = docHandle.FirstChild("root").FirstChild("ShapeModel").FirstChild("NumPts").ToElement();
	str = Element->GetText();
	pModel.ShapeModel.NumPtsPerSample = atoi(str);

	// ShapeModel.MeanShape:
	Element = docHandle.FirstChild("root").FirstChild("ShapeModel").FirstChild("MeanShape").ToElement();
	str = Element->GetText();
	pModel.ShapeModel.MeanShape = cvCreateMat(1, pModel.ShapeModel.NumPtsPerSample*2, CV_32FC1);
	int NumRead = ReadVecFromString(str, pModel.ShapeModel.MeanShape->data.fl, pModel.ShapeModel.NumPtsPerSample*2);
	if(NumRead != pModel.ShapeModel.NumPtsPerSample*2)
	{
		return -1;
	}

	// ShapeModel.Evalues:
	Element = docHandle.FirstChild("root").FirstChild("ShapeModel").FirstChild("Evalues").ToElement();
	str = Element->GetText();
	pModel.ShapeModel.Evalues = cvCreateMat(1, pModel.ShapeModel.NumEvalues, CV_32FC1);
	NumRead = ReadVecFromString(str, pModel.ShapeModel.Evalues->data.fl, pModel.ShapeModel.NumEvalues);
	if(NumRead != pModel.ShapeModel.NumEvalues)
	{
		return -1;
	}

	// ShapeModel.Evectors:
	Element = docHandle.FirstChild("root").FirstChild("ShapeModel").FirstChild("Evectors").ToElement();
	str = Element->GetText();
	pModel.ShapeModel.Evectors = cvCreateMat(pModel.ShapeModel.NumPtsPerSample*2, pModel.ShapeModel.NumEvalues, CV_32FC1);
	NumRead = ReadMatFromString(str, pModel.ShapeModel.Evectors);

	
	// PatchModel.NumPatches:
	Element = docHandle.FirstChild("root").FirstChild("PatchModel").FirstChild("NumPatches").ToElement();
	str = Element->GetText();
	pModel.PatchModel.NumPatches = atoi(str);

	// PatchModel.PatchSize:
	Element = docHandle.FirstChild("root").FirstChild("PatchModel").FirstChild("PatchSize").ToElement();
	str = Element->GetText();
	pModel.PatchModel.PatchSize[0] = atoi(str);

	while(*str<='9' && *str>='0')
		str++;
	if(*str == ' ')
		str++;

	if(*str == 0)
		return -1;
	
	pModel.PatchModel.PatchSize[1] = atoi(str);

	// PatchModel.weights:
	Element = docHandle.FirstChild("root").FirstChild("PatchModel").FirstChild("Weights").ToElement();
	str = Element->GetText();

	auto tempW = cvCreateMat(pModel.PatchModel.PatchSize[1]*pModel.PatchModel.PatchSize[0], pModel.PatchModel.NumPatches, CV_32FC1);
	auto tempWt = cvCreateMat(pModel.PatchModel.NumPatches, pModel.PatchModel.PatchSize[1]*pModel.PatchModel.PatchSize[0], CV_32FC1);
	ReadMatFromString(str, tempW);

	cvT(tempW, tempWt);

	auto pw = tempWt->data.fl;
	auto ptempxx = cvCreateMat(pModel.PatchModel.PatchSize[1], pModel.PatchModel.PatchSize[0], CV_32FC1);
	auto ptempxxn = cvCreateMat(pModel.PatchModel.PatchSize[1], pModel.PatchModel.PatchSize[0], CV_32FC1);

	auto wmat = cvMat(pModel.PatchModel.PatchSize[0], pModel.PatchModel.PatchSize[1], CV_32FC1, pw);
	for(int i=0;i<pModel.PatchModel.NumPatches;i++)
	{
		wmat.data.fl = pw;
		
		cvT(&wmat, ptempxx);
		cvNormalize(ptempxx, ptempxxn, 1.0, -1.0, CV_MINMAX);

		pw+= pModel.PatchModel.PatchSize[1]*pModel.PatchModel.PatchSize[0];

		// Resize to half template size:

		//pModel.PatchModel.WeightMats[i] = cvCreateMat(pModel.PatchModel.PatchSize[1]/2, pModel.PatchModel.PatchSize[0]/2, CV_32FC1);
		//cvResize(ptempxxn, pModel.PatchModel.WeightMats[i]);

		pModel.PatchModel.WeightMats[i] = cvCreateMat(pModel.PatchModel.PatchSize[1], pModel.PatchModel.PatchSize[0], CV_32FC1);
		cvCopy(ptempxxn, pModel.PatchModel.WeightMats[i]);
	}

	cvReleaseMat(&ptempxx);
	cvReleaseMat(&ptempxxn);

	cvReleaseMat(&tempW);
	cvReleaseMat(&tempWt);


	// Do partial calculations:
	int NumX = pModel.ShapeModel.NumPtsPerSample*2;

	pModel.ShapeModel.p2alphaWMat =cvCreateMat(NumX, NumX, CV_32FC1); 

	pModel.ShapeModel.pI_EEtMat = cvCreateMat(NumX, NumX, CV_32FC1);
	auto pIMat = cvCreateMat(NumX, NumX, CV_32FC1);

	cvSetIdentity(pIMat);
	
	cvGEMM(pModel.ShapeModel.Evectors, pModel.ShapeModel.Evectors, -1.0, pIMat, 1, pModel.ShapeModel.pI_EEtMat, CV_GEMM_B_T);
	
	float alpha = (float)CLM_OPTM_ERROR_WEIGHT;
	cvGEMM(pModel.ShapeModel.pI_EEtMat, pModel.ShapeModel.pI_EEtMat, 2*alpha, 0, 0, pModel.ShapeModel.p2alphaWMat, CV_GEMM_A_T);

	cvReleaseMat(&pIMat);


	pModel.ShapeModel.pBMat = cvCreateMat(pModel.ShapeModel.Evectors->rows, pModel.ShapeModel.Evectors->cols, CV_32FC1);
	auto pBMatDat = pModel.ShapeModel.pBMat->data.fl;
	auto pEvecDat = pModel.ShapeModel.Evectors->data.fl;
	auto pEvalues = pModel.ShapeModel.Evalues->data.fl;

	for(int j=0;j<NumX;j++)
	{
		for(int i=0; i<pModel.ShapeModel.NumEvalues;i++)
		{
			*pBMatDat++ = (*pEvecDat++)/sqrt(pEvalues[i]);
		}
	}

	// Initialize temporary working data. 
	pModel.ShapeModel.p_2HMat = cvCreateMat(NumX, NumX, CV_32FC1);
	cvZero(pModel.ShapeModel.p_2HMat);

	
	pModel.ShapeModel.p_FMat = cvCreateMat(NumX, 1, CV_32FC1);

	pModel.ShapeModel.p2alphaWtBMat = cvCreateMat(NumX, 1, CV_32FC1);

	pModel.ShapeModel.pBBaseMat = cvCreateMat(pModel.ShapeModel.NumEvalues, 1, CV_32FC1);


	// Initialize workspace data:
	for(int i=0;i<CURRENT_NUM_THREADS;i++)
	{
		pModel.SvmWorkSpace[i].pAdat = (float*)malloc(65536*3*sizeof(float));	//(TemplateSizeX*scale/SearchArea*2)^2*3
		pModel.SvmWorkSpace[i].prLDat = (float*)malloc(65536*sizeof(float));		//(TemplateSizeX*scale/SearchArea*2)^2
		pModel.SvmWorkSpace[i].pxcoordDat = (float*)malloc(65536*sizeof(float));		//(TemplateSizeX*scale/SearchArea*2)^2
		pModel.SvmWorkSpace[i].pycoordDat = (float*)malloc(65536*sizeof(float));		//(TemplateSizeX*scale/SearchArea*2)^2


		pModel.SvmWorkSpace[i].pnrDat = (float*)malloc(65536*16*sizeof(float));	// (2*dx+TemplateSizeX)^2
	}
	
	return 0;
}



static int ReadVecFromString(const char *str, float *f, int count)
{
	auto ptr = str;
	int i;

	for(i=0;i<count;i++)
	{
		f[i] = (float)atoi(ptr);

		while(*ptr!=' ' && *ptr)
			ptr++;

		if(*ptr ==0) 
			return i+1;
		if(*ptr == ' ')
			ptr++;
		
	}

	return -1;
}

static int ReadMatFromString(const char *str, CvMat *Mat)
{
	auto ptr = str;
	int i;
	float *pdat;
	
	pdat = Mat->data.fl;
	
	for(i=0;i<Mat->cols*Mat->rows;i++)
	{
		*pdat++ = (float)atof(ptr);	

		while(*ptr!=' ' && *ptr)
			ptr++;
			
		if(*ptr ==0) 
			return 0;

		if(*ptr == ' ')
			ptr++;
	}

	return 0;
}



