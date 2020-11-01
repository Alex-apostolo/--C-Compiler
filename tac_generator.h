#ifndef __TAC_G
#define __TAC_G
#include "token.h"

#define BLOCK_OP 278
#define LOAD_OP 279
#define STORE_OP 280

typedef struct expr {
    TOKEN* src1;
    TOKEN* src2;
    TOKEN* dst;
} EXPR;

typedef struct call {
    TOKEN * name ; int arity ;
} CALL ;

typedef struct block {
    int *nvars ;
} BLOCK ;

typedef struct load {
    char *treg ;
    int value;
} LOAD;

typedef struct store {
    char *treg;
    char *identifier;
} STORE;

typedef struct tac {
    int op ;
    union { BLOCK block ; CALL call ; LOAD load; STORE store; EXPR expr; char *ret; } args ;
    struct tac * next ;
} TAC ;

void printTAC(TAC *);
#endif