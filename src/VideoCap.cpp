/*
File: VideoCap.cpp
________________________________
Author(s): Joshua Fleck (jjf8@rice.edu)
Edited by: Zane Zook (gadzooks@rice.edu)

Defines a class to capture video from a webcam for haptic testbed.
Last Changed: 01/07/19
*/


/***********************************************************
******************** LIBRARY IMPORT ************************
************************************************************/
#include <thread>
#include "VideoCap.hpp"


/***********************************************************
********************** CONSTRUCTOR *************************
************************************************************/
/*
Constructor for the VideoCap class, using default parameters
*/
VideoCap::VideoCap()
{
	cap = cv::VideoCapture(0);
	cap.open(cv::CAP_DSHOW);
	width = int(cap.get(cv::CAP_PROP_FRAME_WIDTH));
	height = int(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
	fps = int(cap.get(cv::CAP_PROP_FPS));
	setupCapture();
}

/*
Constructor for the VideoCap class, using custom parameters
*/
VideoCap::VideoCap(int w, int h, int f)
{
	cap = cv::VideoCapture(0);
	cap.open(cv::CAP_DSHOW);
	width = w;
	height = h;
	fps = f;
	setupCapture();
}

/*
Destructor for the VideoCap class
*/
VideoCap::~VideoCap()
{
	cap.release();
	cap.~VideoCapture();
}


/***********************************************************
******************* PUBLIC FUNCTIONS **********************
************************************************************/
/*
Opens and runs video capture
Returns 0 on success, -1 on failure
*/
bool VideoCap::beginCapture(string loc)
{
	location = loc;
	if (cap.isOpened()) {
		int f = int(cap.get(cv::CAP_PROP_FPS));
		vid = cv::VideoWriter(location, fcc, fps, frameSize, true);
		vid.open(location, fcc, fps, frameSize, true);
		keepRunning.store(true);
		thr = thread(&VideoCap::runCapture, this);
		return true;
	}
	else {
		return false;  //oh no
	}
}

/*
Ends capture and joins thread
Returns 0 on success, -1 on failure
*/
bool VideoCap::endCapture()
{
	keepRunning.store(false); //sets control bool false; thread should exit loop
	if (thr.joinable()) { //join thread for safe close
		thr.join();
		return true;
	}
	else {
		return false; //freak out if it doesn't work
	}
}


/***********************************************************
******************* PRIVATE FUNCTIONS **********************
************************************************************/

/*
Sets camera capture object properties
*/
void VideoCap::setupCapture() {
	fcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
	cap.set(cv::CAP_PROP_FOURCC, fcc);
	cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
	cap.set(cv::CAP_PROP_FPS, fps);
	cap.set(cv::CAP_PROP_AUTOFOCUS, 0);
	frameSize = cv::Size(width, height);
	cv::Mat junkFrame;
	cap.read(junkFrame);
}

/*
Operates thread for video capture
*/
void VideoCap::runCapture() {
	while (keepRunning.load())
	{
		cv::Mat frame;
		if (cap.read(frame)) //new frame from camera
		{
			vid.write(frame); //write to video
		}
	}
	vid.release();
	vid.~VideoWriter();
}