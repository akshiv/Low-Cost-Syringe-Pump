/*
* Frequency Counter Code for RPi0
*/

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define LOW 0
#define HIGH 1
#define COUNTER_PIN 0

void counterInterrupt(void);

volatile uint32_t counter = 0;

int main(void) {
	wiringPiSetup();

	wiringPiISR(COUNTER_PIN, INT_EDGE_RISING, counterInterrupt);

	while(TRUE){
		delay(1000);
		printf("Count is: %d\n", counter);
		counter = 0;
	}
	return 0;
}

void counterInterrupt(void){
	counter++;
}
