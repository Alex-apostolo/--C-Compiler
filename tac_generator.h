#ifndef __TAC_G
#define __TAC_G
#include "token.h"
#include "nodes.h"

#define BLOCK_OP 278
#define LOAD_OP 279
#define STORE_OP 280

typedef struct expr {
    char* src1;
    char* src2;
    char* dst;
} EXPR;

typedef struct call {
    TOKEN * name ; int arity ;
} CALL ;

typedef struct block {
    int *nvars ;
} BLOCK ;

typedef struct load {
    char *treg ;
    char *value;
} LOAD;

typedef struct store {
    char *treg;
    char *value;
} STORE;

typedef struct tac {
    int op ;
    union { BLOCK block ; CALL call ; LOAD load; STORE store; EXPR expr; char *ret; } args ;
    struct tac * next ;
} TAC ;

void printTAC(TAC *);
void *tac_generator(NODE *,TAC **);
char* my_itoa(int);

#endif