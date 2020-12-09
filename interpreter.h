#ifndef __INTERPRETER_H
#define __INTERPRETER_H
#include "nodes.h"

typedef struct stack STACK;

typedef struct closure CLOSURE;

typedef struct value {
  int type ;
  union {
    int integer ;
    int boolean ; 
    char * string ;
    CLOSURE *closure;
  } v;
} VALUE; 

typedef struct binding {
  TOKEN *name ;
  VALUE *val ;
  struct binding *next ; 
} BINDING ;

typedef struct frame {
  BINDING *bindings ;
} FRAME ;

typedef struct closure {
  NODE *code ; 
  FRAME *env ; 
} CLOSURE ;

typedef struct env {
  STACK *stack;
  FRAME *global;
} ENV ;

VALUE* interpret(NODE*,ENV*);
void print_bindings(FRAME *);

#endif