/*
* Frequency Counter Code for RPi0
*/

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <errno.h>
#include <string.h>

#define LOW 0
#define HIGH 1
#define COUNTER_PIN 0

volatile uint32_t counter = 0;

int main(void) {
	if(wiringPiSetup() < 1){
		fprintf(stderr, "Unable to setup WiringPi.\n", strerror(errno));
	}

	if(wiringPiISR(COUNTER_PIN, INT_EDGE_RISING, counterInterrupt) < 1){
		fprintf(stderr, "Unable to setup Interrupt.\n", strerror(errno));
	}

	while(true){
		delay(1000);
		printf("Count is: %d\n", counter);
		counter = 0;
	}
	return 0;
}

void counterInterrupt(void){
	counter++;
}
