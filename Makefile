CC = gcc

CFLAGS = -g

all: mainProc

mainProc: mainProc.o barreira.o myFifo.o
	$(CC) $(CFLAGS) -o mainProc mainProc.c barreira.o myFifo.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f mainProc *.o