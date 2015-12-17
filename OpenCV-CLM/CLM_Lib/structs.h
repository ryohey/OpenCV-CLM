//
//  structs.h
//  OpenCV-CLM
//
//  Created by ryohey on 2015/12/17.
//
//

#ifndef structs_h
#define structs_h

#include "config.h"

namespace CLM {
    
    ////////////////////////
    // Data structures
    ////////////////////////
    struct ShapeModel
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
    };
    
    struct PatchModel
    {
        int NumPatches;
        int PatchSize[2];
        
        //CvMat* weights;
        CvMat* WeightMats[100];
    };
    
    struct SVMWorkSpace
    {
        float *pAdat;		// Data buffer for A in SVMSearch.
        float *prLDat;	// Data buffer for rL in SVMSearch.
        float *pxcoordDat;	// Data buffer for xcoord in SVMSearch.
        float *pycoordDat;	// Data buffer for ycoord in SVMSearch.
        
        float *pnrDat;		// Data buffer for nr in SVMSearch
    };
    
    
    struct Model
    {
        ShapeModel 		ShapeModel;
        PatchModel 	PatchModel;
        
        // Temporary working data:
        SVMWorkSpace SvmWorkSpace[MAX_NUM_THREADS];
        
    };
    
    struct Si
    {
        CvMat* xy;
        CvMat* AlignedXY;	// x, y aligned to mean shape.
        float transform[4]; //transform matrix: [c s dx; s -c dy] stored in [c s dx dy].
    };
    
    struct Options
    {
        int FaceSize[2];
        int verbose;
        int SearchRegion[2];	// Search region size.
        
        int NumInterations;
        
        float MaxIterError;		// Iteration error before stop.
        
        int PatchSize[2];
        
    };
}

#endif /* structs_h */
