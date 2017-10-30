/*
* Frequency Counter Code for RPi0
*/

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <errno.h>
#include <time.h>
#include <iostream.h>
#include <string.h>
#include <stdint.h>

#define LOW 0
#define HIGH 1
#define COUNTER_PIN 0

#define SAMPLING_NUMBER 10

void counterInterrupt(void);

volatile int timeDiff = 0;

int main(void) {
	wiringPiSetup();

	wiringPiISR(COUNTER_PIN, INT_EDGE_RISING, counterInterrupt);

	while(TRUE){
		delay(500);
		std::cout << timeDiff << std::endl;
	}
	return 0;
}

void counterInterrupt(void){
	static uint8_t	counter = 0;
	static int oldTime;
	oldTime = micros();
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
}
