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

// Setup constants for control
#define KD 0   							// Derivative Control Constant 
#define KP 2  							// Proportional Control Constant 
#define KI 1							// Integral Control Constant 
#define WINDUP_THRESHOLD 200 			// Used to avoid sudden changes
#define MAX_CORRECTION 10000 			// ?
#define BLOWOFF_TIME 100 				// ?
#define STEP_UP 20						// Per cycle of the PWM rate
#define STEP_UP_MAX 25
#define STEP_DOWN 20
#define STEP_DOWN_MAX 50
#define PWM_LIMIT 450

void exitHandler(int);
int getError(int);
int controlPump(int, int, int);
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
	delay(1000);
	// Set up targets for control
	int target = setTarget(rate);
	int totalTarget = setTotal(volume);
	int error;
	int total = 0;
	
	#ifdef SET_PWM_TEST
	digitalWrite(VALVE_PIN,HIGH);
	delay(1000);
	pwmWrite(PUMP_PIN,power);
	#endif		
	
	printf("Target Rate: %d, Target Total: %d\n\n", target, totalTarget);
	delay(5000);
	while(total < totalTarget){
		printf("Feedback value: %d\n",timeDiff);
		#ifndef SET_PWM_TEST
		error = getError(target);
		total = controlPump(error, total, target);
		#endif
	}
	pwmWrite(PUMP_PIN, PWM_MIN);
	digitalWrite(VALVE_PIN, LOW);
	return 0;
}

// Handles the proper shutdown of the code 
void exitHandler(int sig){
	digitalWrite(VALVE_PIN,LOW);
	pwmWrite(PUMP_PIN,PWM_MIN);
	exit(0);
}

// getErro: Takes the target count value per measurement interval, then counts over the 
// specified interval to determine the error over the interval. The error is returned
int getError(int target){
	int change = 0;
	int valOld = timeDiff;
	while(timeDiff == valOld){}
	change = -1*(timeDiff - valOld);
	// Use the counts read in from the timeDiff value
	// Maybe add some sort of jump rejection to deal with setpoint shifts from movement
	int error = change - target;
	printf("Change: %d\n", change);
	return error;
}

// controlPump: Takes the error over an interval and controls the air pump and valve in
// response to this error. Returns the accumulated total count. 
int controlPump(int error, int total, int target){
	static int integralError = 0;
	static int prevError = 0;

	total = total - (error - target);
	int derivError = error - prevError;
	integralError = integralError + error; 
	if (integralError > WINDUP_THRESHOLD){
		integralError = WINDUP_THRESHOLD;
	} else if(integralError < -1*WINDUP_THRESHOLD){
		integralError = -1*WINDUP_THRESHOLD;
	}

	int correction = KP*error + KD*derivError + KI*integralError;
	printf("Error: %d, Integral Error: %d, Correction: %d\n", error, integralError, correction);	
	drivePump(correction, target);
	prevError = error;
	return total;
}


// drivePump: controls the valve and pump to correctly react to a correction
void drivePump(int correction, int target){
	static int pwmCurrent 	= 0;
	int step = 0;
	if(correction < 0){
	// This means we need to pump more to catch up as we are lagging
		printf("Push\n\n");
		step = -1*correction;
		if(step > STEP_UP_MAX){
			step = STEP_UP_MAX;
		}
			pwmCurrent = pwmCurrent + step; 
		if (pwmCurrent > PWM_LIMIT){
			pwmCurrent = PWM_LIMIT;
		}
		pwmWrite(PUMP_PIN, pwmCurrent);
		digitalWrite(VALVE_PIN, HIGH);
	} else if (correction > 1000*target){
	// If the pump is moving very fast we open the valve to blow off some air
		printf("Blowoff\n\n");
		pwmWrite(PUMP_PIN, PWM_MIN);
		digitalWrite(VALVE_PIN, LOW);
		delay(BLOWOFF_TIME);
		digitalWrite(VALVE_PIN, HIGH);
		pwmCurrent = 0;
	} else {
	// If the pump is slighly too fast we stop pumping
		step = correction;
		if(step > STEP_DOWN_MAX){
			step = STEP_DOWN_MAX;
		}
		printf("Wait\n\n");
		pwmCurrent = pwmCurrent - step;
	  	if(pwmCurrent < 0){
			pwmCurrent = 0;
		}
		pwmWrite(PUMP_PIN, pwmCurrent);
		digitalWrite(VALVE_PIN, HIGH);
	}
	printf("PWMCurrent: %d\n", pwmCurrent);
	return;
}

// Rounds to nearest integer
double round(double number){
	return (number - floor(number) >= 0.5) ? ceil(number) : floor(number);
}

