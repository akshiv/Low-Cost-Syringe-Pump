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
#include <string.h>
#include <stdint.h>

// Setup pin configurations
#define LOW 0
#define HIGH 1
#define COUNTER_PIN 0
#define PUMP_PIN 1
#define VALVE_PIN 3

#define KD 0
#define KP 0
#define KI 0
#define WINDUP_THRESH 10

#define INFUSION_START 2000
#define INFUSION_END 1000 // Need to set infusion limits based on known limits of values

// Configure rate and volume limits
#define MAX_RATE 200000
#define MIN_RATE 10000
#define MAX_VOLUME 20000
#define MIN_VOLUME 1000

#define SAMPLING_NUMBER 10

void counterInterrupt(void);
void rPiSetup(void);

volatile int timeDiff = 0;

int main(int argc, char **argv) {
	// Read in the specified rate and volume parameters
	if(argc != 3){
		fprintf(stderr, "Error: Need to input arguments as Flow rate, then total volume.\n");
		exit(-1);
	}
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
	// Configure Raspberry Pi
	rPiSetup();

	while(TRUE){
		

	}
	return 0;
}

void counterInterrupt(void){
	static uint8_t	counter = 0;
	static int oldTime = micros();
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
void rPiSetup(void){
	wiringPiSetup();

	wiringPiISR(COUNTER_PIN, INT_EDGE_RISING, counterInterrupt);
	pinMode(PUMP_PIN, PWM_OUTPUT);
	pinMode(VALVE_PIN, OUTPUT);
	return;
}
