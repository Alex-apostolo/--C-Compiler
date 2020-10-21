#include "C.tab.h"
#include <stdio.h>
#include "interpreter.h"

VALUE* interpret(NODE *term, ENV *env) {
  //initialize environment

  switch(term->type) {
    case LEAF:
      //Left child has value: STRING_LITERAL, IDENTIFIER, CONSTANT
      return interpret(term->left,env); 
      break;
    case IDENTIFIER:
    {
      TOKEN* t = term;
      return (TOKEN *) term;
      break;
    }
    case CONSTANT:
      {
        //Term holds its value on the right child
        VALUE *val = term;
        val->v.integer = term->right;
        return val;
        break;
      }
    case STRING_LITERAL:
      {
        //Term holds its value on the right child
        VALUE *val = term;
        val->v.string = term->right;
        return val;
        break;
      }
    case APPLY:
      //Process calling of functions
      break;
    case VOID: case FUNCTION: case INT:
      break;
    case 'd':
      // TODO: hold the data in the environment so we know which funcitons are which


      //Left child is an AST for Return type
      interpret(term->left,env);
      //Right child is the Name and arguments
      interpret(term->right,env);
      break;
    case 'D': 
      //Left child is the Function definition
      interpret(term->left,env);
      //Right child is the Function body
      if(term->right != NULL) {
        return interpret(term->right,env);
      } else {
        return 0;
      }
      break;
    case 'F':
      //Left child is the Name of function
      interpret(term->left,env);
      //Right child are the Arguments of function 
      if(term->right != NULL) {
        interpret(term->right,env);
      }
      break;
    case CONTINUE: case BREAK:
      break;
    case RETURN:
      //Left child is an AST of the expression whose value is to be returned
      // TODO: handle the case where left child is identifier and the case for everything else
      if(term->left != NULL) {
        if(term->left->type == LEAF && term->left->left->type == IDENTIFIER) {
          return find_name(interpret(term->left,env),env->frames);
        } else 
          return interpret(term->left,env); 
      } else{
        // TODO: throw an error
        printf("Error no return type\n");
      }
      break;
    case '~':
      //Left child is the type 
      interpret(term->left,env);
      //Right child is the variable name
      if(term->right->type == LEAF){
        declare(interpret(term->right,env),env->frames);
      } else {
        //Right child is the AST "=" and we declare the variable before assigning
        declare(interpret(term->right->left,env),env->frames);
        interpret(term->right,env);
      }
      break;
    case ';':
      //Left child is the first item or a sequence; returned values are ignored
      interpret(term->left,env);
      //Right child is the last item and also the return value
      return interpret(term->right,env);
      break;
    case '=':
      assign_value(interpret(term->left,env),env->frames,interpret(term->right,env));
      break;
    case '+': case '-': case '*': case '/': case '%': case '>': case '<': case NE_OP: case EQ_OP: case LE_OP: case GE_OP:
      // TODO: make another c file with methods handling this cases
      //operations return
      //should i dissalocate the memory or no?
      // TODO: check if left type matches right type
      {
        VALUE* cons1 = interpret(term->left,env);
        VALUE* cons2 = interpret(term->right,env);
        VALUE* result = malloc(sizeof(VALUE*));
        if(cons1 != NULL && cons2 != NULL && cons1->type != NULL && cons2->type != NULL) {
          result->v.integer = cons1->v.integer + cons2->v.integer;
          return result;
        }
        return -1;
      }
      break;
    case IF:
      break;
    case WHILE:
      break;
    default:
      break;
  }
}

VALUE* find_name(TOKEN* t ,FRAME* frame) {
  while ( frame != NULL ) {
    BINDING * bindings = frame -> bindings ;
    while ( bindings != NULL ) {
      if ( bindings->name == t){
        VALUE *temp = bindings->val;
        return temp;
      } 
      bindings = bindings -> next ;
    }
    frame = frame -> next ;
  }
  //error(" unbound variable ");
}

VALUE* assign_value(TOKEN* t ,FRAME* frame ,VALUE* value) {
  while ( frame != NULL ) {
    BINDING * bindings = frame -> bindings ;
    while ( bindings != NULL ) {
      if ( bindings -> name == t){
        bindings->val = value; 
        return value;
      }
      bindings = bindings -> next ;
    }
    frame = frame -> next ;
  }
  //error(" unbound variable");
}

VALUE* declare( TOKEN * x , FRAME * frame ) {
  BINDING * bindings = frame -> bindings ;
  BINDING * new_bind = malloc(sizeof(BINDING));
  if ( new_bind !=0) { 
    new_bind -> name = x;
    new_bind -> val =( VALUE *)0;
    new_bind -> next = bindings ;
    frame -> bindings = new_bind ;
    return ( VALUE *)0; 
  }
  //error (" binding allocation failed " );
}
