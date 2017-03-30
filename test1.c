#include <stdlib.h>
#include "ssalg_multi.h"
#include "remicss/ss.h"
#include "remicss/hexdump.h"

int main(int argc, char **argv)
{
	//132
	//131 segfaults
	srand(atoi(argv[1]));

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

	
	//int many;
	//for(many = 0; many <6; many++){
		ssalg_multi_init(&tester, 5, 5);
		ssalg_split(&tester.super, "hello", 5, myArray);


		for(i = 0; i < 5; i++) {
			printf("%p \n", myArray[i]->data);
			hexdump(stderr, myArray[i]->data, myArray[i]->len);
		}
	//}

	return 0;

}