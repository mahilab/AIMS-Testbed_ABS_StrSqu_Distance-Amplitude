/*
File: absolute_staircase.hpp
________________________________
Author(s): Zane Zook (gadzooks@rice.edu)

Defines a class to run the staircase method to determine
absolute threshold of detection.
Requires MEL to build.
*/

#ifndef ABSOLUTE_STAIRCASE
#define ABSOLUTE_STAIRCASE

/***********************************************************
******************** LIBRARY IMPORT ************************
************************************************************/
// MEL Libraries
#include <MEL/Core/Console.hpp>
#include <MEL/Devices/Windows/Keyboard.hpp>

// other misc standard libraries
#include <random>


/***********************************************************
******************* GLOBAL VARIABLES ***********************
************************************************************/
const int 	kNumberCondtions_(9);
const int	kNumberTrials_(1);
const int 	kInterferenceAngleLow_(36);
const int 	kInterferenceAngleHigh_(72);
const int 	kZeroAngle_(0);
const int   kRangeMin_(0);
const int   kRangeMax_(60);


/***********************************************************
****************** CLASS DECLARATION ***********************
************************************************************/
class Staircase
{
private:
    // private array variables
	std::array<std::string, kNumberCondtions_> condition_names_ = 
		{
		"Stretch_None_Min",
		"Stretch_None_Mid",
		"Stretch_None_Max",
		"StretchXSqueeze_Low_Min",
		"StretchXSqueeze_Low_Mid",
		"StretchXSqueeze_Low_Max",
		"StretchXSqueeze_High_Min",
		"StretchXSqueeze_High_Mid",
		"StretchXSqueeze_High_Max"
		}; // array of conditions
	std::array<int, kNumberCondtions_> conditions_ = { 0,1,2,3,4,5,6,7,8 };
	
    //  holds input keys for MEL
    std::vector<mel::Key> input_keys_ = 
	{ 
		mel::Key::Add, 		mel::Key::Up,
		mel::Key::Subtract,	mel::Key::Down,
		mel::Key::Comma,	mel::Key::Left,
		mel::Key::Period,	mel::Key::Right,
		mel::Key::LControl,	mel::Key::RControl
	};

    // set all relevant staircase method variables
    double  angle_, previous_angle_, step_;
    int     condition_iterator_, condition_true_, crossovers_;

    // random device variable
    std::random_device random_device_; // create random generator

    // initializer
	void 	TrialInitialize();

public:
	// constructor
	Staircase();
	~Staircase();

    // read various names
	std::string     GetTrialName();
	std::string     GetConditionName();
	std::string	    GetConditionName(int condition_num);

    // read various angle values
	double	GetAngle();
	double	GetInterferenceAngle();
	double	GetInterferenceAngle(int condition_num);
	void	GetTestPositions(std::array<std::array<double, 2>,2> &position_desired);
	
	// condition control functions 
	bool	NextCondition();
    bool    SetConditionNum(int condition_num);

    // UI functions    
    bool    ReadInput();

	// inport/export functions
	// bool	importList(string filepath);
	// void	exportList(string filepath, bool timestamp);
};
#endif