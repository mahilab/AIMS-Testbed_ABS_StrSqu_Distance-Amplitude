/*
File: abs_Staircase.hpp
________________________________
Author(s): Zane Zook (gadzooks@rice.edu)

Defines a class to run the staircase method to determine
absolute threshold of detection.
Requires MEL to build.
*/

#ifndef ABS_STAIRCASE
#define ABS_STAIRCASE

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
const int 	NUMBER_CONDITIONS(9);
const int	NUMBER_TRIALS(1);
const int 	INTERFERENCE_ANGLE_LOW(36);
const int 	INTERFERENCE_ANGLE_HIGH(72);
const int 	ZERO_ANGLE(0);
const int   RANGE_MIN(0);
const int   RANGE_MAX(60);


/***********************************************************
****************** CLASS DECLARATION ***********************
************************************************************/
class Staircase
{
private:
    // private array variables
	std::string condition_names[NUMBER_CONDITIONS] = 
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
	int condition[NUMBER_CONDITIONS] = { 0,1,2,3,4,5,6,7,8 };
	
    //  holds input keys for MEL
    std::vector<mel::Key> inputKeys = 
	{ 
		mel::Key::Add, 		mel::Key::Up,
		mel::Key::Subtract,	mel::Key::Down,
		mel::Key::Comma,	mel::Key::Left,
		mel::Key::Period,	mel::Key::Right,
		mel::Key::LControl,	mel::Key::RControl
	};

    // set all relevant staircase method variables
    double  angle, prevAngle, step;
    int     condItr, condTrue, crossovers;

    // random device variable
    std::random_device rd; // create random generator

    // initializer
	void 	trialInit();

public:
	// constructor
	Staircase();
	~Staircase();

    // read various names
	std::string     getTrialName();
	std::string     getConditionName();
	std::string	    getConditionName(int condNum);

    // read various angle values
	double	getAngle();
	double	getInterferenceAngle();
	double	getInterferenceAngle(int condNum);
	void	getTestPositions(std::array<std::array<double, 2>,2> &posDes);
	
	// condition control functions 
	bool	nextCondition();
    bool    setCondNum(int condNum);

    // UI functions    
    bool    readInput();

	// inport/export functions
	// bool	importList(string filepath);
	// void	exportList(string filepath, bool timestamp);
};
#endif