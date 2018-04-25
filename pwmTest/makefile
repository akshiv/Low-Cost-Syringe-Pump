CC=gcc
CFLAGS=-lwiringPi -lm -I.
DEPS = setup.h interrupt.h
OBJ = pwmTest.o setup.o interrupt.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

pwmTest: $(OBJ)
	gcc -Wall -o $@ $^ $(CFLAGS)
