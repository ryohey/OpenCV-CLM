///////////////////////////////////////////////////////
// Constrained Local Model (CLM) Demonstration
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

#include <stdio.h>
#include "cv.h"
#include "highgui.h"

#include "clm.h"

LARGE_INTEGER PerfFreq;
DWORD CountsPerSec;
static LARGE_INTEGER L1;
static LARGE_INTEGER L2;

const static char *OutputWinName = "Constrained Local Model Demo - OpenCV";

long FrameCount = 0;

IplImage *OrigImg=0, *GrayImg=0, *DispImg = 0;

extern void DrawFaceShape(cv::Mat& image, CvMat *xy);


FILE * fpout;
FILE *fpresponse;
FILE *fopout;
FILE *fweights;
FILE *fcoeffs;
FILE *fci;


#define WRITE_VIDEO		0

int pic_vid_main(CLM_MODEL& CLM_Model, const char *dirName)
{
    int key=0;
	int ret;

	int base = 0;
	int x0 = 388, y0= 309;
	int i=base;
	char imgName[256];
	sprintf(imgName, "%s/franck_%05d.jpg", dirName, base);
	
	IplImage * input = cvLoadImage(imgName);

	if(input == 0)
	{
		printf("Cannot load file %s\n", imgName);
		return 0;
	}

    //fpout = fopen("dx.txt", "w");
    //fpresponse = fopen("response.txt", "w");
    //fweights = fopen("weights.txt", "w");
    //fcoeffs = fopen("coeffs.txt", "w");

	IplImage *DispImage = cvCreateImage(cvSize(input->width, input->height), input->depth, input->nChannels);
	IplImage *searchimg = cvCreateImage(cvSize(input->width, input->height), 8, 1);
	
    cvConvertImage(input,searchimg,0);

    ///////////////////////////
    // Make initial values
    ///////////////////////////    
	CLM_SI Si_Init, Si_Final;
	memset(&Si_Init, 0, sizeof(CLM_SI));
	memset(&Si_Final, 0, sizeof(CLM_SI));

	int width = 270;
	int height = 270;

	ret = CLM_MakeInitialShape(CLM_Model, searchimg, x0-width/2, y0-height/2, width, height, 0, &Si_Init);
	ret = CLM_MakeInitialShape(CLM_Model, searchimg, x0-width/2, y0-height/2, width, height, 0, &Si_Final);
    
    ////////////////////////
    // Do search with initial guess
    // on the first image.
    ////////////////////////
	CLM_OPTIONS Options;
	Options.NumInterations = 20;
	
	Options.SearchRegion[0] = 16;
	Options.SearchRegion[1] = 16;

   	ret = CLM_Search(CLM_Model, searchimg, &Si_Init, &Si_Final, &Options);
    cv::Mat input_(input);
	DrawFaceShape(input_, Si_Final.xy);

    #if WRITE_VIDEO
    CvVideoWriter *writer = cvCreateVideoWriter("out.avi",-1,25,cvSize(720,576),1);
    #endif


    #if WRITE_VIDEO
    cvWriteFrame(writer,input);
    #endif
	cvCopy(input, DispImage);
	cvShowImage(OutputWinName, DispImage );
	cvReleaseImage(&input);
	
    /////////////////////////////////////
    // Now we are ready to do tracking.
    /////////////////////////////////////
    while( key != 'q' ) 
	{
		i++;
		CLM_CopySi(&Si_Init, &Si_Final);
		sprintf(imgName, "%s/franck_%05d.jpg", dirName, i);
		input = cvLoadImage(imgName);
		cvConvertImage(input,searchimg,0);
		cvCopy(input, DispImage);

        QueryPerformanceCounter(&L1);

		Options.NumInterations = 10;
		//Options.MaxIterError = 0.015;
    	
    	ret = CLM_Search(CLM_Model, searchimg, &Si_Init, &Si_Final, &Options);
		//CLM_DumpSi(&Si_Init);
		//CLM_DumpSi(&Si_Final);
		
        QueryPerformanceCounter(&L2);
        float time = (float)(L2.LowPart - L1.LowPart)*1000.0f/CountsPerSec;
        printf("Search time: %4.1f\n ", time);

        cv::Mat DispImage_(DispImage);
		DrawFaceShape(DispImage_, Si_Final.xy);

        #if WRITE_VIDEO
        cvWriteFrame(writer,DispImage);
        #endif

		cvShowImage(OutputWinName, DispImage );

		if(i>=4980)
			break;

	    cvReleaseImage(&input);

	    /* exit if user press 'q' */
        key = cvWaitKey( 1 );
    }

	

#if WRITE_VIDEO
    cvReleaseVideoWriter(&writer); 
#endif

    return 0;
}


#include "omp.h"

int main(int argc, char **argv) {
    auto programDir = std::string(argv[0]);
    auto resourceDir = programDir.substr(0, programDir.find_last_of("\\/"));
    
    auto xmlFileName= (resourceDir + "/CLMModel.xml").c_str();

	omp_set_num_threads(CURRENT_NUM_THREADS);


	QueryPerformanceFrequency(&PerfFreq);
	CountsPerSec = PerfFreq.LowPart;

    
	printf("Loading CLM Model file %s ...", xmlFileName);

	CLM_MODEL CLM_Model;

    int ret = CLM_LoadModel(xmlFileName, CLM_Model);
    if(ret)
    {
    	printf("Cannot load %s...\n", xmlFileName);
    	exit(0);
    }

	
    /* create a window for the video */
    cvNamedWindow( OutputWinName, CV_WINDOW_AUTOSIZE );
    
    /*
     place the all-images directory
     contains franck_00000.jpg ~ franck_01999.jpg
     This can be get from:
        First 1000 images
        Second 1000 images
        http://www-prima.inrialpes.fr/FGnet/data/01-TalkingFace/talking_face.html
     */
    auto imagesDir = "/Users/ryohei/Desktop/all-images";
    
	pic_vid_main(CLM_Model, imagesDir);

	/* free memory */
    cvDestroyWindow( OutputWinName );

    return 0;
    
}



