#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include "v4ldevice.h"
#include <utmpx.h>
#include <unistd.h>

using namespace std;
using namespace cv;

void ShowImage(char* Name, IplImage* Img, int Attribute)
{
    cvNamedWindow(Name, Attribute);
    cvShowImage(Name, Img);
    cvWaitKey(0);
    cvDestroyWindow(Name);
}

int main ()
{
    pid_t fpid;
    fpid = fork(); 
    IplImage* pOpenCVImage;
    IplImage* pROIImg; 
    IplImage* pShwImg;
    CvSize ImageSize;
    CvRect rect;
    CvMemStorage* m_storage = NULL;
    CvSeq * m_contour=0;
    
    unsigned char* ImageBuffer = NULL;
    int wKey = -1;
    int edgeThresh = 50;

// camera imagesize
    ImageSize.width = 1280;
    ImageSize.height = 720;
    
    cvNamedWindow((char*)"after_processing", CV_WINDOW_AUTOSIZE);

    printf("Program started\n");
    
// camera device (ls /dev)
    if(fpid != 0) {
		
// roi size
    rect.x = 0;
    rect.y= 500;
    rect.width = 1280;
    rect.height = 100;

// open V4L2 device
    open_device((char*)"/dev/video0");
    }
    
    else {
// roi size
    rect.x = 0;
    rect.y= 420;
    rect.width = 1280;
    rect.height = 80;
        
// open another v4l2 device
    open_device((char*)"/dev/video1");
   }
    
// create image
    pOpenCVImage = cvCreateImage(ImageSize , IPL_DEPTH_8U, 1 ); // Grayscale
    pROIImg = cvCreateImage(cvSize(rect.width, rect.height) , IPL_DEPTH_8U, 1 ); //ROI
    pShwImg = cvCreateImage(cvSize(rect.width, rect.height) , 8, 3);

// init v4l2 device
    init_device(ImageSize.width, ImageSize.height);
    printf("Start capturing\n");
    start_capturing();

    while (wKey == -1 )
    {
        ImageBuffer = snapFrame();
        if (ImageBuffer != NULL ) {
            memcpy( pOpenCVImage->imageData, ImageBuffer, pOpenCVImage->imageSize);
            cvSetImageROI(pOpenCVImage, rect);
   	        cvCopy(pOpenCVImage, pROIImg);
	        cvResetImageROI(pOpenCVImage);
// image processing
            cvThreshold(pROIImg, pROIImg,100,255,CV_THRESH_BINARY);
	        cvCanny(pROIImg, pROIImg, edgeThresh, edgeThresh*3, 3);
	        m_storage = cvCreateMemStorage(0);
	        cvFindContours(pROIImg, m_storage,
                           &m_contour, sizeof(CvContour),
                           CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE,
                           cvPoint(0,0));
// mark contour
			cvZero(pShwImg);  
			int cnt = 0;
			int defect = 0;    
			for (; m_contour != 0; m_contour = m_contour->h_next) {
                cnt++;
				CvScalar color = CV_RGB(255, 255, 255);
				cvDrawContours(pShwImg, m_contour, color, color, 0, 2, CV_FILLED, cvPoint(0, 0));    
				CvRect rect = cvBoundingRect(m_contour,0);
 
//judge contour area
			if (rect.width * rect.height > 10*1 && rect.width * rect.height < 1280*1 ) {
				cvRectangle(pShwImg, cvPoint(rect.x - 5, rect.y - 5),
                cvPoint(rect.x + rect.width + 5, rect.y + rect.height + 5),
                CV_RGB(255, 0, 0), 1, 8, 0);
// count defect
                defect++;
				} 
			}

// release memstorage
        cvReleaseMemStorage(&m_storage);
        m_storage = NULL;  

// show image
        cvShowImage( (char*)"after_processing", pShwImg);
            
// print defect counts
        if(fpid != 0)
            printf( "cam1 defect = %d\n", defect );
        else
	        printf( "cam2 defect = %d\n", defect );

// waitkey
        wKey = cvWaitKey(10);
        }
        
        else
        {
            printf("No image buffer retrieved.\n");
            break;
        }
    }
    
    cvDestroyWindow((char*)"after_processing");
    stop_capturing();
    uninit_device();
    close_device();
    
    printf("Program ended\n");

    return 0;

}
