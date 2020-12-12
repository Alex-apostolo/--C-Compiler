#ifndef __TAC_G
#define __TAC_G
#include "token.h"
#include "nodes.h"
#include <stdio.h>

#define BLOCK_OP 278
#define LOAD_OP 279
#define STORE_OP 280
#define CALL_OP 281
#define PROC_OP 282
#define GLOBAL_OP 283
#define TREG 284

typedef struct expr {
    char* src1;
    char* src2;
    char* dst;
} EXPR;

typedef struct proc {
    TOKEN *name;
    int arity;
} PROC;

typedef struct call {
    TOKEN * name ; 
    int arity ;
    char *store;
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
    int terminal;
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

typedef struct val {
  int type ;
  union {
    int integer ;
    int boolean ; 
    char * string ;
  } v;
} VAL; 

typedef struct global {
    char *name;
    VAL *val;
} GLOBAL;

typedef struct clos {
    NODE *body;
    char *name;
    // Next is for the Linked list of closures when global initialization occurs
    struct closure *next;
} CLOS;

typedef struct tac {
    int op ;
    union { BLOCK block ; CALL call ; LOAD load; PROC proc; GLOBAL glob; STORE store; EXPR expr; RET ret; IF_ if_; GOTO goto_; LABEL label;} args ;
    struct tac * next ;
} TAC ;

typedef struct bb {
    TAC **leader;
    struct bb *next;
} BB;

void printTAC(FILE *,BB *);
BB *tac_generator(NODE *);
char *treg_generator();
TAC *create_single_TAC_seq(BB *);
char* my_itoa(int);

#endif