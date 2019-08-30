/*
File: absolute_staircase.cpp
________________________________
Author(s): Zane Zook (gadzooks@rice.edu)

Defines a class to run the staircase method to determine
absolute threshold of detection.
Requires MEL to build.
*/

/***********************************************************
******************** LIBRARY IMPORT ************************
************************************************************/
// libraries for Staircase Class
#include "absolute_staircase.hpp"


/***********************************************************
********************** CONSTRUCTOR *************************
************************************************************/
/*
Constructor for the Staircase class
*/
Staircase::Staircase()
{
	// create random range
	auto rng = std::default_random_engine{ random_device_() };

	// generate random ordering of conditions_
	std::shuffle(conditions_.begin(), conditions_.end(), rng);

    // set starting condition to the front
    condition_iterator_ = 0; // condition iterator
    condition_true_ = conditions_[condition_iterator_]; // true condition

    // sets up for a new condition
    ConditionInitialize();
}

/*
Destructor for the Staircase class
*/
Staircase::~Staircase()
{
}

/***********************************************************
***************** INITIALIZE FUNCTIONS *********************
************************************************************/
/*
Resets values for next condition
*/
void Staircase::ConditionInitialize()
{
    // resets the trial counter and the step size
    trial_iterator_ = 0; // number of switches in method
	crossovers_ = 0; // number of switches in method

	// initializes a new trial
	TrialInitialize();
}

/*
Resets values for next trial
*/
void Staircase::TrialInitialize()
{
    // chooses random starting location in range 
	std::uniform_real_distribution<double> distribution(kRangeMin_, kRangeMax_[condition_true_]);
	angle_ = distribution(random_device_);  // generates start angle

	// loads initial step value for the condition
	step_ = kInitialStepValues_[condition_true_];
	previous_angle_ = angle_;
}


/***********************************************************
******************** NAME FUNCTIONS ************************
************************************************************/
/*
Returns the name for the current condition
*/
std::string Staircase::GetConditionName()
{
	return condition_names_[condition_true_];
}

/*
Reutrns the name for the indicated condition
*/
std::string Staircase::GetConditionName(int condition_num)
{
	return condition_names_[condition_num];
}

/***********************************************************
******************** ANGLE FUNCTIONS ***********************
************************************************************/
/*
Returns private angle number
*/
double Staircase::GetAngle()
{
	return angle_;
}

/*
Gets the interference angle based on current condition
*/
double Staircase::GetInterferenceAngle()
{
	return GetInterferenceAngle(condition_true_);		
}

/*
Overloads interference call to get the interference angle if condition is provided
*/
double Staircase::GetInterferenceAngle(int condition_num)
{
	if (condition_num % 2) return kInterference_;
	else return kZero_;		
}

/*
Outputs current angle_ combination as an int array.
Current form outputs the angle_ for the stretch rocker
first and the squeeze band second
*/
void Staircase::GetTestPositions(std::array<std::array<double,2>,2> &position_desired)
{
	// determine the angles that will be used for the test for squeeze/stretch
	std::array<double, 2> test_positions;

	// generates test position array and test position array
	if(condition_true_ > 1)
		test_positions = { GetInterferenceAngle(), angle_ };
	else
		test_positions = { angle_, GetInterferenceAngle() };		
	
	// attach zero position for motors to return to after cue
	position_desired[0] = test_positions;
	position_desired[1] = { kZero_, kZero_ };
}

/***********************************************************
*************** CONDITION CONTROL FUNCTIONS *****************
************************************************************/
/*
Simply checks to see if enough crossovers have occured to 
conclude that the user has settled on a value and saves it
*/
bool Staircase::HasSettled()
{
	if(crossovers_ >= kCrossoversRequired_ - 1)
	{
		double average;
		for(int i = 0; i < kCrossoversRequired_; i++){
			average += crossover_angles_[i];
		}
		average = average / kCrossoversRequired_;
		final_angles_[condition_true_][trial_iterator_] = average;
		return true;
	}
	else return false;
}

/*
Checks to see if the condition must repeat for another trial
*/
bool Staircase::HasNextTrial()
{
	if(trial_iterator_ >= kTrials_ - 1)
		return false;
	else
		return true;
}

/*
Moves to next trial
*/
void Staircase::NextTrial()
{
	trial_iterator_ ++;
	TrialInitialize();
}

/*
Indicates if staircase has a next condition
*/
bool Staircase::HasNextCondition()
{
	if (condition_iterator_ >= kConditions_ - 1)
        return false; // return false if no more conditions
	else {
        return true; // else returns true
	}
}

/*
Changes condition index to reference the next trial
*/
void Staircase::NextCondition()
{
	condition_iterator_++;
    condition_true_ = conditions_[condition_iterator_];
    ConditionInitialize();
}

/*
Sets the true condition number
*/
bool Staircase::SetConditionNum(int condition_num)
{
    // sets the method to specified condition if it is a
    // real condition
	if (condition_num >= 0 && condition_num < kConditions_)
    {
        condition_true_ = condition_num;
        ConditionInitialize();
        return true;
    }
    else return false;
}


/***********************************************************
************************ UI FUNCTIONS **********************
************************************************************/
/*
Pauses advancement of the program until the specified
keys are all released
*/
void Staircase::WaitForKeyRelease(std::vector<mel::Key> keys)
{
	bool pressed = true;
	while(pressed)
	{
		for(int i = 0; i < keys.size(); i++)
		{
			if(!mel::Keyboard::is_key_pressed(keys[i]))
			{
				pressed = false;
			}
			else
				pressed = true;			
		}
	}
}

/*
Reads in response from the user regarding the most recent
stimuli
*/
bool Staircase::ReadInput()
{    
    mel::Keyboard::wait_for_any_keys(input_keys_);

	if(mel::Keyboard::is_key_pressed(mel::Key::Add) || mel::Keyboard::is_key_pressed(mel::Key::Up))
	{
		// increments or zeroes number of crossovers_ if neccesary
		if(previous_angle_ > angle_)
		{   
			crossover_angles_[crossovers_] = angle_;
			crossovers_ += 1;	
		}	
		previous_angle_ = angle_;

		// increases angle_ by step_ size
		if((angle_+step_) <= kRangeMax_[condition_true_])	
			angle_ += step_;
		else                        	
			angle_ = kRangeMax_[condition_true_]; 

		// waits until key release before advancing
		WaitForKeyRelease({mel::Key::Add, mel::Key::Up});
	}
	else if(mel::Keyboard::is_key_pressed(mel::Key::Subtract) || mel::Keyboard::is_key_pressed(mel::Key::Down))
	{
		// increments or zeroes number of crossovers_ if neccesary
		if(previous_angle_ < angle_)   		
		{   
			crossover_angles_[crossovers_] = angle_;
			crossovers_ += 1;	
		}		
		previous_angle_ = angle_;

		// decreases angle_ by step_ size
		if((angle_-step_) >= kRangeMin_)	
			angle_ -= step_;
		else                			
			angle_ = kRangeMin_;

		// waits until key release before advancing
		WaitForKeyRelease({mel::Key::Subtract, mel::Key::Down});
	}
	else if(mel::Keyboard::is_key_pressed(mel::Key::Comma) || mel::Keyboard::is_key_pressed(mel::Key::Left))
	{	
		// halves the step_ size if step_ size is to be reduced
        step_ /= 2; 

		// waits until key release before advancing
		WaitForKeyRelease({mel::Key::Comma, mel::Key::Left});
	}
	else if(mel::Keyboard::is_key_pressed(mel::Key::Period) || mel::Keyboard::is_key_pressed(mel::Key::Right))
	{
		// doubles the step_ size if step_ size is to be increased
        step_ *= 2; 

		// waits until key release before advancing
		WaitForKeyRelease({mel::Key::Period, mel::Key::Right});
	}
	else return false;
	
    // outputs the current angle_ and step_ size for debugging purposes
	mel::print("Angle: " + std::to_string(angle_) + " Previous Angle: " + std::to_string(previous_angle_) + " Step: " + std::to_string(step_));
    return true;
}


/***********************************************************
**************** IMPORT/EXPORT FUNCTIONS *******************
************************************************************/
/*
Imports trialList from a saved file
*/
bool Staircase::ImportList(std::string filepath)
{		
	// // imports condition information from trialList file
	// if(!mel::csv_read_row(filepath,conditions_,1,0)) return false;

	// // prints condition labels for debug
	// // for (int j = 0; j < kNumberConditions_; j++)
	// // {
	// // 	print_string(to_string(conditions_[j]) + ",");
	// // }
	// // print(" ");

	// // loads trialList file from import into class
	// std::array<std::array<double, kConditions_>, kAngles_ * kTrials_> output;
	// if (!mel::csv_read_rows(filepath, output, 2, 0)) return false;
	
	// for (int i = 0; i < kAngles_ * kTrials_; i++)
	// {
	// 	for (int j = 0; j < kConditions_; j++)
	// 	{
	// 		final_angles_[j][i] = output[i][j];
	// 		// print_string(to_string(final_angles_[j][i]) + ",");
	// 	}			
	// 	// print(" ");
	// }
	return true;
}


/*
Exports trialList to a saved file
*/
void Staircase::ExportList(std::string filepath)
{
	// create new data logger and prepare output trialList file
	const std::vector<std::string> kHeaderNames = 
	{ 
		"0=Str",
		"1=Str_Squ",
		"2=Squ",
		"3=Squ_Str"
	};
	mel::csv_write_row(filepath, kHeaderNames);

	// output order of conditions_ in current test
	std::vector<double> condition_row = 
	{
		(double)conditions_[0], 
		(double)conditions_[1],
		(double)conditions_[2],
		(double)conditions_[3]
	};	
	mel::csv_append_row(filepath, condition_row);

	// output order of all angle values in current test
	std::array<std::array<double, kConditions_>, kTrials_> output;
	for(int i=0; i < final_angles_.size(); i++) {
    	for(int j=0; j < final_angles_[i].size(); j++) {
    		output[j][i] = final_angles_[i][j];
   		}
 	}
	mel::csv_append_rows(filepath, output);

	// output order of all angle values in current test
	// for (int i = 0; i < kNumberAngles_*kNumberTrials_; i++)
	// {
	// 	condition_row = { 
	// 		final_angles_[0][i], 
	// 		final_angles_[1][i],
	// 		final_angles_[2][i]
	// 		};
	// 	csv_append_row(filepath, condition_row);
	// 	print_string(".");
	// }
	// creates space for next statement
	mel::print("");

	// final messages before end of program
	mel::print("Staircase successfuly exported!");

	// creates space for next statement
	mel::print("");
}