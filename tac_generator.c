#include "nodes.h"
#include "C.tab.h"
#include "tac_generator.h"
#include <stdlib.h>
#include "interpreter.h"

/*Function definitions*/
void append(TAC **,TAC *);
char *treg_generator();

/*Global Variables*/
int ntreg = 0;
int nvars = 0;
char *svars[31];

void *tac_generator(NODE* term, TAC** seq) {
  switch(term->type) {
    case LEAF:
      //Left child has value: STRING_LITERAL, IDENTIFIER, CONSTANT
      return tac_generator(term->left, seq); 
      break;
    case IDENTIFIER: {
      return term;
      break;
    }
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
        block->next = NULL;
        block->op = BLOCK_OP; /* This is the OP CODE for BLOCK*/
        // TODO: make it track how many variables have been declared
        // maybe at the end of the return
        block->args.block.nvars = &nvars;
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
        func->next = NULL;
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
      ret->next = NULL;
      ret->op = RETURN;
      if(term->left != NULL) {
        if(term->left->type == LEAF && term->left->left->type == IDENTIFIER) {
          //return find_name(tac_generator(term->left,seq));
          ret->args.ret = ((TOKEN *)tac_generator(term->left,seq))->lexeme;
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
      //Left child is the type 
      tac_generator(term->left,seq);
      //Right child is the variable name
      if(term->right->type == LEAF){ 
        svars[nvars] = ((TOKEN *)tac_generator(term->right,seq))->lexeme;
      } else {
        //Right child is the AST "=" and we declare the variable before assigning
        svars[nvars] = ((TOKEN *)tac_generator(term->right->left,seq))->lexeme;
        tac_generator(term->right,seq);
      }
      nvars++;
      break;
    case ';':
      //Left child is the first item or a sequence; returned values are ignored
      tac_generator(term->left,seq);
      //Right child is the last item and also the return value
      return tac_generator(term->right,seq);
      break;
    case '=':
    {
      char *treg = treg_generator();
      //Creates load TAC instruction
      TAC *load = malloc(sizeof(TAC));
      load->next = NULL;
      load->op = LOAD_OP;
      load->args.load.treg = treg;
      load->args.load.value = tac_generator(term->right,seq);
      append(seq,load);
      //Creates store TAC instruction
      TAC *store = malloc(sizeof(TAC));
      store->next = NULL;
      store->op = STORE_OP;
      store->args.store.treg = treg;
      store->args.store.identifier = ((TOKEN *)tac_generator(term->left,seq))->lexeme;
      append(seq,store);
      break;
    }
    case '+': case '-': case '*': case '/': case '%': case '>': case '<': case NE_OP: case EQ_OP: case LE_OP: case GE_OP:
      {
        char *treg1 = treg_generator();
        char *treg2 = treg_generator();
        char *treg3 = treg_generator();

        //src1
        //could be an identifier aswell...
        TAC *load1 = malloc(sizeof(TAC));
        load1->next = NULL;
        load1->op = LOAD_OP;
        load1->args.load.treg = treg1;
        load1->args.load.value = tac_generator(term->left,seq);
        append(seq,load1);

        //src2
        TAC *load2 = malloc(sizeof(TAC));
        load2->next = NULL;
        load2->op = LOAD_OP;
        load2->args.load.treg = treg2;
        load2->args.load.value = tac_generator(term->right,seq);
        append(seq,load2);

        TAC *add = (TAC *)malloc(sizeof(TAC));
        add->next = NULL;
        add->op = term->type;

        //assign tregs to add TAC instruction
        add->args.expr.src1 = treg1;
        add->args.expr.src2 = treg2;
        add->args.expr.dst = treg3;
        append(seq,add);
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

char *treg_generator() {
  char *str = malloc(3 * sizeof(char));
  snprintf(str,sizeof(str),"t%d",ntreg++);
  return str;
}

void printTAC(TAC *seq) {
  TAC *temp = seq;
  while(temp != NULL) {
    switch (temp->op)
    {
    case FUNCTION:
      printf("proc %s ()\n",temp->args.call.name->lexeme);
      break;
    case BLOCK_OP:
      printf("block %d\n",*(temp->args.block.nvars));
      break;
    case LOAD_OP:
      printf("load %s %d\n",temp->args.load.treg, temp->args.load.value);
      break;
    case STORE_OP:
      printf("store %s %s\n",temp->args.store.treg, temp->args.store.identifier);
      break;
    case RETURN:
      printf("return %s\n",temp->args.ret);
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