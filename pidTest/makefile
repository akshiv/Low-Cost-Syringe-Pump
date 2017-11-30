CC=gcc
CFLAGS=-lwiringPi -lm -I.
DEPS = setup.h interrupt.h
OBJ = pidTest.o setup.o interrupt.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

pidTest: $(OBJ)
	gcc -Wall -o $@ $^ $(CFLAGS) 
