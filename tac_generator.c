#include "nodes.h"
#include "C.tab.h"
#include "tac_generator.h"
#include <stdlib.h>
#include "interpreter.h"

void append(TAC **,TAC *);

TAC *tac_generator(NODE* term, TAC** seq) {
  switch(term->type) {
    case LEAF:
      //Left child has value: STRING_LITERAL, IDENTIFIER, CONSTANT
      return tac_generator(term->left, seq); 
      break;
    case IDENTIFIER:
      return term;
      break;
    case CONSTANT:
      //Term holds its value on the right child
      return term->right;
      break;
    case STRING_LITERAL:
      //Term holds its value on the left child
      return term->left;
    case APPLY:
      //Process calling of functions
      break;
    case VOID: case FUNCTION: case INT:
      break;
    case 'd':
      //Left child is an AST for Return type
      tac_generator(term->left,seq);
      //Right child is the Name and arguments
      tac_generator(term->right,seq);
      break;
    case 'D': 
      //Left child is the Function definition
      tac_generator(term->left,seq);
      //Right child is the Function body
      if(term->right != NULL) {
        TAC *block = (TAC *)malloc(sizeof(TAC));
        // TODO: make it track how many variables have been declared
        block->args.block.nvars = 1;
        append(seq,block);
        return tac_generator(term->right,seq);
      } else {
        return 0;
      }
      break;
    case 'F':
      {
        //Left child is the Name of function
        //returns identifier assign it to the linked list of TACS
        TAC *func = (TAC *)malloc(sizeof(TAC));
        func->args.call.name = tac_generator(term->left,seq);
        //Right child are the Arguments of function 
        // TODO: look at how many arguments and load them
        if(term->right != NULL) {
          tac_generator(term->right,seq);
        }else {
          func->args.call.arity = 0;
        }
        append(seq,func);
        break;
      }
    case CONTINUE: case BREAK:
      break;
    case RETURN:
     {
      //Left child is an AST of the expression whose value is to be returned
      // TODO: handle the case where left child is identifier and the case for everything else
      TAC *ret = (TAC *)malloc(sizeof(TAC));
      if(term->left != NULL) {
        if(term->left->type == LEAF && term->left->left->type == IDENTIFIER) {
          //return find_name(tac_generator(term->left,seq));
          ret->args.ret = 69;
          append(seq,ret);
          return *seq;
        } else 
          // destination is returned
          ret->args.ret = tac_generator(term->left,seq);
          append(seq,ret); 
          return *seq;
      } else{
        // TODO: throw an error
        printf("Error no return type\n");
      }
      break;
      }
    case '~':
      break;
    case ';':
      //Left child is the first item or a sequence; returned values are ignored
      tac_generator(term->left,seq);
      //Right child is the last item and also the return value
      return tac_generator(term->right,seq);
      break;
    case '=':
      break;
    case '+': case '-': case '*': case '/': case '%': case '>': case '<': case NE_OP: case EQ_OP: case LE_OP: case GE_OP:
      {
        TAC *ret = (TAC *)malloc(sizeof(TAC));
        ret->op = term->type;
        //New temporary node
        TOKEN *dst = (TOKEN *)malloc(sizeof(TOKEN));
        ret->args.expr.dst = dst;
        ret->args.expr.src1 = tac_generator(term->left,seq);
        ret->args.expr.src2 = tac_generator(term->right,seq);
        append(seq,ret);
        //used when we have operation as a return statement
        return dst;
        break;
      }
    case IF:
      break;
    case WHILE:
      break;
    default:
      break;
  }
}

//do not read first node
void append(TAC **seq, TAC *new_node) {
  new_node->next = NULL;
  TAC *last = *seq;

  if(*seq == NULL) {
    *seq = new_node;
    return;
  }
  
  while(last->next != NULL) {
    last = last->next;
  }
  last->next = new_node;
  return;
}

void printTAC(TAC *seq) {
  printf("%d\n",seq->args); 
  printf("%d",seq->next->args);

}