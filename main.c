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

typedef struct binding {
  TOKEN *name ;
  VALUE *val ;
  struct binding *next ; 
} BINDING ;

typedef struct frame {
BINDING *bindings ;
struct frame *next ; 
} FRAME ;


VALUE* read_int() {
  int x; 
  x = 5;
  return ( VALUE *) x;
}
    
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
      break;
    case VOID: case FUNCTION: case INT:
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
      if(term->right != NULL) {
        return interpret(term->right);
      } else {
        return (VALUE *) 0;
      }
      break;
    case 'F':
      //Name of function
      interpret(term->left);
      //Arguments of function 
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
      printf("arrives here");
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
      print_leaf(tree->left, level);
    }
    else {
      for(i=0; i<level; i++) putchar(' ');
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
    int exit_code;
    exit_code = interpret(tree);   
    printf("\nTerminated with exit code '%d'\n",exit_code); 
    return 0;
}
