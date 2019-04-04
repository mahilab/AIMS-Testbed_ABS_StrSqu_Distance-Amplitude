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


/***********************************************************
******************** LIBRARY IMPORT ************************
************************************************************/
// class header file
#include "DaqNI.hpp"

// libraries for MEL
#include <MEL/Core/Console.hpp>


/***********************************************************
********************** CONSTRUCTOR *************************
************************************************************/
/*
Constructor for the DaqNI class
 */
DaqNI::DaqNI()
{
	// set channel numbers to be used
	set_channel_numbers({ 0,1,2,3,4,5,16,17,18,19,20,21 });
	// initialize variables here
	if (DAQmxCreateTask("", &taskHandle) < 0)
		print("Failed to create task...");
	// creates analog input task from the DAQ
	if (DAQmxCreateAIVoltageChan(taskHandle, "Dev1/ai0:5,Dev1/ai16:21", "", DAQmx_Val_Diff, -10.0, 10.0, DAQmx_Val_Volts, NULL) < 0)
		print("Failed to create channel...");
	// start the task
	if (DAQmxStartTask(taskHandle) < 0)
		print("Failed to start task...");
}

/*
Destructor for the DaqNI class
 */
DaqNI::~DaqNI()
{
	// clears memory used by task
	DAQmxStopTask(taskHandle);
	DAQmxClearTask(taskHandle);
}


/***********************************************************
******************* PUBLIC FUNCTIONS **********************
************************************************************/

/*
Updates all channels of the daq simultaneously
 */
bool DaqNI::update()
{
	if (DAQmxReadAnalogF64(taskHandle, 1, 10.0, DAQmx_Val_GroupByScanNumber, &values_.get()[0], 12, &read, NULL) < 0)
		return false;
	else
		return true;
}

/*
Virtually updates a single channel. Only included to 
compile code correctly with MEL
*/
bool DaqNI::update_channel(uint32 channel_number) 
{
	return update();
}