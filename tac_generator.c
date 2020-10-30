#include "nodes.h"
#include "C.tab.h"
#include "tac_generator.h"
#include <stdlib.h>
#include "interpreter.h"

#define BLOCK_CON 278

void append(TAC **,TAC *);

void *tac_generator(NODE* term, TAC** seq) {
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
        block->op = BLOCK_CON; /* This is the OP CODE for BLOCK*/
        // TODO: make it track how many variables have been declared
        // maybe at the end of the return
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
        TAC *func = (TAC *)malloc(sizeof(TAC));
        func->op = FUNCTION;
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
      ret->op = RETURN;
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
        //Generate new temporary register name
        ret->args.expr.dst = 2;
        ret->args.expr.src1 = tac_generator(term->left,seq);
        ret->args.expr.src2 = tac_generator(term->right,seq);
        append(seq,ret);
        return seq;
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

  if(*seq == NULL) {
    *seq = new_node;
    return;
  }

  TAC *last = *seq;
  while(last->next != NULL) {
    last = last->next;
  }
  last->next = new_node;
  return;
}

void printTAC(TAC *seq) {
  TAC *temp = seq;
  while(temp != NULL) {
    switch (temp->op)
    {
    case FUNCTION:
      printf("proc %s ()\n",temp->args.call.name->lexeme);
      break;
    case BLOCK_CON:
      printf("block %d\n",temp->args.block.nvars);
      break;
    case RETURN:
      printf("return %d\n",temp->args.ret);
      break;
    case '+':
      printf("add %d %d %d\n", temp->args.expr.dst, temp->args.expr.src1, temp->args.expr.src2);
      break;
    default:
      break;
    }
    temp = temp->next;
  }
}