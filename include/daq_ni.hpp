/*
File: daq_ni.cpp
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
#include <MEL/Logging/Csv.hpp>
#include <MEL/Daq/Input.hpp>

// C libraries
#include "NIDAQmx.h"

/***********************************************************
****************** CLASS DECLARATION ***********************
************************************************************/
class DaqNI : public mel::AnalogInput, mel::NonCopyable
{
private:
	// member variables
	TaskHandle  task_handle_;	// creates a new task_handle_
	signed long error_;
	signed long read_;
	char        error_buffer_[2048] = { '\0' };

public:
	// constructor
	DaqNI();
	~DaqNI();

	// DAQ Update functions
	bool Update();
	bool UpdateChannel(mel::uint32 channel_number);
};
#endif DAQNI