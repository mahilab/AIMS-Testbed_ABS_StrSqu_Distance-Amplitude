/*
File: abs_TrialList.cpp
________________________________
Author(s): Joshua Fleck (jjf8@rice.edu)
Edited by: Zane Zook (gadzooks@rice.edu), Andrew Low (andrew.low@rice.edu)

Defines a class to randomize a list of trial conditions_ for
Absolute Threshold experiment.
*/

/***********************************************************
******************** LIBRARY IMPORT ************************
************************************************************/
// libraries for TrialList Class
#include "absolute_triallist.hpp"


/***********************************************************
********************** CONSTRUCTOR *************************
************************************************************/
/*
Constructor for the TrialList class
*/
TrialList::TrialList()
{
	// fill out trialArrs with angles
	for (int condition_num = 0; condition_num < kNumberConditions_; condition_num++)
	{
		for (int j = 0; j < kNumberAngles_*kNumberTrials_; j += kNumberAngles_) 
		{
			for (int k = 0; k < kNumberAngles_; k++) 
			{
				double next_angle;
				switch(condition_num)
				{
				case 0:
					next_angle = kStretchAngles_[k];
					break;
				case 1:
					next_angle = kStretchAnglesInterferenceLow_[k];
					break;
				case 2:
					next_angle = kStretchAnglesInterferenceMed_[k];
					break;
				case 3:
					next_angle = kStretchAnglesInterferenceHigh_[k];
					break;
				case 4:
					next_angle = kStretchAnglesInterferenceLow_[k];
					break;
				case 5:
					next_angle = kStretchAnglesInterferenceMed_[k];
					break;
				case 6:
					next_angle = kStretchAnglesInterferenceHigh_[k];
					break;
				case 7:
					next_angle = kStretchAnglesInterferenceLow_[k];
					break;
				case 8:
					next_angle = kStretchAnglesInterferenceMed_[k];
					break;
				case 9:
					next_angle = kStretchAnglesInterferenceHigh_[k];
					break;
				}			
				angles_[condition_num][j + k] = next_angle;
			}
		}
	}
}

/*
Destructor for the TrialList class
*/
TrialList::~TrialList()
{
}


/***********************************************************
******************* PRIVATE FUNCTIONS **********************
************************************************************/
/*
Outputs current condition and angle name
*/
std::string TrialList::GetTrialName(int condition, int angle)
{
	return GetConditionName() + "_" + std::to_string(GetAngleNumber(condition, angle));
}

/*
Outputs current angle number
*/
double TrialList::GetAngleNumber(int condition, int angle)
{
	return angles_[conditions_[condition]][angle];
}

/*
Outputs current angle combination as an int std::array. 
Current form outputs the angle for the stretch rocker
first and the squeeze band second. Also indicates 
if the reference angle should be given first or second
*/
void TrialList::GetTestPositions(std::array<std::array<double,2>,2> &position_desired, int condition, int angle)
{
	// determine the angles_ that will be used for the test for squeeze/stretch
	std::array<double, 2> test_positions;

	// generates test position std::array and test position std::array
	double interference_angle = GetInterferenceAngle();
	//if(condition == 0 || condition == 1)
	//{

	// then the condition is manipulating stretch with squeeze interference
	test_positions = { GetAngleNumber(condition_iterator_, angle), interference_angle };
	
		// mel::print("Stretch");
	//}
	/*
	else
	{
		// the condition is manipulating squeeze with stretch interference
		test_positions = { interference_angle, GetAngleNumber(condition_iterator_, angle) };	
		// mel::print("Squeeze");
	}
	*/
	// attach zero position for motors to return to after cue
	position_desired[0] = test_positions;
	position_desired[1] = { kZeroAngle_, kZeroAngle_ };
}

/*
Outputs current iteration number as an int
*/
int TrialList::GetIterationNumber(int condition, int angle)
{
	return (kNumberAngles_*kNumberTrials_*condition) + (angle + 1);
}


/***********************************************************
****************** RANDOMIZER FUNCTION *********************
************************************************************/
void TrialList::scramble()
{
	// create random range
	auto rng = std::default_random_engine{ random_device_() };

	// generate random ordering of angles_ in each of the conditions_
	for (int i = 0; i < kNumberConditions_; i++) {
		std::shuffle(angles_[i].begin(), angles_[i].end(), rng);
	}

	// generate random ordering of conditions_
	std::shuffle(conditions_.begin(), conditions_.end(), rng);
}


/***********************************************************
***************** TRIAL NAME FUNCTIONS *********************
************************************************************/
/*
Calls private function to get current condition and angle
names
*/
std::string TrialList::GetTrialName()
{
	return GetTrialName(condition_iterator_,angle_iterator_);
}

/*
Calls self to get current condition name
*/
std::string TrialList::GetConditionName()
{
	return GetConditionName(conditions_[condition_iterator_]);
}

/*
Overloads condition call to give name if condition is specified
*/
std::string TrialList::GetConditionName(int condition_num)
{
	return condition_names_[condition_num];
}

/*
Calls private function to get current angle number
*/
double TrialList::GetAngleNumber()
{
	return angles_[conditions_[condition_iterator_]][angle_iterator_];
}

/*
Gets the interference angle based on current condition
*/
int TrialList::GetInterferenceAngle()
{
	int condition_num = GetConditionNum();
	return GetInterferenceAngle(condition_num);		
}

/*
Overloads interference call to get the interference angle if condition is provided
*/
int TrialList::GetInterferenceAngle(int condition_num)
{
	if (condition_num == 9 || condition_num == 6 || condition_num == 3) return kInterferenceAngleHigh_;	
	else if (condition_num == 8 || condition_num == 5 || condition_num == 2) return kInterferenceAngleMed_;
	else if (condition_num == 7 || condition_num == 4 || condition_num == 1) return kInterferenceAngleLow_;
	else return kZeroAngle_;
}

/*
Outputs current angle combination as an int std::array.
Current form outputs the angle for the stretch rocker
first and the squeeze band second
*/
void TrialList::GetTestPositions(std::array<std::array<double,2>,2> &position_desired)
{
	GetTestPositions(position_desired, conditions_[condition_iterator_], angle_iterator_);
}

/*
Calls private function to get current iteration number
*/
int TrialList::GetIterationNumber()
{
	return GetIterationNumber(condition_iterator_, angle_iterator_);
}

/*
Calls private function to get current angle name
*/
std::string TrialList::GetComboNames()
{
	std::string combo_names = "";
	for (int i = 0; i < kNumberConditions_; i++) {
		std::array<double, kNumberAngles_*kNumberTrials_> local_array = angles_[i];
		for (int j = 0; j < kNumberAngles_*kNumberTrials_; j++) {
			combo_names += std::to_string(GetIterationNumber(i, j)) + ": " + GetTrialName(i, j) + "\n";
		}
	}
	return combo_names;
}


/***********************************************************
*************** ITERATOR CONTROL FUNCTIONS *****************
************************************************************/
/*
Changes angle index to reference the next trial. 
*/
void TrialList::NextAngle()
{
	if (angle_iterator_ == (kNumberAngles_*kNumberTrials_) - 1); // do nothing
	else angle_iterator_++;
}

/*
Changes angle index to reference the previous trial. 
*/
void TrialList::PrevAngle()
{
	if (angle_iterator_ == 0); // do nothing
	else angle_iterator_--;
}

/*
Checks if there is a next angle trial to run.
*/
bool TrialList::HasNextAngle()
{
	if (angle_iterator_ == (kNumberAngles_*kNumberTrials_) - 1) return false;
	else return true;
}

/*
Changes condition index to reference the next trial
*/
void TrialList::NextCondition()
{
	if (condition_iterator_ == (kNumberConditions_) - 1); // do nothing
	else {
		condition_iterator_++;
		angle_iterator_ = 0;
	}
}

/*
Changes condition index to reference the previous trial
*/
void TrialList::PrevCondition()
{
	if (condition_iterator_ == 0); // do nothing
	else condition_iterator_--;
}

/*
Checks if there is a next condition to run.
*/
bool TrialList::HasNextCondition()
{
	if (condition_iterator_ == (kNumberConditions_) - 1) return false;
	else return true;
}

/*
Changes indexes to reference a specific trial in the set
*/
void TrialList::SetCombo(int iteration, int angle)
{
	if (angle == (kNumberAngles_*kNumberTrials_))
	{
		condition_iterator_ = (iteration - (angle + 1)) / (kNumberAngles_*kNumberTrials_) + 1;
		angle_iterator_ = 0;
	}
	else
	{
		condition_iterator_ = (iteration - (angle + 1)) / (kNumberAngles_*kNumberTrials_);
		angle_iterator_ = angle;
	}
}


/***********************************************************
*************** ITERATOR RETURN FUNCTIONS ******************
************************************************************/
/*
Returns current number representing the condition being tested
*/
int TrialList::GetConditionNum()
{
	return conditions_[condition_iterator_];
}

/*
Returns current index for the angle
*/
int TrialList::GetAngleIndex()
{
	return angle_iterator_;
}


/***********************************************************
**************** IMPORT/EXPORT FUNCTIONS *******************
************************************************************/
/*
Imports trialList from a saved file
*/
bool TrialList::ImportList(std::string filepath)
{		
	// imports condition information from trialList file
	if(!mel::csv_read_row(filepath,conditions_,1,0)) return false;

	// // prints condition labels for debug
	// for (int j = 0; j < kNumberConditions_; j++)
	// {
	// 	mel::print_string(std::to_string(conditions_[j]) + ",");
	// }
	// mel::print(" ");

	// loads trialList file from import into class
	std::array<std::array<double, kNumberConditions_>, kNumberAngles_ * kNumberTrials_> output;
	if (!mel::csv_read_rows(filepath, output, 2, 0)) return false;
	
	for (int i = 0; i < kNumberAngles_ * kNumberTrials_; i++)
	{
		for (int j = 0; j < kNumberConditions_; j++)
		{
			angles_[j][i] = output[i][j];
			// print_string(to_string(angles_[j][i]) + ",");
		}			
		// print(" ");
	}
	return true;
}

/*
Exports trialList to a saved file
*/
void TrialList::ExportList(std::string filepath, bool timestamp)
{
	// create new data logger and prepare output trialList file
	const std::vector<std::string> kHeaderNames = 
	{ 
		"0=Str",
		"1=Str_SquLow_Close",
		"2=Str_SquMed_Med",
		"3=Str_SquHigh_Far",
		"4=Str_SquLow_Close",
		"5=Str_SquMed_Med",
		"6=Str_SquHigh_Far",
		"7=Str_SquLow_Close",
		"8=Str_SquMed_Med",
		"9=Str_SquHigh_Far"
	};
	mel::csv_write_row(filepath, kHeaderNames);

	// output order of conditions_ in current test
	std::vector<double> output_row = 
	{
		(double)conditions_[0], 
		(double)conditions_[1],
		(double)conditions_[2],
		(double)conditions_[3],
		(double)conditions_[4],
		(double)conditions_[5],
		(double)conditions_[6],
		(double)conditions_[7],
		(double)conditions_[8],
		(double)conditions_[9],
	};	
	mel::csv_append_row(filepath, output_row);

	// output order of all angle values in current test
	std::array<std::array<double, kNumberConditions_>, kNumberAngles_ * kNumberTrials_> output;
	for(int i=0; i < angles_.size(); i++) {
    	for(int j=0; j < angles_[i].size(); j++) {
    		output[j][i] = angles_[i][j];
   		}
 	}
	mel::csv_append_rows(filepath, output);

	// output order of all angle values in current test
	// for (int i = 0; i < kNumberAngles_*kNumberTrials_; i++)
	// {
	// 	output_row = { 
	// 		angles_[0][i], 
	// 		angles_[1][i],
	// 		angles_[2][i]
	// 		};
	// 	csv_append_row(filepath, output_row);
	// 	print_string(".");
	// }
	// creates space for next statement
	mel::print("");

	// final messages before end of program
	mel::print("TrialList successfuly exported!");

	// creates space for next statement
	mel::print("");
}