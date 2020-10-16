#include <stdio.h>
#include <ctype.h>
#include "nodes.h"
#include "C.tab.h"
#include <string.h>

typedef struct value {
  int type ;
  union {
    int integer ;
    int boolean ; 
    char * string ;
  } v;
} VALUE ;
/*, ENV* env*/
    
void interpret(NODE *term) {
  switch(term->type) {
    case LEAF:
      //Left child has value: STRING_LITERAL, IDENTIFIER, CONSTANT
      interpret(term->left);
      break;
    case IDENTIFIER:
      printf("welcome brothas this is an identifier \n\n");
      break;
    case CONSTANT: case STRING_LITERAL:
      printf("welcome brothas this is a string literal  \n\n");
      break;
    case APPLY:
      break;
    case VOID: case FUNCTION: case INT:
      printf("welcome brothas this is an int   \n\n");
      break;
    case 'd':
      //AST for Return type
      interpret(term->left);
      //Function definition
      interpret(term->right);
      break;
    case 'D': 
      //Function definition
      interpret(term->left);
      //Return value
      interpret(term->right);
      break;
    case 'F':
      //Name of function
      interpret(term->left);
      //Arguments of function *** HAVE TO TEST IF EMPTY ***
      //interpret(term->right);
      break;
    case CONTINUE: case BREAK:
      break;
    case RETURN:
      //Left child is an AST of the expression whose value is to be returned
      interpret(term->left);
      break;
    case '~':
      break;
    case ';':
      break;
    case '=':
      break;
    case '+': case '-': case '*': case '/': case '%': case '>': case '<': case NE_OP: case EQ_OP: case LE_OP: case GE_OP:
      break; 
    case IF:
      break;
    case WHILE:
      break;
    default:
      break;
  }
}

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
        printf("%d", tree->type);
      print_leaf(tree->left, level);
    }
    else {
      for(i=0; i<level; i++) putchar(' ');
      printf("%d", tree->type);
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
    NODE* tree;
    if (argc>1 && strcmp(argv[1],"-d")==0) yydebug = 1;
    init_symbtable();
    printf("--C COMPILER\n");
    yyparse();
    tree = ans;
    printf("parse finished with %p\n", tree);
    print_tree(tree);
    tree = ans;
    interpret(tree);
    //interpret(tree);
    return 0;
}
