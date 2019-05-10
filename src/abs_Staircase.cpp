/*
File: abs_Staircase.cpp
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
#include "abs_Staircase.hpp"

// namespace for std
using namespace mel;


/***********************************************************
********************** CONSTRUCTOR *************************
************************************************************/
/*
Constructor for the Staircase class
*/
Staircase::Staircase()
{
	// create random range
	auto rng = default_random_engine{ rd() };

	// generate random ordering of conditions
	shuffle(conditions.begin(), conditions.end(), rng);

    // set starting condition to the front
    condItr = 0; // condition iterator
    condTrue = conditions[condItr]; // true condition

    // set initial angle values
    trialInit();
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
void Staircase::trialInit()
{
    // chooses whether to start above or below the threshold
	uniform_real_distribution<double> distribution(RANGE_MIN, RANGE_MAX);
	angle = distribution(rd);  // generates start angle
    
    // sets other initial values
    prevAngle = angle; 
    step = 1; // step size 
    crossovers = 0; // number of switches in method
}


/***********************************************************
******************** NAME FUNCTIONS ************************
************************************************************/
/*
Outputs current condition and angle name
*/
string Staircase::getTrialName()
{
	return getConditionName() + "_" + to_string(angle);
}

/*
Returns the name for the current condition
*/
string Staircase::getConditionName()
{
	return conditionNames[condTrue];
}

/*
Reutrns the name for the indicated condition
*/
string Staircase::getConditionName(int condNum)
{
	return conditionNames[condNum];
}

/***********************************************************
******************** ANGLE FUNCTIONS ***********************
************************************************************/
/*
Returns private angle number
*/
double Staircase::getAngle()
{
	return angle;
}

/*
Gets the interference angle based on current condition
*/
double Staircase::getInterferenceAngle()
{
	return getInterferenceAngle(condTrue);		
}

/*
Overloads interference call to get the interference angle if condition is provided
*/
double Staircase::getInterferenceAngle(int condNum)
{
	if (condNum >= 3 && condNum < 6) return INTERFERENCE_ANGLE_LOW;	
	else if (condNum >= 6 && condNum < 9) return INTERFERENCE_ANGLE_HIGH;
	else return ZERO_ANGLE;		
}

/*
Outputs current angle combination as an int array.
Current form outputs the angle for the stretch rocker
first and the squeeze band second
*/
void Staircase::getTestPositions(array<array<double,2>,2> &posDes)
{
	// determine the angles that will be used for the test for squeeze/stretch
	array<double, 2> testPositions;

	// generates test position array and test position array
	testPositions = { angle, getInterferenceAngle() };
	
	// attach zero position for motors to return to after cue
	posDes[0] = testPositions;
	posDes[1] = { ZERO_ANGLE, ZERO_ANGLE };
}

/***********************************************************
*************** CONDITION CONTROL FUNCTIONS *****************
************************************************************/
/*
Changes condition index to reference the next trial
*/
bool Staircase::nextCondition()
{
	if (condItr == (NUMBER_CONDITIONS) - 1)
        return false; // return false if no more conditions
	else {
		condItr++;
        condTrue = conditions[condItr];
        trialInit();
        return true;
	}
}

/*
Sets the true condition number
*/
bool Staircase::setCondNum(int condNum)
{
    // sets the method to specified condition if it is a
    // real condition
	if (condNum >= 0 && condNum < NUMBER_CONDITIONS)
    {
        condTrue = condNum;
        trialInit();
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
bool Staircase::readInput()
{    
    Keyboard::wait_for_any_keys(inputKeys);

	if(	Keyboard::is_key_pressed(Key::Add) || Keyboard::is_key_pressed(Key::Up))
	{
		// increments or zeroes number of crossovers if neccesary
		if(prevAngle > angle)   crossovers += 1;
		else                    crossovers = 0;		
		prevAngle = angle;

		// increases angle by step size
		if((angle+step) <= RANGE_MAX)	angle += step;
		else                        	angle = RANGE_MAX; 
	}
	else if(Keyboard::is_key_pressed(Key::Subtract) || Keyboard::is_key_pressed(Key::Down))
	{
		// increments or zeroes number of crossovers if neccesary
		if(prevAngle < angle)   crossovers += 1;
		else        			crossovers = 0;		
		prevAngle = angle;

		// decreases angle by step size
		if((angle-step) >= RANGE_MIN)	angle -= step;
		else                			angle = RANGE_MIN;
	}
	else if(Keyboard::is_key_pressed(Key::Comma) || Keyboard::is_key_pressed(Key::Left))
		// halves the step size if step size is to be reduced
        step /= 2; 
	else if(Keyboard::is_key_pressed(Key::Period) || Keyboard::is_key_pressed(Key::Right))
		// doubles the step size if step size is to be increased
        step *= 2; 
	else return false;
	
    // outputs the current angle and step size for debugging purposes
	print("Angle: " + to_string(angle) + " Step: " + to_string(step));
    return true;
}