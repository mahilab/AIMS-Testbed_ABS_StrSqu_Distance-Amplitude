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

    // set initial angle values
    TrialInitialize();
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
Sets default values for the staircase method
*/
void Staircase::TrialInitialize()
{
    // chooses whether to start above or below the threshold
	std::uniform_real_distribution<double> distribution(kRangeMin_, kRangeMax_);
	angle_ = distribution(random_device_);  // generates start angle
    
    // sets other initial values
    previous_angle_ = angle_; 
    step_ = 1; // step_ size 
    crossovers_ = 0; // number of switches in method
}


/***********************************************************
******************** NAME FUNCTIONS ************************
************************************************************/
/*
Outputs current condition and angle_ name
*/
std::string Staircase::GetTrialName()
{
	return GetConditionName() + "_" + std::to_string(angle_);
}

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
Returns private angle_ number
*/
double Staircase::GetAngle()
{
	return angle_;
}

/*
Gets the interference angle_ based on current condition
*/
double Staircase::GetInterferenceAngle()
{
	return GetInterferenceAngle(condition_true_);		
}

/*
Overloads interference call to get the interference angle_ if condition is provided
*/
double Staircase::GetInterferenceAngle(int condition_num)
{
	if (condition_num >= 3 && condition_num < 6) return kInterferenceLow_;	
	else if (condition_num >= 6 && condition_num < 9) return kInterferenceHigh_;
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
	test_positions = { angle_, GetInterferenceAngle() };
	
	// attach zero position for motors to return to after cue
	position_desired[0] = test_positions;
	position_desired[1] = { kZero_, kZero_ };
}

/***********************************************************
*************** CONDITION CONTROL FUNCTIONS *****************
************************************************************/
/*
Changes condition index to reference the next trial
*/
bool Staircase::NextCondition()
{
	if (condition_iterator_ == (kNumberCondtions_) - 1)
        return false; // return false if no more conditions_
	else {
		condition_iterator_++;
        condition_true_ = conditions_[condition_iterator_];
        TrialInitialize();
        return true;
	}
}

/*
Sets the true condition number
*/
bool Staircase::SetConditionNum(int condition_num)
{
    // sets the method to specified condition if it is a
    // real condition
	if (condition_num >= 0 && condition_num < kNumberCondtions_)
    {
        condition_true_ = condition_num;
        TrialInitialize();
        return true;
    }
    else return false;
}


/***********************************************************
************************ UI FUNCTIONS **********************
************************************************************/
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
			crossovers_ += 1;
		else                    
			crossovers_ = 0;		
		previous_angle_ = angle_;

		// increases angle_ by step_ size
		if((angle_+step_) <= kRangeMax_)	
			angle_ += step_;
		else                        	
			angle_ = kRangeMax_; 
	}
	else if(mel::Keyboard::is_key_pressed(mel::Key::Subtract) || mel::Keyboard::is_key_pressed(mel::Key::Down))
	{
		// increments or zeroes number of crossovers_ if neccesary
		if(previous_angle_ < angle_)   
			crossovers_ += 1;
		else        			
			crossovers_ = 0;		
		previous_angle_ = angle_;

		// decreases angle_ by step_ size
		if((angle_-step_) >= kRangeMin_)	
			angle_ -= step_;
		else                			
			angle_ = kRangeMin_;
	}
	else if(mel::Keyboard::is_key_pressed(mel::Key::Comma) || mel::Keyboard::is_key_pressed(mel::Key::Left))
		// halves the step_ size if step_ size is to be reduced
        step_ /= 2; 
	else if(mel::Keyboard::is_key_pressed(mel::Key::Period) || mel::Keyboard::is_key_pressed(mel::Key::Right))
		// doubles the step_ size if step_ size is to be increased
        step_ *= 2; 
	else return false;
	
    // outputs the current angle_ and step_ size for debugging purposes
	mel::print("Angle: " + std::to_string(angle_) + " Step: " + std::to_string(step_));
    return true;
}