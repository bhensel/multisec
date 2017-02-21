#include <limits.h>
#include "element.h"


typedef struct gf257_element {
	element_t super;
    int contents;
} gf257_element_t;


void gf257_add(element_t* a, element_t* b, element_t* result);
void gf257_mult(element_t* a, element_t* b, element_t* result);
void gf257_add_inv(element_t* a, element_t* result);
void gf257_mult_inv(element_t* a, element_t* result);
void gf257_add_id(element_t* result);
void gf257_mult_id(element_t* result);
void gf257_randelement(element_t* result);
void gf257_init(gf257_element_t* a);
void gf257_set(int a, gf257_element_t* result);