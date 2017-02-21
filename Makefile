CFLAGS= -ggdb -Wall -Wextra -std=c99


all: tests

tests: tests.c  primeField.o element.o 
	gcc $(CFLAGS) -c tests.c
	gcc $(CFLAGS) tests.o  primeField.o  element.o -o tests

primeField.o: primeField.c primeField.h element.h
	gcc $(CFLAGS) -c primeField.c

element.o: element.c element.h
	gcc $(CFLAGS) -c element.c

clean:
	rm *.o
