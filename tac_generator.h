#ifndef __TAC_G
#define __TAC_G
#include "token.h"

typedef struct expr {
    TOKEN* src1;
    TOKEN* src2;
    TOKEN* dst;
} EXPR;

typedef struct call {
    TOKEN * name ; int arity ;
} CALL ;

typedef struct block {
    int nvars ;
} BLOCK ;

typedef struct tac {
    int op ;
    union { BLOCK block ; CALL call ; EXPR expr; int ret; } args ;
    struct tac * next ;
} TAC ;

void printTAC(TAC *);
#endif