// main.cpp : Defines the entry point for the application.
//
// Background average sample code done with averages.
// 
// Files are: ch9_backgroundAVG.cpp 
//            ch9_AvgBackground.cpp and .h
// Resource: Learning OpenCV: Computer Vision with the OpenCV Library, Gary Bradski, 2008.

#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "ch9_AvgBackground.h"

void my_mouse_callback( int event, int x, int y, int flags, void* param );
int mX, mY;

void help() {
	printf("\nLearn background and find foreground using simple average and average difference learning method:\n"
		"\nUSAGE:\n  ch9_background startFrameCollection# endFrameCollection# [movie filename, else from camera]\n"
		"If from AVI, then optionally add HighAvg, LowAvg, HighCB_Y LowCB_Y HighCB_U LowCB_U HighCB_V LowCB_V\n\n"
		"***Keep the focus on the video windows, NOT the consol***\n\n"
		"INTERACTIVE PARAMETERS:\n"
		"\tESC,q,Q  - quit the program\n"
		"\th	- print this help\n"
		"\tp	- pause toggle\n"
		"\ts	- single step\n"
		"\tr	- run mode (single step off)\n"
		"=== AVG PARAMS ===\n"
		"\t-    - bump high threshold UP by 0.25\n"
		"\t=    - bump high threshold DOWN by 0.25\n"
		"\t[    - bump low threshold UP by 0.25\n"
		"\t]    - bump low threshold DOWN by 0.25\n"
		"=== CODEBOOK PARAMS ===\n"
		"\ty,u,v- only adjust channel 0(y) or 1(u) or 2(v) respectively\n"
		"\ta	- adjust all 3 channels at once\n"
		"\tb	- adjust both 2 and 3 at once\n"
		"\ti,o	- bump upper threshold up,down by 1\n"
		"\tk,l	- bump lower threshold up,down by 1\n"
		);
}

// Implement mouse callback
void my_mouse_callback( int event, int x, int y, int flags, void* param ){
	IplImage* image = (IplImage*) param;

	switch( event ){
		case CV_EVENT_MOUSEMOVE: 
			mX = x;
			mY = y;
			break;

		case CV_EVENT_LBUTTONDOWN:
			break;

		case CV_EVENT_LBUTTONUP:
			break;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
 	IplImage* rawImage = 0; 
    IplImage *ImaskAVG = 0,*ImaskAVGCC = 0;
    CvCapture* capture = 0;

	int startcapture = 1;
	int endcapture = 30;
	int c,n;

	float scalehigh = HIGH_SCALE_NUM;
	float scalelow = LOW_SCALE_NUM;
	
	//capture = cvCaptureFromCAM( 0 );
	printf("Capture from file %s\n","camera.avi");
	capture = cvCreateFileCapture( "camera.avi" );
	if(!capture) { printf("Couldn't open %ls\n",argv[3]); return -1;}

	//MAIN PROCESSING LOOP:
	bool pause = false;
	bool singlestep = false;

	//Rectangle to use to put around the people.
	CvRect bndRect = cvRect(0,0,0,0);

	//Points for the edges of the rectangle.
	CvPoint pt1, pt2;

	//The midpoint X position of the rectangle surrounding the moving objects.
	int avgX = 0;

	//Capture the movie frame by frame.
	int prevX = 0;

	//Create a font object.
	CvFont font;

    if( capture )
    {
		cvNamedWindow("Raw", 1 );
		cvNamedWindow("AVG_ConnectComp", 1);

		// Set up the callback
		cvSetMouseCallback("Raw", my_mouse_callback, NULL);

        int i = -1;
        
        for(;;)
        {
    			if(!pause){
				rawImage = cvQueryFrame( capture );
				++i;//count it
				if(!rawImage) 
					break;
			}
			if(singlestep){
				pause = true;
			}
			//First time:
			if(0 == i) {
				printf("\n . . . wait for it . . .\n"); //Just in case you wonder why the image is white at first
				//AVG METHOD ALLOCATION
				AllocateImages(rawImage);
				scaleHigh(scalehigh);
				scaleLow(scalelow);
				ImaskAVG = cvCreateImage( cvGetSize(rawImage), IPL_DEPTH_8U, 1 );
				ImaskAVGCC = cvCreateImage( cvGetSize(rawImage), IPL_DEPTH_8U, 1 );
				cvSet(ImaskAVG,cvScalar(255));
			}
			//If we've got an rawImage and are good to go:                
        	if( rawImage )
        	{
				//This is where we build our background model
				if( !pause && i >= startcapture && i < endcapture  ){
					//LEARNING THE AVERAGE AND AVG DIFF BACKGROUND
					accumulateBackground(rawImage);

				}
				//When done, create the background model
				if(i == endcapture){
					createModelsfromStats();
				}
				//Find the foreground if any
				if(i >= endcapture) {
					//FIND FOREGROUND BY AVG METHOD:
					backgroundDiff(rawImage,ImaskAVG);
					cvCopy(ImaskAVG,ImaskAVGCC);				
				}

				cvDilate( ImaskAVGCC, ImaskAVGCC, cvCreateStructuringElementEx( 9, 9, 4, 4, CV_SHAPE_ELLIPSE), 1 );
				cvErode( ImaskAVGCC, ImaskAVGCC, cvCreateStructuringElementEx( 9, 9, 4, 4, CV_SHAPE_ELLIPSE), 1 );

				cvErode( ImaskAVGCC, ImaskAVGCC, cvCreateStructuringElementEx( 3, 3, 1, 1, CV_SHAPE_ELLIPSE), 1 );
				cvDilate( ImaskAVGCC, ImaskAVGCC, cvCreateStructuringElementEx( 3, 3, 1, 1, CV_SHAPE_ELLIPSE), 1 );

				// find the contours
				CvMemStorage* storage = cvCreateMemStorage(0);
				CvSeq* contour = 0;
				cvFindContours( ImaskAVGCC, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
				
				mX = 0;
				mY = 0;

				for( ; contour != 0; contour = contour->h_next )
				{
					//Get a bounding rectangle around the moving object.
					bndRect = cvBoundingRect(contour, 0);

					pt1.x = bndRect.x;
					pt1.y = bndRect.y;
					pt2.x = bndRect.x + bndRect.width;
					pt2.y = bndRect.y + bndRect.height;

					//Get an average X position of the moving contour.
					avgX = (pt1.x + pt2.x) / 2;

					//Draw the bounding rectangle around the moving object.
					if(pt1.x > 20 && pt1.y > 70 && pt2.y - pt1.y > 25)
					{
						if(pt1.x >= 100 && pt2.x <= 150 && pt1.y >= 170 && pt2.y <= 240)
						{
							// do nothing
						}
						else {
							mX = pt1.x;
							mY = pt1.y;
							cvRectangle(rawImage, pt1, pt2, CV_RGB(255, 0, 0), 1);
						}
					}

					//Save the current X value to use as the previous in the next iteration.
					prevX = avgX; 

					CvScalar color = CV_RGB( (double)rand()/RAND_MAX*255, (double)rand()/RAND_MAX*255, (double)rand()/RAND_MAX*255);
					cvDrawContours( ImaskAVGCC, contour, color, color, 1, 1, 8 );
				}


				cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3, 0.3, 0, 1);
				char buffer [50];
				sprintf_s (buffer, "x = %d y = %d", mX, mY);

				cvPutText(rawImage, buffer, cvPoint(0, 10), &font, cvScalar(0, 0, 300));

				//Display
           		cvShowImage( "Raw", rawImage );
				cvShowImage( "AVG_ConnectComp",ImaskAVGCC);

				//USER INPUT:
	         	c = cvWaitKey(10)&0xFF;
				//End processing on ESC
				if (c == 27)
					break;
				//Else check for user input
				switch(c)
				{
					case 'h':
						help();
						break;
					case 'p':
						pause ^= 1;
						break;
					case 's':
						singlestep = 1;
						pause = false;
						break;
					case 'r':
						pause = false;
						singlestep = false;
						break;
					//AVG BACKROUND PARAMS
					case '-':
						if(i > endcapture){
							scalehigh += 0.25;
							printf("AVG scalehigh=%f\n",scalehigh);
							scaleHigh(scalehigh);
						}
						break;
					case '=':
						if(i > endcapture){
							scalehigh -= 0.25;
							printf("AVG scalehigh=%f\n",scalehigh);
							scaleHigh(scalehigh);
						}
						break;
					case '[':
						if(i > endcapture){
							scalelow += 0.25;
							printf("AVG scalelow=%f\n",scalelow);
							scaleLow(scalelow);
						}
						break;
					case ']':
						if(i > endcapture){
							scalelow -= 0.25;
							printf("AVG scalelow=%f\n",scalelow);
							scaleLow(scalelow);
						}
						break;			
				}
				
            }
		}		
		cvReleaseCapture( &capture );
		cvDestroyWindow( "Raw" );
		cvDestroyWindow( "AVG_ConnectComp");

		DeallocateImages();
		if(ImaskAVG) cvReleaseImage(&ImaskAVG);
		if(ImaskAVGCC) cvReleaseImage(&ImaskAVGCC);
    }
	else{ printf("\n\nDarn, Something wrong with the parameters\n\n"); help();
	}

    return 0;
}