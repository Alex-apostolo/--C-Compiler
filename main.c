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
    int interp = false;
    int tac = false;
    int mips = false;

    NODE* tree;
    TAC *seq;

    ENV *env = malloc(sizeof(ENV));
    FRAME *main = malloc(sizeof(FRAME));
    env->frames = main;

    int option;
    while((option = getopt(argc,argv,"pdist")) != -1) {
      switch (option){
      case 'p':
        init_symbtable();
        printf("--C COMPILER\n");
        yyparse();
        tree = ans;
        printf("parse finished with %p\n", tree);
        print_tree(tree);
        tree = ans;

        VALUE *exit_code = interpret(tree,env);
        printf("\nTerminated with exit code '%d'\n",exit_code);
        return exit_code;
        break;
      case 'd':
        yydebug = 1;
        break;
      case 'i':
        interp = true;
        break;
      case 's':
        mips = true;
        break;
      case 't': 
        tac = true;
        break;
      default:
        break;
      }
    }

    if((interp + tac + mips) > 1) {
      fprintf(stderr,"ERROR: Flags -i,-t,-s must be mutually exlusive!\n");
      return -1;
    }

    char *filename;
    init_symbtable();

    if ((filename = argv[optind])) {

      if(access(filename,R_OK) == -1){
        fprintf(stderr,"ERROR: File specified does not exist!");
        return -1;
      }

      //read from current directory
      FILE *file = fopen(argv[optind],'r');
      char *singleLine[150];
      char *wholeFile[500*150];
      while(feof(file)){
        fgets(singleLine,150,file);
        strcat(wholeFile,singleLine);
      }
      yy_scan_string(wholeFile);
      yyparse();
      tree = ans;
      fclose(file);

    }

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