/*
 * Code to setup raspberry Pi and flow rates for syringe pump function
 * Authors: Akshiv Bansal and Ian Thompson
*/

#include "setup.h"
#include "interrupt.h"

// rPiSetup: Sets up WiringPi, Interrupt, and Output Pins for proper board operation
/// The board is setup with the pump off, interrupt enabled and valve open.
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
int setTarget(int rate, int initialReading){
	return round(HR_TO_SEC * rate * READING_PER_UL * initialReading * US_TO_SEC);
}

// setTotal: Takes the specified rate, volume, and averaging interval, and returns the 
// total target count value over the duration of the pump infusion
int setTotal(int volume, int initialReading){
	return (initialReading - round(READING_PER_UL * volume));
}

