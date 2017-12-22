#ifndef SETUP_H_INCLUDED
#define SETUP_H_INCLUDED

#include <math.h>
#include <wiringPi.h>

// Setup pin configurations
#define LOW 0
#define HIGH 1
#define COUNTER_PIN 0
#define PUMP_PIN 1
#define VALVE_PIN 2
#define PWM_MIN 0
#define PWM_MAX 1024


// Configure syringe information
#define SYRINGE_MAX_READING 2153750
#define SYRINGE_MIN_READING 2117780 // Need to set infusion limits based on known limits of values
#define SYRINGE_VOLUME_UL 22000
#define READING_PER_UL (double) (SYRINGE_MAX_READING-SYRINGE_MIN_READING)/SYRINGE_VOLUME_UL //Estimated as Linear
#define MAX_RATE 200000
#define MIN_RATE 10000
#define MAX_VOLUME 20000
#define MIN_VOLUME 1000

#define SAMPLING_NUMBER 250
#define HR_TO_SEC 0.000277778
#define US_TO_SEC 0.000001

// rPiSetup: Sets up WiringPi, Interrupt, and Output Pins for proper board operation
// The board is setup with the pump off, interrupt enabled and valve open.
void rPiSetup(void);


// setTarget: Takes the specified rate, volume, and averaging interval, and returns the 
// target count value per measurement interval 
int setTarget(int rate, int initialReading);


// setTotal: Takes the specified rate, volume, and averaging interval, and returns the 
// total target count value over the duration of the pump infusion
int setTotal(int volume, int initialReading);

#endif
