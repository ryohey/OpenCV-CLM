/////////////////////////////////////////////////////////
// Implementation of CLM utility functions.
// 
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

#include "cv.h"
#include "highgui.h"

#include "utils.h"

using namespace CLM;

int procrustes0(float *pdat, float *pbase, int numPts)
{
	int i;
	float *pdat0 = pdat, *pbase0 = pbase;
	float sumx=0, sumy=0, sumxb=0, sumyb=0;

	
	////////////////////////////////
	// Implement the simple way:
	////////////////////////////////


	//	a. remove mean:
	for(i=0;i<numPts;i++)
	{
		sumx+=*pdat++;
		sumy+=*pdat++;

		sumxb+=*pbase++;
		sumyb+=*pbase++;
	}

	float meanx, meany, meanxb, meanyb;
	meanx = sumx/numPts; 
	meany = sumy/numPts;
	meanxb = sumxb/numPts;
	meanyb = sumyb/numPts;
	

	pdat = pdat0;
	pbase = pbase0;


	for(i=0;i<numPts;i++)
	{
		*pdat++-=meanx;
		*pdat++-=meany;

		*pbase++-=meanxb;
		*pbase++-=meanyb;
	}

	//	b. calculate scale:
	pdat = pdat0;
	pbase = pbase0;

	sumx = 0; sumy=0; sumxb = 0; sumyb=0;
	
	for(i=0;i<numPts;i++)
	{
		sumx+=(*pdat)*(*pdat++);
		sumx+=(*pdat)*(*pdat++);

		sumxb+=(*pbase)*(*pbase++);
		sumxb+=(*pbase)*(*pbase++);
	}

	float scale = sqrt(sumxb)/sqrt(sumx);

	pdat = pdat0;
	pbase = pbase0;

	for(i=0;i<numPts*2;i++)
	{
		(*pdat++)*=scale;
	}

	//	c. ... THIS IS GETTING TOO COMPLICATED...

	return 0;
}


///////////////////////////////////////////////////////////////////
// My own procrustes analysis:
// pdat: input shape (x1, y1, x2, y2, ... xn, yn);
// pbase: input base shape (x1, y1, x2, y2, ... xn, yn);
// numpts: number of points (n)
// pout: pdat aligned to base shape (x1, y1, x2, y2, ... xn, yn);
// tform: transform from pdat to pbase, [a b dx, b -a dy] stored in [a b dx dy];
//////////////////////////////////////////////////////////////////
int CLM::alignData(float *pdat, float *pbase, int numPts, float *pout, float *tform)
{
	int i;
    float *pdat0 = pdat;
    
	float ux=0, uy=0, uxp=0, uyp=0, s=0, w1=0, w2=0;
	float tx, ty, txp, typ;
	for(i=0;i<numPts;i++)
	{
		tx = *pdat++;
		ty = *pdat++;

		txp = *pbase++;
		typ = *pbase++;

		ux+=tx;
		uy+=ty;
		uxp+=txp;
		uyp+=typ;

		s+=tx*tx+ty*ty;

		w1+=txp*ty - typ*tx;
		w2+=tx*txp + ty*typ;
		
	}

	ux /= numPts;
	uy /= numPts;
	uxp /= numPts;
	uyp /= numPts;

	float pmat[16], invp[16];
	pmat[0] = ux; pmat[1] = uy; pmat[2] = 1; pmat[3] = 0;
	pmat[4] = uy; pmat[5] =-ux; pmat[6]=0; pmat[7]=1;
	pmat[8] = 0; pmat[9] = s; pmat[10]=numPts*uy; pmat[11] = -numPts*ux;
	pmat[12] = s; pmat[13] = 0; pmat[14]=numPts*ux; pmat[15] = numPts*uy;

	float rmat[4];
	rmat[0] = uxp; rmat[1] = uyp; rmat[2] = w1; rmat[3] = w2;

	// pmat^-1 * rmat;
    cv::Mat MatP(4, 4, CV_32FC1, pmat);
    cv::Mat MatR(4, 1, CV_32FC1, rmat);

    cv::Mat MatPinv(4, 4, CV_32FC1, invp);
    cv::invert(MatP, MatPinv);

    cv::Mat MatRes(4, 1, CV_32FC1, tform);

    cv::gemm(MatPinv, MatR, 1, 0, 0, MatRes);

	//	assemble output:
	pdat = pdat0;
	for(i=0;i<numPts;i++)
	{
		tx = *pdat++;
		ty = *pdat++;

		*pout++ = tform[0]*tx + tform[1]*ty + tform[2];
		*pout++ = -tform[1]*tx + tform[0]*ty + tform[3];
	}

	return 0;
}

int CLM::alignDataInverse(float *pdat, float *tform, int numPts, float *pout)
{
	int i;
	float tx, ty;
	float sc = tform[0]*tform[0]+tform[1]*tform[1];
	//	assemble output:
	for(i=0;i<numPts;i++)
	{
		tx = *pdat++ - tform[2];
		ty = *pdat++ - tform[3];
		
		*pout++ = (tform[0]*tx - tform[1]*ty)/sc;
		*pout++ = (tform[1]*tx + tform[0]*ty)/sc;
	}

	return 0;
}

int CLM::copySi(Si& dst, Si& src)
{
	cvCopy(src.xy, dst.xy);
	cvCopy(src.
           AlignedXY, dst.AlignedXY);

	for(int i=0;i<4;i++)
	{
		dst.transform[i] = src.transform[i];
	}

	return 0;
}

int CLM::dumpSi(Si& si)
{
	//printf("center: %d, %d\n", (int)si->center[0], (int)si->center[1]);
	//printf("rotation: %f, scale: %f\n", si->rotation, si->scale);
FILE *psi = fopen("si.txt", "w");

	float *pdat = si.xy->data.fl;
	float x, y;
	for(int i=0;i<si.xy->rows;i++)
	{
		x = *pdat++;
		y = *pdat++;

		fprintf(psi, "%4.1f %4.1f ", x, y);
	}
	printf("\n");

	fclose(psi);

	return 0;
}
