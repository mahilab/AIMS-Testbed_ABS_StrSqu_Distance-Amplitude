/*
File: maxon_motor.cpp
________________________________
Author(s): Zane Zook (gadzooks@rice.edu)

This file defines the Maxon Motor class which holds all the
lower level commands sent to the Maxon controllers in the 
system. This specific version is customized to work with 
the EPOS4 controller but it can be modified to work with 
other controllers
*/


/***********************************************************
******************** LIBRARY IMPORT ************************
************************************************************/
// libraries for Maxon Motor Class
#include "maxon_motor.hpp"

// other misc standard libraries
#include <iostream>

/***********************************************************
********************** CONSTRUCTOR *************************
************************************************************/
/*
Constructor for the maxonMotor class
 */
MaxonMotor::MaxonMotor(mel::QuanserEncoder::Channel encoder) :
	encoder_(encoder)
{
	// initializing controller information
	port_name_ = (char*)"USB0";
	node_id_ =	1;
	error_code_ = 0;
	key_handle_ = 0;

	// sets encoder to be used to read position values
	encoder_.zero(); // zeroes the encoder at the beginning of the experiment

	// initializing motor control parameters
	desired_velocity_ = 10000;
	desired_acceleration_ = 100000;
	desired_deceleration_ = 100000;
}

/*
Destructor for the maxonMotor class
 */
MaxonMotor::~MaxonMotor()
{
}


/***********************************************************
******************* PRIVATE FUNCTIONS **********************
************************************************************/
/*
Once the device has been opened, attempts to set the
controller into position control mode.
 */
void MaxonMotor::EnableControl()
{
	BOOL in_fault = FALSE;

	// checks the controller for any faults
	if (VCS_GetFaultState(key_handle_, node_id_, &in_fault, &error_code_))
	{
		// attempts to clear the fault from the controller if in a fault
		if (in_fault && !VCS_ClearFault(key_handle_, node_id_, &error_code_))
		{
			std::cout << "Clear fault failed!, error code = " << error_code_ << std::endl;
			return;
		}

		BOOL enabled = FALSE;

		// attempts to enable controller
		if (VCS_GetEnableState(key_handle_, node_id_, &enabled, &error_code_))
		{
			if (!enabled && !VCS_SetEnableState(key_handle_, node_id_, &error_code_))
			{
				std::cout << "Set enable state failed!, error code = " << error_code_ << std::endl;
			}
			else
			{
				std::cout << "Set enable state succeeded!" << std::endl;
			}
		}
	}
	else
	{
		std::cout << "Get fault state failed!, error code = " << error_code_ << std::endl;
	}

	// attempts to set controller to position control mode
	if (!VCS_ActivateProfilePositionMode(key_handle_, node_id_, &error_code_))
	{
		std::cout << "Activate profile position mode failed!" << std::endl;
	}
}

/*
Turns off the position control on the controller
 */
void MaxonMotor::DisableControl()
{
	BOOL in_fault = FALSE;

	// checks the controller for any faults
	if (VCS_GetFaultState(key_handle_, node_id_, &in_fault, &error_code_))
	{
		// attempts to clear the fault from the controller if in a fault
		if (in_fault && !VCS_ClearFault(key_handle_, node_id_, &error_code_))
		{
			std::cout << "Clear fault failed!, error code = " << error_code_ << std::endl;
			return;
		}

		BOOL enabled = FALSE;

		// attempts to disable controller
		if (VCS_GetEnableState(key_handle_, node_id_, &enabled, &error_code_))
		{
			if (enabled && !VCS_SetDisableState(key_handle_, node_id_, &error_code_))
			{
				std::cout << "Set disable state failed!, error code = " << error_code_ << std::endl;
			}
			else
			{
				std::cout << "Set disable state succeeded!" << std::endl;
			}
		}
	}
	else
	{
		std::cout << "Get fault state failed!, error code = " << error_code_ << std::endl;
	}
}


/***********************************************************
************* DEVICE CONNECTION FUNCTIONS ******************
************************************************************/
/*
Opens communication from the computer to the specific 
controller being referenced through USB comms.
 */
void MaxonMotor::Start()
{
	// Configuring EPOS4 for motor control
	char device_name[] =	"EPOS4";
	char protocol_name[] =	"MAXON SERIAL V2";
	char interface_name[] =	"USB";

	// Opens device communication
	key_handle_ = VCS_OpenDevice(device_name, protocol_name, interface_name, port_name_, &error_code_);
	if (key_handle_ == 0)
	{
		std::cout << "Open device failure, error code = " << error_code_ << std::endl;
	}
	else
	{
		std::cout << "Open device success!" << std::endl;
	}

	// Enables device in position control mode
	EnableControl();
}

/*
Closes communication from the computer to the specific
controller being referenced through USB comms.
 */
void MaxonMotor::End()
{
	// turns off position control
	DisableControl();

	std::cout << "Closing Device!" << std::endl;

	// closes communication with controller
	if (key_handle_ != 0)
	{
		VCS_CloseDevice(key_handle_, &error_code_);
	}
	VCS_CloseAllDevices(&error_code_);
}


/***********************************************************
************** DEVICE PARAMETER FUNCTIONS ******************
************************************************************/
/*
Indicates what USB port the MAXON motor controller is 
connected to.
 */
void MaxonMotor::SetPort(char* port)
{
	port_name_ = port;
}

/*
Sets each one of the control parameters for position 
control mode
 */
void MaxonMotor::SetControlParam(
	unsigned int desired_velocity, 
	unsigned int desired_acceleration, 
	unsigned int desired_deceleration)
{
	desired_velocity_ =		desired_velocity;
	desired_acceleration_ =	desired_acceleration;
	desired_deceleration_ =	desired_deceleration;
	
	// sets the controller's control parameters
	// SetControlParam();
}


/***********************************************************
****************** MOVEMENT FUNCTIONS **********************
************************************************************/

/*
Commands motor controller to move motor to specified position
 */
void MaxonMotor::Move(double desired_position)
{
	BOOL absolute_flag =	TRUE; 
	BOOL immediate_flag =	TRUE;

	// convert from degrees to encoder counts
	desired_position_ = desired_position * kDegreesToCount_;

	// sends signal to move Maxon motor to specified position
	if (!VCS_MoveToPosition(key_handle_, node_id_, (long)desired_position_, absolute_flag, immediate_flag, &error_code_)) 
	{
		std::cout << "Move to position failed!, error code = " << error_code_ << std::endl;
		Halt();
	}
}

/*
Pings motor for its current position
 */
void MaxonMotor::GetPosition(double& position)
{
	// attempts to acquire current position of the motor
	actual_position_ = encoder_.get_value();

	// convert from encoder counts to degrees
	position = actual_position_ / kDegreesToCount_;
}

/*
Pings motor to stop
 */
void MaxonMotor::Halt()
{
	// attempts to stop motor in its place
	if (!VCS_HaltPositionMovement(key_handle_, node_id_, &error_code_))
	{
		std::cout << "Halt position movement failed!, error code = " << error_code_ << std::endl;
	}
}

/*
Checks to see if the motor is still moving or if it has
reached its final destination
*/
BOOL MaxonMotor::TargetReached()
{
	BOOL target_reached = FALSE;

	// update the position of the encoder
	actual_position_ = encoder_.get_value();
	
	// set limits for the motor reaching its desired target
	int small_limit = 3;
	int large_limit = 100;

	// if within 4 encoder clicks of the desired position end data logging
	// switches the fidelity of the switch based on if it is mlarge or small
	if(desired_position_ < 1000) {
		if(abs(actual_position_ - desired_position_) <= small_limit)
			target_reached = TRUE;
	}
	else if(desired_position_ >= 1000) {
		if(abs(actual_position_ - desired_position_) <= large_limit)
			target_reached = TRUE;
	}

	
	
	return target_reached;
}