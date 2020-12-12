#ifndef __INTERPRETER_H
#define __INTERPRETER_H
#include "nodes.h"

typedef struct stack STACK;
typedef struct closure CLOSURE;

// VALUE struct: used as a return for the interpreter
typedef struct value {
  int type ;
  union {
    int integer ;
    int boolean ; 
    char * string ;
    CLOSURE *closure;
  } v;
} VALUE; 

// BINDING struct: used as a means of saving name to value maps in the environment
typedef struct binding {
  TOKEN *name ;
  VALUE *val ;
  struct binding *next ; 
} BINDING ;

// FRAME struct: used to hold all the bindings of a function(aka frame)
typedef struct frame {
  BINDING *bindings ;
} FRAME ;

// CLOSURE struct: used to hold the params, code and env of functions defined inside other functions
typedef struct closure {
  NODE *params ;
  NODE *code ; 
  FRAME *env ; 
} CLOSURE ;

// ENV struct: contains a stack of the called frame environments, and a global environment
typedef struct env {
  STACK *stack;
  FRAME *global;
} ENV ;

VALUE* interpret(NODE*,ENV*);
// Helper method for debugging, uses -b flag to print the global bindings
void print_bindings(FRAME *);

#endif