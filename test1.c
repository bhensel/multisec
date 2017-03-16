#include <stdlib.h>
#include "ssalg_multi.h"
#include "remicss/ss.h"
#include "remicss/hexdump.h"

int main()
{
	srand(12);

	struct ss_packet array[5];

	struct ss_packet *myArray[5];
	int i;

	for(i = 0; i<5; i++)
		myArray[i] = &array[i];

	struct ssalg_multi tester;

	ssalg_multi_init(&tester, 5, 5);

	ssalg_split(&tester.super, "hello", 5, myArray);


	for(i = 0; i < 5; i++)
		hexdump(stderr, myArray[i]->data, myArray[i]->len);

	return 0;

}