CC=gcc
CFLAGS=-lwiringPi -lm -I.
DEPS = setup.h interrupt.h
OBJ = pidPump.o setup.o interrupt.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

pidPump: $(OBJ)
	gcc -Wall -o $@ $^ $(CFLAGS)
