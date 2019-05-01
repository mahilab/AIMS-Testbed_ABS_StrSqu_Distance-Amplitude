/*
File: MaxonMotor.hpp
________________________________
Author(s): Zane Zook (gadzooks@rice.edu)
Edited By: Andrew Low (andrew.low@rice.edu)

This is the header file for the Maxon Motor class. This 
class holds all the lower level commands sent to the Maxon
controllers. This specific version is customized to work 
with the EPOS4 controller but it can be modified to work 
with other controllers
*/

#ifndef MAXONMOTOR
#define MAXONMOTOR

/***********************************************************
******************** LIBRARY IMPORT ************************
************************************************************/
// Maxon controller header file
#include "Definitions.h" 


/***********************************************************
******************* GLOBAL VARIABLES ***********************
************************************************************/
const long		 g_GEAR_RATIO = 388125 / 4693; 
const int		 g_ENCODER_COUNTS = 1024 * 4; // counts per rotation (using quadrature encoding)
const int		 g_DEGREES_ROTATION = 360; // degress per rotation
const long		 g_DEGREES_TO_COUNT = g_ENCODER_COUNTS * g_GEAR_RATIO / g_DEGREES_ROTATION;


/***********************************************************
****************** CLASS DECLARATION ***********************
************************************************************/
class MaxonMotor
{
private:
	// device variable
	char*		 portName;
	DWORD		 errorCode;
	byte		 nodeId;
	void*		 keyHandle;

	// control parameter variables
	unsigned int desVelocity;
	unsigned int desAcceleration;
	unsigned int desDeceleration;

	// data recorder variables
	//WORD		 samplePeriod; 
	//WORD		 samples;

	// device connection functions
	void		 enableControl();
	void		 disableControl();

	// device parameter functions
	void		 setControlParam();
	//void setRecorderParam();

	// movement functions
	void		 halt();

public:
	// constructor
	MaxonMotor();
	~MaxonMotor();

	// device connection functions
	void		 start();
	void		 end();

	// device parameter functions
	void		 setPort(char* port);
	void		 setControlParam(unsigned int desVel, unsigned int desAcc, unsigned int desDec);
	//void setRecorderParam(unsigned int desSampleFreq, unsigned int desSamples);

	// movement functions
	void		 move(double desPosition);
	void		 getPosition(long& position);
	BOOL		 targetReached();

	// data recorder functions
	/*void startRecord();
	void stopRecord();
	void outputData();*/
};
#endif MAXONMOTOR