/////////////////////////////////////////////////////////
// Implementation of CLM linear SVM classification
// and quadratic fitting.
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

#include "omp.h"

#include "cv.h"
#include "highgui.h"

#include "clm.h"
#include "clm_priv.h"

#include "QuadProg_3.hh"

static void CopyImageToMat(IplImage *pImg, CvMat *pMat);
static void DumpResponse(CvMat * r);
static void DumpWeights(CvMat * r);


double* CLM_SvmSearch(CLM_SI* Si, CLM_MODEL& pModel, IplImage* Image, float *QuadCoeffs, CLM_OPTIONS * Options)
{
	auto& ShapeModel = pModel.ShapeModel;
	auto& PatchModel = pModel.PatchModel;
	auto& pMeanShape = ShapeModel.MeanShape;

	int cw, ch;
	cw = Options->SearchRegion[0] + pModel.PatchModel.PatchSize[0];
	ch = Options->SearchRegion[1] + pModel.PatchModel.PatchSize[1];

	///////////////////////////////////////
	// Step 1, align current shape to mean shape:
	///////////////////////////////////////
	CLM_align_data(Si->xy->data.fl, pMeanShape->data.fl, ShapeModel.NumPtsPerSample, Si->AlignedXY->data.fl, Si->transform);

	// Invert transform matrix...
	float tm[9], invtm[9];
	tm[0] = Si->transform[0]; tm[1] = Si->transform[1]; tm[2] = Si->transform[2]; 
	tm[3] = -Si->transform[1]; tm[4] = Si->transform[0]; tm[5] = Si->transform[3];
	tm[6] = 0; tm[7] = 0; tm[8] = 1;
    cv::Mat MatTm(3, 3, CV_32FC1, tm);
    cv::Mat invMatTm(3, 3, CV_32FC1, invtm);
    cv::invert(MatTm, invMatTm);

	///////////////////////////////////////////////////////////////
	// Step 2, crop patches around each feature point position, 
	// do SVM classification, and quadratic fitting.
	//////////////////////////////////////////////////////////////
	auto pxy = Si->AlignedXY->data.fl;

	#pragma omp parallel for // This is where OpenMP shines.
	for(int i=0;i<ShapeModel.NumPtsPerSample;i++)
	{
		int thread_id = omp_get_thread_num();
		//int thread_id = 0; // if you don't have OpenMP,

		float m[6];
		CvMat M = cvMat( 2, 3, CV_32F, m );
        cv::Mat Response(Options->SearchRegion[1] + 1, Options->SearchRegion[0] + 1, CV_32FC1);
        cv::Mat weights = PatchModel.WeightMats[i];
		
		float x0c = *(pxy + i*2), y0c = *(pxy + i*2 + 1);

		///////////////////////////////////
		// Step 1, crop patches
		///////////////////////////////////
		m[0] = invtm[0];
		m[1] = invtm[1];
		m[2] = invtm[2] + invtm[0]*x0c + invtm[1]*y0c;
		m[3] = -m[1];
		m[4] = m[0];
		m[5] = invtm[5] - invtm[1]*x0c + invtm[0]*y0c;
		
        cv::Mat pCrop = cv::Mat(cw, ch, CV_8UC1, 1);
        IplImage pCrop_ = pCrop;
		cvGetQuadrangleSubPix(Image, &pCrop_, &M);

		

		//////////////////////////////////
		// Step 2, Convert to float type.
		//////////////////////////////////
        cv::Mat Matnr;
        pCrop.convertTo(Matnr, CV_32FC1);
        
		////////////////////////////////////////////////
		// Step 3, cross-correlation with SVM image. 
		// Again, this is not exactly SVM classifcation
		// as John Platt proposed, but it should suffice.
		////////////////////////////////////////////////
        CvMat Matnr_ = Matnr;
        cv::matchTemplate(Matnr, weights, Response, CV_TM_CCORR_NORMED);

        cv::normalize(Response, Response, 1.0, -1.0, CV_MINMAX);
		

		///////////////////////////
		// Step 4, quadratic fit:
		///////////////////////////

		// 4.1, Find center:
		auto pxd = pModel.SvmWorkSpace[thread_id].pxcoordDat;
		auto pyd = pModel.SvmWorkSpace[thread_id].pycoordDat;
		int rr= Response.rows, rc = Response.cols;
		
		for (int j=0; j<rr;j++)
		{
			for(int i=0;i<rc;i++)
			{
				*pxd++ = (float)i;
				*pyd++ = (float)j;
			}
		}

		#define USE_WMEAN_CENTER 0	

		#if USE_WMEAN_CENTER
			// Calculate weighed mean center...
			CvMat *pweighx = cvCreateMat(Response->rows, Response->cols, CV_32FC1);
			CvMat *pweighy = cvCreateMat(Response->rows, Response->cols, CV_32FC1);
			
			cvMul(&MatXcoord, Response, pweighx);
			cvMul(&MatYcoord, Response, pweighy);

			CvScalar sumrx = cvSum(pweighx);
			CvScalar sumry = cvSum(pweighy);
			if(abs(sumr.val[0]<1e-5))
				sumr.val[0] = 1;


			float centerx = float(sumrx.val[0]/sumr.val[0]);
			if(centerx<=0)
				centerx = 0;
			else if(centerx>=Response->cols)
				centerx = Response->cols;

			float centery = float(sumry.val[0]/sumr.val[0]);
			if(centery<=0)
				centery = 0;
			else if(centery>=Response->rows)
				centery = Response->rows;


			cvReleaseMat(&pweighx);
			cvReleaseMat(&pweighy);
		#else
			// Use max response position as center.
			double maxv;
            cv::Point maxLoc;

            cv::minMaxLoc(Response, 0, &maxv, 0, &maxLoc);
			auto centerx = (float)maxLoc.x;
			auto centery = (float)maxLoc.y;

		#endif

		
		// 4.2, Assemble A:
		int wr, hr;
		wr = Response.rows;
		hr = Response.cols;

        cv::Mat MatA(wr*hr, 3, CV_32FC1, pModel.SvmWorkSpace[thread_id].pAdat);
        cv::Mat MatrL(wr*hr, 1, CV_32FC1, pModel.SvmWorkSpace[thread_id].prLDat);
		
		float *pAd = pModel.SvmWorkSpace[thread_id].pAdat;
		float *prLd = pModel.SvmWorkSpace[thread_id].prLDat;
		
		pxd = pModel.SvmWorkSpace[thread_id].pxcoordDat;
		pyd = pModel.SvmWorkSpace[thread_id].pycoordDat;

		float x2t, y2t;
		int wrhr = wr*hr;
		float x2tc, y2tc;
		float hr2 = 1.0f/hr/hr, wr2 = 1.0f/wr/wr;

		for(int j=0;j<wrhr; j++)
		{
			x2t = *pxd++;
			y2t = *pyd++;
			
			x2tc = x2t-centerx;
			y2tc = y2t-centery;
			x2t = x2tc*x2tc*hr2;
			y2t = y2tc*y2tc*wr2;
			
			*pAd++ = x2t;
			*pAd++ = y2t;
			*pAd++ = 1;

			*prLd++ = Response.at<float>(j);
		}

		// 4.3, Prepare G, g0, CI, ci0;
		float Dat2AtA[9];
        cv::Mat Mat2AtA(3, 3, CV_32FC1, Dat2AtA);
        cv::gemm(MatA, MatA, 2.0, 0, 0, Mat2AtA, CV_GEMM_A_T);

		float Dat_2At_rl[3];
        cv::Mat Mat_2At_rL(3, 1, CV_32FC1, Dat_2At_rl);
        cv::gemm(MatA, MatrL, -2.0, 0, 0, Mat_2At_rL, CV_GEMM_A_T);

		// 4.4, Quadratic programming:
		double G[3][3];
		float *pa2ad = Dat2AtA;
		G[0][0] = pa2ad[0];
		G[0][1] = pa2ad[1];
		G[0][2] = pa2ad[2];
		G[1][0] = pa2ad[3];
		G[1][1] = pa2ad[4];
		G[1][2] = pa2ad[5];
		G[2][0] = pa2ad[6];
		G[2][1] = pa2ad[7];
		G[2][2] = pa2ad[8];
		double g0[3];
		float *p2atrld= Dat_2At_rl;
		g0[0] = p2atrld[0];
		g0[1] = p2atrld[1];
		g0[2] = p2atrld[2];

		// CI = [-1 0 0; 0 -1 0; 0 0 -1]; ci0 = [0; 0; +inf];
		double CI[3][3];
		memset(CI, 0, sizeof(CI));
		CI[0][0]= -1;
		CI[1][1] = -1;
		CI[2][2] = -1;
		double ci0[3];
		ci0[0] = 0;
		ci0[1] = 0;
		ci0[2] = 1e8;

		double coeffs[3];
		solve_quadprog_3(G, g0, 3, 0, 0, 0, CI, ci0, 3, coeffs);

		// Sanity check on results...
		double a=coeffs[0]/wr/wr, b=coeffs[1]/hr/hr, c=coeffs[2];
		if(a>1e-5 || b > 1e-5)
		{
			printf("Warning: convex fitting result incorrect: %4.6f, %4.6f\n", a, b);
			if(a>0.0) a= -a;
			if(b>0.0) b= -b;

		}
		
		/////////////////////////////////////
		// Step 5, Assemble output.
		// Again, the ordering looks awkward, 
		// but it works somehow, so I didn't
		// both to change it.
		/////////////////////////////////////
		QuadCoeffs[0+8*i] = (float)b;
		QuadCoeffs[1+8*i] = 0;
		QuadCoeffs[2+8*i] = -2*(float)b*centery;
		QuadCoeffs[3+8*i] = (float)a;
		QuadCoeffs[4+8*i] = -2*(float)a*centerx;
		QuadCoeffs[5+8*i] = (float)a*centerx*centerx + (float)b*centery*centery + (float)c;
		QuadCoeffs[6+8*i] = centerx;
		QuadCoeffs[7+8*i] = centery;

	}	// end of for 68 pts.

	return 0;
}

static void CopyImageToMat(IplImage *pImg, CvMat *pMat)
{
	uchar *pidat = (uchar*) pImg->imageData;
	float *pmdat = pMat->data.fl;

	for(int j=0;j<pImg->height;j++)
	{
		for(int i=0;i<pImg->width;i++)
		{
			pmdat[i] = (float)pidat[i];
		}
		pidat+=pImg->widthStep;
		pmdat+=pMat->cols;
	}
}

//////////////////////////////
// Below are debug functions.
//////////////////////////////
extern FILE *fpresponse;
static void DumpResponse(CvMat * r)
{
	float *pdat = r->data.fl;

	for(int i=0;i<r->rows*r->cols;i++)
		fprintf(fpresponse, "%4.8f ", *pdat++);

	fprintf(fpresponse, "\n");
	fflush(fpresponse);

}
extern FILE *fweights;
static void DumpWeights(CvMat * r)
{
	float *pdat = r->data.fl;

	for(int i=0;i<r->rows*r->cols;i++)
		fprintf(fweights, "%4.4f ", *pdat++);

	fprintf(fweights, "\n");
	fflush(fweights);

}

