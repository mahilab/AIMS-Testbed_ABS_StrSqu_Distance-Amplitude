/*
File: abs_TrialList.cpp
________________________________
Author(s): Joshua Fleck (jjf8@rice.edu)
Edited by: Zane Zook (gadzooks@rice.edu), Andrew Low (andrew.low@rice.edu)

Defines a class to randomize a list of trial conditions for
Absolute Threshold experiment.
Last Changed: 12/21/18
*/

/***********************************************************
******************** LIBRARY IMPORT ************************
************************************************************/
// libraries for TrialList Class
#include "abs_trialList.hpp"

// libraries for MEL
#include <MEL/Logging/Csv.hpp>
#include <MEL/Core/Console.hpp>

// string libraries
#include <string>

// namespace for std
using namespace std;


/***********************************************************
********************** CONSTRUCTOR *************************
************************************************************/
/*
Constructor for the TrialList class
*/
TrialList::TrialList()
{
	// fill out trialArrs with angles
	array<double, g_NUMBER_ANGLES*g_NUMBER_TRIALS> conditionAngles;
	for (int i = 0; i < g_NUMBER_ANGLES*g_NUMBER_TRIALS; i = i + g_NUMBER_ANGLES) 
	{
		for (int j = 0; j < g_NUMBER_ANGLES; j++) 
		{
			conditionAngles[i + j] = g_DEFAULT_ANGLES[j];
		}
		
	}
	angles.fill(conditionAngles);

	// create random range
	auto rng = default_random_engine{ rd() };

	// generate random ordering  of angles for each of the conditions
	for (int i = 0; i < g_NUMBER_CONDITIONS; i++) {
		shuffle(angles[i].begin(), angles[i].end(), rng);
	}

	// generate random ordering of angles by condition
	shuffle(conditions.begin(), conditions.end(), rng);
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
string TrialList::getTrialName(int con, int ang)
{
	return getConditionName(con) + "_" + to_string(getAngleNumber(con, ang));
}

/*
Outputs current condition name
*/
string TrialList::getConditionName(int con)
{
	int condNameIndex = conditions[con];
	return conditionNames[condNameIndex];
}

/*
Outputs current angle number
*/
double TrialList::getAngleNumber(int con, int ang)
{
	return angles[con][ang];
}

/*
Outputs current angle combination as an int array. 
Current form outputs the angle for the stretch rocker
first and the squeeze band second. Also indicates 
if the reference angle should be given first or second
*/
void TrialList::getTestPositions(array<array<double,2>,2> &posDes, int con, int ang)
{
	// determine the angles that will be used for the test for squeeze/stretch
	array<double, 2> testPositions;

	// generates test position array and reference position array
	switch (getCondNum())
	{
	case 0:
		testPositions = { getAngleNumber(con, ang), g_ZERO_ANGLE };
		break;
	case 1:
		testPositions = { getAngleNumber(con, ang), g_INTERFERENCE_ANGLE_LOW };
		break;
	case 2:
		testPositions = { getAngleNumber(con, ang), g_INTERFERENCE_ANGLE_HIGH };
		break;
	}
	
	// attach zero position for motors to return to after cue
	posDes[0] = testPositions;
	posDes[1] = { g_ZERO_ANGLE,g_ZERO_ANGLE };
}

/*
Outputs current iteration number as an int
*/
int TrialList::getIterationNumber(int con, int ang)
{
	return (g_NUMBER_ANGLES*g_NUMBER_TRIALS*con) + (ang + 1);
}


/***********************************************************
***************** TRIAL NAME FUNCTIONS *********************
************************************************************/
/*
Calls private function to get current condition and angle
names
*/
string TrialList::getTrialName()
{
	return getTrialName(conCurr,angCurr);
}

/*
Calls private function to get current condition name
*/
string TrialList::getConditionName()
{
	return getConditionName(conCurr);
}

/*
Calls private function to get current angle number
*/
double TrialList::getAngleNumber()
{
	return angles[conCurr][angCurr];
}

/*
Calls private function to get the interference angle
*/
// ****************** ADD IN INTEREFENCE LOW AND INTERFERENCE HIGH ******************
/*
int TrialList::getReferenceAngle()
{
	return g_REFERENCE_ANGLE;
}
*/

/*
Calls private function to get the interference angle
*/
int TrialList::getInterference(int interferenceFlag)
{
	if (interferenceFlag == 0)
	{
		return g_ZERO_ANGLE;
	}
	else if (interferenceFlag == 1)
	{
		return g_INTERFERENCE_ANGLE_LOW;
	}
	else
	{
		return g_INTERFERENCE_ANGLE_HIGH;
	}
	
}

/*
Outputs current angle combination as an int array.
Current form outputs the angle for the stretch rocker
first and the squeeze band second
*/
void TrialList::getTestPositions(array<array<double,2>,2> &posDes)
{
	getTestPositions(posDes, conCurr, angCurr);
}

/*
Calls private function to get current iteration number
*/
int TrialList::getIterationNumber()
{
	return getIterationNumber(conCurr, angCurr);
}

/*
Calls private function to get current angle name
*/
string TrialList::getComboNames()
{
	string retString = "";
	for (int i = 0; i < g_NUMBER_CONDITIONS; i++) {
		array<double, g_NUMBER_ANGLES*g_NUMBER_TRIALS> locArr = angles[i];
		for (int j = 0; j < g_NUMBER_ANGLES*g_NUMBER_TRIALS; j++) {
			retString += to_string(getIterationNumber(i, j)) + ": " + getTrialName(i, j) + "\n";
		}
	}
	return retString;
}


/***********************************************************
*************** ITERATOR CONTROL FUNCTIONS *****************
************************************************************/
/*
Changes angle index to reference the next trial. 
*/
void TrialList::nextAngle()
{
	if (angCurr == (g_NUMBER_ANGLES*g_NUMBER_TRIALS) - 1); // do nothing
	else angCurr++;
}

/*
Changes angle index to reference the previous trial. 
*/
void TrialList::prevAngle()
{
	if (angCurr == 0); // do nothing
	else angCurr--;
}

/*
Checks if there is a next angle trial to run.
*/
bool TrialList::hasNextAngle()
{
	if (angCurr == (g_NUMBER_ANGLES*g_NUMBER_TRIALS) - 1) return false;
	else return true;
}

/*
Changes condition index to reference the next trial
*/
void TrialList::nextCondition()
{
	if (conCurr == (g_NUMBER_CONDITIONS) - 1); // do nothing
	else {
		conCurr++;
		angCurr = 0;
	}
}

/*
Changes condition index to reference the previous trial
*/
void TrialList::prevCondition()
{
	if (conCurr == 0); // do nothing
	else conCurr--;
}

/*
Checks if there is a next condition to run.
*/
bool TrialList::hasNextCondition()
{
	if (conCurr == (g_NUMBER_CONDITIONS) - 1) return false;
	else return true;
}

/*
Changes indexes to reference a specific trial in the set
*/
void TrialList::setCombo(int itr, int ang)
{
	if (ang == (g_NUMBER_ANGLES*g_NUMBER_TRIALS))
	{
		conCurr = (itr - (ang + 1)) / (g_NUMBER_ANGLES*g_NUMBER_TRIALS) + 1;
		angCurr = 0;
	}
	else
	{
		conCurr = (itr - (ang + 1)) / (g_NUMBER_ANGLES*g_NUMBER_TRIALS);
		angCurr = ang;
	}
}


/***********************************************************
*************** ITERATOR RETURN FUNCTIONS ******************
************************************************************/
/*
Returns current number representing the condition being tested
*/
int TrialList::getCondNum()
{
	return conditions[conCurr];
}

/*
Returns current index for the angle
*/
int TrialList::getAngCurr()
{
	return angCurr;
}


/***********************************************************
**************** IMPORT/EXPORT FUNCTIONS *******************
************************************************************/
/*
Imports trialList from a saved file
*/
bool TrialList::importList(string filepath)
{
	using namespace mel;
	// create prepare output trialList file
	vector<vector<double>> output;
	// if (!csv_read_rows(filepath, output, 0, 1))
	// {
	// 	print("error");
	// 	return false;
	// }
	// imports condition information from trialList file
	print("here da output");
	for(int i = 1; i < g_NUMBER_ANGLES*g_NUMBER_TRIALS + 1; i++)
	{
		vector<double> outputRow;
		csv_read_row(filepath, outputRow, i, 0);
		print(outputRow[0]);
		print(i);
	}
	// vector<double>
	// csv_read_row(filepath, row, 0, 1);
    // print(row);
		
	// vector<double> outputRow = output[0];
	// print("here");
	// for (int j = 0; j < outputRow.size(); j++)
	// {
	// 	conditions[j] = (int)outputRow[j];
	// }

	// // loads trialList file from import into class
	// for (int i = 0; i < output.size() - 2; i++)
	// {
	// 	outputRow = output[i+2];
	// 	for (int j = 0; j < outputRow.size(); j++)
	// 	{
	// 		angles[j][i] = (int)outputRow[j];
	// 	}
	// }
	return true;
}

/*
Exports trialList to a saved file
*/
void TrialList::exportList(string filepath, bool timestamp)
{
	using namespace mel;

	// create new data logger and prepare output trialList file
	const vector<string> HEADER_NAMES = { "0=St", "1=StXsq(lo)", "1=StXsq(hi)"};
	csv_write_row(filepath, HEADER_NAMES);

	// output order of conditions in current test
	vector<double> outputRow = {
		(double)conditions[0], 
		(double)conditions[1],
		(double)conditions[2]
		};
	
	csv_append_row(filepath, outputRow);

	// output order of all angle values in current test
	for (int i = 0; i < g_NUMBER_ANGLES*g_NUMBER_TRIALS; i++)
	{
		outputRow = { 
			angles[0][i], 
			angles[1][i],
			angles[2][i]
			};
		csv_append_row(filepath, outputRow);
	}
}