/*
 * Code to setup counter input interrupt
 * Authors: Akshiv Bansal and Ian Thompson
*/

#include "interrupt.h"
#include "setup.h"

extern int timeDiff; 

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
