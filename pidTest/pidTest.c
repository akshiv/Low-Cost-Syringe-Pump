/*
* PID Test Code for RPi0
* Written By: Akshiv Bansal & Ian Thompson
* Project: $20 Syringe Pump for Digital Health Innovation Lab
*/

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

// Setup pin configurations
#define LOW 0
#define HIGH 1
#define COUNTER_PIN 0
#define PUMP_PIN 1
#define VALVE_PIN 2
#define PWM_MIN 0
#define PWM_MAX 1024

// Setup constants for control
#define KD 0
#define KP 10
#define KI 4
#define WINDUP_THRESHOLD 100
#define JUMP_REJECTION_THRESHOLD 10000 // Need to determine and set properly
#define MAX_CORRECTION 10000 
#define BLOWOFF_TIME 200

// Configure syringe information
#define SYRINGE_MAX_READING 930000
#define SYRINGE_MIN_READING 867000 // Need to set infusion limits based on known limits of values
#define SYRINGE_VOLUME_UL 19000
#define READING_PER_UL (double) (SYRINGE_MAX_READING-SYRINGE_MIN_READING)/SYRINGE_VOLUME_UL //Estimated as Linear
#define MAX_RATE 200000
#define MIN_RATE 10000
#define MAX_VOLUME 20000
#define MIN_VOLUME 1000

#define SAMPLING_NUMBER 20
#define AVERAGING_INTERVAL_S 1
#define AVERAGING_INTERVAL_US AVERAGING_INTERVAL_S * 1000000
#define HR_TO_SEC 0.000277778


void exitHandler(int);
void counterInterrupt(void);
void rPiSetup(void);
int setTarget(int);
int setTotal(int);
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
	while(total < totalTarget){
		printf("Feedback value: %d\n",timeDiff);
		#ifndef SET_PWM_TEST
		error = getError(target);
		total = controlPump(error, total, target);
		#endif
	}
	return 0;
}

// Handles the proper shutdown of the code 
void exitHandler(int sig){
	digitalWrite(VALVE_PIN,LOW);
	pwmWrite(PUMP_PIN,PWM_MIN);
	exit(0);
}

// Runs an interrupt to count input pulses from oscillator
void counterInterrupt(void){
	static uint8_t	counter = 0;
	static uint8_t first = 1;
	static int oldTime;
	if(first){
		oldTime = micros();
		first = 0;
	}
	static int newTime;
	counter++;
	if(counter >= SAMPLING_NUMBER){
		newTime = micros();
		if(newTime > oldTime){
			timeDiff = newTime - oldTime;
		}
		oldTime = newTime;
		counter = 0;
	}
	return;
}

// rPiSetup: Sets up WiringPi, Interrupt, and Output Pins for proper board operation
// The board is setup with the pump off, interrupt enabled and valve open.
void rPiSetup(void){
	wiringPiSetup();

	wiringPiISR(COUNTER_PIN, INT_EDGE_RISING, counterInterrupt);
	pinMode(PUMP_PIN, PWM_OUTPUT);
	pwmWrite(PUMP_PIN, PWM_MIN);

	pinMode(VALVE_PIN, OUTPUT);
	digitalWrite(VALVE_PIN, LOW);
	return;
}

// setTarget: Takes the specified rate, volume, and averaging interval, and returns the 
// target count value per measurement interval 
int setTarget(int rate){
	return round(HR_TO_SEC * rate * READING_PER_UL * AVERAGING_INTERVAL_S);
}

// setTotal: Takes the specified rate, volume, and averaging interval, and returns the 
// total target count value over the duration of the pump infusion
int setTotal(int volume){
	return round(READING_PER_UL * volume);
}

// getErro: Takes the target count value per measurement interval, then counts over the 
// specified interval to determine the error over the interval. The error is returned
int getError(int target){
	int change = 0;
	int valOld = timeDiff;
	int timeStart = micros();
	while(micros() < (timeStart + AVERAGING_INTERVAL_US)){
		// Make sure that this condition is what we want to use. Alternatively could
		// force the values to only sum if changes indicate correct direction, but maybe
		// this is bad for back pressure
		if(fabs(timeDiff - valOld) < JUMP_REJECTION_THRESHOLD){
			change = change - (timeDiff-valOld);
		} 
		valOld = timeDiff;
	}
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
	printf("Error: %d, Integral Error: %d\n", error, integralError);	
	drivePump(correction, target);
	prevError = error;
	return total;
}
// drivePump: controls the valve and pump to correctly react to a correction
void drivePump(int correction, int target){
	printf("Correction: %d\n", correction);
	if(correction < 0){
	// This means we need to pump more to catch up as we are lagging
		printf("Push\n\n");
		if (correction < -1*PWM_MAX/2){
			correction = -1*PWM_MAX/2;
		}
		pwmWrite(PUMP_PIN, (-1*correction));
		digitalWrite(VALVE_PIN, HIGH);
	} else if (correction > PWM_MAX){
	// If the pump is moving very fast we open the valve to blow off some air
		printf("Blowoff\n\n");
		pwmWrite(PUMP_PIN, PWM_MIN);
		digitalWrite(VALVE_PIN, LOW);
		delay(BLOWOFF_TIME);
		digitalWrite(VALVE_PIN, HIGH);
	} else {
	// If the pump is slighly too fast we stop pumping
		printf("Wait\n\n");
		pwmWrite(PUMP_PIN, PWM_MIN);
		digitalWrite(VALVE_PIN, HIGH);
	}
}
// Rounds to nearest integer
double round(double number){
	return (number - floor(number) >= 0.5) ? ceil(number) : floor(number);
}

