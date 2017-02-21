#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "primeField.h"

void test_primeField();

int main() {

	printf("Testing primeField: \n");

    test_primeField();

    return 0;
}

void test_primeField() {
    gf257_element_t a;
    gf257_init(&a);
    gf257_randelement(&a.super);
    printf("%d\n",a.contents);

    gf257_element_t b;
    gf257_init(&b);
    gf257_randelement(&b.super);
    printf("%d\n",b.contents);

    gf257_element_t c;
    gf257_init(&c);
    gf257_randelement(&c.super);
    printf("%d\n",c.contents);

    gf257_element_t result;
    gf257_init(&result);

    gf257_add((element_t*)(&a), (element_t*)(&b), (element_t*)&result);
    printf("%d\n",result.contents);

}