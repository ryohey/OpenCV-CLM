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

#include <stdio.h>
#include <chrono>
#include "cv.h"
#include "highgui.h"

#include "CLM.h"

using namespace CLM;

const static char *OutputWinName = "Constrained Local Model Demo - OpenCV";

long FrameCount = 0;

IplImage *OrigImg=0, *GrayImg=0, *DispImg = 0;

extern void drawFaceShape(cv::Mat& image, CvMat *xy);


FILE * fpout;
FILE *fpresponse;
FILE *fopout;
FILE *fweights;
FILE *fcoeffs;
FILE *fci;


#define WRITE_VIDEO		0

int pic_vid_main(Model& Model, const char *dirName)
{
    int key=0;
	int ret;

	int base = 0;
	int x0 = 388, y0= 309;
	int i=base;
	char imgName[256];
	sprintf(imgName, "%s/franck_%05d.jpg", dirName, base);
	
    cv::Mat input = cv::imread(imgName);
    
	if(input.empty())
	{
		printf("Cannot load file %s\n", imgName);
		return 0;
	}

    //fpout = fopen("dx.txt", "w");
    //fpresponse = fopen("response.txt", "w");
    //fweights = fopen("weights.txt", "w");
    //fcoeffs = fopen("coeffs.txt", "w");

    cv::Mat DispImage(input.rows, input.cols, input.depth(), input.channels());
    cv::Mat searchimg;
    
    cv::cvtColor(input, searchimg, CV_BGR2GRAY, 1);

    ///////////////////////////
    // Make initial values
    ///////////////////////////    
	Si Si_Init, Si_Final;
	memset(&Si_Init, 0, sizeof(Si));
	memset(&Si_Final, 0, sizeof(Si));

	int width = 270;
	int height = 270;

	ret = makeInitialShape(Model, searchimg, x0-width/2, y0-height/2, width, height, 0, Si_Init);
	ret = makeInitialShape(Model, searchimg, x0-width/2, y0-height/2, width, height, 0, Si_Final);
    
    ////////////////////////
    // Do search with initial guess
    // on the first image.
    ////////////////////////
	Options Options;
	Options.NumInterations = 20;
	
	Options.SearchRegion[0] = 16;
	Options.SearchRegion[1] = 16;

   	ret = search(Model, searchimg, Si_Init, Si_Final, Options);
	drawFaceShape(input, Si_Final.xy);

    #if WRITE_VIDEO
    CvVideoWriter *writer = cvCreateVideoWriter("out.avi",-1,25,cvSize(720,576),1);
    #endif


    #if WRITE_VIDEO
    cvWriteFrame(writer,input);
    #endif
    input.copyTo(DispImage);
    cv::imshow(OutputWinName, DispImage );
	
    /////////////////////////////////////
    // Now we are ready to do tracking.
    /////////////////////////////////////
    while( key != 'q' ) 
	{
		i++;
		copySi(Si_Init, Si_Final);
		sprintf(imgName, "%s/franck_%05d.jpg", dirName, i);
        input = cvLoadImage(imgName);
        cv::cvtColor(input, searchimg, CV_BGR2GRAY, 1);
        input.copyTo(DispImage);
        
        auto startTime = std::chrono::system_clock::now();

		Options.NumInterations = 10;
		//Options.MaxIterError = 0.015;
    	
    	ret = search(Model, searchimg, Si_Init, Si_Final, Options);
		//dumpSi(&Si_Init);
		//dumpSi(&Si_Final);
        
        auto endTime = std::chrono::system_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(startTime - endTime).count();

        printf("Search time: %4.1lld\n ", time);

        cv::Mat DispImage_(DispImage);
		drawFaceShape(DispImage_, Si_Final.xy);

        #if WRITE_VIDEO
        cvWriteFrame(writer,DispImage);
        #endif

        cv::imshow(OutputWinName, DispImage );

		if(i>=4980)
            break;

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
    
	printf("Loading CLM Model file %s ...", xmlFileName);

	Model Model;

    int ret = loadModel(xmlFileName, Model);
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
    auto imagesDir = "/Users/ryohey/Desktop/all-images";
    
	pic_vid_main(Model, imagesDir);

	/* free memory */
    cvDestroyWindow( OutputWinName );

    return 0;
    
}



