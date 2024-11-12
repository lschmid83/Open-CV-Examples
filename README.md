# Open CV Examples

These are examples of computer vision work using Visual Studio, OpenCV and VC++.

* Object Detection
* Camera Pong

# Setting up OpenCV 2.1.0

Download the OpenCV 2.1.0 for Visual Studio Installer here:

[OpenCV-2.1.0](https://drive.google.com/file/d/1jSO7ugMoLimJsyJjdkMbSc36Cxw1sDCq/view?usp=sharing)

Install it to a folder, say "C:\OpenCV2.1\". This readme will refer to this path as $openCVDir.

If you want to create a new VC++ project in Visual Studio which uses the libraries you will need to:

1. Right click the Project Name in the Solution Explorer and select Properties
2. Select VC++ Directories
3. Click Include Directories -> Edit
   - Add "$openCVDir\include\opencv"
4. Click Library Directories -> Edit
   - Add "$openCVDir\lib"
5. Click Source Directories -> Edit -> Add
```
$openCVDir\src\cv
$openCVDir\src\cvaux
$openCVDir\src\cxcore
$openCVDir\src\highgui
```

If the project doesn't build and there are unresolved external errors go back and setup the paths again.

You may have to change the build target to x86 to build the project.

# Object Detection

This project simply demonstrates people tracking object detection in an .avi recorded video. The moving objects will be tracked with bounding boxes and the x,y coordinates in the video frame displayed.

First you will need to download the example .avi file here:

[camera.avi](https://drive.google.com/file/d/1431a-mrQY5g9dcfeTn_L41vZ0kRMB58S/view?usp=sharing)

Place this file in the source code folder.

Here is an example screenshot of the motion detection application running:

<img src='https://drive.google.com/uc?id=1q2FKvb97Oiunq8uggx57KrwAlfbjMix-' width='640'>

# Camera Pong

Camera Pong is a table tennis game controlled by detecting moving red and blue coloured objects in front of a webcam using computer vision algorithms.

The game involves hitting a ball back and forth across the screen using paddles which can be moved vertically across the screen. If the opponent fails to return the ball and it goes out of play the player who hit the ball earns a point.  

In one player mode the computer opponent will try and return the ball. The game also supports two players mode with one player controlling the left paddle with a red object and the second player controlling the right paddle with a blue object. 

Here is an example screenshot of the game:

<img src='https://drive.google.com/uc?id=1-Wat7diBBqBp2Jd95npxnjkPU_4WoMXe' width='640'>

# Issues

My webcam is not detected? 

This problem can occur if you have more than one imaging device installed. 

- Check your camera is connected to the computer and not in use in another program
- Edit settings.ini using Notepad and change the CameraID value from 0 to 1

The red or blue object is not detected by the camera?

- Lower the red or blue threshold value by moving the slider left 
- Use an object with a deeper red or blue colour
- Adjust the webcam focus and calibrate settings 

When you run the project make sure to target the x86 platform to avoid errors.

# Note

You may need to change the Build Configuration to Debug and x86 Platform for the project to build.










