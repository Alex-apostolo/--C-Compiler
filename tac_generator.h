#ifndef __TAC_G
#define __TAC_G
#include "token.h"
#include "nodes.h"
#include <stdio.h>

#define BLOCK_OP 278
#define LOAD_OP 279
#define STORE_OP 280
#define TREG 281

/*Global Variables*/
extern int ntreg;
extern char *latest_treg;
extern int nvars;
extern char *svars[];

typedef struct expr {
    char* src1;
    char* src2;
    char* dst;
} EXPR;

typedef struct proc {
    TOKEN *name;
    int arity;
} PROC;

// INCLUDE 
typedef struct call {
    TOKEN * name ; int arity ;
} CALL ;

typedef struct block {
    int nvars ;
} BLOCK ;

typedef struct load {
    char *treg ;
    int type;
    union {char *identifier ; char *constant ;} val;
} LOAD;

typedef struct store {
    char *treg;
    char *value;
} STORE;

typedef struct ret {
    int type;
    union {char *identifier ; int constant ; char *treg ;} val;
} RET;

typedef struct if_ {
    char *antecedent;
    char *label;
} IF_;

typedef struct goto_ {
    char *label;
} GOTO;

typedef struct label {
    char *name;
} LABEL;

typedef struct tac {
    int op ;
    union { BLOCK block ; CALL call ; LOAD load; PROC proc; STORE store; EXPR expr; RET ret; IF_ if_; GOTO goto_; LABEL label;} args ;
    struct tac * next ;
} TAC ;

typedef struct bb {
    TAC **leader;
    struct bb *next;
} BB;

void printTAC(FILE *,TAC *);
BB *tac_generator(NODE *,TAC **);
char *treg_generator();
char* my_itoa(int);

#endif