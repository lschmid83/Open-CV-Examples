// main.cpp : 2D pong game controlled by moving red and blue objects in front of a web camera.
// Author: Lawrence Schmid
// Resources:
// - http://myopencv.wordpress.com/2009/06/13/detecting-colors-using-rgb-color-space/
// - http://sundararajana.blogspot.com/2007/05/motion-detection-using-opencv.html
// - Learning OpenCV: Computer Vision with the OpenCV Library, Gary Bradski, 2008.

#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include <iostream>
#include <fstream>

using namespace std;

CvPoint ballPoint, racketPoint, enemyPoint, ballSpeed; //Points for the racket, enemy, ball and ball speed
int numPlayers, playerScore, enemyScore, enemySpeed; //Number of players, player and opponent score count and 1 player game opponent speed
int racketMoveTo, enemyMoveTo; //The y coordinate of the red or blue object to move to
int redThreshold, blueThreshold, smooth; //The red and blue threshold for object detection and smooth movement to last detected position
bool gameStarted;

/**
 * Moves the ball along the x,y axis
 */
void moveBall() {
	ballPoint.x = (ballPoint.x + ballSpeed.x);
	ballPoint.y = (ballPoint.y + ballSpeed.y);
}

/** 
 * Moves the enemy based on the distance from the ball
 */
void moveEnemy() {
	if(numPlayers == 1)
	{
		int dist = abs(ballPoint.y - enemyPoint.y);
		if (ballSpeed.x > 0) {
			if (enemyPoint.y < (ballPoint.y - 3))
				enemyPoint.y = (enemyPoint.y + dist / 10) + enemySpeed;
			else if (enemyPoint.y > (ballPoint.y + 3))
				enemyPoint.y = (enemyPoint.y - dist / 10) - enemySpeed; 		
		}
	}
}

/** 
 * Check for collisions between the ball and opponent right paddle and return the ball in the opposite direction
 */
void checkEnemy() {
	if (ballPoint.x + ballSpeed.x + 6 >= enemyPoint.x)
	{
		if ((ballPoint.y + 7 > enemyPoint.y && ballPoint.y < (enemyPoint.y + 70))) {
			int racketHit = (ballPoint.y - (enemyPoint.y + 25));
			ballSpeed.y = (ballSpeed.y + (racketHit / 7));
			ballSpeed.x = (ballSpeed.x * -1);
			enemySpeed = rand() % (7 - 2) + 2;
		}
	}
}

/** 
 * Check for collisions between the ball and player one left paddle and return the ball in the opposite direction
 */
void checkRacket() {
	if ((ballPoint.x + ballSpeed.x) <= racketPoint.x + 6 && (ballPoint.x > racketPoint.x))
	{
		if ((ballPoint.y + 7 > racketPoint.y && ballPoint.y < (racketPoint.y + 70))) {
			int racketHit = (ballPoint.y - (racketPoint.y + 25));
			ballSpeed.y = (ballSpeed.y + (racketHit / 7));
			ballSpeed.x = (ballSpeed.x * -1);
			enemySpeed = rand() % (7 - 2) + 2;
		}
	}
}

/** 
 * Invoked when the ball goes out of play and hits the left or right hand wall. Adds one to player or opponent score 
 * depending on ball direction. Sets a random enemy speed, resets the ball to the centre of the screen and sets the
 * game started variable to false.
 */
void miss() {
	if (ballSpeed.x < 0)
		enemyScore++;
	else
		playerScore++;
	enemySpeed = rand() % (7 - 2) + 2;
	racketPoint.y = 205; enemyPoint.y = 205;
	ballPoint.x = 315; ballPoint.y = 235;
	ballSpeed.x = 0; ballSpeed.y = 0;
	gameStarted = false;
}

/** 
 * Check for collisions between the ball and walls. If the ball goes out of play invoke the miss() method. 
 * If the ball hits the top or bottom wall set the speed vector to the opposite direction.
 */
void checkWalls() {
	if ((ballPoint.x + ballSpeed.x) <= 0)
		miss();
	if ((ballPoint.x + ballSpeed.x) >= (640 - 20))
		miss();
	if ((ballPoint.y + ballSpeed.y) <= 0)
		ballSpeed.y = (ballSpeed.y * -1);
	if ((ballPoint.y + ballSpeed.y) >= (470))
		ballSpeed.y = (ballSpeed.y * -1);
}

/**
 * The main function
 */
int _tmain(int argc, _TCHAR* argv[])
{
	numPlayers = 0;
	playerScore = 0;
	enemyScore = 0;
	gameStarted = false;
	enemySpeed = rand() % (7 - 2) + 2;
	redThreshold = 45;
	blueThreshold = 45;
	smooth=1;

	//Detecting Colors using  RGB Color Space
	//http://myopencv.wordpress.com/2009/06/13/detecting-colors-using-rgb-color-space/
	IplImage* frame = 0;
	int i,j;
	int height,width,step,channels;
	int stepr, channelsr;
	uchar *imageData,*redData,*blueData,*gameData;
	IplImage *imageMask = 0;
	i=j=0;
	
	//Rectangle to use to put around the objects
	CvRect bndRect = cvRect(0,0,0,0);

	//Points for the edges of the rectangle.
	CvPoint pt1, pt2;

	//Init player paddle position
	racketPoint.x = 20; racketPoint.y = 205;
	racketMoveTo = racketPoint.y;

	//Init enemy paddle position
	enemyPoint.x = 608; enemyPoint.y = 205;
	enemyMoveTo = enemyPoint.y;

	//Init ball position
	ballPoint.x = 315; ballPoint.y = 235;

	//Read settings.ini
	int camera_id = 0;
	int showThresholdWindow = 0;
	ifstream fin("settings.ini");
	if(fin.is_open())
	{     
		//Camera id
		string line;     
		getline(fin,line); 

		//Reading an input file in C++
		//http://stackoverflow.com/questions/745897/reading-an-input-file-in-c 
		stringstream str(line);
		string text; 
		getline(str,text,'='); 	getline(str,text,'='); 
		//Convert a string to int
		//http://stackoverflow.com/questions/200090/how-do-you-convert-a-c-string-to-an-int
		stringstream(text) >> camera_id; 

		//Show threshold window
		getline(fin,line); 
		stringstream str2(line);
		getline(str2,text,'='); 	getline(str2,text,'='); 
		stringstream(text) >> showThresholdWindow; 
	}

	//Allocates CvCapture structure and  binds it to the video camera
	//http://www710.univ-lyon1.fr/~bouakaz/OpenCV-0.9.5/docs/ref/OpenCVRef_Highgui.htm
	CvCapture* capture = 0;
    capture = cvCaptureFromCAM(camera_id);

	if( !capture )
    {
        fprintf(stderr,"Could not initialize capturing...\n");
        return -1;
    }
	
	//Create camera, game and settings windows
    cvNamedWindow("camera", 1 );	
	cvMoveWindow("camera", 100, 100);
	cvNamedWindow("game", 1 );	
	cvMoveWindow("game", 460, 100);
	cvResizeWindow("game",640,480);
	cvNamedWindow("settings", 1 );
	cvResizeWindow("settings",320,180);
	cvMoveWindow("settings", 100, 400);

	//Trackbar - Learning OpenCV p.20 and p.101
	cvCreateTrackbar("Red Thld","settings", &redThreshold, 255, NULL);
	cvCreateTrackbar("Blue Thld","settings", &blueThreshold, 255, NULL);
	cvCreateTrackbar("Movement","settings", &smooth, 1, NULL);
	
	//Create red and blue threshold image result windows
	if(showThresholdWindow == 1)
	{
		cvNamedWindow( "red", 1 );
		cvMoveWindow("red", 100, 640);
		cvNamedWindow( "blue", 1 );
		cvMoveWindow("blue", 460, 640);
	}

	while(true)
    {
		checkRacket();
		checkEnemy();
		checkWalls();
		moveBall();
		moveEnemy();
		//---------------------------------------------------------------------------------------------------------

		//Grabs and returns a frame from camera or AVI
		//http://www710.univ-lyon1.fr/~bouakaz/OpenCV-0.9.5/docs/ref/OpenCVRef_Highgui.htm
		frame = cvQueryFrame( capture );
        if( !frame )
            break;

		//Create images to hold the game window and red and blue threshold image results
		IplImage *gameResult=cvCreateImage(cvSize(640, 480), 8, 1 );
		IplImage *redResult=cvCreateImage( cvGetSize(frame), 8, 1 );
		IplImage *blueResult=cvCreateImage( cvGetSize(frame), 8, 1 );

		//Red object color detection
		//http://myopencv.wordpress.com/2009/06/13/detecting-colors-using-rgb-color-space/
		//---------------------------------------------------------------------------------------------------------
		height = frame->height;  
		width = frame->width;  
		step =frame->widthStep; 
		channels = frame->nChannels;
		imageData = (uchar *)frame->imageData;
		
		/*Here I use the Suffix r to diffenrentiate the result data and the frame data
		Example :stepr denotes widthStep of the result IplImage and step is for frame IplImage */
		stepr=redResult->widthStep;
		channelsr=redResult->nChannels;
		redData = (uchar *)redResult->imageData;
		blueData = (uchar *)blueResult->imageData;
		gameData = (uchar *)gameResult->imageData;

		for(i=0;i < (height);i++) 
		{
			for(j=0;j <(width);j++)
			{
				//select pixels which are more red than any other colour if the difference between
				//the red - green and red - blue is greater than the threshold.
				if(imageData[i*step+j*channels+2] - imageData[i*step+j*channels] > redThreshold && 
				   imageData[i*step+j*channels+2] - imageData[i*step+j*channels+1] > redThreshold)
					redData[i*stepr+j*channelsr]=255;
				else
					redData[i*stepr+j*channelsr]=0;
			}		
		}
		//---------------------------------------------------------------------------------------------------------

		//Find red object contours and draw bounding box
		//http://sundararajana.blogspot.com/2007/05/motion-detection-using-opencv.html
		//---------------------------------------------------------------------------------------------------------
		imageMask = cvCreateImage( cvGetSize(redResult), IPL_DEPTH_8U, 1 );	
		cvCopy(redResult,imageMask);

		//Find the contours
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;
		cvFindContours( imageMask, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

		int percentY = 0;
		float mid = 0;
		float percent = 0;
		for( ; contour != 0; contour = contour->h_next )
		{
			//Get a bounding rectangle around the moving object.
			bndRect = cvBoundingRect(contour, 0);

			pt1.x = bndRect.x;
			pt1.y = bndRect.y;
			pt2.x = bndRect.x + bndRect.width;
			pt2.y = bndRect.y + bndRect.height;
		//-------------------------------------------------------------------------------------------------------

			//If the red object width and height is greater than 40 pixels
			if(pt2.x - pt1.x > 10 && pt2.y - pt1.y > 10 &&
			   pt2.x - pt1.x < 80 && pt2.y - pt1.y < 80)
			{
				cvRectangle(frame, pt1, pt2, CV_RGB(255,0,0), 1); //draw rectangle
				racketPoint.x = 20;
				//Calculate percentage of mid point of bounding rectangle in camera frame
				mid = pt1.y + ((pt2.y - pt1.y) / 2);
				percent = (mid / height); 
				racketMoveTo = (percent * 480) - 35; //Set the y coordinate in the game window the racket should move to				
			}
		}

		//http://stackoverflow.com/questions/5951292/how-do-you-delete-a-cvseq-in-opencv
		// Release memory 
		cvReleaseMemStorage(&storage); 

		//Move racket to the new y coordinate
		if(smooth == 0)
			racketPoint.y = racketMoveTo; 
		else //Move with velocity to the new y coordinate
		{
			if(racketPoint.y > racketMoveTo - 20 && racketPoint.y < racketMoveTo + 20)
				racketPoint.y = racketMoveTo;
			else if(racketPoint.y > racketMoveTo)
				racketPoint.y-=20;
			else if(racketPoint.y < racketMoveTo)
				racketPoint.y+=20;
		}

		//Blue object color detection
		//http://myopencv.wordpress.com/2009/06/13/detecting-colors-using-rgb-color-space/
		//---------------------------------------------------------------------------------------------------------
		for(i=0;i < (height);i++) 
		{
			for(j=0;j <(width);j++)
			{
				if(((imageData[i*step+j*channels]) > (blueThreshold+imageData[i*step+j*channels+2])) && 
				   ((imageData[i*step+j*channels]) > (blueThreshold+imageData[i*step+j*channels+1])))
					blueData[i*stepr+j*channelsr]=255;
				else
					blueData[i*stepr+j*channelsr]=0;
			}
		}
		//---------------------------------------------------------------------------------------------------------

		//Find blue object contours and draw bounding box
		//http://sundararajana.blogspot.com/2007/05/motion-detection-using-opencv.html
		//---------------------------------------------------------------------------------------------------------
		imageMask = cvCreateImage( cvGetSize(blueResult), IPL_DEPTH_8U, 1 );	
		cvCopy(blueResult,imageMask);

		//Find the contours
		storage = cvCreateMemStorage(0);
		contour = 0;
		cvFindContours( imageMask, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

		for( ; contour != 0; contour = contour->h_next )
		{
			//Get a bounding rectangle around the moving object.
			bndRect = cvBoundingRect(contour, 0);

			pt1.x = bndRect.x;
			pt1.y = bndRect.y;
			pt2.x = bndRect.x + bndRect.width;
			pt2.y = bndRect.y + bndRect.height;	
		//-------------------------------------------------------------------------------------------------------

			//If the blue object width and height is greater than 40 pixels
			if(pt2.x - pt1.x > 15 && pt2.y - pt1.y > 15 &&
			   pt2.x - pt1.x < 50 && pt2.y - pt1.y < 50)
			{
				cvRectangle(frame, pt1, pt2, CV_RGB(0,162,232), 1);

				if(numPlayers == 0 || numPlayers == 2)
				{
					enemyPoint.x = 608;
					//Calculate percentage of mid point of bounding rectangle in camera frame
					mid = pt1.y + ((pt2.y - pt1.y) / 2);
					percent = (mid / height);
					if(enemyPoint.y == enemyMoveTo)
						enemyMoveTo = (percent * 480) - 35;	 //Set the y coordinate in the game window the racket should move to	
				}
			}
		}

		//http://stackoverflow.com/questions/5951292/how-do-you-delete-a-cvseq-in-opencv
		// Release memory 
		cvReleaseMemStorage(&storage); 
		
		//Move enemy to the new y coordinate in 2 player mode
		if(numPlayers == 0 || numPlayers == 2) //Title screen or 2 player mode
		{
			if(smooth == 0)
				enemyPoint.y = enemyMoveTo;
			else //Move with velocity to the new y coordinate
			{
				if(enemyPoint.y > enemyMoveTo - 20 && enemyPoint.y < enemyMoveTo + 20)
					enemyPoint.y = enemyMoveTo;
				else if(enemyPoint.y > enemyMoveTo)
					enemyPoint.y-=20;
				else if(enemyPoint.y < enemyMoveTo)
					enemyPoint.y+=20;
			}
		}
		
		//Clear the game image
		cvSet(gameResult, cvScalar(0,0));

		//Draw the player paddles
		CvPoint racket;
		racket.x = racketPoint.x + 15; racket.y = racketPoint.y + 70;
		cvRectangle(gameResult, racketPoint, racket, CV_RGB(0,162,232), CV_FILLED);
		CvPoint enemy;
		enemy.x = enemyPoint.x + 15; enemy.y = enemyPoint.y + 70;
		cvRectangle(gameResult, enemyPoint, enemy, CV_RGB(0,162,232), CV_FILLED);

		//Draw the ball
		CvPoint ball;
		ball.x = ballPoint.x + 10; ball.y = ballPoint.y + 10;
		cvRectangle(gameResult, ballPoint, ball, CV_RGB(0,162,232), CV_FILLED);

		//Initialize font and draw game text (Learning OpenCV p.80)
		CvFont gameFont;
		cvInitFont(&gameFont, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);
		char buffer [50];
		sprintf_s (buffer, "%d", playerScore);
		cvPutText(gameResult, buffer, cvPoint(160, 45), &gameFont, cvScalar(255, 255, 255, 0));
		sprintf_s (buffer, "%d", enemyScore);
		cvPutText(gameResult, buffer, cvPoint(460, 45), &gameFont, cvScalar(255, 255, 255, 0));
		
		//Title screen
		if(numPlayers == 0) 
		{
			cvPutText(gameResult, "Camera Pong", cvPoint(265, 90), &gameFont, cvScalar(255, 255, 255, 0));
			cvPutText(gameResult, "Press 1 or 2 Player", cvPoint(242, 400), &gameFont, cvScalar(255, 255, 255, 0));
		}

		//Ball has gone out of bounds
		if(!gameStarted && numPlayers > 0) 
		{
			cvPutText(gameResult, "Press Space Bar to Start", cvPoint(220, 360), &gameFont, cvScalar(255, 255, 255, 0));
		}

		//Show camera input frame image window
        cvShowImage( "camera", frame);
		//Show the game image window 
		cvShowImage( "game", gameResult);
		if(showThresholdWindow == 1) //Show the red and blue threshold result images windows
		{
			cvShowImage( "red", redResult);
			cvShowImage( "blue", blueResult);
		}
		
		//Free the allocated memory for the game, red and blue threshold images
		cvReleaseImage(&gameResult);
		cvReleaseImage(&redResult);
		cvReleaseImage(&blueResult);

        //Wait for a number of milliseconds for a keystroke (Learning OpenCV p.18)
		int c = cvWaitKey(1);
        if( (char) c == 27 ) //Esc key
		{
			if(numPlayers == 0) //Title Screen
				break; //Exit game
			else //Return to title screen
			{
				numPlayers = 0;
				playerScore = 0; enemyScore = 0; //Reset score
				ballPoint.x = 315; ballPoint.y = 235;
				ballSpeed.x = 0; ballSpeed.y = 0;
			}
		}
		else if( (char) c == '1' || (char) c == '2' ) //1 or 2 keys
		{
			if(numPlayers == 0) //Title screen
			{
				playerScore = 0; enemyScore = 0; 
				if( (char) c == '1')
					numPlayers = 1;
				else
					numPlayers = 2;
				ballSpeed.x = 10; ballSpeed.y = 4;
				gameStarted = true;
			}
		}
		else if( (char) c == 32 ) //Space key
		{
			if(numPlayers > 0 && !gameStarted)
			{
				ballSpeed.x = 10; ballSpeed.y = 4;
				gameStarted = true;
			}
		}	
    }

	//Free the memory associated with the CvCapture structure (Learning OpenCV p.19)
    cvReleaseCapture( &capture );
    
	//Free the allocated memory for the camera, game, settings, red and blue threshold windows
	cvDestroyWindow("camera");
	cvDestroyWindow("game");
	cvDestroyWindow("settings");
	cvDestroyWindow("red");
	cvDestroyWindow("blue");
    return 0;
}

