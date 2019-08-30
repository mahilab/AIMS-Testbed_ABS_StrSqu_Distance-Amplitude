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
const int 	kConditions_(4);
const int	kTrials_(1);
const int	kCrossoversRequired_(7);
const int 	kInterference_(52);
const int 	kZero_(0);
const int   kRangeMin_(0);
const std::array<double, kConditions_> kRangeMax_ = 
	{2,60,5,90};
const std::array<double, kConditions_> kInitialStepValues_ =
	{0.05,2,0.05,4};


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
		"Stretch",
		"Stretch_Squeeze",
		"Squeeze",
		"Squeeze_Stretch"
		}; // array of conditions
	std::array<int, kConditions_> conditions_ = { 0,1,2,3};
		
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