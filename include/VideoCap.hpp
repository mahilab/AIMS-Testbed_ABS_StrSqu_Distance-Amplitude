/*
File: VideoCap.hpp
________________________________
Author(s): Joshua Fleck (jjf8@rice.edu)
Edited by: Zane Zook (gadzooks@rice.edu)

Declares a class to capture video from a webcam for haptic testbed.
Last Changed: 01/07/19
*/

#ifndef VIDEOCAP_H
#define VIDEOCAP_H

/***********************************************************
******************** LIBRARY IMPORT ************************
************************************************************/
#include <opencv2/opencv.hpp>
#include <atomic>
using namespace std;


/***********************************************************
****************** CLASS DECLARATION ***********************
************************************************************/
class VideoCap
{
private:
	string location; //where to save and the name of video
	int width; //width of video
	int height; //height of video
	int fps; //rate of video capture
	int fcc; //video codec
	cv::Size frameSize; //combined frames dimensions

	cv::VideoCapture cap; //video capture object from OpenCV
	cv::VideoWriter vid; //video writer object from OpenCV
	thread thr; //thread for running video capture

	volatile atomic_bool keepRunning; //communicates start/stop information

	void setupCapture(); //sets capture object properties
	void runCapture(); //operates video capture thread

public:
	VideoCap(); //default initialize
	VideoCap(int w, int h, int f); //initialize w/ specific video settings
	~VideoCap(); //destuctor

	bool beginCapture(string loc); //start capture
	bool endCapture(); //end capture
};
#endif