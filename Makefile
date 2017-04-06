CFLAGS= -ggdb -Wall -Wextra -std=c99 -Wno-unused-parameter -Wno-sign-compare

test1: ssalg_multi.o remicss/ssalg.o remicss/hexdump.o fieldpoly/primeField.o sha1.o fieldpoly/fieldpoly.o fieldpoly/element.o

ssalg_multi.o: ssalg_multi.c ssalg_multi.h sha1.o
	make -C fieldpoly/
	make -C remicss/
	gcc $(CFLAGS) -c ssalg_multi.c


sha1.o: sha1.c sha1.h
	gcc -c sha1.c


clean:
	rm *.o
	rm test1
