#include "string.h"
#include "element.h"


void f_add(element_t* a, element_t* b) {
   f_addr(a,b,a);
}
void f_mult(element_t* a, element_t* b) {
    f_multr(a,b,a);
}
void f_add_inv(element_t* a) {
    f_add_invr(a,a);
}
void f_mult_inv(element_t* a) {
    f_mult_invr(a,a);
}
void f_add_id(element_t* a) {
    f_add_idr(a);
}
void f_mult_id(element_t* a) {
    f_mult_idr(a);
}
void f_rand(element_t *a) {
    f_randr(a);
}

void f_sub(element_t* a, element_t* b) {
    f_subr(a,b,a);
}
void f_div(element_t* a, element_t* b) {
    f_divr(a,b,a);
}


void f_addr(element_t* a, element_t* b, element_t* result) {
   a->field->add(a,b,result); 
}
void f_multr(element_t* a, element_t* b, element_t* result) {
   a->field->mult(a,b,result); 
}
void f_add_invr(element_t* a, element_t* result) {
   a->field->addinv(a,result); 
}
void f_mult_invr(element_t* a, element_t* result) {
   a->field->multinv(a,result); 
}
void f_add_idr(element_t* result) {
   result->field->addid(result); 
}
void f_mult_idr(element_t* result) {
   result->field->multid(result); 
}
void f_randr(element_t* result) {
    result->field->randelement(result);
}
    


void f_subr(element_t* a, element_t* b, element_t* result) {
   element_t invb;
   f_add_invr(b,&invb);
   f_addr(a,&invb,result); 
}
void f_divr(element_t* a, element_t* b, element_t* result) {
   element_t invb;
   f_mult_invr(b,&invb);
   f_multr(a,&invb,result); 
}

int f_sizeof(element_t* a) {
    return a->size;
        
}

void assign(element_t* a, element_t* b) {
    memcpy((void*) a, (void*) b, b->size);
}
    
