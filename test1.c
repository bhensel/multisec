#include <stdlib.h>
#include "ssalg_multi.h"
#include "remicss/ss.h"
#include "remicss/hexdump.h"
#include "string.h"
#define CHARCOUNT 500

int main(int argc, char **argv)
{
	//CHARCOUNT causes problems if greater than 88
	//132
	//131 segfaults
	//srand(atoi(argv[1]));

	srand(132);

	struct ss_packet array[5];
	//struct ss_packet *array = malloc(5 * sizeof(array[0]));

	struct ss_packet *myArray[5];
	//struct ss_packet **myArray = malloc(5 * sizeof(myArray[0]));

	int i;

	for(i = 0; i<5; i++){
		myArray[i] = &array[i];
		myArray[i]->len = 1500; 
	}

	struct ssalg_multi tester;

	uint8_t charBuf[CHARCOUNT];
	memset(charBuf, 'x', CHARCOUNT);
	
	//int many;
	//for(many = 0; many <6; many++){
		ssalg_multi_init(&tester, 5, 5);
		ssalg_split(&tester.super, charBuf, CHARCOUNT, myArray);


		for(i = 0; i < 5; i++) {
			printf("%p \n", myArray[i]->data);
			hexdump(stderr, myArray[i]->data, myArray[i]->len);
		}

		uint8_t returnBuf[CHARCOUNT];
		size_t returned;
		returned = ssalg_recombine(&tester.super, returnBuf, CHARCOUNT, myArray);

		printf("returned: \n");
		hexdump(stderr, returnBuf, returned);
		printf("\n");
	//}

	return 0;

}