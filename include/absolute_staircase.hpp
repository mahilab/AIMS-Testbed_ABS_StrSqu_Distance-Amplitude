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
#include <MEL/Logging/Csv.hpp>
#include <MEL/Devices/Windows/Keyboard.hpp>

// other misc standard libraries
#include <random>


/***********************************************************
************************ CONSTANTS *************************
************************************************************/
const int 	kConditions_(9);
const int	kTrials_(1);
const int	kCrossoversRequired_(10);
const int 	kInterferenceLow_(26); //36
const int 	kInterferenceHigh_(52); //72
const int 	kZero_(0);
const int   kRangeMin_(0);
const std::array<double, kConditions_> kRangeMax_ = 
	{1,1,1,60,60,60,90,90,90};
const std::array<double, kConditions_> kInitialStepValues_ =
	{0.05,0.05,0.05,2,2,2,4,4,4};


/***********************************************************
****************** CLASS DECLARATION ***********************
************************************************************/
class Staircase
{
private:
    // private array variables
	std::array<std::array<double, kTrials_>, kConditions_> final_angles_; // array of arrays that hold angle positions from the method
	std::array<double, kCrossoversRequired_>  crossover_angles_;	
	std::array<std::string, kConditions_> condition_names_ = 
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
	std::array<int, kConditions_> conditions_ = { 0,1,2,3,4,5,6,7,8 };
		
    //  holds input keys for MEL
    std::vector<mel::Key> input_keys_ = 
	{ 
		mel::Key::Add, 		mel::Key::Up,
		mel::Key::Subtract,	mel::Key::Down,
		mel::Key::Comma,	mel::Key::Left,
		mel::Key::Period,	mel::Key::Right
	};

    // set all other relevant staircase method variables				
	double 	angle_,					previous_angle_,
			step_;
    int     condition_iterator_, 	condition_true_,
			trial_iterator_,		crossovers_;

    // random device variable
    std::random_device random_device_; // create random generator

    // initializer
	void 	ConditionInitialize();	
	void 	TrialInitialize();

	// private UI functions
	void WaitForKeyRelease(std::vector<mel::Key> keys);

public:
	// constructor
	Staircase();
	~Staircase();

    // read various names
	std::string     GetConditionName();
	std::string	    GetConditionName(int condition_num);

    // read various angle values
	double	GetAngle();
	double	GetInterferenceAngle();
	double	GetInterferenceAngle(int condition_num);
	void	GetTestPositions(std::array<std::array<double, 2>,2> &position_desired);
	
	// iterator control functions 
	bool	HasSettled(); 
	bool 	HasNextTrial();
	void	NextTrial();
	bool	HasNextCondition();
	void	NextCondition();
    bool    SetConditionNum(int condition_num);

    // UI functions   
    bool    ReadInput();

	// inport/export functions
	bool	ImportList(std::string filepath);
	void	ExportList(std::string filepath);
};
#endif