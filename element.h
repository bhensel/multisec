#ifndef ELEMENT_H
#define ELEMENT_H

/* a generic element of a field */
typedef struct element {
    struct field_ops* field;
    int size;
} element_t;

/* a struct containing pointers to all the function that must 
 * be defined for a field to work with this library 
 */
typedef struct field_ops {
    void (*add)(element_t*, element_t*,element_t*);
    void (*mult)(element_t*, element_t*,element_t*);
    void (*addinv)(element_t*,element_t*);
    void (*multinv)(element_t*,element_t*);
    void (*addid)(element_t*);
    void (*multid)(element_t*);
    void (*randelement)(element_t*);
    element_t* (*makezero)(void);
} field_ops_t;


// sets a = a + b in the given field
void  f_add(element_t* a, element_t* b);
// sets a = a * b 
void  f_mult(element_t* a, element_t* b);
// sets a = -a
void  f_add_inv(element_t* a);
// sets a = 1/a
void  f_mult_inv(element_t* a);
// sets a = 0
void f_add_id(element_t* a);
// sets a = 1
void f_mult_id(element_t* a);
/*
 * sets a equal to to a random element of the 
 * field. To keep things secure each element
 * of the field should be equally likely
 */
void f_rand(element_t* a);

// sets a = a - b
void f_sub(element_t* a, element_t* b);
// sets a = a/b
void f_div(element_t* a, element_t* b);


// sets result = a + b
void  f_addr(element_t* a, element_t* b, element_t* result);
// sets result = a + b
void  f_multr(element_t* a, element_t* b, element_t* result);
// sets result = -a
void  f_add_invr(element_t* a, element_t* result);
// sets result = 1/a
void  f_mult_invr(element_t* a, element_t* result);
// sets result = 0 (same as f_add_id)
void f_add_idr( element_t* result);
// sets result = 1 (same as f_mult_id)
void f_mult_idr(element_t* result);
// same as f_rand
void f_randr(element_t* result);

// sets result = a - b
void f_subr(element_t* a, element_t* b, element_t* result);
// sets result = a/b
void f_divr(element_t* a, element_t* b, element_t* result);

void assign(element_t* a, element_t* b);

int f_sizeof(element_t* a);
element_t* make_zero();

#endif
