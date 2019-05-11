/*
File: absolute_triallist.hpp
________________________________
Author(s): Joshua Fleck (jjf8@rice.edu)
Edited by: Zane Zook (gadzooks@rice.edu), Andrew Low (andrew.low@rice.edu)

Defines a class to randomize a list of trial conditions for
Absolute Threshold experiment.
*/

#ifndef ABSOLUTE_TRIALLIST
#define ABSOLUTE_TRIALLIST

/***********************************************************
******************** LIBRARY IMPORT ************************
************************************************************/
// other misc standard libraries
#include <random>
#include <array>

/***********************************************************
******************* GLOBAL VARIABLES ***********************
************************************************************/
const int	kNumberAngles_(7);
const int 	kNumberConditions_(9);
const int	kNumberTrials_(1);
const int 	kInterferenceAngleLow_(36);
const int 	kInterferenceAngleHigh_(72);
const int 	kZeroAngle_(0);
const std::array<double, kNumberAngles_> kDefaultAngles_ = 
	{24, 24, 24, 24, 24, 24, 24};
const std::array<double, kNumberAngles_> kStretchAngles_ = 
	{0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08};
const std::array<double, kNumberAngles_> kStretchAnglesLowInterference_ = 
	{18, 24, 30, 36, 42, 48, 54};	
const std::array<double, kNumberAngles_> kStretchAnglesHighInterference_ = 
	{24, 40, 56, 72, 88, 104, 120};

/***********************************************************
****************** CLASS DECLARATION ***********************
************************************************************/
class TrialList
{
private:		
	// private array variables
	std::array<std::array<double, kNumberAngles_ * kNumberTrials_>, kNumberConditions_> angles_; // array of arrays that hold angle positions
	std::array<std::string, kNumberConditions_> condition_names_ = 
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
		}; // array of conditions_
	std::array<int, kNumberConditions_> conditions_ = { 0,1,2,3,4,5,6,7,8 };
	
	// random device variable
	std::random_device random_device_; // create random generator
	
	// iterator variables
	int condition_iterator_; // iterators on arrays
	int angle_iterator_;

	// overloaded functions to directly access name information
	std::string	GetTrialName(int condition, int angle);
	double		GetAngleNumber(int condition, int angle);
	int		 	GetIterationNumber(int condition, int angle);
	void	 	GetTestPositions(std::array<std::array<double, 2>,2> &position_desired, int condition, int angle);

public:
	// constructor
	TrialList();
	~TrialList();

	// randomizer
	void 	scramble();

	// read various combinations names
	std::string  	GetTrialName();
	std::string  	GetConditionName();
	std::string		GetConditionName(int condition_number);
	std::string		GetComboNames(); // get full list of combination orderings

	// read various angle values
	double	GetAngleNumber();
	int		GetInterferenceAngle();
	int		GetInterferenceAngle(int condition_number);
	void	GetTestPositions(std::array<std::array<double, 2>,2> &position_desired);
	int		GetIterationNumber();

	// control iterator positions
	void	NextAngle();
	void	PrevAngle();
	bool	HasNextAngle();
	void	NextCondition();
	void	PrevCondition();
	bool	HasNextCondition();
	void	SetCombo(int iteration, int angle);

	// get iterator positions
	int		GetConditionNum();
	int		GetCurrentAngle();	

	// inport/export functions
	bool	ImportList(std::string filepath);
	void	ExportList(std::string filepath, bool timestamp);
};
#endif