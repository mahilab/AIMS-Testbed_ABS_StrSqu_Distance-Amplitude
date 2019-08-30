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

// includes relevant MEL libraries
#include <MEL/Daq/Quanser/QPid.hpp>


/***********************************************************
******************* GLOBAL VARIABLES ***********************
************************************************************/
const double	 kGearRatio_(388125 / 4693); 
const int		 kEncoderCounts_(1024 * 4); // counts per rotation (using quadrature encoding)
const int		 kDegreesToRotation_(360); // degress per rotation
const double	 kDegreesToCount_(kEncoderCounts_ * kGearRatio_ / kDegreesToRotation_);


/***********************************************************
****************** CLASS DECLARATION ***********************
************************************************************/
class MaxonMotor
{
private:
	// device variable
	char*	port_name_;
	DWORD	error_code_;
	byte	node_id_;
	void*	key_handle_;
	mel::QuanserEncoder::Channel	encoder_;

	// control parameter variables
	unsigned int desired_velocity_;
	unsigned int desired_acceleration_;
	unsigned int desired_deceleration_;

	// position variables
	double desired_position_;
	double actual_position_;

	// device connection functions
	void		 EnableControl();
	void		 DisableControl();

	// movement functions
	void		 Halt();

public:
	// constructor
	MaxonMotor(mel::QuanserEncoder::Channel encoder);
	~MaxonMotor();

	// device connection functions
	void	Start();
	void	End();

	// device parameter functions
	void	SetPort(char* port);
	void	SetControlParam(unsigned int desired_velocity, 
							unsigned int desired_acceleration, 
							unsigned int desired_deceleration);

	// movement functions
	void	Move(double desired_position);
	void	GetPosition(double& position);
	BOOL	TargetReached();
};
#endif MAXONMOTOR