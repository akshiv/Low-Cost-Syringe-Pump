CC=gcc
CFLAGS=-lwiringPi -lm -I.
DEPS = setup.h interrupt.h
OBJ = pidSimple.o setup.o interrupt.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

pidSimple: $(OBJ)
	gcc -Wall -o $@ $^ $(CFLAGS)
