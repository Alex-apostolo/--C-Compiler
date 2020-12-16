#ifndef __TAC_G
#define __TAC_G
#include "token.h"
#include "C.tab.h"
#include "nodes.h"
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_OP 278
#define LOAD_OP 279
#define STORE_OP 280
#define CALL_OP 281
#define PROC_OP 282
#define GLOBAL_OP 283
#define CLOS_OP 284
#define LABEL_OP 285
#define RET_OP 286
#define IF_OP 287
#define GOTO_OP 288
#define TREG 289

typedef struct expr {
    char* src1;
    char* src2;
    char* dst;
} EXPR;

typedef struct proc {
    char *name;
    int arity;
} PROC;

typedef struct call {
    char * name ; 
    int arity ;
    char *store;
} CALL ;

typedef struct var {
    char *name;
    struct var *next;
} VAR;

typedef struct block {
    int *nvars ;
    VAR **svars;
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
} CLOS;

typedef struct tac {
    int op ;
    union { BLOCK *block ; CALL *call ; LOAD *load; PROC *proc; CLOS *clos; GLOBAL *glob; STORE *store; EXPR *expr; RET *ret; IF_ *if_; GOTO *goto_; LABEL *label;} args ;
    struct tac * next ;
} TAC ;

typedef struct bb {
    TAC **leader;
    struct bb *next;
} BB;

void printTAC(FILE *,BB *);
void tac_generator(NODE *, TAC **);
char *treg_generator();
TAC *create_single_TAC_seq(BB *);
char* my_itoa(int);

#endif