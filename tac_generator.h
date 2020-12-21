#ifndef __TAC_G
#define __TAC_G
#include "C.tab.h"
#include "nodes.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

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
#define AREG 290

typedef struct expr {
    char *src1;
    char *src2;
    char *dst;
} EXPR;

typedef struct proc {
    char *name;
    int arity;
} PROC;

typedef struct var {
    char *name;
    int type;
    struct var *next;
} VAR;

typedef struct call {
    char *name;
    int arity;
    char *store;
    int *nvars;
    VAR **svars;
} CALL;

typedef struct load {
    char *treg;
    int type;
    union {
        char *identifier;
        char *constant;
    } val;
} LOAD;

typedef struct store {
    char *treg;
    char *value;
} STORE;

typedef struct ret {
    int type;
    union {
        char *identifier;
        int constant;
        char *treg;
    } val;
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
    int type;
    union {
        int integer;
        int boolean;
        char *string;
    } v;
} VAL;

typedef struct global {
    char *name;
    int type;
    VAL *val;
} GLOBAL;

typedef struct clos {
    NODE *body;
    char *name;
} CLOS;

typedef struct tac {
    int op;
    union {
        CALL *call;
        LOAD *load;
        PROC *proc;
        CLOS *clos;
        GLOBAL *glob;
        STORE *store;
        EXPR *expr;
        RET *ret;
        IF_ *if_;
        GOTO *goto_;
        LABEL *label;
    } args;
    struct tac *next;
} TAC;

typedef struct env_tac {
    int ntreg;
    char *latest_treg;
    int nlabel;
    char *latest_label;
    int arguments;
    int *nvars;
    VAR **svars;
} ENV_TAC;

typedef struct bb {
    TAC **leader;
    struct bb *next;
} BB;

void printTAC(FILE *, BB *);
void tac_generator(NODE *, TAC **);
char *treg_generator();
char *my_itoa(int);

#endif