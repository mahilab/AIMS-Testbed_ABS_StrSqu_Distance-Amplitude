/*
File: threshold_tests.cpp
________________________________
Author(s): Zane Zook (gadzooks@rice.edu)
Edited By: Andrew Low (andrew.low@rice.edu)

This file is the Main file of the HapEE-Control project
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
#include "daq_ni.hpp"

// libraries for Maxon Motor Class
#include "Definitions.h"
#include "maxon_motor.hpp"

// libraries for TrialList Class
#include "absolute_triallist.hpp"

// libraries for the staircase class
#include "absolute_staircase.hpp"

// libraries for MEL
#include <MEL/Core/Console.hpp>
#include <MEL/Core/Timer.hpp>
#include <MEL/Logging/Csv.hpp>
#include <MEL/Utility/System.hpp>
#include <MEL/Utility/Mutex.hpp>
#include <MEL/Utility/Options.hpp>
#include <MEL/Devices/AtiSensor.hpp>
#include <MEL/Devices/Windows/Keyboard.hpp>

// other misc standard libraries
#include <queue>
#include <thread>
#include <string>

// namespace for MEL
using namespace mel;


/***********************************************************
******************* GLOBAL VARIABLES ***********************
************************************************************/
// constant variables 
const int	 		kTimeBetweenCues(1000);// sets the number of milliseconds to wait in between cues
const int	 		kConfirmValue(123);
const bool	 		kTimestamp(false);
const std::string	kDataPath("C:/Users/zaz2/Desktop/Absolute_Threshold_Tests"); //file path to Main project files

// variable to track protocol being run						
bool		 staircase_flag(false);

// subject specific variables
Staircase	 staircase;
TrialList	 trial_list;
int			 subject = 0;

// actual motor positions variable
double		 motor_position[2];
double		 motor_desired_position[2];
bool	     motor_flag(false);
ctrl_bool	 stop(false);
Mutex		 mutex; // mutex for the separate motor position reading thread


/***********************************************************
******************** MOTOR FUNCTIONS ***********************
************************************************************/
/*
Sends relevant parameters to set up the Maxon motor
*/
void MotorInitialize(MaxonMotor &motor, char* port_name)
{
	// define relevant parameters for the controller
	const unsigned int kDesiredVelocity =		10000;
	const unsigned int kDesiredAcceleration =	100000;
	const unsigned int kDesiredDeceleration =	100000;

	// set important parameters of the motor
	motor.SetPort(port_name);

	// activate motor controller
	motor.Start();

	// set motor control parameters
	motor.SetControlParam(kDesiredVelocity, kDesiredAcceleration, kDesiredDeceleration);
}


/***********************************************************
****************** MOVEMENT FUNCTIONS **********************
************************************************************/
/*
Function to move the motor and measure its position. Position
measurement occurs at 100Hz in a separate thread to the 
1000Hz force sensor recorder
*/
void MotorPositionGet(std::array<std::array<double,2>,2> &position_desired, MaxonMotor &motor_a, MaxonMotor &motor_b) 
{
	// loops through each of the positions in the std::array for the trial
	for (int i = 0; i < position_desired.size(); i++)
	{
		// update desired positions
		{
			Lock lock(mutex);
			motor_desired_position[0] = position_desired[i][0];
			motor_desired_position[1] = position_desired[i][1];
		}

		// move motors to desired positions
		motor_a.Move((double) motor_desired_position[0]);
		motor_b.Move((double) motor_desired_position[1]);

		// create 100Hz timer
		Timer timer(hertz(100));

		// measures the motor positions during any motor movement
		while (!motor_a.TargetReached() || !motor_b.TargetReached()) {

			// creates and defines the actual positions of the motors
			long position_a, position_b;
			motor_a.GetPosition(position_a);
			motor_b.GetPosition(position_b);
			{
				Lock lock(mutex);
				motor_position[0] = (double)position_a;
				motor_position[1] = (double)position_b;
			}
			timer.wait();
		}

		//waits between cue 1 and cue 2
		/*
		if (i == 1)
		{
			sleep(milliseconds(kTimeBetweenCues));
		}
		*/
	}
	// tells the force sensor loop to exit once trial is complete
	motor_flag = true;
}

/*
Measures force/torque data, motor position data and time information
during the motor movement
*/
void RecordMovementTrial(std::array<std::array<double,2>,2> &position_desired, 	DaqNI &daq_ni,
						 AtiSensor &ati_a,					AtiSensor &ati_b,
						 MaxonMotor &motor_a,				MaxonMotor &motor_b,
						 std::vector<std::vector<double>>* output_)
{
	// initial sample
	int sample = 0;

	// starts motor position read thread
	std::thread motor_position_thread(MotorPositionGet, std::ref(position_desired), std::ref(motor_a), std::ref(motor_b));

	// create 1000Hz timer
	Timer timer(hertz(1000)); 

	// movement data record loop
	while (!motor_flag)
	{
		// DAQmx Read Code
		daq_ni.update();

		// measuring force/torque sensors
		std::vector<double> forceA =	ati_a.get_forces();
		std::vector<double> forceB =  	ati_b.get_forces();
		std::vector<double> torqueA = 	ati_a.get_torques();
		std::vector<double> torqueB = 	ati_b.get_torques();
		std::vector<double> output_row;	

		// locks pulls data into the output_row format and unlocks
		{
			Lock lock(mutex);

			output_row = { (double)sample,
				// Motor/Sensor A
				motor_desired_position[0],	motor_position[0], 
				forceA[0],			forceA[1],			forceA[2], 
				torqueA[0],			torqueA[1],			torqueA[2],

				// Motor/Sensor B
				motor_desired_position[1],	motor_position[1],
				forceB[0],			forceB[1],			forceB[2],
				torqueB[0],			torqueB[1],			torqueB[2]
			};
		}
		// input the the sampled data into output buffer
		output_->push_back(output_row);

		// increment sample number
		sample++;
		timer.wait();
	}

	// joins threads together and resets flag before continuing
	motor_position_thread.join();
	motor_flag = false;
}

/*
Runs a single test trial on motor_a to ensure data logging
is working.
*/
void RunMovementTrial(std::array<std::array<double,2>,2> &position_desired,	DaqNI &daq_ni, 
					  AtiSensor &ati_a,					AtiSensor &ati_b,
					  MaxonMotor &motor_a,				MaxonMotor &motor_b)
{
	// create new output buffer
	std::vector<std::vector<double>> movementOutput;

	// defining the file name for the export data 
	std::string filename, filepath;
	filename = "/sub" + std::to_string(subject) + "_" + std::to_string(trial_list.GetIterationNumber()) + "_" + trial_list.GetTrialName() + "_data.csv";
	filepath = kDataPath + "/data/FT/subject" + std::to_string(subject) + filename;

	// starting haptic trial
	RecordMovementTrial(position_desired, daq_ni, ati_a, ati_b, motor_a, motor_b, &movementOutput);

	// Defines header names of the csv
	const std::vector<std::string> header_names = 
		{ 
		"Samples",
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
	if(!staircase_flag)
	{
		csv_write_row(filepath, header_names);
		csv_append_rows(filepath, movementOutput);
	}
}


/***********************************************************
*************** IMPORT UI HELPER FUNCTIONS *****************
************************************************************/
/*
Asks the experimenter for the subject number and stores it
*/
void ImportSubjectNumber()
{
	// variable for input
	int input_value = 0;

	// asks experimenter to input subject number for the experiment
	mel::print("Please indicate the subject number: ");
	std::cin	>> subject;

	mel::print("You typed " + std::to_string(subject) + ", is this correct?");
	mel::print("Please type CONFIRM_VALUE to confirm subject number");
	std::cin	>> input_value;

	// loops until a proper response is given
	while (input_value != kConfirmValue)
	{
		mel::print("Subject number was not confirmed. You typed: " + std::to_string(input_value));
		mel::print("Please indicate the subject number: ");

		std::cin >> subject;
		mel::print("You typed " + std::to_string(subject) + ", is this correct?");
		mel::print("Please type CONFIRM_VALUE to confirm subject number");
		std::cin >> input_value;
	}
	mel::print("Subject number " + std::to_string(subject) + " confirmed");
	mel::print("");
}

/*
Based on the subject number, attempts to import the relevant
trialList to the experiment.
*/
void ImportTrialList()
{
	// attempts to import trialList for subject
	std::string filename = "/sub" + std::to_string(subject) + "_trialList.csv";
	std::string filepath = kDataPath + "/data/trialList" + filename;
	if (trial_list.ImportList(filepath))
	{
		print("Subject " + std::to_string(subject) + "'s trialList has been successfully imported");
	}
	else
	{
		// if there was no trial_list to import, randomizes a new trialList
		trial_list.scramble();
		print("Subject " + std::to_string(subject) + "'s trialList has been made and randomized successfully");
	}
	print("");
}

/*
Based on the subject number, attempts to import the relevant
trialList to the experiment.
*/
void ImportRecordABS(std::vector<std::vector<double>>* threshold_output)
{
	// declares variables for filename and output
	std::string filename = "/sub" + std::to_string(subject) + "_ABS_data.csv";
	std::string filepath = kDataPath + "/data/ABS" + filename;

	// defines relevant variables for data import
	int 		rows = 0;
	int 		cols = 0;
	const int	kRowOffset(1);
	const int	kIterationNumIndex(0);
	const int	kAngleNumIndex(2);

	// determine size of the input 
	std::ifstream file(filepath);
	if (file.is_open())
	{
		std::string line_string;	
		while (getline(file, line_string)) 
		{
			rows ++;
			if (rows == kRowOffset)
			{
				std::istringstream line_stream(line_string);
				std::string value_string;
				while(getline(line_stream, value_string, ','))
					cols ++;
			}
		}
		rows -= kRowOffset;
	}

	// prints the size of the input file
	// print("Rows: " + std::to_string(rows) + " | Cols: " + std::to_string(cols));

	// defines variables to hold data input	
	std::vector<std::vector<double>>	input(rows, std::vector<double>(cols));
	std::vector<double> 				input_row(cols);

	// loads ABS threshold record into experiment
	if(csv_read_rows(filepath, input, kRowOffset, 0))
	{
		for (int i = 0; i < rows; i++)
		{
			input_row = input[i];
			threshold_output->push_back(input_row);
		}

		// confirms import with experimenter 
		trial_list.SetCombo((int)input_row[kIterationNumIndex] + 1, (int)input_row[kAngleNumIndex] + 1);
		print("Subject " + std::to_string(subject) + "'s ABS record has been successfully imported");
		print("Current trial detected @");
		print("Iteration: " + std::to_string(trial_list.GetIterationNumber()));
		print("Condition: " + std::to_string(trial_list.GetConditionNum()) + " - " + trial_list.GetConditionName());
		print("Angle: " + std::to_string(trial_list.GetAngleIndex()) + " - " + std::to_string(trial_list.GetAngleNumber()));
		
		// waits for confirmation of import
		print("Is this correct? Please type CONFIRM_VALUE to confirm...");
		int			input_value = 0;
		std::cin >> input_value;

		// loops until import is confirmed 
		while (input_value != kConfirmValue)
		{
			print("Import Rejected. Please input desired iteration index number:");
			std::cin		>> input_value;
			trial_list.SetCombo(input_value, trial_list.GetAngleIndex());

			print("Please input desired angle index number:");
			std::cin		>> input_value;
			trial_list.SetCombo(trial_list.GetIterationNumber(), input_value);

			print("Current trial detected @");
			print("Iteration: " + std::to_string(trial_list.GetIterationNumber()));
			print("Condition: " + std::to_string(trial_list.GetConditionNum()) + " - " + trial_list.GetConditionName());
			print("Angle: " + std::to_string(trial_list.GetAngleIndex()) + " - " + std::to_string(trial_list.GetAngleNumber()));
			print("Is this correct? Please type CONFIRM_VALUE to confirm.");
			std::cin		>> input_value;
		}
		print("Import Accepted.");
	}
	else
	{
		print("Subject " + std::to_string(subject) + "'s ABS record has been built successfully");
	}

	// makes space for next print statements
	print("");
}


/***********************************************************
************* EXPERIMENT UI HELPER FUNCTIONS ***************
************************************************************/
/*
Record's participant's ABS response to current trial
*/
//void RecordExperimentABS(std::vector<std::vector<double>>* threshold_output, bool ref2Test)
void RecordExperimentABS(std::vector<std::vector<double>>* threshold_output)
{
	// creates an integer for user input
	int input_value = 0;

	// asks user for input regarding their comparison
	print("Iteration: " + std::to_string(trial_list.GetIterationNumber()));
	print("Could you detect the cue? 1 for yes, 2 for no.....");
	
	std::vector<Key> input_keys = { Key::Num1, Key::Numpad1, Key::Num2, Key::Numpad2 };
	Keyboard::wait_for_any_keys(input_keys);
	
	if (Keyboard::is_key_pressed(Key::Num1) || Keyboard::is_key_pressed(Key::Numpad1))
	{
		input_value = 1;
		print("You typed " + std::to_string(input_value));
	}
	else if (Keyboard::is_key_pressed(Key::Num2) || Keyboard::is_key_pressed(Key::Numpad2))
	{
		input_value = 2;
		print("You typed " + std::to_string(input_value));
	}

	// add current row for ABS testing to the buffer
	std::vector<double> output_row = { 
		(double)trial_list.GetIterationNumber(),	(double)trial_list.GetConditionNum(),
		(double)trial_list.GetAngleIndex(),			(double)trial_list.GetInterferenceAngle(),
		(double)trial_list.GetAngleNumber(),		(double)input_value 
	};
	threshold_output->push_back(output_row);
}

/*
Advances to the next condition or exits test based on
experimenter input.
*/
void AdvanceExperimentCondition()
{
	if (stop) return;

	// moves to next condition if next condition exists
	if (!trial_list.HasNextCondition())
	{
		print("All conditions have been completed...");
		stop = true;
	}

	// creates space for next statement
	print("");

	// creates input value to ask experimenter if trial should continue to next condition
	int input_value = 0;
	print("Please register a save to exit or input CONFIRM_VALUE to continue to next condition...");
	std::cin >> input_value;

	// loops until a proper response is given
	while(!stop && input_value != kConfirmValue)
	{ 
		print("Please register a save to exit or input CONFIRM_VALUE to continue to next condition...");
		std::cin >> input_value;
	}

	// creates space for next statement
	print("");

	// advances experiment to next trial
	trial_list.NextCondition();
}


/***********************************************************
************* MAIN USER INTERACTION FUNCTIONS **************
************************************************************/
/*
Asks the experimenter for the subject number. Then, if
relevant, imports trialList and ABS file from previous
experiment
*/
void RunImportUI(std::vector<std::vector<double>>* threshold_output)
{
	ImportSubjectNumber();
	ImportTrialList();
	ImportRecordABS(threshold_output);
}

/*
Run a single condition on a user automatically. If
experimenter enters the exit value, exits the program.
*/
void RunExperimentUI(DaqNI &daq_ni,
					 AtiSensor &ati_a,	 AtiSensor &ati_b,
					 MaxonMotor &motor_a, MaxonMotor &motor_b,
					 std::vector<std::vector<double>>* threshold_output)
{
	// defines positions of the currrent test cue
	std::array<std::array<double, 2>, 2> position_desired;

	// prints current condition for participant/experimenter 
	int input_value = 0;
	print("Current Condition: " + trial_list.GetConditionName());

	// waits for confirmation before continuing
	while(input_value != kConfirmValue)
	{	
		print("Insert CONFIRM_VALUE when you are ready to begin condition");
		std::cin >> input_value;
	}

	// runs trials on the selected condition with data collection
	while (trial_list.HasNextAngle())
	{
		// check for exit condition
		if (stop) return;

		// get next experiment cue
		trial_list.GetTestPositions(position_desired);

		// prints current desired test position for debug purposes
		print(position_desired[0]);
		
		// provides cue to user
		RunMovementTrial(position_desired, daq_ni, ati_a, ati_b, motor_a, motor_b);

		// record ABS trial response
		RecordExperimentABS(threshold_output);

		// moves experiment to the next trial within current condition
		trial_list.NextAngle();
	}

	// check for exit condition before final cue
	if (stop) return;

	// get final experiment cue
	trial_list.GetTestPositions(position_desired);

	// provides final cue of condition to user
	RunMovementTrial(position_desired, daq_ni, ati_a, ati_b, motor_a, motor_b);

	// record final ABS trial response
	RecordExperimentABS(threshold_output);
}

/*
Saves the ABS data file as well as the trialList given
to the participant.
*/
void RunExportUI(std::vector<std::vector<double>>* threshold_output)
{
	// defining the file name for the ABS data file
	std::string filename = "/sub" + std::to_string(subject) + "_ABS_data.csv";
	std::string filepath = kDataPath + "/data/ABS" + filename;

	// builds header names for threshold logger
	const std::vector<std::string> header_names = 
	{ 
		"Iteration",			"Condition",
		"AngCurr",				"Interference Angle",
		"Test Angle",			"Detected (1=Detected 2=Not Detected)"
	};

	// saves the ABS data
	csv_write_row(filepath, header_names);
	csv_append_rows(filepath, *threshold_output);

	// information about the current trial the test was exited on
	print("Test Saved @ ");
	print("Iteration: " + std::to_string(trial_list.GetIterationNumber()));
	print("Condition:" + std::to_string(trial_list.GetConditionNum()) + " - " 
		+ trial_list.GetConditionName());
	print("Angle:" + std::to_string(trial_list.GetAngleIndex()) + " - "
		+ std::to_string(trial_list.GetAngleNumber()));

	// exporting the trialList for this subject
	// defining the file name for the ABS data file
	filename = "/sub" + std::to_string(subject) + "_trialList.csv";
	filepath = kDataPath + "/data/trialList" + filename;
	trial_list.ExportList(filepath, kTimestamp);
}


/***********************************************************
********************* MISC FUNCTIONS ***********************
************************************************************/
/*
Control C handler to cancel the program at any point and save all data to that point
*/
bool MyHandler(CtrlEvent event) {
	if (event == CtrlEvent::CtrlC) {
		print("Save and exit registered");
		print("");
		stop = true;
	}
	return true;
}

/***********************************************************
****************** STAIRCASE FUNCTIONS *********************
************************************************************/
void RunStaircaseUI(DaqNI &daq_ni,
					AtiSensor &ati_a,	 AtiSensor &ati_b,
					MaxonMotor &motor_a, MaxonMotor &motor_b)
{
	// define relevant variable containers for desire position
	std::array<std::array<double, 2>, 2> position_desired;

	// prompt user with options
	print("Please select desired condition to test:");
	print("0) Stretch with no interference and minimum distance between cues");
	print("1) Stretch with no interference and medium distance between cues");
	print("2) Stretch with no interference and maximum distance between cues");
	print("3) Stretch with low squeeze interference and minimum distance between cues");
	print("4) Stretch with low squeeze interference and medium distance between cues");
	print("5) Stretch with low squeeze interference and maximum distance between cues");
	print("6) Stretch with high squeeze interference and minimum distance between cues");
	print("7) Stretch with high squeeze interference and medium distance between cues");
	print("8) Stretch with high squeeze interference and maximum distance between cues");
	print("9) To randomly go through all conditions");
	print("CTRL+C) To end staircase protocol");

	// recieve user input
	int input_value = -1;
	std::cin >> input_value;

	// run specific condition based on user input
	if(input_value >= 0 && input_value < 9)
	{
		staircase.SetConditionNum(input_value);
		print(staircase.GetConditionName());
		while(!staircase.HasSettled())
		{
			staircase.GetTestPositions(position_desired);
			RunMovementTrial(position_desired, daq_ni, ati_a, ati_b, motor_a, motor_b);
			staircase.ReadInput();
		}
		print("Trial Completed");
	}

	// runs through all conditions with the staircase method
	else if(input_value == 9)
	{
		print(staircase.GetConditionName());
		while(!stop)
		{
			while(!staircase.HasSettled())
			{
				staircase.GetTestPositions(position_desired);
				RunMovementTrial(position_desired, daq_ni, ati_a, ati_b, motor_a, motor_b);
				staircase.ReadInput();
			}
			print("Trial Completed");
			if(staircase.HasNextTrial())
				staircase.NextTrial();
			else if(staircase.HasNextCondition())
			{
				print("Condition Completed");
				staircase.NextCondition();
				print(staircase.GetConditionName());
			}
		}
	}
}


/***********************************************************
********************* MAIN FUNCTION ************************
************************************************************/

/*
Main function of the program references all other functions
*/
int main(int argc, char* argv[])
{
	// registers the mel handler to exit the program using Ctrl-c
	register_ctrl_handler(MyHandler);

	// creates all neccesary objects for the program
	DaqNI					daq_ni;					// creates a new analog input from the NI DAQ
	AtiSensor				ati_a, ati_b;				// create the ATI FT Sensors
	MaxonMotor				motor_a, motor_b;			// create new motors
	std::vector<std::vector<double>>	threshold_output;		// creates pointer to the output data file for the experiment
	
	// Sensor Initialization
	// calibrate the FT sensors 
	ati_a.load_calibration("FT26062.cal");
	ati_b.load_calibration("FT26061.cal");

	// set channels used for the FT sensors
	ati_a.set_channels(daq_ni[{ 0, 1, 2, 3, 4, 5 }]);	 
	ati_b.set_channels(daq_ni[{ 16, 17, 18, 19, 20, 21 }]);

	// zero the ATI FT sensors 
	daq_ni.update();
	ati_a.zero();
	ati_b.zero();
	
	// Motor Initialization
	MotorInitialize(motor_a, (char*)"USB0");
	MotorInitialize(motor_b, (char*)"USB1");

	// Defines and parses console options
    Options options("AIMS_Control.exe", "AIMS Testbed Control");
    options.add_options()
        ("s,staircase", "Opens staircase method control")
        ("h,help", "Prints this Help Message");
    auto input = options.parse(argc, argv);

    // print help message if requested
    if (input.count("h") > 0) {
        print(options.help());
        return EXIT_SUCCESS;
    }

	// runs staircase method protocol if selected
	if (input.count("s") > 0)
	{
		// sets to staircase mode
		staircase_flag = true;

		// imports the current subject number
		ImportSubjectNumber();

		// runs staircase method until directed to exit
		while(!stop)
		{
			RunStaircaseUI(daq_ni, ati_a, ati_b, motor_a, motor_b);
		}

		// exports staircase method output
		std::string filename = "/sub" + std::to_string(subject) + "_data.csv";
		std::string filepath = kDataPath + "/staircase" + filename;
		staircase.ExportList(filepath);
	}

	// runs standard method of constants protocol in all other cases
	else 
	{		 
		// User Interaction Portion of Program
		// import relevant data or creates new data structures
		RunImportUI(&threshold_output);

		// runs ABS experimental protocol automatically
		while (!stop)
		{
			// runs a full condition unless interupted
			RunExperimentUI(daq_ni, ati_a, ati_b, motor_a, motor_b, &threshold_output);

			// advance to the next condition if another condition exists
			AdvanceExperimentCondition();		
		}

		// exports relevant ABS data
		RunExportUI(&threshold_output);
	}

	// informs user that the application is over
	print("Exiting application...");
	
	return EXIT_SUCCESS;
}