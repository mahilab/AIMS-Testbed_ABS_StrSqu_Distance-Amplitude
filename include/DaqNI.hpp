/*
File: DaqNI.cpp
________________________________
Author(s): Zane Zook (gadzooks@rice.edu)

This file defines the DaqNI class which holds all the
lower level commands sent to the National Instruments DAQ
used for this experiment set. This specific version is
customized to work with the two ATI sensors hooked up
to the PCIe-6323 board connected to the two ATI Nano 25
sensors. Uses MEL's development ATIsensor class.
*/

#ifndef DAQNI
#define DAQNI

/***********************************************************
******************** LIBRARY IMPORT ************************
************************************************************/
// libraries for MEL
#include <MEL/Logging/DataLogger.hpp>
#include <MEL/Daq/Input.hpp>

// C libraries
#include "NIDAQmx.h"

// choosing mel namespace
using namespace mel;

/***********************************************************
****************** CLASS DECLARATION ***********************
************************************************************/
class DaqNI : public AnalogInput, NonCopyable
{
private:
	// member variables
	TaskHandle  taskHandle;	// creates a new taskHandle
	signed long error;
	signed long read;
	char        errBuff[2048] = { '\0' };

public:
	// constructor
	DaqNI();
	~DaqNI();

	// DAQ update functions
	bool update();
	bool update_channel(uint32 channel_number);
};
#endif DAQNI