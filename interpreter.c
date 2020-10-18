
#include "C.tab.h"
#include <stdio.h>
#include "interpreter.h"

VALUE* interpret(NODE *term) {
  VALUE result;

  switch(term->type) {
    case LEAF:
      //Left child has value: STRING_LITERAL, IDENTIFIER, CONSTANT
      return (VALUE *)interpret(term->left); 
      break;
    case IDENTIFIER:
      break;
    case CONSTANT: case STRING_LITERAL:
      {
        TOKEN *t = (TOKEN *)term;
        return t->value;
        break;
      }
    case APPLY:
      //Left child is the Name of the function(identifier)
      interpret(term->left);
      //Right child are the arguments of the function
      if(term->right != NULL) {
        return interpret(term->right);
      }
      break;
    case VOID: case FUNCTION: case INT:
      break;
    case 'd':
      //Left child is an AST for Return type
      interpret(term->left);
      //Right child is the Function definition
      interpret(term->right);
      break;
    case 'D': 
      //Left child is the Function definition
      interpret(term->left);
      //Right child is the Return value 
      if(term->right != NULL) {
        return interpret(term->right);
      } else {
        return (VALUE *) 0;
      }
      break;
    case 'F':
      //Left child is the Name of function(identifier)
      interpret(term->left);
      //Right child are the Arguments of function 
      if(term->right != NULL) {
        interpret(term->right);
      }
      break;
    case CONTINUE: case BREAK:
      break;
    case RETURN:
      // TODO: RETURN THE VALUE HERE  DONT FORGET
      //Left child is an AST of the expression whose value is to be returned
      if(term->left != NULL) {
        return (VALUE *)interpret(term->left); 
      } else{
        // TODO: throw an error
        printf("Error no return type\n");
      }
      break;
    case '~':
      break;
    case ';':
      break;
    case '=':
      break;
    case '+': case '-': case '*': case '/': case '%': case '>': case '<': case NE_OP: case EQ_OP: case LE_OP: case GE_OP:
      // TODO: make another c file with methods handling this cases
      //operations return
      //should i dissalocate the memory or no?
      // TODO: check if left type matches right type
      return (VALUE*) ((int)interpret(term->left) + (int)interpret(term->right));
      break;
    case IF:
      break;
    case WHILE:
      break;
    default:
      break;
  }
}