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

VALUE* interpret(NODE*);

