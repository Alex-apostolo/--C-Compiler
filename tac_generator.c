#include <stdlib.h>
#include "nodes.h"
#include "C.tab.h"
#include "tac_generator.h"
#include "interpreter.h"

/*Function definitions*/
void append(TAC **,TAC *);
char *treg_generator();
TAC *create_load_TAC(TOKEN *term);
TAC *create_store_TAC(TOKEN *term);

/*Global Variables*/
int ntreg = 0;
char *latest_treg;
int nvars = 0;
char *svars[31];

void *tac_generator(NODE* term, TAC** seq) {
  switch(term->type) {
    case LEAF:
      //Left child has value: STRING_LITERAL, IDENTIFIER, CONSTANT
      return tac_generator(term->left, seq); 
      break;
    case IDENTIFIER: case CONSTANT: case STRING_LITERAL:
      //Interpret as TOKEN*
      return term;
      break;
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
        if(term->left->type == LEAF) {
          TOKEN *temp = tac_generator(term->left,seq);
          if(term->left->left->type == IDENTIFIER) ret->args.ret = temp->lexeme;
          else if(term->left->left->type == CONSTANT) ret->args.ret = my_itoa(temp->value);
        } else ret->args.ret = tac_generator(term->left,seq);
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
      //Creates TAC for rhs of expression
      if(term->right->type == LEAF) {
        TAC *rhs = create_load_TAC(tac_generator(term->right,seq));
        append(seq,rhs);
      }else tac_generator(term->right,seq);

      //Creates TAC for lhs of expression
      TAC *lhs = create_store_TAC(tac_generator(term->left,seq));
      append(seq,lhs);
      break;
    }
    case '+': case '-': case '*': case '/': case '%': case '>': case '<': case NE_OP: case EQ_OP: case LE_OP: case GE_OP:
      {
        //Creates load TAC instructions
        TAC *src1 = create_load_TAC(tac_generator(term->left,seq));
        append(seq,src1);

        TAC *src2 = create_load_TAC(tac_generator(term->right,seq));
        append(seq,src2);

        //Creates add TAC instruction
        TAC *add = (TAC *)malloc(sizeof(TAC));
        add->next = NULL;
        add->op = term->type;

        add->args.expr.src1 = src1->args.load.treg;
        add->args.expr.src2 = src2->args.load.treg;
        add->args.expr.dst = treg_generator();
        append(seq,add);
        return add->args.expr.dst;
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

char* my_itoa(int num) {
   char *str = malloc(100 * sizeof(char));
   sprintf(str, "%d", num);
   return str;
}

char *treg_generator() {
  char *str = malloc(3 * sizeof(char));
  snprintf(str,sizeof(str),"t%d",ntreg++);
  latest_treg = str;
  return str;
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

TAC *create_load_TAC(TOKEN *term) {
  char *treg = treg_generator();
  TAC *load = malloc(sizeof(TAC));
  load->next = NULL;
  load->op = LOAD_OP;
  load->args.load.treg = treg;
  if(term->type == IDENTIFIER) load->args.load.value = term->lexeme;
  else{
    char *temp = malloc(100*sizeof(char));
    snprintf(temp,100,"%d",term->value);
    load->args.load.value = temp;
  }
  return load;
}

TAC *create_store_TAC(TOKEN *term) {
  char *treg = latest_treg;
  TAC *store = malloc(sizeof(TAC));
  store->next = NULL;
  store->op = STORE_OP;
  store->args.store.treg = treg;
  if(term->type == IDENTIFIER) store->args.store.value = term->lexeme;
  else{
    char *temp = malloc(100*sizeof(char));
    snprintf(temp,100,"%d",term->value);
    store->args.store.value = temp;
  }
  return store;
}

void printTAC(TAC *seq) {
  printf("\n");
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
      printf("load %s %s\n",temp->args.load.treg, temp->args.load.value);
      break;
    case STORE_OP:
      printf("store %s %s\n",temp->args.store.treg, temp->args.store.value);
      break;
    case RETURN:
      printf("return %s\n",temp->args.ret);
      break;
    case '+':
      printf("add %s %s %s\n",temp->args.expr.src1, temp->args.expr.src2, temp->args.expr.dst);
      break;
    default:
      break;
    }
    temp = temp->next;
  }
  printf("\n");
}