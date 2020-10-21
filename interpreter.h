#ifndef __INTERPRETER_H
#define __INTERPRETER_H
#include "nodes.h"

typedef struct value {
  int type ;
  union {
    int integer ;
    int boolean ; 
    char * string ;
  } v;
} VALUE; 

typedef struct binding {
  TOKEN *name ;
  VALUE *val ;
  struct binding *next ; 
} BINDING ;

typedef struct frame {
  BINDING *bindings ;
  struct frame *next ; 
} FRAME ;

typedef struct env {
  FRAME *frames ;
} ENV ;


VALUE* interpret(NODE*,ENV*);
VALUE* find_name(TOKEN*,FRAME*);
VALUE* assign_value(TOKEN*,FRAME*,VALUE*);
VALUE* declare(TOKEN*,FRAME*);

#endif