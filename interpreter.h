#ifndef __INTERPRETER_H
#define __INTERPRETER_H
#include "nodes.h"

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
  int index;
  BINDING *bindings ;
  struct frame *next ; 
} FRAME ;

typedef struct closure {
  NODE *code ; 
  FRAME *env ; 
} CLOSURE ;

typedef struct bb {
  FRAME *frame_leader ;
} BB ;

VALUE* interpret(NODE*,BB*);
VALUE* interpret_(NODE*,BB*);
void print_bindings(FRAME *);
VALUE* find_ident_value(TOKEN*,FRAME*);
FRAME *find_frame(int,BB*);
VALUE* assign_value(TOKEN*,FRAME*,VALUE*);
void declare(TOKEN*,FRAME*);

#endif