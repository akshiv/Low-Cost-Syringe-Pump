/*
* PID Test Code for RPi0
* Written By: Akshiv Bansal & Ian Thompson
* Project: $20 Syringe Pump for Digital Health Innovation Lab
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include "setup.h"
#include "interrupt.h"

#define NUM_AVERAGING 5			// Number of readings to average over for determining initial reading

// Constants for error control
#define KD 0   							// Derivative Control Constant 
#define KP 10  							// Proportional Control Constant 
#define KI 0	 // Integral Control Constant 
#define KDIV 5 // Weighting constant
#define WINDUP_THRESHOLD 20000 			// Used to avoid sudden changes

// Constants for pump driving
#define STEP_UP 20						// Per cycle of the control loop
#define STEP_UP_MAX 25
#define STEP_DOWN 20
#define STEP_DOWN_MAX 50
#define PWM_LIMIT 400
#define BLOWOFF_TIME 100 				// Controls the duration of valve opening for releasing pressure

void exitHandler(int);
int getInitialReading(int);
int getError(int);
void controlPump(int, int);
void drivePump(int, int);
double round(double);

volatile int timeDiff = 0;

int main(int argc, char **argv) {
	// Read in the specified rate and volume parameters
	#ifdef SET_PWM_TEST
	if(argc != 4){
		fprintf(stderr, "Error: Need to input arguments as Flow rate, then total volume.\n");
		exit(-1);
	}
	int power = atoi(argv[3]); // Get power to push the pump with
	// Check to make sure the power is in range
	if(power < PWM_MIN || power > PWM_MAX){
		fprintf(stderr, "Error: Power target out of range.\n");
		exit(-1);
	}
	#else
	if(argc != 3){
		fprintf(stderr, "Error: Need to input arguments as Flow rate, then total volume.\n");
		exit(-1);
	}
	#endif
	int rate = atoi(argv[1]); // Get flow rate in uL/h
	// Check to make sure the flow rate is in range
	if( rate < MIN_RATE || rate > MAX_RATE ){
		fprintf(stderr, "Error: Flow rate out of range.\n");
		exit(-1);
	}
	int volume = atoi(argv[2]); // Get total volume goal in uL
	// Check to make sure the volume is in range
	if(volume < MIN_VOLUME || volume > MAX_VOLUME){
		fprintf(stderr, "Error: Volume target out of range.\n");
		exit(-1);
	}

	// Setup shutdown protocol
	signal(SIGINT, exitHandler);

	// Configure Raspberry Pi
	rPiSetup();
	delay(6000);

	// Average over some samples of reading to get start value
	int initialReading = getInitialReading(NUM_AVERAGING);
	printf("initial Reading: %d\n", initialReading);
	// Set up targets for control
	int increment = setTarget(rate, initialReading);
	int totalTarget = setTotal(volume, initialReading);
	int error;
	int total = 0;
	int loopCount = 0;
	int target;
	
	#ifdef SET_PWM_TEST
	// If testing pump response to different driving PWM, this mode simply lets the air pump 
	// run at a specified PWM
	digitalWrite(VALVE_PIN,HIGH);
	delay(1000);
	pwmWrite(PUMP_PIN,power);
	#endif		
	
	// Display the setup info, then wait to let feedback counter settle 
	// Without this delay, the first feedback value is often incorrect and large
	printf("Target Rate: %d, Target Total: %d\n\n", increment, totalTarget);

	// Run this loop until we have infused the desired total volume
	// May need to debounce in some way to avoid exiting this loop from noise
	while(timeDiff > totalTarget){
		printf("Feedback value: %d\n",timeDiff);
		loopCount++;
		target = initialReading - increment * loopCount;
		// Compute errors and apply control feedback for each iteration
		#ifndef SET_PWM_TEST
		error = getError(target);
		controlPump(error, increment);
		#endif
	}

	// Once infusion is finished, turn all pumping off
	pwmWrite(PUMP_PIN, PWM_MIN);
	digitalWrite(VALVE_PIN, LOW);
	return 0;
}

// Handles the proper shutdown of the code, setting the pump off and valve open when ctrl+C is
// used to stop the code.
void exitHandler(int sig){
	digitalWrite(VALVE_PIN,LOW);
	pwmWrite(PUMP_PIN,PWM_MIN);
	exit(0);
}

// Obtains the feedback value for the syringe's initial position
int getInitialReading(int numAveraging){
	int sum = 0;
	int oldVal = timeDiff;
	// Sum up distinct feedback readings
	for(int i = 0; i < numAveraging; i++){
		while(timeDiff == oldVal){}
		sum += timeDiff;
		oldVal = timeDiff;
	}
	return sum/numAveraging;
}

// getError: Takes the target count value per measurement interval, then counts over the 
// specified interval to determine the error over the interval. The error is returned
int getError(int target){
	int valOld = timeDiff;

	// Wait for a change in the feedback read-in then record this change
	while(timeDiff == valOld){}
	// Maybe add some sort of jump rejection to deal with setpoint shifts from movement
	
	// Calculate the error by comparing the real and target values
	// If timeDiff-target is positive, pump is lagging
	return (timeDiff-target);
}

// controlPump: Takes the error over an interval and controls the air pump and valve in
// response to this error. Returns the accumulated total count. 
void controlPump(int error, int increment){
	static int integralError = 0;
	static int prevError = 0;

	// Calculate the derivative and integral error (with wind-up limits)
	int derivError = error - prevError;
	integralError = integralError + error; 
	if (integralError > WINDUP_THRESHOLD){
		integralError = WINDUP_THRESHOLD;
	} else if(integralError < -1*WINDUP_THRESHOLD){
		integralError = -1*WINDUP_THRESHOLD;
	}

	// Calculate & output the final correction based on the errors
	int correction = (KP*error + KD*derivError + KI*integralError)/KDIV;
	printf("Error: %d, Integral Error: %d, Correction: %d\n", error, integralError, correction);	
	drivePump(correction, increment);
	prevError = error;
}


// drivePump: controls the valve and pump to correctly react to a correction
void drivePump(int correction, int increment){
	static int prevBlowoff = 0;
	// Based on the correction values we apply different actions to the pump
	if(correction > 0){
	// If the pump is lagging, we need to pump more to catch up as we are lagging
		printf("Push\n\n");
		
		// Apply a limit on the maximum pump speed
		if (correction > PWM_LIMIT){
			correction = PWM_LIMIT;
		}
		pwmWrite(PUMP_PIN, correction);
		digitalWrite(VALVE_PIN, HIGH);
		prevBlowoff = 0;
	} else if (correction < -5*increment){
	// If the pump is moving too fast we open the valve to release pressure
		printf("Blowoff\n\n");
		if(!prevBlowoff){
			pwmWrite(PUMP_PIN, PWM_MIN);
			digitalWrite(VALVE_PIN, LOW);
			delay(BLOWOFF_TIME);
			digitalWrite(VALVE_PIN, HIGH);
			prevBlowoff = 1;
		}
	} else {
	// If the pump is slighly too fast we slow the pump speed, proportionally to the correction
		printf("Wait\n\n");
		correction = PWM_MIN;
		pwmWrite(PUMP_PIN, correction);
		digitalWrite(VALVE_PIN, HIGH);
		prevBlowoff = 0;
	}
	printf("Applied Correction: %d\n", correction);
	return;
}

// Rounds to nearest integer
double round(double number){
	return (number - floor(number) >= 0.5) ? ceil(number) : floor(number);
}

