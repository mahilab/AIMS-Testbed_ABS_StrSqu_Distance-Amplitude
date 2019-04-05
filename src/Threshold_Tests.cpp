/*
File: Threshold_Tests.cpp
________________________________
Author(s): Zane Zook (gadzooks@rice.edu)

This file is the main file of the HapEE-Control project
which send messages using the MAXON motor controller
EPOS library to the EPOS controllers and measures
forces/torques from the NI DAQ simultaneously. Hardware
used with this program include: 2 custom MAXON motors,
2 EPOS4 24/1.5 CAN Motor controllers, 2 Nano 25 ATI
Force/Torque Sensors, and 1 NI DAQ for the force sensors.
*/


/***********************************************************
******************** LIBRARY IMPORT ************************
************************************************************/
// libraries for DaqNI Class
#include "DaqNI.hpp"

// libraries for Maxon Motor Class
#include "Definitions.h"
#include "MaxonMotor.hpp"

// libraries for Trial List Class
#include "TrialList.hpp"

// libraries for VideoCap Class
// #include "VideoCap.hpp"

// libraries for MEL
#include <MEL/Core/Console.hpp>
#include <MEL/Core/Timer.hpp>
#include <MEL/Logging/Csv.hpp>
#include <MEL/Utility/Mutex.hpp>
#include <MEL/Devices/AtiSensor.hpp>
#include <MEL/Devices/Windows/Keyboard.hpp>
#include <MEL/Utility/System.hpp>

// other misc standard libraries
#include <queue>
#include <thread>

// namespace for MEL
using namespace mel;
using namespace std;


/***********************************************************
******************* GLOBAL VARIABLES ***********************
************************************************************/
// constant variables 

int			 g_TIME_BW_CUES(1000);// sets the number of milliseconds to wait in between cues
const int	 g_CONFIRM_VALUE(123);
const bool	 g_TIMESTAMP(false);
const string g_DATA_PATH("C:/Users/zaz2/Documents/Interference JND"); //file path to main project files
																										 
// subject specific variables
TrialList	 g_trialList;
int			 g_subject		= 0;

// actual motor positions variable
double		 g_motorPos[2];
double		 g_motorDesPos[2];
bool	     g_motorFlag(false);
ctrl_bool	 g_stop(false);
Mutex		 g_mutex; // mutex for the separate motor position reading thread


/***********************************************************
******************** MOTOR FUNCTIONS ***********************
************************************************************/
/*
Sends relevant parameters to set up the Maxon motor
*/
void motorInitialize(MaxonMotor &motor, char* portName)
{
	// define relevant parameters for the controller
	const unsigned int DESIRED_VELOCITY =		10000;
	const unsigned int DESIRED_ACCELERATION =	100000;
	const unsigned int DESIRED_DECELERATION =	100000;

	// set important parameters of the motor
	motor.setPort(portName);

	// activate motor controller
	motor.start();

	// set motor control parameters
	motor.setControlParam(DESIRED_VELOCITY, DESIRED_ACCELERATION, DESIRED_DECELERATION);
}


/***********************************************************
****************** MOVEMENT FUNCTIONS **********************
************************************************************/
/*
Function to move the motor and measure its position. Position
measurement occurs at 100Hz in a separate thread to the 
1000Hz force sensor recorder
*/
void motorPosGet(array<array<int,2>,4> &posDes, MaxonMotor &motorA, MaxonMotor &motorB) 
{
	// loops through each of the positions in the array for the trial
	for (int i = 0; i < posDes.size(); i++)
	{
		// update desired positions
		{
			Lock lock(g_mutex);
			g_motorDesPos[0] = posDes[i][0];
			g_motorDesPos[1] = posDes[i][1];
		}

		// move motors to desired positions
		motorA.move((long) g_motorDesPos[0]);
		motorB.move((long) g_motorDesPos[1]);

		// create 100Hz timer
		Timer timer(hertz(100));

		// measures the motor positions during any motor movement
		while (!motorA.targetReached() || !motorB.targetReached()) {

			// creates and defines the actual positions of the motors
			long posA, posB;
			motorA.getPosition(posA);
			motorB.getPosition(posB);
			{
				Lock lock(g_mutex);
				g_motorPos[0] = (double)posA;
				g_motorPos[1] = (double)posB;
			}
			timer.wait();
		}

		//waits between cue 1 and cue 2
		if (i == 1)
		{
			sleep(milliseconds(g_TIME_BW_CUES));
		}
	}
	// tells the force sensor loop to exit once trial is complete
	g_motorFlag = true;
}

/*
Measures force/torque data, motor position data and time information
during the motor movement
*/
void recordMovementTrial(array<array<int,2>,4> &posDes, DaqNI &daqNI,
						 AtiSensor &atiA,				AtiSensor &atiB,
						 MaxonMotor &motorA,			MaxonMotor &motorB,
						 vector<vector<double>>* output_)
{
	// initial sample
	int sample = 0;

	// starts motor position read thread
	std::thread motorPosThread(motorPosGet, std::ref(posDes), std::ref(motorA), std::ref(motorB));

	// create 1000Hz timer
	Timer timer(hertz(1000)); 

	// movement data record loop
	while (!g_motorFlag)
	{
		// DAQmx Read Code
		daqNI.update();

		// measuring force/torque sensors
		vector<double> forceA =	 atiA.get_forces();
		vector<double> forceB =  atiB.get_forces();
		vector<double> torqueA = atiA.get_torques();
		vector<double> torqueB = atiB.get_torques();
		vector<double> outputRow;

		// locks pulls data into the outputRow format and unlocks
		{
			Lock lock(g_mutex);

			outputRow = { (double)sample,
				// Motor/Sensor A
				g_motorDesPos[0],	g_motorPos[0], 
				forceA[0],			forceA[1],			forceA[2], 
				torqueA[0],			torqueA[1],			torqueA[2],

				// Motor/Sensor B
				g_motorDesPos[1],	g_motorPos[1],
				forceB[0],			forceB[1],			forceB[2],
				torqueB[0],			torqueB[1],			torqueB[2]
			};
		}
		// input the the sampled data into output buffer
		output_->push_back(outputRow);

		// increment sample number
		sample++;
		timer.wait();
	}

	// joins threads together and resets flag before continuing
	motorPosThread.join();
	g_motorFlag = false;
}

/*
Runs a single test trial on motorA to ensure data logging
is working.
*/
void runMovementTrial(array<array<int, 2>, 4> &posDes,	DaqNI &daqNI, 
					  AtiSensor &atiA,					AtiSensor &atiB,
					  MaxonMotor &motorA,				MaxonMotor &motorB)
{
	// create new output buffer
	vector<vector<double>> movementOutput;

	// defining the file name for the export data 
	string fileName = "sub" + to_string(g_subject) + "_" + to_string(g_trialList.getIterationNumber()) + "_" + g_trialList.getTrialName() + "_data";
	string filepath = g_DATA_PATH + "/data/FT" + "/subject" + to_string(g_subject) + fileName;

	// // create new camera directory if needed
	// string camDir = g_DATA_PATH + "/data/CAM" + "/subject" + to_string(g_subject);
	// create_directory(camDir);

	// // starts camera video acquisition
	// if (!camera.beginCapture(camDir + "/" + fileName + ".avi")) print("Camera did not begin capture");

	// starting haptic trial
	recordMovementTrial(posDes, daqNI, atiA, atiB, motorA, motorB, &movementOutput);
	
	// // stops camera video acquisition
	// if (!camera.endCapture()) print("Camera did not end capture");

	// Defines header names of the csv
	const vector<string> HEADER_NAMES = { "Samples",
		// Motor/Sensor A
		"Position A Desired", "Position A Actual",
		"FxA", "FyA", "FzA",
		"TxA", "TyA", "TzA",

		// Motor/Sensor B
		"Position B Desired", "Position B Actual", 
		"FxB", "FyB", "FzB", 
		"TxB", "TyB", "TzB" 
		};

	// saves and exports trial data
	csv_write_row(filepath, HEADER_NAMES);
	csv_append_rows(filepath, movementOutput);
	//print(fileName + "Output Successfull");
}


/***********************************************************
*************** IMPORT UI HELPER FUNCTIONS *****************
************************************************************/
/*
Asks the experimenter for the subject number and stores it
*/
void importSubjectNumber()
{
	// variable for input
	int inputVal = 0;

	// asks experimenter to input subject number for the experiment
	mel::print("Please indicate the subject number: ");
	std::cin		>> g_subject;

	mel::print("You typed " + to_string(g_subject) + ", is this correct?");
	mel::print("Please type CONFIRM_VALUE to confirm subject number");
	std::cin		>> inputVal;

	// loops until a proper response is given
	while (inputVal != g_CONFIRM_VALUE)
	{
		mel::print("Subject number was not confirmed. You typed: " + to_string(inputVal));
		mel::print("Please indicate the subject number: ");

		std::cin		>> g_subject;
		mel::print("You typed " + to_string(g_subject) + ", is this correct?");
		mel::print("Please type CONFIRM_VALUE to confirm subject number");
		std::cin >> inputVal;
	}
	mel::print("Subject number " + to_string(g_subject) + " confirmed");
	mel::print("");
}

/*
Based on the subject number, attempts to import the relevant
trialList to the experiment.
*/
void importTrialList()
{
	// attempts to import trialList for subject
	string fileName = "sub" + to_string(g_subject) + "_trialList";
	if (g_trialList.importList(fileName, g_DATA_PATH + "/data/trialList"))
	{
		mel::print("Subject " + to_string(g_subject) + "'s trialList has been successfully imported");
	}
	else mel::print("Subject " + to_string(g_subject) + "'s trialList has been made and randomized successfully");
	mel::print("");
}

/*
Based on the subject number, attempts to import the relevant
trialList to the experiment.
*/
void importRecordJND(vector<vector<double>>* thresholdOutput_)
{
	// declares variables for filename and output
	string fileName = "sub" + to_string(g_subject) + "_JND_data";
	string filepath = g_DATA_PATH + "/data/JND" + fileName;
	vector<vector<double>> output;

	// attempts to import JND record for subject
	if (csv_read_rows(filepath, output))
	{
		// defines relevant variables for data import
		vector<double>	outputRow;
		const int		ITERATION_NUM_INDEX = 0;
		const int		ANGLE_NUM_INDEX = 2;

		// loads JND record into experiment
		for (int i = 0; i < output.size() - 1; i++)
		{
			outputRow = output[i + 1];
			thresholdOutput_->push_back(outputRow);
		}

		// confirms import with experimenter 
		g_trialList.setCombo((int)outputRow[ITERATION_NUM_INDEX] + 1, (int)outputRow[ANGLE_NUM_INDEX] + 1);
		print("Subject " + to_string(g_subject) + "'s JND record has been successfully imported");
		print("Current trial detected @");
		print("Iteration: " + to_string(g_trialList.getIterationNumber()));
		print("Condition:" + to_string(g_trialList.getCondNum()) + " - " + g_trialList.getConditionName());
		print("Angle:" + to_string(g_trialList.getAngCurr()) + " - " + to_string(g_trialList.getAngleNumber()));
		
		// waits for confirmation of import
		print("Is this correct? Please type CONFIRM_VALUE to confirm...");
		int		inputVal = 0;
		cin		>> inputVal;

		// loops until import is confirmed 
		while (inputVal != g_CONFIRM_VALUE)
		{
			print("Import Rejected. Please input desired iteration index number:");
			cin		>> inputVal;
			g_trialList.setCombo(inputVal, g_trialList.getAngCurr());

			print("Please input desired angle index number:");
			cin		>> inputVal;
			g_trialList.setCombo(g_trialList.getIterationNumber(), inputVal);

			print("Current trial detected @");
			print("Iteration: " + to_string(g_trialList.getIterationNumber()));
			print("Condition:" + to_string(g_trialList.getCondNum()) + " - " + g_trialList.getConditionName());
			print("Angle:" + to_string(g_trialList.getAngCurr()) + " - " + to_string(g_trialList.getAngleNumber()));
			print("Is this correct? Please type CONFIRM_VALUE to confirm.");
			cin		>> inputVal;
		}
		print("Import Accepted.");
	}
	else print("Subject " + to_string(g_subject) + "'s JND record has been built successfully");

	// makes space for next print statements
	print("");
}


/***********************************************************
************* EXPERIMENT UI HELPER FUNCTIONS ***************
************************************************************/
/*
Record's participant's JND response to current trial
*/
void recordExperimentJND(vector<vector<double>>* thresholdOutput_, bool ref2Test)
{
	// creates an integer for user input
	int inputVal = 0;

	// asks user for input regarding their comparison
	print("Iteration: " + to_string(g_trialList.getIterationNumber()));
	print("Which cue was larger, 1 or 2? Please enter an integer response...");
	vector<Key> inputKeys = { Key::Num1, Key::Numpad1, Key::Num2, Key::Numpad2 };
	Keyboard::wait_for_any_keys(inputKeys);
	
	if (Keyboard::is_key_pressed(Key::Num1) || Keyboard::is_key_pressed(Key::Numpad1))
	{
		inputVal = 1;
		print("You typed " + to_string(inputVal));
	}
	else if (Keyboard::is_key_pressed(Key::Num2) || Keyboard::is_key_pressed(Key::Numpad2))
	{
		inputVal = 2;
		print("You typed " + to_string(inputVal));
	}

	// based on cue order and user input determines what was
	// percieved as the greater cue
	int greaterCue;
	if (ref2Test)
	{
		/*
		Reference cue and then comparision cue administered
		If the user indicates 1 was greater, that would be the reference cue (0).
		If the user indicates 2 was greater, that would be the comparison cue (1).
		*/
		greaterCue = inputVal - 1;
	}
	else
	{
		/*
		Comparision cue and then Reference cue administered
		If the user indicates 1 was greater, that would be the comparison cue (1).
		If the user indicates 2 was greater, that would be the reference cue (0).
		*/
		greaterCue = inputVal % 2;
	}

	// add current row for JND testing to the buffer
	vector<double> outputRow = { 
		(double)g_trialList.getIterationNumber(),	(double)g_trialList.getCondNum(),
		(double)g_trialList.getAngCurr(),			(double)g_trialList.getReferenceAngle(),
		(double)g_trialList.getAngleNumber(),		(double)greaterCue,
		(double)inputVal,							(double)ref2Test 
	};
	thresholdOutput_->push_back(outputRow);
}

/*
Advances to the next condition or exits test based on
experimenter input.
*/
void advanceExperimentCondition()
{
	if (g_stop) return;

	// informs experimenter condition is complete
	//print("");
	//print("Condition " + to_string(g_trialList.getCondNum()) + ": " +
	//	g_trialList.getConditionName() + " Completed!");

	// moves to next condition if next condition exists
	if (!g_trialList.hasNextCondition())
	{
		print("All conditions have been completed...");
		g_stop = true;
	}

	// creates space for next statement
	print("");

	// creates input value to ask experimenter if trial should continue to next condition
	int inputVal = 0;
	print("Please register a save to exit or input CONFIRM_VALUE to continue to next condition...");
	cin >> inputVal;

	// loops until a proper response is given
	while(!g_stop && inputVal != g_CONFIRM_VALUE)
	{ 
		print("Please register a save to exit or input CONFIRM_VALUE to continue to next condition...");
		cin >> inputVal;
	}

	// creates space for next statement
	print("");

	// advances experiment to next trial
	g_trialList.nextCondition();
}


/***********************************************************
************* MAIN USER INTERACTION FUNCTIONS **************
************************************************************/
/*
Asks the experimenter for the subject number. Then, if
relevant, imports trialList and JND file from previous
experiment
*/
void runImportUI(vector<vector<double>>* thresholdOutput_)
{
	importSubjectNumber();
	importTrialList();
	importRecordJND(thresholdOutput_);
}

/*
Run a single condition on a user automatically. If
experimenter enters the exit value, exits the program.
*/
void runExperimentUI(DaqNI &daqNI,
					 AtiSensor &atiA,	 AtiSensor &atiB,
					 MaxonMotor &motorA, MaxonMotor &motorB,
					 vector<vector<double>>* thresholdOutput_)
{
	// defines positions of the currrent test cue
	array<array<int, 2>, 4> posDes;
	
	// prints current condition for participant/experimenter 
	int inputVal = 0;
	print("Current Condition: " + g_trialList.getConditionName());

	// waits for confirmation before continuing
	while(inputVal != g_CONFIRM_VALUE)
	{	
		print("Insert CONFIRM_VALUE when you are ready to begin condition");
		cin >> inputVal;
	}

	// runs trials on the selected condition with data collection
	while (g_trialList.hasNextAngle())
	{
		// check for exit condition
		if (g_stop) return;

		// get next experiment cue
		bool ref2Test = g_trialList.getTestPositions(posDes);

		// provides cue to user
		runMovementTrial(posDes, daqNI, atiA, atiB, motorA, motorB);

		// record JND trial response
		recordExperimentJND(thresholdOutput_, ref2Test);

		// moves experiment to the next trial within current condition
		g_trialList.nextAngle();
	}

	// check for exit condition before final cue
	if (g_stop) return;

	// get final experiment cue
	bool ref2Test = g_trialList.getTestPositions(posDes);

	// provides final cue of condition to user
	runMovementTrial(posDes, daqNI, atiA, atiB, motorA, motorB);

	// record final JND trial response
	recordExperimentJND(thresholdOutput_, ref2Test);
}

/*
Saves the JND data file as well as the trialList given
to the participant.
*/
void runExportUI(vector<vector<double>>* thresholdOutput_)
{
	// defining the file name for the JND data file
	string fileName = "sub" + to_string(g_subject) + "_JND_data";
	string filepath = g_DATA_PATH + "/data/JND" + filepath;

	// builds header names for threshold logger
	const vector<string> HEADER_NAMES = { 
		"Iteration",			"Condition (0=St 1=StXSq 2=Sq 3=SqXSt)",
		"AngCurr",				"Reference Angle",
		"Comparision Angle",	"Greater Cue (0=Reference 1=Comparison)",
		"User Direct Input (1=First Cue Greater 2=Second Cue Greater)",
		"Cue Order (0=Test First 1=Ref First)" 
	};

	// saves the JND data
	csv_write_row(filepath, HEADER_NAMES);
	csv_append_rows(filepath, *thresholdOutput_);

	// information about the current trial the test was exited on
	print("Test Saved @ ");
	print("Iteration: " + to_string(g_trialList.getIterationNumber()));
	print("Condition:" + to_string(g_trialList.getCondNum()) + " - " 
		+ g_trialList.getConditionName());
	print("Angle:" + to_string(g_trialList.getAngCurr()) + " - "
		+ to_string(g_trialList.getAngleNumber()));

	// exporting the trialList for this subject
	// defining the file name for the JND data file
	fileName = "sub" + to_string(g_subject) + "_trialList";
	g_trialList.exportList(fileName, g_DATA_PATH + "/data/trialList", g_TIMESTAMP);

	// final messages before end of program
	print("trialList exported...");

	// creates space for next statement
	print("");
}


/***********************************************************
********************* MAIN FUNCTION ************************
************************************************************/
/*
Control C handler to cancel the program at any point and save all data to that point
*/
bool my_handler(CtrlEvent event) {
	if (event == CtrlEvent::CtrlC) {
		print("Save and exit registered");
		print("");
		g_stop = true;
	}
	return true;
}

/*
Main function of the program references all other functions
*/
int main()
{
	// registers the mel handler to exit the program using Ctrl-c
	register_ctrl_handler(my_handler);

	// creates all neccesary objects for the program
	DaqNI					daqNI;					// creates a new analog input from the NI DAQ
	AtiSensor				atiA, atiB;				// create the ATI FT Sensors
	MaxonMotor				motorA, motorB;			// create new motors
	vector<vector<double>>	thresholdOutput_;		// creates pointer to the output data file for the experiment
	// VideoCap	camera(1920,1080,30);	// create new camera object for this trial 
										// with a resolution of 960, 720p and 30 fps

	/*
	Sensor Initialization
	*/
	// calibrate the FT sensors 
	atiA.load_calibration("FT26062.cal");
	atiB.load_calibration("FT26061.cal");

	// set channels used for the FT sensors
	atiA.set_channels(daqNI[{0, 1, 2, 3, 4, 5}]);	 
	atiB.set_channels(daqNI[{16, 17, 18, 19, 20, 21}]);

	// zero the ATI FT sensors 
	daqNI.update();
	atiA.zero();
	atiB.zero();
	
	/*
	Motor Initialization
	*/
	motorInitialize(motorA, (char*)"USB0");
	motorInitialize(motorB, (char*)"USB1");
		
	/* 
	User Interaction Portion of Program
	*/
	// import relevant data or creates new data structures
	runImportUI(&thresholdOutput_);

	// runs JND experimental protocal automatically
	while (!g_stop)
	{
		// runs a full condition unless interupted
		runExperimentUI(daqNI, atiA, atiB, motorA, motorB, &thresholdOutput_);

		// advance to the next condition if another condition exists
		advanceExperimentCondition();		
	}

	// exports relevant JND data
	runExportUI(&thresholdOutput_);
	
	// // delete camera object to free memory
	// camera.~VideoCap();

	print("Press Escape to end program...");
	Keyboard::wait_for_key(Key::Escape);
	print("Escape pressed! Exiting application...");
	
	return EXIT_SUCCESS;
}