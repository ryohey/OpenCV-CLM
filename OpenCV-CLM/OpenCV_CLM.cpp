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
#include "DrawFace.h"

using namespace CLM;

const static char *OutputWinName = "Constrained Local Model Demo - OpenCV";

FILE *fpout;
FILE *fpresponse;
FILE *fopout;
FILE *fweights;
FILE *fcoeffs;
FILE *fci;

class FaceTracker {
private:
    Options options = {};
    Si Si_Init = {};
    Si Si_Final = {};
    Model& model;
    
public:
    FaceTracker(Model& model, cv::Mat& image);
    Si& update(cv::Mat& image);
};

FaceTracker::FaceTracker(Model& model, cv::Mat& image) : model(model) {
    cv::Mat searchimg;
    cv::cvtColor(image, searchimg, CV_BGR2GRAY, 1);
    
    int width = 270;
    int height = 270;
    int x0 = 388;
    int y0= 309;
    
    makeInitialShape(model, searchimg, x0-width/2, y0-height/2, width, height, 0, Si_Init);
    makeInitialShape(model, searchimg, x0-width/2, y0-height/2, width, height, 0, Si_Final);
    
    options.NumInterations = 20;
    options.SearchRegion[0] = 16;
    options.SearchRegion[1] = 16;
}

Si& FaceTracker::update(cv::Mat& image) {
    copySi(Si_Init, Si_Final);
    
    cv::Mat searchimg;
    cv::cvtColor(image, searchimg, CV_BGR2GRAY, 1);
    
    auto startTime = std::chrono::system_clock::now();
    
    options.NumInterations = 10;
    //Options.MaxIterError = 0.015;
    
    search(model, searchimg, Si_Init, Si_Final, options);
    //dumpSi(&Si_Init);
    //dumpSi(&Si_Final);
    
    auto endTime = std::chrono::system_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    printf("Search time: %4.1lld\n ", time);
    
    return Si_Final;
}

#define WRITE_VIDEO		0

int pic_vid_main(Model& model, const char *dirName)
{
	char imgName[256];
	sprintf(imgName, "%s/franck_%05d.jpg", dirName, 0);
	
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

    #if WRITE_VIDEO
    CvVideoWriter *writer = cvCreateVideoWriter("out.avi",-1,25,cvSize(720,576),1);
    cvWriteFrame(writer,input);
    #endif
    
    FaceTracker tracker(model, input);
    
    cv::imshow(OutputWinName, input);
	
    /////////////////////////////////////
    // Now we are ready to do tracking.
    /////////////////////////////////////
    int i = 0;
    /* exit if user press 'ESC' */
    while(true)
	{
		i++;
		sprintf(imgName, "%s/franck_%05d.jpg", dirName, i);
        auto input = cv::imread(imgName);
        cv::Mat dispImage;
        input.copyTo(dispImage);
        
        auto& si = tracker.update(input);

		drawFaceShape(dispImage, si.xy);

        #if WRITE_VIDEO
        cvWriteFrame(writer,DispImage);
        #endif

        cv::imshow(OutputWinName, dispImage);

        if (i >= 4980) {
            break;
        }
        
        if (cv::waitKey(1) == 27) {
            break;
        }
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

	Model model;

    int ret = loadModel(xmlFileName, model);
    if(ret)
    {
    	printf("Cannot load %s...\n", xmlFileName);
    	exit(0);
    }

    /* create a window for the video */
    cv::namedWindow(OutputWinName);
    
    /*
     place the all-images directory
     contains franck_00000.jpg ~ franck_01999.jpg
     This can be get from:
        First 1000 images
        Second 1000 images
        http://www-prima.inrialpes.fr/FGnet/data/01-TalkingFace/talking_face.html
     */
    auto imagesDir = "/Users/ryohey/Desktop/all-images";
    
	pic_vid_main(model, imagesDir);
    
    cv::destroyAllWindows();

    return 0;
    
}



