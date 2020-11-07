#include <stdio.h>
#include <ctype.h>
#include "nodes.h"
#include "C.tab.h"
#include <string.h>
#include "interpreter.h"
#include "tac_generator.h"
#include "mips_generator.h"
#include <unistd.h>
#include <stdbool.h>



char *named(int t)
{
    static char b[100];
    if (isgraph(t) || t==' ') {
      sprintf(b, "%c", t);
      return b;
    }
    switch (t) {
      default: return "???";
    case IDENTIFIER:
      return "id";
    case CONSTANT:
      return "constant";
    case STRING_LITERAL:
      return "string";
    case LE_OP:
      return "<=";
    case GE_OP:
      return ">=";
    case EQ_OP:
      return "==";
    case NE_OP:
      return "!=";
    case EXTERN:
      return "extern";
    case AUTO:
      return "auto";
    case INT:
      return "int";
    case VOID:
      return "void";
    case APPLY:
      return "apply";
    case LEAF:
      return "leaf";
    case IF:
      return "if";
    case ELSE:
      return "else";
    case WHILE:
      return "while";
    case CONTINUE:
      return "continue";
    case BREAK:
      return "break";
    case RETURN:
      return "return";
    }
}

void print_leaf(NODE *tree, int level)
{
    TOKEN *t = (TOKEN *)tree;
    int i;
    for (i=0; i<level; i++) putchar(' ');
    if (t->type == CONSTANT) printf("%d\n", t->value);
    else if (t->type == STRING_LITERAL) printf("\"%s\"\n", t->lexeme);
    else if (t) puts(t->lexeme);
}

void print_tree0(NODE *tree, int level)
{
    int i;
    if (tree==NULL) return;
    if (tree->type==LEAF) {
      //printf("%d", tree->type);
      print_leaf(tree->left, level);
    }
    else {
      for(i=0; i<level; i++) putchar(' ');
      //printf("%d", tree->type);
      printf("%s\n", named(tree->type));
/*       if (tree->type=='~') { */
/*         for(i=0; i<level+2; i++) putchar(' '); */
/*         printf("%p\n", tree->left); */
/*       } */
/*       else */
        print_tree0(tree->left, level+2);
      print_tree0(tree->right, level+2);
    }
}

void print_tree(NODE *tree)
{
    print_tree0(tree, 0);
}

extern int yydebug;
extern NODE* yyparse(void);
extern NODE* ans;
extern void init_symbtable(void);

int main(int argc, char** argv)
{
    bool interp = false;
    bool tac = false;
    bool mips = false;

    NODE* tree;
    //First frame is the main function
    ENV *env = (ENV *)malloc(sizeof(ENV));
    TAC *seq;

    int option;
    while((option = getopt(argc,argv,"diItTsS")) != -1) {
      switch (option){
      case 'd':
        yydebug = 1;
        printf("d");
        break;
      case 'i': case 'I':
        interp = true;
        break;
      case 't': case 'T':
        tac = true;
        break;
      case 's': case 'S':
        mips = true;
        break;

      default:
      //do the default shit
        break;
      }
    }

    if(!(interp ^ tac ^ mips)) {
      fprintf(stderr,"Flags -i/I,-t/T,-s/S should be mutually exlusive!\n");
      return -1;
    }

    init_symbtable();
    printf("--C COMPILER\n");
    yyparse();
    tree = ans;
    printf("parse finished with %p\n", tree);
    print_tree(tree);
    tree = ans;

    if(interpret == true){
      VALUE *exit_code = interpret(tree,env);
      printf("\nTerminated with exit code '%d'\n",exit_code);
      return exit_code;
    } 

    if(tac == true){
      tac_generator(tree,&seq);
      printTAC(seq);
      return 0;
    } 

    if(mips == true){
      tac_generator(tree,&seq);
      mips_generator(seq);
      return 0;
    }
}